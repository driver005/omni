module;
export module app_handlers:dummy_connector;

import std;
import interfaces;
import core_router;
import core_serde;
import core_cache;
import core_error;
import utils_openapi;
import app_model;
import app_dto;
import app_context;
import app_auth;
import app_http;
import app_metrics;
import app_services;

// ---------------------------------------------------------------------------
// Local typed DTOs. Upstream declares these in routes/dummy_connector/types.rs;
// this domain has no app_dto partition, so the serializable shapes live here.
// They are declared BEFORE the handler class so serde::Serializable<T> is visible
// at every encode_json / deserialize<T> instantiation site below.
// ---------------------------------------------------------------------------
export namespace app::handlers::dummy_connector {

/** @brief Upstream `DummyConnectors` enum (types.rs:28-42), lowercase wire form. */
enum class DummyConnectorKind : std::uint8_t
{
    Phonypay,
    Fauxpay,
    Pretendpay,
    Stripe,
    Adyen,
    Checkout,
    Paypal,
};

/** @brief Typed `DummyConnectorPaymentRequest` (types.rs:114-121). */
class DummyPaymentRequestData
{
public:
    void set_amount(std::int64_t value) noexcept { m_amount = value; }
    [[nodiscard]] std::int64_t get_amount() const noexcept { return m_amount; }
    void set_currency(std::string value) { m_currency = std::move(value); }
    [[nodiscard]] const std::string& get_currency() const noexcept { return m_currency; }
    void set_payment_method_type(std::string value) { m_payment_method_type = std::move(value); }
    [[nodiscard]] const std::string& get_payment_method_type() const noexcept { return m_payment_method_type; }
    void set_return_url(std::string value) { m_return_url = std::move(value); }
    [[nodiscard]] const std::string& get_return_url() const noexcept { return m_return_url; }
    void set_connector(std::string value) { m_connector = std::move(value); }
    [[nodiscard]] const std::string& get_connector() const noexcept { return m_connector; }
    // Upstream Secret<String> / CardNumber fields. Kept typed but never echoed;
    // masked to a fixed placeholder when materialized into persisted data.
    void set_secret(std::string value) { m_secret = std::move(value); }
    [[nodiscard]] const std::string& get_secret() const noexcept { return m_secret; }
    void set_card_number(std::string value) { m_card_number = std::move(value); }
    [[nodiscard]] const std::string& get_card_number() const noexcept { return m_card_number; }

private:
    std::int64_t m_amount{0};
    std::string m_currency{};
    std::string m_payment_method_type{};
    std::string m_return_url{};
    std::string m_connector{};
    std::string m_secret{};
    std::string m_card_number{};
};

/** @brief Typed `DummyConnectorRefundRequest` (types.rs:370-374). */
class DummyRefundRequestData
{
public:
    void set_amount(std::int64_t value) noexcept { m_amount = value; }
    [[nodiscard]] std::int64_t get_amount() const noexcept { return m_amount; }
    void set_currency(std::string value) { m_currency = std::move(value); }
    [[nodiscard]] const std::string& get_currency() const noexcept { return m_currency; }

private:
    std::int64_t m_amount{0};
    std::string m_currency{};
};

/** @brief Upstream `DummyConnectorPaymentData` (types.rs:290-298), the Redis value. */
class DummyPaymentData
{
public:
    void set_id(std::string value) { m_id = std::move(value); }
    [[nodiscard]] const std::string& get_id() const noexcept { return m_id; }
    void set_attempt_id(std::string value) { m_attempt_id = std::move(value); }
    [[nodiscard]] const std::string& get_attempt_id() const noexcept { return m_attempt_id; }
    void set_status(std::string value) { m_status = std::move(value); }
    [[nodiscard]] const std::string& get_status() const noexcept { return m_status; }
    void set_amount(std::int64_t value) noexcept { m_amount = value; }
    [[nodiscard]] std::int64_t get_amount() const noexcept { return m_amount; }
    void set_eligible_amount(std::int64_t value) noexcept { m_eligible_amount = value; }
    [[nodiscard]] std::int64_t get_eligible_amount() const noexcept { return m_eligible_amount; }
    void set_currency(std::string value) { m_currency = std::move(value); }
    [[nodiscard]] const std::string& get_currency() const noexcept { return m_currency; }
    void set_created(std::string value) { m_created = std::move(value); }
    [[nodiscard]] const std::string& get_created() const noexcept { return m_created; }
    void set_payment_method_type(std::string value) { m_payment_method_type = std::move(value); }
    [[nodiscard]] const std::string& get_payment_method_type() const noexcept { return m_payment_method_type; }
    void set_connector(std::string value) { m_connector = std::move(value); }
    [[nodiscard]] const std::string& get_connector() const noexcept { return m_connector; }
    void set_return_url(std::string value) { m_return_url = std::move(value); }
    [[nodiscard]] const std::string& get_return_url() const noexcept { return m_return_url; }
    void set_next_action(std::string value) { m_next_action = std::move(value); }
    [[nodiscard]] const std::string& get_next_action() const noexcept { return m_next_action; }

private:
    std::string m_id{};
    std::string m_attempt_id{};
    std::string m_status{};
    std::int64_t m_amount{0};
    std::int64_t m_eligible_amount{0};
    std::string m_currency{};
    std::string m_created{};
    std::string m_payment_method_type{};
    std::string m_connector{};
    std::string m_return_url{};
    std::string m_next_action{"null"};
};

/** @brief Upstream `DummyConnectorRefundResponse` (types.rs:376-385), the Redis value. */
class DummyRefundData
{
public:
    void set_id(std::string value) { m_id = std::move(value); }
    [[nodiscard]] const std::string& get_id() const noexcept { return m_id; }
    void set_payment_id(std::string value) { m_payment_id = std::move(value); }
    [[nodiscard]] const std::string& get_payment_id() const noexcept { return m_payment_id; }
    void set_status(std::string value) { m_status = std::move(value); }
    [[nodiscard]] const std::string& get_status() const noexcept { return m_status; }
    void set_currency(std::string value) { m_currency = std::move(value); }
    [[nodiscard]] const std::string& get_currency() const noexcept { return m_currency; }
    void set_created(std::string value) { m_created = std::move(value); }
    [[nodiscard]] const std::string& get_created() const noexcept { return m_created; }
    void set_payment_amount(std::int64_t value) noexcept { m_payment_amount = value; }
    [[nodiscard]] std::int64_t get_payment_amount() const noexcept { return m_payment_amount; }
    void set_refund_amount(std::int64_t value) noexcept { m_refund_amount = value; }
    [[nodiscard]] std::int64_t get_refund_amount() const noexcept { return m_refund_amount; }

private:
    std::string m_id{};
    std::string m_payment_id{};
    std::string m_status{};
    std::string m_currency{};
    std::string m_created{};
    std::int64_t m_payment_amount{0};
    std::int64_t m_refund_amount{0};
};

} // namespace app::handlers::dummy_connector

template<>
struct serde::Serializable<::app::handlers::dummy_connector::DummyPaymentRequestData>
{
    static constexpr auto fields()
    {
        using T = ::app::handlers::dummy_connector::DummyPaymentRequestData;
        return std::tuple{
            serde::FieldDesc<"amount", &T::get_amount, &T::set_amount, serde::FieldOptions{}>{},
            serde::FieldDesc<"currency", &T::get_currency, &T::set_currency, serde::FieldOptions{}>{},
            serde::FieldDesc<"payment_method_type", &T::get_payment_method_type, &T::set_payment_method_type, serde::FieldOptions{}>{},
            serde::FieldDesc<"return_url", &T::get_return_url, &T::set_return_url, serde::FieldOptions{}>{},
            serde::FieldDesc<"connector", &T::get_connector, &T::set_connector, serde::FieldOptions{}>{},
            serde::FieldDesc<"secret", &T::get_secret, &T::set_secret, serde::FieldOptions{}>{},
            serde::FieldDesc<"card_number", &T::get_card_number, &T::set_card_number, serde::FieldOptions{}>{},
        };
    }
    static constexpr std::string_view table_name() { return "dummy_connector_payment_request"; }
};

template<>
struct serde::Serializable<::app::handlers::dummy_connector::DummyRefundRequestData>
{
    static constexpr auto fields()
    {
        using T = ::app::handlers::dummy_connector::DummyRefundRequestData;
        return std::tuple{
            serde::FieldDesc<"amount", &T::get_amount, &T::set_amount, serde::FieldOptions{}>{},
            serde::FieldDesc<"currency", &T::get_currency, &T::set_currency, serde::FieldOptions{}>{},
        };
    }
    static constexpr std::string_view table_name() { return "dummy_connector_refund_request"; }
};

template<>
struct serde::Serializable<::app::handlers::dummy_connector::DummyPaymentData>
{
    static constexpr auto fields()
    {
        using T = ::app::handlers::dummy_connector::DummyPaymentData;
        return std::tuple{
            serde::FieldDesc<"id", &T::get_id, &T::set_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"attempt_id", &T::get_attempt_id, &T::set_attempt_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"status", &T::get_status, &T::set_status, serde::FieldOptions{}>{},
            serde::FieldDesc<"amount", &T::get_amount, &T::set_amount, serde::FieldOptions{}>{},
            serde::FieldDesc<"eligible_amount", &T::get_eligible_amount, &T::set_eligible_amount, serde::FieldOptions{}>{},
            serde::FieldDesc<"currency", &T::get_currency, &T::set_currency, serde::FieldOptions{}>{},
            serde::FieldDesc<"created", &T::get_created, &T::set_created, serde::FieldOptions{}>{},
            serde::FieldDesc<"payment_method_type", &T::get_payment_method_type, &T::set_payment_method_type, serde::FieldOptions{}>{},
            serde::FieldDesc<"connector", &T::get_connector, &T::set_connector, serde::FieldOptions{}>{},
            serde::FieldDesc<"return_url", &T::get_return_url, &T::set_return_url, serde::FieldOptions{}>{},
            serde::FieldDesc<"next_action", &T::get_next_action, &T::set_next_action, serde::FieldOptions{}>{},
        };
    }
    static constexpr std::string_view table_name() { return "dummy_connector_payment_data"; }
};

template<>
struct serde::Serializable<::app::handlers::dummy_connector::DummyRefundData>
{
    static constexpr auto fields()
    {
        using T = ::app::handlers::dummy_connector::DummyRefundData;
        return std::tuple{
            serde::FieldDesc<"id", &T::get_id, &T::set_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"payment_id", &T::get_payment_id, &T::set_payment_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"status", &T::get_status, &T::set_status, serde::FieldOptions{}>{},
            serde::FieldDesc<"currency", &T::get_currency, &T::set_currency, serde::FieldOptions{}>{},
            serde::FieldDesc<"created", &T::get_created, &T::set_created, serde::FieldOptions{}>{},
            serde::FieldDesc<"payment_amount", &T::get_payment_amount, &T::set_payment_amount, serde::FieldOptions{}>{},
            serde::FieldDesc<"refund_amount", &T::get_refund_amount, &T::set_refund_amount, serde::FieldOptions{}>{},
        };
    }
    static constexpr std::string_view table_name() { return "dummy_connector_refund_data"; }
};

export namespace app::handlers::dummy_connector {

class DummyConnectorRouteHandlers
{
public:
    void handle_dummy_connector_authorize_payment(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        // NoAuth rule from the registry (dummy_connector.rs:32). No Host guard on
        // the authorize/complete routes (app.rs:857-862).
        if (const auto auth_error = authorize(req, "dummy_connector_authorize_payment")) {
            send_error(res, send, *auth_error);
            return;
        }
        // Upstream payment_authorize (core.rs:59-85) is the one flow with no
        // tokio_mock_sleep, so no latency is applied here.

        const auto attempt_id = last_segment(req);
        if (attempt_id.empty()) {
            send_error(res, send, make_error(core::error::AppErrorKind::INVALID_REQUEST, "attempt_id is required"));
            return;
        }

        // Upstream get_payment_data_by_attempt_id(core.rs:63): resolve the attempt key,
        // then load the payment value; emit the HTML authorize page (TEXT_HTML).
        const auto attempt = m_context->cache().get(attempt_key(attempt_id));
        if (!attempt.hit || attempt.value_or<std::string>("").empty()) {
            m_context->metrics().record_flow("DummyConnectorAuthorizePaymentExpired");
            send_html(res, send, expired_page(), true);
            return;
        }
        const auto payment_id = attempt.value_or<std::string>("");
        const auto stored = load_payment(payment_id);
        if (!stored) {
            m_context->metrics().record_flow("DummyConnectorAuthorizePaymentExpired");
            send_html(res, send, expired_page(), true);
            return;
        }

        // Authorize marker with its own short TTL (upstream authorize_ttl).
        m_context->cache().put(
            "dummy_connector.authorize:" + attempt_id,
            core::cache::to_cache_value(payment_id),
            ttl("authorize")
        );

        m_context->metrics().record_flow("DummyConnectorAuthorizePayment");
        m_context->metrics().counter("DUMMY_CONNECTOR_AUTHORIZE").increment();
        send_html(res, send, authorize_page(attempt_id, base_url(req), stored->get_payment_method_type()), true);
    }

    void handle_dummy_connector_complete_payment(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto auth_error = authorize(req, "dummy_connector_complete_payment")) {
            send_error(res, send, *auth_error);
            return;
        }
        apply_mock_latency();

        const auto attempt_id = last_segment(req);
        if (attempt_id.empty()) {
            send_error(res, send, make_error(core::error::AppErrorKind::INVALID_REQUEST, "attempt_id is required"));
            return;
        }
        const auto confirm_value = app::http::RequestQuery{}(req).get("confirm").value_or("");
        const bool confirm = confirm_value == "true" || confirm_value == "1";

        const auto attempt = m_context->cache().get(attempt_key(attempt_id));
        const auto payment_id = attempt.hit ? attempt.value_or<std::string>("") : std::string{};
        m_context->cache().erase(attempt_key(attempt_id)); // core.rs:106 - delete attempt key

        std::string return_url = m_context->config_store()
                                     .get_value("dummy_connector", "default_return_url")
                                     .value_or(std::string{"http://localhost:8080/dummy-connector/complete"});
        if (!payment_id.empty()) {
            if (auto payment = load_payment(payment_id)) {
                payment->set_status(confirm ? "succeeded" : "failed");
                payment->set_next_action(std::string{"null"});
                save_payment(*payment);
                if (!payment->get_return_url().empty()) {
                    return_url = payment->get_return_url();
                }
            }
        }

        m_context->metrics().record_flow("DummyConnectorCompletePayment");
        m_context->metrics().counter("DUMMY_CONNECTOR_COMPLETE").increment();

        // JsonForRedirection(RedirectionResponse) - core.rs:127-152.
        auto text = std::string{R"({"http_method":"GET","params":[],"headers":[],"return_url_with_query_params":)"}
            + serde::Ser::encode_json(return_url) + "}";
        send_json(res, send, text, true);
    }

    void handle_dummy_connector_payment(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto auth_error = authorize(req, "dummy_connector_payment")) {
            send_error(res, send, *auth_error);
            return;
        }
        if (const auto host_error = require_local_host(req)) {
            send_error(res, send, *host_error);
            return;
        }
        apply_mock_latency();

        const auto body = req.body_as_string();
        if (body.empty()) {
            send_error(res, send, make_error(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }
        auto request = parse_payment_request(body);
        if (!request) {
            send_error(res, send, request.error());
            return;
        }
        const auto& input = *request;

        // Upstream generates dummy_* ids (consts.rs:1-2), not a request-body hash.
        const auto payment_id = m_context->ids().payment_id("dummy_pay_");
        const auto attempt_id = m_context->ids().payment_id("dummy_attempt_");

        // process_payment_attempt (types.rs:100-125): build the payment value with
        // eligible_amount seeded from amount, connector enum and PM-specific next action.
        DummyPaymentData data{};
        data.set_id(payment_id);
        data.set_attempt_id(attempt_id);
        data.set_status(std::string{"processing"});
        data.set_amount(input.get_amount());
        data.set_eligible_amount(input.get_amount());
        data.set_currency(input.get_currency());
        data.set_created(now_stamp());
        const std::string pm_type = input.get_payment_method_type().empty()
                                        ? std::string{"card"}
                                        : input.get_payment_method_type();
        data.set_payment_method_type(pm_type);
        data.set_connector(connector_name(input.get_connector()));
        data.set_return_url(input.get_return_url());
        data.set_next_action(next_action_json(pm_type, attempt_id, base_url(req)));

        // Redis substitute: store with TTLs, attempt-key -> payment id + payment value.
        save_payment(data);
        m_context->cache().put(
            attempt_key(attempt_id),
            core::cache::to_cache_value(payment_id),
            ttl("attempt")
        );

        m_context->metrics().record_flow("DummyConnectorPayment");
        m_context->metrics().counter("DUMMY_CONNECTOR_PAYMENT").increment();
        send_json(res, send, render_payment_response(data), true);
    }

    void handle_dummy_connector_payment_data(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto auth_error = authorize(req, "dummy_connector_payment_data")) {
            send_error(res, send, *auth_error);
            return;
        }
        if (const auto host_error = require_local_host(req)) {
            send_error(res, send, *host_error);
            return;
        }
        apply_mock_latency();

        const auto payment_id = last_segment(req);
        if (payment_id.empty()) {
            send_error(res, send, make_error(core::error::AppErrorKind::INVALID_REQUEST, "payment_id is required"));
            return;
        }
        const auto stored = load_payment(payment_id);
        if (!stored) {
            send_error(res, send, make_error(core::error::AppErrorKind::NOT_FOUND, "payment not found"));
            return;
        }

        m_context->metrics().record_flow("DummyConnectorPaymentData");
        m_context->metrics().counter("DUMMY_CONNECTOR_PAYMENT_DATA").increment();
        send_json(res, send, render_payment_response(*stored), true);
    }

    void handle_dummy_connector_refund(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto auth_error = authorize(req, "dummy_connector_refund")) {
            send_error(res, send, *auth_error);
            return;
        }
        if (const auto host_error = require_local_host(req)) {
            send_error(res, send, *host_error);
            return;
        }
        apply_mock_latency();

        // `/:payment_id/refund`: payment id is the segment before the trailing `refund`.
        const auto segments = app::http::RequestPath{}(req);
        if (segments.count() < 2) {
            send_error(res, send, make_error(core::error::AppErrorKind::INVALID_REQUEST, "payment_id is required"));
            return;
        }
        const auto payment_id = std::string{segments.at(segments.count() - 2).value_or("")};
        if (payment_id.empty()) {
            send_error(res, send, make_error(core::error::AppErrorKind::INVALID_REQUEST, "payment_id is required"));
            return;
        }

        const auto body = req.body_as_string();
        if (body.empty()) {
            send_error(res, send, make_error(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }
        auto request = parse_refund_request(body);
        if (!request) {
            send_error(res, send, request.error());
            return;
        }
        const auto& input = *request;

        auto payment = load_payment(payment_id);
        if (!payment) {
            send_error(res, send, make_error(core::error::AppErrorKind::NOT_FOUND, "payment not found"));
            return;
        }

        // is_eligible_for_refund (types.rs:301-315): eligible_amount first, then status.
        if (payment->get_eligible_amount() < input.get_amount()) {
            send_error(res, send, make_error(
                core::error::AppErrorKind::VALIDATION_FAILED,
                "RefundAmountExceedsPaymentAmount: eligible amount is lesser than refund amount"
            ));
            return;
        }
        if (payment->get_status() != "succeeded") {
            send_error(res, send, make_error(
                core::error::AppErrorKind::PRECONDITION_FAILED,
                "PaymentNotSuccessful: payment is not successful to process the refund"
            ));
            return;
        }

        // Decrement eligible_amount and persist the updated payment value (core.rs:180).
        payment->set_eligible_amount(payment->get_eligible_amount() - input.get_amount());
        save_payment(*payment);

        DummyRefundData refund{};
        refund.set_id(m_context->ids().payment_id("dummy_ref_"));
        refund.set_payment_id(payment_id);
        refund.set_status(std::string{"succeeded"});
        refund.set_currency(payment->get_currency());
        refund.set_created(now_stamp());
        refund.set_payment_amount(payment->get_amount());
        refund.set_refund_amount(input.get_amount());
        m_context->cache().put(
            refund_key(refund.get_id()),
            core::cache::to_cache_value(serde::Ser::encode_json(refund)),
            ttl("refund")
        );

        m_context->metrics().record_flow("DummyConnectorRefund");
        m_context->metrics().counter("DUMMY_CONNECTOR_REFUND").increment();
        send_json(res, send, render_refund_response(refund), true);
    }

    void handle_dummy_connector_refund_data(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto auth_error = authorize(req, "dummy_connector_refund_data")) {
            send_error(res, send, *auth_error);
            return;
        }
        if (const auto host_error = require_local_host(req)) {
            send_error(res, send, *host_error);
            return;
        }
        apply_mock_latency();

        const auto refund_id = last_segment(req);
        if (refund_id.empty()) {
            send_error(res, send, make_error(core::error::AppErrorKind::INVALID_REQUEST, "refund_id is required"));
            return;
        }

        const auto stored = m_context->cache().get(refund_key(refund_id));
        if (!stored.hit) {
            send_error(res, send, make_error(core::error::AppErrorKind::NOT_FOUND, "RefundNotFound"));
            return;
        }
        auto refund = serde::Ser::deserialize<DummyRefundData>("application/json", stored.value_or<std::string>("{}"));
        if (!refund) {
            send_error(res, send, make_error(core::error::AppErrorKind::INTERNAL_ERROR, "InternalServerError: corrupt refund data"));
            return;
        }

        m_context->metrics().record_flow("DummyConnectorRefundData");
        m_context->metrics().counter("DUMMY_CONNECTOR_REFUND_DATA").increment();
        send_json(res, send, render_refund_response(*refund), true);
    }

    void install(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "/dummy-connector"}
            .add_route(
                Route{"/authorize/:attempt_id"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_dummy_connector_authorize_payment(req, res, std::move(send));
                    })
                    .summary("Hyperswitch dummy_connector_authorize_payment")
            )
            .add_route(
                Route{"/complete/:attempt_id"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_dummy_connector_complete_payment(req, res, std::move(send));
                    })
                    .summary("Hyperswitch dummy_connector_complete_payment")
            )
            .add_route(
                Route{"/payment"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_dummy_connector_payment(req, res, std::move(send));
                    })
                    .summary("Hyperswitch dummy_connector_payment")
            )
            .add_route(
                Route{"/payments/:payment_id"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_dummy_connector_payment_data(req, res, std::move(send));
                    })
                    .summary("Hyperswitch dummy_connector_payment_data")
            )
            .add_route(
                Route{"/:payment_id/refund"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_dummy_connector_refund(req, res, std::move(send));
                    })
                    .summary("Hyperswitch dummy_connector_refund")
            )
            .add_route(
                Route{"/refunds/:refund_id"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_dummy_connector_refund_data(req, res, std::move(send));
                    })
                    .summary("Hyperswitch dummy_connector_refund_data")
            );
    }

    explicit DummyConnectorRouteHandlers(app::context::AppContext& context) noexcept
        : m_context{&context}
    {
    }

private:
    // -- auth / guards ------------------------------------------------------

    [[nodiscard]] std::optional<core::error::AppError> authorize(
        interfaces::io::IRequest& req,
        std::string_view operation
    ) const
    {
        // All six operations are NO_AUTH in the registry (registry.cppm:112-117),
        // so authorize_route passes through without a bearer token - matching
        // auth::NoAuth in dummy_connector.rs. Every upstream route also declares
        // #[instrument(flow=..., skip_all)] and LockAction::NotApplicable; the
        // record_flow/counter calls in each handler and the absence of any
        // locks().acquire() here are that contract's local equivalent.
        const auto rule = m_context->auth_rules().rule_for(operation);
        return m_context->authorizer().authorize_route(app::auth::AuthContextBuilder{}(req), rule);
    }

    [[nodiscard]] std::optional<core::error::AppError> require_local_host(
        interfaces::io::IRequest& req
    ) const
    {
        // Upstream gates payment/payments/refund/refunds behind Host("localhost")
        // unless external_access_dc is set (app.rs:857-862). A missing Host header
        // (local test harness) is treated as local; an explicit foreign host is denied.
        const auto host = req.find_header("host");
        if (host.empty()) {
            return std::nullopt;
        }
        const bool local = host == "localhost" || host.starts_with("localhost:")
                        || host == "127.0.0.1" || host.starts_with("127.0.0.1:");
        if (local) {
            return std::nullopt;
        }
        const auto external = m_context->config_store()
                                  .get_value("dummy_connector", "external_access_dc")
                                  .value_or(std::string{"false"});
        if (external == "true" || external == "1") {
            return std::nullopt;
        }
        return make_error(core::error::AppErrorKind::FORBIDDEN, "DummyConnectorExternalAccessDisabled");
    }

    [[nodiscard]] core::error::AppError make_error(
        core::error::AppErrorKind kind,
        std::string message
    ) const
    {
        return core::error::AppErrorBuilder{}.operator()(kind, std::move(message));
    }

    // -- config-driven deterministic substitutes ---------------------------

    void apply_mock_latency() const
    {
        // Deterministic substitute for tokio_mock_sleep(duration, tolerance) on
        // every flow (core.rs:17-21). Latency is opt-in via config; default 0 so
        // local tests stay fast. Capped to avoid unbounded sleeps.
        const auto value = m_context->config_store().get_value("dummy_connector", "mock_sleep_ms");
        if (!value) {
            return;
        }
        std::int64_t millis{0};
        const auto parsed = std::from_chars(value->data(), value->data() + value->size(), millis);
        if (parsed.ec != std::errc{} || parsed.ptr != value->data() + value->size()) {
            return;
        }
        millis = std::clamp<std::int64_t>(millis, 0, 1000);
        if (millis > 0) {
            std::this_thread::sleep_for(std::chrono::milliseconds(millis));
        }
    }

    [[nodiscard]] core::cache::Ttl ttl(std::string_view flow) const
    {
        // Local stand-ins for authorize_ttl / payment_ttl / refund_ttl (core.rs:27-40).
        if (flow == "authorize") {
            return core::cache::Ttl{}.seconds(300);
        }
        if (flow == "attempt") {
            return core::cache::Ttl{}.minutes(30);
        }
        if (flow == "refund") {
            return core::cache::Ttl{}.hours(24);
        }
        return core::cache::Ttl{}.hours(1); // payment value + status
    }

    [[nodiscard]] std::string base_url(interfaces::io::IRequest& req) const
    {
        const auto host = req.find_header("host");
        if (!host.empty()) {
            return std::string{"http://"} + std::string{host};
        }
        return m_context->config_store()
                   .get_value("dummy_connector", "base_url")
                   .value_or(std::string{"http://localhost:8080"});
    }

    // -- path helpers -------------------------------------------------------

    [[nodiscard]] std::string last_segment(interfaces::io::IRequest& req) const
    {
        // RequestPath splits the full mounted path (/dummy-connector/...). The id
        // is always the last non-empty segment for authorize/complete/retrieve.
        const auto segments = app::http::RequestPath{}(req);
        if (segments.count() == 0) {
            return {};
        }
        return std::string{segments.at(segments.count() - 1).value_or("")};
    }

    // -- typed request parsing + validation --------------------------------

    [[nodiscard]] std::string read_string(const interfaces::Value::Object& object, const char* key) const
    {
        const auto field = object.get(key);
        if (!field) {
            return {};
        }
        const auto* text = std::get_if<std::string>(&field->get());
        return text != nullptr ? *text : std::string{};
    }

    [[nodiscard]] std::int64_t read_int(const interfaces::Value::Object& object, const char* key) const
    {
        const auto field = object.get(key);
        if (!field) {
            return 0;
        }
        if (const auto* number = std::get_if<std::int64_t>(&field->get())) {
            return *number;
        }
        if (const auto* number = std::get_if<double>(&field->get())) {
            return static_cast<std::int64_t>(*number);
        }
        return 0;
    }

    [[nodiscard]] std::expected<DummyPaymentRequestData, core::error::AppError> parse_payment_request(
        std::string_view body
    ) const
    {
        auto tree = serde::Ser::decode_generic("application/json", body);
        if (!tree) {
            return std::unexpected{make_error(core::error::AppErrorKind::INVALID_JSON, "invalid JSON")};
        }
        const auto* root = std::get_if<interfaces::Value::Object>(&tree->get());
        if (root == nullptr) {
            return std::unexpected{make_error(core::error::AppErrorKind::INVALID_JSON, "invalid JSON")};
        }
        DummyPaymentRequestData request{};
        request.set_amount(read_int(*root, "amount"));
        request.set_currency(read_string(*root, "currency"));
        request.set_payment_method_type(read_string(*root, "payment_method_type"));
        request.set_return_url(read_string(*root, "return_url"));
        request.set_connector(read_string(*root, "connector"));
        request.set_secret(read_string(*root, "secret"));
        request.set_card_number(read_string(*root, "card_number"));
        if (request.get_amount() <= 0) {
            return std::unexpected{make_error(core::error::AppErrorKind::VALIDATION_FAILED, "MissingRequiredField: amount")};
        }
        if (request.get_currency().empty()) {
            return std::unexpected{make_error(core::error::AppErrorKind::VALIDATION_FAILED, "MissingRequiredField: currency")};
        }
        return request;
    }

    [[nodiscard]] std::expected<DummyRefundRequestData, core::error::AppError> parse_refund_request(
        std::string_view body
    ) const
    {
        auto tree = serde::Ser::decode_generic("application/json", body);
        if (!tree) {
            return std::unexpected{make_error(core::error::AppErrorKind::INVALID_JSON, "invalid JSON")};
        }
        const auto* root = std::get_if<interfaces::Value::Object>(&tree->get());
        if (root == nullptr) {
            return std::unexpected{make_error(core::error::AppErrorKind::INVALID_JSON, "invalid JSON")};
        }
        DummyRefundRequestData request{};
        request.set_amount(read_int(*root, "amount"));
        request.set_currency(read_string(*root, "currency"));
        if (request.get_amount() <= 0) {
            return std::unexpected{make_error(core::error::AppErrorKind::VALIDATION_FAILED, "MissingRequiredField: amount")};
        }
        return request;
    }

    // -- state machine / domain helpers ------------------------------------

    [[nodiscard]] std::string connector_name(std::string_view raw) const
    {
        if (raw == "fauxpay") {
            return "fauxpay";
        }
        if (raw == "pretendpay") {
            return "pretendpay";
        }
        if (raw == "stripe") {
            return "stripe";
        }
        if (raw == "adyen") {
            return "adyen";
        }
        if (raw == "checkout") {
            return "checkout";
        }
        if (raw == "paypal") {
            return "paypal";
        }
        return "phonypay"; // DummyConnectors default
    }

    [[nodiscard]] std::string next_action_json(
        std::string_view pm_type,
        std::string_view attempt_id,
        std::string_view base
    ) const
    {
        // process_payment_attempt picks a per-payment-method next action. The
        // dummy card flow is a 3DS redirect to the authorize page; other PMs
        // complete without a next action. Shaped as the upstream tagged enum
        // `DummyConnectorNextAction::RedirectToUrl` (types.rs:317-320).
        const bool redirect = pm_type == "card" || pm_type == "bank_transfer"
                           || pm_type == "upi" || pm_type.empty();
        if (!redirect) {
            return std::string{"null"};
        }
        const std::string url = std::string{base}
            + "/dummy-connector/authorize/" + std::string{attempt_id};
        return std::string{R"({"RedirectToUrl":)"} + serde::Ser::encode_json(url) + "}";
    }

    [[nodiscard]] std::string authorize_page(
        std::string_view attempt_id,
        std::string_view base,
        std::string_view pm_type
    ) const
    {
        const std::string action = std::string{base}
            + "/dummy-connector/complete/" + std::string{attempt_id} + "?confirm=true";
        const bool card = pm_type == "card" || pm_type.empty();
        // Deterministic substitute for get_authorize_page + THREE_DS_CSS
        // (utils.rs / consts.rs:3). The upstream CSS asset is an external static
        // file not present in this port; an inline stylesheet keeps the same
        // card-vs-other flow distinction.
        std::string page{R"(<!doctype html><html><head><meta charset="utf-8"><title>Dummy Connector - )"};
        page += card ? "3DS" : "Payment";
        page += R"(</title><style>)";
        page += R"(body{font-family:system-ui;margin:3rem;color:#111})";
        page += R"(form{max-width:22rem;padding:1.5rem;border:1px solid #ddd;border-radius:8px})";
        page += R"(button{padding:.6rem 1rem;background:#111;color:#fff;border:0;border-radius:4px;cursor:pointer})";
        page += R"(</style></head><body><h1>Dummy Connector )";
        page += card ? "3DS Authentication" : "Payment Confirmation";
        page += R"(</h1><form method="get" action=")";
        page += action;
        page += R"("><p>Attempt: )";
        page += serde::Ser::encode_json(std::string{attempt_id});
        page += R"(</p><button type="submit">Confirm payment</button></form></body></html>)";
        return page;
    }

    [[nodiscard]] std::string expired_page() const
    {
        // Deterministic substitute for get_expired_page (utils.rs): same HTML
        // FileData(TEXT_HTML) shape, expired message.
        return std::string{R"(<!doctype html><html><head><meta charset="utf-8"><title>Expired</title></head><body><h1>Payment attempt expired</h1></body></html>)"};
    }

    // -- persistence (Redis substitute: cache + TTL) -----------------------

    [[nodiscard]] std::string attempt_key(std::string_view attempt_id) const
    {
        return "dummy_connector.attempt:" + std::string{attempt_id};
    }

    [[nodiscard]] std::string payment_key(std::string_view payment_id) const
    {
        return "dummy_connector.payment:" + std::string{payment_id};
    }

    [[nodiscard]] std::string refund_key(std::string_view refund_id) const
    {
        return "dummy_connector.refund:" + std::string{refund_id};
    }

    void save_payment(const DummyPaymentData& data) const
    {
        m_context->cache().put(
            payment_key(data.get_id()),
            core::cache::to_cache_value(serde::Ser::encode_json(data)),
            ttl("payment")
        );
    }

    [[nodiscard]] std::optional<DummyPaymentData> load_payment(std::string_view payment_id) const
    {
        const auto stored = m_context->cache().get(payment_key(payment_id));
        if (!stored.hit) {
            return std::nullopt;
        }
        auto decoded = serde::Ser::deserialize<DummyPaymentData>("application/json", stored.value_or<std::string>("{}"));
        if (!decoded) {
            return std::nullopt;
        }
        return *decoded;
    }

    [[nodiscard]] std::string now_stamp() const
    {
        return std::to_string(
            std::chrono::duration_cast<std::chrono::seconds>(
                std::chrono::system_clock::now().time_since_epoch()
            ).count()
        );
    }

    // -- typed wire responses ----------------------------------------------

    [[nodiscard]] std::string render_payment_response(const DummyPaymentData& data) const
    {
        // DummyConnectorPaymentResponse (types.rs:323-333). Values come from the
        // typed record; next_action is a pre-built raw JSON object/null.
        return std::string{R"({"status":)"} + serde::Ser::encode_json(data.get_status())
            + R"(,"id":)" + serde::Ser::encode_json(data.get_id())
            + R"(,"attempt_id":)" + serde::Ser::encode_json(data.get_attempt_id())
            + R"(,"amount":)" + std::to_string(data.get_amount())
            + R"(,"currency":)" + serde::Ser::encode_json(data.get_currency())
            + R"(,"created":)" + serde::Ser::encode_json(data.get_created())
            + R"(,"payment_method_type":)" + serde::Ser::encode_json(data.get_payment_method_type())
            + R"(,"connector":)" + serde::Ser::encode_json(data.get_connector())
            + R"(,"next_action":)" + data.get_next_action() + "}";
    }

    [[nodiscard]] std::string render_refund_response(const DummyRefundData& refund) const
    {
        // DummyConnectorRefundResponse (types.rs:376-385).
        return std::string{R"({"status":)"} + serde::Ser::encode_json(refund.get_status())
            + R"(,"id":)" + serde::Ser::encode_json(refund.get_id())
            + R"(,"payment_id":)" + serde::Ser::encode_json(refund.get_payment_id())
            + R"(,"currency":)" + serde::Ser::encode_json(refund.get_currency())
            + R"(,"created":)" + serde::Ser::encode_json(refund.get_created())
            + R"(,"payment_amount":)" + std::to_string(refund.get_payment_amount())
            + R"(,"refund_amount":)" + std::to_string(refund.get_refund_amount())
            + "}";
    }

    // -- response sinks -----------------------------------------------------

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

    void send_html(
        interfaces::io::IResponse& res,
        std::function<void()> send,
        std::string_view html,
        bool ok
    ) const
    {
        auto bytes = serde::Ser::serialize_raw("text/html", html);
        res.add_content_type("text/html; charset=utf-8");
        res.set_status(ok ? interfaces::io::types::Status::OK
                          : interfaces::io::types::Status::BAD_REQUEST);
        res.set_body(std::move(bytes));
        send();
    }

    app::context::AppContext* m_context{nullptr};
};

} // namespace app::handlers::dummy_connector
