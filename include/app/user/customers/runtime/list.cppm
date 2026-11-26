export module app_user:customers_runtime_list;

import std;
import core_error;
import core_serde;
import app_context;
import app_store;

import :customers_input;
import :customers_result;
import :customers_runtime_common;

export namespace app::handlers::customers {

class CustomersListRuntime
{
public:
    explicit CustomersListRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] std::expected<CustomersListResult, ::core::error::AppError>
    list(const CustomersListRouteInput& input) const
    {
        auto value = list_core(input);
        if (!value) {
            return std::unexpected{value.error()};
        }
        return CustomersListResult{std::move(*value)};
    }

    [[nodiscard]] std::expected<CustomersListV2Result, ::core::error::AppError>
    list_v2(const CustomersListRouteInput& input) const
    {
        auto value = list_core(input);
        if (!value) {
            return std::unexpected{value.error()};
        }
        return CustomersListV2Result{std::move(*value)};
    }

private:
    [[nodiscard]] std::expected<serde::Value, ::core::error::AppError>
    list_core(const CustomersListRouteInput& input) const
    {
        const std::size_t off = input.offset.value_or(0);
        const std::size_t lim =
            input.limit ? static_cast<std::size_t>(*input.limit) : kDefaultListApiLimit;
        auto repo = m_context.repository();
        std::size_t position{0};
        serde::Value::Array data{};
        for (const auto& record: repo.list_customers()) {
            if (!input.merchant_id.empty() && record.get_merchant_id() != input.merchant_id) {
                continue;
            }
            if (input.customer_id && record.get_customer_id() != *input.customer_id) {
                continue;
            }
            if (position < off) {
                ++position;
                continue;
            }
            if (position - off >= lim) {
                break;
            }
            const auto aggregate = m_support.load_aggregate(record);
            data.push_back(
                m_support.customer_document(*aggregate, std::nullopt, std::nullopt, input.version)
            );
            ++position;
        }
        m_context.metrics().record_flow("customers.list");
        m_context.metrics().counter("customers_list").increment();
        return serde::Value{std::move(data)};
    }

    ::app::context::AppContext& m_context;
    CustomersRuntimeSupport m_support;
};

} // namespace app::handlers::customers
