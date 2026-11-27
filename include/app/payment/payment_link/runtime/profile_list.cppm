export module app_payment:payment_link_runtime_profile_list;

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

class PaymentLinkProfileListRuntime
{
public:
    explicit PaymentLinkProfileListRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] std::
        expected<::app::model::payments::PaymentLinkListResponse, core::error::AppError>
        profile_list(const PaymentLinkListInput& input) const
    {
        m_context.metrics().record_flow("payment_link.profile_list");
        m_context.metrics().counter("payment_link.profile_list").increment();

        auto response = m_support.build_list_response(input);
        if (!response) {
            return std::unexpected{response.error()};
        }
        return ::app::model::payments::PaymentLinkListResponse{std::move(*response)};
    }

private:
    ::app::context::AppContext& m_context;
    PaymentLinkRuntimeSupport m_support;
};

} // namespace app::handlers::payment_link
