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
        // The local cache is the deterministic stand-in for the real Redis key:
        // same key-type-specific natural key, namespaced so it cannot collide with
        // unrelated cache entries. Real Redis access is an external dependency.
        const auto redis_key = std::string{"customer:"} + connector_customer_id
            + (is_tokens ? ":tokens" : ":status");
        const auto cache_key = std::string{"revenue_recovery_redis.redis:"} + redis_key;

        // GenericNotFoundError mapping from core/revenue_recovery_data_backfill.rs:146-154.
        const auto not_found_message = std::string{
            "Redis data not found for connector customer id:- '"
        } + connector_customer_id + "'";

        // Read-only lookup: upstream never writes here. A cache miss is a Redis
        // miss and maps to NOT_FOUND; the handler must not self-populate and then
        // report a hit (that was the old fake-success stand-in).
        core::cache::CacheQuerySpec spec;
        spec.prefix = cache_key;
        const auto entries = m_context->cache().execute(spec);
        const core::cache::CacheEntry* found = nullptr;
        for (const auto& entry : entries) {
            if (entry.key == cache_key) {
                found = &entry;
                break;
            }
        }
        if (found == nullptr || core::cache::IsMiss{}(found->value)) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(
                core::error::AppErrorKind::NOT_FOUND,
                not_found_message
            ));
            return;
        }

        // The producer stores the Redis value as a JSON string in the cache entry.
        // Status values are plain strings; Tokens values are hash maps encoded as
        // JSON objects. Decode into a typed Value rather than echoing path/query
        // inputs into a synthetic envelope.
        const auto payload = core::cache::value_as<std::string>(found->value, std::string{});
        std::optional<serde::Value> data;
        auto decoded = serde::Ser::decode_generic("application/json", payload);
        if (decoded) {
            data = std::move(*decoded);
        } else if (!payload.empty()) {
            // A raw status string is not JSON-quoted upstream; keep it as the
            // JSON string value that serde_json::Value::String would produce.
            data = serde::Value{payload};
        }
        if (!data) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(
                core::error::AppErrorKind::NOT_FOUND,
                not_found_message
            ));
            return;
        }

        // get_ttl substitute: the cache entry carries the real remaining TTL set
        // by the producer's put(key, value, ttl). Redis returns -1 for a key with
        // no expiry; expose 0 in that case rather than a fabricated value.
        std::int64_t ttl_seconds{0};
        if (found->expires_at != core::cache::CacheEntry::Clock::time_point::max()) {
            const auto now = core::cache::CacheEntry::Clock::now();
            ttl_seconds = std::max<std::int64_t>(
                0,
                std::chrono::duration_cast<std::chrono::seconds>(found->expires_at - now).count()
            );
        }

        // Existence derivation (revenue_recovery_redis_operation.rs:1101-1135):
        // Status key presence means exists=true. Tokens hash presence means the
        // decoded object is non-empty.
        bool exists{true};
        if (is_tokens) {
            if (const auto* object = std::get_if<serde::Value::Object>(&data->get())) {
                exists = !object->empty();
            } else {
                exists = !data->is_null();
            }
        }

        // Flow::RevenueRecoveryRedis instrumentation (revenue_recovery_redis.rs:11, :18).
        m_context->metrics().record_flow("RevenueRecoveryRedis");
        m_context->metrics().counter("revenue_recovery_redis.get_revenue_recovery_redis_data").increment();

        // Upstream response: RedisDataResponse { exists, ttl_seconds, data }.
        using Response = app::dto::revenue_recovery_data_backfill::RedisDataResponse;
        Response response;
        response.set_exists(exists);
        response.set_ttl_seconds(ttl_seconds);
        response.set_data(std::move(data));
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
