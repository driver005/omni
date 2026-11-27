export module app_payment:payouts_runtime_update;

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

class PayoutsUpdateRuntime
{
public:
    explicit PayoutsUpdateRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    // ───────────────────────── Core: update ─────────────────────────
    [[nodiscard]] std::expected<::app::model::payouts::PayoutCreateResponse, core::error::AppError>
    update(const PayoutsUpdateInput& input) const
    {
        PayoutLockGuard lock{
            m_context.locks(),
            std::format("payout:{}", input.payout_id),
            "payouts_update"
        };
        if (!lock.held()) {
            return std::unexpected{m_support.make_error(
                core::error::AppErrorKind::PRECONDITION_FAILED,
                "payout is locked by another request"
            )};
        }

        m_context.metrics().record_flow("payout_update");
        m_context.metrics().counter("payouts_update_total").increment();

        auto found = m_support.find_payout_record(input.payout_id);
        if (!found.has_value()) {
            return std::unexpected{
                m_support.make_error(core::error::AppErrorKind::NOT_FOUND, "payout not found")
            };
        }
        auto record = *found;

        // Upstream `payouts_update_core:864-872`: terminal or already-initiated payouts
        // cannot be updated (`InvalidRequestData`).
        if (m_support.is_payout_terminal_state(record.get_status()) ||
            m_support.is_payout_initiated(record.get_status())) {
            return std::unexpected{m_support.make_error(
                core::error::AppErrorKind::INVALID_REQUEST,
                std::format(
                    "Payout {} cannot be updated for status {}",
                    record.get_payout_id(),
                    m_support.normalize_status(record.get_status())
                )
            )};
        }

        if (!input.header_merchant_id.empty()) {
            record.set_merchant_id(input.header_merchant_id);
        } else if (input.request.merchant_id) {
            record.set_merchant_id(input.request.merchant_id->get_value());
        }

        std::int64_t amount{record.get_amount()};
        if (input.request.amount) {
            const auto& amount_text = input.request.amount->get_value();
            std::int64_t decoded{0};
            const auto result = std::from_chars(
                amount_text.data(),
                amount_text.data() + amount_text.size(),
                decoded
            );
            if (result.ec == std::errc{}) {
                amount = decoded;
            }
        }
        if (amount <= 0) {
            return std::unexpected{m_support.make_error(
                core::error::AppErrorKind::PRECONDITION_FAILED,
                "payout amount must be positive"
            )};
        }
        record.set_amount(amount);

        if (input.request.currency) {
            record.set_currency(input.request.currency->get_value());
        }
        if (input.request.payout_type) {
            record.set_payout_type(input.request.payout_type->get_value());
        }
        if (input.request.customer_id) {
            record.set_customer_id(input.request.customer_id->get_value());
        }

        auto connector_request = ::app::connector::ConnectorRequestBuilder{}
                                     .operation("payout")
                                     .merchant_id(record.get_merchant_id())
                                     .amount(record.get_amount())
                                     .currency(record.get_currency())
                                     .resource_id(record.get_payout_id())
                                     .raw_payload(input.body)();
        if (input.request.confirm.value_or(false)) {
            auto choice = m_support.make_connector_decision(
                connector_request,
                record.get_merchant_id(),
                record.get_payout_id(),
                record.get_amount(),
                record.get_currency(),
                record.get_payout_type(),
                m_support.first_requested_connector(input.request.connector)
            );
            if (choice.error) {
                return std::unexpected{*choice.error};
            }
            if (choice.connector) {
                const auto outcome =
                    m_support.invoke_connector(*choice.connector, "authorize", connector_request);
                if (outcome.error) {
                    return std::unexpected{*outcome.error};
                }
                if (!outcome.status.empty()) {
                    record.set_status(m_support.status_from_connector(
                        outcome.status,
                        input.request.auto_fulfill.value_or(false)
                    ));
                }
                record.set_connector(std::string{choice.connector->name()});
                m_support
                    .update_latest_attempt(record.get_payout_id(), record.get_status(), {}, {});
            }
        }

        const auto previous_meta = record.get_metadata();
        record.set_metadata(m_support.merge_meta_browser_info(
            m_support.merge_meta_profile(previous_meta, input.profile_id),
            input.browser_info
        ));

        m_context.repository().update_payout(record);
        m_context.cache().put(
            "payouts.updated." + record.get_payout_id(),
            core::cache::to_cache_value(record.get_status())
        );
        m_support.emit_payout_webhook("updated", record);

        auto response = m_support.response_from_record(record);
        m_support.enrich_response(response, record, false);
        if (input.request.merchant_order_reference_id) {
            response.set_merchant_order_reference_id(*input.request.merchant_order_reference_id);
        }
        if (input.request.description) {
            response.set_description(*input.request.description);
        }

        return ::app::model::payouts::PayoutCreateResponse{response};
    }

private:
    ::app::context::AppContext& m_context;
    PayoutsRuntimeSupport m_support;
};

} // namespace app::handlers::payouts
