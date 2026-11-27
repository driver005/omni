export module app_auth:connector_onboarding_runtime_reset_tracking_id;

import std;
import interfaces;
import core_router;
import core_serde;
import utils_openapi;
import app_model;
import app_dto;
import app_context;
import core_cache;
import core_error;
import app_context_core;
import app_http;
import app_crypto;
import app_metrics;
import app_context_core;
import app_connector;
import app_events;
import app_workflow;
import app_keystore;

import :connector_onboarding_input;
import :connector_onboarding_result;
import :connector_onboarding_runtime_common;

export namespace app::handlers::connector_onboarding {

class ConnectorOnboardingResetTrackingIdRuntime
{
public:
    explicit ConnectorOnboardingResetTrackingIdRuntime(::app::context::AppContext& context) noexcept
        :
        m_context{context},
        m_support{context}
    {
    }

    // Upstream core::reset_tracking_id (core/connector_onboarding.rs:122-138):
    // no is_enabled gate and no connector-type match; just upsert the tracking id.
    [[nodiscard]] std::expected<serde::Value, core::error::AppError>
    reset_tracking_id(const ResetTrackingIdInput& input) const
    {
        if (const auto guard = m_support.check_if_connector_exists(
                input.request.get_connector_id(),
                input.merchant_id,
                std::nullopt
            )) {
            return std::unexpected{*guard};
        }
        if (const auto guard = m_support.set_tracking_id_in_configs(
                input.request.get_connector_id(),
                input.request.get_connector()
            )) {
            return std::unexpected{*guard};
        }

        m_context.metrics().record_flow("ResetTrackingId");
        m_context.metrics().counter("connector_onboarding.reset_tracking_id").increment();
        // Upstream returns ApplicationResponse::StatusOk (no body).
        return ResponseDocument::empty();
    }

private:
    ::app::context::AppContext& m_context;
    ConnectorOnboardingRuntimeSupport m_support;
};

} // namespace app::handlers::connector_onboarding
