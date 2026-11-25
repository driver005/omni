export module app_handlers_payment_methods:runtime_render_pm_collect_link;

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

export namespace app::handlers::payment_methods {

class RenderPmCollectLinkRuntime
{
public:
    explicit RenderPmCollectLinkRuntime(app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] std::expected<RenderPmCollectLinkResult, core::error::AppError>
    render_pm_collect_link(const RenderPmCollectLinkInput& input) const
    {
        m_context.metrics().record_flow("render_pm_collect_link");
        m_context.metrics().counter("PAYMENT_METHOD_RENDER_PM_COLLECT_LINK").increment();
        app::model::payment_methods::PaymentMethodCollectLinkRenderRequest response{};
        app::model::payment_methods::MerchantIdValue response_merchant{};
        response_merchant.set_value(input.merchant_id);
        response.set_merchant_id(std::move(response_merchant));
        response.set_pm_collect_link_id(input.collect_id);
        return RenderPmCollectLinkResult{std::move(response)};
    }

private:
    app::context::AppContext& m_context;
    PaymentMethodsRuntimeSupport m_support;
};

} // namespace app::handlers::payment_methods
