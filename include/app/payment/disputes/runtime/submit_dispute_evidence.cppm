export module app_payment:disputes_runtime_submit_dispute_evidence;

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

import :disputes_input;
import :disputes_result;
import :disputes_runtime_common;

export namespace app::handlers::disputes {

class DisputesSubmitDisputeEvidenceRuntime
{
public:
    explicit DisputesSubmitDisputeEvidenceRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] std::expected<::app::model::disputes::DisputeResponse, Error>
    submit_dispute_evidence(const SubmitDisputeEvidenceInput& input) const
    {
        m_context.metrics().record_flow("dispute_submit_evidence");
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
        const std::string stage = m_support.canonical_stage(dispute->get_stage());
        const std::string status = m_support.canonical_status(dispute->get_status());
        if (!m_support.should_proceed_with_submit_evidence(stage, status)) {
            return std::unexpected{m_support.status_validation_failed(
                std::format(
                    "Evidence cannot be submitted because the dispute is in {} stage and has {} "
                    "status",
                    stage,
                    status
                )
            )};
        }

        m_context.metrics().counter("disputes_submit_evidence_total").increment();
        const std::string lock_key = std::string{"dispute:evidence:"} + req.get_dispute_id();
        const auto lock =
            m_context.locks().acquire(lock_key, std::int64_t{30}, "dispute_submit_evidence");
        if (!lock) {
            return std::unexpected{core::error::AppErrorBuilder{}(
                Kind::DUPLICATE_REQUEST,
                "dispute evidence update already in progress"
            )};
        }

        // transformers::get_evidence_request_data — resolve each uploaded file id.
        const auto evidence = m_support.get_evidence_request_data(req, *dispute);
        const bool defend = m_support.requires_defend_dispute(dispute->get_connector());
        auto raw = evidence;
        if (defend && !raw.empty() && raw.back() == '}') {
            raw.pop_back();
            raw += ",\"defend\":true}";
        }
        const auto response = m_support.execute_dispute_gateway(
            defend ? "defend_dispute" : "submit_evidence",
            req.get_dispute_id(),
            scope,
            *dispute,
            raw
        );
        // core/disputes.rs:743-752 — DisputeUpdate::StatusUpdate from connector response.
        dispute->set_status(defend ? "dispute_challenged" : "dispute_won");
        m_context.config_store().put(
            "dispute_connector_status",
            req.get_dispute_id(),
            response.get_status().empty() ? dispute->get_status() : response.get_status()
        );
        m_context.repository().update_dispute(*dispute);
        m_support.notify_dispute(*dispute, "dispute.evidence_submitted");
        m_context.locks().release(lock_key);
        return ::app::model::disputes::DisputeResponse{m_support.to_response(*dispute)};
    }

private:
    ::app::context::AppContext& m_context;
    DisputesRuntimeSupport m_support;
};

} // namespace app::handlers::disputes
