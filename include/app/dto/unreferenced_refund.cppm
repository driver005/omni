module;
export module app_dto:unreferenced_refund;

import std;
import core_serde;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace app::dto::unreferenced_refund {

class CardNumberValue
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

enum class RecipientPaymentMethodData
{
    Card
};

class RelayErrorValue
{
public:
    void set_value(std::string value) { this->value = std::move(value); }
    [[nodiscard]] const std::string& get_value() const noexcept { return value; }
    std::string value{};
};

class RelayIdValue
{
public:
    void set_value(std::string value) { this->value = std::move(value); }
    [[nodiscard]] const std::string& get_value() const noexcept { return value; }
    std::string value{};
};

class RelayStatusValue
{
public:
    void set_value(std::string value) { this->value = std::move(value); }
    [[nodiscard]] const std::string& get_value() const noexcept { return value; }
    std::string value{};
};

class RecipientCardData
{
public:
    void set_card_number(CardNumberValue value) { card_number = std::move(value); }
    [[nodiscard]] const CardNumberValue& get_card_number() const noexcept { return card_number; }
    void set_card_exp_month(std::string value) { card_exp_month = std::move(value); }
    [[nodiscard]] const std::string& get_card_exp_month() const noexcept { return card_exp_month; }
    void set_card_exp_year(std::string value) { card_exp_year = std::move(value); }
    [[nodiscard]] const std::string& get_card_exp_year() const noexcept { return card_exp_year; }
    void set_card_holder_name(std::optional<std::string> value) { card_holder_name = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_card_holder_name() const noexcept { return card_holder_name; }
    CardNumberValue card_number{};
    std::string card_exp_month{};
    std::string card_exp_year{};
    std::optional<std::string> card_holder_name{};
};

class UnreferencedRefundRequest
{
public:
    void set_amount(MinorUnitValue value) { amount = std::move(value); }
    [[nodiscard]] const MinorUnitValue& get_amount() const noexcept { return amount; }
    void set_currency(CurrencyValue value) { currency = std::move(value); }
    [[nodiscard]] const CurrencyValue& get_currency() const noexcept { return currency; }
    void set_connector_id(MerchantConnectorAccountIdValue value) { connector_id = std::move(value); }
    [[nodiscard]] const MerchantConnectorAccountIdValue& get_connector_id() const noexcept { return connector_id; }
    void set_customer_id(std::optional<std::string> value) { customer_id = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_customer_id() const noexcept { return customer_id; }
    void set_connector_resource_id(std::optional<std::string> value) { connector_resource_id = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_connector_resource_id() const noexcept { return connector_resource_id; }
    void set_recipient_payment_method_data(RecipientPaymentMethodData value) { recipient_payment_method_data = std::move(value); }
    [[nodiscard]] const RecipientPaymentMethodData& get_recipient_payment_method_data() const noexcept { return recipient_payment_method_data; }
    MinorUnitValue amount{};
    CurrencyValue currency{};
    MerchantConnectorAccountIdValue connector_id{};
    std::optional<std::string> customer_id{};
    std::optional<std::string> connector_resource_id{};
    RecipientPaymentMethodData recipient_payment_method_data{};
};

class UnreferencedRefundResponse
{
public:
    void set_id(RelayIdValue value) { id = std::move(value); }
    [[nodiscard]] const RelayIdValue& get_id() const noexcept { return id; }
    void set_status(RelayStatusValue value) { status = std::move(value); }
    [[nodiscard]] const RelayStatusValue& get_status() const noexcept { return status; }
    void set_connector(std::string value) { connector = std::move(value); }
    [[nodiscard]] const std::string& get_connector() const noexcept { return connector; }
    void set_connector_id(MerchantConnectorAccountIdValue value) { connector_id = std::move(value); }
    [[nodiscard]] const MerchantConnectorAccountIdValue& get_connector_id() const noexcept { return connector_id; }
    void set_connector_resource_id(std::string value) { connector_resource_id = std::move(value); }
    [[nodiscard]] const std::string& get_connector_resource_id() const noexcept { return connector_resource_id; }
    void set_connector_reference_id(std::optional<std::string> value) { connector_reference_id = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_connector_reference_id() const noexcept { return connector_reference_id; }
    void set_profile_id(ProfileIdValue value) { profile_id = std::move(value); }
    [[nodiscard]] const ProfileIdValue& get_profile_id() const noexcept { return profile_id; }
    void set_error(std::optional<RelayErrorValue> value) { error = std::move(value); }
    [[nodiscard]] const std::optional<RelayErrorValue>& get_error() const noexcept { return error; }
    void set_raw_connector_response(std::optional<std::string> value) { raw_connector_response = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_raw_connector_response() const noexcept { return raw_connector_response; }
    RelayIdValue id{};
    RelayStatusValue status{};
    std::string connector{};
    MerchantConnectorAccountIdValue connector_id{};
    std::string connector_resource_id{};
    std::optional<std::string> connector_reference_id{};
    ProfileIdValue profile_id{};
    std::optional<RelayErrorValue> error{};
    std::optional<std::string> raw_connector_response{};
};
} // namespace app::dto::unreferenced_refund

template<>
struct serde::Serializable<app::dto::unreferenced_refund::UnreferencedRefundRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"amount", &app::dto::unreferenced_refund::UnreferencedRefundRequest::get_amount, &app::dto::unreferenced_refund::UnreferencedRefundRequest::set_amount, serde::FieldOptions{}>{},
            serde::FieldDesc<"currency", &app::dto::unreferenced_refund::UnreferencedRefundRequest::get_currency, &app::dto::unreferenced_refund::UnreferencedRefundRequest::set_currency, serde::FieldOptions{}>{},
            serde::FieldDesc<"connector_id", &app::dto::unreferenced_refund::UnreferencedRefundRequest::get_connector_id, &app::dto::unreferenced_refund::UnreferencedRefundRequest::set_connector_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"customer_id", &app::dto::unreferenced_refund::UnreferencedRefundRequest::get_customer_id, &app::dto::unreferenced_refund::UnreferencedRefundRequest::set_customer_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"connector_resource_id", &app::dto::unreferenced_refund::UnreferencedRefundRequest::get_connector_resource_id, &app::dto::unreferenced_refund::UnreferencedRefundRequest::set_connector_resource_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"recipient_payment_method_data", &app::dto::unreferenced_refund::UnreferencedRefundRequest::get_recipient_payment_method_data, &app::dto::unreferenced_refund::UnreferencedRefundRequest::set_recipient_payment_method_data, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "unreferenced_refund_request"; }
};

template<>
struct serde::Serializable<app::dto::unreferenced_refund::RecipientCardData>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"card_number", &app::dto::unreferenced_refund::RecipientCardData::get_card_number, &app::dto::unreferenced_refund::RecipientCardData::set_card_number, serde::FieldOptions{}>{},
            serde::FieldDesc<"card_exp_month", &app::dto::unreferenced_refund::RecipientCardData::get_card_exp_month, &app::dto::unreferenced_refund::RecipientCardData::set_card_exp_month, serde::FieldOptions{}>{},
            serde::FieldDesc<"card_exp_year", &app::dto::unreferenced_refund::RecipientCardData::get_card_exp_year, &app::dto::unreferenced_refund::RecipientCardData::set_card_exp_year, serde::FieldOptions{}>{},
            serde::FieldDesc<"card_holder_name", &app::dto::unreferenced_refund::RecipientCardData::get_card_holder_name, &app::dto::unreferenced_refund::RecipientCardData::set_card_holder_name, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "recipient_card_data"; }
};

template<>
struct serde::Serializable<app::dto::unreferenced_refund::UnreferencedRefundResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"id", &app::dto::unreferenced_refund::UnreferencedRefundResponse::get_id, &app::dto::unreferenced_refund::UnreferencedRefundResponse::set_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"status", &app::dto::unreferenced_refund::UnreferencedRefundResponse::get_status, &app::dto::unreferenced_refund::UnreferencedRefundResponse::set_status, serde::FieldOptions{}>{},
            serde::FieldDesc<"connector", &app::dto::unreferenced_refund::UnreferencedRefundResponse::get_connector, &app::dto::unreferenced_refund::UnreferencedRefundResponse::set_connector, serde::FieldOptions{}>{},
            serde::FieldDesc<"connector_id", &app::dto::unreferenced_refund::UnreferencedRefundResponse::get_connector_id, &app::dto::unreferenced_refund::UnreferencedRefundResponse::set_connector_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"connector_resource_id", &app::dto::unreferenced_refund::UnreferencedRefundResponse::get_connector_resource_id, &app::dto::unreferenced_refund::UnreferencedRefundResponse::set_connector_resource_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"connector_reference_id", &app::dto::unreferenced_refund::UnreferencedRefundResponse::get_connector_reference_id, &app::dto::unreferenced_refund::UnreferencedRefundResponse::set_connector_reference_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"profile_id", &app::dto::unreferenced_refund::UnreferencedRefundResponse::get_profile_id, &app::dto::unreferenced_refund::UnreferencedRefundResponse::set_profile_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"error", &app::dto::unreferenced_refund::UnreferencedRefundResponse::get_error, &app::dto::unreferenced_refund::UnreferencedRefundResponse::set_error, serde::FieldOptions{}>{},
            serde::FieldDesc<"raw_connector_response", &app::dto::unreferenced_refund::UnreferencedRefundResponse::get_raw_connector_response, &app::dto::unreferenced_refund::UnreferencedRefundResponse::set_raw_connector_response, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "unreferenced_refund_response"; }
};

#ifdef CONGELADO_TEST
namespace app::dto::unreferenced_refund::tests {
using namespace boost::ut;
suite<"app_dto_unreferenced_refund"> app_dto_unreferenced_refund_suite = [] {
    "reflects converted types"_test = [] {
        expect(std::tuple_size_v<decltype(serde::Serializable<app::dto::unreferenced_refund::UnreferencedRefundRequest>::fields())> > 0);
    };
};
}
#endif
