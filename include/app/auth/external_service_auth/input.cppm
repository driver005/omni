export module app_auth:external_service_auth_input;

import std;
import interfaces;
import core_error;
import core_behavior;
import app_context;
import app_dto;
import app_context_core;

export namespace app::handlers::external_service_auth {

/**
 * @brief Plain input for the validate_token route.
 *
 * Upstream routes/external_service_auth.rs:11-27 passes only the deserialized
 * request body into the core; no path or query data is used.
 */
struct ValidateTokenInput
{
    ::app::dto::external_service_auth::ValidateTokenRequest request{};
};

/**
 * @brief Parse/auth support for the external_service_auth handler.
 *
 * Kept free of business logic: it only turns request data into the plain Input
 * structs consumed by the domain runtime.
 */
class ExternalServiceAuthInputSupport
{
public:
    explicit ExternalServiceAuthInputSupport(::app::context::AppContext& context) noexcept :
        m_context{context}
    {
    }

    // routes/external_service_auth.rs:23-24 — authentication::NoAuth with
    // LockAction::NotApplicable. "validate_token" resolves to NO_AUTH
    // (auth/registry.cppm:456); authorize_route is an exact no-op
    // (auth/authorizer.cppm:59-62). No lock is acquired, matching NotApplicable.
    [[nodiscard]] std::optional<core::error::AppError>
    authorize_no_auth(interfaces::io::IRequest& req) const
    {
        const auto rule = m_context.auth_rules().rule_for("validate_token");
        return m_context.authorizer().authorize_route(::app::auth::AuthContextBuilder{}(req), rule);
    }

private:
    ::app::context::AppContext& m_context;
};

/** @brief Domain policy for the NO_AUTH validate-token route. */
class ExternalServiceAuthPolicy
{
public:
    explicit ExternalServiceAuthPolicy(::app::context::AppContext& context) noexcept :
        m_support{context}
    {
    }

    [[nodiscard]] std::optional<core::error::AppError> authorize(
        interfaces::io::IRequest& req,
        std::string_view operation,
        const core::behavior::AuthSpec& spec
    ) const
    {
        (void)operation;
        (void)spec;
        return m_support.authorize_no_auth(req);
    }

    [[nodiscard]] core::behavior::Scope scope(interfaces::io::IRequest& req) const
    {
        (void)req;
        return {};
    }

    [[nodiscard]] std::optional<core::error::AppError>
    scope_error(const core::behavior::Scope& scope) const
    {
        (void)scope;
        return std::nullopt;
    }

private:
    ExternalServiceAuthInputSupport m_support;
};

} // namespace app::handlers::external_service_auth
