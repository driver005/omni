module;
export module app_handlers:fraud_check;

import std;
import interfaces;
import core_router;
import core_serde;
import utils_openapi;
import app_model;
import app_dto;
import app_context;
import core_error;
import app_auth;
import app_store;
import app_connector;
import app_http;
import app_metrics;
import app_events;

namespace app::handlers::fraud_check {

/**
 * @brief Local typed mirror of upstream `FrmFulfillmentRequest`
 * (hyperswitch_domain_models/src/router_request_types/fraud_check.rs:108-137).
 *
 * Upstream carries a nested `fulfillments` object array that has no local serde
 * wrapper, so the shipment ids are normalized from the generic JSON tree while
 * the remaining fields are declared here. The JSON codec ignores unknown
 * upstream fields, so this shape is forward compatible with the full DTO.
 */
class FrmFulfillmentRequestWire
{
public:
    void set_payment_id(std::string value) { payment_id = std::move(value); }
    [[nodiscard]] const std::string& get_payment_id() const noexcept { return payment_id; }
    void set_order_id(std::string value) { order_id = std::move(value); }
    [[nodiscard]] const std::string& get_order_id() const noexcept { return order_id; }
    void set_fulfillment_status(std::optional<std::string> value) { fulfillment_status = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_fulfillment_status() const noexcept { return fulfillment_status; }
    void set_tracking_company(std::optional<std::string> value) { tracking_company = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_tracking_company() const noexcept { return tracking_company; }
    void set_carrier(std::optional<std::string> value) { carrier = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_carrier() const noexcept { return carrier; }
    void set_fulfillment_method(std::optional<std::string> value) { fulfillment_method = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_fulfillment_method() const noexcept { return fulfillment_method; }
    void set_shipment_status(std::optional<std::string> value) { shipment_status = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_shipment_status() const noexcept { return shipment_status; }
    void set_shipped_at(std::optional<std::string> value) { shipped_at = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_shipped_at() const noexcept { return shipped_at; }

    std::string payment_id{};
    std::string order_id{};
    std::optional<std::string> fulfillment_status{};
    std::optional<std::string> tracking_company{};
    std::optional<std::string> carrier{};
    std::optional<std::string> fulfillment_method{};
    std::optional<std::string> shipment_status{};
    std::optional<std::string> shipped_at{};
};

/**
 * @brief Typed mirror of upstream `FraudCheckResponseData::FulfillmentResponse`
 * (hyperswitch_domain_models/src/router_response_types/fraud_check.rs:15-18).
 *
 * Upstream returns this connector result, not the decoded request, for a
 * fulfillment call. Keeping the shape typed closes the response wire gap.
 */
class FulfillmentResponseWire
{
public:
    void set_order_id(std::string value) { order_id = std::move(value); }
    [[nodiscard]] const std::string& get_order_id() const noexcept { return order_id; }
    void set_shipment_ids(std::vector<std::string> value) { shipment_ids = std::move(value); }
    [[nodiscard]] const std::vector<std::string>& get_shipment_ids() const noexcept { return shipment_ids; }

    std::string order_id{};
    std::vector<std::string> shipment_ids{};
};

} // namespace app::handlers::fraud_check

template<>
struct serde::Serializable<::app::handlers::fraud_check::FrmFulfillmentRequestWire>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"payment_id", &::app::handlers::fraud_check::FrmFulfillmentRequestWire::get_payment_id, &::app::handlers::fraud_check::FrmFulfillmentRequestWire::set_payment_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"order_id", &::app::handlers::fraud_check::FrmFulfillmentRequestWire::get_order_id, &::app::handlers::fraud_check::FrmFulfillmentRequestWire::set_order_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"fulfillment_status", &::app::handlers::fraud_check::FrmFulfillmentRequestWire::get_fulfillment_status, &::app::handlers::fraud_check::FrmFulfillmentRequestWire::set_fulfillment_status, serde::FieldOptions{}>{},
            serde::FieldDesc<"tracking_company", &::app::handlers::fraud_check::FrmFulfillmentRequestWire::get_tracking_company, &::app::handlers::fraud_check::FrmFulfillmentRequestWire::set_tracking_company, serde::FieldOptions{}>{},
            serde::FieldDesc<"carrier", &::app::handlers::fraud_check::FrmFulfillmentRequestWire::get_carrier, &::app::handlers::fraud_check::FrmFulfillmentRequestWire::set_carrier, serde::FieldOptions{}>{},
            serde::FieldDesc<"fulfillment_method", &::app::handlers::fraud_check::FrmFulfillmentRequestWire::get_fulfillment_method, &::app::handlers::fraud_check::FrmFulfillmentRequestWire::set_fulfillment_method, serde::FieldOptions{}>{},
            serde::FieldDesc<"shipment_status", &::app::handlers::fraud_check::FrmFulfillmentRequestWire::get_shipment_status, &::app::handlers::fraud_check::FrmFulfillmentRequestWire::set_shipment_status, serde::FieldOptions{}>{},
            serde::FieldDesc<"shipped_at", &::app::handlers::fraud_check::FrmFulfillmentRequestWire::get_shipped_at, &::app::handlers::fraud_check::FrmFulfillmentRequestWire::set_shipped_at, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "frm_fulfillment_request_wire"; }
};

template<>
struct serde::Serializable<::app::handlers::fraud_check::FulfillmentResponseWire>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"order_id", &::app::handlers::fraud_check::FulfillmentResponseWire::get_order_id, &::app::handlers::fraud_check::FulfillmentResponseWire::set_order_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"shipment_ids", &::app::handlers::fraud_check::FulfillmentResponseWire::get_shipment_ids, &::app::handlers::fraud_check::FulfillmentResponseWire::set_shipment_ids, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "frm_fulfillment_response_wire"; }
};

export namespace app::handlers::fraud_check {

class FraudCheckRouteHandlers
{
public:
    void handle_frm_fulfillment(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        // Upstream route uses platform ApiKeyAuth (fraud_check.rs:25-28). The
        // declarative rule lives in RouteAuthRegistry; authorize_route applies
        // it so handlers no longer construct auth rules inline.
        const auto rule = m_context->auth_rules().rule_for("frm_fulfillment");
        const auto auth = m_context->authorizer().authorize_route(app::auth::AuthContextBuilder{}(req), rule);
        if (auth) { send_error(res, send, *auth); return; }

        const auto body = req.body_as_string();
        if (body.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }

        using Input = ::app::handlers::fraud_check::FrmFulfillmentRequestWire;
        auto parsed = serde::Ser::deserialize<Input>("application/json", body);
        if (!parsed) {
            send_error(res, send, core::error::AppErrorBuilder{}(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }
        const auto& request = *parsed;
        if (request.get_payment_id().empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}(core::error::AppErrorKind::VALIDATION_FAILED, "payment_id is required"));
            return;
        }
        if (request.get_order_id().empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}(core::error::AppErrorKind::VALIDATION_FAILED, "order_id is required"));
            return;
        }
        const auto shipment_ids = extract_shipment_ids(body);

        // Upstream scopes by the platform's processor_merchant_id. Locally the
        // processor merchant is resolved from the payment row; an explicit
        // x-merchant-id that disagrees is refused as a deterministic stand-in
        // for that platform scoping.
        const std::string header_merchant_id = app::http::merchant_id_from_header{}(req);
        const std::string org_id = app::http::org_id_from_header{}(req);

        const auto payment = m_context->repository().find_payment_by_string(request.get_payment_id());
        if (!payment) {
            send_error(res, send, core::error::AppErrorBuilder{}(core::error::AppErrorKind::NOT_FOUND, "payment not found"));
            return;
        }
        if (!header_merchant_id.empty() && payment->get_merchant_id() != header_merchant_id) {
            send_error(res, send, core::error::AppErrorBuilder{}(core::error::AppErrorKind::FORBIDDEN, "merchant id does not match request context"));
            return;
        }
        const std::string merchant_id = payment->get_merchant_id();
        const auto& status = payment->get_status();
        const bool succeeded = status == "succeeded" || status == "Succeeded";
        if (!succeeded) {
            send_error(res, send, core::error::AppErrorBuilder{}(core::error::AppErrorKind::PRECONDITION_FAILED, "Fulfillment can be performed only for succeeded payment"));
            return;
        }

        // Upstream resolves payment_attempt.active_frm_id then
        // find_fraud_check_by_frm_id (core/fraud_check.rs:1207-1212). The local
        // payment record has no attempt/active_frm_id, so resolve the FRM row
        // by its natural (merchant_id, payment_id) key instead.
        auto fraud_check = find_fraud_check(merchant_id, request.get_payment_id());
        if (!fraud_check) {
            send_error(res, send, core::error::AppErrorBuilder{}(core::error::AppErrorKind::INVALID_REQUEST, "no fraud check entry found for this payment_id"));
            return;
        }
        if (!frm_flow_terminated(*fraud_check)) {
            send_error(res, send, core::error::AppErrorBuilder{}(core::error::AppErrorKind::PRECONDITION_FAILED, "Frm pre/post flow hasn't terminated yet, so fulfillment cannot be called"));
            return;
        }

        // Upstream dispatches to the FRM connector persisted on the fraud_check
        // row (FraudCheckConnectorData::get_connector_by_name(&frm_name),
        // core/fraud_check.rs:1254). The local ConnectorRouter has no public
        // select-by-name, so carry the persisted frm_name on the request and
        // dispatch through the merchant-selected backend (documented substitute).
        app::connector::ConnectorRequest connector_request{};
        connector_request.set_operation("fulfillment");
        connector_request.set_merchant_id(merchant_id);
        connector_request.set_resource_id(request.get_payment_id());
        connector_request.set_payment_method_type(fraud_check->get_connector());
        connector_request.set_raw_payload(body);

        auto connector = m_context->connector_router().select(merchant_id);
        if (!connector) {
            send_error(res, send, core::error::AppErrorBuilder{}(core::error::AppErrorKind::CONNECTOR_ERROR, "no FRM connector available"));
            return;
        }
        const auto connector_response = connector->authorize(connector_request);
        if (connector_call_failed(connector_response)) {
            send_error(res, send, core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::CONNECTOR_ERROR,
                connector_error_message(connector_response, fraud_check->get_connector())
            ));
            return;
        }
        const std::string connector_status = connector_response.get_status();

        // Upstream update_fraud_check_response_with_frm_id advances last_step to
        // Fulfillment and persists the FRM status/score (core/fraud_check.rs:1279-1293).
        // The local record has no reason/metadata columns, so the transition is
        // encoded in `decision` and the FRM status in `status`.
        fraud_check->set_status(connector_status.empty() ? std::string{"success"} : connector_status);
        fraud_check->set_decision("Fulfillment");
        (void)m_context->repository().update_fraud_check(*fraud_check);

        const std::string event_id = m_context->events().publish("frm_fulfillment", body);
        persist_fulfillment_event(event_id, merchant_id, org_id, request.get_payment_id(), body, connector_status);

        m_context->metrics().record_flow("FrmFulfillment");
        m_context->metrics().counter("FRM_FULFILLMENT").increment();

        ::app::handlers::fraud_check::FulfillmentResponseWire response{};
        response.set_order_id(request.get_order_id());
        response.set_shipment_ids(shipment_ids);
        auto text = serde::Ser::encode_json(response);
        send_json(res, send, text, true);
    }

    void install(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "/webhooks"}
            .add_route(
                Route{"/frm_fulfillment"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_frm_fulfillment(req, res, std::move(send));
                    })
                    .summary("Hyperswitch frm_fulfillment")
            );
    }

    explicit FraudCheckRouteHandlers(app::context::AppContext& context) noexcept
        : m_context{&context}
    {
    }

private:
    [[nodiscard]] std::vector<std::string> extract_shipment_ids(std::string_view body) const
    {
        std::vector<std::string> ids{};
        auto generic = serde::Ser::decode_generic("application/json", body);
        if (!generic) {
            return ids;
        }
        const auto* root = std::get_if<interfaces::Value::Object>(&generic->get());
        if (root == nullptr) {
            return ids;
        }
        const auto field = root->get("fulfillments");
        if (!field) {
            return ids;
        }
        const auto* array = std::get_if<interfaces::Value::Array>(&field.value().get());
        if (array == nullptr) {
            return ids;
        }
        for (const auto& element : *array) {
            const auto* item = std::get_if<interfaces::Value::Object>(&element.get());
            if (item == nullptr) {
                continue;
            }
            const auto shipment = item->get("shipment_id");
            if (!shipment) {
                continue;
            }
            if (const auto* text = std::get_if<std::string>(&shipment.value().get())) {
                ids.push_back(*text);
            }
        }
        return ids;
    }

    [[nodiscard]] std::optional<app::store::fraud_checks::FraudCheckRecord>
    find_fraud_check(std::string_view merchant_id, std::string_view payment_id) const
    {
        for (const auto& record : m_context->repository().list_fraud_checks()) {
            if (record.get_payment_id() == payment_id && record.get_merchant_id() == merchant_id) {
                return record;
            }
        }
        return std::nullopt;
    }

    [[nodiscard]] bool frm_flow_terminated(const app::store::fraud_checks::FraudCheckRecord& record) const
    {
        // Local FraudCheckRecord carries no frm_transaction_type column; the
        // last_step is encoded in `decision`, optionally prefixed with the FRM
        // flow type. Both prefixed and bare terminal values are accepted.
        const bool decision_terminal = is_terminal_step(record.get_decision());
        const bool status_terminal = is_terminal_step(record.get_status());
        return decision_terminal || status_terminal;
    }

    [[nodiscard]] bool is_terminal_step(std::string_view step) const
    {
        return step == "PreFrm:TransactionOrRecordRefund"
            || step == "PostFrm:CheckoutOrSale"
            || step == "TransactionOrRecordRefund"
            || step == "CheckoutOrSale";
    }

    [[nodiscard]] bool connector_call_failed(const app::connector::ConnectorResponse& response) const
    {
        const std::string& status = response.get_status();
        return response.get_retryable() || status.empty() || status == "failed"
            || status == "error" || status == "connector_error" || status == "timeout";
    }

    [[nodiscard]] std::string connector_error_message(
        const app::connector::ConnectorResponse& response,
        std::string_view frm_name
    ) const
    {
        // Mirrors upstream ExternalConnectorError {code, message, connector,
        // status_code, reason} (core/fraud_check.rs:1294-1303) on a single
        // deterministic CONNECTOR_ERROR message.
        return std::string{"ExternalConnectorError: code=FRM_FULFILLMENT_FAILED"}
            + "; message=" + response.get_message()
            + "; connector=" + std::string{frm_name}
            + "; status_code=" + response.get_status()
            + "; reason=" + response.get_raw_json();
    }

    // Mirror the upstream FraudCheck analytics/notification event as a persisted
    // webhook event. The event id comes from the shared EventBus so the event and
    // this analytics row carry the same stable identifier.
    void persist_fulfillment_event(
        std::string_view event_id,
        const std::string& merchant_id,
        const std::string& org_id,
        const std::string& payment_id,
        std::string_view payload,
        std::string_view connector_status
    ) const
    {
        app::store::webhook_events::WebhookEventRecord record;
        record.set_event_id(std::string{event_id});
        record.set_merchant_id(merchant_id);
        record.set_event_type("frm_fulfillment");
        record.set_event_class(org_id.empty()
            ? std::string{"frm_fulfillment"}
            : std::string{"frm_fulfillment:org:"} + org_id);
        record.set_status(connector_status.empty() ? std::string{"fulfilled"} : std::string{connector_status});
        record.set_payload(std::string{payload});
        record.set_delivery_attempts(0);
        record.set_created_at(now_value());
        (void)m_context->repository().save_webhook_event(std::move(record));
    }

    [[nodiscard]] std::string now_value() const
    {
        const auto seconds = std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::system_clock::now().time_since_epoch()
        ).count();
        return std::to_string(seconds);
    }

    void send_error(
        interfaces::io::IResponse& res,
        std::function<void()> send,
        core::error::AppError error
    ) const
    {
        auto text = serde::Ser::encode_json(error);
        auto bytes = serde::Ser::serialize_raw("application/json", text);
        res.set_status(core::error::AppErrorStatus{}(error.get_kind()));
        res.set_body(std::move(bytes));
        send();
    }

    void send_json(
        interfaces::io::IResponse& res,
        std::function<void()> send,
        std::string_view text,
        bool ok
    ) const
    {
        auto bytes = serde::Ser::serialize_raw("application/json", text);
        res.set_status(ok ? interfaces::io::types::Status::OK
                          : interfaces::io::types::Status::BAD_REQUEST);
        res.set_body(std::move(bytes));
        send();
    }

    app::context::AppContext* m_context{nullptr};
};

} // namespace app::handlers::fraud_check
