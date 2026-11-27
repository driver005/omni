export module app_auth:hypersense_input;

import std;
import interfaces;
import core_error;
import core_behavior;
import app_context;
import app_dto;
import app_context_core;
import app_http;

export namespace app::handlers::hypersense {

/**
 * @brief Parse/auth support for the hypersense handlers.
 *
 * Kept free of business logic: it only turns request data into the plain Input
 * structs consumed by the domain runtime. DashboardNoPermissionAuth
 * (routes/hypersense.rs:28-31) has no named permission; the bearer credential and
 * the merchant-id header are the only request facts the runtime needs.
 */
class HypersenseInputSupport
{
public:
    explicit HypersenseInputSupport(::app::context::AppContext& context) noexcept :
        m_context{context}
    {
    }

    /** @brief ::app::auth::TokenParser — case-insensitive Bearer scheme, empty when absent. */
    [[nodiscard]] std::string bearer_token(interfaces::io::IRequest& req) const
    {
        const auto bearer = ::app::auth::TokenParser{}(req);
        return bearer.is_empty() ? std::string{} : std::string{bearer.value()};
    }

private:
    ::app::context::AppContext& m_context;
};

/**
 * @brief Domain policy for the dashboard-no-permission hypersense routes.
 *
 * These routes apply no declarative authorization; the bearer credential and
 * merchant identity are request facts exposed through RequestSupport::scope.
 */
class HypersensePolicy
{
public:
    [[nodiscard]] std::optional<core::error::AppError> authorize(
        interfaces::io::IRequest& req,
        std::string_view operation,
        const core::behavior::AuthSpec& spec
    ) const
    {
        (void)req;
        (void)operation;
        (void)spec;
        return std::nullopt;
    }

    [[nodiscard]] core::behavior::Scope scope(interfaces::io::IRequest& req) const
    {
        core::behavior::Scope out{};
        out.merchant_id = ::app::http::merchant_id_from_header{}(req);
        return out;
    }

    [[nodiscard]] std::optional<core::error::AppError>
    scope_error(const core::behavior::Scope& scope) const
    {
        (void)scope;
        return std::nullopt;
    }
};

// ───────────────────────── Plain inputs ─────────────────────────
// Each struct carries exactly what the old handler passed into its core logic.

struct GetTokenInput
{
    std::string bearer_token{};
    std::string merchant_id_from_header{};
};

struct VerifyTokenInput
{
    ::app::dto::external_service_auth::ExternalVerifyTokenRequest request{};
};

struct SignoutTokenInput
{
    ::app::dto::external_service_auth::ExternalSignoutTokenRequest request{};
};

} // namespace app::handlers::hypersense
