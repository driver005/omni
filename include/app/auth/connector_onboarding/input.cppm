export module app_auth:connector_onboarding_input;

import std;
import interfaces;
import core_error;
import core_behavior;
import app_context;
import app_dto;
import app_context_core;
import app_http;

export namespace app::handlers::connector_onboarding {

/**
 * @brief Wire-level parsing helpers for the connector_onboarding handlers.
 *
 * Auth and scope resolution live in @ref ConnectorOnboardingPolicy; this class
 * only keeps validation and error-status translation used by the route layer.
 */
class ConnectorOnboardingInputSupport
{
public:
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
};

/**
 * @brief Domain policy: declarative registry rule plus the connected-merchant
 * identity the onboarding runtimes expect as `merchant_id`.
 */
class ConnectorOnboardingPolicy
{
public:
    explicit ConnectorOnboardingPolicy(::app::context::AppContext& context) noexcept :
        m_context{context}
    {
    }

    [[nodiscard]] std::optional<core::error::AppError> authorize(
        interfaces::io::IRequest& req,
        std::string_view operation,
        const core::behavior::AuthSpec& spec
    ) const
    {
        (void)spec;
        const auto rule = m_context.auth_rules().rule_for(operation);
        return m_context.authorizer().authorize_route(::app::auth::AuthContextBuilder{}(req), rule);
    }

    [[nodiscard]] core::behavior::Scope scope(interfaces::io::IRequest& req) const
    {
        core::behavior::Scope out{};
        const auto auth = ::app::auth::AuthContextBuilder{}(req);
        const auto connected = auth.claim(::app::auth::kClaimConnectedMerchantId);
        if (connected.has_value() && !connected->empty()) {
            out.connected_merchant_id = *connected;
        }
        out.merchant_id = ::app::http::merchant_id_from_header{}(req);
        return out;
    }

    [[nodiscard]] std::optional<core::error::AppError>
    scope_error(const core::behavior::Scope& scope) const
    {
        (void)scope;
        return std::nullopt;
    }

private:
    ::app::context::AppContext& m_context;
};

// ───────────────────────── Plain inputs ─────────────────────────
// Each struct carries exactly what the old core call received.

struct GetActionUrlInput
{
    ::app::dto::connector_onboarding::ActionUrlRequest request{};
    std::string merchant_id{};
};

struct SyncOnboardingStatusInput
{
    ::app::dto::connector_onboarding::OnboardingSyncRequest request{};
    std::string merchant_id{};
};

struct ResetTrackingIdInput
{
    ::app::dto::connector_onboarding::ResetTrackingIdRequest request{};
    std::string merchant_id{};
};

} // namespace app::handlers::connector_onboarding
