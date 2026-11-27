export module app_user:customers_runtime_list_with_count;

import std;
import core_error;
import core_serde;
import app_context;
import app_store;

import :customers_input;
import :customers_runtime_common;

export namespace app::handlers::customers {

class CustomersListWithCountRuntime
{
public:
    explicit CustomersListWithCountRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] std::expected<serde::Value, core::error::AppError>
    list_with_count(const CustomersListWithCountRouteInput& input) const
    {
        auto value = list_with_count_core(input);
        if (!value) {
            return std::unexpected{value.error()};
        }
        return std::move(*value);
    }

    [[nodiscard]] std::expected<serde::Value, core::error::AppError>
    list_with_count_v2(const CustomersListWithCountRouteInput& input) const
    {
        auto value = list_with_count_core(input);
        if (!value) {
            return std::unexpected{value.error()};
        }
        return std::move(*value);
    }

private:
    [[nodiscard]] std::expected<serde::Value, ::core::error::AppError>
    list_with_count_core(const CustomersListWithCountRouteInput& input) const
    {
        auto repo = m_context.repository();
        std::vector<CustomerAggregate> filtered{};
        for (const auto& record: repo.list_customers()) {
            if (!input.merchant_id.empty() && record.get_merchant_id() != input.merchant_id) {
                continue;
            }
            if (input.customer_id && record.get_customer_id() != *input.customer_id) {
                continue;
            }
            if (input.time_range &&
                !m_support.within_range(record.get_created_at(), *input.time_range)) {
                continue;
            }
            if (auto aggregate = m_support.load_aggregate(record)) {
                filtered.push_back(std::move(*aggregate));
            }
        }
        const std::size_t total = filtered.size();
        serde::Value::Array data{};
        std::size_t position{0};
        for (const auto& aggregate: filtered) {
            if (position < input.offset) {
                ++position;
                continue;
            }
            if (position - input.offset >= input.limit) {
                break;
            }
            data.push_back(
                m_support.customer_document(aggregate, std::nullopt, std::nullopt, input.version)
            );
            ++position;
        }
        serde::Value::Object response{};
        response["data"] = serde::Value{std::move(data)};
        response["total_count"] = serde::Value{static_cast<std::int64_t>(total)};
        m_context.metrics().record_flow("customers.list_with_count");
        m_context.metrics().counter("customers_list_with_count").increment();
        return serde::Value{std::move(response)};
    }

    ::app::context::AppContext& m_context;
    CustomersRuntimeSupport m_support;
};

} // namespace app::handlers::customers
