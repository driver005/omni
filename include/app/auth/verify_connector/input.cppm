export module app_auth:verify_connector_input;

import std;
import interfaces;
import core_error;
import app_context;
import app_dto;
import app_context_core;
import app_http;

export namespace app::handlers::verify_connector {

/** @brief Auth decision plus the merchant identity resolved from the request. */
struct VerifyConnectorAuthorized
{
    std::optional<core::error::AppError> error{};
    std::string merchant_id{};
};

/**
 * @brief Parse/auth support for the verify-connector handler.
 *
 * Kept free of business logic: it only turns request data into the plain Input
 * struct consumed by the domain runtime. Error builders used before the runtime
 * call live here so both the handler and the runtime agree on wire kind names.
 */
class VerifyConnectorInputSupport
{
    using Error = core::error::AppError;
    using Kind = core::error::AppErrorKind;

public:
    explicit VerifyConnectorInputSupport(::app::context::AppContext& context) noexcept :
        m_context{context}
    {
    }

    // Upstream: JWTAuth { permission: MerchantConnectorWrite, allow_connected: true,
    // allow_platform: false }. Resolve the declarative rule from the shared registry.
    // (For JWT the authorizer ignores allow_platform, so the registry row is sufficient.)
    [[nodiscard]] VerifyConnectorAuthorized authorize(interfaces::io::IRequest& req) const
    {
        const auto rule = m_context.auth_rules().rule_for("payment_connector_verify");
        const auto auth_ctx = ::app::auth::AuthContextBuilder{}(req);
        const auto auth = m_context.authorizer().authorize_route(auth_ctx, rule);

        VerifyConnectorAuthorized out{};
        if (auth) {
            out.error = *auth;
        }

        // Upstream takes merchant/profile from the auth extractor (routes/verify_connector.rs:24-29
        // passes auth.profile.id as profile_id). No URL path segment and no merchant-account
        // lookup: upstream authenticates via JWT and performs no DB write on this route.
        std::string merchant_id = auth_ctx.merchant_id();
        if (merchant_id.empty()) {
            merchant_id = ::app::http::merchant_id_from_header{}(req);
        }
        // const std::string profile_id{auth_ctx.profile_id()}; — core takes _profile_id but does
        // not use it (core/verify_connector.rs:22), so it is deliberately not carried.
        out.merchant_id = std::move(merchant_id);
        return out;
    }

    /** @brief InvalidRequestData -> 422 (api_error_response.rs:586-588). */
    [[nodiscard]] Error invalid_request_data_error(std::string message) const
    {
        auto error = core::error::AppErrorBuilder{}(Kind::VALIDATION_FAILED, std::move(message));
        error.set_kind_name("InvalidRequestData");
        return error;
    }

    /** @brief IncorrectConnectorNameGiven -> 404 (api_error_response.rs:532-534). */
    [[nodiscard]] Error incorrect_connector_name_given_error() const
    {
        auto error = core::error::AppErrorBuilder{}(
            Kind::NOT_FOUND,
            "The connector provided in the request is incorrect or not available"
        );
        error.set_kind_name("IncorrectConnectorNameGiven");
        return error;
    }

private:
    ::app::context::AppContext& m_context;
};

// ───────────────────────── Plain input ─────────────────────────
// Carries exactly what the old core call received.

struct PaymentConnectorVerifyInput
{
    ::app::dto::verify_connector::VerifyConnectorRequest request{};
    ::app::dto::verify_connector::Connector connector{};
    std::string connector_name{};
    std::string merchant_id{};
};

} // namespace app::handlers::verify_connector
