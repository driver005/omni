module;
export module app_dto:relay;

import std;
import core_serde;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace app::dto::relay {

class CaptureMethodValue
{
public:
    void set_value(std::string value) { this->value = std::move(value); }
    [[nodiscard]] const std::string& get_value() const noexcept { return value; }
    std::string value{};
};

class CurrencyValue
{
public:
    void set_value(std::string value) { this->value = std::move(value); }
    [[nodiscard]] const std::string& get_value() const noexcept { return value; }
    std::string value{};
};

class MerchantConnectorAccountIdValue
{
public:
    void set_value(std::string value) { this->value = std::move(value); }
    [[nodiscard]] const std::string& get_value() const noexcept { return value; }
    std::string value{};
};

class MinorUnitValue
{
public:
    void set_value(std::string value) { this->value = std::move(value); }
    [[nodiscard]] const std::string& get_value() const noexcept { return value; }
    std::string value{};
};

class ProfileIdValue
{
public:
    void set_value(std::string value) { this->value = std::move(value); }
    [[nodiscard]] const std::string& get_value() const noexcept { return value; }
    std::string value{};
};

class RecipientPaymentMethodDataValue
{
public:
    void set_value(std::string value) { this->value = std::move(value); }
    [[nodiscard]] const std::string& get_value() const noexcept { return value; }
    std::string value{};
};

enum class RelayData
{
    Refund,
    Capture,
    IncrementalAuthorization,
    Void,
    UnreferencedRefund
};

class RelayError
{
public:
    void set_code(std::string value) { code = std::move(value); }
    [[nodiscard]] const std::string& get_code() const noexcept { return code; }
    void set_message(std::string value) { message = std::move(value); }
    [[nodiscard]] const std::string& get_message() const noexcept { return message; }
    std::string code{};
    std::string message{};
};

class RelayIdValue
{
public:
    void set_value(std::string value) { this->value = std::move(value); }
    [[nodiscard]] const std::string& get_value() const noexcept { return value; }
    std::string value{};
};

class RelayRetrieveBody
{
public:
    void set_force_sync(bool value) { force_sync = std::move(value); }
    [[nodiscard]] const bool& get_force_sync() const noexcept { return force_sync; }
    bool force_sync{};
};

class RelayStatusValue
{
public:
    void set_value(std::string value) { this->value = std::move(value); }
    [[nodiscard]] const std::string& get_value() const noexcept { return value; }
    std::string value{};
};

class RelayTypeValue
{
public:
    void set_value(std::string value) { this->value = std::move(value); }
    [[nodiscard]] const std::string& get_value() const noexcept { return value; }
    std::string value{};
};

class RelayCaptureRequestData
{
public:
    void set_authorized_amount(MinorUnitValue value) { authorized_amount = std::move(value); }
    [[nodiscard]] const MinorUnitValue& get_authorized_amount() const noexcept { return authorized_amount; }
    void set_amount_to_capture(MinorUnitValue value) { amount_to_capture = std::move(value); }
    [[nodiscard]] const MinorUnitValue& get_amount_to_capture() const noexcept { return amount_to_capture; }
    void set_currency(CurrencyValue value) { currency = std::move(value); }
    [[nodiscard]] const CurrencyValue& get_currency() const noexcept { return currency; }
    void set_capture_method(std::optional<CaptureMethodValue> value) { capture_method = std::move(value); }
    [[nodiscard]] const std::optional<CaptureMethodValue>& get_capture_method() const noexcept { return capture_method; }
    MinorUnitValue authorized_amount{};
    MinorUnitValue amount_to_capture{};
    CurrencyValue currency{};
    std::optional<CaptureMethodValue> capture_method{};
};

class RelayIncrementalAuthorizationRequestData
{
public:
    void set_total_amount(MinorUnitValue value) { total_amount = std::move(value); }
    [[nodiscard]] const MinorUnitValue& get_total_amount() const noexcept { return total_amount; }
    void set_additional_amount(MinorUnitValue value) { additional_amount = std::move(value); }
    [[nodiscard]] const MinorUnitValue& get_additional_amount() const noexcept { return additional_amount; }
    void set_currency(CurrencyValue value) { currency = std::move(value); }
    [[nodiscard]] const CurrencyValue& get_currency() const noexcept { return currency; }
    MinorUnitValue total_amount{};
    MinorUnitValue additional_amount{};
    CurrencyValue currency{};
};

class RelayRefundRequestData
{
public:
    void set_amount(MinorUnitValue value) { amount = std::move(value); }
    [[nodiscard]] const MinorUnitValue& get_amount() const noexcept { return amount; }
    void set_currency(CurrencyValue value) { currency = std::move(value); }
    [[nodiscard]] const CurrencyValue& get_currency() const noexcept { return currency; }
    void set_reason(std::optional<std::string> value) { reason = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_reason() const noexcept { return reason; }
    MinorUnitValue amount{};
    CurrencyValue currency{};
    std::optional<std::string> reason{};
};

class RelayRequest
{
public:
    void set_connector_resource_id(std::string value) { connector_resource_id = std::move(value); }
    [[nodiscard]] const std::string& get_connector_resource_id() const noexcept { return connector_resource_id; }
    void set_connector_id(MerchantConnectorAccountIdValue value) { connector_id = std::move(value); }
    [[nodiscard]] const MerchantConnectorAccountIdValue& get_connector_id() const noexcept { return connector_id; }
    void set_relay_type(RelayTypeValue value) { relay_type = std::move(value); }
    [[nodiscard]] const RelayTypeValue& get_relay_type() const noexcept { return relay_type; }
    void set_data(std::optional<RelayData> value) { data = std::move(value); }
    [[nodiscard]] const std::optional<RelayData>& get_data() const noexcept { return data; }
    std::string connector_resource_id{};
    MerchantConnectorAccountIdValue connector_id{};
    RelayTypeValue relay_type{};
    std::optional<RelayData> data{};
};

class RelayResponse
{
public:
    void set_id(RelayIdValue value) { id = std::move(value); }
    [[nodiscard]] const RelayIdValue& get_id() const noexcept { return id; }
    void set_status(RelayStatusValue value) { status = std::move(value); }
    [[nodiscard]] const RelayStatusValue& get_status() const noexcept { return status; }
    void set_connector_resource_id(std::string value) { connector_resource_id = std::move(value); }
    [[nodiscard]] const std::string& get_connector_resource_id() const noexcept { return connector_resource_id; }
    void set_error(std::optional<RelayError> value) { error = std::move(value); }
    [[nodiscard]] const std::optional<RelayError>& get_error() const noexcept { return error; }
    void set_connector_reference_id(std::optional<std::string> value) { connector_reference_id = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_connector_reference_id() const noexcept { return connector_reference_id; }
    void set_connector_id(MerchantConnectorAccountIdValue value) { connector_id = std::move(value); }
    [[nodiscard]] const MerchantConnectorAccountIdValue& get_connector_id() const noexcept { return connector_id; }
    void set_profile_id(ProfileIdValue value) { profile_id = std::move(value); }
    [[nodiscard]] const ProfileIdValue& get_profile_id() const noexcept { return profile_id; }
    void set_relay_type(RelayTypeValue value) { relay_type = std::move(value); }
    [[nodiscard]] const RelayTypeValue& get_relay_type() const noexcept { return relay_type; }
    void set_data(std::optional<RelayData> value) { data = std::move(value); }
    [[nodiscard]] const std::optional<RelayData>& get_data() const noexcept { return data; }
    RelayIdValue id{};
    RelayStatusValue status{};
    std::string connector_resource_id{};
    std::optional<RelayError> error{};
    std::optional<std::string> connector_reference_id{};
    MerchantConnectorAccountIdValue connector_id{};
    ProfileIdValue profile_id{};
    RelayTypeValue relay_type{};
    std::optional<RelayData> data{};
};

class RelayRetrieveRequest
{
public:
    void set_force_sync(bool value) { force_sync = std::move(value); }
    [[nodiscard]] const bool& get_force_sync() const noexcept { return force_sync; }
    void set_id(RelayIdValue value) { id = std::move(value); }
    [[nodiscard]] const RelayIdValue& get_id() const noexcept { return id; }
    bool force_sync{};
    RelayIdValue id{};
};

class RelayUnreferencedRefundData
{
public:
    void set_amount(MinorUnitValue value) { amount = std::move(value); }
    [[nodiscard]] const MinorUnitValue& get_amount() const noexcept { return amount; }
    void set_currency(CurrencyValue value) { currency = std::move(value); }
    [[nodiscard]] const CurrencyValue& get_currency() const noexcept { return currency; }
    void set_customer_id(std::optional<std::string> value) { customer_id = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_customer_id() const noexcept { return customer_id; }
    void set_recipient_payment_method_data(std::optional<RecipientPaymentMethodDataValue> value) { recipient_payment_method_data = std::move(value); }
    [[nodiscard]] const std::optional<RecipientPaymentMethodDataValue>& get_recipient_payment_method_data() const noexcept { return recipient_payment_method_data; }
    MinorUnitValue amount{};
    CurrencyValue currency{};
    std::optional<std::string> customer_id{};
    std::optional<RecipientPaymentMethodDataValue> recipient_payment_method_data{};
};

class RelayVoidRequestData
{
public:
    void set_amount(std::optional<MinorUnitValue> value) { amount = std::move(value); }
    [[nodiscard]] const std::optional<MinorUnitValue>& get_amount() const noexcept { return amount; }
    void set_currency(std::optional<CurrencyValue> value) { currency = std::move(value); }
    [[nodiscard]] const std::optional<CurrencyValue>& get_currency() const noexcept { return currency; }
    void set_cancellation_reason(std::optional<std::string> value) { cancellation_reason = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_cancellation_reason() const noexcept { return cancellation_reason; }
    std::optional<MinorUnitValue> amount{};
    std::optional<CurrencyValue> currency{};
    std::optional<std::string> cancellation_reason{};
};
} // namespace app::dto::relay

template<>
struct serde::Serializable<app::dto::relay::RelayRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"connector_resource_id", &app::dto::relay::RelayRequest::get_connector_resource_id, &app::dto::relay::RelayRequest::set_connector_resource_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"connector_id", &app::dto::relay::RelayRequest::get_connector_id, &app::dto::relay::RelayRequest::set_connector_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"type", &app::dto::relay::RelayRequest::get_relay_type, &app::dto::relay::RelayRequest::set_relay_type, serde::FieldOptions{}>{},
            serde::FieldDesc<"data", &app::dto::relay::RelayRequest::get_data, &app::dto::relay::RelayRequest::set_data, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "relay_request"; }
};

template<>
struct serde::Serializable<app::dto::relay::RelayUnreferencedRefundData>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"amount", &app::dto::relay::RelayUnreferencedRefundData::get_amount, &app::dto::relay::RelayUnreferencedRefundData::set_amount, serde::FieldOptions{}>{},
            serde::FieldDesc<"currency", &app::dto::relay::RelayUnreferencedRefundData::get_currency, &app::dto::relay::RelayUnreferencedRefundData::set_currency, serde::FieldOptions{}>{},
            serde::FieldDesc<"customer_id", &app::dto::relay::RelayUnreferencedRefundData::get_customer_id, &app::dto::relay::RelayUnreferencedRefundData::set_customer_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"recipient_payment_method_data", &app::dto::relay::RelayUnreferencedRefundData::get_recipient_payment_method_data, &app::dto::relay::RelayUnreferencedRefundData::set_recipient_payment_method_data, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "relay_unreferenced_refund_data"; }
};

template<>
struct serde::Serializable<app::dto::relay::RelayRefundRequestData>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"amount", &app::dto::relay::RelayRefundRequestData::get_amount, &app::dto::relay::RelayRefundRequestData::set_amount, serde::FieldOptions{}>{},
            serde::FieldDesc<"currency", &app::dto::relay::RelayRefundRequestData::get_currency, &app::dto::relay::RelayRefundRequestData::set_currency, serde::FieldOptions{}>{},
            serde::FieldDesc<"reason", &app::dto::relay::RelayRefundRequestData::get_reason, &app::dto::relay::RelayRefundRequestData::set_reason, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "relay_refund_request_data"; }
};

template<>
struct serde::Serializable<app::dto::relay::RelayCaptureRequestData>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"authorized_amount", &app::dto::relay::RelayCaptureRequestData::get_authorized_amount, &app::dto::relay::RelayCaptureRequestData::set_authorized_amount, serde::FieldOptions{}>{},
            serde::FieldDesc<"amount_to_capture", &app::dto::relay::RelayCaptureRequestData::get_amount_to_capture, &app::dto::relay::RelayCaptureRequestData::set_amount_to_capture, serde::FieldOptions{}>{},
            serde::FieldDesc<"currency", &app::dto::relay::RelayCaptureRequestData::get_currency, &app::dto::relay::RelayCaptureRequestData::set_currency, serde::FieldOptions{}>{},
            serde::FieldDesc<"capture_method", &app::dto::relay::RelayCaptureRequestData::get_capture_method, &app::dto::relay::RelayCaptureRequestData::set_capture_method, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "relay_capture_request_data"; }
};

template<>
struct serde::Serializable<app::dto::relay::RelayIncrementalAuthorizationRequestData>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"total_amount", &app::dto::relay::RelayIncrementalAuthorizationRequestData::get_total_amount, &app::dto::relay::RelayIncrementalAuthorizationRequestData::set_total_amount, serde::FieldOptions{}>{},
            serde::FieldDesc<"additional_amount", &app::dto::relay::RelayIncrementalAuthorizationRequestData::get_additional_amount, &app::dto::relay::RelayIncrementalAuthorizationRequestData::set_additional_amount, serde::FieldOptions{}>{},
            serde::FieldDesc<"currency", &app::dto::relay::RelayIncrementalAuthorizationRequestData::get_currency, &app::dto::relay::RelayIncrementalAuthorizationRequestData::set_currency, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "relay_incremental_authorization_request_data"; }
};

template<>
struct serde::Serializable<app::dto::relay::RelayVoidRequestData>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"amount", &app::dto::relay::RelayVoidRequestData::get_amount, &app::dto::relay::RelayVoidRequestData::set_amount, serde::FieldOptions{}>{},
            serde::FieldDesc<"currency", &app::dto::relay::RelayVoidRequestData::get_currency, &app::dto::relay::RelayVoidRequestData::set_currency, serde::FieldOptions{}>{},
            serde::FieldDesc<"cancellation_reason", &app::dto::relay::RelayVoidRequestData::get_cancellation_reason, &app::dto::relay::RelayVoidRequestData::set_cancellation_reason, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "relay_void_request_data"; }
};

template<>
struct serde::Serializable<app::dto::relay::RelayError>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"code", &app::dto::relay::RelayError::get_code, &app::dto::relay::RelayError::set_code, serde::FieldOptions{}>{},
            serde::FieldDesc<"message", &app::dto::relay::RelayError::get_message, &app::dto::relay::RelayError::set_message, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "relay_error"; }
};

template<>
struct serde::Serializable<app::dto::relay::RelayResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"id", &app::dto::relay::RelayResponse::get_id, &app::dto::relay::RelayResponse::set_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"status", &app::dto::relay::RelayResponse::get_status, &app::dto::relay::RelayResponse::set_status, serde::FieldOptions{}>{},
            serde::FieldDesc<"connector_resource_id", &app::dto::relay::RelayResponse::get_connector_resource_id, &app::dto::relay::RelayResponse::set_connector_resource_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"error", &app::dto::relay::RelayResponse::get_error, &app::dto::relay::RelayResponse::set_error, serde::FieldOptions{}>{},
            serde::FieldDesc<"connector_reference_id", &app::dto::relay::RelayResponse::get_connector_reference_id, &app::dto::relay::RelayResponse::set_connector_reference_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"connector_id", &app::dto::relay::RelayResponse::get_connector_id, &app::dto::relay::RelayResponse::set_connector_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"profile_id", &app::dto::relay::RelayResponse::get_profile_id, &app::dto::relay::RelayResponse::set_profile_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"type", &app::dto::relay::RelayResponse::get_relay_type, &app::dto::relay::RelayResponse::set_relay_type, serde::FieldOptions{}>{},
            serde::FieldDesc<"data", &app::dto::relay::RelayResponse::get_data, &app::dto::relay::RelayResponse::set_data, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "relay_response"; }
};

template<>
struct serde::Serializable<app::dto::relay::RelayRetrieveRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"force_sync", &app::dto::relay::RelayRetrieveRequest::get_force_sync, &app::dto::relay::RelayRetrieveRequest::set_force_sync, serde::FieldOptions{}>{},
            serde::FieldDesc<"id", &app::dto::relay::RelayRetrieveRequest::get_id, &app::dto::relay::RelayRetrieveRequest::set_id, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "relay_retrieve_request"; }
};

template<>
struct serde::Serializable<app::dto::relay::RelayRetrieveBody>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"force_sync", &app::dto::relay::RelayRetrieveBody::get_force_sync, &app::dto::relay::RelayRetrieveBody::set_force_sync, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "relay_retrieve_body"; }
};

#ifdef CONGELADO_TEST
namespace app::dto::relay::tests {
using namespace boost::ut;
suite<"app_dto_relay"> app_dto_relay_suite = [] {
    "reflects converted types"_test = [] {
        expect(std::tuple_size_v<decltype(serde::Serializable<app::dto::relay::RelayRequest>::fields())> > 0);
    };
};
}
#endif
