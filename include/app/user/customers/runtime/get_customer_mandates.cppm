export module app_user:customers_runtime_get_customer_mandates;

import std;
import core_error;
import core_serde;
import app_context;
import app_store;

import :customers_input;
import :customers_result;
import :customers_runtime_common;

export namespace app::handlers::customers {

class CustomersMandatesRuntime
{
public:
    explicit CustomersMandatesRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] std::expected<GetCustomerMandatesResult, ::core::error::AppError>
    get_customer_mandates(const CustomersMandatesRouteInput& input) const
    {
        auto repo = m_context.repository();
        const auto customer = m_support.locate_customer(repo, input.merchant_id, input.id);
        if (!customer) {
            return std::unexpected{
                m_support.upstream_error("CustomerNotFound", "customer not found")
            };
        }
        const std::string merchant_id = input.merchant_id;
        serde::Value::Array out{};
        for (const auto& record:
             repo.find_mandates_by_customer_id(customer->record.get_customer_id())) {
            if (!merchant_id.empty() && record.get_merchant_id() != merchant_id) {
                continue;
            }
            serde::Value::Object item{};
            item["mandate_id"] = serde::Value{record.get_mandate_id()};
            item["status"] = serde::Value{record.get_status()};
            item["payment_method_id"] = serde::Value{record.get_payment_method_id()};
            item["payment_method"] = serde::Value{record.get_payment_method()};
            out.push_back(serde::Value{std::move(item)});
        }
        m_context.metrics().record_flow("customers.get_customer_mandates");
        m_context.metrics().counter("customers_get_customer_mandates").increment();
        return GetCustomerMandatesResult{serde::Value{std::move(out)}};
    }

private:
    ::app::context::AppContext& m_context;
    CustomersRuntimeSupport m_support;
};

} // namespace app::handlers::customers
