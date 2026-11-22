module;
#include <time.h>
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
// Local, faithful port of upstream `crates/router/src/routes/dummy_connector`.
// Upstream declares its DTOs in routes/dummy_connector/types.rs and its logic in
// core.rs / utils.rs / consts.rs / errors.rs. This domain has no `app_dto`
// partition, so the wire shapes live here; every function below is a direct
// local translation of the named upstream function (line references inline).
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
    std::string card_number{};                 // DummyConnectorCard.number (types.rs:185)
    std::string vpa_id{};                      // DummyConnectorUpiCollect.vpa_id (types.rs:193)
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
    std::string created{};                     // custom_serde::iso8601 (types.rs:293)
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

/** @brief One upstream `DummyConnectorErrors` variant (errors.rs:9-40). */
struct DcError
{
    std::string code{};
    std::string type{};
    std::string message{};
    interfaces::io::types::Status status{interfaces::io::types::Status::BAD_REQUEST};
};

// -- consts.rs -----------------------------------------------------------------
inline constexpr std::string_view ATTEMPT_ID_PREFIX{"dummy_attempt"};  // consts.rs:1
inline constexpr std::string_view REFUND_ID_PREFIX{"dummy_ref"};       // consts.rs:2
inline constexpr std::string_view UPI_FAILURE_VPA{"failure@upi"};      // consts.rs:4
inline constexpr std::string_view UPI_SUCCESS_VPA{"success@upi"};      // consts.rs:5

// consts.rs:3 - include_str!("threeds_page.css"). The asset is compiled into
// the upstream binary; embedding it verbatim here keeps the two HTML pages
// byte-identical without a runtime file dependency.
inline constexpr std::string_view THREE_DS_CSS{R"DC_CSS(@import url('https://fonts.googleapis.com/css2?family=Inter:wght@200;300;400;500;600;700&display=swap');
body {
    font-family: Inter;
    background-image: url('https://app.hyperswitch.io/images/hyperswitchImages/PostLoginBackground.svg');
    display: flex; 
    background-size: cover;
    height: 100%; 
    padding-top: 3rem;
    margin: 0; 
    align-items: center; 
    flex-direction: column; 
}
.authorize {
    color: white;
    background-color: #006DF9;
    border: 1px solid #006DF9;
    box-sizing: border-box;
    margin: 1.5rem 0.5rem 1rem 0;
    border-radius: 0.25rem;
    padding: 0.75rem 1rem;
    font-weight: 500;
    font-size: 1.1rem;
}
.authorize:hover {
    background-color: #0099FF;
    border: 1px solid #0099FF;
    cursor: pointer;
}
.reject {
    background-color: #F7F7F7;
    color: black;
    border: 1px solid #E8E8E8;
    box-sizing: border-box;
    border-radius: 0.25rem;
    margin-left: 0.5rem;
    font-size: 1.1rem;
    padding: 0.75rem 1rem;
    font-weight: 500;
}
.reject:hover {
    cursor: pointer;
    background-color: #E8E8E8;
    border: 1px solid #E8E8E8;
}
.container {
    background-color: white;
    width: 33rem;
    margin: 1rem 0;
    border: 1px solid #E8E8E8;
    color: black;
    border-radius: 0.25rem;
    padding: 1rem 1.4rem 1rem 1.4rem;
    font-family: Inter;
}
.container p {
    font-weight: 400;
    margin-top: 0.5rem 1rem 0.5rem 0.5rem;
    color: #151A1F;
    opacity: 0.5;
}
b {
    font-weight: 600;
}
.disclaimer {
    font-size: 1.25rem;
    font-weight: 500 !important;
    margin-top: 0.5rem !important;
    margin-bottom: 0.5rem !important;
    opacity: 1 !important;
}
.heading {
    display: flex;
    justify-content: center;
    flex-direction: column;
    align-items: center;
    margin-bottom: 1rem;
}
.logo {
    width: 8rem;
}
.payment_details {
    height: 2rem;
    display: flex;
    margin: 1rem 0 2rem 0;
}
.border_horizontal {
    border-top: 1px dashed #151a1f80;
    height: 1px;
    margin: 0 1rem;
    margin-top: 1rem;
    width: 20%;
}
.contact {
    display: flex;
    gap: 10%;
    margin: 2rem 0 1rem 0;
    height: 3rem;
}
.contact img {
    aspect-ratio: 1/1;
    height: 2rem;
}
.contact_item {
    display: flex;
    height: 100%;
    flex-direction: column;
    justify-content: center;
}
.contact_item p {
    margin: 0;
}
.border_vertical {
    border-left: 1px solid #151a1f80;
    width: 1px;
    height: 100%;
}
.email {
    justify-content: space-between;
}
.hover_cursor:hover {
    cursor: pointer
}
a {
    text-decoration: none;
    opacity: 0.8;
})DC_CSS"};

// -- small value helpers -------------------------------------------------------
inline std::string jstr(std::string_view value)
{
    return serde::Ser::encode_json(std::string{value});
}

inline std::string html_escape(std::string_view value)
{
    std::string out{};
    for (const char c : value) {
        switch (c) {
        case '&': out += "&amp;"; break;
        case '<': out += "&lt;"; break;
        case '>': out += "&gt;"; break;
        case '"': out += "&quot;"; break;
        default: out += c;
        }
    }
    return out;
}

inline std::string_view status_wire(Status status) noexcept
{
    switch (status) {
    case Status::Succeeded: return "succeeded";
    case Status::Failed: return "failed";
    case Status::Processing: return "processing";
    }
    return "processing";
}

inline std::optional<Status> parse_status(std::string_view value) noexcept
{
    if (value == "succeeded") return Status::Succeeded;
    if (value == "failed") return Status::Failed;
    if (value == "processing") return Status::Processing;
    return std::nullopt;
}

inline std::string_view connector_wire(Connector connector) noexcept
{
    switch (connector) {
    case Connector::PhonyPay: return "phonypay";
    case Connector::FauxPay: return "fauxpay";
    case Connector::PretendPay: return "pretendpay";
    case Connector::StripeTest: return "stripe_test";
    case Connector::AdyenTest: return "adyen_test";
    case Connector::CheckoutTest: return "checkout_test";
    case Connector::PaypalTest: return "paypal_test";
    }
    return "phonypay";
}

inline std::optional<Connector> parse_connector(std::string_view value) noexcept
{
    if (value == "phonypay") return Connector::PhonyPay;
    if (value == "fauxpay") return Connector::FauxPay;
    if (value == "pretendpay") return Connector::PretendPay;
    if (value == "stripe_test") return Connector::StripeTest;
    if (value == "adyen_test") return Connector::AdyenTest;
    if (value == "checkout_test") return Connector::CheckoutTest;
    if (value == "paypal_test") return Connector::PaypalTest;
    return std::nullopt;
}

/** @brief `DummyConnectors::get_connector_image_link` (types.rs:44-56). */
inline std::string connector_image(Connector connector, std::string_view assets)
{
    std::string_view name{"PHONYPAY.svg"};
    switch (connector) {
    case Connector::PhonyPay: name = "PHONYPAY.svg"; break;
    case Connector::FauxPay: name = "FAUXPAY.svg"; break;
    case Connector::PretendPay: name = "PRETENDPAY.svg"; break;
    case Connector::StripeTest: name = "STRIPE_TEST.svg"; break;
    case Connector::PaypalTest: name = "PAYPAL_TEST.svg"; break;
    default: name = "PHONYPAY.svg"; break;
    }
    return std::string{assets} + std::string{name};
}

inline std::optional<Wallet> parse_wallet(std::string_view value) noexcept
{
    if (value == "GooglePay") return Wallet::GooglePay;
    if (value == "Paypal") return Wallet::Paypal;
    if (value == "WeChatPay") return Wallet::WeChatPay;
    if (value == "MbWay") return Wallet::MbWay;
    if (value == "AliPay") return Wallet::AliPay;
    if (value == "AliPayHK") return Wallet::AliPayHK;
    return std::nullopt;
}

inline std::string_view wallet_wire(Wallet wallet) noexcept
{
    switch (wallet) {
    case Wallet::GooglePay: return "GooglePay";
    case Wallet::Paypal: return "Paypal";
    case Wallet::WeChatPay: return "WeChatPay";
    case Wallet::MbWay: return "MbWay";
    case Wallet::AliPay: return "AliPay";
    case Wallet::AliPayHK: return "AliPayHK";
    }
    return "GooglePay";
}

/** @brief `GetPaymentMethodDetails::get_name` (types.rs:236-246). */
inline std::string_view wallet_name(Wallet wallet) noexcept
{
    switch (wallet) {
    case Wallet::GooglePay: return "Google Pay";
    case Wallet::Paypal: return "PayPal";
    case Wallet::WeChatPay: return "WeChat Pay";
    case Wallet::MbWay: return "Mb Way";
    case Wallet::AliPay: return "Alipay";
    case Wallet::AliPayHK: return "Alipay HK";
    }
    return "Google Pay";
}

inline std::string_view wallet_image(Wallet wallet) noexcept
{
    switch (wallet) {
    case Wallet::GooglePay: return "GOOGLE_PAY.svg";
    case Wallet::Paypal: return "PAYPAL.svg";
    case Wallet::WeChatPay: return "WECHAT_PAY.svg";
    case Wallet::MbWay: return "MBWAY.svg";
    case Wallet::AliPay: return "ALIPAY.svg";
    case Wallet::AliPayHK: return "ALIPAY.svg";
    }
    return "GOOGLE_PAY.svg";
}

inline std::optional<PayLater> parse_pay_later(std::string_view value) noexcept
{
    if (value == "Klarna") return PayLater::Klarna;
    if (value == "Affirm") return PayLater::Affirm;
    if (value == "AfterPayClearPay") return PayLater::AfterPayClearPay;
    return std::nullopt;
}

inline std::string_view pay_later_wire(PayLater value) noexcept
{
    switch (value) {
    case PayLater::Klarna: return "Klarna";
    case PayLater::Affirm: return "Affirm";
    case PayLater::AfterPayClearPay: return "AfterPayClearPay";
    }
    return "Klarna";
}

inline std::string_view pay_later_name(PayLater value) noexcept
{
    switch (value) {
    case PayLater::Klarna: return "Klarna";
    case PayLater::Affirm: return "Affirm";
    case PayLater::AfterPayClearPay: return "Afterpay Clearpay";
    }
    return "Klarna";
}

inline std::string_view pay_later_image(PayLater value) noexcept
{
    switch (value) {
    case PayLater::Klarna: return "KLARNA.svg";
    case PayLater::Affirm: return "AFFIRM.svg";
    case PayLater::AfterPayClearPay: return "AFTERPAY.svg";
    }
    return "KLARNA.svg";
}

/** @brief `From<DummyConnectorPaymentMethodData> for DummyConnectorPaymentMethodType` (types.rs:149-160). */
inline PaymentMethodType to_payment_method_type(const PaymentMethodData& data) noexcept
{
    PaymentMethodType out{};
    out.kind = static_cast<PaymentMethodType::Kind>(data.kind);
    out.wallet = data.wallet;
    out.pay_later = data.pay_later;
    return out;
}

/** @brief Externally-tagged, `rename_all = "lowercase"` wire form (types.rs:137-147). */
inline std::string pm_type_wire(const PaymentMethodType& type)
{
    switch (type.kind) {
    case PaymentMethodType::Kind::Card: return "\"card\"";
    case PaymentMethodType::Kind::Upi: return "{\"upi\":\"UpiCollect\"}";
    case PaymentMethodType::Kind::Wallet:
        return "{\"wallet\":" + jstr(wallet_wire(type.wallet)) + "}";
    case PaymentMethodType::Kind::PayLater:
        return "{\"pay_later\":" + jstr(pay_later_wire(type.pay_later)) + "}";
    }
    return "\"card\"";
}

/** @brief `GetPaymentMethodDetails::get_name` (types.rs:162-170). */
inline std::string pm_type_name(const PaymentMethodType& type)
{
    switch (type.kind) {
    case PaymentMethodType::Kind::Card: return "3D Secure";
    case PaymentMethodType::Kind::Upi: return "UPI Collect";
    case PaymentMethodType::Kind::Wallet: return std::string{wallet_name(type.wallet)};
    case PaymentMethodType::Kind::PayLater: return std::string{pay_later_name(type.pay_later)};
    }
    return "3D Secure";
}

/** @brief `GetPaymentMethodDetails::get_image_link` (types.rs:171-180). */
inline std::string pm_type_image(const PaymentMethodType& type, std::string_view assets)
{
    std::string_view name{"CARD.svg"};
    switch (type.kind) {
    case PaymentMethodType::Kind::Card: name = "CARD.svg"; break;
    case PaymentMethodType::Kind::Upi: name = "UPI_COLLECT.svg"; break;
    case PaymentMethodType::Kind::Wallet: name = wallet_image(type.wallet); break;
    case PaymentMethodType::Kind::PayLater: name = pay_later_image(type.pay_later); break;
    }
    return std::string{assets} + std::string{name};
}

/** @brief `Option<DummyConnectorNextAction>` (types.rs:317-321, snake_case). */
inline std::string next_action_wire(const std::optional<NextAction>& action)
{
    if (!action || !action->redirect) {
        return "null";
    }
    return "{\"redirect_to_url\":" + jstr(action->url) + "}";
}

// -- JSON tree navigation ------------------------------------------------------
using Object = interfaces::Value::Object;

// rfl::Object::get() returns a Result<T> by value; return an owned optional so
// callers never hold a pointer into a destroyed temporary.
inline std::optional<interfaces::Value> field(const Object& object, std::string_view key)
{
    auto found = object.get(std::string{key});
    if (!found) {
        return std::nullopt;
    }
    return *found;
}

inline std::string read_string(const Object& object, std::string_view key)
{
    const auto value = field(object, key);
    if (!value) {
        return {};
    }
    const auto* text = std::get_if<std::string>(&value->get());
    return text != nullptr ? *text : std::string{};
}

inline std::int64_t read_int(const Object& object, std::string_view key)
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

inline std::optional<std::string> read_optional_string(const Object& object, std::string_view key)
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
inline std::optional<PaymentMethodData> parse_pm_data(const interfaces::Value& value)
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

/** @brief Decode the persisted `payment_method_type` (externally tagged). */
inline PaymentMethodType parse_pm_type(const interfaces::Value& value)
{
    PaymentMethodType out{};
    if (const auto* text = std::get_if<std::string>(&value.get())) {
        if (*text == "card") {
            out.kind = PaymentMethodType::Kind::Card;
        }
        return out;
    }
    const auto* object = std::get_if<Object>(&value.get());
    if (object == nullptr) {
        return out;
    }
    if (field(*object, "upi").has_value()) {
        out.kind = PaymentMethodType::Kind::Upi;
    } else if (const auto wallet = field(*object, "wallet")) {
        if (const auto* text = std::get_if<std::string>(&wallet->get())) {
            out.kind = PaymentMethodType::Kind::Wallet;
            out.wallet = parse_wallet(*text).value_or(Wallet::GooglePay);
        }
    } else if (const auto pay_later = field(*object, "pay_later")) {
        if (const auto* text = std::get_if<std::string>(&pay_later->get())) {
            out.kind = PaymentMethodType::Kind::PayLater;
            out.pay_later = parse_pay_later(*text).value_or(PayLater::Klarna);
        }
    }
    return out;
}

// -- persistence codecs (the values stored under the upstream Redis keys) -----
inline std::string encode_payment_data(const PaymentData& data)
{
    return "{\"attempt_id\":" + jstr(data.attempt_id)
        + ",\"payment_id\":" + jstr(data.payment_id)
        + ",\"status\":" + jstr(status_wire(data.status))
        + ",\"amount\":" + std::to_string(data.amount)
        + ",\"eligible_amount\":" + std::to_string(data.eligible_amount)
        + ",\"currency\":" + jstr(data.currency)
        + ",\"created\":" + jstr(data.created)
        + ",\"payment_method_type\":" + pm_type_wire(data.pm_type)
        + ",\"connector\":" + jstr(connector_wire(data.connector))
        + ",\"next_action\":" + next_action_wire(data.next_action)
        + ",\"return_url\":" + (data.return_url ? jstr(*data.return_url) : std::string{"null"})
        + "}";
}

inline std::optional<PaymentData> decode_payment_data(std::string_view json)
{
    auto tree = serde::Ser::decode_generic("application/json", json);
    if (!tree) {
        return std::nullopt;
    }
    const auto* object = std::get_if<Object>(&tree->get());
    if (object == nullptr) {
        return std::nullopt;
    }
    PaymentData out{};
    out.attempt_id = read_string(*object, "attempt_id");
    out.payment_id = read_string(*object, "payment_id");
    out.status = parse_status(read_string(*object, "status")).value_or(Status::Processing);
    out.amount = read_int(*object, "amount");
    out.eligible_amount = read_int(*object, "eligible_amount");
    out.currency = read_string(*object, "currency");
    out.created = read_string(*object, "created");
    out.connector = parse_connector(read_string(*object, "connector")).value_or(Connector::PhonyPay);
    out.return_url = read_optional_string(*object, "return_url");
    if (const auto pm = field(*object, "payment_method_type")) {
        out.pm_type = parse_pm_type(*pm);
    }
    if (const auto action = field(*object, "next_action")) {
        const auto* action_object = std::get_if<Object>(&action->get());
        if (action_object != nullptr) {
            if (const auto url = field(*action_object, "redirect_to_url")) {
                if (const auto* text = std::get_if<std::string>(&url->get())) {
                    out.next_action = NextAction{true, *text};
                }
            }
        }
    }
    return out;
}

inline std::string encode_refund_data(const RefundData& data)
{
    return "{\"status\":" + jstr(status_wire(data.status))
        + ",\"id\":" + jstr(data.id)
        + ",\"currency\":" + jstr(data.currency)
        + ",\"created\":" + jstr(data.created)
        + ",\"payment_amount\":" + std::to_string(data.payment_amount)
        + ",\"refund_amount\":" + std::to_string(data.refund_amount)
        + "}";
}

inline std::optional<RefundData> decode_refund_data(std::string_view json)
{
    auto tree = serde::Ser::decode_generic("application/json", json);
    if (!tree) {
        return std::nullopt;
    }
    const auto* object = std::get_if<Object>(&tree->get());
    if (object == nullptr) {
        return std::nullopt;
    }
    RefundData out{};
    out.status = parse_status(read_string(*object, "status")).value_or(Status::Succeeded);
    out.id = read_string(*object, "id");
    out.currency = read_string(*object, "currency");
    out.created = read_string(*object, "created");
    out.payment_amount = read_int(*object, "payment_amount");
    out.refund_amount = read_int(*object, "refund_amount");
    return out;
}

/** @brief `DummyConnectorPaymentResponse` (types.rs:323-333). */
inline std::string payment_response(const PaymentData& data)
{
    return "{\"status\":" + jstr(status_wire(data.status))
        + ",\"id\":" + jstr(data.payment_id)
        + ",\"amount\":" + std::to_string(data.amount)
        + ",\"currency\":" + jstr(data.currency)
        + ",\"created\":" + jstr(data.created)
        + ",\"payment_method_type\":" + pm_type_wire(data.pm_type)
        + ",\"next_action\":" + next_action_wire(data.next_action)
        + "}";
}

/** @brief `DummyConnectorRefundResponse` (types.rs:376-385). */
inline std::string refund_response(const RefundData& data)
{
    return "{\"status\":" + jstr(status_wire(data.status))
        + ",\"id\":" + jstr(data.id)
        + ",\"currency\":" + jstr(data.currency)
        + ",\"created\":" + jstr(data.created)
        + ",\"payment_amount\":" + std::to_string(data.payment_amount)
        + ",\"refund_amount\":" + std::to_string(data.refund_amount)
        + "}";
}

// -- error constructors (errors.rs:13-40 and ErrorSwitch, errors.rs:53-87) ----
inline DcError dc_error(std::string_view code, std::string_view type, std::string message,
                        interfaces::io::types::Status status)
{
    return DcError{std::string{code}, std::string{type}, std::move(message), status};
}

inline DcError dc_internal()
{
    return dc_error("DC_00", "api", "Something went wrong",
                    interfaces::io::types::Status::INTERNAL_SERVER_ERROR);
}
inline DcError dc_payment_not_found()
{
    return dc_error("DC_01", "invalid_request", "Payment does not exist in our records",
                    interfaces::io::types::Status::NOT_FOUND);
}
inline DcError dc_missing_field(std::string_view field)
{
    return dc_error("DC_02", "invalid_request", "Missing required param: " + std::string{field},
                    interfaces::io::types::Status::BAD_REQUEST);
}
inline DcError dc_refund_amount_exceeds()
{
    return dc_error("DC_03", "api", "The refund amount exceeds the amount captured",
                    interfaces::io::types::Status::INTERNAL_SERVER_ERROR);
}
inline DcError dc_card_not_supported()
{
    return dc_error("DC_04", "invalid_request", "Card not supported. Please use test cards",
                    interfaces::io::types::Status::BAD_REQUEST);
}
inline DcError dc_refund_not_found()
{
    return dc_error("DC_05", "invalid_request", "Refund does not exist in our records",
                    interfaces::io::types::Status::NOT_FOUND);
}
inline DcError dc_payment_not_successful()
{
    return dc_error("DC_06", "invalid_request", "Payment is not successful",
                    interfaces::io::types::Status::BAD_REQUEST);
}
inline DcError dc_store_error()
{
    return dc_error("DC_07", "api", "Error occurred while storing the payment",
                    interfaces::io::types::Status::INTERNAL_SERVER_ERROR);
}
inline DcError dc_payment_declined(std::string_view message)
{
    return dc_error("DC_08", "invalid_request", "Payment declined: " + std::string{message},
                    interfaces::io::types::Status::BAD_REQUEST);
}

/** @brief `ErrorResponse` wire shape: {"error":{"type","message","code"}} (errors/types.rs:37-58). */
inline std::string dc_error_body(const DcError& error)
{
    return "{\"error\":{\"type\":" + jstr(error.type)
        + ",\"message\":" + jstr(error.message)
        + ",\"code\":" + jstr(error.code) + "}}";
}

/** @brief `common_utils::date_time::now()` + custom_serde::iso8601: YYYY-MM-DDTHH:MM:SS.mmmZ. */
inline std::string now_iso8601()
{
    const auto now = std::chrono::system_clock::now();
    const auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(
                            now.time_since_epoch()).count();
    const std::time_t seconds = static_cast<std::time_t>(millis / 1000);
    std::tm broken{};
    gmtime_r(&seconds, &broken);
    char buffer[64]{};
    std::snprintf(buffer, sizeof(buffer), "%04d-%02d-%02dT%02d:%02d:%02d.%03lldZ",
                  broken.tm_year + 1900, broken.tm_mon + 1, broken.tm_mday,
                  broken.tm_hour, broken.tm_min, broken.tm_sec,
                  static_cast<long long>(millis % 1000));
    return std::string{buffer};
}

} // namespace app::handlers::dummy_connector

export namespace app::handlers::dummy_connector {

class DummyConnectorRouteHandlers
{
public:
    // -----------------------------------------------------------------------
    // routes/dummy_connector.rs:18-36 -> core.rs:59-85 (payment_authorize).
    // -----------------------------------------------------------------------
    void handle_dummy_connector_authorize_payment(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto auth_error = authorize(req, "dummy_connector_authorize_payment")) {
            send_app_error(res, send, *auth_error);
            return;
        }
        // core.rs:63 - get_payment_data_by_attempt_id. No mock sleep in this flow.
        const auto attempt_id = last_segment(req);
        if (attempt_id.empty()) {
            send_html(res, send, expired_page(), interfaces::io::types::Status::OK);
            return;
        }
        const auto attempt = m_context->cache().get(attempt_id);
        const std::string payment_id = attempt.hit ? attempt.value_or<std::string>("") : std::string{};
        const auto payment = payment_id.empty() ? std::optional<PaymentData>{}
                                                : load_payment(payment_id);
        if (!payment) {
            // core.rs:78-84 - FileData(get_expired_page, TEXT_HTML)
            m_context->metrics().record_flow("DummyPaymentAuthorize");
            send_html(res, send, expired_page(), interfaces::io::types::Status::OK);
            return;
        }

        // core.rs:67-76 - return_url uses state.base_url, page via get_authorize_page.
        const std::string return_url = base_url() + "/dummy-connector/complete/" + attempt_id;
        m_context->metrics().record_flow("DummyPaymentAuthorize");
        m_context->metrics().counter("DUMMY_CONNECTOR_AUTHORIZE").increment();
        send_html(res, send, authorize_page(*payment, return_url), interfaces::io::types::Status::OK);
    }

    // -----------------------------------------------------------------------
    // routes/dummy_connector.rs:40-62 -> core.rs:88-153 (payment_complete).
    // -----------------------------------------------------------------------
    void handle_dummy_connector_complete_payment(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto auth_error = authorize(req, "dummy_connector_complete_payment")) {
            send_app_error(res, send, *auth_error);
            return;
        }
        apply_mock_latency("payment_duration", "payment_tolerance");

        const auto attempt_id = last_segment(req);
        const auto confirm_value = app::http::RequestQuery{}(req).get("confirm");
        if (!confirm_value || (*confirm_value != "true" && *confirm_value != "false")) {
            // web::Query<DummyConnectorPaymentCompleteBody> requires a bool.
            send_dc_error(res, send, dc_missing_field("confirm"));
            return;
        }
        const bool confirm = *confirm_value == "true";

        // core.rs:98 - get_payment_data_by_attempt_id (result kept for the branch).
        const auto attempt = m_context->cache().get(attempt_id);
        const std::string payment_id = attempt.hit ? attempt.value_or<std::string>("") : std::string{};
        auto payment = payment_id.empty() ? std::optional<PaymentData>{} : load_payment(payment_id);

        // core.rs:112 - delete the attempt key unconditionally.
        m_context->cache().erase(attempt_id);

        const std::string default_url = config_string("default_return_url", "https://app.hyperswitch.io/");
        std::string return_url = default_url;
        if (payment) {
            // core.rs:114-126 - status + next_action=None, store with payment_ttl.
            payment->status = confirm ? Status::Succeeded : Status::Failed;
            payment->next_action.reset();
            save_payment(*payment);
            return_url = payment->return_url.value_or(default_url);
        }

        // core.rs:127-152 - JsonForRedirection(RedirectionResponse): 302 + Location.
        m_context->metrics().record_flow("DummyPaymentComplete");
        m_context->metrics().counter("DUMMY_CONNECTOR_COMPLETE").increment();
        const std::string body = "{\"return_url\":\"\",\"params\":[],"
                                 "\"return_url_with_query_params\":" + jstr(return_url)
                               + ",\"http_method\":\"GET\",\"headers\":[]}";
        send_redirect(res, send, body, return_url);
    }

    // -----------------------------------------------------------------------
    // routes/dummy_connector.rs:66-83 -> core.rs:13-42 (payment).
    // -----------------------------------------------------------------------
    void handle_dummy_connector_payment(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto auth_error = authorize(req, "dummy_connector_payment")) {
            send_app_error(res, send, *auth_error);
            return;
        }
        if (const auto host_error = require_local_host(req)) {
            send_no_route(res, send);
            return;
        }
        apply_mock_latency("payment_duration", "payment_tolerance");

        const auto body = req.body_as_string();
        const auto request = parse_payment_request(body);
        if (!request) {
            send_dc_error(res, send, request.error());
            return;
        }
        const PaymentRequest& input = *request;

        // types.rs:77-88 - PaymentId::default() (pay_*) and
        // generate_id_with_default_len("dummy_attempt").
        const std::string payment_id = m_context->ids().payment_id("pay_");
        const std::string attempt_id = m_context->ids().payment_id(std::string{ATTEMPT_ID_PREFIX} + "_");

        // types.rs:405-419 - process_payment_attempt: redirect URL + PM flow.
        const std::string redirect_url = base_url() + "/dummy-connector/authorize/" + attempt_id;
        auto flow = process_payment_attempt(input, redirect_url);
        if (!flow) {
            send_dc_error(res, send, flow.error());
            return;
        }

        PaymentData data{};
        data.attempt_id = attempt_id;
        data.payment_id = payment_id;
        data.status = flow->status;
        data.amount = input.amount;
        data.eligible_amount = input.amount;
        data.currency = input.currency;
        data.created = now_iso8601();
        data.pm_type = to_payment_method_type(input.pm);
        data.connector = input.connector;
        data.next_action = std::move(flow->next_action);
        data.return_url = std::move(flow->return_url);

        // core.rs:27-40 - two Redis writes with authorize_ttl / payment_ttl.
        m_context->cache().put(attempt_id, core::cache::to_cache_value(payment_id),
                               ttl("authorize"));
        save_payment(data);

        m_context->metrics().record_flow("DummyPaymentCreate");
        m_context->metrics().counter("DUMMY_CONNECTOR_PAYMENT").increment();
        send_json(res, send, payment_response(data), interfaces::io::types::Status::OK);
    }

    // -----------------------------------------------------------------------
    // routes/dummy_connector.rs:87-105 -> core.rs:44-56 (payment_data).
    // -----------------------------------------------------------------------
    void handle_dummy_connector_payment_data(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto auth_error = authorize(req, "dummy_connector_payment_data")) {
            send_app_error(res, send, *auth_error);
            return;
        }
        if (const auto host_error = require_local_host(req)) {
            send_no_route(res, send);
            return;
        }
        apply_mock_latency("payment_retrieve_duration", "payment_retrieve_tolerance");

        const auto payment_id = last_segment(req);
        const auto payment = load_payment(payment_id);
        if (!payment) {
            // core.rs:54 -> utils.rs:67 - PaymentNotFound (DC_01).
            send_dc_error(res, send, dc_payment_not_found());
            return;
        }
        m_context->metrics().record_flow("DummyPaymentRetrieve");
        m_context->metrics().counter("DUMMY_CONNECTOR_PAYMENT_DATA").increment();
        send_json(res, send, payment_response(*payment), interfaces::io::types::Status::OK);
    }

    // -----------------------------------------------------------------------
    // routes/dummy_connector.rs:109-128 -> core.rs:156-207 (refund_payment).
    // -----------------------------------------------------------------------
    void handle_dummy_connector_refund(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto auth_error = authorize(req, "dummy_connector_refund")) {
            send_app_error(res, send, *auth_error);
            return;
        }
        if (const auto host_error = require_local_host(req)) {
            send_no_route(res, send);
            return;
        }
        apply_mock_latency("refund_duration", "refund_tolerance");

        // `/:payment_id/refund`: payment_id is the segment before `refund`.
        const auto segments = app::http::RequestPath{}(req);
        if (segments.count() < 2) {
            send_dc_error(res, send, dc_missing_field("payment_id"));
            return;
        }
        const std::string payment_id{segments.at(segments.count() - 2).value_or("")};
        if (payment_id.empty()) {
            send_dc_error(res, send, dc_missing_field("payment_id"));
            return;
        }

        const auto request = parse_refund_request(req.body_as_string());
        if (!request) {
            send_dc_error(res, send, request.error());
            return;
        }

        // core.rs:173-175 - get_payment_data_from_payment_id.
        auto payment = load_payment(payment_id);
        if (!payment) {
            send_dc_error(res, send, dc_payment_not_found());
            return;
        }

        // types.rs:301-315 - is_eligible_for_refund: amount first, then status.
        const auto eligibility = is_eligible_for_refund(*payment, request->amount);
        if (!eligibility) {
            send_dc_error(res, send, eligibility.error());
            return;
        }

        // core.rs:179-188 - refund id, decrement, persist payment with payment_ttl.
        const std::string refund_id = m_context->ids().payment_id(std::string{REFUND_ID_PREFIX} + "_");
        payment->eligible_amount -= request->amount;
        save_payment(*payment);

        // core.rs:190-205 - new RefundResponse, store with refund_ttl.
        RefundData refund{};
        refund.status = Status::Succeeded;
        refund.id = refund_id;
        refund.currency = payment->currency;
        refund.created = now_iso8601();
        refund.payment_amount = payment->amount;
        refund.refund_amount = request->amount;
        m_context->cache().put(refund.id, core::cache::to_cache_value(encode_refund_data(refund)),
                               ttl("refund"));

        m_context->metrics().record_flow("DummyRefundCreate");
        m_context->metrics().counter("DUMMY_CONNECTOR_REFUND").increment();
        send_json(res, send, refund_response(refund), interfaces::io::types::Status::OK);
    }

    // -----------------------------------------------------------------------
    // routes/dummy_connector.rs:132-150 -> core.rs:210-234 (refund_data).
    // -----------------------------------------------------------------------
    void handle_dummy_connector_refund_data(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto auth_error = authorize(req, "dummy_connector_refund_data")) {
            send_app_error(res, send, *auth_error);
            return;
        }
        if (const auto host_error = require_local_host(req)) {
            send_no_route(res, send);
            return;
        }
        apply_mock_latency("refund_retrieve_duration", "refund_retrieve_tolerance");

        const auto refund_id = last_segment(req);
        const auto stored = m_context->cache().get(refund_id);
        if (!stored.hit) {
            // core.rs:226-232 - RefundNotFound (DC_05).
            send_dc_error(res, send, dc_refund_not_found());
            return;
        }
        const auto refund = decode_refund_data(stored.value_or<std::string>(""));
        if (!refund) {
            send_dc_error(res, send, dc_internal());
            return;
        }
        m_context->metrics().record_flow("DummyRefundRetrieve");
        m_context->metrics().counter("DUMMY_CONNECTOR_REFUND_DATA").increment();
        send_json(res, send, refund_response(*refund), interfaces::io::types::Status::OK);
    }

    void install(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "/dummy-connector"}
            .add_route(
                Route{"/authorize/:attempt_id"}
                    .get([this](interfaces::io::IRequest& req, interfaces::io::IResponse& res,
                                std::function<void()> send) {
                        handle_dummy_connector_authorize_payment(req, res, std::move(send));
                    })
                    .summary("Hyperswitch dummy_connector_authorize_payment")
            )
            .add_route(
                Route{"/complete/:attempt_id"}
                    .get([this](interfaces::io::IRequest& req, interfaces::io::IResponse& res,
                                std::function<void()> send) {
                        handle_dummy_connector_complete_payment(req, res, std::move(send));
                    })
                    .summary("Hyperswitch dummy_connector_complete_payment")
            )
            .add_route(
                Route{"/payment"}
                    .post([this](interfaces::io::IRequest& req, interfaces::io::IResponse& res,
                                 std::function<void()> send) {
                        handle_dummy_connector_payment(req, res, std::move(send));
                    })
                    .summary("Hyperswitch dummy_connector_payment")
            )
            .add_route(
                Route{"/payments/:payment_id"}
                    .get([this](interfaces::io::IRequest& req, interfaces::io::IResponse& res,
                                std::function<void()> send) {
                        handle_dummy_connector_payment_data(req, res, std::move(send));
                    })
                    .summary("Hyperswitch dummy_connector_payment_data")
            )
            .add_route(
                Route{"/:payment_id/refund"}
                    .post([this](interfaces::io::IRequest& req, interfaces::io::IResponse& res,
                                 std::function<void()> send) {
                        handle_dummy_connector_refund(req, res, std::move(send));
                    })
                    .summary("Hyperswitch dummy_connector_refund")
            )
            .add_route(
                Route{"/refunds/:refund_id"}
                    .get([this](interfaces::io::IRequest& req, interfaces::io::IResponse& res,
                                std::function<void()> send) {
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
        // All six ops use auth::NoAuth (dummy_connector.rs:32,58,79,101,124,146);
        // the registry declares them NO_AUTH (registry.cppm:112-117). The
        // #[instrument(flow=...)]/LockAction::NotApplicable contract is mirrored by
        // record_flow/counter in each handler and the absence of a lock acquire.
        const auto rule = m_context->auth_rules().rule_for(operation);
        return m_context->authorizer().authorize_route(app::auth::AuthContextBuilder{}(req), rule);
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
        return m_context->config_store()
            .get_value("dummy_connector", key)
            .value_or(std::string{fallback});
    }

    [[nodiscard]] std::int64_t config_int(std::string_view key, std::int64_t fallback) const
    {
        const auto value = m_context->config_store().get_value("dummy_connector", key);
        if (!value) {
            return fallback;
        }
        std::int64_t parsed{0};
        const auto result = std::from_chars(value->data(), value->data() + value->size(), parsed);
        return result.ec == std::errc{} ? parsed : fallback;
    }

    /** @brief Upstream `state.base_url`; config-backed here (no router-level state URL). */
    [[nodiscard]] std::string base_url() const
    {
        return config_string("base_url", "http://localhost:8080");
    }

    [[nodiscard]] core::cache::Ttl ttl(std::string_view flow) const
    {
        // config.example.toml:747,754,759 - payment/refund 172800s, authorize 36000s.
        if (flow == "authorize") {
            return core::cache::Ttl{}.seconds(config_int("authorize_ttl", 36000));
        }
        if (flow == "refund") {
            return core::cache::Ttl{}.seconds(config_int("refund_ttl", 172800));
        }
        return core::cache::Ttl{}.seconds(config_int("payment_ttl", 172800));
    }

    /** @brief utils.rs:18-29 - tokio_mock_sleep(delay, tolerance). */
    void apply_mock_latency(std::string_view delay_key, std::string_view tolerance_key) const
    {
        const std::int64_t delay = config_int(delay_key, 0);
        const std::int64_t tolerance = config_int(tolerance_key, 0);
        if (delay <= 0) {
            return;
        }
        const std::int64_t low = delay > tolerance ? delay - tolerance : 0;
        const std::int64_t high = delay + tolerance;
        // Deterministic midpoint of the half-open upstream range [low, high).
        const std::int64_t effective = std::clamp<std::int64_t>((low + high) / 2, 0, 1000);
        if (effective > 0) {
            std::this_thread::sleep_for(std::chrono::milliseconds(effective));
        }
    }

    // -- path helpers -------------------------------------------------------
    [[nodiscard]] std::string last_segment(interfaces::io::IRequest& req) const
    {
        const auto segments = app::http::RequestPath{}(req);
        if (segments.count() == 0) {
            return {};
        }
        return std::string{segments.at(segments.count() - 1).value_or("")};
    }

    // -- request parsing ----------------------------------------------------
    [[nodiscard]] std::expected<PaymentRequest, DcError> parse_payment_request(
        std::string_view body
    ) const
    {
        auto tree = serde::Ser::decode_generic("application/json", body);
        if (!tree) {
            return std::unexpected{dc_error("IR_00", "invalid_request", "Json deserialize error",
                                            interfaces::io::types::Status::BAD_REQUEST)};
        }
        const auto* root = std::get_if<Object>(&tree->get());
        if (root == nullptr) {
            return std::unexpected{dc_error("IR_00", "invalid_request", "Json deserialize error",
                                            interfaces::io::types::Status::BAD_REQUEST)};
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
            return std::unexpected{dc_error("IR_00", "invalid_request", "Json deserialize error",
                                            interfaces::io::types::Status::BAD_REQUEST)};
        }
        request.pm = std::move(*pm);
        request.return_url = read_optional_string(*root, "return_url");
        if (!field(*root, "connector").has_value()) {
            return std::unexpected{dc_missing_field("connector")};
        }
        const auto connector = parse_connector(read_string(*root, "connector"));
        if (!connector) {
            return std::unexpected{dc_error("IR_00", "invalid_request", "Json deserialize error",
                                            interfaces::io::types::Status::BAD_REQUEST)};
        }
        request.connector = *connector;
        return request;
    }

    [[nodiscard]] std::expected<RefundRequest, DcError> parse_refund_request(
        std::string_view body
    ) const
    {
        auto tree = serde::Ser::decode_generic("application/json", body);
        if (!tree) {
            return std::unexpected{dc_error("IR_00", "invalid_request", "Json deserialize error",
                                            interfaces::io::types::Status::BAD_REQUEST)};
        }
        const auto* root = std::get_if<Object>(&tree->get());
        if (root == nullptr) {
            return std::unexpected{dc_error("IR_00", "invalid_request", "Json deserialize error",
                                            interfaces::io::types::Status::BAD_REQUEST)};
        }
        if (!field(*root, "amount").has_value()) {
            return std::unexpected{dc_missing_field("amount")};
        }
        return RefundRequest{read_int(*root, "amount"), {}};
    }

    // -- domain logic -------------------------------------------------------

    /** @brief utils.rs:294-350 - `get_flow_from_card_number`. */
    [[nodiscard]] std::expected<void, DcError> card_flow(
        const PaymentRequest& input,
        PaymentData& data,
        const std::string& redirect_url
    ) const
    {
        const std::string& number = input.pm.card_number;
        if (number == "4111111111111111" || number == "4242424242424242"
            || number == "5555555555554444" || number == "38000000000006"
            || number == "378282246310005" || number == "6011111111111117") {
            // NoThreeDS(Succeeded, None) -> build_payment_data(Succeeded, None, None).
            data.status = Status::Succeeded;
            return {};
        }
        if (number == "5105105105105100" || number == "4000000000000002") {
            return std::unexpected{dc_payment_declined("Card declined")};
        }
        if (number == "4000000000009995") {
            if (input.connector == Connector::StripeTest) {
                data.status = Status::Succeeded;
                return {};
            }
            return std::unexpected{dc_payment_declined(
                "Internal Server Error from Connector, Please try again later")};
        }
        if (number == "4000000000009987") {
            return std::unexpected{dc_payment_declined("Lost card")};
        }
        if (number == "4000000000009979") {
            return std::unexpected{dc_payment_declined("Stolen card")};
        }
        if (number == "4000003800000446") {
            // ThreeDS(Succeeded, _) -> Processing + RedirectToUrl + return_url.
            data.status = Status::Processing;
            data.next_action = NextAction{true, redirect_url};
            data.return_url = input.return_url;
            return {};
        }
        return std::unexpected{dc_card_not_supported()};
    }

    /** @brief utils.rs:267-292 - `get_flow_from_upi_collect`. */
    [[nodiscard]] std::expected<void, DcError> upi_flow(
        const PaymentRequest& input,
        PaymentData& data,
        const std::string& redirect_url
    ) const
    {
        if (input.pm.vpa_id == UPI_FAILURE_VPA) {
            return std::unexpected{dc_payment_not_successful()};
        }
        if (input.pm.vpa_id == UPI_SUCCESS_VPA) {
            data.status = Status::Processing;
            data.next_action = NextAction{true, redirect_url};
            data.return_url = input.return_url;
            return {};
        }
        return std::unexpected{dc_payment_declined("Invalid Upi id")};
    }

    struct AttemptResult
    {
        Status status{Status::Processing};
        std::optional<NextAction> next_action{};
        std::optional<std::string> return_url{};
    };

    /** @brief utils.rs:381-419 - `process_payment_attempt`. */
    [[nodiscard]] std::expected<AttemptResult, DcError> process_payment_attempt(
        const PaymentRequest& input,
        const std::string& redirect_url
    ) const
    {
        AttemptResult result{};
        PaymentData scratch{};
        switch (input.pm.kind) {
        case PaymentMethodData::Kind::Card: {
            auto flow = card_flow(input, scratch, redirect_url);
            if (!flow) {
                return std::unexpected{flow.error()};
            }
            break;
        }
        case PaymentMethodData::Kind::Upi: {
            auto flow = upi_flow(input, scratch, redirect_url);
            if (!flow) {
                return std::unexpected{flow.error()};
            }
            break;
        }
        case PaymentMethodData::Kind::Wallet:
        case PaymentMethodData::Kind::PayLater:
            // utils.rs:353-379 - Processing + RedirectToUrl + return_url.
            scratch.status = Status::Processing;
            scratch.next_action = NextAction{true, redirect_url};
            scratch.return_url = input.return_url;
            break;
        }
        result.status = scratch.status;
        result.next_action = scratch.next_action;
        result.return_url = scratch.return_url;
        return result;
    }

    /** @brief types.rs:301-315 - `is_eligible_for_refund`. */
    [[nodiscard]] std::expected<void, DcError> is_eligible_for_refund(
        const PaymentData& payment,
        std::int64_t refund_amount
    ) const
    {
        if (payment.eligible_amount < refund_amount) {
            return std::unexpected{dc_refund_amount_exceeds()};
        }
        if (payment.status != Status::Succeeded) {
            return std::unexpected{dc_payment_not_successful()};
        }
        return {};
    }

    // -- persistence (Redis keys, values and TTLs reproduced on the cache) --
    [[nodiscard]] std::optional<PaymentData> load_payment(std::string_view payment_id) const
    {
        const auto stored = m_context->cache().get(payment_id);
        if (!stored.hit) {
            return std::nullopt;
        }
        return decode_payment_data(stored.value_or<std::string>(""));
    }

    void save_payment(const PaymentData& data) const
    {
        m_context->cache().put(data.payment_id, core::cache::to_cache_value(encode_payment_data(data)),
                               ttl("payment"));
    }

    // -- HTML pages (utils.rs:95-209) ---------------------------------------
    [[nodiscard]] std::string authorize_page(const PaymentData& data, std::string_view return_url) const
    {
        const std::string assets = config_string("assets_base_url",
                                                 "https://app.hyperswitch.io/assets/TestProcessor/");
        const std::string image = pm_type_image(data.pm_type, assets);
        const std::string connector_image_url = connector_image(data.connector, assets);
        const std::string mode = pm_type_name(data.pm_type);
        const std::string currency = data.currency;

        std::string page{"<head><title>Authorize Payment</title><style>"};
        // maud escapes interpolated &str values (& < > ") inside <style>.
        page += html_escape(THREE_DS_CSS);
        page += "</style><link rel=\"icon\" href=\"" + html_escape(connector_image_url)
             + "\"/></head><body><div class=\"heading\">";
        page += "<img class=\"logo\" src=\"https://app.hyperswitch.io/assets/Dark/"
                "hyperswitchLogoIconWithText.svg\" alt=\"Hyperswitch Logo\"/>";
        page += "<h1>Test Payment Page</h1></div><div class=\"container\">"
                "<div class=\"payment_details\">";
        page += "<img src=\"" + html_escape(image) + "\"/>";
        page += "<div class=\"border_horizontal\"></div>";
        page += "<img src=\"" + html_escape(connector_image_url) + "\"/>";
        page += "</div>";
        // maud::PreEscaped block (utils.rs:126-135) - emitted verbatim.
        page += "\n                        <p class=\"disclaimer\">\n"
                "                            This is a test payment of <span id=\"amount\"></span> "
             + currency + " using " + mode
             + "\n                            <script>\n"
               "                                document.getElementById(\"amount\").innerHTML = ("
             + std::to_string(data.amount) + " / 100).toFixed(2);\n"
               "                            </script>\n"
               "                        </p>\n                        ";
        page += "<p><b>Real money will not be debited for the payment.</b> You can choose to "
                "simulate successful or failed payment while testing this payment.</p>";
        page += "<div class=\"user_action\">";
        const std::string authorize_js = "window.location.href='" + std::string{return_url} + "?confirm=true'";
        const std::string reject_js = "window.location.href='" + std::string{return_url} + "?confirm=false'";
        page += "<button class=\"authorize\" onclick=\"" + html_escape(authorize_js)
             + "\">Complete Payment</button>";
        page += "<button class=\"reject\" onclick=\"" + html_escape(reject_js)
             + "\">Reject Payment</button>";
        page += "</div></div><div class=\"container\"><p class=\"disclaimer\">What is this page?</p>";
        page += "<p>" + html_escape(
            "This page is just a simulation for integration and testing purpose. In live mode, "
            "this page will not be displayed and the user will be taken to the Bank page (or) "
            "Google Pay cards popup (or) original payment method's page. Contact us for any queries.")
             + "</p>";
        page += "<div class=\"contact\">";
        page += "<div class=\"contact_item hover_cursor\" onclick=\""
             + html_escape(config_string("slack_invite_url", "https://hyperswitch.io/slack"))
             + "\"><img src=\"https://hyperswitch.io/logos/logo_slack.svg\" alt=\"Slack Logo\"/></div>";
        page += "<div class=\"contact_item hover_cursor\" onclick=\""
             + html_escape(config_string("discord_invite_url", "https://hyperswitch.io/discord"))
             + "\"><img src=\"https://hyperswitch.io/logos/logo_discord.svg\" alt=\"Discord Logo\"/></div>";
        page += "<div class=\"border_vertical\"></div>";
        page += "<div class=\"contact_item email\"><p>Or email us at</p>"
                "<a href=\"mailto:support.global@juspay.io\">support.global@juspay.io</a></div>";
        page += "</div></div></body>";
        return page;
    }

    [[nodiscard]] std::string expired_page() const
    {
        std::string page{"<head><title>Authorize Payment</title><style>"};
        // maud escapes interpolated &str values (& < > ") inside <style>.
        page += html_escape(THREE_DS_CSS);
        page += "</style><link rel=\"icon\" href=\"https://app.hyperswitch.io/HyperswitchFavicon.png\"/>"
                "</head><body><div class=\"heading\">";
        page += "<img class=\"logo\" src=\"https://app.hyperswitch.io/assets/Dark/"
                "hyperswitchLogoIconWithText.svg\" alt=\"Hyperswitch Logo\"/>";
        page += "<h1>Test Payment Page</h1></div><div class=\"container\">"
                "<p class=\"disclaimer\">This link is not valid or it is expired</p></div>";
        page += "<div class=\"container\"><p class=\"disclaimer\">What is this page?</p>";
        page += "<p>" + html_escape(
            "This page is just a simulation for integration and testing purpose."
            "In live mode, this is not visible. Contact us for any queries.")
             + "</p>";
        page += "<div class=\"contact\">";
        page += "<div class=\"contact_item hover_cursor\" onclick=\""
             + html_escape(config_string("slack_invite_url", "https://hyperswitch.io/slack"))
             + "\"><img src=\"https://hyperswitch.io/logos/logo_slack.svg\" alt=\"Slack Logo\"/></div>";
        page += "<div class=\"contact_item hover_cursor\" onclick=\""
             + html_escape(config_string("discord_invite_url", "https://hyperswitch.io/discord"))
             + "\"><img src=\"https://hyperswitch.io/logos/logo_discord.svg\" alt=\"Discord Logo\"/></div>";
        page += "<div class=\"border_vertical\"></div>";
        page += "<div class=\"contact_item email\"><p>Or email us at</p>"
                "<a href=\"mailto:support.global@juspay.io\">support.global@juspay.io</a></div>";
        page += "</div></div></body>";
        return page;
    }

    // -- response sinks -----------------------------------------------------
    void send_app_error(interfaces::io::IResponse& res, std::function<void()> send,
                        core::error::AppError error) const
    {
        auto text = serde::Ser::encode_json(error);
        auto bytes = serde::Ser::serialize_raw("application/json", text);
        res.add_content_type("application/json");
        res.set_status(core::error::AppErrorStatus{}(error.get_kind()));
        res.set_body(std::move(bytes));
        send();
    }

    void send_dc_error(interfaces::io::IResponse& res, std::function<void()> send,
                       DcError error) const
    {
        auto bytes = serde::Ser::serialize_raw("application/json", dc_error_body(error));
        res.add_content_type("application/json");
        res.set_status(error.status);
        res.set_body(std::move(bytes));
        send();
    }

    void send_json(interfaces::io::IResponse& res, std::function<void()> send,
                   std::string_view text, interfaces::io::types::Status status) const
    {
        auto bytes = serde::Ser::serialize_raw("application/json", text);
        res.add_content_type("application/json");
        res.set_status(status);
        res.set_body(std::move(bytes));
        send();
    }

    void send_redirect(interfaces::io::IResponse& res, std::function<void()> send,
                       std::string_view text, std::string_view location) const
    {
        auto bytes = serde::Ser::serialize_raw("application/json", text);
        res.add_content_type("application/json");
        res.add_location(location);
        res.set_status(interfaces::io::types::Status::FOUND);
        res.set_body(std::move(bytes));
        send();
    }

    void send_html(interfaces::io::IResponse& res, std::function<void()> send,
                   std::string_view html, interfaces::io::types::Status status) const
    {
        auto bytes = serde::Ser::serialize_raw("text/html", html);
        res.add_content_type("text/html");
        res.set_status(status);
        res.set_body(std::move(bytes));
        send();
    }

    void send_no_route(interfaces::io::IResponse& res, std::function<void()> send) const
    {
        // A failed actix Host guard means no matching resource: 404.
        res.set_status(interfaces::io::types::Status::NOT_FOUND);
        res.set_body(std::vector<std::byte>{});
        send();
    }

    app::context::AppContext* m_context{nullptr};
};

} // namespace app::handlers::dummy_connector
