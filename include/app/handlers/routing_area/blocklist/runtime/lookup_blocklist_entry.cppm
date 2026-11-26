export module app_routing:blocklist_runtime_lookup_blocklist_entry;

import std;
import interfaces;
import core_router;
import core_serde;
import utils_openapi;
import app_model;
import app_dto;
import app_context;
import core_cache;
import core_error;
import app_auth;
import app_http;
import app_crypto;
import app_metrics;
import app_store;
import app_connector;
import app_events;
import app_workflow;
import app_keystore;

import :blocklist_input;
import :blocklist_result;
import :blocklist_runtime_common;

export namespace app::handlers::blocklist {

class LookupBlocklistEntryRuntime
{
public:
    explicit LookupBlocklistEntryRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    // utils.rs:271-314 lookup_blocklist_entry.
    [[nodiscard]] std::expected<LookupBlocklistEntryResult, core::error::AppError>
    lookup_blocklist_entry(const LookupBlocklistEntryInput& input) const
    {
        std::vector<std::string> distinct;
        for (const auto& record: m_support.list_entries()) {
            if (!m_support.matches_merchant(record, input.merchant) ||
                !m_support.matches_profile(record, input.profile)) {
                continue;
            }
            if (record.get_fingerprint() != input.data) {
                continue;
            }
            if (!input.transaction_type.empty() &&
                !m_support.tt_matches(record, input.transaction_type)) {
                continue;
            }
            const auto tt = m_support.legacy_transaction_type(record);
            if (std::find(distinct.begin(), distinct.end(), tt) == distinct.end()) {
                distinct.push_back(tt);
            }
        }

        std::vector<::app::model::blocklist::BlocklistTransactionTypeValue> types;
        types.reserve(distinct.size());
        for (const auto& tt: distinct) {
            types.push_back(
                m_support.wrap<::app::model::blocklist::BlocklistTransactionTypeValue>(tt)
            );
        }
        ::app::model::blocklist::BlocklistLookupResponse response;
        response.set_data(input.data);
        response.set_blocked(!types.empty());
        response.set_transaction_types(std::move(types));
        m_support.emit("blocklist.lookup", response);
        m_support.track("lookup_blocklist_entry");
        return LookupBlocklistEntryResult{response};
    }

private:
    ::app::context::AppContext& m_context;
    BlocklistRuntimeSupport m_support;
};

} // namespace app::handlers::blocklist
