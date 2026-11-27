export module app_payment:payments_runtime_get_payment_filters_profile;

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

class GetPaymentFiltersProfileRuntime
{
public:
    explicit GetPaymentFiltersProfileRuntime(
        ::app::context::AppContext& context,
        PaymentsRuntimeSupport& support
    ) noexcept :
        m_context{context},
        m_support{support}
    {
    }

    [[nodiscard]] core::behavior::Outcome<::app::model::payments::PaymentsResponse>
    get_payment_filters_profile(const ::app::model::payments::PaymentsRequest& dto, const core::behavior::CallContext& ctx) const
    {
        const PaymentsRequestData input = m_support.request_data(dto, ctx);
        if (const auto err = m_support.ensure_payment_exists(input)) {
            return std::unexpected{*err};
        }
        return ::app::model::payments::PaymentsResponse{
            m_support.payment_response_view(input)
        };
    }

private:
    ::app::context::AppContext& m_context;
    PaymentsRuntimeSupport& m_support;
};

} // namespace app::handlers::payments
