export module app_payment:payments_runtime_get_payments_aggregates;

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

class GetPaymentsAggregatesRuntime
{
public:
    explicit GetPaymentsAggregatesRuntime(
        ::app::context::AppContext& context,
        PaymentsRuntimeSupport& support
    ) noexcept :
        m_context{context},
        m_support{support}
    {
    }

    [[nodiscard]] core::behavior::Outcome<::app::model::payments::PaymentsAggregateResponse>
        get_payments_aggregates(const ::app::model::payments::PaymentsRequest& dto, const core::behavior::CallContext& ctx) const
    {
        const PaymentsRequestData input = m_support.request_data(dto, ctx);
        auto records = m_support.list_payments_scoped(input);
        if (const auto status_filter = m_support.query(input, "status");
            !status_filter.empty()) {
            std::vector<::app::store::payments::PaymentIntentRecord> filtered{};
            for (const auto& record: records) {
                if (record.get_status() == status_filter) {
                    filtered.push_back(record);
                }
            }
            records = std::move(filtered);
        }
        if (const auto err = m_support.ensure_payment_exists(input)) {
            return std::unexpected{*err};
        }
        std::unordered_map<std::string, std::int64_t> status_counts;
        for (const auto& record: records) {
            ++status_counts[record.get_status()];
        }
        ::app::model::payments::PaymentsAggregateResponse response{};
        response.set_status_with_count(std::move(status_counts));
        return ::app::model::payments::PaymentsAggregateResponse{response};
    }

private:
    ::app::context::AppContext& m_context;
    PaymentsRuntimeSupport& m_support;
};

} // namespace app::handlers::payments
