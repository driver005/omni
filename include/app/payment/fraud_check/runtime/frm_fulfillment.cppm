export module app_payment:fraud_check_runtime_frm_fulfillment;

import std;
import core_error;
import app_context;

import :fraud_check_input;
import :fraud_check_result;
import :fraud_check_runtime_common;

export namespace app::handlers::fraud_check {

/**
 * @brief Single-route runtime for `frm_fulfillment`.
 *
 * Strict upstream port:
 *   routes/fraud_check.rs:11-32 frm_fulfillment
 *     -> core/fraud_check.rs:1175-1241 frm_fulfillment_core
 *       -> core/fraud_check.rs:1245-1305 make_fulfillment_api_call
 *         -> FraudCheckConnectorData::get_connector_by_name
 *         -> flows/fulfillment_flow.rs:33-151 construct_fulfillment_router_data
 *         -> services::execute_connector_processing_step + to_payment_failed_response
 *         -> db.update_fraud_check_response_with_frm_id
 */
class FrmFulfillmentRuntime
{
public:
    explicit FrmFulfillmentRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] std::expected<FulfillmentResponseWire, core::error::AppError>
    frm_fulfillment(const FrmFulfillmentInput& input) const
    {
        // ───────────────── core/fraud_check.rs:1175-1241 frm_fulfillment_core ─────────────────

        // 1181-1189 find_payment_intent_by_payment_id_processor_merchant_id.
        auto payment_intent = m_support.find_payment_intent_by_payment_id_processor_merchant_id(
            input.request.get_payment_id()
        );
        if (!payment_intent) {
            return std::unexpected{m_support.make_error(
                core::error::AppErrorKind::NOT_FOUND,
                "Payment does not exist in our records"
            )};
        }
        // Upstream scopes the read by platform processor_merchant_id. The local processor
        // merchant id is the one persisted on the intent; an explicit conflicting header is
        // refused as the deterministic stand-in for platform scoping.
        const std::string processor_merchant_id = payment_intent->get_merchant_id();
        if (!input.header_merchant_id.empty() &&
            processor_merchant_id != input.header_merchant_id) {
            return std::unexpected{m_support.make_error(
                core::error::AppErrorKind::FORBIDDEN,
                "merchant id does not match request context"
            )};
        }

        // 1190-1191 IntentStatus::Succeeded gate.
        const auto& status = payment_intent->get_status();
        if (status != "succeeded" && status != "Succeeded") {
            return std::unexpected{m_support.make_error(
                core::error::AppErrorKind::PRECONDITION_FAILED,
                "Fulfillment can be performed only for succeeded payment"
            )};
        }

        // 1192-1194 invalid_request_error (ApiErrorResponse::InvalidRequestData -> 422).
        // 1196-1205 find_payment_attempt_by_payment_id_processor_merchant_id_attempt_id.
        auto payment_attempt =
            m_support.find_payment_attempt_by_payment_id_processor_merchant_id_attempt_id(
                processor_merchant_id,
                payment_intent->get_payment_id()
            );
        if (!payment_attempt) {
            return std::unexpected{m_support.make_error(
                core::error::AppErrorKind::NOT_FOUND,
                "Payment does not exist in our records"
            )};
        }

        // 1207-1212 payment_attempt.active_frm_id -> find_fraud_check_by_frm_id.
        const std::string active_frm_id =
            m_support.resolve_active_frm_id(*payment_attempt, processor_merchant_id);
        if (active_frm_id.empty()) {
            return std::unexpected{m_support.make_error(
                core::error::AppErrorKind::VALIDATION_FAILED,
                "no fraud check entry found for this payment_id"
            )};
        }
        auto fraud_check = m_support.find_fraud_check_by_frm_id(active_frm_id);
        if (!fraud_check) {
            return std::unexpected{m_support.make_error(
                core::error::AppErrorKind::VALIDATION_FAILED,
                "no fraud check entry found for this payment_id"
            )};
        }

        // 1214-1231 termination guard:
        //   (PreFrm && last_step == TransactionOrRecordRefund)
        //   || (PostFrm && last_step == CheckoutOrSale)
        if (!m_support.frm_flow_terminated(*fraud_check)) {
            return std::unexpected{m_support.make_error(
                core::error::AppErrorKind::PRECONDITION_FAILED,
                "Frm pre/post flow hasn't terminated yet, so fulfillment cannot be called"
            )};
        }

        // ───────────── core/fraud_check.rs:1245-1305 make_fulfillment_api_call ─────────────

        // 1254 FraudCheckConnectorData::get_connector_by_name(&fraud_check.frm_name).
        const std::string frm_name = fraud_check->get_connector();
        auto connector = m_support.get_connector_by_name(frm_name);
        if (!connector) {
            return std::unexpected{m_support.make_error(
                core::error::AppErrorKind::NOT_FOUND,
                "The connector provided in the request is incorrect or not available"
            )};
        }

        // 1260-1268 construct_fulfillment_router_data(...).
        const auto router_data = m_support.construct_fulfillment_router_data(
            *payment_intent,
            *payment_attempt,
            frm_name,
            input.request,
            input.raw_payload
        );

        // 1269-1278 execute_connector_processing_step(..., Fulfillment, Trigger)
        //          .to_payment_failed_response()?
        const auto connector_response =
            m_support.execute_connector_processing_step(*connector, router_data);
        if (m_support.connector_transport_failed(connector_response)) {
            return std::unexpected{m_support.make_error(
                core::error::AppErrorKind::CONNECTOR_ERROR,
                m_support.external_connector_error_message(connector_response, frm_name)
            )};
        }

        // 1279-1293 update_fraud_check_response_with_frm_id(ResponseUpdate{last_step: Fulfillment})
        //          .to_not_found_response(FraudCheckNotFound)?
        if (!m_support.update_fraud_check_response_with_frm_id(*fraud_check)) {
            return std::unexpected{m_support.make_error(
                core::error::AppErrorKind::NOT_FOUND,
                "Fraud check does not exist in our records"
            )};
        }

        // 1294-1303 response.response.map_err(ExternalConnectorError { code, message,
        //          connector, status_code, reason })?
        if (m_support.connector_reported_error(connector_response)) {
            return std::unexpected{m_support.make_error(
                core::error::AppErrorKind::CONNECTOR_ERROR,
                m_support.external_connector_error_message(connector_response, frm_name)
            )};
        }

        // ApiEventMetric::FraudCheck / FraudCheck analytics event (server_wrap metric layer).
        const std::string event_id =
            m_context.events().publish("frm_fulfillment", std::string{input.raw_payload});
        m_support.persist_fulfillment_event(
            event_id,
            m_support.processor_merchant_id_of(*payment_intent),
            input.org_id,
            input.request.get_payment_id(),
            input.raw_payload,
            connector_response.get_status()
        );

        // 1304 ApplicationResponse::Json(fulfillment_response).
        return FulfillmentResponseWire{
            m_support.build_fulfillment_response(connector_response, input.request)
        };
    }

private:
    ::app::context::AppContext& m_context;
    FrmRuntimeSupport m_support;
};

} // namespace app::handlers::fraud_check
