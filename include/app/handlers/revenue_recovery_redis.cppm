module;
export module app_handlers:revenue_recovery_redis;

import std;
import interfaces;
import core_router;
import core_serde;
import utils_openapi;
import app_model;
import app_dto;
import app_context;
import core_cache;
import core_error;
import app_auth;
import app_http;
import app_metrics;

export namespace app::handlers::revenue_recovery_redis {

/**
 * @brief Output of the ported `RedisTokenManager::get_redis_key_data_raw`.
 *
 * Mirrors the upstream tuple `(bool, i64, Option<serde_json::Value>)`:
 * `exists` (key/hash presence), `ttl_seconds` (Redis TTL), `data` (the raw
 * Redis value materialized as a JSON value).
 *
 * Upstream: types/storage/revenue_recovery_redis_operation.rs:1073-1146.
 */
struct RawRedisData
{
    bool exists{false};
    std::int64_t ttl_seconds{-2};
    serde::Value data{};
};

class RevenueRecoveryRedisRouteHandlers
{
public:
    void handle_get_revenue_recovery_redis_data(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        // Upstream route uses V2AdminApiAuth (revenue_recovery_redis.rs:30). The
        // declarative rule in RouteAuthRegistry is PLATFORM_ADMIN / ADMIN; use it
        // instead of building a rule inline so the permission stays in one place.
        const auto rule = m_context->auth_rules().rule_for("get_revenue_recovery_redis_data");
        const auto auth = m_context->authorizer().authorize_route(
            app::auth::AuthContextBuilder{}(req),
            rule
        );
        if (auth) {
            send_error(res, send, *auth);
            return;
        }

        // Upstream route: /:connector_customer_id with web::Path<String>. The id is
        // the final non-empty path segment, read via RequestPath rather than a manual
        // rfind on get_path (which also failed to strip a trailing query string).
        const auto segments = app::http::RequestPath{}(req);
        const auto id_index = segments.count() == 0 ? std::size_t{0} : segments.count() - 1;
        const auto connector_customer_id = std::string{segments.at(id_index).value_or("")};
        if (connector_customer_id.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(
                core::error::AppErrorKind::INVALID_REQUEST,
                "missing connector_customer_id"
            ));
            return;
        }

        // Upstream input: web::Query<GetRedisDataQuery>.key_type with enum
        // RedisKeyType { Status, Tokens } (api_models:204-213). serde rejects a
        // missing or unknown variant, so do the same instead of silently defaulting
        // to Status.
        const auto key_type_raw = app::http::RequestQuery{}(req).get("key_type");
        bool is_tokens{false};
        if (!key_type_raw) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(
                core::error::AppErrorKind::INVALID_REQUEST,
                "missing key_type; expected Status or Tokens"
            ));
            return;
        }
        if (*key_type_raw == "Status") {
            is_tokens = false;
        } else if (*key_type_raw == "Tokens") {
            is_tokens = true;
        } else {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(
                core::error::AppErrorKind::INVALID_REQUEST,
                "invalid key_type; expected Status or Tokens"
            ));
            return;
        }

        // Upstream key construction (revenue_recovery_redis_operation.rs:139-145,
        // :1086-1089): Status -> customer:{id}:status, Tokens -> customer:{id}:tokens.
        // The in-process cache is the local substitute for the Redis instance; the
        // natural Redis key is used verbatim, namespaced so it cannot collide with
        // unrelated cache entries.
        const auto redis_key = std::string{"customer:"} + connector_customer_id
            + (is_tokens ? ":tokens" : ":status");
        const auto cache_key = std::string{"revenue_recovery_redis.redis:"} + redis_key;

        // GenericNotFoundError mapping from core/revenue_recovery_data_backfill.rs:146-154.
        const auto not_found_message = std::string{
            "Redis data not found for connector customer id:- '"
        } + connector_customer_id + "'";

        // Upstream only errors when the Redis connection / get_ttl fails
        // (revenue_recovery_redis_operation.rs:1078-1098). A closed local cache is
        // the in-process equivalent of that infrastructure failure. A key that is
        // simply absent is NOT an error upstream: it yields a 200 response with
        // exists=false (see get_redis_key_data_raw below).
        if (!m_context->cache().is_open()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(
                core::error::AppErrorKind::NOT_FOUND,
                not_found_message
            ));
            return;
        }

        const auto raw = get_redis_key_data_raw(cache_key, is_tokens, connector_customer_id);

        // Flow::RevenueRecoveryRedis instrumentation (revenue_recovery_redis.rs:11, :18).
        m_context->metrics().record_flow("RevenueRecoveryRedis");
        m_context->metrics().counter("revenue_recovery_redis.get_revenue_recovery_redis_data").increment();

        // Upstream core/revenue_recovery_data_backfill.rs:130-144:
        // Ok((exists, ttl_seconds, data)) -> RedisDataResponse -> ApplicationResponse::Json.
        using Response = app::dto::revenue_recovery_data_backfill::RedisDataResponse;
        Response response;
        response.set_exists(raw.exists);
        response.set_ttl_seconds(raw.ttl_seconds);
        response.set_data(raw.data);
        auto text = serde::Ser::encode_json(response);
        send_json(res, send, text, true);
    }

    void install(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "/v2/recovery/data-backfill"}
            .add_route(
                Route{"/redis-data/:connector_cutomer_id"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_get_revenue_recovery_redis_data(req, res, std::move(send));
                    })
                    .summary("Hyperswitch get_revenue_recovery_redis_data")
            );
    }

    explicit RevenueRecoveryRedisRouteHandlers(app::context::AppContext& context) noexcept
        : m_context{&context}
    {
    }

private:
    /**
     * @brief Local, in-process port of `RedisTokenManager::get_redis_key_data_raw`.
     *
     * Upstream: types/storage/revenue_recovery_redis_operation.rs:1073-1146.
     * The cache entry is the substitute for the Redis key; expiry carries the TTL
     * and the raw string value carries the Redis value (a plain string for the
     * Status key, a JSON object for the Tokens hash).
     *
     * Behavior ported line-by-line:
     *  - TTL (:1091-1098): Redis TTL semantics -2 = missing, -1 = no expiry,
     *    otherwise remaining whole seconds.
     *  - Status (:1101-1114): get_key::<String> Ok -> (true, Value::String(v));
     *    Err -> (false, Value::String("Error retrieving status key: <err>")), where
     *    <err> is RedisError::GetFailed's display "Failed to get key value in Redis"
     *    (redis_interface/src/errors.rs:20-21).
     *  - Tokens (:1115-1134): get_hash_fields::<HashMap<String,String>> Ok ->
     *    exists = !map.is_empty(), data = Value(map) or Value::Object({}); Err ->
     *    (false, Value::Null). An absent key makes HGETALL return an empty map,
     *    i.e. Ok(empty) -> (false, Value::Object({})).
     */
    [[nodiscard]] RawRedisData get_redis_key_data_raw(
        std::string_view cache_key,
        bool is_tokens,
        std::string_view customer_id
    ) const
    {
        RawRedisData out;

        // Locate the value entry. For the Status lock key also look at the
        // producer's lock-registry key (app/locking/lock_registry.cppm:57-60,
        // revenue_recovery_data_backfill.cppm:442-445) so the substitute reflects
        // a lock actually acquired by the backfill flow. The lock registry stores
        // the owner (payment id) as a plain string, exactly the upstream Status
        // value (revenue_recovery_redis_operation.rs:162-168).
        std::optional<core::cache::CacheEntry> entry = find_entry(cache_key);
        if (!is_tokens && !entry) {
            const auto lock_key = std::string{"lock:revenue_recovery.status:"}
                + std::string{customer_id};
            entry = find_entry(lock_key);
        }
        const core::cache::CacheEntry* value_entry = entry ? &*entry : nullptr;

        // TTL (revenue_recovery_redis_operation.rs:1091-1098).
        out.ttl_seconds = -2;
        if (value_entry != nullptr) {
            if (value_entry->expires_at == core::cache::CacheEntry::Clock::time_point::max()) {
                out.ttl_seconds = -1;
            } else {
                out.ttl_seconds = std::max<std::int64_t>(
                    0,
                    std::chrono::duration_cast<std::chrono::seconds>(
                        value_entry->expires_at - core::cache::CacheEntry::Clock::now()
                    ).count()
                );
            }
        }

        if (!is_tokens) {
            // Status: get_key::<String> (:1102-1114).
            if (value_entry != nullptr && !core::cache::IsMiss{}(value_entry->value)) {
                auto text = core::cache::value_as<std::string>(value_entry->value, std::string{});
                // A test/producer may store the value JSON-quoted; unwrap it so it
                // matches the plain String redis returns from GET.
                if (auto decoded = serde::Ser::decode_generic("application/json", text)) {
                    if (const auto* scalar = std::get_if<std::string>(&decoded->get())) {
                        text = *scalar;
                    }
                }
                out.exists = true;
                out.data = serde::Value{std::move(text)};
            } else {
                out.exists = false;
                out.data = serde::Value{std::string{
                    "Error retrieving status key: Failed to get key value in Redis"
                }};
            }
            return out;
        }

        // Tokens: get_hash_fields::<HashMap<String,String>> (:1115-1134).
        if (value_entry == nullptr) {
            // HGETALL on a missing key -> empty map -> Ok(empty).
            out.exists = false;
            out.data = serde::Value{serde::Value::Object{}};
            return out;
        }
        if (core::cache::IsMiss{}(value_entry->value)) {
            out.exists = false;
            out.data = serde::Value{serde::Value::Object{}};
            return out;
        }
        const auto raw = core::cache::value_as<std::string>(value_entry->value, std::string{});
        const auto decoded = serde::Ser::decode_generic("application/json", raw);
        if (!decoded) {
            // Hash decode failure mirrors get_hash_fields Err -> (false, Value::Null).
            out.exists = false;
            out.data = serde::Value{};
            return out;
        }
        const auto* object = std::get_if<serde::Value::Object>(&decoded->get());
        if (object == nullptr) {
            out.exists = false;
            out.data = serde::Value{};
            return out;
        }
        out.exists = !object->empty();
        out.data = serde::Value{*object};
        return out;
    }

    [[nodiscard]] std::optional<core::cache::CacheEntry> find_entry(std::string_view key) const
    {
        core::cache::CacheQuerySpec spec;
        spec.prefix = std::string{key};
        for (const auto& entry : m_context->cache().execute(spec)) {
            if (entry.key == key) {
                return entry;
            }
        }
        return std::nullopt;
    }

    void send_error(
        interfaces::io::IResponse& res,
        std::function<void()> send,
        core::error::AppError error
    ) const
    {
        auto text = serde::Ser::encode_json(error);
        auto bytes = serde::Ser::serialize_raw("application/json", text);
        res.set_status(core::error::AppErrorStatus{}(error.get_kind()));
        res.set_body(std::move(bytes));
        send();
    }

    void send_json(
        interfaces::io::IResponse& res,
        std::function<void()> send,
        std::string_view text,
        bool ok
    ) const
    {
        auto bytes = serde::Ser::serialize_raw("application/json", text);
        res.set_status(ok ? interfaces::io::types::Status::OK
                          : interfaces::io::types::Status::BAD_REQUEST);
        res.set_body(std::move(bytes));
        send();
    }

    app::context::AppContext* m_context{nullptr};
};

} // namespace app::handlers::revenue_recovery_redis
