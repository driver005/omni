export module app_payment:fraud_check_runtime_common;

import std;
import interfaces;
import core_serde;
import core_error;
import app_context;
import app_store;
import app_connector;
import app_events;

import :fraud_check_input;
import :fraud_check_result;

export namespace app::handlers::fraud_check {

/**
 * @brief Shared business helpers for the fraud-check route runtime.
 *
 * Each method is a direct port of the same-named upstream/local helper in the original
 * handler. No HTTP types appear here; failures are returned as AppError values.
 */
class FrmRuntimeSupport
{
public:
    explicit FrmRuntimeSupport(::app::context::AppContext& context) noexcept :
        m_context{context}
    {
    }

    [[nodiscard]] core::error::AppError
    make_error(core::error::AppErrorKind kind, std::string message) const
    {
        return core::error::AppErrorBuilder{}(kind, std::move(message));
    }

    // core/fraud_check/flows/fulfillment_flow.rs:33-151 -- builds the FrmFulfillmentRouterData
    // substitute. Fields with no local record column are documented in the parity report and
    // requested in "## Shared contract requests".
    [[nodiscard]] ::app::connector::ConnectorRequest construct_fulfillment_router_data(
        const ::app::store::payments::PaymentIntentRecord& payment_intent,
        const ::app::store::payment_attempts::PaymentAttemptRecord& payment_attempt,
        std::string_view frm_name,
        const FrmFulfillmentRequestWire& request,
        std::string_view raw_payload
    ) const
    {
        // FraudCheckFulfillmentData { amount, order_details, fulfillment_req } is carried as the
        // raw request payload; the ConnectorRequest is the in-process router-data stand-in.
        ::app::connector::ConnectorRequest router_data{};
        router_data.set_operation("fulfillment");
        router_data.set_merchant_id(payment_intent.get_merchant_id());
        router_data.set_resource_id(payment_attempt.get_payment_id());
        router_data.set_amount(
            payment_attempt.get_amount() != 0 ? payment_attempt.get_amount()
                                              : payment_intent.get_amount()
        );
        router_data.set_currency(payment_intent.get_currency());
        router_data.set_payment_method_type(std::string{frm_name});
        router_data.set_raw_payload(std::string{raw_payload});
        (void)request; // order_details / fulfillment_req are transported in raw_payload.
        return router_data;
    }

    // types/api/fraud_check.rs:44-56 FraudCheckConnectorData::get_connector_by_name. The local
    // ConnectorRouter has no public select-by-name; select_for with the FRM name as the
    // payment-method-type resolves the in-process FRM connector substitute, and the name is
    // verified against the upstream FrmConnectors enum values (api_models/src/enums.rs:208-217).
    [[nodiscard]] std::shared_ptr<::app::connector::IConnector>
    get_connector_by_name(std::string_view frm_name) const
    {
        static constexpr std::array<std::string_view, 5> kFrmConnectors{
            "signifyd",
            "riskified",
            "cybersourcedecisionmanager",
            "sanlam_payshield",
            "nsure"
        };
        if (std::find(kFrmConnectors.begin(), kFrmConnectors.end(), frm_name) ==
            kFrmConnectors.end()) {
            return nullptr;
        }
        ::app::connector::ConnectorRequest lookup{};
        lookup.set_merchant_id("frm");
        lookup.set_payment_method_type(std::string{frm_name});
        return m_context.connector_router().select_for(lookup);
    }

    // services::execute_connector_processing_step with CallConnectorAction::Trigger. The local
    // IConnector has no FRM Fulfillment method; `authorize` is the deterministic in-process
    // substitute, invoked exactly once with the Fulfillment router data.
    [[nodiscard]] ::app::connector::ConnectorResponse execute_connector_processing_step(
        ::app::connector::IConnector& connector,
        const ::app::connector::ConnectorRequest& router_data
    ) const
    {
        return connector.authorize(router_data);
    }

    // update_fraud_check_response_with_frm_id: ResponseUpdate keeps frm_status / transaction id
    // / reason / score / metadata and advances last_step to Fulfillment. Locally the last_step is
    // persisted in `decision`; the other values are unchanged because they have no separate
    // columns (see shared contract requests). updated_at has no local column either.
    [[nodiscard]] bool update_fraud_check_response_with_frm_id(
        const ::app::store::fraud_checks::FraudCheckRecord& fraud_check
    ) const
    {
        ::app::store::fraud_checks::FraudCheckRecord update = fraud_check;
        update.set_decision("Fulfillment");
        return m_context.repository().update_fraud_check(update);
    }

    // 1181-1189 find_payment_intent_by_payment_id_processor_merchant_id is the repository
    // find_payment_by_string plus the processor-merchant scope applied in frm_fulfillment_core.
    [[nodiscard]] std::optional<::app::store::payments::PaymentIntentRecord>
    find_payment_intent_by_payment_id_processor_merchant_id(std::string_view payment_id) const
    {
        return m_context.repository().find_payment_by_string(payment_id);
    }

    // 1196-1205 find_payment_attempt_by_payment_id_processor_merchant_id_attempt_id. The repository
    // has no composite lookup; filter by payment_id + processor merchant id deterministically.
    [[nodiscard]] std::optional<::app::store::payment_attempts::PaymentAttemptRecord>
    find_payment_attempt_by_payment_id_processor_merchant_id_attempt_id(
        std::string_view processor_merchant_id,
        std::string_view payment_id
    ) const
    {
        for (const auto& attempt: m_context.repository().list_payment_attempts()) {
            if (attempt.get_payment_id() == payment_id &&
                attempt.get_merchant_id() == processor_merchant_id) {
                return attempt;
            }
        }
        return std::nullopt;
    }

    // Upstream reads payment_attempt.active_frm_id (core/fraud_check.rs:1207). The local
    // PaymentAttemptRecord has no active_frm_id column, so the FRM id is the fraud_checks row's
    // natural key for this payment; a shared contract request asks for the column to be added.
    [[nodiscard]] std::string resolve_active_frm_id(
        const ::app::store::payment_attempts::PaymentAttemptRecord& payment_attempt,
        std::string_view processor_merchant_id
    ) const
    {
        for (const auto& record: m_context.repository().list_fraud_checks()) {
            if (record.get_payment_id() == payment_attempt.get_payment_id() &&
                record.get_merchant_id() == processor_merchant_id) {
                return record.get_fraud_check_id();
            }
        }
        return {};
    }

    // 1209-1212 find_fraud_check_by_frm_id -- exact repository lookup by fraud_check_id.
    [[nodiscard]] std::optional<::app::store::fraud_checks::FraudCheckRecord>
    find_fraud_check_by_frm_id(std::string_view frm_id) const
    {
        return m_context.repository().find_fraud_check_by_string(frm_id);
    }

    // (PreFrm && last_step == TransactionOrRecordRefund) || (PostFrm && last_step ==
    // CheckoutOrSale). The local FraudCheckRecord has no frm_transaction_type / last_step columns;
    // both are encoded in `decision` as "<PreFrm|PostFrm>:<step>" (bare step accepted for legacy
    // rows).
    [[nodiscard]] bool
    frm_flow_terminated(const ::app::store::fraud_checks::FraudCheckRecord& record) const
    {
        const std::string& decision = record.get_decision();
        constexpr std::string_view pre = "PreFrm:";
        constexpr std::string_view post = "PostFrm:";
        if (decision.starts_with(pre)) {
            return decision.substr(pre.size()) == "TransactionOrRecordRefund";
        }
        if (decision.starts_with(post)) {
            return decision.substr(post.size()) == "CheckoutOrSale";
        }
        return decision == "TransactionOrRecordRefund" || decision == "CheckoutOrSale";
    }

    // to_payment_failed_response layer: the connector could not be executed at all.
    [[nodiscard]] bool
    connector_transport_failed(const ::app::connector::ConnectorResponse& response) const
    {
        const auto& s = response.get_status();
        return response.get_retryable() || s.empty() || s == "timeout" || s == "connector_error";
    }

    // response.response Err layer: the connector executed and reported a failure.
    [[nodiscard]] bool
    connector_reported_error(const ::app::connector::ConnectorResponse& response) const
    {
        const auto& s = response.get_status();
        return s == "failed" || s == "error" ||
               response.get_raw_json().find("\"error\"") != std::string::npos;
    }

    // ApiErrorResponse::ExternalConnectorError { code, message, connector, status_code, reason }
    // (core/fraud_check.rs:1294-1303, api_error_response.rs:390-396). The local error wire shape
    // is {"error":KIND,"message":TEXT}, so all five fields are carried in the deterministic
    // message; the connector error HTTP status is carried in status_code.
    [[nodiscard]] std::string external_connector_error_message(
        const ::app::connector::ConnectorResponse& response,
        std::string_view frm_name
    ) const
    {
        return std::string{"FRM_FULFILLMENT_FAILED: "} + response.get_message() +
               "; connector=" + std::string{frm_name} +
               "; status_code=" + connector_error_status_code(response) +
               "; reason=" + response.get_raw_json();
    }

    [[nodiscard]] std::string
    connector_error_status_code(const ::app::connector::ConnectorResponse& response) const
    {
        const auto& s = response.get_status();
        if (s == "failed" || s == "error") {
            return "400";
        }
        if (s == "timeout") {
            return "504";
        }
        return "500";
    }

    // Upstream connectors build FulfillmentResponse from the connector result
    // (signifyd/transformers/api.rs:916-920). Prefer the connector raw payload when it carries
    // order_id/shipment_ids; the in-process substitute echoes the request, so fall back to the
    // typed request values (documented local substitute).
    [[nodiscard]] FulfillmentResponseWire build_fulfillment_response(
        const ::app::connector::ConnectorResponse& response,
        const FrmFulfillmentRequestWire& request
    ) const
    {
        FulfillmentResponseWire result{};
        result.set_order_id(request.get_order_id());
        std::vector<std::string> ids{};
        for (const auto& item: request.get_fulfillments()) {
            ids.push_back(item.get_shipment_id());
        }
        result.set_shipment_ids(std::move(ids));

        auto generic = serde::Ser::decode_generic("application/json", response.get_raw_json());
        if (!generic) {
            return result;
        }
        const auto* root = std::get_if<interfaces::Value::Object>(&generic->get());
        if (root == nullptr) {
            return result;
        }
        if (const auto order = root->get("order_id")) {
            if (const auto* text = std::get_if<std::string>(&order->get())) {
                result.set_order_id(*text);
            }
        }
        if (const auto shipments = root->get("shipment_ids")) {
            if (const auto* array = std::get_if<interfaces::Value::Array>(&shipments->get())) {
                std::vector<std::string> connector_ids{};
                for (const auto& element: *array) {
                    if (const auto* text = std::get_if<std::string>(&element.get())) {
                        connector_ids.push_back(*text);
                    }
                }
                if (!connector_ids.empty()) {
                    result.set_shipment_ids(std::move(connector_ids));
                }
            }
        }
        return result;
    }

    [[nodiscard]] std::string
    processor_merchant_id_of(const ::app::store::payments::PaymentIntentRecord& intent) const
    {
        return intent.get_merchant_id();
    }

    // Mirror the upstream FraudCheck analytics/notification event as a persisted webhook event.
    void persist_fulfillment_event(
        std::string_view event_id,
        const std::string& merchant_id,
        std::string_view org_id,
        std::string_view payment_id,
        std::string_view payload,
        std::string_view connector_status
    ) const
    {
        ::app::store::webhook_events::WebhookEventRecord record{};
        record.set_event_id(std::string{event_id});
        record.set_merchant_id(merchant_id);
        record.set_event_type("frm_fulfillment");
        record.set_event_class(
            org_id.empty() ? std::string{"frm_fulfillment"}
                           : std::string{"frm_fulfillment:org:"} + std::string{org_id}
        );
        record.set_status(
            connector_status.empty() ? std::string{"fulfilled"} : std::string{connector_status}
        );
        record.set_payload(std::string{payload});
        record.set_delivery_attempts(0);
        record.set_created_at(
            std::to_string(
                std::chrono::duration_cast<std::chrono::seconds>(
                    std::chrono::system_clock::now().time_since_epoch()
                )
                    .count()
            )
        );
        (void)m_context.repository().save_webhook_event(std::move(record));
    }

private:
    ::app::context::AppContext& m_context;
};

} // namespace app::handlers::fraud_check
