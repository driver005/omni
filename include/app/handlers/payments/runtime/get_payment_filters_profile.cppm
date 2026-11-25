export module app_handlers_payments:runtime_get_payment_filters_profile;

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

import :input;
import :result;
import :runtime_common;

export namespace app::handlers::payments {

class GetPaymentFiltersProfileRuntime
{
public:
    explicit GetPaymentFiltersProfileRuntime(
        app::context::AppContext& context,
        PaymentsRuntimeSupport& support
    ) noexcept :
        m_context{context},
        m_support{support}
    {
    }

    [[nodiscard]] std::expected<GetPaymentFiltersProfileResult, core::error::AppError>
    get_payment_filters_profile(const GetPaymentFiltersProfileInput& input) const
    {
        if (const auto err = m_support.ensure_payment_exists(input.data)) {
            return std::unexpected{*err};
        }
        return GetPaymentFiltersProfileResult{m_support.payment_response_view(input.data)};
    }

private:
    app::context::AppContext& m_context;
    PaymentsRuntimeSupport& m_support;
};

} // namespace app::handlers::payments
