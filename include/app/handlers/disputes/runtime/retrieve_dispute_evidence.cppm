export module app_handlers_disputes:runtime_retrieve_dispute_evidence;

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

class DisputesRetrieveDisputeEvidenceRuntime
{
public:
    explicit DisputesRetrieveDisputeEvidenceRuntime(app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] std::expected<RetrieveDisputeEvidenceResult, Error>
    retrieve_dispute_evidence(const RetrieveDisputeEvidenceInput& input) const
    {
        m_context.metrics().record_flow("dispute_retrieve_evidence");
        const auto& scope = input.scope;
        const auto& req = input.dispute_id;
        auto dispute =
            m_support.find_dispute_by_processor_merchant_id_dispute_id(scope, req.get_dispute_id());
        if (!dispute) {
            return std::unexpected{m_support.dispute_not_found(req.get_dispute_id())};
        }
        if (const auto invalid = m_support.validate_profile_id_from_auth_layer(scope, *dispute)) {
            return std::unexpected{*invalid};
        }
        return RetrieveDisputeEvidenceResult{
            m_support.get_dispute_evidence_vec(req.get_dispute_id())
        };
    }

private:
    app::context::AppContext& m_context;
    DisputesRuntimeSupport m_support;
};

} // namespace app::handlers::disputes
