export module app_handlers_disputes:runtime_retrieve_dispute;

import std;
import interfaces;
import core_router;
import core_serde;
import core_cache;
import core_database;
import core_error;
import utils_openapi;
import app_model;
import app_dto;
import app_context;
import app_auth;
import app_http;
import app_metrics;
import app_store;
import app_connector;
import app_workflow;
import app_events;
import app_locking;
import app_services;

import :input;
import :result;
import :runtime_common;

export namespace app::handlers::disputes {

class DisputesRetrieveDisputeRuntime
{
public:
    explicit DisputesRetrieveDisputeRuntime(app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] std::expected<RetrieveDisputeResult, Error>
    retrieve_dispute(const RetrieveDisputeInput& input) const
    {
        m_context.metrics().record_flow("dispute_retrieve");
        const auto& scope = input.scope;
        const auto& request = input.request;
        const auto& db = m_context.repository();
        auto dispute = m_support.find_dispute_by_processor_merchant_id_dispute_id(
            scope,
            request.get_dispute_id()
        );
        if (!dispute) {
            return std::unexpected{m_support.dispute_not_found(request.get_dispute_id())};
        }
        if (const auto invalid = m_support.validate_profile_id_from_auth_layer(scope, *dispute)) {
            return std::unexpected{*invalid};
        }

        app::model::disputes::DisputeResponse response{};
        const bool force = m_support.should_call_connector_for_dispute_sync(
            request.get_force_sync().value_or(false),
            m_support.canonical_status(dispute->get_status())
        );
        if (force) {
            // Force-sync Dsync substitute: short-lived dedupe marker + connector sync +
            // update_dispute_data + outgoing webhook (update_dispute_data writes the
            // connector fields back into the local record and emits the event).
            const std::string key = std::string{"disputes.sync."} + request.get_dispute_id();
            if (!m_context.cache().get(key).hit) {
                m_context.cache().put(
                    key,
                    core::cache::to_cache_value(std::string{"pending"}),
                    core::cache::Ttl{}.seconds(60)
                );
                m_support.call_connector(
                    "sync",
                    request.get_dispute_id(),
                    scope,
                    *dispute,
                    serde::Ser::encode_json(request)
                );
                m_support.update_dispute_data(scope, *dispute, "dispute.retrieve.force_sync");
                m_context.metrics().counter("disputes_force_sync_total").increment();
            } else {
                m_context.metrics().counter("disputes_force_sync_deduped").increment();
            }
        }
        response = m_support.to_response(*dispute);

        // core/disputes.rs:166-177 — validation_result.is_err() -> is_already_refunded.
        response.set_is_already_refunded(m_support.is_already_refunded(db, *dispute));
        return RetrieveDisputeResult{response};
    }

private:
    app::context::AppContext& m_context;
    DisputesRuntimeSupport m_support;
};

} // namespace app::handlers::disputes
