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
 * @brief Typed mirror of upstream `Address`
 * (hyperswitch_domain_models/src/router_request_types/fraud_check.rs:184-194).
 */
class FrmAddressWire
{
public:
    void set_street_address(std::string value) { street_address = std::move(value); }
    [[nodiscard]] const std::string& get_street_address() const noexcept { return street_address; }
    void set_unit(std::optional<std::string> value) { unit = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_unit() const noexcept { return unit; }
    void set_postal_code(std::string value) { postal_code = std::move(value); }
    [[nodiscard]] const std::string& get_postal_code() const noexcept { return postal_code; }
    void set_city(std::string value) { city = std::move(value); }
    [[nodiscard]] const std::string& get_city() const noexcept { return city; }
    void set_province_code(std::string value) { province_code = std::move(value); }
    [[nodiscard]] const std::string& get_province_code() const noexcept { return province_code; }
    void set_country_code(std::string value) { country_code = std::move(value); }
    [[nodiscard]] const std::string& get_country_code() const noexcept { return country_code; }

    std::string street_address{};
    std::optional<std::string> unit{};
    std::string postal_code{};
    std::string city{};
    std::string province_code{};
    std::string country_code{};
};

/**
 * @brief Typed mirror of upstream `Destination` (fraud_check.rs:174-182).
 */
class FrmDestinationWire
{
public:
    void set_full_name(std::string value) { full_name = std::move(value); }
    [[nodiscard]] const std::string& get_full_name() const noexcept { return full_name; }
    void set_organization(std::optional<std::string> value) { organization = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_organization() const noexcept { return organization; }
    void set_email(std::optional<std::string> value) { email = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_email() const noexcept { return email; }
    void set_address(FrmAddressWire value) { address = std::move(value); }
    [[nodiscard]] const FrmAddressWire& get_address() const noexcept { return address; }

    std::string full_name{};
    std::optional<std::string> organization{};
    std::optional<std::string> email{};
    FrmAddressWire address{};
};

/**
 * @brief Typed mirror of upstream `Product` (fraud_check.rs:165-172).
 */
class FrmProductWire
{
public:
    void set_item_name(std::string value) { item_name = std::move(value); }
    [[nodiscard]] const std::string& get_item_name() const noexcept { return item_name; }
    void set_item_quantity(std::int64_t value) { item_quantity = value; }
    [[nodiscard]] std::int64_t get_item_quantity() const noexcept { return item_quantity; }
    void set_item_id(std::string value) { item_id = std::move(value); }
    [[nodiscard]] const std::string& get_item_id() const noexcept { return item_id; }

    std::string item_name{};
    std::int64_t item_quantity{};
    std::string item_id{};
};

/**
 * @brief Typed mirror of upstream `Fulfillments` (fraud_check.rs:139-152).
 */
class FrmFulfillmentWire
{
public:
    void set_shipment_id(std::string value) { shipment_id = std::move(value); }
    [[nodiscard]] const std::string& get_shipment_id() const noexcept { return shipment_id; }
    void set_products(std::optional<std::vector<FrmProductWire>> value) { products = std::move(value); }
    [[nodiscard]] const std::optional<std::vector<FrmProductWire>>& get_products() const noexcept { return products; }
    void set_destination(FrmDestinationWire value) { destination = std::move(value); }
    [[nodiscard]] const FrmDestinationWire& get_destination() const noexcept { return destination; }

    std::string shipment_id{};
    std::optional<std::vector<FrmProductWire>> products{};
    FrmDestinationWire destination{};
};

/**
 * @brief Typed mirror of upstream `FrmFulfillmentRequest`
 * (hyperswitch_domain_models/src/router_request_types/fraud_check.rs:104-137).
 * `#[serde(deny_unknown_fields)]` is enforced locally by @ref parse_and_validate_request.
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
    void set_fulfillments(std::vector<FrmFulfillmentWire> value) { fulfillments = std::move(value); }
    [[nodiscard]] const std::vector<FrmFulfillmentWire>& get_fulfillments() const noexcept { return fulfillments; }
    void set_tracking_company(std::optional<std::string> value) { tracking_company = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_tracking_company() const noexcept { return tracking_company; }
    void set_tracking_numbers(std::optional<std::vector<std::string>> value) { tracking_numbers = std::move(value); }
    [[nodiscard]] const std::optional<std::vector<std::string>>& get_tracking_numbers() const noexcept { return tracking_numbers; }
    void set_tracking_urls(std::optional<std::vector<std::string>> value) { tracking_urls = std::move(value); }
    [[nodiscard]] const std::optional<std::vector<std::string>>& get_tracking_urls() const noexcept { return tracking_urls; }
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
    std::vector<FrmFulfillmentWire> fulfillments{};
    std::optional<std::string> tracking_company{};
    std::optional<std::vector<std::string>> tracking_numbers{};
    std::optional<std::vector<std::string>> tracking_urls{};
    std::optional<std::string> carrier{};
    std::optional<std::string> fulfillment_method{};
    std::optional<std::string> shipment_status{};
    std::optional<std::string> shipped_at{};
};

/**
 * @brief Typed mirror of upstream `FraudCheckResponseData::FulfillmentResponse`
 * (hyperswitch_domain_models/src/router_response_types/fraud_check.rs:15-18).
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
struct serde::Serializable<::app::handlers::fraud_check::FrmAddressWire>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"street_address", &::app::handlers::fraud_check::FrmAddressWire::get_street_address, &::app::handlers::fraud_check::FrmAddressWire::set_street_address, serde::FieldOptions{}>{},
            serde::FieldDesc<"unit", &::app::handlers::fraud_check::FrmAddressWire::get_unit, &::app::handlers::fraud_check::FrmAddressWire::set_unit, serde::FieldOptions{}>{},
            serde::FieldDesc<"postal_code", &::app::handlers::fraud_check::FrmAddressWire::get_postal_code, &::app::handlers::fraud_check::FrmAddressWire::set_postal_code, serde::FieldOptions{}>{},
            serde::FieldDesc<"city", &::app::handlers::fraud_check::FrmAddressWire::get_city, &::app::handlers::fraud_check::FrmAddressWire::set_city, serde::FieldOptions{}>{},
            serde::FieldDesc<"province_code", &::app::handlers::fraud_check::FrmAddressWire::get_province_code, &::app::handlers::fraud_check::FrmAddressWire::set_province_code, serde::FieldOptions{}>{},
            serde::FieldDesc<"country_code", &::app::handlers::fraud_check::FrmAddressWire::get_country_code, &::app::handlers::fraud_check::FrmAddressWire::set_country_code, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "frm_address_wire"; }
};

template<>
struct serde::Serializable<::app::handlers::fraud_check::FrmDestinationWire>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"full_name", &::app::handlers::fraud_check::FrmDestinationWire::get_full_name, &::app::handlers::fraud_check::FrmDestinationWire::set_full_name, serde::FieldOptions{}>{},
            serde::FieldDesc<"organization", &::app::handlers::fraud_check::FrmDestinationWire::get_organization, &::app::handlers::fraud_check::FrmDestinationWire::set_organization, serde::FieldOptions{}>{},
            serde::FieldDesc<"email", &::app::handlers::fraud_check::FrmDestinationWire::get_email, &::app::handlers::fraud_check::FrmDestinationWire::set_email, serde::FieldOptions{}>{},
            serde::FieldDesc<"address", &::app::handlers::fraud_check::FrmDestinationWire::get_address, &::app::handlers::fraud_check::FrmDestinationWire::set_address, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "frm_destination_wire"; }
};

template<>
struct serde::Serializable<::app::handlers::fraud_check::FrmProductWire>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"item_name", &::app::handlers::fraud_check::FrmProductWire::get_item_name, &::app::handlers::fraud_check::FrmProductWire::set_item_name, serde::FieldOptions{}>{},
            serde::FieldDesc<"item_quantity", &::app::handlers::fraud_check::FrmProductWire::get_item_quantity, &::app::handlers::fraud_check::FrmProductWire::set_item_quantity, serde::FieldOptions{}>{},
            serde::FieldDesc<"item_id", &::app::handlers::fraud_check::FrmProductWire::get_item_id, &::app::handlers::fraud_check::FrmProductWire::set_item_id, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "frm_product_wire"; }
};

template<>
struct serde::Serializable<::app::handlers::fraud_check::FrmFulfillmentWire>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"shipment_id", &::app::handlers::fraud_check::FrmFulfillmentWire::get_shipment_id, &::app::handlers::fraud_check::FrmFulfillmentWire::set_shipment_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"products", &::app::handlers::fraud_check::FrmFulfillmentWire::get_products, &::app::handlers::fraud_check::FrmFulfillmentWire::set_products, serde::FieldOptions{}>{},
            serde::FieldDesc<"destination", &::app::handlers::fraud_check::FrmFulfillmentWire::get_destination, &::app::handlers::fraud_check::FrmFulfillmentWire::set_destination, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "frm_fulfillment_wire"; }
};

template<>
struct serde::Serializable<::app::handlers::fraud_check::FrmFulfillmentRequestWire>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"payment_id", &::app::handlers::fraud_check::FrmFulfillmentRequestWire::get_payment_id, &::app::handlers::fraud_check::FrmFulfillmentRequestWire::set_payment_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"order_id", &::app::handlers::fraud_check::FrmFulfillmentRequestWire::get_order_id, &::app::handlers::fraud_check::FrmFulfillmentRequestWire::set_order_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"fulfillment_status", &::app::handlers::fraud_check::FrmFulfillmentRequestWire::get_fulfillment_status, &::app::handlers::fraud_check::FrmFulfillmentRequestWire::set_fulfillment_status, serde::FieldOptions{}>{},
            serde::FieldDesc<"fulfillments", &::app::handlers::fraud_check::FrmFulfillmentRequestWire::get_fulfillments, &::app::handlers::fraud_check::FrmFulfillmentRequestWire::set_fulfillments, serde::FieldOptions{}>{},
            serde::FieldDesc<"tracking_company", &::app::handlers::fraud_check::FrmFulfillmentRequestWire::get_tracking_company, &::app::handlers::fraud_check::FrmFulfillmentRequestWire::set_tracking_company, serde::FieldOptions{}>{},
            serde::FieldDesc<"tracking_numbers", &::app::handlers::fraud_check::FrmFulfillmentRequestWire::get_tracking_numbers, &::app::handlers::fraud_check::FrmFulfillmentRequestWire::set_tracking_numbers, serde::FieldOptions{}>{},
            serde::FieldDesc<"tracking_urls", &::app::handlers::fraud_check::FrmFulfillmentRequestWire::get_tracking_urls, &::app::handlers::fraud_check::FrmFulfillmentRequestWire::set_tracking_urls, serde::FieldOptions{}>{},
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

/**
 * Strict upstream port of the single fraud-check route.
 *
 * Upstream call graph (all v1):
 *   routes/fraud_check.rs:11-32 frm_fulfillment
 *     -> core/fraud_check.rs:1175-1241 frm_fulfillment_core
 *       -> core/fraud_check.rs:1245-1305 make_fulfillment_api_call
 *         -> FraudCheckConnectorData::get_connector_by_name (types/api/fraud_check.rs:44-56)
 *         -> core/fraud_check/flows/fulfillment_flow.rs:33-151 construct_fulfillment_router_data
 *         -> services::execute_connector_processing_step + to_payment_failed_response
 *         -> db.update_fraud_check_response_with_frm_id
 *
 * Each upstream function is ported as a same-named private local helper below. External FRM
 * connector infrastructure is substituted by the injected in-process ConnectorRouter; the
 * observable algorithm, wire shapes, auth, error statuses/messages and persistence mirror
 * upstream exactly. The v1/v2 `todo!()` variants have no behavior to port and are documented
 * in docs/hyperswitch-parity/fraud_check.md.
 */
class FraudCheckRouteHandlers
{
public:
    // routes/fraud_check.rs:11-32 -- server_wrap(Flow::FrmFulfillment, ApiKeyAuth{false,false},
    // LockAction::NotApplicable).
    void handle_frm_fulfillment(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto rule = m_context->auth_rules().rule_for("frm_fulfillment");
        const auto auth_ctx = app::auth::AuthContextBuilder{}(req);
        if (const auto auth = m_context->authorizer().authorize_route(auth_ctx, rule)) {
            send_error(res, send, *auth);
            return;
        }

        // ApiKeyAuth{ allow_connected_scope_operation: false, allow_platform_self_operation:
        // false } (fraud_check.rs:25-28). The declarative rule is the local stand-in for the
        // scheme; the two override flags are enforced here exactly as upstream declares them.
        if (const auto connected = auth_ctx.claim(app::auth::kClaimConnectedMerchantId);
            connected.has_value() && !connected->empty()) {
            send_error(res, send, make_error(
                core::error::AppErrorKind::FORBIDDEN,
                "The provided API key does not support connected scope operations for this flow"));
            return;
        }
        if (const auto internal = auth_ctx.claim(app::auth::kClaimInternalApiKey);
            internal.has_value() && !internal->empty()) {
            send_error(res, send, make_error(
                core::error::AppErrorKind::FORBIDDEN,
                "The provided API key does not support platform self-operations for this flow"));
            return;
        }

        // Flow::FrmFulfillment accounting; LockAction::NotApplicable -> no API lock.
        m_context->metrics().record_flow("FrmFulfillment");
        m_context->metrics().counter("FRM_FULFILLMENT").increment();

        const auto body = req.body_as_string();
        if (body.empty()) {
            send_error(res, send, make_error(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }

        const auto request = parse_and_validate_request(body);
        if (!request) {
            send_error(res, send, make_error(
                core::error::AppErrorKind::INVALID_JSON,
                "invalid FrmFulfillmentRequest payload"));
            return;
        }

        const std::string org_id{app::http::org_id_from_header{}(req)};
        const std::string header_merchant_id{app::http::merchant_id_from_header{}(req)};

        frm_fulfillment_core(*request, body, header_merchant_id, org_id, res, send);
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
    using SendFn = std::function<void()>;
    using Input = ::app::handlers::fraud_check::FrmFulfillmentRequestWire;
    using Fulfillment = ::app::handlers::fraud_check::FulfillmentResponseWire;
    using PaymentIntent = app::store::payments::PaymentIntentRecord;
    using PaymentAttempt = app::store::payment_attempts::PaymentAttemptRecord;
    using FraudCheck = app::store::fraud_checks::FraudCheckRecord;

    // core/fraud_check.rs:1175-1241
    void frm_fulfillment_core(
        const Input& request,
        std::string_view raw_payload,
        std::string_view header_merchant_id,
        std::string_view org_id,
        interfaces::io::IResponse& res,
        SendFn& send
    ) const
    {
        // 1181-1189 find_payment_intent_by_payment_id_processor_merchant_id.
        auto payment_intent = find_payment_intent_by_payment_id_processor_merchant_id(
            request.get_payment_id());
        if (!payment_intent) {
            send_error(res, send, make_error(
                core::error::AppErrorKind::NOT_FOUND,
                "Payment does not exist in our records"));
            return;
        }
        // Upstream scopes the read by platform processor_merchant_id. The local processor
        // merchant id is the one persisted on the intent; an explicit conflicting header is
        // refused as the deterministic stand-in for platform scoping.
        const std::string processor_merchant_id = payment_intent->get_merchant_id();
        if (!header_merchant_id.empty() && processor_merchant_id != header_merchant_id) {
            send_error(res, send, make_error(
                core::error::AppErrorKind::FORBIDDEN,
                "merchant id does not match request context"));
            return;
        }

        // 1190-1191 IntentStatus::Succeeded gate.
        const auto& status = payment_intent->get_status();
        if (status != "succeeded" && status != "Succeeded") {
            send_error(res, send, make_error(
                core::error::AppErrorKind::PRECONDITION_FAILED,
                "Fulfillment can be performed only for succeeded payment"));
            return;
        }

        // 1192-1194 invalid_request_error (ApiErrorResponse::InvalidRequestData -> 422).
        // 1196-1205 find_payment_attempt_by_payment_id_processor_merchant_id_attempt_id.
        auto payment_attempt = find_payment_attempt_by_payment_id_processor_merchant_id_attempt_id(
            processor_merchant_id, payment_intent->get_payment_id());
        if (!payment_attempt) {
            send_error(res, send, make_error(
                core::error::AppErrorKind::NOT_FOUND,
                "Payment does not exist in our records"));
            return;
        }

        // 1207-1212 payment_attempt.active_frm_id -> find_fraud_check_by_frm_id.
        const std::string active_frm_id = resolve_active_frm_id(*payment_attempt, processor_merchant_id);
        if (active_frm_id.empty()) {
            send_error(res, send, make_error(
                core::error::AppErrorKind::VALIDATION_FAILED,
                "no fraud check entry found for this payment_id"));
            return;
        }
        auto fraud_check = find_fraud_check_by_frm_id(active_frm_id);
        if (!fraud_check) {
            send_error(res, send, make_error(
                core::error::AppErrorKind::VALIDATION_FAILED,
                "no fraud check entry found for this payment_id"));
            return;
        }

        // 1214-1231 termination guard:
        //   (PreFrm && last_step == TransactionOrRecordRefund)
        //   || (PostFrm && last_step == CheckoutOrSale)
        if (!frm_flow_terminated(*fraud_check)) {
            send_error(res, send, make_error(
                core::error::AppErrorKind::PRECONDITION_FAILED,
                "Frm pre/post flow hasn't terminated yet, so fulfillment cannot be called"));
            return;
        }

        // 1219-1228 make_fulfillment_api_call.
        make_fulfillment_api_call(
            *fraud_check, *payment_intent, *payment_attempt, request, raw_payload, org_id, res, send);
    }

    // core/fraud_check.rs:1245-1305
    void make_fulfillment_api_call(
        const FraudCheck& fraud_check,
        const PaymentIntent& payment_intent,
        const PaymentAttempt& payment_attempt,
        const Input& request,
        std::string_view raw_payload,
        std::string_view org_id,
        interfaces::io::IResponse& res,
        SendFn& send
    ) const
    {
        // 1254 FraudCheckConnectorData::get_connector_by_name(&fraud_check.frm_name).
        const std::string frm_name = fraud_check.get_connector();
        auto connector = get_connector_by_name(frm_name);
        if (!connector) {
            send_error(res, send, make_error(
                core::error::AppErrorKind::NOT_FOUND,
                "The connector provided in the request is incorrect or not available"));
            return;
        }

        // 1260-1268 construct_fulfillment_router_data(...).
        const auto router_data = construct_fulfillment_router_data(
            payment_intent, payment_attempt, frm_name, request, raw_payload);

        // 1269-1278 execute_connector_processing_step(..., Fulfillment, Trigger)
        //          .to_payment_failed_response()?
        const auto connector_response = execute_connector_processing_step(*connector, router_data);
        if (connector_transport_failed(connector_response)) {
            send_error(res, send, make_error(
                core::error::AppErrorKind::CONNECTOR_ERROR,
                external_connector_error_message(connector_response, frm_name)));
            return;
        }

        // 1279-1293 update_fraud_check_response_with_frm_id(ResponseUpdate{last_step: Fulfillment})
        //          .to_not_found_response(FraudCheckNotFound)?
        if (!update_fraud_check_response_with_frm_id(fraud_check)) {
            send_error(res, send, make_error(
                core::error::AppErrorKind::NOT_FOUND,
                "Fraud check does not exist in our records"));
            return;
        }

        // 1294-1303 response.response.map_err(ExternalConnectorError { code, message,
        //          connector, status_code, reason })?
        if (connector_reported_error(connector_response)) {
            send_error(res, send, make_error(
                core::error::AppErrorKind::CONNECTOR_ERROR,
                external_connector_error_message(connector_response, frm_name)));
            return;
        }

        // ApiEventMetric::FraudCheck / FraudCheck analytics event (server_wrap metric layer).
        const std::string event_id = m_context->events().publish("frm_fulfillment", std::string{raw_payload});
        persist_fulfillment_event(
            event_id, processor_merchant_id_of(payment_intent), org_id,
            request.get_payment_id(), raw_payload, connector_response.get_status());

        // 1304 ApplicationResponse::Json(fulfillment_response).
        Fulfillment response = build_fulfillment_response(connector_response, request);
        auto text = serde::Ser::encode_json(response);
        send_json(res, send, text, true);
    }

    // core/fraud_check/flows/fulfillment_flow.rs:33-151 -- builds the FrmFulfillmentRouterData
    // substitute. Fields with no local record column are documented in the parity report and
    // requested in "## Shared contract requests".
    [[nodiscard]] app::connector::ConnectorRequest construct_fulfillment_router_data(
        const PaymentIntent& payment_intent,
        const PaymentAttempt& payment_attempt,
        std::string_view frm_name,
        const Input& request,
        std::string_view raw_payload
    ) const
    {
        // FraudCheckFulfillmentData { amount, order_details, fulfillment_req } is carried as the
        // raw request payload; the ConnectorRequest is the in-process router-data stand-in.
        app::connector::ConnectorRequest router_data{};
        router_data.set_operation("fulfillment");
        router_data.set_merchant_id(payment_intent.get_merchant_id());
        router_data.set_resource_id(payment_attempt.get_payment_id());
        router_data.set_amount(payment_attempt.get_amount() != 0
            ? payment_attempt.get_amount()
            : payment_intent.get_amount());
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
    [[nodiscard]] std::shared_ptr<app::connector::IConnector> get_connector_by_name(
        std::string_view frm_name
    ) const
    {
        static constexpr std::array<std::string_view, 5> kFrmConnectors{
            "signifyd", "riskified", "cybersourcedecisionmanager", "sanlam_payshield", "nsure"};
        if (std::find(kFrmConnectors.begin(), kFrmConnectors.end(), frm_name) == kFrmConnectors.end()) {
            return nullptr;
        }
        app::connector::ConnectorRequest lookup{};
        lookup.set_merchant_id("frm");
        lookup.set_payment_method_type(std::string{frm_name});
        return m_context->connector_router().select_for(lookup);
    }

    // services::execute_connector_processing_step with CallConnectorAction::Trigger. The local
    // IConnector has no FRM Fulfillment method; `authorize` is the deterministic in-process
    // substitute, invoked exactly once with the Fulfillment router data.
    [[nodiscard]] app::connector::ConnectorResponse execute_connector_processing_step(
        app::connector::IConnector& connector,
        const app::connector::ConnectorRequest& router_data
    ) const
    {
        return connector.authorize(router_data);
    }

    // update_fraud_check_response_with_frm_id: ResponseUpdate keeps frm_status / transaction id
    // / reason / score / metadata and advances last_step to Fulfillment. Locally the last_step is
    // persisted in `decision`; the other values are unchanged because they have no separate
    // columns (see shared contract requests). updated_at has no local column either.
    [[nodiscard]] bool update_fraud_check_response_with_frm_id(const FraudCheck& fraud_check) const
    {
        FraudCheck update = fraud_check;
        update.set_decision("Fulfillment");
        return m_context->repository().update_fraud_check(update);
    }

    // 1181-1189 find_payment_intent_by_payment_id_processor_merchant_id is the repository
    // find_payment_by_string plus the processor-merchant scope applied in frm_fulfillment_core.
    [[nodiscard]] std::optional<PaymentIntent> find_payment_intent_by_payment_id_processor_merchant_id(
        std::string_view payment_id
    ) const
    {
        return m_context->repository().find_payment_by_string(payment_id);
    }

    // 1196-1205 find_payment_attempt_by_payment_id_processor_merchant_id_attempt_id. The repository
    // has no composite lookup; filter by payment_id + processor merchant id deterministically.
    [[nodiscard]] std::optional<PaymentAttempt>
    find_payment_attempt_by_payment_id_processor_merchant_id_attempt_id(
        std::string_view processor_merchant_id,
        std::string_view payment_id
    ) const
    {
        for (const auto& attempt : m_context->repository().list_payment_attempts()) {
            if (attempt.get_payment_id() == payment_id
                && attempt.get_merchant_id() == processor_merchant_id) {
                return attempt;
            }
        }
        return std::nullopt;
    }

    // Upstream reads payment_attempt.active_frm_id (core/fraud_check.rs:1207). The local
    // PaymentAttemptRecord has no active_frm_id column, so the FRM id is the fraud_checks row's
    // natural key for this payment; a shared contract request asks for the column to be added.
    [[nodiscard]] std::string resolve_active_frm_id(
        const PaymentAttempt& payment_attempt,
        std::string_view processor_merchant_id
    ) const
    {
        for (const auto& record : m_context->repository().list_fraud_checks()) {
            if (record.get_payment_id() == payment_attempt.get_payment_id()
                && record.get_merchant_id() == processor_merchant_id) {
                return record.get_fraud_check_id();
            }
        }
        return {};
    }

    // 1209-1212 find_fraud_check_by_frm_id -- exact repository lookup by fraud_check_id.
    [[nodiscard]] std::optional<FraudCheck> find_fraud_check_by_frm_id(std::string_view frm_id) const
    {
        return m_context->repository().find_fraud_check_by_string(frm_id);
    }

    // (PreFrm && last_step == TransactionOrRecordRefund) || (PostFrm && last_step == CheckoutOrSale).
    // The local FraudCheckRecord has no frm_transaction_type / last_step columns; both are encoded
    // in `decision` as "<PreFrm|PostFrm>:<step>" (bare step accepted for legacy rows).
    [[nodiscard]] bool frm_flow_terminated(const FraudCheck& record) const
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
    [[nodiscard]] bool connector_transport_failed(const app::connector::ConnectorResponse& response) const
    {
        const auto& s = response.get_status();
        return response.get_retryable() || s.empty() || s == "timeout" || s == "connector_error";
    }

    // response.response Err layer: the connector executed and reported a failure.
    [[nodiscard]] bool connector_reported_error(const app::connector::ConnectorResponse& response) const
    {
        const auto& s = response.get_status();
        return s == "failed" || s == "error"
            || response.get_raw_json().find("\"error\"") != std::string::npos;
    }

    // ApiErrorResponse::ExternalConnectorError { code, message, connector, status_code, reason }
    // (core/fraud_check.rs:1294-1303, api_error_response.rs:390-396). The local error wire shape
    // is {"error":KIND,"message":TEXT}, so all five fields are carried in the deterministic
    // message; the connector error HTTP status is carried in status_code.
    [[nodiscard]] std::string external_connector_error_message(
        const app::connector::ConnectorResponse& response,
        std::string_view frm_name
    ) const
    {
        return std::string{"FRM_FULFILLMENT_FAILED: "} + response.get_message()
            + "; connector=" + std::string{frm_name}
            + "; status_code=" + connector_error_status_code(response)
            + "; reason=" + response.get_raw_json();
    }

    [[nodiscard]] std::string connector_error_status_code(
        const app::connector::ConnectorResponse& response
    ) const
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
    [[nodiscard]] Fulfillment build_fulfillment_response(
        const app::connector::ConnectorResponse& response,
        const Input& request
    ) const
    {
        Fulfillment result{};
        result.set_order_id(request.get_order_id());
        std::vector<std::string> ids{};
        for (const auto& item : request.get_fulfillments()) {
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
                for (const auto& element : *array) {
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

    // Rejects unknown root fields (serde deny_unknown_fields, fraud_check.rs:105) and enforces
    // the required fields of FrmFulfillmentRequest / Fulfillments / Destination / Address.
    [[nodiscard]] std::optional<Input> parse_and_validate_request(std::string_view body) const
    {
        auto generic = serde::Ser::decode_generic("application/json", body);
        if (!generic) {
            return std::nullopt;
        }
        const auto* root = std::get_if<interfaces::Value::Object>(&generic->get());
        if (root == nullptr) {
            return std::nullopt;
        }
        static constexpr std::array<std::string_view, 11> kAllowed{
            "payment_id", "order_id", "fulfillment_status", "fulfillments", "tracking_company",
            "tracking_numbers", "tracking_urls", "carrier", "fulfillment_method",
            "shipment_status", "shipped_at"};
        for (const auto& [key, value] : *root) {
            (void)value;
            if (std::find(kAllowed.begin(), kAllowed.end(), std::string_view{key}) == kAllowed.end()) {
                return std::nullopt;
            }
        }
        if (!has_string_field(*root, "payment_id") || !has_string_field(*root, "order_id")) {
            return std::nullopt;
        }
        const auto fulfillments = root->get("fulfillments");
        if (!fulfillments) {
            return std::nullopt;
        }
        const auto* array = std::get_if<interfaces::Value::Array>(&fulfillments->get());
        if (array == nullptr) {
            return std::nullopt;
        }
        for (const auto& element : *array) {
            const auto* item = std::get_if<interfaces::Value::Object>(&element.get());
            if (item == nullptr || !has_string_field(*item, "shipment_id")) {
                return std::nullopt;
            }
            const auto destination = item->get("destination");
            if (!destination) {
                return std::nullopt;
            }
            const auto* dest = std::get_if<interfaces::Value::Object>(&destination->get());
            if (dest == nullptr || !has_string_field(*dest, "full_name")) {
                return std::nullopt;
            }
            const auto address = dest->get("address");
            if (!address) {
                return std::nullopt;
            }
            const auto* addr = std::get_if<interfaces::Value::Object>(&address->get());
            if (addr == nullptr || !has_string_field(*addr, "street_address")
                || !has_string_field(*addr, "postal_code") || !has_string_field(*addr, "city")
                || !has_string_field(*addr, "province_code") || !has_string_field(*addr, "country_code")) {
                return std::nullopt;
            }
        }
        if (const auto status = root->get("fulfillment_status")) {
            if (const auto* text = std::get_if<std::string>(&status->get())) {
                static constexpr std::array<std::string_view, 4> kStatuses{
                    "PARTIAL", "COMPLETE", "REPLACEMENT", "CANCELED"};
                if (std::find(kStatuses.begin(), kStatuses.end(), std::string_view{*text})
                    == kStatuses.end()) {
                    return std::nullopt;
                }
            }
        }
        return serde::Ser::deserialize<Input>("application/json", body);
    }

    [[nodiscard]] bool has_string_field(const interfaces::Value::Object& object, std::string_view key) const
    {
        const auto field = object.get(key);
        return field.has_value() && std::holds_alternative<std::string>(field->get());
    }

    [[nodiscard]] std::string processor_merchant_id_of(const PaymentIntent& intent) const
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
        app::store::webhook_events::WebhookEventRecord record{};
        record.set_event_id(std::string{event_id});
        record.set_merchant_id(merchant_id);
        record.set_event_type("frm_fulfillment");
        record.set_event_class(org_id.empty()
            ? std::string{"frm_fulfillment"}
            : std::string{"frm_fulfillment:org:"} + std::string{org_id});
        record.set_status(connector_status.empty() ? std::string{"fulfilled"} : std::string{connector_status});
        record.set_payload(std::string{payload});
        record.set_delivery_attempts(0);
        record.set_created_at(std::to_string(std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::system_clock::now().time_since_epoch()).count()));
        (void)m_context->repository().save_webhook_event(std::move(record));
    }

    [[nodiscard]] core::error::AppError make_error(
        core::error::AppErrorKind kind,
        std::string message
    ) const
    {
        return core::error::AppErrorBuilder{}(kind, std::move(message));
    }

    void send_error(
        interfaces::io::IResponse& res,
        SendFn& send,
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
        SendFn& send,
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
