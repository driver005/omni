export module app_routing:routing_runtime_list_routing_configs_for_profile;

import std;
import core_serde;
import core_error;
import app_context;
import app_model;
import core_behavior;

import :routing_result;
import :routing_runtime_common;

export namespace app::handlers::routing {

class ListRoutingConfigsForProfileRuntime
{
public:
    explicit ListRoutingConfigsForProfileRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] core::behavior::Outcome<serde::Value>
    list_routing_configs_for_profile(const ::core::behavior::CallContext& ctx) const
    {
        const auto list_limit = ctx_query_number<std::uint16_t>(ctx, "limit");
        const auto list_offset = ctx_query_number<std::uint8_t>(ctx, "offset");
        ::app::model::routing::RoutingRetrieveQuery filter;
        if (list_limit) {
            filter.set_limit(*list_limit);
        }
        if (list_offset) {
            filter.set_offset(*list_offset);
        }
        if (ctx_query(ctx, "transaction_type")) {
            filter.set_transaction_type(m_support.wrap<::app::model::routing::TransactionTypeValue>(
                *ctx_query(ctx, "transaction_type")
            ));
        }
        auto items = m_support.list_impl(
            ctx.merchant_id,
            ctx.profile_id,
            ctx_transaction_type(ctx),
            true,
            "list_routing_configs_for_profile",
            filter
        );
        return ResponseDocument::from(items);
    }

private:
    ::app::context::AppContext& m_context;
    RoutingRuntimeSupport m_support;
};

} // namespace app::handlers::routing
