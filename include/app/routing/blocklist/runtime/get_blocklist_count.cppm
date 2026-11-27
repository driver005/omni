export module app_routing:blocklist_runtime_get_blocklist_count;

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
import core_behavior;

import :blocklist_result;
import :blocklist_runtime_common;

export namespace app::handlers::blocklist {

class GetBlocklistCountRuntime
{
public:
    explicit GetBlocklistCountRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    // utils.rs:192-269 get_blocklist_count.
    [[nodiscard]] core::behavior::Outcome<::app::model::blocklist::BlocklistCountResponse>
    get_blocklist_count(const ::core::behavior::CallContext& ctx) const
    {
        const std::string kind = m_support.query_value(ctx, "data_kind").value_or("");
        const std::string transaction_type =
            m_support.query_value(ctx, "transaction_type").value_or("");
        std::size_t total{0};
        std::map<std::string, std::size_t> by_length;
        for (const auto& record: m_support.list_entries()) {
            if (!m_support.matches_scope(
                    record,
                    ctx.merchant_id,
                    ctx.profile_id,
                    kind,
                    transaction_type
                )) {
                continue;
            }
            ++total;
            // utils.rs:231-260 groups by length(fingerprint_id); fingerprints (fixed width)
            // have no breakdown, so counts_by_length is None for payment_method.
            if (kind != "payment_method") {
                ++by_length[std::to_string(record.get_fingerprint().size())];
            }
        }

        ::app::model::blocklist::BlocklistCountResponse response;
        response.set_data_kind(
            m_support.wrap<::app::model::blocklist::BlocklistDataKindValue>(kind)
        );
        response.set_total_count(total);
        if (kind != "payment_method" && !by_length.empty()) {
            response.set_counts_by_length(std::move(by_length));
        }
        m_support.emit("blocklist.count", response);
        m_support.track("get_blocklist_count");
        return response;
    }

private:
    ::app::context::AppContext& m_context;
    BlocklistRuntimeSupport m_support;
};

} // namespace app::handlers::blocklist
