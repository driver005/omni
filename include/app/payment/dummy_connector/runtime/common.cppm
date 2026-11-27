module;
#include <time.h>
export module app_payment:dummy_connector_runtime_common;

import std;
import interfaces;
import core_serde;
import core_cache;
import core_error;
import app_context;
import app_metrics;

import :dummy_connector_input;
import :dummy_connector_result;

export namespace app::handlers::dummy_connector {

// consts.rs:3 - include_str!("threeds_page.css"). The asset is compiled into
// the upstream binary; embedding it verbatim here keeps the two HTML pages
// byte-identical without a runtime file dependency.
inline constexpr std::string_view THREE_DS_CSS{
    R"DC_CSS(@import url('https://fonts.googleapis.com/css2?family=Inter:wght@200;300;400;500;600;700&display=swap');
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
})DC_CSS"
};

/** @brief Result of utils.rs:381-419 - `process_payment_attempt`. */
struct AttemptResult
{
    Status status{Status::Processing};
    std::optional<NextAction> next_action{};
    std::optional<std::string> return_url{};
};

/** @brief Shared business helpers for the dummy-connector sub-runtimes. */
class DummyConnectorRuntimeSupport
{
public:
    explicit DummyConnectorRuntimeSupport(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_values{context}
    {
    }

    [[nodiscard]] std::string jstr(std::string_view value) const
    {
        return m_values.jstr(value);
    }

    /** @brief Shared parse/value/DC-error helpers (upstream errors.rs constructors). */
    [[nodiscard]] const DummyConnectorInputSupport& values() const noexcept
    {
        return m_values;
    }

    [[nodiscard]] std::int64_t config_int(std::string_view key, std::int64_t fallback) const
    {
        const auto value = m_context.config_store().get_value("dummy_connector", key);
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
        return m_values.config_string("base_url", "http://localhost:8080");
    }

    [[nodiscard]] core::cache::Ttl ttl(std::string_view flow) const
    {
        // config.example.toml:747,754,759 - payment/refund 172800s, authorize 36000s.
        if (flow == "authorize") {
            return core::cache::Ttl{}.seconds(config_int("authorize_ttl", 36'000));
        }
        if (flow == "refund") {
            return core::cache::Ttl{}.seconds(config_int("refund_ttl", 172'800));
        }
        return core::cache::Ttl{}.seconds(config_int("payment_ttl", 172'800));
    }

    /** @brief common_utils::date_time::now() + custom_serde::iso8601: YYYY-MM-DDTHH:MM:SS.mmmZ. */
    [[nodiscard]] std::string now_iso8601() const
    {
        const auto now = std::chrono::system_clock::now();
        const auto millis =
            std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
        const std::time_t seconds = static_cast<std::time_t>(millis / 1'000);
        std::tm broken{};
        gmtime_r(&seconds, &broken);
        char buffer[64]{};
        std::snprintf(
            buffer,
            sizeof(buffer),
            "%04d-%02d-%02dT%02d:%02d:%02d.%03lldZ",
            broken.tm_year + 1'900,
            broken.tm_mon + 1,
            broken.tm_mday,
            broken.tm_hour,
            broken.tm_min,
            broken.tm_sec,
            static_cast<long long>(millis % 1'000)
        );
        return std::string{buffer};
    }

    [[nodiscard]] std::string html_escape(std::string_view value) const
    {
        std::string out{};
        for (const char c: value) {
            switch (c) {
                case '&':
                    out += "&amp;";
                    break;
                case '<':
                    out += "&lt;";
                    break;
                case '>':
                    out += "&gt;";
                    break;
                case '"':
                    out += "&quot;";
                    break;
                default:
                    out += c;
            }
        }
        return out;
    }

    /** @brief `DummyConnectors::get_connector_image_link` (types.rs:44-56). */
    [[nodiscard]] std::string connector_image(Connector connector, std::string_view assets) const
    {
        std::string_view name{"PHONYPAY.svg"};
        switch (connector) {
            case Connector::PhonyPay:
                name = "PHONYPAY.svg";
                break;
            case Connector::FauxPay:
                name = "FAUXPAY.svg";
                break;
            case Connector::PretendPay:
                name = "PRETENDPAY.svg";
                break;
            case Connector::StripeTest:
                name = "STRIPE_TEST.svg";
                break;
            case Connector::PaypalTest:
                name = "PAYPAL_TEST.svg";
                break;
            default:
                name = "PHONYPAY.svg";
                break;
        }
        return std::string{assets} + std::string{name};
    }

    /** @brief `GetPaymentMethodDetails::get_name` (types.rs:236-246). */
    [[nodiscard]] std::string_view wallet_name(Wallet wallet) const noexcept
    {
        switch (wallet) {
            case Wallet::GooglePay:
                return "Google Pay";
            case Wallet::Paypal:
                return "PayPal";
            case Wallet::WeChatPay:
                return "WeChat Pay";
            case Wallet::MbWay:
                return "Mb Way";
            case Wallet::AliPay:
                return "Alipay";
            case Wallet::AliPayHK:
                return "Alipay HK";
        }
        return "Google Pay";
    }

    [[nodiscard]] std::string_view wallet_image(Wallet wallet) const noexcept
    {
        switch (wallet) {
            case Wallet::GooglePay:
                return "GOOGLE_PAY.svg";
            case Wallet::Paypal:
                return "PAYPAL.svg";
            case Wallet::WeChatPay:
                return "WECHAT_PAY.svg";
            case Wallet::MbWay:
                return "MBWAY.svg";
            case Wallet::AliPay:
                return "ALIPAY.svg";
            case Wallet::AliPayHK:
                return "ALIPAY.svg";
        }
        return "GOOGLE_PAY.svg";
    }

    [[nodiscard]] std::string_view pay_later_name(PayLater value) const noexcept
    {
        switch (value) {
            case PayLater::Klarna:
                return "Klarna";
            case PayLater::Affirm:
                return "Affirm";
            case PayLater::AfterPayClearPay:
                return "Afterpay Clearpay";
        }
        return "Klarna";
    }

    [[nodiscard]] std::string_view pay_later_image(PayLater value) const noexcept
    {
        switch (value) {
            case PayLater::Klarna:
                return "KLARNA.svg";
            case PayLater::Affirm:
                return "AFFIRM.svg";
            case PayLater::AfterPayClearPay:
                return "AFTERPAY.svg";
        }
        return "KLARNA.svg";
    }

    /** @brief `From<DummyConnectorPaymentMethodData> for DummyConnectorPaymentMethodType`
     * (types.rs:149-160). */
    [[nodiscard]] PaymentMethodType
    to_payment_method_type(const PaymentMethodData& data) const noexcept
    {
        PaymentMethodType out{};
        out.kind = static_cast<PaymentMethodType::Kind>(data.kind);
        out.wallet = data.wallet;
        out.pay_later = data.pay_later;
        return out;
    }

    /** @brief Externally-tagged, `rename_all = "lowercase"` wire form (types.rs:137-147). */
    [[nodiscard]] std::string pm_type_wire(const PaymentMethodType& type) const
    {
        switch (type.kind) {
            case PaymentMethodType::Kind::Card:
                return "\"card\"";
            case PaymentMethodType::Kind::Upi:
                return "{\"upi\":\"UpiCollect\"}";
            case PaymentMethodType::Kind::Wallet:
                return "{\"wallet\":" + m_values.jstr(m_values.wallet_wire(type.wallet)) + "}";
            case PaymentMethodType::Kind::PayLater:
                return "{\"pay_later\":" + m_values.jstr(m_values.pay_later_wire(type.pay_later)) +
                       "}";
        }
        return "\"card\"";
    }

    /** @brief `GetPaymentMethodDetails::get_name` (types.rs:162-170). */
    [[nodiscard]] std::string pm_type_name(const PaymentMethodType& type) const
    {
        switch (type.kind) {
            case PaymentMethodType::Kind::Card:
                return "3D Secure";
            case PaymentMethodType::Kind::Upi:
                return "UPI Collect";
            case PaymentMethodType::Kind::Wallet:
                return std::string{wallet_name(type.wallet)};
            case PaymentMethodType::Kind::PayLater:
                return std::string{pay_later_name(type.pay_later)};
        }
        return "3D Secure";
    }

    /** @brief `GetPaymentMethodDetails::get_image_link` (types.rs:171-180). */
    [[nodiscard]] std::string
    pm_type_image(const PaymentMethodType& type, std::string_view assets) const
    {
        std::string_view name{"CARD.svg"};
        switch (type.kind) {
            case PaymentMethodType::Kind::Card:
                name = "CARD.svg";
                break;
            case PaymentMethodType::Kind::Upi:
                name = "UPI_COLLECT.svg";
                break;
            case PaymentMethodType::Kind::Wallet:
                name = wallet_image(type.wallet);
                break;
            case PaymentMethodType::Kind::PayLater:
                name = pay_later_image(type.pay_later);
                break;
        }
        return std::string{assets} + std::string{name};
    }

    /** @brief `Option<DummyConnectorNextAction>` (types.rs:317-321, snake_case). */
    [[nodiscard]] std::string next_action_wire(const std::optional<NextAction>& action) const
    {
        if (!action || !action->redirect) {
            return "null";
        }
        return "{\"redirect_to_url\":" + m_values.jstr(action->url) + "}";
    }

    /** @brief Externally-tagged payment_method_type as a typed Value document. */
    [[nodiscard]] serde::Value pm_type_value(const PaymentMethodType& type) const
    {
        switch (type.kind) {
            case PaymentMethodType::Kind::Card:
                return serde::Value{std::string{"card"}};
            case PaymentMethodType::Kind::Upi:
                {
                    serde::Value::Object object{};
                    object["upi"] = serde::Value{std::string{"UpiCollect"}};
                    return serde::Value{std::move(object)};
                }
            case PaymentMethodType::Kind::Wallet:
                {
                    serde::Value::Object object{};
                    object["wallet"] = serde::Value{std::string{m_values.wallet_wire(type.wallet)}};
                    return serde::Value{std::move(object)};
                }
            case PaymentMethodType::Kind::PayLater:
                {
                    serde::Value::Object object{};
                    object["pay_later"] =
                        serde::Value{std::string{m_values.pay_later_wire(type.pay_later)}};
                    return serde::Value{std::move(object)};
                }
        }
        return serde::Value{std::string{"card"}};
    }

    /** @brief Optional next_action as a typed Value document (null when absent). */
    [[nodiscard]] serde::Value next_action_value(const std::optional<NextAction>& action) const
    {
        if (!action || !action->redirect) {
            return serde::Value{};
        }
        serde::Value::Object object{};
        object["redirect_to_url"] = serde::Value{action->url};
        return serde::Value{std::move(object)};
    }

    /** @brief Decode the persisted `payment_method_type` (externally tagged). */
    [[nodiscard]] PaymentMethodType parse_pm_type(const interfaces::Value& value) const
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
        if (m_values.field(*object, "upi").has_value()) {
            out.kind = PaymentMethodType::Kind::Upi;
        } else if (const auto wallet = m_values.field(*object, "wallet")) {
            if (const auto* text = std::get_if<std::string>(&wallet->get())) {
                out.kind = PaymentMethodType::Kind::Wallet;
                out.wallet = m_values.parse_wallet(*text).value_or(Wallet::GooglePay);
            }
        } else if (const auto pay_later = m_values.field(*object, "pay_later")) {
            if (const auto* text = std::get_if<std::string>(&pay_later->get())) {
                out.kind = PaymentMethodType::Kind::PayLater;
                out.pay_later = m_values.parse_pay_later(*text).value_or(PayLater::Klarna);
            }
        }
        return out;
    }

    // -- persistence codecs (the values stored under the upstream Redis keys) -----
    [[nodiscard]] std::string encode_payment_data(const PaymentData& data) const
    {
        return "{\"attempt_id\":" + m_values.jstr(data.attempt_id) +
               ",\"payment_id\":" + m_values.jstr(data.payment_id) +
               ",\"status\":" + m_values.jstr(m_values.status_wire(data.status)) +
               ",\"amount\":" + std::to_string(data.amount) +
               ",\"eligible_amount\":" + std::to_string(data.eligible_amount) +
               ",\"currency\":" + m_values.jstr(data.currency) +
               ",\"created\":" + m_values.jstr(data.created) +
               ",\"payment_method_type\":" + pm_type_wire(data.pm_type) +
               ",\"connector\":" + m_values.jstr(m_values.connector_wire(data.connector)) +
               ",\"next_action\":" + next_action_wire(data.next_action) + ",\"return_url\":" +
               (data.return_url ? m_values.jstr(*data.return_url) : std::string{"null"}) + "}";
    }

    [[nodiscard]] std::optional<PaymentData> decode_payment_data(std::string_view json) const
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
        out.attempt_id = m_values.read_string(*object, "attempt_id");
        out.payment_id = m_values.read_string(*object, "payment_id");
        out.status = m_values.parse_status(m_values.read_string(*object, "status"))
                         .value_or(Status::Processing);
        out.amount = m_values.read_int(*object, "amount");
        out.eligible_amount = m_values.read_int(*object, "eligible_amount");
        out.currency = m_values.read_string(*object, "currency");
        out.created = m_values.read_string(*object, "created");
        out.connector = m_values.parse_connector(m_values.read_string(*object, "connector"))
                            .value_or(Connector::PhonyPay);
        out.return_url = m_values.read_optional_string(*object, "return_url");
        if (const auto pm = m_values.field(*object, "payment_method_type")) {
            out.pm_type = parse_pm_type(*pm);
        }
        if (const auto action = m_values.field(*object, "next_action")) {
            const auto* action_object = std::get_if<Object>(&action->get());
            if (action_object != nullptr) {
                if (const auto url = m_values.field(*action_object, "redirect_to_url")) {
                    if (const auto* text = std::get_if<std::string>(&url->get())) {
                        out.next_action = NextAction{true, *text};
                    }
                }
            }
        }
        return out;
    }

    [[nodiscard]] std::string encode_refund_data(const RefundData& data) const
    {
        return "{\"status\":" + m_values.jstr(m_values.status_wire(data.status)) +
               ",\"id\":" + m_values.jstr(data.id) +
               ",\"currency\":" + m_values.jstr(data.currency) +
               ",\"created\":" + m_values.jstr(data.created) +
               ",\"payment_amount\":" + std::to_string(data.payment_amount) +
               ",\"refund_amount\":" + std::to_string(data.refund_amount) + "}";
    }

    [[nodiscard]] std::optional<RefundData> decode_refund_data(std::string_view json) const
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
        out.status = m_values.parse_status(m_values.read_string(*object, "status"))
                         .value_or(Status::Succeeded);
        out.id = m_values.read_string(*object, "id");
        out.currency = m_values.read_string(*object, "currency");
        out.created = m_values.read_string(*object, "created");
        out.payment_amount = m_values.read_int(*object, "payment_amount");
        out.refund_amount = m_values.read_int(*object, "refund_amount");
        return out;
    }

    /** @brief `DummyConnectorPaymentResponse` (types.rs:323-333) as a Value document. */
    [[nodiscard]] serde::Value payment_response(const PaymentData& data) const
    {
        serde::Value::Object document{};
        document["status"] = serde::Value{std::string{m_values.status_wire(data.status)}};
        document["id"] = serde::Value{data.payment_id};
        document["amount"] = serde::Value{data.amount};
        document["currency"] = serde::Value{data.currency};
        document["created"] = serde::Value{data.created};
        document["payment_method_type"] = pm_type_value(data.pm_type);
        document["next_action"] = next_action_value(data.next_action);
        return serde::Value{std::move(document)};
    }

    /** @brief `RedirectionResponse` for payment_complete as a Value document. */
    [[nodiscard]] serde::Value redirect_response(std::string_view return_url) const
    {
        serde::Value::Object document{};
        document["return_url"] = serde::Value{std::string{}};
        document["params"] = serde::Value{serde::Value::Array{}};
        document["return_url_with_query_params"] = serde::Value{std::string{return_url}};
        document["http_method"] = serde::Value{std::string{"GET"}};
        document["headers"] = serde::Value{serde::Value::Array{}};
        return serde::Value{std::move(document)};
    }

    /** @brief `DummyConnectorRefundResponse` (types.rs:376-385) as a typed DTO. */
    [[nodiscard]] ::app::handlers::dummy_connector::DummyConnectorRefundWire
    refund_response(const RefundData& data) const
    {
        return ::app::handlers::dummy_connector::DummyConnectorRefundWire{
            std::string{m_values.status_wire(data.status)},
            data.id,
            data.currency,
            data.created,
            data.payment_amount,
            data.refund_amount
        };
    }

    /** @brief Retrieve-route variant of the same flat refund response DTO. */
    [[nodiscard]] ::app::handlers::dummy_connector::DummyConnectorRefundWire
    refund_data_response(const RefundData& data) const
    {
        return ::app::handlers::dummy_connector::DummyConnectorRefundWire{
            std::string{m_values.status_wire(data.status)},
            data.id,
            data.currency,
            data.created,
            data.payment_amount,
            data.refund_amount
        };
    }

    // -- domain logic -------------------------------------------------------

    /** @brief utils.rs:294-350 - `get_flow_from_card_number`. */
    [[nodiscard]] std::expected<void, core::error::AppError>
    card_flow(const PaymentRequest& input, PaymentData& data, const std::string& redirect_url) const
    {
        const std::string& number = input.pm.card_number;
        if (number == "4111111111111111" || number == "4242424242424242" ||
            number == "5555555555554444" || number == "38000000000006" ||
            number == "378282246310005" || number == "6011111111111117") {
            // NoThreeDS(Succeeded, None) -> build_payment_data(Succeeded, None, None).
            data.status = Status::Succeeded;
            return {};
        }
        if (number == "5105105105105100" || number == "4000000000000002") {
            return std::unexpected{m_values.dc_payment_declined("Card declined")};
        }
        if (number == "4000000000009995") {
            if (input.connector == Connector::StripeTest) {
                data.status = Status::Succeeded;
                return {};
            }
            return std::unexpected{m_values.dc_payment_declined(
                "Internal Server Error from Connector, Please try again later"
            )};
        }
        if (number == "4000000000009987") {
            return std::unexpected{m_values.dc_payment_declined("Lost card")};
        }
        if (number == "4000000000009979") {
            return std::unexpected{m_values.dc_payment_declined("Stolen card")};
        }
        if (number == "4000003800000446") {
            // ThreeDS(Succeeded, _) -> Processing + RedirectToUrl + return_url.
            data.status = Status::Processing;
            data.next_action = NextAction{true, redirect_url};
            data.return_url = input.return_url;
            return {};
        }
        return std::unexpected{m_values.dc_card_not_supported()};
    }

    /** @brief utils.rs:267-292 - `get_flow_from_upi_collect`. */
    [[nodiscard]] std::expected<void, core::error::AppError>
    upi_flow(const PaymentRequest& input, PaymentData& data, const std::string& redirect_url) const
    {
        if (input.pm.vpa_id == UPI_FAILURE_VPA) {
            return std::unexpected{m_values.dc_payment_not_successful()};
        }
        if (input.pm.vpa_id == UPI_SUCCESS_VPA) {
            data.status = Status::Processing;
            data.next_action = NextAction{true, redirect_url};
            data.return_url = input.return_url;
            return {};
        }
        return std::unexpected{m_values.dc_payment_declined("Invalid Upi id")};
    }

    /** @brief utils.rs:381-419 - `process_payment_attempt`. */
    [[nodiscard]] std::expected<AttemptResult, core::error::AppError>
    process_payment_attempt(const PaymentRequest& input, const std::string& redirect_url) const
    {
        AttemptResult result{};
        PaymentData scratch{};
        switch (input.pm.kind) {
            case PaymentMethodData::Kind::Card:
                {
                    auto flow = card_flow(input, scratch, redirect_url);
                    if (!flow) {
                        return std::unexpected{flow.error()};
                    }
                    break;
                }
            case PaymentMethodData::Kind::Upi:
                {
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
    [[nodiscard]] std::expected<void, core::error::AppError>
    is_eligible_for_refund(const PaymentData& payment, std::int64_t refund_amount) const
    {
        if (payment.eligible_amount < refund_amount) {
            return std::unexpected{m_values.dc_refund_amount_exceeds()};
        }
        if (payment.status != Status::Succeeded) {
            return std::unexpected{m_values.dc_payment_not_successful()};
        }
        return {};
    }

    // -- persistence (Redis keys, values and TTLs reproduced on the cache) --
    [[nodiscard]] std::optional<PaymentData> load_payment(std::string_view payment_id) const
    {
        const auto stored = m_context.cache().get(payment_id);
        if (!stored.hit) {
            return std::nullopt;
        }
        return decode_payment_data(stored.value_or<std::string>(""));
    }

    void save_payment(const PaymentData& data) const
    {
        m_context.cache().put(
            data.payment_id,
            core::cache::to_cache_value(encode_payment_data(data)),
            ttl("payment")
        );
    }

    // -- HTML pages (utils.rs:95-209) ---------------------------------------
    [[nodiscard]] std::string
    authorize_page(const PaymentData& data, std::string_view return_url) const
    {
        const std::string assets = m_values.config_string(
            "assets_base_url",
            "https://app.hyperswitch.io/assets/TestProcessor/"
        );
        const std::string image = pm_type_image(data.pm_type, assets);
        const std::string connector_image_url = connector_image(data.connector, assets);
        const std::string mode = pm_type_name(data.pm_type);
        const std::string currency = data.currency;

        std::string page{"<head><title>Authorize Payment</title><style>"};
        // maud escapes interpolated &str values (& < > ") inside <style>.
        page += html_escape(THREE_DS_CSS);
        page += "</style><link rel=\"icon\" href=\"" + html_escape(connector_image_url) +
                "\"/></head><body><div class=\"heading\">";
        page += "<img class=\"logo\" src=\"https://app.hyperswitch.io/assets/Dark/"
                "hyperswitchLogoIconWithText.svg\" alt=\"Hyperswitch Logo\"/>";
        page += "<h1>Test Payment Page</h1></div><div class=\"container\">"
                "<div class=\"payment_details\">";
        page += "<img src=\"" + html_escape(image) + "\"/>";
        page += "<div class=\"border_horizontal\"></div>";
        page += "<img src=\"" + html_escape(connector_image_url) + "\"/>";
        page += "</div>";
        // maud::PreEscaped block (utils.rs:126-135) - emitted verbatim.
        page +=
            "\n                        <p class=\"disclaimer\">\n"
            "                            This is a test payment of <span id=\"amount\"></span> " +
            currency + " using " + mode +
            "\n                            <script>\n"
            "                                document.getElementById(\"amount\").innerHTML = (" +
            std::to_string(data.amount) +
            " / 100).toFixed(2);\n"
            "                            </script>\n"
            "                        </p>\n                        ";
        page += "<p><b>Real money will not be debited for the payment.</b> You can choose to "
                "simulate successful or failed payment while testing this payment.</p>";
        page += "<div class=\"user_action\">";
        const std::string authorize_js =
            "window.location.href='" + std::string{return_url} + "?confirm=true'";
        const std::string reject_js =
            "window.location.href='" + std::string{return_url} + "?confirm=false'";
        page += "<button class=\"authorize\" onclick=\"" + html_escape(authorize_js) +
                "\">Complete Payment</button>";
        page += "<button class=\"reject\" onclick=\"" + html_escape(reject_js) +
                "\">Reject Payment</button>";
        page +=
            "</div></div><div class=\"container\"><p class=\"disclaimer\">What is this page?</p>";
        page +=
            "<p>" +
            html_escape(
                "This page is just a simulation for integration and testing purpose. In live mode, "
                "this page will not be displayed and the user will be taken to the Bank page (or) "
                "Google Pay cards popup (or) original payment method's page. Contact us for any "
                "queries."
            ) +
            "</p>";
        page += "<div class=\"contact\">";
        page += "<div class=\"contact_item hover_cursor\" onclick=\"" +
                html_escape(
                    m_values.config_string("slack_invite_url", "https://hyperswitch.io/slack")
                ) +
                "\"><img src=\"https://hyperswitch.io/logos/logo_slack.svg\" alt=\"Slack "
                "Logo\"/></div>";
        page += "<div class=\"contact_item hover_cursor\" onclick=\"" +
                html_escape(
                    m_values.config_string("discord_invite_url", "https://hyperswitch.io/discord")
                ) +
                "\"><img src=\"https://hyperswitch.io/logos/logo_discord.svg\" alt=\"Discord "
                "Logo\"/></div>";
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
        page += "</style><link rel=\"icon\" "
                "href=\"https://app.hyperswitch.io/HyperswitchFavicon.png\"/>"
                "</head><body><div class=\"heading\">";
        page += "<img class=\"logo\" src=\"https://app.hyperswitch.io/assets/Dark/"
                "hyperswitchLogoIconWithText.svg\" alt=\"Hyperswitch Logo\"/>";
        page += "<h1>Test Payment Page</h1></div><div class=\"container\">"
                "<p class=\"disclaimer\">This link is not valid or it is expired</p></div>";
        page += "<div class=\"container\"><p class=\"disclaimer\">What is this page?</p>";
        page += "<p>" +
                html_escape(
                    "This page is just a simulation for integration and testing purpose."
                    "In live mode, this is not visible. Contact us for any queries."
                ) +
                "</p>";
        page += "<div class=\"contact\">";
        page += "<div class=\"contact_item hover_cursor\" onclick=\"" +
                html_escape(
                    m_values.config_string("slack_invite_url", "https://hyperswitch.io/slack")
                ) +
                "\"><img src=\"https://hyperswitch.io/logos/logo_slack.svg\" alt=\"Slack "
                "Logo\"/></div>";
        page += "<div class=\"contact_item hover_cursor\" onclick=\"" +
                html_escape(
                    m_values.config_string("discord_invite_url", "https://hyperswitch.io/discord")
                ) +
                "\"><img src=\"https://hyperswitch.io/logos/logo_discord.svg\" alt=\"Discord "
                "Logo\"/></div>";
        page += "<div class=\"border_vertical\"></div>";
        page += "<div class=\"contact_item email\"><p>Or email us at</p>"
                "<a href=\"mailto:support.global@juspay.io\">support.global@juspay.io</a></div>";
        page += "</div></div></body>";
        return page;
    }

private:
    ::app::context::AppContext& m_context;
    DummyConnectorInputSupport m_values;
};

} // namespace app::handlers::dummy_connector
