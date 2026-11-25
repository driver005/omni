export module app_handlers_payouts:runtime_confirm;

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

class PayoutsConfirmRuntime
{
public:
    explicit PayoutsConfirmRuntime(app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    // ───────────────────────── Core: confirm ─────────────────────────
    [[nodiscard]] std::expected<PayoutsConfirmResult, core::error::AppError>
    confirm(const PayoutsConfirmInput& input) const
    {
        PayoutLockGuard lock{
            m_context.locks(),
            std::format("payout:{}", input.payout_id),
            "payouts_confirm"
        };
        if (!lock.held()) {
            return std::unexpected{m_support.make_error(
                core::error::AppErrorKind::PRECONDITION_FAILED,
                "payout is locked by another request"
            )};
        }

        m_context.metrics().record_flow("payout_confirm");
        m_context.metrics().counter("payouts_confirm_total").increment();

        auto found = m_support.find_payout_record(input.payout_id);
        if (!found.has_value()) {
            return std::unexpected{
                m_support.make_error(core::error::AppErrorKind::NOT_FOUND, "payout not found")
            };
        }
        auto record = *found;

        // `validate_payout_status_against_not_allowed_statuses` (`core/payouts.rs:791-804`).
        if (m_support.is_confirm_disallowed(record.get_status())) {
            return std::unexpected{m_support.make_error(
                core::error::AppErrorKind::PRECONDITION_FAILED,
                std::format(
                    "You cannot confirm this payout because it has status {}",
                    m_support.normalize_status(record.get_status())
                )
            )};
        }

        auto connector_request = app::connector::ConnectorRequestBuilder{}
                                     .operation("payout")
                                     .merchant_id(record.get_merchant_id())
                                     .amount(record.get_amount())
                                     .currency(record.get_currency())
                                     .resource_id(record.get_payout_id())
                                     .raw_payload(input.body)();
        auto choice = m_support.make_connector_decision(
            connector_request,
            record.get_merchant_id(),
            record.get_payout_id(),
            record.get_amount(),
            record.get_currency(),
            record.get_payout_type(),
            input.request.has_value()
                ? m_support.first_requested_connector(input.request->connector)
                : std::string{}
        );
        if (choice.error) {
            return std::unexpected{*choice.error};
        }
        if (choice.connector) {
            const auto outcome =
                m_support.invoke_connector(*choice.connector, "authorize", connector_request);
            if (outcome.error) {
                // Connector failure keeps the payout in a failed state with mapped error metadata.
                record.set_status(std::string{kFailed});
                m_context.repository().update_payout(record);
                m_support.update_latest_attempt(
                    record.get_payout_id(),
                    kFailed,
                    "CONNECTOR_ERROR",
                    outcome.error->get_message()
                );
                m_support.emit_payout_webhook("failed", record);
                return std::unexpected{*outcome.error};
            }
            record.set_status(m_support.status_from_connector(
                outcome.status,
                input.request.has_value() && input.request->auto_fulfill.value_or(false)
            ));
            record.set_connector(std::string{choice.connector->name()});
        } else {
            record.set_status(std::string{kRequiresConfirmation});
        }
        record.set_metadata(m_support.merge_meta_payout_link(record.get_metadata(), "Submitted"));
        m_context.repository().update_payout(record);
        m_support.update_latest_attempt(record.get_payout_id(), record.get_status(), {}, {});
        m_support.emit_payout_webhook("confirmed", record);

        auto response = m_support.response_from_record(record);
        m_support.enrich_response(response, record, false);
        if (input.request.has_value()) {
            if (input.request->merchant_order_reference_id) {
                response.set_merchant_order_reference_id(
                    *input.request->merchant_order_reference_id
                );
            }
            if (input.request->description) {
                response.set_description(*input.request->description);
            }
        }
        return PayoutsConfirmResult{response};
    }

private:
    app::context::AppContext& m_context;
    PayoutsRuntimeSupport m_support;
};

} // namespace app::handlers::payouts
