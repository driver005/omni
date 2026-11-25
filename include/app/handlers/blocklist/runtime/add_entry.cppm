export module app_handlers_blocklist:runtime_add_entry;

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

import :input;
import :result;
import :runtime_common;

export namespace app::handlers::blocklist {

class AddEntryRuntime
{
public:
    explicit AddEntryRuntime(app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    // utils.rs:377-430 (fingerprint) and :448-504 (BIN duplicate check).
    [[nodiscard]] std::expected<AddEntryResult, core::error::AppError>
    add_entry(const AddEntryInput& input) const
    {
        if (m_support.find_by_identity(
                input.merchant,
                input.profile,
                input.data,
                input.transaction_type
            )) {
            const auto message =
                input.kind == "payment_method"
                    ? std::string{"data associated with the given fingerprint is already blocked"}
                    : std::string{"provided bin is already blocked"};
            // Upstream PreconditionFailed maps to HTTP 400 (api_error_response.rs:626-628).
            return std::unexpected{
                m_support.error(core::error::AppErrorKind::INVALID_REQUEST, message)
            };
        }

        const auto created = m_support.now_iso8601();
        m_support.save_entry(m_support.make_entry(
            input.data,
            input.merchant,
            input.profile,
            input.kind,
            input.transaction_type,
            created
        ));

        // transformers.rs:17-27 (ForeignFrom<storage::Blocklist>).
        app::model::blocklist::BlocklistResponse response;
        response.set_fingerprint_id(input.data);
        response.set_data_kind(
            m_support.wrap<app::model::blocklist::BlocklistDataKindValue>(input.kind)
        );
        response.set_transaction_type(
            m_support.wrap<app::model::blocklist::BlocklistTransactionTypeValue>(
                input.transaction_type
            )
        );
        response.set_created_at(
            m_support.wrap<app::model::blocklist::PrimitiveDateTimeValue>(created)
        );
        if (!input.profile.empty()) {
            response.set_profile_id(
                m_support.wrap<app::model::blocklist::ProfileIdValue>(input.profile)
            );
        }
        m_support.emit("blocklist.entry.added", response);
        m_support.track("add_entry_to_blocklist");
        return AddEntryResult{response};
    }

private:
    app::context::AppContext& m_context;
    BlocklistRuntimeSupport m_support;
};

} // namespace app::handlers::blocklist
