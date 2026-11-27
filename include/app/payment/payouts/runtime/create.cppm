export module app_payment:payouts_runtime_create;

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

class PayoutsCreateRuntime
{
public:
    explicit PayoutsCreateRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    // ───────────────────────── Core: create ─────────────────────────
    template<class Dto>
    [[nodiscard]] core::behavior::Outcome<::app::model::payouts::PayoutCreateResponse>
        create(const Dto& dto, const core::behavior::CallContext& ctx) const
    {
        const auto input = PayoutsCreateData{dto, ctx};
        m_context.metrics().record_flow("payout_create");
        m_context.metrics().counter("payouts_create_total").increment();

        if (const auto invalid =
                m_support.validate_create_input(input.request, input.header_merchant_id)) {
            return std::unexpected{*invalid};
        }

        std::int64_t amount{0};
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
        if (input.request.amount && amount <= 0) {
            return std::unexpected{m_support.make_error(
                core::error::AppErrorKind::PRECONDITION_FAILED,
                "payout amount must be positive"
            )};
        }

        std::string merchant_id =
            input.request.merchant_id ? input.request.merchant_id->get_value() : std::string{};
        if (!input.header_merchant_id.empty()) {
            merchant_id = input.header_merchant_id;
        } else if (merchant_id.empty()) {
            merchant_id = input.auth_merchant_id;
        }
        const auto currency =
            input.request.currency ? input.request.currency->get_value() : std::string{};
        std::string payout_id = input.request.payout_id ? input.request.payout_id->get_value()
                                                        : m_context.ids().payment_id("po_");
        const auto requested_connector =
            m_support.first_requested_connector(input.request.connector);

        if (m_context.repository().find_payout_by_string(payout_id)) {
            return std::unexpected{m_support.make_error(
                core::error::AppErrorKind::ALREADY_EXISTS,
                "payout already exists"
            )};
        }

        std::string payout_link_id{};
        if (input.request.payout_link.value_or(false) ||
            input.request.payout_link_config.has_value()) {
            payout_link_id = m_context.ids().payment_id("pl_");
        }

        std::string note{};
        if (input.request.customer_id && !input.request.customer_id->get_value().empty()) {
            const auto customer = m_context.repository().find_customer_by_string(
                input.request.customer_id->get_value()
            );
            note = customer.has_value() ? "customer_resolved" : "customer_unresolved";
        }

        auto connector_request = ::app::connector::ConnectorRequestBuilder{}
                                     .operation("payout")
                                     .merchant_id(merchant_id)
                                     .amount(amount)
                                     .currency(currency)
                                     .resource_id(payout_id)
                                     .raw_payload(input.body)();

        // Upstream initial status (`core/payouts.rs:3668-3679`): with method data a payout
        // starts in RequiresCreation (confirm) / RequiresConfirmation; otherwise
        // RequiresPayoutMethodData.
        const bool has_method_data =
            input.request.payout_method_data.has_value() ||
            (input.request.payout_token.has_value() && !input.request.payout_token->empty()) ||
            input.request.payout_method_id.has_value() ||
            input.request.source_bank_data.has_value();
        std::string status =
            has_method_data
                ? std::
                      string{input.request.confirm.value_or(false) ? kRequiresCreation : kRequiresConfirmation}
                : std::string{kRequiresPayoutMethodData};
        std::string connector_name{};
        if (input.request.payout_token && !input.request.payout_token->empty()) {
            // External-vault proxy substitute: token payouts park method data in the local
            // key store and keep the upstream pre-confirmation status.
            connector_name = "external_vault";
            note =
                note.empty() ? std::string{"external_vault_token"} : note + ";external_vault_token";
            m_support.store_payout_method_data(payout_id, input.body);
        } else if (input.request.confirm.value_or(false)) {
            auto choice = m_support.make_connector_decision(
                connector_request,
                merchant_id,
                payout_id,
                amount,
                currency,
                input.request.payout_type ? input.request.payout_type->get_value() : std::string{},
                requested_connector
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
                status = m_support.status_from_connector(
                    outcome.status,
                    input.request.auto_fulfill.value_or(false)
                );
                connector_name = std::string{choice.connector->name()};
                if (outcome.shadow_killed) {
                    m_context.metrics().counter("payouts_shadow_killed").increment();
                }
                if (!outcome.reference_id.empty()) {
                    note = note.empty() ? std::string{"connector_ref="} + outcome.reference_id
                                        : note + ";connector_ref=" + outcome.reference_id;
                }
                m_context.process_trackers().schedule(
                    "payout_sync",
                    std::format(R"({{"payout_id":"{}"}})", payout_id),
                    "0"
                );
            } else {
                note = note.empty() ? std::string{"connector_unavailable"}
                                    : note + ";connector_unavailable";
            }
        }

        if (!input.request.payout_token && (input.request.payout_method_data.has_value() ||
                                            input.request.payout_method_id.has_value() ||
                                            input.request.source_bank_data.has_value())) {
            m_support.store_payout_method_data(payout_id, input.body);
        }

        ::app::store::payouts::PayoutRecord record{};
        record.set_payout_id(payout_id);
        record.set_merchant_id(merchant_id);
        record.set_amount(amount);
        record.set_currency(currency);
        record.set_status(status);
        record.set_connector(connector_name);
        record.set_created_at(m_support.created_at_now());
        if (input.request.payout_type) {
            record.set_payout_type(input.request.payout_type->get_value());
        }
        if (input.request.customer_id) {
            record.set_customer_id(input.request.customer_id->get_value());
        }
        record.set_metadata(m_support.encode_meta(
            input.profile_id,
            input.browser_info,
            payout_link_id,
            payout_link_id.empty() ? std::string{} : std::string{"Submitted"},
            note,
            input.request.merchant_order_reference_id.value_or(""),
            input.request.entity_type ? input.request.entity_type->get_value() : std::string{}
        ));
        record = m_context.repository().save_payout(record);

        m_support.record_payout_attempt(
            record.get_payout_id(),
            record.get_merchant_id(),
            connector_name,
            record.get_status(),
            record.get_amount(),
            record.get_currency()
        );

        m_context.cache().put(
            "payouts.idempotency." + payout_id,
            core::cache::to_cache_value(merchant_id)
        );
        m_support.emit_payout_webhook("created", record);

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
