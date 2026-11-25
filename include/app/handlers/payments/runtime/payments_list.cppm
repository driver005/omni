export module app_handlers_payments:runtime_payments_list;

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

class PaymentsListRuntime
{
public:
    explicit PaymentsListRuntime(
        app::context::AppContext& context,
        PaymentsRuntimeSupport& support
    ) noexcept :
        m_context{context},
        m_support{support}
    {
    }

    [[nodiscard]] std::expected<PaymentsListResult, core::error::AppError>
    payments_list(const PaymentsListInput& input) const
    {
        auto records = m_support.list_payments_scoped(input.data);
        if (const auto status_filter = m_support.query(input.data, "status");
            !status_filter.empty()) {
            std::vector<app::store::payments::PaymentIntentRecord> filtered{};
            for (const auto& record: records) {
                if (record.get_status() == status_filter) {
                    filtered.push_back(record);
                }
            }
            records = std::move(filtered);
        }
        if (const auto err = m_support.ensure_payment_exists(input.data)) {
            return std::unexpected{*err};
        }
        return PaymentsListResult{m_support.payment_list_response(records)};
    }

private:
    app::context::AppContext& m_context;
    PaymentsRuntimeSupport& m_support;
};

} // namespace app::handlers::payments
