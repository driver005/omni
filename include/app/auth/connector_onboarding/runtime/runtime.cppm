export module app_auth:connector_onboarding_runtime;

import std;
import core_behavior;
import core_error;
import core_serde;
import app_context;
import app_dto;

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

    [[nodiscard]] core::behavior::Outcome<serde::Value>
    get_action_url(
        const ::app::dto::connector_onboarding::ActionUrlRequest& request,
        const core::behavior::CallContext& ctx
    ) const
    {
        return m_get_action_url.get_action_url(request, ctx);
    }

    [[nodiscard]] core::behavior::Outcome<serde::Value> sync_onboarding_status(
        const ::app::dto::connector_onboarding::OnboardingSyncRequest& request,
        const core::behavior::CallContext& ctx
    ) const
    {
        return m_sync_onboarding_status.sync_onboarding_status(request, ctx);
    }

    [[nodiscard]] core::behavior::Outcome<serde::Value> reset_tracking_id(
        const ::app::dto::connector_onboarding::ResetTrackingIdRequest& request,
        const core::behavior::CallContext& ctx
    ) const
    {
        return m_reset_tracking_id.reset_tracking_id(request, ctx);
    }

private:
    ::app::context::AppContext& m_context;
    ConnectorOnboardingGetActionUrlRuntime m_get_action_url;
    ConnectorOnboardingSyncOnboardingStatusRuntime m_sync_onboarding_status;
    ConnectorOnboardingResetTrackingIdRuntime m_reset_tracking_id;
};

} // namespace app::handlers::connector_onboarding
