export module app_payment:payments_runtime_payments_dynamic_tax_calculation;

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

class PaymentsDynamicTaxCalculationRuntime
{
public:
    explicit PaymentsDynamicTaxCalculationRuntime(
        ::app::context::AppContext& context,
        PaymentsRuntimeSupport& support
    ) noexcept :
        m_context{context},
        m_support{support}
    {
    }

    [[nodiscard]] std::expected<::app::model::payments::PaymentsResponse, core::error::AppError>
    payments_dynamic_tax_calculation(const PaymentsDynamicTaxCalculationInput& input) const
    {
        if (const auto err = m_support.ensure_payment_exists(input.data)) {
            return std::unexpected{*err};
        }
        return ::app::model::payments::PaymentsResponse{
            m_support.payment_response_view(input.data)
        };
    }

private:
    ::app::context::AppContext& m_context;
    PaymentsRuntimeSupport& m_support;
};

} // namespace app::handlers::payments
