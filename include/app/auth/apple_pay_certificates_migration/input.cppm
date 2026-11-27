export module app_auth:apple_pay_certificates_migration_input;

import std;
import interfaces;
import core_error;
import core_behavior;
import app_context;
import app_dto;
import app_context_core;

export namespace app::handlers::apple_pay_certificates_migration {

/** @brief Parse/auth support for the migration handler.
 *
 *  Kept free of business logic: it only reproduces the route-layer authorization
 *  performed by the original handler before the request body is read.
 */
class ApplePayCertificatesMigrationInputSupport
{
public:
    explicit ApplePayCertificatesMigrationInputSupport(::app::context::AppContext& context) noexcept
        :
        m_context{context}
    {
    }

    // AdminApiAuth is modeled by the PLATFORM_ADMIN/ADMIN rule registered for
    // this operation (app/auth/registry.cppm:38); upstream
    // routes/apple_pay_certificates_migration.rs:26. Returns the denial error,
    // so the handler keeps the single response-writing path for every route.
    [[nodiscard]] std::optional<core::error::AppError>
    authorize(interfaces::io::IRequest& req) const
    {
        const auto rule = m_context.auth_rules().rule_for("apple_pay_certificates_migration");
        return m_context.authorizer().authorize_route(::app::auth::AuthContextBuilder{}(req), rule);
    }

private:
    ::app::context::AppContext& m_context;
};

/**
 * @brief Domain policy for the admin-only certificate migration route.
 *
 * The declarative rule stays in the shared registry; the policy only adapts it
 * to the generic RequestSupport contract. Permission/scope flags are resolved
 * from the rule for this operation, not re-derived from the request.
 */
class ApplePayCertificatesMigrationPolicy
{
public:
    explicit ApplePayCertificatesMigrationPolicy(::app::context::AppContext& context) noexcept :
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
        return m_support.authorize(req);
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
    ApplePayCertificatesMigrationInputSupport m_support;
};

// Plain input — carries exactly what the old core call received. The core only
// consumed the deserialized request; the route-level auth was handled above.
struct ApplePayCertificatesMigrationInput
{
    ::app::dto::apple_pay_certificates_migration::ApplePayCertificatesMigrationRequest request{};
};

} // namespace app::handlers::apple_pay_certificates_migration
