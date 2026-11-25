export module app_handlers_payouts:runtime_fulfill;

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

class PayoutsFulfillRuntime
{
public:
    explicit PayoutsFulfillRuntime(app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    // ───────────────────────── Core: fulfill ─────────────────────────
    [[nodiscard]] std::expected<PayoutsFulfillResult, core::error::AppError>
    fulfill(const PayoutsFulfillInput& input) const
    {
        PayoutLockGuard lock{
            m_context.locks(),
            std::format("payout:{}", input.payout_id),
            "payouts_fulfill"
        };
        if (!lock.held()) {
            return std::unexpected{m_support.make_error(
                core::error::AppErrorKind::PRECONDITION_FAILED,
                "payout is locked by another request"
            )};
        }

        m_context.metrics().record_flow("payout_fulfill");
        m_context.metrics().counter("payouts_fulfill_total").increment();

        auto found = m_support.find_payout_record(input.payout_id);
        if (!found.has_value()) {
            return std::unexpected{
                m_support.make_error(core::error::AppErrorKind::NOT_FOUND, "payout not found")
            };
        }
        auto record = *found;
        if (!m_support.is_fulfill_allowed(record.get_status())) {
            return std::unexpected{m_support.make_error(
                core::error::AppErrorKind::INVALID_REQUEST,
                std::format(
                    "Payout {:?} cannot be fulfilled for status {}",
                    record.get_payout_id(),
                    m_support.normalize_status(record.get_status())
                )
            )};
        }

        auto fulfill_request = app::connector::ConnectorRequestBuilder{}
                                   .operation("payout_fulfill")
                                   .merchant_id(record.get_merchant_id())
                                   .amount(record.get_amount())
                                   .currency(record.get_currency())
                                   .resource_id(record.get_payout_id())();
        auto connector = m_context.connector_router().select(record.get_merchant_id());
        if (connector) {
            const auto outcome = m_support.invoke_connector(*connector, "capture", fulfill_request);
            if (outcome.error) {
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
            record.set_connector(std::string{connector->name()});
        } else {
            m_context.metrics().counter("payouts_connector_unavailable").increment();
        }
        record.set_status(std::string{kSuccess});
        m_context.repository().update_payout(record);
        m_support.update_latest_attempt(record.get_payout_id(), kSuccess, {}, {});
        m_support.emit_payout_webhook("fulfilled", record);

        auto response = m_support.response_from_record(record);
        m_support.enrich_response(response, record, false);
        return PayoutsFulfillResult{response};
    }

private:
    app::context::AppContext& m_context;
    PayoutsRuntimeSupport m_support;
};

} // namespace app::handlers::payouts
