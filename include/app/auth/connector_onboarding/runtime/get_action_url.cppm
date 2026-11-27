export module app_auth:connector_onboarding_runtime_get_action_url;

import std;
import core_behavior;
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

import :call_context;
import :connector_onboarding_input;
import :connector_onboarding_result;
import :connector_onboarding_runtime_common;

export namespace app::handlers::connector_onboarding {

class ConnectorOnboardingGetActionUrlRuntime
{
public:
    explicit ConnectorOnboardingGetActionUrlRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    // Upstream core::get_action_url (core/connector_onboarding.rs:20-58).
    [[nodiscard]] core::behavior::Outcome<serde::Value> get_action_url(
        const ::app::dto::connector_onboarding::ActionUrlRequest& request,
        const core::behavior::CallContext& ctx
    ) const
    {
        if (const auto guard = m_support.check_if_connector_exists(
                request.get_connector_id(),
                detail::effective_merchant_id(ctx),
                std::nullopt
            )) {
            return std::unexpected{*guard};
        }
        const auto conf = m_support.connector_onboarding_conf();
        const auto enabled = m_support.is_enabled(request.get_connector(), conf);
        auto tracking = m_support.get_tracking_id_from_configs(
            request.get_connector_id(),
            request.get_connector()
        );
        if (!tracking) {
            return std::unexpected{std::move(tracking.error())};
        }
        if (!enabled.has_value() || !*enabled || !m_support.is_paypal(request.get_connector())) {
            return std::unexpected{m_support.flow_not_supported(request.get_connector())};
        }
        auto action_url = m_support.get_action_url_from_paypal(
            *tracking,
            request.get_return_url(),
            detail::effective_merchant_id(ctx)
        );
        if (!action_url) {
            return std::unexpected{std::move(action_url.error())};
        }

        // ActionUrlResponse::PayPal externally-tagged lowercase envelope (api_models:12-16).
        ::app::dto::connector_onboarding::PayPalActionUrlResponse inner{};
        inner.set_action_url(*action_url);
        serde::Value::Object envelope{};
        envelope["paypal"] = ResponseDocument::from(inner);

        m_context.logger().record_flow("GetActionUrl");
        m_context.logger().counter("connector_onboarding.get_action_url").increment();
        return serde::Value{std::move(envelope)};
    }

private:
    ::app::context::AppContext& m_context;
    ConnectorOnboardingRuntimeSupport m_support;
};

} // namespace app::handlers::connector_onboarding
