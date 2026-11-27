export module app_store:cache_input;

import std;
import interfaces;
import core_error;
import core_behavior;
import app_context;
import app_context_core;
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
    explicit CacheInputSupport(::app::context::AppContext& context) noexcept :
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
        return m_context.authorizer().authorize_route(::app::auth::AuthContextBuilder{}(req), rule);
    }

private:
    ::app::context::AppContext& m_context;
};

/**
 * @brief Domain policy consumed by core::behavior::RequestSupport.
 *
 * Cache invalidation has one declarative rule ("invalidate"); the cache key is
 * a generic path segment, so RequestSupport::path owns that extraction.
 */
class CachePolicy
{
public:
    explicit CachePolicy(CacheInputSupport& support) noexcept :
        m_support{support}
    {
    }

    [[nodiscard]] std::optional<core::error::AppError> authorize(
        interfaces::io::IRequest& req,
        std::string_view,
        const core::behavior::AuthSpec&
    ) const
    {
        return m_support.authorize(req);
    }

    [[nodiscard]] core::behavior::Scope scope(interfaces::io::IRequest&) const noexcept
    {
        return {};
    }

    [[nodiscard]] std::optional<core::error::AppError>
    scope_error(const core::behavior::Scope&) const noexcept
    {
        return std::nullopt;
    }

private:
    CacheInputSupport& m_support;
};

struct CacheInvalidateInput
{
    std::optional<core::error::AppError> auth_error{};
    std::string key{};
};

} // namespace app::handlers::cache
