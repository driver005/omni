export module app_routing:routing_runtime_routing_retrieve_linked_config;

import std;
import core_error;
import core_serde;
import app_context;
import app_model;
import core_behavior;

import :routing_result;
import :routing_runtime_common;

export namespace app::handlers::routing {

class RoutingRetrieveLinkedConfigRuntime
{
public:
    explicit RoutingRetrieveLinkedConfigRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] core::behavior::Outcome<serde::Value>
    routing_retrieve_linked_config(const ::core::behavior::CallContext& ctx) const
    {
        const auto merchant = ctx.merchant_id;
        const auto txn = ctx_transaction_type(ctx);
        const auto profile = ctx.profile_id;
        if (!profile.empty()) {
            if (const auto validation_error =
                    m_support.profile_validation_error(ctx_auth_profile(ctx), profile)) {
                return std::unexpected{m_support.error(
                    core::error::AppErrorKind::PRECONDITION_FAILED,
                    *validation_error
                )};
            }
        }

        std::vector<DictionaryRecord> active;
        const auto active_id = m_support.active_id_for(ctx_query(ctx, "active_id"), profile, txn);
        for (auto& record: m_context.repository().list_routing_configs()) {
            if (!merchant.empty() && record.get_merchant_id() != merchant) {
                continue;
            }
            if (!profile.empty() && record.get_profile_id() != profile) {
                continue;
            }
            if (!ctx_is_v2(ctx)) {
                if (active_id && record.get_config_id() == *active_id) {
                    active.push_back(m_support.dictionary_record(record));
                }
            } else if (m_support.transaction_type_of(record.get_config_json()) == txn) {
                // core:2155-2159 filters v2 metadata by transaction type.
                active.push_back(m_support.dictionary_record(record));
            }
        }
        m_support.track("routing_retrieve_linked_config");
        return ResponseDocument::from(active);
    }

private:
    ::app::context::AppContext& m_context;
    RoutingRuntimeSupport m_support;
};

} // namespace app::handlers::routing
