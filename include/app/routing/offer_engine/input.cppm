export module app_routing:offer_engine_input;

import std;
import interfaces;
import core_error;
import core_behavior;
import app_context;
import app_auth;

import :policy;

export namespace app::handlers::offer_engine {

/**
 * @brief Parse/auth/path/query support for the offer_engine handlers.
 *
 * Kept free of business logic: it only resolves the route auth rule and turns
 * request data into the plain Input structs consumed by the domain runtime.
 */
class OfferEngineInputSupport
{
public:
    explicit OfferEngineInputSupport(::app::context::AppContext& context) noexcept :
        m_context{context}
    {
    }

    /**
     * @brief Upstream `&auth::AdminApiAuth` / `HeaderAuth(ApiKeyAuth { ... })`
     * (routes/offer_engine.rs:23, :46-49). The registry entry carries the equivalent
     * rule and `authorize_route` enforces it, exactly as in the original handler.
     */
    [[nodiscard]] std::optional<core::error::AppError>
    authorize(const ::app::auth::AuthContext& auth, std::string_view operation) const
    {
        const auto rule = m_context.auth_rules().rule_for(operation);
        return m_context.authorizer().authorize_route(auth, rule);
    }

private:
    ::app::context::AppContext& m_context;
};

/** @brief Domain policy: registry rule for the operation, evaluated on request auth. */
class OfferEnginePolicy
{
public:
    explicit OfferEnginePolicy(OfferEngineInputSupport& support) noexcept :
        m_support{support}
    {
    }

    [[nodiscard]] std::optional<core::error::AppError> authorize(
        interfaces::io::IRequest& req,
        std::string_view operation,
        const ::core::behavior::AuthSpec&
    ) const
    {
        return m_support.authorize(::app::auth::AuthContextBuilder{}(req), operation);
    }

    [[nodiscard]] ::core::behavior::Scope scope(interfaces::io::IRequest& req) const
    {
        return ::app::routing::policy::scope_of(req);
    }

    [[nodiscard]] ::core::behavior::CallContext context(interfaces::io::IRequest& req) const
    {
        return ::app::routing::policy::context_of(req);
    }

    [[nodiscard]] std::optional<core::error::AppError>
    scope_error(const ::core::behavior::Scope& scope) const
    {
        return ::app::routing::policy::scope_error(scope);
    }

private:
    OfferEngineInputSupport& m_support;
};

using OfferEngineRequestSupport = ::core::behavior::RequestSupport<OfferEnginePolicy>;

} // namespace app::handlers::offer_engine
