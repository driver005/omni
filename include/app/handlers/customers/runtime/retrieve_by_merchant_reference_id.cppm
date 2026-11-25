export module app_handlers_customers:runtime_retrieve_by_merchant_reference_id;

import std;
import core_error;
import core_serde;
import app_context;
import app_store;

import :input;
import :result;
import :runtime_common;

export namespace app::handlers::customers {

class CustomersRetrieveByReferenceRuntime
{
public:
    explicit CustomersRetrieveByReferenceRuntime(app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] std::
        expected<CustomersRetrieveByMerchantReferenceIdResult, ::core::error::AppError>
        retrieve_by_merchant_reference_id(const CustomersRetrieveByReferenceRouteInput& input) const
    {
        auto repo = m_context.repository();
        const auto aggregate =
            m_support.find_by_reference(repo, input.reference, input.merchant_id);
        if (!aggregate) {
            return std::unexpected{
                m_support.upstream_error("CustomerNotFound", "customer not found")
            };
        }
        serde::Value document =
            m_support.customer_document(*aggregate, std::nullopt, std::nullopt, ApiVersion::V2);
        m_context.metrics().record_flow("customers.retrieve_by_merchant_reference_id");
        m_context.metrics().counter("customers_retrieve_by_merchant_reference_id").increment();
        return CustomersRetrieveByMerchantReferenceIdResult{std::move(document)};
    }

private:
    app::context::AppContext& m_context;
    CustomersRuntimeSupport m_support;
};

} // namespace app::handlers::customers
