export module app_payment:dummy_connector_input;

import std;
import interfaces;
import core_serde;
import core_error;
import app_context;
import app_auth;
import app_http;

// ---------------------------------------------------------------------------
// Local, faithful port of upstream `crates/router/src/routes/dummy_connector`.
// Upstream declares its DTOs in routes/dummy_connector/types.rs and its logic in
// core.rs / utils.rs / consts.rs / errors.rs. This domain has no `app_dto`
// partition, so the wire shapes live here; every helper is a direct local
// translation of the named upstream function (line references inline).
// Redis is the only external dependency; the in-process cache is a local stand-in
// that keeps the same keys, values and TTLs.
// ---------------------------------------------------------------------------
export namespace app::handlers::dummy_connector {

/** @brief `DummyConnectorStatus` (types.rs:58-67), lowercase wire form. */
enum class Status : std::uint8_t
{
    Succeeded,
    Processing,
    Failed,
};

/** @brief `DummyConnectors` (types.rs:25-42), wire names from serde renames. */
enum class Connector : std::uint8_t
{
    PhonyPay,
    FauxPay,
    PretendPay,
    StripeTest,
    AdyenTest,
    CheckoutTest,
    PaypalTest,
};

/** @brief `DummyConnectorWallet` (types.rs:207-215). */
enum class Wallet : std::uint8_t
{
    GooglePay,
    Paypal,
    WeChatPay,
    MbWay,
    AliPay,
    AliPayHK,
};

/** @brief `DummyConnectorPayLater` (types.rs:260-265). */
enum class PayLater : std::uint8_t
{
    Klarna,
    Affirm,
    AfterPayClearPay,
};

/** @brief `DummyConnectorPaymentMethodData` (types.rs:128-135), externally tagged. */
struct PaymentMethodData
{
    enum class Kind : std::uint8_t
    {
        Card,
        Upi,
        Wallet,
        PayLater,
    };
    Kind kind{Kind::Card};
    std::string card_number{}; // DummyConnectorCard.number (types.rs:185)
    std::string vpa_id{};      // DummyConnectorUpiCollect.vpa_id (types.rs:193)
    Wallet wallet{Wallet::GooglePay};
    PayLater pay_later{PayLater::Klarna};
};

/** @brief `DummyConnectorPaymentMethodType` (types.rs:137-147), the persisted form. */
struct PaymentMethodType
{
    enum class Kind : std::uint8_t
    {
        Card,
        Upi,
        Wallet,
        PayLater,
    };
    Kind kind{Kind::Card};
    Wallet wallet{Wallet::GooglePay};
    PayLater pay_later{PayLater::Klarna};
};

/** @brief `DummyConnectorNextAction::RedirectToUrl` (types.rs:317-321). */
struct NextAction
{
    bool redirect{false};
    std::string url{};
};

/** @brief `DummyConnectorPaymentRequest` (types.rs:114-121). */
struct PaymentRequest
{
    std::int64_t amount{0};
    std::string currency{};
    PaymentMethodData pm{};
    std::optional<std::string> return_url{};
    Connector connector{Connector::PhonyPay};
};

/** @brief `DummyConnectorPaymentData` (types.rs:285-299), the stored value. */
struct PaymentData
{
    std::string attempt_id{};
    std::string payment_id{};
    Status status{Status::Processing};
    std::int64_t amount{0};
    std::int64_t eligible_amount{0};
    std::string currency{};
    std::string created{}; // custom_serde::iso8601 (types.rs:293)
    PaymentMethodType pm_type{};
    Connector connector{Connector::PhonyPay};
    std::optional<NextAction> next_action{};
    std::optional<std::string> return_url{};
};

/** @brief `DummyConnectorRefundRequest` (types.rs:370-374); payment_id comes from path. */
struct RefundRequest
{
    std::int64_t amount{0};
    std::string payment_id{};
};

/** @brief `DummyConnectorRefundResponse` (types.rs:376-385), the stored value. */
struct RefundData
{
    Status status{Status::Succeeded};
    std::string id{};
    std::string currency{};
    std::string created{};
    std::int64_t payment_amount{0};
    std::int64_t refund_amount{0};
};

// -- consts.rs -----------------------------------------------------------------
inline constexpr std::string_view ATTEMPT_ID_PREFIX{"dummy_attempt"}; // consts.rs:1
inline constexpr std::string_view REFUND_ID_PREFIX{"dummy_ref"};      // consts.rs:2
inline constexpr std::string_view UPI_FAILURE_VPA{"failure@upi"};     // consts.rs:4
inline constexpr std::string_view UPI_SUCCESS_VPA{"success@upi"};     // consts.rs:5

// -- JSON tree navigation ------------------------------------------------------
using Object = interfaces::Value::Object;

// -- Plain inputs -------------------------------------------------------------
struct AuthorizePaymentInput
{
    std::string attempt_id{};
};

struct CompletePaymentInput
{
    std::string attempt_id{};
    bool confirm{false};
};

struct PaymentInput
{
    PaymentRequest request{};
};

struct PaymentDataInput
{
    std::string payment_id{};
};

struct RefundInput
{
    std::string payment_id{};
    std::int64_t amount{0};
};

struct RefundDataInput
{
    std::string refund_id{};
};

/**
 * @brief Parse/auth/path/query support for the dummy-connector handlers.
 *
 * Holds no business logic: it turns request data into the plain Input structs
 * consumed by the domain runtime. It also owns the value/JSON and DC-error
 * helpers shared with the runtime support class.
 */
class DummyConnectorInputSupport
{
public:
    explicit DummyConnectorInputSupport(::app::context::AppContext& context) noexcept :
        m_context{context}
    {
    }

    // -- small value helpers -------------------------------------------------
    [[nodiscard]] std::string jstr(std::string_view value) const
    {
        return serde::Ser::encode_json(std::string{value});
    }

    [[nodiscard]] std::string_view status_wire(Status status) const noexcept
    {
        switch (status) {
            case Status::Succeeded:
                return "succeeded";
            case Status::Failed:
                return "failed";
            case Status::Processing:
                return "processing";
        }
        return "processing";
    }

    [[nodiscard]] std::optional<Status> parse_status(std::string_view value) const noexcept
    {
        if (value == "succeeded") {
            return Status::Succeeded;
        }
        if (value == "failed") {
            return Status::Failed;
        }
        if (value == "processing") {
            return Status::Processing;
        }
        return std::nullopt;
    }

    [[nodiscard]] std::string_view connector_wire(Connector connector) const noexcept
    {
        switch (connector) {
            case Connector::PhonyPay:
                return "phonypay";
            case Connector::FauxPay:
                return "fauxpay";
            case Connector::PretendPay:
                return "pretendpay";
            case Connector::StripeTest:
                return "stripe_test";
            case Connector::AdyenTest:
                return "adyen_test";
            case Connector::CheckoutTest:
                return "checkout_test";
            case Connector::PaypalTest:
                return "paypal_test";
        }
        return "phonypay";
    }

    [[nodiscard]] std::optional<Connector> parse_connector(std::string_view value) const noexcept
    {
        if (value == "phonypay") {
            return Connector::PhonyPay;
        }
        if (value == "fauxpay") {
            return Connector::FauxPay;
        }
        if (value == "pretendpay") {
            return Connector::PretendPay;
        }
        if (value == "stripe_test") {
            return Connector::StripeTest;
        }
        if (value == "adyen_test") {
            return Connector::AdyenTest;
        }
        if (value == "checkout_test") {
            return Connector::CheckoutTest;
        }
        if (value == "paypal_test") {
            return Connector::PaypalTest;
        }
        return std::nullopt;
    }

    [[nodiscard]] std::optional<Wallet> parse_wallet(std::string_view value) const noexcept
    {
        if (value == "GooglePay") {
            return Wallet::GooglePay;
        }
        if (value == "Paypal") {
            return Wallet::Paypal;
        }
        if (value == "WeChatPay") {
            return Wallet::WeChatPay;
        }
        if (value == "MbWay") {
            return Wallet::MbWay;
        }
        if (value == "AliPay") {
            return Wallet::AliPay;
        }
        if (value == "AliPayHK") {
            return Wallet::AliPayHK;
        }
        return std::nullopt;
    }

    [[nodiscard]] std::string_view wallet_wire(Wallet wallet) const noexcept
    {
        switch (wallet) {
            case Wallet::GooglePay:
                return "GooglePay";
            case Wallet::Paypal:
                return "Paypal";
            case Wallet::WeChatPay:
                return "WeChatPay";
            case Wallet::MbWay:
                return "MbWay";
            case Wallet::AliPay:
                return "AliPay";
            case Wallet::AliPayHK:
                return "AliPayHK";
        }
        return "GooglePay";
    }

    [[nodiscard]] std::optional<PayLater> parse_pay_later(std::string_view value) const noexcept
    {
        if (value == "Klarna") {
            return PayLater::Klarna;
        }
        if (value == "Affirm") {
            return PayLater::Affirm;
        }
        if (value == "AfterPayClearPay") {
            return PayLater::AfterPayClearPay;
        }
        return std::nullopt;
    }

    [[nodiscard]] std::string_view pay_later_wire(PayLater value) const noexcept
    {
        switch (value) {
            case PayLater::Klarna:
                return "Klarna";
            case PayLater::Affirm:
                return "Affirm";
            case PayLater::AfterPayClearPay:
                return "AfterPayClearPay";
        }
        return "Klarna";
    }

    // rfl::Object::get() returns a Result<T> by value; return an owned optional so
    // callers never hold a pointer into a destroyed temporary.
    [[nodiscard]] std::optional<interfaces::Value>
    field(const Object& object, std::string_view key) const
    {
        auto found = object.get(std::string{key});
        if (!found) {
            return std::nullopt;
        }
        return *found;
    }

    [[nodiscard]] std::string read_string(const Object& object, std::string_view key) const
    {
        const auto value = field(object, key);
        if (!value) {
            return {};
        }
        const auto* text = std::get_if<std::string>(&value->get());
        return text != nullptr ? *text : std::string{};
    }

    [[nodiscard]] std::int64_t read_int(const Object& object, std::string_view key) const
    {
        const auto value = field(object, key);
        if (!value) {
            return 0;
        }
        if (const auto* number = std::get_if<std::int64_t>(&value->get())) {
            return *number;
        }
        if (const auto* number = std::get_if<double>(&value->get())) {
            return static_cast<std::int64_t>(*number);
        }
        return 0;
    }

    [[nodiscard]] std::optional<std::string>
    read_optional_string(const Object& object, std::string_view key) const
    {
        const auto value = field(object, key);
        if (!value) {
            return std::nullopt;
        }
        const auto* text = std::get_if<std::string>(&value->get());
        if (text == nullptr) {
            return std::nullopt;
        }
        return *text;
    }

    /** @brief Parse the externally-tagged `DummyConnectorPaymentMethodData` (types.rs:128-135). */
    [[nodiscard]] std::optional<PaymentMethodData>
    parse_pm_data(const interfaces::Value& value) const
    {
        const auto* object = std::get_if<Object>(&value.get());
        if (object == nullptr) {
            return std::nullopt;
        }
        PaymentMethodData out{};
        if (const auto card = field(*object, "card")) {
            const auto* body = std::get_if<Object>(&card->get());
            if (body == nullptr) {
                return std::nullopt;
            }
            out.kind = PaymentMethodData::Kind::Card;
            out.card_number = read_string(*body, "number");
            return out;
        }
        if (const auto upi = field(*object, "upi")) {
            const auto* body = std::get_if<Object>(&upi->get());
            if (body == nullptr) {
                return std::nullopt;
            }
            const auto collect = field(*body, "upi_collect");
            if (!collect) {
                return std::nullopt;
            }
            const auto* collect_body = std::get_if<Object>(&collect->get());
            if (collect_body == nullptr) {
                return std::nullopt;
            }
            out.kind = PaymentMethodData::Kind::Upi;
            out.vpa_id = read_string(*collect_body, "vpa_id");
            return out;
        }
        if (const auto wallet = field(*object, "wallet")) {
            const auto* text = std::get_if<std::string>(&wallet->get());
            if (text == nullptr) {
                return std::nullopt;
            }
            const auto parsed = parse_wallet(*text);
            if (!parsed) {
                return std::nullopt;
            }
            out.kind = PaymentMethodData::Kind::Wallet;
            out.wallet = *parsed;
            return out;
        }
        if (const auto pay_later = field(*object, "pay_later")) {
            const auto* text = std::get_if<std::string>(&pay_later->get());
            if (text == nullptr) {
                return std::nullopt;
            }
            const auto parsed = parse_pay_later(*text);
            if (!parsed) {
                return std::nullopt;
            }
            out.kind = PaymentMethodData::Kind::PayLater;
            out.pay_later = *parsed;
            return out;
        }
        return std::nullopt;
    }

    // -- error constructors (errors.rs:13-40 and ErrorSwitch, errors.rs:53-87) --
    // Upstream code (DC_00, IR_00, ...) goes into AppError::kind_name so the
    // serialized "error" field preserves it; message is the real human-readable
    // text. The kind maps to the same HTTP status the original DcError.status
    // carried.
    [[nodiscard]] core::error::AppError dc_error(
        std::string_view code,
        std::string_view /*type*/,
        std::string message,
        core::error::AppErrorKind kind
    ) const
    {
        core::error::AppError error{};
        error.set_kind(kind);
        error.set_kind_name(std::string{code});
        error.set_message(std::move(message));
        return error;
    }

    [[nodiscard]] core::error::AppError dc_internal() const
    {
        return dc_error(
            "DC_00",
            "api",
            "Something went wrong",
            core::error::AppErrorKind::INTERNAL_ERROR
        );
    }

    [[nodiscard]] core::error::AppError dc_payment_not_found() const
    {
        return dc_error(
            "DC_01",
            "invalid_request",
            "Payment does not exist in our records",
            core::error::AppErrorKind::NOT_FOUND
        );
    }

    [[nodiscard]] core::error::AppError dc_missing_field(std::string_view field_name) const
    {
        return dc_error(
            "DC_02",
            "invalid_request",
            "Missing required param: " + std::string{field_name},
            core::error::AppErrorKind::INVALID_REQUEST
        );
    }

    [[nodiscard]] core::error::AppError dc_refund_amount_exceeds() const
    {
        return dc_error(
            "DC_03",
            "api",
            "The refund amount exceeds the amount captured",
            core::error::AppErrorKind::INTERNAL_ERROR
        );
    }

    [[nodiscard]] core::error::AppError dc_card_not_supported() const
    {
        return dc_error(
            "DC_04",
            "invalid_request",
            "Card not supported. Please use test cards",
            core::error::AppErrorKind::INVALID_REQUEST
        );
    }

    [[nodiscard]] core::error::AppError dc_refund_not_found() const
    {
        return dc_error(
            "DC_05",
            "invalid_request",
            "Refund does not exist in our records",
            core::error::AppErrorKind::NOT_FOUND
        );
    }

    [[nodiscard]] core::error::AppError dc_payment_not_successful() const
    {
        return dc_error(
            "DC_06",
            "invalid_request",
            "Payment is not successful",
            core::error::AppErrorKind::INVALID_REQUEST
        );
    }

    [[nodiscard]] core::error::AppError dc_store_error() const
    {
        return dc_error(
            "DC_07",
            "api",
            "Error occurred while storing the payment",
            core::error::AppErrorKind::INTERNAL_ERROR
        );
    }

    [[nodiscard]] core::error::AppError dc_payment_declined(std::string_view message) const
    {
        return dc_error(
            "DC_08",
            "invalid_request",
            "Payment declined: " + std::string{message},
            core::error::AppErrorKind::INVALID_REQUEST
        );
    }

    [[nodiscard]] core::error::AppError dc_json_error() const
    {
        return dc_error(
            "IR_00",
            "invalid_request",
            "Json deserialize error",
            core::error::AppErrorKind::INVALID_REQUEST
        );
    }

    // -- auth / guards ------------------------------------------------------
    [[nodiscard]] std::optional<core::error::AppError>
    authorize(interfaces::io::IRequest& req, std::string_view operation) const
    {
        // All six ops use auth::NoAuth (dummy_connector.rs:32,58,79,101,124,146);
        // the registry declares them NO_AUTH (registry.cppm:112-117). The
        // #[instrument(flow=...)]/LockAction::NotApplicable contract is mirrored by
        // record_flow/counter in each handler and the absence of a lock acquire.
        const auto rule = m_context.auth_rules().rule_for(operation);
        return m_context.authorizer().authorize_route(::app::auth::AuthContextBuilder{}(req), rule);
    }

    [[nodiscard]] std::optional<int> require_local_host(interfaces::io::IRequest& req) const
    {
        // app.rs:857-862 - Host("localhost") guard unless external_access_dc.
        // A missing/foreign Host does not match the guard, so the route is not found.
        const auto external = config_string("external_access_dc", "false");
        if (external == "true" || external == "1") {
            return std::nullopt;
        }
        // get_host() reads the HOST token (`:authority` on HTTP/2, Host on HTTP/1).
        const auto host = req.get_host();
        if (host == "localhost" || host.starts_with("localhost:")) {
            return std::nullopt;
        }
        return 1;
    }

    // -- config -------------------------------------------------------------
    [[nodiscard]] std::string config_string(std::string_view key, std::string_view fallback) const
    {
        return m_context.config_store()
            .get_value("dummy_connector", key)
            .value_or(std::string{fallback});
    }

    /** @brief utils.rs:18-29 - tokio_mock_sleep(delay, tolerance). */
    void apply_mock_latency(std::string_view delay_key, std::string_view tolerance_key) const
    {
        const auto read_count = [this](std::string_view key)
        {
            const auto value = m_context.config_store().get_value("dummy_connector", key);
            if (!value) {
                return std::int64_t{0};
            }
            std::int64_t parsed{0};
            const auto result =
                std::from_chars(value->data(), value->data() + value->size(), parsed);
            return result.ec == std::errc{} ? parsed : std::int64_t{0};
        };
        const std::int64_t delay = read_count(delay_key);
        const std::int64_t tolerance = read_count(tolerance_key);
        if (delay <= 0) {
            return;
        }
        const std::int64_t low = delay > tolerance ? delay - tolerance : 0;
        const std::int64_t high = delay + tolerance;
        // Deterministic midpoint of the half-open upstream range [low, high).
        const std::int64_t effective = std::clamp<std::int64_t>((low + high) / 2, 0, 1'000);
        if (effective > 0) {
            std::this_thread::sleep_for(std::chrono::milliseconds(effective));
        }
    }

    // -- path helpers -------------------------------------------------------
    [[nodiscard]] std::string last_segment(interfaces::io::IRequest& req) const
    {
        const auto segments = ::app::http::RequestPath{}(req);
        if (segments.count() == 0) {
            return {};
        }
        return std::string{segments.at(segments.count() - 1).value_or("")};
    }

    // -- request parsing ----------------------------------------------------
    [[nodiscard]] std::expected<PaymentRequest, core::error::AppError>
    parse_payment_request(std::string_view body) const
    {
        auto tree = serde::Ser::decode_generic("application/json", body);
        if (!tree) {
            return std::unexpected{dc_json_error()};
        }
        const auto* root = std::get_if<Object>(&tree->get());
        if (root == nullptr) {
            return std::unexpected{dc_json_error()};
        }
        PaymentRequest request{};
        if (!field(*root, "amount").has_value()) {
            return std::unexpected{dc_missing_field("amount")};
        }
        request.amount = read_int(*root, "amount");
        const auto currency = read_string(*root, "currency");
        if (currency.empty()) {
            return std::unexpected{dc_missing_field("currency")};
        }
        request.currency = currency;
        const auto pm_value = field(*root, "payment_method_data");
        if (!pm_value) {
            return std::unexpected{dc_missing_field("payment_method_data")};
        }
        auto pm = parse_pm_data(*pm_value);
        if (!pm) {
            return std::unexpected{dc_json_error()};
        }
        request.pm = std::move(*pm);
        request.return_url = read_optional_string(*root, "return_url");
        if (!field(*root, "connector").has_value()) {
            return std::unexpected{dc_missing_field("connector")};
        }
        const auto connector = parse_connector(read_string(*root, "connector"));
        if (!connector) {
            return std::unexpected{dc_json_error()};
        }
        request.connector = *connector;
        return request;
    }

    [[nodiscard]] std::expected<RefundRequest, core::error::AppError>
    parse_refund_request(std::string_view body) const
    {
        auto tree = serde::Ser::decode_generic("application/json", body);
        if (!tree) {
            return std::unexpected{dc_json_error()};
        }
        const auto* root = std::get_if<Object>(&tree->get());
        if (root == nullptr) {
            return std::unexpected{dc_json_error()};
        }
        if (!field(*root, "amount").has_value()) {
            return std::unexpected{dc_missing_field("amount")};
        }
        return RefundRequest{read_int(*root, "amount"), {}};
    }

private:
    ::app::context::AppContext& m_context;
};

} // namespace app::handlers::dummy_connector
