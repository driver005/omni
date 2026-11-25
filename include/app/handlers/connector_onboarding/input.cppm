export module app_handlers_connector_onboarding:input;

import std;
import interfaces;
import core_error;
import app_context;
import app_dto;
import app_auth;
import app_http;

export namespace app::handlers::connector_onboarding {

/**
 * @brief Parse/auth support for the connector_onboarding handlers.
 *
 * Kept free of business logic: it only turns request data into the plain Input
 * structs consumed by the domain runtime.
 */
class ConnectorOnboardingInputSupport
{
public:
    explicit ConnectorOnboardingInputSupport(app::context::AppContext& context) noexcept :
        m_context{context}
    {
    }

    struct AuthorizedRequest
    {
        std::optional<core::error::AppError> error{};
        app::auth::AuthContext context{};
    };

    [[nodiscard]] AuthorizedRequest
    authorize(interfaces::io::IRequest& req, std::string_view operation) const
    {
        const auto rule = m_context.auth_rules().rule_for(operation);
        const auto auth = app::auth::AuthContextBuilder{}(req);
        return AuthorizedRequest{m_context.authorizer().authorize_route(auth, rule), auth};
    }

    [[nodiscard]] std::string
    onboarding_merchant_id(interfaces::io::IRequest& req, const app::auth::AuthContext& auth) const
    {
        const auto connected = auth.claim(app::auth::kClaimConnectedMerchantId);
        if (connected.has_value() && !connected->empty()) {
            return std::string{*connected};
        }
        return app::http::merchant_id_from_header{}(req);
    }

    // Upstream id_type::is_valid_id_character (common_utils/src/id_type.rs:74-77).
    [[nodiscard]] bool valid_id(std::string_view id) const
    {
        return !id.empty() && std::ranges::all_of(
                                  id,
                                  [](char ch)
                                  {
                                      return std::isalnum(static_cast<unsigned char>(ch)) ||
                                             ch == '_' || ch == '-';
                                  }
                              );
    }

    // The runtime returns AppError (no HTTP types). Errors whose wire status differs
    // from the status derived from their AppErrorKind carry the upstream variant name
    // in kind_name; translate that name back to the status override here so the handler
    // can write status_override.value_or(error.get_status()).
    [[nodiscard]] std::optional<interfaces::io::types::Status>
    status_override_for(std::string_view kind_name) const noexcept
    {
        using interfaces::io::types::Status;
        if (kind_name == "NotImplemented") {
            return Status::NOT_IMPLEMENTED;
        }
        if (kind_name == "FlowNotSupported") {
            return Status::BAD_REQUEST;
        }
        if (kind_name == "PreconditionFailed") {
            return Status::BAD_REQUEST;
        }
        return std::nullopt;
    }

private:
    app::context::AppContext& m_context;
};

// ───────────────────────── Plain inputs ─────────────────────────
// Each struct carries exactly what the old core call received.

struct GetActionUrlInput
{
    app::dto::connector_onboarding::ActionUrlRequest request{};
    std::string merchant_id{};
};

struct SyncOnboardingStatusInput
{
    app::dto::connector_onboarding::OnboardingSyncRequest request{};
    std::string merchant_id{};
};

struct ResetTrackingIdInput
{
    app::dto::connector_onboarding::ResetTrackingIdRequest request{};
    std::string merchant_id{};
};

} // namespace app::handlers::connector_onboarding
