export module app_handlers_hypersense:input;

import std;
import interfaces;
import app_context;
import app_dto;
import app_auth;
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
    explicit HypersenseInputSupport(app::context::AppContext& context) noexcept :
        m_context{context}
    {
    }

    /** @brief app::auth::TokenParser — case-insensitive Bearer scheme, empty when absent. */
    [[nodiscard]] std::string bearer_token(interfaces::io::IRequest& req) const
    {
        const auto bearer = app::auth::TokenParser{}(req);
        return bearer.is_empty() ? std::string{} : std::string{bearer.value()};
    }

    /** @brief Locally issued dashboard tokens carry the merchant id as a header. */
    [[nodiscard]] std::string merchant_id(interfaces::io::IRequest& req) const
    {
        return app::http::merchant_id_from_header{}(req);
    }

private:
    app::context::AppContext& m_context;
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
    app::dto::external_service_auth::ExternalVerifyTokenRequest request{};
};

struct SignoutTokenInput
{
    app::dto::external_service_auth::ExternalSignoutTokenRequest request{};
};

} // namespace app::handlers::hypersense
