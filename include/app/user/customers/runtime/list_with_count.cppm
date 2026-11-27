export module app_user:customers_runtime_list_with_count;

import std;
import core_behavior;
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

    [[nodiscard]] core::behavior::Outcome<serde::Value>
    list_with_count(const ::core::behavior::CallContext& ctx) const
    {
        auto value = list_with_count_core(ctx);
        if (!value) {
            return std::unexpected{value.error()};
        }
        return std::move(*value);
    }

    [[nodiscard]] core::behavior::Outcome<serde::Value>
    list_with_count_v2(const ::core::behavior::CallContext& ctx) const
    {
        auto value = list_with_count_core(ctx);
        if (!value) {
            return std::unexpected{value.error()};
        }
        return std::move(*value);
    }

private:
    [[nodiscard]] core::behavior::Outcome<serde::Value>
    list_with_count_core(const ::core::behavior::CallContext& ctx) const
    {
        auto repo = m_context.repository();
        std::vector<CustomerAggregate> filtered{};
        for (const auto& record: repo.list_customers()) {
            if (!ctx_string(ctx,"merchant_id").empty() && record.get_merchant_id() != ctx_string(ctx,"merchant_id")) {
                continue;
            }
            if (ctx_optional_string(ctx,"customer_id") && record.get_customer_id() != *ctx_optional_string(ctx,"customer_id")) {
                continue;
            }
            if (ctx_time_range(ctx) &&
                !m_support.within_range(record.get_created_at(), *ctx_time_range(ctx))) {
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
            if (position < ctx_size(ctx,"offset")) {
                ++position;
                continue;
            }
            if (position - ctx_size(ctx,"offset") >= ctx_size(ctx,"limit")) {
                break;
            }
            data.push_back(
                m_support.customer_document(aggregate, std::nullopt, std::nullopt, ctx_version(ctx))
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
