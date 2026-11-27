export module app_payment:payouts_runtime_cancel;

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

import :payouts_input;
import :payouts_result;
import :payouts_runtime_common;

export namespace app::handlers::payouts {

class PayoutsCancelRuntime
{
public:
    explicit PayoutsCancelRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    // ───────────────────────── Core: cancel ─────────────────────────
    [[nodiscard]] std::expected<::app::model::payouts::PayoutCreateResponse, core::error::AppError>
    cancel(const PayoutsCancelInput& input) const
    {
        PayoutLockGuard lock{
            m_context.locks(),
            std::format("payout:{}", input.payout_id),
            "payouts_cancel"
        };
        if (!lock.held()) {
            return std::unexpected{m_support.make_error(
                core::error::AppErrorKind::PRECONDITION_FAILED,
                "payout is locked by another request"
            )};
        }

        m_context.metrics().record_flow("payout_cancel");
        m_context.metrics().counter("payouts_cancel_total").increment();

        auto found = m_support.find_payout_record(input.payout_id);
        if (!found.has_value()) {
            return std::unexpected{
                m_support.make_error(core::error::AppErrorKind::NOT_FOUND, "payout not found")
            };
        }
        auto record = *found;
        if (m_support.is_cancel_terminal(record.get_status())) {
            return std::unexpected{m_support.make_error(
                core::error::AppErrorKind::INVALID_REQUEST,
                std::format(
                    "Payout {:?} cannot be cancelled for status {}",
                    record.get_payout_id(),
                    m_support.normalize_status(record.get_status())
                )
            )};
        }

        // Upstream local cancellation requires both an eligible status and a connector
        // that supports instant payout (`core/payouts.rs:1008-1012`).
        if (m_support.is_eligible_for_local_payout_cancellation(record.get_status()) &&
            m_support.supports_instant_payout(record.get_connector(), record.get_payout_type())) {
            record.set_status(std::string{kCancelled});
            record.set_metadata(
                m_support.merge_meta_note(record.get_metadata(), "Cancelled by user")
            );
            m_context.repository().update_payout(record);
            // Upstream sets `error_message: Some("Cancelled by user")`, `error_code: None`
            // (`core/payouts.rs:1019-1020`).
            m_support
                .update_latest_attempt(record.get_payout_id(), kCancelled, {}, "Cancelled by user");
        } else {
            auto cancel_request = ::app::connector::ConnectorRequestBuilder{}
                                      .operation("payout_cancel")
                                      .merchant_id(record.get_merchant_id())
                                      .amount(record.get_amount())
                                      .currency(record.get_currency())
                                      .resource_id(record.get_payout_id())();
            auto connector = m_context.connector_router().select(record.get_merchant_id());
            if (!connector) {
                // No connector backend registered locally: apply the deterministic local
                // cancellation path rather than failing the request.
                record.set_status(std::string{kCancelled});
            } else {
                const auto outcome =
                    m_support.invoke_connector(*connector, "cancel", cancel_request);
                if (outcome.error) {
                    return std::unexpected{*outcome.error};
                }
                record.set_status(
                    outcome.status.empty() ? std::string{kCancelled}
                                           : m_support.status_from_connector(outcome.status, false)
                );
            }
            m_context.repository().update_payout(record);
            m_support.update_latest_attempt(
                record.get_payout_id(),
                record.get_status(),
                {},
                "Cancelled by user"
            );
        }
        m_support.emit_payout_webhook("cancelled", record);

        auto response = m_support.response_from_record(record);
        m_support.enrich_response(response, record, false);
        response.set_error_message(std::string{"Cancelled by user"});
        return ::app::model::payouts::PayoutCreateResponse{response};
    }

private:
    ::app::context::AppContext& m_context;
    PayoutsRuntimeSupport m_support;
};

} // namespace app::handlers::payouts
