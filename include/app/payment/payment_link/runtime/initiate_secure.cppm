export module app_payment:payment_link_runtime_initiate_secure;

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

import :payment_link_input;
import :payment_link_result;
import :payment_link_runtime_common;

export namespace app::handlers::payment_link {

class PaymentLinkInitiateSecureRuntime
{
public:
    explicit PaymentLinkInitiateSecureRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    // initiate_secure_payment_link (routes:100-133) ->
    // initiate_secure_payment_link_flow (core:362-470). Renders a GenericLinks
    // form (SecurePaymentLink) or the status page, after header validation.
    [[nodiscard]] std::expected<serde::Value, core::error::AppError>
    initiate_secure(const PaymentLinkInitiateSecureInput& input) const
    {
        m_context.metrics().record_flow("payment_link.initiate_secure");
        m_context.metrics().counter("payment_link.initiate_secure").increment();

        auto form = m_support.form_payment_link_data(input.merchant_id, input.payment_id);
        if (!form) {
            return std::unexpected{form.error()};
        }

        // validate_secure_payment_link_render_request (core:374-378,
        // validator:12-109): allowed_domains + secure_link + iframe + origin.
        if (const auto denied = m_support.validate_secure_render_request(
                input.payment_id,
                form->config,
                input.headers
            )) {
            return std::unexpected{*denied};
        }

        const std::string css = m_support.get_css_script(form->config);
        if (form->is_status) {
            // core:382-397: status page is a PaymentLinkStatus payload (not a
            // GenericLinks wrapper), even on the secure route.
            return serde::Value{
                m_support.status_page_data_document(form->status, css, form->status_inner)
            };
        }

        // core:398-468: SecurePaymentLinkDetails -> URL-encoded js script,
        // meta tags, sdk url, wrapped in GenericLinks with allowed_domains.
        const std::string inner = serde::Ser::encode_json(
            m_support.secure_payment_link_details_document(form->details, form->config)
        );
        const std::string js = "window.__PAYMENT_DETAILS = '" + m_support.url_encode(inner) + "';";
        serde::Value data = m_support.payment_form_data_document_fields(
            js,
            css,
            form->config.sdk_url,
            m_support.get_meta_tags_html(form->details)
        );
        return serde::Value{
            m_support.generic_links_document(form->config.allowed_domains, std::move(data), "en")
        };
    }

private:
    ::app::context::AppContext& m_context;
    PaymentLinkRuntimeSupport m_support;
};

} // namespace app::handlers::payment_link
