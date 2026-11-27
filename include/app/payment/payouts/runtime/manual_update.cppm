export module app_payment:payouts_runtime_manual_update;
import core_behavior;

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

class PayoutsManualUpdateRuntime
{
public:
    explicit PayoutsManualUpdateRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    // ───────────────────────── Core: manual_update ─────────────────────────
    template<class Dto>
    [[nodiscard]] core::behavior::Outcome<::app::model::payouts::PayoutsManualUpdateResponse>
    manual_update(const Dto& dto, const core::behavior::CallContext& ctx) const
    {
        const auto input = PayoutsManualUpdateData{dto, ctx};
        // `PayoutsManualUpdateRequest::is_update_parameter_present`
        // (`api_models/payouts.rs:1931-1938`).
        if (!input.request.get_status().has_value() &&
            !input.request.get_error_code().has_value() &&
            !input.request.get_error_message().has_value() &&
            !input.request.get_connector_payout_id().has_value()) {
            return std::unexpected{m_support.make_error(
                core::error::AppErrorKind::VALIDATION_FAILED,
                "Request must contain atleast one parameter to update"
            )};
        }

        PayoutLockGuard lock{
            m_context.locks(),
            std::format("payout:{}", input.payout_id),
            "payouts_manual_update"
        };
        if (!lock.held()) {
            return std::unexpected{m_support.make_error(
                core::error::AppErrorKind::PRECONDITION_FAILED,
                "payout is locked by another request"
            )};
        }

        m_context.logger().record_flow("payout_manual_update");
        m_context.logger().counter("payouts_manual_update_total").increment();

        auto found = m_support.find_payout_record(input.payout_id);
        if (!found.has_value()) {
            return std::unexpected{
                m_support.make_error(core::error::AppErrorKind::NOT_FOUND, "payout not found")
            };
        }
        auto record = *found;
        const auto attempt_status =
            input.request.get_status()
                ? m_support.normalize_status(input.request.get_status()->get_value())
                : std::string{};
        const std::string attempt_id = input.request.get_payout_attempt_id();
        std::string final_attempt_status = attempt_status;

        if (!attempt_id.empty()) {
            const auto existing = m_context.repository().find_payout_attempt_by_string(attempt_id);
            if (existing.has_value()) {
                auto attempt = *existing;
                if (!attempt_status.empty()) {
                    attempt.set_status(attempt_status);
                }
                if (input.request.get_error_code()) {
                    attempt.set_error_code(*input.request.get_error_code());
                }
                if (input.request.get_error_message()) {
                    attempt.set_error_message(*input.request.get_error_message());
                }
                m_context.repository().update_payout_attempt(attempt);
                if (final_attempt_status.empty()) {
                    final_attempt_status = attempt.get_status();
                }
            } else {
                m_support.record_payout_attempt(
                    record.get_payout_id(),
                    record.get_merchant_id(),
                    input.request.get_connector_payout_id().value_or(""),
                    attempt_status,
                    record.get_amount(),
                    record.get_currency(),
                    input.request.get_error_code().value_or(""),
                    input.request.get_error_message().value_or("")
                );
            }
        }

        // Upstream updates the payout status only when the manual-updated attempt is the
        // active attempt (`payouts_manual_update_core:4633-4651`).
        if (!attempt_status.empty() &&
            attempt_id == m_support.payout_attempt_id(record.get_payout_id())) {
            record.set_status(attempt_status);
            m_context.repository().update_payout(record);
        }
        m_support.emit_payout_webhook("manual_updated", record);

        ::app::model::payouts::PayoutsManualUpdateResponse response{};
        ::app::model::payouts::PayoutIdValue payout_value{};
        payout_value.set_value(record.get_payout_id());
        response.set_payout_id(payout_value);
        response.set_payout_attempt_id(attempt_id);
        ::app::model::payouts::MerchantIdValue merchant_value{};
        merchant_value.set_value(record.get_merchant_id());
        response.set_merchant_id(merchant_value);
        if (!final_attempt_status.empty()) {
            ::app::model::payouts::PayoutStatusValue status_value{};
            status_value.set_value(final_attempt_status);
            response.set_attempt_status(status_value);
        }
        response.set_error_code(input.request.get_error_code());
        response.set_error_message(input.request.get_error_message());
        response.set_connector_payout_id(input.request.get_connector_payout_id());

        return ::app::model::payouts::PayoutsManualUpdateResponse{response};
    }

private:
    ::app::context::AppContext& m_context;
    PayoutsRuntimeSupport m_support;
};

} // namespace app::handlers::payouts
