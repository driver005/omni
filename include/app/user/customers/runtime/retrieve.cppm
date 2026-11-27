export module app_user:customers_runtime_retrieve;

import std;
import core_behavior;
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

    [[nodiscard]] core::behavior::Outcome<serde::Value>
    retrieve(const ::core::behavior::CallContext& ctx) const
    {
        auto value = retrieve_core(ctx);
        if (!value) {
            return std::unexpected{value.error()};
        }
        return std::move(*value);
    }

    [[nodiscard]] core::behavior::Outcome<serde::Value>
    retrieve_v2(const ::core::behavior::CallContext& ctx) const
    {
        auto value = retrieve_core(ctx);
        if (!value) {
            return std::unexpected{value.error()};
        }
        return std::move(*value);
    }

private:
    [[nodiscard]] core::behavior::Outcome<serde::Value>
    retrieve_core(const ::core::behavior::CallContext& ctx) const
    {
        auto repo = m_context.repository();
        const auto aggregate = ctx_version(ctx) == ApiVersion::V1
                                   ? m_support.locate_customer(
                                         repo,
                                         ctx_string(ctx, "merchant_id"),
                                         ctx_string(ctx, "id")
                                     )
                                   : m_support.locate_global_customer(repo, ctx_string(ctx, "id"));
        if (!aggregate) {
            return std::unexpected{
                m_support.upstream_error("CustomerNotFound", "customer not found")
            };
        }
        serde::Value document =
            m_support.customer_document(*aggregate, std::nullopt, std::nullopt, ctx_version(ctx));
        m_context.logger().record_flow("customers.retrieve");
        m_context.logger().counter("customers_retrieve").increment();
        return document;
    }

    ::app::context::AppContext& m_context;
    CustomersRuntimeSupport m_support;
};

} // namespace app::handlers::customers
