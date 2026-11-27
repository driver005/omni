export module app_payment:disputes_runtime_accept_dispute;

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

class DisputesAcceptDisputeRuntime
{
public:
    explicit DisputesAcceptDisputeRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] std::expected<::app::model::disputes::DisputeResponse, Error>
    accept_dispute(const AcceptDisputeInput& input) const
    {
        m_context.metrics().record_flow("dispute_accept");
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
        const std::string stage = m_support.canonical_stage(dispute->get_stage());
        const std::string status = m_support.canonical_status(dispute->get_status());
        if (!m_support.should_proceed_with_accept_dispute(stage, status)) {
            return std::unexpected{m_support.status_validation_failed(
                std::format(
                    "This dispute cannot be accepted because the dispute is in {} stage and has {} "
                    "status",
                    stage,
                    status
                )
            )};
        }

        m_context.metrics().counter("disputes_accept_total").increment();
        const std::string lock_key = std::string{"dispute:accept:"} + req.get_dispute_id();
        const auto lock = m_context.locks().acquire(lock_key, std::int64_t{30}, "dispute_accept");
        if (!lock) {
            return std::unexpected{core::error::AppErrorBuilder{}(
                Kind::DUPLICATE_REQUEST,
                "dispute accept already in progress"
            )};
        }

        // construct_accept_dispute_router_data + UCS routing + gateway execution are
        // represented by the local connector router (execute_dispute_gateway substitute).
        m_support.execute_dispute_gateway(
            "accept_dispute",
            req.get_dispute_id(),
            scope,
            *dispute,
            m_support.query_json({}, "dispute_id", req.get_dispute_id())
        );
        // core/disputes.rs:538-552 — DisputeUpdate::StatusUpdate.
        dispute->set_status("dispute_accepted");
        m_context.repository().update_dispute(*dispute);
        m_support.notify_dispute(*dispute, "dispute.accepted");
        m_context.locks().release(lock_key);
        return ::app::model::disputes::DisputeResponse{m_support.to_response(*dispute)};
    }

private:
    ::app::context::AppContext& m_context;
    DisputesRuntimeSupport m_support;
};

} // namespace app::handlers::disputes
