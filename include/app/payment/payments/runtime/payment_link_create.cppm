export module app_payment:payments_runtime_payment_link_create;
import core_behavior;

import std;
import interfaces;
import core_serde;
import core_error;
import app_context;
import app_model;
import app_dto;
import app_store;
import app_connector;
import app_crypto;
import core_cache;
import app_http;

import :payments_input;
import :payments_result;
import :payments_runtime_common;

export namespace app::handlers::payments {

class PaymentLinkCreateRuntime
{
public:
    explicit PaymentLinkCreateRuntime(
        ::app::context::AppContext& context,
        PaymentsRuntimeSupport& support
    ) noexcept :
        m_context{context},
        m_support{support}
    {
    }

    [[nodiscard]] core::behavior::Outcome<::app::model::payments::PaymentLinkResponse>
    payment_link_create(
        const ::app::model::payments::PaymentsRequest& dto,
        const core::behavior::CallContext& ctx
    ) const
    {
        const PaymentsRequestData input = m_support.request_data(dto, ctx);
        const std::string new_payment_id = m_context.ids().payment_id();
        ::app::store::payments::PaymentIntentRecord record{};
        record.set_payment_id(new_payment_id);
        record.set_merchant_id(input.merchant_id);
        record.set_status("requires_payment_method");
        m_context.repository().save_payment(std::move(record));
        m_context.logger().counter("payments.created").increment();
        ::app::model::payments::PaymentLinkResponse response{};
        response.set_link(input.body);
        response.set_payment_link_id(new_payment_id);
        return ::app::model::payments::PaymentLinkResponse{response};
    }

private:
    ::app::context::AppContext& m_context;
    PaymentsRuntimeSupport& m_support;
};

} // namespace app::handlers::payments
