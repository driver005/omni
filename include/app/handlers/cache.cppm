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
import app_events;

export namespace app::handlers::cache {

/** @brief Typed envelope for the local stand-in of upstream CacheKind::All(key). */
class CacheInvalidateEvent
{
public:
    void set_kind(std::string value) { kind = std::move(value); }
    [[nodiscard]] const std::string& get_kind() const noexcept { return kind; }
    void set_key(std::string value) { key = std::move(value); }
    [[nodiscard]] const std::string& get_key() const noexcept { return key; }

    std::string kind{};
    std::string key{};
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
        // Upstream wraps this route in server_wrap(Flow::CacheInvalidate,
        // AdminApiAuth, LockAction::NotApplicable). Resolve the scheme from the
        // registry row "invalidate" (PLATFORM_ADMIN + ADMIN); NotApplicable means
        // no lock is acquired here. Flow instrumentation is recorded below.
        const auto rule = m_context->auth_rules().rule_for("invalidate");
        const auto auth = m_context->authorizer().authorize_route(app::auth::AuthContextBuilder{}(req), rule);
        if (auth) {
            send_error(res, send, *auth);
            return;
        }

        // Upstream takes the key from web::Path<String> (/cache/invalidate/:key);
        // no body is read. Read the matched path segment instead of parsing a
        // synthetic JSON document out of thin air.
        const auto key = cache_key_from_request(req);
        if (key.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::INVALID_REQUEST, "cache key is required"));
            return;
        }

        // Local half of redact_from_redis_and_publish(CacheKind::All(key)):
        // evict the raw key. Upstream does not treat a zero-deletion Redis reply
        // as an error, so a cache miss is not the success gate here.
        m_context->cache().erase(key);

        // Deterministic local substitute for the Redis pub/sub fan-out: publish
        // a typed All(key) envelope to the durable event bus. Consumers of the
        // "cache.invalidate" topic are the local subscriber set. Cross-process
        // fan-out still needs a real broker and is reported as deferred.
        CacheInvalidateEvent envelope{};
        envelope.set_kind("All");
        envelope.set_key(key);
        const std::string event_id = m_context->events().publish(
            "cache.invalidate",
            serde::Ser::encode_json(envelope)
        );

        // Upstream returns InternalServerError when result == 0, i.e. the
        // message reached no channels. Our "reached a channel" signal is the
        // envelope being durably recorded and findable by its event id.
        if (event_id.empty() || !m_context->events().find_by_event_id(event_id).has_value()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::INTERNAL_ERROR, "Failed to invalidate cache"));
            return;
        }

        m_context->metrics().record_flow("CacheInvalidate");
        m_context->metrics().counter("CACHE_INVALIDATED").increment();

        // Upstream ApplicationResponse::StatusOk has no body. Use the in-repo
        // empty-body StatusOk pattern (cf. connector_onboarding) instead of a
        // synthetic status/removed document.
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

    explicit CacheRouteHandlers(app::context::AppContext& context) noexcept
        : m_context{&context}
    {
    }

private:
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

} // namespace app::handlers::cache

template<>
struct serde::Serializable<app::handlers::cache::CacheInvalidateEvent>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"kind", &app::handlers::cache::CacheInvalidateEvent::get_kind, &app::handlers::cache::CacheInvalidateEvent::set_kind, serde::FieldOptions{}>{},
            serde::FieldDesc<"key", &app::handlers::cache::CacheInvalidateEvent::get_key, &app::handlers::cache::CacheInvalidateEvent::set_key, serde::FieldOptions{}>{},
        };
    }
    static constexpr std::string_view table_name() { return "cache_invalidate_event"; }
};
