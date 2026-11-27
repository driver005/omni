export module app_user:customers_runtime_list;

import std;
import core_behavior;
import core_error;
import core_serde;
import app_context;
import app_store;

import :customers_input;
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

    [[nodiscard]] core::behavior::Outcome<serde::Value>
    list(const ::core::behavior::CallContext& ctx) const
    {
        auto value = list_core(ctx);
        if (!value) {
            return std::unexpected{value.error()};
        }
        return std::move(*value);
    }

    [[nodiscard]] core::behavior::Outcome<serde::Value>
    list_v2(const ::core::behavior::CallContext& ctx) const
    {
        auto value = list_core(ctx);
        if (!value) {
            return std::unexpected{value.error()};
        }
        return std::move(*value);
    }

private:
    [[nodiscard]] core::behavior::Outcome<serde::Value>
    list_core(const ::core::behavior::CallContext& ctx) const
    {
        const std::size_t off = ctx_optional_number<std::uint32_t>(ctx, "offset").value_or(0);
        const std::size_t lim =
            ctx_optional_number<std::uint16_t>(ctx, "limit")
                ? static_cast<std::size_t>(*ctx_optional_number<std::uint16_t>(ctx, "limit"))
                : kDefaultListApiLimit;
        auto repo = m_context.repository();
        std::size_t position{0};
        serde::Value::Array data{};
        for (const auto& record: repo.list_customers()) {
            if (!ctx_string(ctx, "merchant_id").empty() &&
                record.get_merchant_id() != ctx_string(ctx, "merchant_id")) {
                continue;
            }
            if (ctx_optional_string(ctx, "customer_id") &&
                record.get_customer_id() != *ctx_optional_string(ctx, "customer_id")) {
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
                m_support
                    .customer_document(*aggregate, std::nullopt, std::nullopt, ctx_version(ctx))
            );
            ++position;
        }
        m_context.logger().record_flow("customers.list");
        m_context.logger().counter("customers_list").increment();
        return serde::Value{std::move(data)};
    }

    ::app::context::AppContext& m_context;
    CustomersRuntimeSupport m_support;
};

} // namespace app::handlers::customers
