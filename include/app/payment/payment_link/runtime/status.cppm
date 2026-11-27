export module app_payment:payment_link_runtime_status;
import core_behavior;

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

class PaymentLinkStatusRuntime
{
public:
    explicit PaymentLinkStatusRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    // payment_link_status (routes:217-249) -> get_payment_link_status
    // (core:892-1073). Always renders an IntentStatus page with redirect=true.
    [[nodiscard]] core::behavior::Outcome<serde::Value>
    status(const core::behavior::CallContext& ctx) const
    {
        const auto input = PaymentLinkStatusData{ctx};
        m_context.logger().record_flow("payment_link.status");
        m_context.logger().counter("payment_link.status").increment();

        auto page = m_support.build_status_page_data(input.merchant_id, input.payment_id);
        if (!page) {
            return std::unexpected{page.error()};
        }
        return serde::Value{m_support.status_page_data_document(
            std::get<0>(*page),
            std::get<1>(*page),
            std::get<2>(*page)
        )};
    }

private:
    ::app::context::AppContext& m_context;
    PaymentLinkRuntimeSupport m_support;
};

} // namespace app::handlers::payment_link
