export module app_handlers_routing:runtime_routing_retrieve_linked_config;

import std;
import core_error;
import core_serde;
import app_context;
import app_model;

import :input;
import :result;
import :runtime_common;

export namespace app::handlers::routing {

class RoutingRetrieveLinkedConfigRuntime
{
public:
    explicit RoutingRetrieveLinkedConfigRuntime(app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] std::expected<RoutingRetrieveLinkedConfigResult, core::error::AppError>
    routing_retrieve_linked_config(const RoutingInput& input) const
    {
        const auto merchant = input.merchant_id;
        const auto txn = input.txn;
        const auto profile = input.profile_id;
        if (!profile.empty()) {
            if (const auto validation_error =
                    m_support.profile_validation_error(input.auth_profile, profile)) {
                return std::unexpected{m_support.error(
                    core::error::AppErrorKind::PRECONDITION_FAILED,
                    *validation_error
                )};
            }
        }

        std::vector<DictionaryRecord> active;
        const auto active_id = m_support.active_id_for(input.active_id, profile, txn);
        for (auto& record: m_context.repository().list_routing_configs()) {
            if (!merchant.empty() && record.get_merchant_id() != merchant) {
                continue;
            }
            if (!profile.empty() && record.get_profile_id() != profile) {
                continue;
            }
            if (!input.is_v2) {
                if (active_id && record.get_config_id() == *active_id) {
                    active.push_back(m_support.dictionary_record(record));
                }
            } else if (m_support.transaction_type_of(record.get_config_json()) == txn) {
                // core:2155-2159 filters v2 metadata by transaction type.
                active.push_back(m_support.dictionary_record(record));
            }
        }
        m_support.track("routing_retrieve_linked_config");
        return RoutingRetrieveLinkedConfigResult{ResponseDocument::from(active)};
    }

private:
    app::context::AppContext& m_context;
    RoutingRuntimeSupport m_support;
};

} // namespace app::handlers::routing
