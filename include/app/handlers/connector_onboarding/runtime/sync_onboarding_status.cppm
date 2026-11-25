export module app_handlers_connector_onboarding:runtime_sync_onboarding_status;

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
import app_auth;
import app_http;
import app_crypto;
import app_metrics;
import app_store;
import app_connector;
import app_events;
import app_workflow;
import app_keystore;

import :input;
import :result;
import :runtime_common;

export namespace app::handlers::connector_onboarding {

class ConnectorOnboardingSyncOnboardingStatusRuntime
{
public:
    explicit ConnectorOnboardingSyncOnboardingStatusRuntime(
        app::context::AppContext& context
    ) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    // Upstream core::sync_onboarding_status (core/connector_onboarding.rs:60-120).
    [[nodiscard]] std::expected<SyncOnboardingStatusResult, core::error::AppError>
    sync_onboarding_status(const SyncOnboardingStatusInput& input) const
    {
        // The profile used for the ownership check is the request body's profile_id
        // (utils/connector_onboarding.rs:71), not a query/auth fallback.
        if (const auto guard = m_support.check_if_connector_exists(
                input.request.get_connector_id(),
                input.merchant_id,
                std::optional<std::string>{input.request.get_profile_id()}
            )) {
            return std::unexpected{*guard};
        }
        const auto conf = m_support.connector_onboarding_conf();
        const auto enabled = m_support.is_enabled(input.request.get_connector(), conf);
        auto tracking = m_support.get_tracking_id_from_configs(
            input.request.get_connector_id(),
            input.request.get_connector()
        );
        if (!tracking) {
            return std::unexpected{std::move(tracking.error())};
        }
        if (!enabled.has_value() || !*enabled ||
            !m_support.is_paypal(input.request.get_connector())) {
            return std::unexpected{m_support.flow_not_supported(input.request.get_connector())};
        }

        auto status = m_support.sync_merchant_onboarding_status(*tracking, input.merchant_id);
        if (!status) {
            return std::unexpected{std::move(status.error())};
        }

        serde::Value document{};
        if (status->success) {
            // Upstream core:87-110: on Success build SignatureKey auth from the config
            // client_secret/client_id + PayPal payer_id, call update_mca, and return
            // OnboardingStatus::PayPal::ConnectorIntegrated(MerchantConnectorResponse).
            const ConnectorAuth
                auth_details{"SignatureKey", conf.client_secret, conf.client_id, status->payer_id};
            auto mca = m_support.update_mca(
                input.merchant_id,
                input.request.get_profile_id(),
                input.request.get_connector_id(),
                auth_details
            );
            if (!mca) {
                return std::unexpected{std::move(mca.error())};
            }
            serde::Value::Object integrated{};
            integrated["connector_integrated"] = ResponseDocument::from(*mca);
            serde::Value::Object envelope{};
            envelope["paypal"] = serde::Value{std::move(integrated)};
            document = serde::Value{std::move(envelope)};
        } else {
            // Unit variants are externally tagged as lowercase snake_case strings.
            serde::Value::Object envelope{};
            envelope["paypal"] = serde::Value{std::string{status->wire}};
            document = serde::Value{std::move(envelope)};
        }

        m_context.metrics().record_flow("SyncOnboardingStatus");
        m_context.metrics().counter("connector_onboarding.sync_onboarding_status").increment();
        return SyncOnboardingStatusResult{std::move(document)};
    }

private:
    app::context::AppContext& m_context;
    ConnectorOnboardingRuntimeSupport m_support;
};

} // namespace app::handlers::connector_onboarding
