export module app_user:customers_runtime_retrieve;

import std;
import core_error;
import core_serde;
import app_context;
import app_store;

import :customers_input;
import :customers_runtime_common;

export namespace app::handlers::customers {

class CustomersRetrieveRuntime
{
public:
    explicit CustomersRetrieveRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] std::expected<serde::Value, core::error::AppError>
    retrieve(const CustomersRetrieveRouteInput& input) const
    {
        auto value = retrieve_core(input);
        if (!value) {
            return std::unexpected{value.error()};
        }
        return std::move(*value);
    }

    [[nodiscard]] std::expected<serde::Value, core::error::AppError>
    retrieve_v2(const CustomersRetrieveRouteInput& input) const
    {
        auto value = retrieve_core(input);
        if (!value) {
            return std::unexpected{value.error()};
        }
        return std::move(*value);
    }

private:
    [[nodiscard]] std::expected<serde::Value, ::core::error::AppError>
    retrieve_core(const CustomersRetrieveRouteInput& input) const
    {
        auto repo = m_context.repository();
        const auto aggregate = input.version == ApiVersion::V1
                                   ? m_support.locate_customer(repo, input.merchant_id, input.id)
                                   : m_support.locate_global_customer(repo, input.id);
        if (!aggregate) {
            return std::unexpected{
                m_support.upstream_error("CustomerNotFound", "customer not found")
            };
        }
        serde::Value document =
            m_support.customer_document(*aggregate, std::nullopt, std::nullopt, input.version);
        m_context.metrics().record_flow("customers.retrieve");
        m_context.metrics().counter("customers_retrieve").increment();
        return document;
    }

    ::app::context::AppContext& m_context;
    CustomersRuntimeSupport m_support;
};

} // namespace app::handlers::customers
