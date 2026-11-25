export module app_handlers_payment_link:runtime_initiate;

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

export namespace app::handlers::payment_link {

class PaymentLinkInitiateRuntime
{
public:
    explicit PaymentLinkInitiateRuntime(app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    // initiate_payment_link (routes:66-98) -> initiate_payment_link_flow
    // (core:472-522). Renders either the payment form data or the status page.
    [[nodiscard]] std::expected<PaymentLinkInitiateResult, core::error::AppError>
    initiate(const PaymentLinkInitiateInput& input) const
    {
        m_context.metrics().record_flow("payment_link.initiate");
        m_context.metrics().counter("payment_link.initiate").increment();

        auto form = m_support.form_payment_link_data(input.merchant_id, input.payment_id);
        if (!form) {
            return std::unexpected{form.error()};
        }

        const std::string css = m_support.get_css_script(form->config);
        if (form->is_status) {
            return PaymentLinkInitiateResult{
                m_support.status_page_data_document(form->status, css, form->status_inner)
            };
        }
        return PaymentLinkInitiateResult{
            m_support.payment_form_data_document(form->details, css, form->config.sdk_url)
        };
    }

private:
    app::context::AppContext& m_context;
    PaymentLinkRuntimeSupport m_support;
};

} // namespace app::handlers::payment_link
