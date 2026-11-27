export module app_routing:blocklist_runtime_list_blocked_payment_methods;

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

class ListBlockedPaymentMethodsRuntime
{
public:
    explicit ListBlockedPaymentMethodsRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    // utils.rs:120-162 list_blocklist_entries_for_merchant.
    [[nodiscard]] std::
        expected<::app::model::blocklist::ListBlocklistResponse, core::error::AppError>
        list_blocked_payment_methods(const ListBlockedPaymentMethodsInput& input) const
    {
        std::vector<EntryRecord> selected;
        for (const auto& record: m_support.list_entries()) {
            if (!m_support.matches_scope(
                    record,
                    input.merchant,
                    input.profile,
                    input.kind,
                    input.transaction_type
                )) {
                continue;
            }
            selected.push_back(record);
        }
        m_support.sort_by_created_desc(selected);
        const std::size_t total = selected.size();
        auto page = m_support.paginate(std::move(selected), input.offset, input.limit);

        std::vector<::app::model::blocklist::BlocklistResponse> data;
        data.reserve(page.size());
        for (const auto& record: page) {
            data.push_back(m_support.response_of(record));
        }

        ::app::model::blocklist::ListBlocklistResponse response;
        response.set_count(data.size());
        response.set_total_count(total);
        response.set_data(std::move(data));
        m_support.emit("blocklist.list", response);
        m_support.track("list_blocked_payment_methods");
        return response;
    }

private:
    ::app::context::AppContext& m_context;
    BlocklistRuntimeSupport m_support;
};

} // namespace app::handlers::blocklist
