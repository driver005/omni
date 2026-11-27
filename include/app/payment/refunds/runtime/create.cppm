module;
#include <uuid.h>

export module app_payment:refunds_runtime_create;
import core_behavior;

import std;
import interfaces;
import core_router;
import core_serde;
import utils_openapi;
import app_model;
import app_dto;
import app_context;
import app_context_core;
import core_cache;
import core_error;
import app_auth;
import app_store;
import app_connector;
import app_http;
import app_crypto;
import app_metrics;
import app_locking;
import app_events;
import app_workflow;
import app_services;

import :refunds_input;
import :refunds_result;
import :refunds_runtime_common;

export namespace app::handlers::refunds {

class RefundCreateRuntime
{
public:
    explicit RefundCreateRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] core::behavior::Outcome<::app::model::Refund>
    create(const ::app::model::Refund& dto, const core::behavior::CallContext& ctx) const
    {
        const auto input = RefundsCreateData{dto, ctx};
        // routes/refunds.rs:16-32,146 — serialize creation against the same payment.
        const std::string lock_key{"refund:" + input.payment_id};
        const auto lock = m_context.locks().acquire(lock_key, std::int64_t{30}, "refund_create");
        if (!lock) {
            m_context.logger().counter("refunds_lock_contended").increment();
            return std::unexpected{m_support.err(
                Kind::DUPLICATE_REQUEST,
                "a refund is already being processed for this payment"
            )};
        }
        if (input.is_v2) {
            auto result = create_v2(input.v2_payload.value(), input.facts);
            m_context.locks().release(lock_key);
            return result;
        }
        auto result = create_v1(input.v1_payload.value(), input.facts);
        m_context.locks().release(lock_key);
        return result;
    }

private:
    std::expected<::app::model::Refund, core::error::AppError>
    create_v1(const ::app::model::Refund& payload, const RefundRequestFacts& facts) const
    {
        m_context.logger().record_flow("refund_create");
        m_context.logger().counter("refunds_create_total").increment();

        const std::string payment_id{uuids::to_string(payload.payment_id)};
        const auto intent = m_support.find_payment_intent(payment_id);
        if (!intent) {
            return std::unexpected{m_support.err(Kind::NOT_FOUND, "PaymentNotFound")};
        }

        // core/refunds.rs:426 — validate_profile_id_from_auth_layer.
        if (const auto e =
                m_support.validate_profile_id_from_auth_layer(facts.ctx_profile_id, *intent)) {
            return std::unexpected{*e};
        }
        // core/refunds.rs:428-440 — status gate.
        if (const auto e = m_support.validate_refundable_status(*intent)) {
            return std::unexpected{*e};
        }
        // core/refunds.rs:442-447 — amount against intent state metadata.
        const std::optional<std::int64_t> requested =
            payload.amount != 0 ? std::optional<std::int64_t>{payload.amount} : std::nullopt;
        if (const auto e =
                m_support.validate_amount_against_intent_state_metadata(*intent, requested)) {
            return std::unexpected{*e};
        }
        // core/refunds.rs:449 — prevent_refund_after_post_capture_void.
        if (const auto e = m_support.prevent_refund_after_post_capture_void(*intent)) {
            return std::unexpected{*e};
        }

        // core/refunds.rs:451-465 — req.amount.or(amount_captured), positive check.
        const std::int64_t amount = requested.value_or(intent->get_amount());
        if (amount <= 0) {
            return std::unexpected{
                m_support.err(Kind::INVALID_REQUEST, "amount less than or equal to zero")
            };
        }
        // core/refunds.rs:467-474 — last successful/partially-captured attempt.
        const auto attempt = m_support.find_last_successful_attempt(payment_id);
        if (!attempt) {
            return std::unexpected{m_support.err(Kind::NOT_FOUND, "SuccessfulPaymentNotFound")};
        }
        // core/refunds.rs:476-487 — creds identifier + insert MCD into config.
        std::string creds_identifier{};
        if (!payload.merchant_connector_details.empty()) {
            const auto& value = payload.merchant_connector_details;
            creds_identifier = m_support.extract_creds_identifier(value);
            m_support.insert_merchant_connector_creds_to_config(value);
        }

        // core/refunds.rs:489-499 — validate_and_create_refund.
        auto result = m_support.validate_and_create_refund(
            uuids::to_string(payload.payment_id),
            intent.value(),
            attempt.value(),
            amount,
            creds_identifier,
            payload.refund_id.empty() ? std::optional<std::string>{}
                                      : std::optional<std::string>{payload.refund_id},
            std::optional<std::string>{payload.merchant_id},
            payload.reason.empty() ? std::optional<std::string>{}
                                   : std::optional<std::string>{payload.reason},
            payload.metadata.empty() ? std::nullopt : std::optional<std::string>{payload.metadata},
            payload.split_refunds.empty() ? std::nullopt
                                          : std::optional<std::string>{payload.split_refunds},
            payload.refund_type.empty() ? std::optional<std::string>{}
                                        : std::optional<std::string>{payload.refund_type},
            payload.feature_metadata.empty() ? std::nullopt
                                             : std::optional<std::string>{payload.feature_metadata},
            payload.all_keys_required,
            payload.payment_method_data.empty()
                ? std::nullopt
                : std::optional<std::string>{payload.payment_method_data},
            facts
        );
        if (!result) {
            return std::unexpected{result.error()};
        }
        return m_support.to_refund_response(*result);
    }

    std::expected<::app::model::Refund, core::error::AppError>
    create_v2(const ::app::model::Refund& payload, const RefundRequestFacts& facts) const
    {
        m_context.logger().record_flow("refund_create");
        m_context.logger().counter("refunds_create_total").increment();

        const std::string payment_id{uuids::to_string(payload.payment_id)};
        const auto intent = m_support.find_payment_intent(payment_id);
        if (!intent) {
            return std::unexpected{m_support.err(Kind::NOT_FOUND, "PaymentNotFound")};
        }
        // core/refunds_v2.rs:63-65 — profile from auth must equal intent profile.
        if (!facts.ctx_profile_id.empty() &&
            facts.ctx_profile_id != m_support.ext_of(*intent).profile_id) {
            return std::unexpected{m_support.err(Kind::NOT_FOUND, "PaymentNotFound")};
        }
        if (const auto e = m_support.validate_refundable_status(*intent)) {
            return std::unexpected{*e};
        }
        const auto attempt = m_support.find_last_successful_attempt(payment_id);
        if (!attempt) {
            return std::unexpected{m_support.err(Kind::NOT_FOUND, "SuccessfulPaymentNotFound")};
        }
        // core/refunds_v2.rs:94-102 — no connector account => RefundNotPossible.
        if (attempt->get_connector().empty()) {
            return std::unexpected{m_support.err(Kind::PRECONDITION_FAILED, "RefundNotPossible")};
        }
        const std::optional<std::int64_t> requested =
            payload.amount != 0 ? std::optional<std::int64_t>{payload.amount} : std::nullopt;
        const std::int64_t amount = requested.value_or(intent->get_amount());
        if (amount <= 0) {
            return std::unexpected{
                m_support.err(Kind::INVALID_REQUEST, "amount less than or equal to zero")
            };
        }
        std::string creds_identifier{};
        if (!payload.merchant_connector_details.empty()) {
            const auto& value = payload.merchant_connector_details;
            creds_identifier = m_support.extract_creds_identifier(value);
            m_support.insert_merchant_connector_creds_to_config(value);
        }
        // v2 always generates a GlobalRefundId (routes/refunds.rs:135-144).
        const std::string generated = m_context.ids().payment_id("rf_");

        auto result = m_support.validate_and_create_refund(
            payment_id,
            intent.value(),
            attempt.value(),
            amount,
            creds_identifier,
            std::optional<std::string>{generated},
            std::nullopt,
            payload.reason.empty() ? std::optional<std::string>{}
                                   : std::optional<std::string>{payload.reason},
            payload.metadata.empty() ? std::nullopt : std::optional<std::string>{payload.metadata},
            std::nullopt,
            payload.refund_type.empty() ? std::optional<std::string>{}
                                        : std::optional<std::string>{payload.refund_type},
            payload.feature_metadata.empty() ? std::nullopt
                                             : std::optional<std::string>{payload.feature_metadata},
            std::nullopt,
            std::nullopt,
            facts
        );
        if (!result) {
            return std::unexpected{result.error()};
        }
        return m_support.to_refund_response(*result);
    }

    ::app::context::AppContext& m_context;
    RefundRuntimeSupport m_support;
};

} // namespace app::handlers::refunds
