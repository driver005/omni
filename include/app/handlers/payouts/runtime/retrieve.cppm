export module app_handlers_payouts:runtime_retrieve;

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

export namespace app::handlers::payouts {

class PayoutsRetrieveRuntime
{
public:
    explicit PayoutsRetrieveRuntime(app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    // ───────────────────────── Core: retrieve ─────────────────────────
    [[nodiscard]] std::expected<PayoutsRetrieveResult, core::error::AppError>
    retrieve(const PayoutsRetrieveInput& input) const
    {
        m_context.metrics().record_flow("payout_retrieve");
        m_context.metrics().counter("payouts_retrieve_total").increment();

        auto found = m_support.find_payout_record(input.payout_id);
        if (!found.has_value()) {
            return std::unexpected{
                m_support.make_error(core::error::AppErrorKind::NOT_FOUND, "payout not found")
            };
        }
        auto record = *found;

        // Profile scope from the authenticated principal (`payouts_retrieve_core:928-940`).
        if (!input.profile_id.empty() &&
            m_support.meta_value(record.get_metadata(), "profile_id") != input.profile_id) {
            return std::unexpected{
                m_support.make_error(core::error::AppErrorKind::NOT_FOUND, "payout not found")
            };
        }

        if (input.force_sync && m_support.should_call_retrieve(record.get_status())) {
            auto sync_request = app::connector::ConnectorRequestBuilder{}
                                    .operation("payout_sync")
                                    .merchant_id(record.get_merchant_id())
                                    .amount(record.get_amount())
                                    .currency(record.get_currency())
                                    .resource_id(record.get_payout_id())();
            if (auto connector = m_context.connector_router().select(record.get_merchant_id())) {
                const auto outcome = m_support.invoke_connector(*connector, "sync", sync_request);
                if (outcome.error) {
                    return std::unexpected{*outcome.error};
                }
                if (!outcome.status.empty()) {
                    record.set_status(m_support.status_from_connector(outcome.status, false));
                }
                m_context.repository().update_payout(record);
                m_support
                    .update_latest_attempt(record.get_payout_id(), record.get_status(), {}, {});
                m_support.emit_payout_webhook("synced", record);
            } else {
                m_context.metrics().counter("payouts_connector_unavailable").increment();
            }
        }

        auto response = m_support.response_from_record(record);
        m_support.enrich_response(response, record, input.expand_attempts);
        return PayoutsRetrieveResult{response};
    }

private:
    app::context::AppContext& m_context;
    PayoutsRuntimeSupport m_support;
};

} // namespace app::handlers::payouts
