export module app_user:customers_runtime_retrieve_by_merchant_reference_id;

import std;
import core_behavior;
import core_error;
import core_serde;
import app_context;
import app_store;

import :customers_input;
import :customers_runtime_common;

export namespace app::handlers::customers {

class CustomersRetrieveByReferenceRuntime
{
public:
    explicit CustomersRetrieveByReferenceRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] core::behavior::Outcome<serde::Value>
    retrieve_by_merchant_reference_id(const ::core::behavior::CallContext& ctx) const
    {
        auto repo = m_context.repository();
        const auto aggregate =
            m_support.find_by_reference(repo, ctx_string(ctx,"reference"), ctx_string(ctx,"merchant_id"));
        if (!aggregate) {
            return std::unexpected{
                m_support.upstream_error("CustomerNotFound", "customer not found")
            };
        }
        serde::Value document =
            m_support.customer_document(*aggregate, std::nullopt, std::nullopt, ApiVersion::V2);
        m_context.metrics().record_flow("customers.retrieve_by_merchant_reference_id");
        m_context.metrics().counter("customers_retrieve_by_merchant_reference_id").increment();
        return std::move(document);
    }

private:
    ::app::context::AppContext& m_context;
    CustomersRuntimeSupport m_support;
};

} // namespace app::handlers::customers
