export module app_handlers_disputes:runtime_delete_dispute_evidence;

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

class DisputesDeleteDisputeEvidenceRuntime
{
public:
    explicit DisputesDeleteDisputeEvidenceRuntime(app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] std::expected<DeleteDisputeEvidenceResult, Error>
    delete_dispute_evidence(const DeleteDisputeEvidenceInput& input) const
    {
        m_context.metrics().record_flow("dispute_delete_evidence");
        const auto& scope = input.scope;
        const auto& req = input.request;
        auto dispute =
            m_support.find_dispute_by_processor_merchant_id_dispute_id(scope, req.get_dispute_id());
        if (!dispute) {
            return std::unexpected{m_support.dispute_not_found(req.get_dispute_id())};
        }
        if (const auto invalid = m_support.validate_profile_id_from_auth_layer(scope, *dispute)) {
            return std::unexpected{*invalid};
        }

        m_context.metrics().counter("disputes_delete_evidence_total").increment();
        const std::string lock_key = std::string{"dispute:delete:"} + req.get_dispute_id();
        const auto lock =
            m_context.locks().acquire(lock_key, std::int64_t{30}, "dispute_delete_evidence");
        if (!lock) {
            return std::unexpected{core::error::AppErrorBuilder{}(
                Kind::DUPLICATE_REQUEST,
                "dispute evidence delete already in progress"
            )};
        }
        // transformers::delete_evidence_file + DisputeUpdate::EvidenceUpdate.
        auto evidence = m_support.delete_evidence_file(
            m_support.get_dispute_evidence_vec(req.get_dispute_id()),
            req.get_evidence_type()
        );
        m_context.config_store()
            .put("dispute_evidence", req.get_dispute_id(), m_support.encode_evidence(evidence));
        m_context.repository().update_dispute(*dispute);
        m_support.notify_dispute(*dispute, "dispute.evidence_deleted");
        m_context.locks().release(lock_key);
        return DeleteDisputeEvidenceResult{"success"};
    }

private:
    app::context::AppContext& m_context;
    DisputesRuntimeSupport m_support;
};

} // namespace app::handlers::disputes
