export module app_payment:disputes_runtime_fetch_disputes;

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

class DisputesFetchDisputesRuntime
{
public:
    explicit DisputesFetchDisputesRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    template<class Dto>
    [[nodiscard]] core::behavior::Outcome<std::vector<::app::model::disputes::DisputeResponse>>
        fetch_disputes(const Dto& dto, const core::behavior::CallContext& ctx) const
    {
        const auto input = FetchDisputesData{dto, ctx};
        m_context.metrics().record_flow("dispute_fetch");
        m_context.metrics().counter("disputes_fetch_total").increment();
        const auto& scope = input.scope;
        const auto& merchant_connector_id = input.connector_id;
        const auto& payload = input.payload;
        // core/disputes.rs:997-1119 — MerchantConnectorAccount lookup, connector
        // fetch, per-dispute payment-attempt lookup and process-tracker scheduling.
        const auto mca =
            m_context.repository().find_merchant_connector_by_string(merchant_connector_id);
        if (!mca) {
            m_context.metrics().counter("disputes_mca_not_found").increment();
            return std::unexpected{core::error::AppErrorBuilder{}(
                Kind::NOT_FOUND,
                std::string{merchant_connector_id} + " does not exist"
            )};
        }
        if (const auto invalid = m_support.validate_profile_id_from_auth_mca(scope, *mca)) {
            return std::unexpected{*invalid};
        }
        const std::string connector_name = mca->get_connector_name();

        Record probe{};
        probe.set_dispute_id(std::string{merchant_connector_id});
        probe.set_connector(connector_name);
        probe.set_merchant_id(scope.merchant_id);
        probe.set_profile_id(scope.profile_id);
        const auto raw = std::format(
            "{{\"fetch_from\":\"{}\",\"fetch_till\":\"{}\"}}",
            payload.get_fetch_from(),
            payload.get_fetch_till()
        );
        m_support.call_connector("fetch_disputes", merchant_connector_id, scope, probe, raw);

        std::vector<::app::model::disputes::DisputeResponse> result{};
        for (const auto& record: m_context.repository().list_disputes()) {
            if (record.get_connector() != connector_name) {
                continue;
            }
            if (!scope.profile_id.empty() && !record.get_profile_id().empty() &&
                record.get_profile_id() != scope.profile_id) {
                continue;
            }
            if (!m_support.attempt_id_for(record.get_payment_id()).empty()) {
                (void)m_support.add_process_dispute_task_to_pt(
                    connector_name,
                    record,
                    scope,
                    payload.get_fetch_till()
                );
            }
            result.push_back(m_support.to_response(record));
        }
        return std::vector<::app::model::disputes::DisputeResponse>{std::move(result)};
    }

private:
    ::app::context::AppContext& m_context;
    DisputesRuntimeSupport m_support;
};

} // namespace app::handlers::disputes
