export module app_auth:connector_onboarding_runtime;

import std;
import core_error;
import app_context;

import :connector_onboarding_input;
import :connector_onboarding_result;
import :connector_onboarding_runtime_common;
import :connector_onboarding_runtime_get_action_url;
import :connector_onboarding_runtime_sync_onboarding_status;
import :connector_onboarding_runtime_reset_tracking_id;

export namespace app::handlers::connector_onboarding {

class ConnectorOnboardingRuntime
{
public:
    explicit ConnectorOnboardingRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_get_action_url{context},
        m_sync_onboarding_status{context},
        m_reset_tracking_id{context}
    {
    }

    [[nodiscard]] std::expected<GetActionUrlResult, core::error::AppError>
    get_action_url(const GetActionUrlInput& input) const
    {
        return m_get_action_url.get_action_url(input);
    }

    [[nodiscard]] std::expected<SyncOnboardingStatusResult, core::error::AppError>
    sync_onboarding_status(const SyncOnboardingStatusInput& input) const
    {
        return m_sync_onboarding_status.sync_onboarding_status(input);
    }

    [[nodiscard]] std::expected<ResetTrackingIdResult, core::error::AppError>
    reset_tracking_id(const ResetTrackingIdInput& input) const
    {
        return m_reset_tracking_id.reset_tracking_id(input);
    }

private:
    ::app::context::AppContext& m_context;
    ConnectorOnboardingGetActionUrlRuntime m_get_action_url;
    ConnectorOnboardingSyncOnboardingStatusRuntime m_sync_onboarding_status;
    ConnectorOnboardingResetTrackingIdRuntime m_reset_tracking_id;
};

} // namespace app::handlers::connector_onboarding
