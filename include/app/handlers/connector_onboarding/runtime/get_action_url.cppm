export module app_handlers_connector_onboarding:runtime_get_action_url;

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

class ConnectorOnboardingGetActionUrlRuntime
{
public:
    explicit ConnectorOnboardingGetActionUrlRuntime(app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    // Upstream core::get_action_url (core/connector_onboarding.rs:20-58).
    [[nodiscard]] std::expected<GetActionUrlResult, core::error::AppError>
    get_action_url(const GetActionUrlInput& input) const
    {
        if (const auto guard = m_support.check_if_connector_exists(
                input.request.get_connector_id(),
                input.merchant_id,
                std::nullopt
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
        auto action_url = m_support.get_action_url_from_paypal(
            *tracking,
            input.request.get_return_url(),
            input.merchant_id
        );
        if (!action_url) {
            return std::unexpected{std::move(action_url.error())};
        }

        // ActionUrlResponse::PayPal externally-tagged lowercase envelope (api_models:12-16).
        app::dto::connector_onboarding::PayPalActionUrlResponse inner{};
        inner.set_action_url(*action_url);
        serde::Value::Object envelope{};
        envelope["paypal"] = ResponseDocument::from(inner);

        m_context.metrics().record_flow("GetActionUrl");
        m_context.metrics().counter("connector_onboarding.get_action_url").increment();
        return GetActionUrlResult{serde::Value{std::move(envelope)}};
    }

private:
    app::context::AppContext& m_context;
    ConnectorOnboardingRuntimeSupport m_support;
};

} // namespace app::handlers::connector_onboarding
