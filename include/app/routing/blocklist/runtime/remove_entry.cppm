export module app_routing:blocklist_runtime_remove_entry;

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

class RemoveEntryRuntime
{
public:
    explicit RemoveEntryRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    // utils.rs:25-74, 506-525.
    [[nodiscard]] std::expected<::app::model::blocklist::BlocklistResponse, core::error::AppError>
    remove_entry(const RemoveEntryInput& input) const
    {
        const auto existing = m_support.find_by_identity(
            input.merchant,
            input.profile,
            input.data,
            input.transaction_type
        );
        if (!existing) {
            const auto message =
                input.kind == "payment_method"
                    ? std::string{"no blocklist record for the given fingerprint id was found"}
                    : std::string{"could not find a blocklist entry for the given bin"};
            return std::unexpected{m_support.error(core::error::AppErrorKind::NOT_FOUND, message)};
        }

        auto response = m_support.response_of(*existing);
        m_support.remove_entry(existing->get_id());
        m_support.emit("blocklist.entry.removed", response);
        m_support.track("remove_entry_from_blocklist");
        return response;
    }

private:
    ::app::context::AppContext& m_context;
    BlocklistRuntimeSupport m_support;
};

} // namespace app::handlers::blocklist
