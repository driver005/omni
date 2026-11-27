export module app_payment:payments_runtime_payment_link_create;

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

    [[nodiscard]] std::expected<::app::model::payments::PaymentLinkResponse, core::error::AppError>
    payment_link_create(const PaymentLinkCreateInput& input) const
    {
        const std::string new_payment_id = m_context.ids().payment_id();
        ::app::store::payments::PaymentIntentRecord record{};
        record.set_payment_id(new_payment_id);
        record.set_merchant_id(input.data.merchant_id);
        record.set_status("requires_payment_method");
        m_context.repository().save_payment(std::move(record));
        m_context.metrics().counter("payments.created").increment();
        ::app::model::payments::PaymentLinkResponse response{};
        response.set_link(input.data.body);
        response.set_payment_link_id(new_payment_id);
        return ::app::model::payments::PaymentLinkResponse{response};
    }

private:
    ::app::context::AppContext& m_context;
    PaymentsRuntimeSupport& m_support;
};

} // namespace app::handlers::payments
