module;
export module app_handlers:cache;

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

export namespace app::handlers::cache {

/**
 * @brief Process-wide, in-process substitute for Redis pub/sub — the exact
 * infrastructure upstream uses to broadcast cache invalidations
 * (storage_impl/src/redis/pub_sub.rs:9-29,32-89). `publish()` fans the message
 * out to every subscriber of the channel and returns the number of subscribers
 * that received it — precisely the `usize` Redis `PUBLISH` returns and the
 * `result` summed by `redact_from_redis_and_publish`
 * (storage_impl/src/redis/cache.rs:988-1012).
 *
 * Upstream installs one process-wide subscriber whose `on_message` invalidates
 * the shared `Caches` for every tenant (app.rs:566; pub_sub.rs:34-69,92-133).
 * Here each AppContext registers one subscription, so a publish invalidates
 * every registered instance's cache — a real, in-process cross-instance fan-out
 * rather than a single local `cache().erase()`.
 *
 * The instance is intentionally never destroyed (leaky singleton): it is a
 * process-wide registry whose subscriptions live for the whole process, so it
 * must not be torn down by static destruction before a late publish.
 */
class InProcessPubSubBroker
{
public:
    using Handler = std::function<void(std::string_view)>;

    [[nodiscard]] static InProcessPubSubBroker& instance()
    {
        static InProcessPubSubBroker* broker = new InProcessPubSubBroker{};
        return *broker;
    }

    /** @brief Mirrors `subscriber.subscribe(channel)` (pub_sub.rs:34-69). */
    [[nodiscard]] std::size_t subscribe(std::string channel, Handler handler)
    {
        std::lock_guard lock{m_mutex};
        const std::size_t id = m_next_id++;
        m_channels[std::move(channel)].emplace_back(id, std::move(handler));
        return id;
    }

    void unsubscribe(std::size_t id)
    {
        std::lock_guard lock{m_mutex};
        for (auto& [channel, handlers]: m_channels) {
            std::erase_if(handlers, [id](const auto& entry) { return entry.first == id; });
        }
    }

    /**
     * @brief Mirrors `publisher.publish(channel, message)` (pub_sub.rs:72-89):
     * delivers the payload to all subscribers and returns the receiver count.
     */
    [[nodiscard]] std::size_t publish(std::string_view channel, std::string_view message) const
    {
        std::vector<Handler> targets;
        {
            std::lock_guard lock{m_mutex};
            const auto it = m_channels.find(std::string{channel});
            if (it == m_channels.end()) {
                return 0;
            }
            targets.reserve(it->second.size());
            for (const auto& [id, handler]: it->second) {
                (void)id;
                targets.push_back(handler);
            }
        }
        for (const auto& handler: targets) {
            handler(message);
        }
        return targets.size();
    }

private:
    mutable std::mutex m_mutex{};
    std::unordered_map<std::string, std::vector<std::pair<std::size_t, Handler>>> m_channels{};
    std::size_t m_next_id{0};
};

class CacheRouteHandlers
{
public:
    void handle_invalidate(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        // Flow::CacheInvalidate instrumentation: upstream records this through
        // server_wrap (routes/cache.rs:16,20-28). The local metrics facade is the
        // in-repo substitute; record it up front so it also covers auth failures.
        m_context->metrics().record_flow("CacheInvalidate");

        // Upstream wraps this route in server_wrap(Flow::CacheInvalidate,
        // AdminApiAuth, LockAction::NotApplicable) (routes/cache.rs:20-28).
        // AdminApiAuth is an admin-api-key bearer check (authentication.rs:
        // 1713-1767). The registry row "invalidate" is the declarative local
        // form: PLATFORM_ADMIN + ADMIN + bearer requirement
        // (auth/registry.cppm:187; route_auth.cppm:43,74-78). NotApplicable
        // means no lock is acquired here.
        const auto rule = m_context->auth_rules().rule_for("invalidate");
        const auto auth = m_context->authorizer().authorize_route(app::auth::AuthContextBuilder{}(req), rule);
        if (auth) {
            send_auth_error(res, send);
            return;
        }

        // Upstream takes the key from web::Path<String> (/cache/invalidate/:key);
        // no body is read (routes/cache.rs:14,18). Read the matched path segment.
        const auto key = cache_key_from_request(req);

        // Exact port of core::cache::invalidate (core/cache.rs:8-24):
        //   redact_from_redis_and_publish(store, [CacheKind::All(key)]) -> result;
        //   if result > 0 -> StatusOk, else -> InternalServerError.
        // A zero-delete Redis reply is only logged upstream (cache.rs:988-998),
        // never a failure; a cache miss is therefore not the success gate.
        const std::size_t result = redact_from_redis_and_publish(key);
        if (result == 0) {
            // core/cache.rs:21-24: InternalServerError with the printable
            // attachment "Failed to invalidate cache".
            send_internal_server_error(res, send);
            return;
        }

        m_context->metrics().counter("CACHE_INVALIDATED").increment();

        // ApplicationResponse::StatusOk has no body (core/cache.rs:20;
        // services/api.rs:499 -> http_response_ok()).
        send_json(res, send, std::string_view{}, true);
    }

    void install(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "/cache"}
            .add_route(
                Route{"/invalidate/:key"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_invalidate(req, res, std::move(send));
                    })
                    .summary("Hyperswitch invalidate")
            );
    }

    explicit CacheRouteHandlers(app::context::AppContext& context)
        : m_context{&context}
    {
        // Upstream's process-wide subscriber invalidates the shared Caches for
        // every CacheKind (app.rs:566; pub_sub.rs:34-69,92-133). Register this
        // instance's cache as such a subscriber: on_message rebuilds the
        // CacheKey from the CacheRedact tenant + kind and removes it from every
        // cache selected for that kind (pub_sub.rs:107-123). CacheKind::All
        // selects all caches, so the key is erased from this instance's cache.
        app::context::AppContext* ctx = m_context;
        m_subscription = InProcessPubSubBroker::instance().subscribe(
            kInvalidationChannel,
            [ctx](std::string_view message) {
                if (const auto key = key_from_cache_redact(message); !key.empty()) {
                    ctx->cache().erase(key);
                }
            }
        );
    }

private:
    /** @brief DEFAULT_IMC_INVALIDATION_CHANNEL (storage_impl/redis/cache.rs:23,190-193). */
    static constexpr std::string_view kInvalidationChannel{"hyperswitch_invalidate"};

    /**
     * @brief Port of storage_impl::redis::cache::redact_from_redis_and_publish
     * (core/cache.rs:13; storage_impl/src/redis/cache.rs:967-1012):
     *   1. DEL the unprefixed key from the cache (miss is logged, not an error).
     *   2. PUBLISH CacheRedact { tenant, kind: All(key) } to the invalidation
     *      channel; every subscriber runs on_message and invalidates its caches.
     *   3. Return the published-channel count (sum of PUBLISH replies).
     */
    [[nodiscard]] std::size_t redact_from_redis_and_publish(std::string_view key)
    {
        m_context->cache().erase(key);
        return InProcessPubSubBroker::instance().publish(
            kInvalidationChannel,
            encode_cache_redact(key)
        );
    }

    [[nodiscard]] static std::string json_escape(std::string_view value)
    {
        std::string out{};
        out.reserve(value.size());
        for (const char ch: value) {
            switch (ch) {
            case '\\': out += "\\\\"; break;
            case '"': out += "\\\""; break;
            case '\n': out += "\\n"; break;
            case '\r': out += "\\r"; break;
            case '\t': out += "\\t"; break;
            default: out.push_back(ch); break;
            }
        }
        return out;
    }

    /**
     * @brief serde wire of `CacheRedact { tenant, kind: CacheKind::All(key) }`
     * (storage_impl/src/redis/cache.rs:372-392,413-421): the externally-tagged
     * enum kind serializes as {"All":"<key>"}. "public" is DEFAULT_TENANT
     * (common_utils/src/consts.rs:103), the local tenant-prefix substitute.
     */
    [[nodiscard]] static std::string encode_cache_redact(std::string_view key)
    {
        return std::format(
            "{{\"tenant\":\"public\",\"kind\":{{\"All\":\"{}\"}}}}",
            json_escape(key)
        );
    }

    /** @brief Inverse of encode_cache_redact, used by the on_message subscriber. */
    [[nodiscard]] static std::string key_from_cache_redact(std::string_view message)
    {
        constexpr std::string_view marker{R"("All":")"};
        const auto pos = message.find(marker);
        if (pos == std::string_view::npos) {
            return {};
        }
        const std::size_t begin = pos + marker.size();
        std::size_t end = begin;
        while (end < message.size()) {
            if (message[end] == '\\' && end + 1 < message.size()) {
                end += 2; // skip the whole escape sequence, incl. an escaped quote
                continue;
            }
            if (message[end] == '"') {
                break;
            }
            ++end;
        }
        std::string out{};
        out.reserve(end - begin);
        for (std::size_t i = begin; i < end; ++i) {
            if (message[i] == '\\' && i + 1 < end) {
                switch (message[i + 1]) {
                case '"': out.push_back('"'); break;
                case '\\': out.push_back('\\'); break;
                case 'n': out.push_back('\n'); break;
                case 'r': out.push_back('\r'); break;
                case 't': out.push_back('\t'); break;
                default: out.push_back(message[i + 1]); break;
                }
                ++i;
            } else {
                out.push_back(message[i]);
            }
        }
        return out;
    }

    [[nodiscard]] std::string cache_key_from_request(interfaces::io::IRequest& req) const
    {
        const auto segments = app::http::RequestPath{}(req);
        // The :key route parameter is the third segment of /cache/invalidate/:key.
        if (const auto key = segments.at(2U)) {
            return std::string{*key};
        }
        // Fall back to the last segment if the router mounts the route under a
        // longer prefix, preserving upstream's web::Path<String> semantics.
        if (segments.count() > 0U) {
            return std::string{segments.at(segments.count() - 1U).value_or("")};
        }
        return {};
    }

    /**
     * @brief Exact ApiErrorResponse::InternalServerError wire body.
     * ApiError::new("HE", 0, "Something went wrong", None) -> code "HE_00",
     * type "api"; Display wraps it in {"error": ...} (api_models/errors/types.rs
     * :60-70,89-118; hyperswitch_domain_models/errors/api_error_response.rs
     * :422-423). The attach_printable("Failed to invalidate cache") is an
     * attachment only and is not part of current_context()/the wire body
     * (core/cache.rs:22-23). HTTP 500 (api_models/errors/actix.rs:15).
     */
    void send_internal_server_error(
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) const
    {
        constexpr std::string_view body =
            R"({"error":{"type":"api","message":"Something went wrong","code":"HE_00"}})";
        auto bytes = serde::Ser::serialize_raw("application/json", body);
        res.set_status(interfaces::io::types::Status::INTERNAL_SERVER_ERROR);
        res.set_body(std::move(bytes));
        send();
    }

    /**
     * @brief Exact AdminApiAuth failure wire body. Upstream returns
     * ApiErrorResponse::Unauthorized -> ApiError::new("IR", 1,
     * "API key not provided or invalid API key used", None): code "IR_01",
     * type "invalid_request", HTTP 401 (api_error_response.rs:560-564;
     * api_models/errors/types.rs:159-176; actix.rs:8,25-31).
     */
    void send_auth_error(
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) const
    {
        constexpr std::string_view body =
            R"({"error":{"type":"invalid_request","message":"API key not provided or invalid API key used","code":"IR_01"}})";
        auto bytes = serde::Ser::serialize_raw("application/json", body);
        res.set_status(interfaces::io::types::Status::UNAUTHORIZED);
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
    std::size_t m_subscription{0};
};

} // namespace app::handlers::cache
