export module app_payment:payment_methods_runtime_initiate_pm_collect_link_flow;

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

import :payment_methods_input;
import :payment_methods_result;
import :payment_methods_runtime_common;

export namespace app::handlers::payment_methods {

class InitiatePmCollectLinkFlowRuntime
{
public:
    explicit InitiatePmCollectLinkFlowRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] std::expected<InitiatePmCollectLinkFlowResult, core::error::AppError>
    initiate_pm_collect_link_flow(const InitiatePmCollectLinkFlowInput& input) const
    {
        const std::string collect_id = m_context.ids().payment_id("pml_");
        const std::string token = m_support.issued_token(collect_id);
        m_support.remember_client_secret(collect_id, token);
        m_context.cache().put(
            "payment_methods.collect:" + collect_id,
            core::cache::to_cache_value(m_support.encrypt_data(token)),
            core::cache::Ttl{}.seconds(3'600)
        );
        const auto connector_response =
            m_support.call_connector(input.merchant_id, "create_link", collect_id, input.body);
        std::string link = connector_response.get_raw_json();
        if (link.empty()) {
            link = "/payment_methods/collect/" + input.merchant_id + "/" + collect_id +
                   "?token=" + token;
        }
        m_context.metrics().record_flow("initiate_pm_collect_link_flow");
        m_context.metrics().counter("PAYMENT_METHOD_INITIATE_PM_COLLECT_LINK_FLOW").increment();
        ::app::model::payment_methods::PaymentMethodCollectLinkResponse response{};
        response.set_pm_collect_link_id(collect_id);
        response.set_customer_id(input.request.get_customer_id());
        ::app::model::payment_methods::PrimitiveDateTimeValue expiry{};
        expiry.set_value(std::to_string(std::stoll(m_support.now_value()) + 3'600));
        response.set_expiry(std::move(expiry));
        response.set_link(link);
        if (input.request.get_return_url()) {
            response.set_return_url(*input.request.get_return_url());
        }
        return InitiatePmCollectLinkFlowResult{std::move(response)};
    }

private:
    ::app::context::AppContext& m_context;
    PaymentMethodsRuntimeSupport m_support;
};

} // namespace app::handlers::payment_methods
