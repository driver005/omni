export module app_handlers_cache:input;

import std;
import interfaces;
import core_error;
import app_context;
import app_auth;
import app_http;

export namespace app::handlers::cache {

/**
 * @brief Parse/auth/path support for the cache handler.
 *
 * Kept free of business logic: it only turns request data into the plain
 * CacheInvalidateInput consumed by the domain runtime.
 */
class CacheInputSupport
{
public:
    explicit CacheInputSupport(app::context::AppContext& context) noexcept :
        m_context{context}
    {
    }

    // Upstream wraps this route in server_wrap(Flow::CacheInvalidate,
    // AdminApiAuth, LockAction::NotApplicable) (routes/cache.rs:20-28).
    // AdminApiAuth is an admin-api-key bearer check. The registry row
    // "invalidate" is the declarative local form: PLATFORM_ADMIN + ADMIN +
    // bearer requirement. NotApplicable means no lock is acquired here.
    [[nodiscard]] std::optional<core::error::AppError>
    authorize(interfaces::io::IRequest& req) const
    {
        const auto rule = m_context.auth_rules().rule_for("invalidate");
        return m_context.authorizer().authorize_route(app::auth::AuthContextBuilder{}(req), rule);
    }

    // Upstream takes the key from web::Path<String> (/cache/invalidate/:key);
    // no body is read (routes/cache.rs:14,18). Read the matched path segment.
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

private:
    app::context::AppContext& m_context;
};

struct CacheInvalidateInput
{
    std::optional<core::error::AppError> auth_error{};
    std::string key{};
};

} // namespace app::handlers::cache
