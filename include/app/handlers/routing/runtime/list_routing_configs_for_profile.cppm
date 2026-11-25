export module app_handlers_routing:runtime_list_routing_configs_for_profile;

import std;
import core_error;
import app_context;
import app_model;

import :input;
import :result;
import :runtime_common;

export namespace app::handlers::routing {

class ListRoutingConfigsForProfileRuntime
{
public:
    explicit ListRoutingConfigsForProfileRuntime(app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] std::expected<ListRoutingConfigsForProfileResult, core::error::AppError>
    list_routing_configs_for_profile(const RoutingInput& input) const
    {
        app::model::routing::RoutingRetrieveQuery filter;
        if (input.list_limit) {
            filter.set_limit(*input.list_limit);
        }
        if (input.list_offset) {
            filter.set_offset(*input.list_offset);
        }
        if (input.query_txn) {
            filter.set_transaction_type(
                m_support.wrap<app::model::routing::TransactionTypeValue>(*input.query_txn)
            );
        }
        auto items = m_support.list_impl(
            input.merchant_id,
            input.profile_id,
            input.txn,
            true,
            "list_routing_configs_for_profile",
            filter
        );
        return ListRoutingConfigsForProfileResult{ResponseDocument::from(items)};
    }

private:
    app::context::AppContext& m_context;
    RoutingRuntimeSupport m_support;
};

} // namespace app::handlers::routing
