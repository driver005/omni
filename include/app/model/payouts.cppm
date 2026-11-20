module;
export module app_model:payouts;

import std;
import core_serde;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace app::model::payouts {

class AddressValue
{
public:
    void set_value(std::string value) { this->value = std::move(value); }
    [[nodiscard]] const std::string& get_value() const noexcept { return value; }
    std::string value{};
};

class AmountValue
{
public:
    void set_value(std::string value) { this->value = std::move(value); }
    [[nodiscard]] const std::string& get_value() const noexcept { return value; }
    std::string value{};
};

class BankAdditionalDataValue
{
public:
    void set_value(std::string value) { this->value = std::move(value); }
    [[nodiscard]] const std::string& get_value() const noexcept { return value; }
    std::string value{};
};

class BankNamesValue
{
public:
    void set_value(std::string value) { this->value = std::move(value); }
    [[nodiscard]] const std::string& get_value() const noexcept { return value; }
    std::string value{};
};

class BankTypeValue
{
public:
    void set_value(std::string value) { this->value = std::move(value); }
    [[nodiscard]] const std::string& get_value() const noexcept { return value; }
    std::string value{};
};

class BrowserInformationValue
{
public:
    void set_value(std::string value) { this->value = std::move(value); }
    [[nodiscard]] const std::string& get_value() const noexcept { return value; }
    std::string value{};
};

class CardNetworkValue
{
public:
    void set_value(std::string value) { this->value = std::move(value); }
    [[nodiscard]] const std::string& get_value() const noexcept { return value; }
    std::string value{};
};

class CardNumberValue
{
public:
    void set_value(std::string value) { this->value = std::move(value); }
    [[nodiscard]] const std::string& get_value() const noexcept { return value; }
    std::string value{};
};

class CountryAlpha2Value
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

class CustomerDetailsResponseValue
{
public:
    void set_value(std::string value) { this->value = std::move(value); }
    [[nodiscard]] const std::string& get_value() const noexcept { return value; }
    std::string value{};
};

class CustomerDetailsValue
{
public:
    void set_value(std::string value) { this->value = std::move(value); }
    [[nodiscard]] const std::string& get_value() const noexcept { return value; }
    std::string value{};
};

class CustomerIdValue
{
public:
    void set_value(std::string value) { this->value = std::move(value); }
    [[nodiscard]] const std::string& get_value() const noexcept { return value; }
    std::string value{};
};

class EmailValue
{
public:
    void set_value(std::string value) { this->value = std::move(value); }
    [[nodiscard]] const std::string& get_value() const noexcept { return value; }
    std::string value{};
};

class EnabledPaymentMethodValue
{
public:
    void set_value(std::string value) { this->value = std::move(value); }
    [[nodiscard]] const std::string& get_value() const noexcept { return value; }
    std::string value{};
};

class FrmMessageValue
{
public:
    void set_value(std::string value) { this->value = std::move(value); }
    [[nodiscard]] const std::string& get_value() const noexcept { return value; }
    std::string value{};
};

class GenericLinkUiConfigFormDataValue
{
public:
    void set_value(std::string value) { this->value = std::move(value); }
    [[nodiscard]] const std::string& get_value() const noexcept { return value; }
    std::string value{};
};

class GenericLinkUiConfigValue
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

class MerchantConnectorInfoValue
{
public:
    void set_value(std::string value) { this->value = std::move(value); }
    [[nodiscard]] const std::string& get_value() const noexcept { return value; }
    std::string value{};
};

/**
 * @brief Wire mirror of upstream `admin::MerchantConnectorInfo`
 * (`api_models/src/admin.rs:1626-1633`). Used as the value type of
 * `PayoutListFiltersV2.connector` so `GET /payouts/v2/{profile/}filter` emits the exact
 * upstream JSON shape: `{connector_label, merchant_connector_id, connector_type}`.
 */
class MerchantConnectorInfo
{
public:
    void set_connector_label(std::string value) { connector_label = std::move(value); }
    [[nodiscard]] const std::string& get_connector_label() const noexcept { return connector_label; }
    void set_merchant_connector_id(std::string value) { merchant_connector_id = std::move(value); }
    [[nodiscard]] const std::string& get_merchant_connector_id() const noexcept { return merchant_connector_id; }
    void set_connector_type(std::string value) { connector_type = std::move(value); }
    [[nodiscard]] const std::string& get_connector_type() const noexcept { return connector_type; }
    std::string connector_label{};
    std::string merchant_connector_id{};
    std::string connector_type{};
};

class MerchantIdValue
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

class OptionalEncryptableEmailValue
{
public:
    void set_value(std::string value) { this->value = std::move(value); }
    [[nodiscard]] const std::string& get_value() const noexcept { return value; }
    std::string value{};
};

class OptionalEncryptableNameValue
{
public:
    void set_value(std::string value) { this->value = std::move(value); }
    [[nodiscard]] const std::string& get_value() const noexcept { return value; }
    std::string value{};
};

class OptionalEncryptablePhoneValue
{
public:
    void set_value(std::string value) { this->value = std::move(value); }
    [[nodiscard]] const std::string& get_value() const noexcept { return value; }
    std::string value{};
};

class PageOffsetValue
{
public:
    void set_value(std::string value) { this->value = std::move(value); }
    [[nodiscard]] const std::string& get_value() const noexcept { return value; }
    std::string value{};
};

class PageSizeValue
{
public:
    void set_value(std::string value) { this->value = std::move(value); }
    [[nodiscard]] const std::string& get_value() const noexcept { return value; }
    std::string value{};
};

class PaymentMethodTypeValue
{
public:
    void set_value(std::string value) { this->value = std::move(value); }
    [[nodiscard]] const std::string& get_value() const noexcept { return value; }
    std::string value{};
};

class PaymentMethodValue
{
public:
    void set_value(std::string value) { this->value = std::move(value); }
    [[nodiscard]] const std::string& get_value() const noexcept { return value; }
    std::string value{};
};

class PayoutConnectorsValue
{
public:
    void set_value(std::string value) { this->value = std::move(value); }
    [[nodiscard]] const std::string& get_value() const noexcept { return value; }
    std::string value{};
};

class PayoutEntityTypeValue
{
public:
    void set_value(std::string value) { this->value = std::move(value); }
    [[nodiscard]] const std::string& get_value() const noexcept { return value; }
    std::string value{};
};

class PayoutFundsTransferTypeValue
{
public:
    void set_value(std::string value) { this->value = std::move(value); }
    [[nodiscard]] const std::string& get_value() const noexcept { return value; }
    std::string value{};
};

class PayoutIdValue
{
public:
    void set_value(std::string value) { this->value = std::move(value); }
    [[nodiscard]] const std::string& get_value() const noexcept { return value; }
    std::string value{};
};

class PayoutPurposeValue
{
public:
    void set_value(std::string value) { this->value = std::move(value); }
    [[nodiscard]] const std::string& get_value() const noexcept { return value; }
    std::string value{};
};

class PayoutSendPriorityValue
{
public:
    void set_value(std::string value) { this->value = std::move(value); }
    [[nodiscard]] const std::string& get_value() const noexcept { return value; }
    std::string value{};
};

class PayoutStatusValue
{
public:
    void set_value(std::string value) { this->value = std::move(value); }
    [[nodiscard]] const std::string& get_value() const noexcept { return value; }
    std::string value{};
};

class PayoutTypeValue
{
public:
    void set_value(std::string value) { this->value = std::move(value); }
    [[nodiscard]] const std::string& get_value() const noexcept { return value; }
    std::string value{};
};

class PayoutsAggregateResponse
{
public:
    void set_status_with_count(std::unordered_map<std::string, std::int64_t> value) { status_with_count = std::move(value); }
    [[nodiscard]] const std::unordered_map<std::string, std::int64_t>& get_status_with_count() const noexcept { return status_with_count; }
    std::unordered_map<std::string, std::int64_t> status_with_count{};
};

class PayoutsBillingDescriptorValue
{
public:
    void set_value(std::string value) { this->value = std::move(value); }
    [[nodiscard]] const std::string& get_value() const noexcept { return value; }
    std::string value{};
};

class PrimitiveDateTimeValue
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

class RequiredFieldInfoValue
{
public:
    void set_value(std::string value) { this->value = std::move(value); }
    [[nodiscard]] const std::string& get_value() const noexcept { return value; }
    std::string value{};
};

class SecretSerdeValueValue
{
public:
    void set_value(std::string value) { this->value = std::move(value); }
    [[nodiscard]] const std::string& get_value() const noexcept { return value; }
    std::string value{};
};

class SecretValue
{
public:
    void set_value(std::string value) { this->value = std::move(value); }
    [[nodiscard]] const std::string& get_value() const noexcept { return value; }
    std::string value{};
};

class SenderDetailsValue
{
public:
    void set_value(std::string value) { this->value = std::move(value); }
    [[nodiscard]] const std::string& get_value() const noexcept { return value; }
    std::string value{};
};

class SourceOfFundsValue
{
public:
    void set_value(std::string value) { this->value = std::move(value); }
    [[nodiscard]] const std::string& get_value() const noexcept { return value; }
    std::string value{};
};

class StringMajorUnitValue
{
public:
    void set_value(std::string value) { this->value = std::move(value); }
    [[nodiscard]] const std::string& get_value() const noexcept { return value; }
    std::string value{};
};

class TimeRangeValue
{
public:
    void set_value(std::string value) { this->value = std::move(value); }
    [[nodiscard]] const std::string& get_value() const noexcept { return value; }
    std::string value{};
};

class UIWidgetFormLayoutValue
{
public:
    void set_value(std::string value) { this->value = std::move(value); }
    [[nodiscard]] const std::string& get_value() const noexcept { return value; }
    std::string value{};
};

class UnifiedCodeValue
{
public:
    void set_value(std::string value) { this->value = std::move(value); }
    [[nodiscard]] const std::string& get_value() const noexcept { return value; }
    std::string value{};
};

class UnifiedMessageValue
{
public:
    void set_value(std::string value) { this->value = std::move(value); }
    [[nodiscard]] const std::string& get_value() const noexcept { return value; }
    std::string value{};
};

class UrlValue
{
public:
    void set_value(std::string value) { this->value = std::move(value); }
    [[nodiscard]] const std::string& get_value() const noexcept { return value; }
    std::string value{};
};

class AchBankTransfer
{
public:
    void set_bank_name(std::optional<std::string> value) { bank_name = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_bank_name() const noexcept { return bank_name; }
    void set_bank_country_code(std::optional<CountryAlpha2Value> value) { bank_country_code = std::move(value); }
    [[nodiscard]] const std::optional<CountryAlpha2Value>& get_bank_country_code() const noexcept { return bank_country_code; }
    void set_bank_city(std::optional<std::string> value) { bank_city = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_bank_city() const noexcept { return bank_city; }
    void set_bank_account_number(SecretValue value) { bank_account_number = std::move(value); }
    [[nodiscard]] const SecretValue& get_bank_account_number() const noexcept { return bank_account_number; }
    void set_bank_routing_number(SecretValue value) { bank_routing_number = std::move(value); }
    [[nodiscard]] const SecretValue& get_bank_routing_number() const noexcept { return bank_routing_number; }
    void set_account_holder_name(std::optional<SecretValue> value) { account_holder_name = std::move(value); }
    [[nodiscard]] const std::optional<SecretValue>& get_account_holder_name() const noexcept { return account_holder_name; }
    std::optional<std::string> bank_name{};
    std::optional<CountryAlpha2Value> bank_country_code{};
    std::optional<std::string> bank_city{};
    SecretValue bank_account_number{};
    SecretValue bank_routing_number{};
    std::optional<SecretValue> account_holder_name{};
};

class ApplePayDecrypt
{
public:
    void set_dpan(CardNumberValue value) { dpan = std::move(value); }
    [[nodiscard]] const CardNumberValue& get_dpan() const noexcept { return dpan; }
    void set_expiry_month(SecretValue value) { expiry_month = std::move(value); }
    [[nodiscard]] const SecretValue& get_expiry_month() const noexcept { return expiry_month; }
    void set_expiry_year(SecretValue value) { expiry_year = std::move(value); }
    [[nodiscard]] const SecretValue& get_expiry_year() const noexcept { return expiry_year; }
    void set_card_holder_name(std::optional<SecretValue> value) { card_holder_name = std::move(value); }
    [[nodiscard]] const std::optional<SecretValue>& get_card_holder_name() const noexcept { return card_holder_name; }
    void set_card_network(std::optional<CardNetworkValue> value) { card_network = std::move(value); }
    [[nodiscard]] const std::optional<CardNetworkValue>& get_card_network() const noexcept { return card_network; }
    CardNumberValue dpan{};
    SecretValue expiry_month{};
    SecretValue expiry_year{};
    std::optional<SecretValue> card_holder_name{};
    std::optional<CardNetworkValue> card_network{};
};

class BacsBankTransfer
{
public:
    void set_bank_name(std::optional<std::string> value) { bank_name = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_bank_name() const noexcept { return bank_name; }
    void set_bank_country_code(std::optional<CountryAlpha2Value> value) { bank_country_code = std::move(value); }
    [[nodiscard]] const std::optional<CountryAlpha2Value>& get_bank_country_code() const noexcept { return bank_country_code; }
    void set_bank_city(std::optional<std::string> value) { bank_city = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_bank_city() const noexcept { return bank_city; }
    void set_bank_account_number(SecretValue value) { bank_account_number = std::move(value); }
    [[nodiscard]] const SecretValue& get_bank_account_number() const noexcept { return bank_account_number; }
    void set_bank_sort_code(SecretValue value) { bank_sort_code = std::move(value); }
    [[nodiscard]] const SecretValue& get_bank_sort_code() const noexcept { return bank_sort_code; }
    void set_account_holder_name(std::optional<SecretValue> value) { account_holder_name = std::move(value); }
    [[nodiscard]] const std::optional<SecretValue>& get_account_holder_name() const noexcept { return account_holder_name; }
    std::optional<std::string> bank_name{};
    std::optional<CountryAlpha2Value> bank_country_code{};
    std::optional<std::string> bank_city{};
    SecretValue bank_account_number{};
    SecretValue bank_sort_code{};
    std::optional<SecretValue> account_holder_name{};
};

class CardPayout
{
public:
    void set_card_number(CardNumberValue value) { card_number = std::move(value); }
    [[nodiscard]] const CardNumberValue& get_card_number() const noexcept { return card_number; }
    void set_expiry_month(SecretValue value) { expiry_month = std::move(value); }
    [[nodiscard]] const SecretValue& get_expiry_month() const noexcept { return expiry_month; }
    void set_expiry_year(SecretValue value) { expiry_year = std::move(value); }
    [[nodiscard]] const SecretValue& get_expiry_year() const noexcept { return expiry_year; }
    void set_card_holder_name(std::optional<SecretValue> value) { card_holder_name = std::move(value); }
    [[nodiscard]] const std::optional<SecretValue>& get_card_holder_name() const noexcept { return card_holder_name; }
    void set_card_network(std::optional<CardNetworkValue> value) { card_network = std::move(value); }
    [[nodiscard]] const std::optional<CardNetworkValue>& get_card_network() const noexcept { return card_network; }
    CardNumberValue card_number{};
    SecretValue expiry_month{};
    SecretValue expiry_year{};
    std::optional<SecretValue> card_holder_name{};
    std::optional<CardNetworkValue> card_network{};
};

class GooglePayDecrypt
{
public:
    void set_application_primary_account_number(CardNumberValue value) { application_primary_account_number = std::move(value); }
    [[nodiscard]] const CardNumberValue& get_application_primary_account_number() const noexcept { return application_primary_account_number; }
    void set_expiry_month(SecretValue value) { expiry_month = std::move(value); }
    [[nodiscard]] const SecretValue& get_expiry_month() const noexcept { return expiry_month; }
    void set_expiry_year(SecretValue value) { expiry_year = std::move(value); }
    [[nodiscard]] const SecretValue& get_expiry_year() const noexcept { return expiry_year; }
    void set_card_holder_name(std::optional<SecretValue> value) { card_holder_name = std::move(value); }
    [[nodiscard]] const std::optional<SecretValue>& get_card_holder_name() const noexcept { return card_holder_name; }
    void set_card_network(std::optional<CardNetworkValue> value) { card_network = std::move(value); }
    [[nodiscard]] const std::optional<CardNetworkValue>& get_card_network() const noexcept { return card_network; }
    CardNumberValue application_primary_account_number{};
    SecretValue expiry_month{};
    SecretValue expiry_year{};
    std::optional<SecretValue> card_holder_name{};
    std::optional<CardNetworkValue> card_network{};
};

class Interac
{
public:
    void set_email(EmailValue value) { email = std::move(value); }
    [[nodiscard]] const EmailValue& get_email() const noexcept { return email; }
    EmailValue email{};
};

class Mifinity
{
public:
    void set_destination_account(SecretValue value) { destination_account = std::move(value); }
    [[nodiscard]] const SecretValue& get_destination_account() const noexcept { return destination_account; }
    SecretValue destination_account{};
};

class OpenBanking
{
public:
    void set_account_holder_name(SecretValue value) { account_holder_name = std::move(value); }
    [[nodiscard]] const SecretValue& get_account_holder_name() const noexcept { return account_holder_name; }
    void set_iban(SecretValue value) { iban = std::move(value); }
    [[nodiscard]] const SecretValue& get_iban() const noexcept { return iban; }
    SecretValue account_holder_name{};
    SecretValue iban{};
};

class OpenBankingUk
{
public:
    void set_account_holder_name(SecretValue value) { account_holder_name = std::move(value); }
    [[nodiscard]] const SecretValue& get_account_holder_name() const noexcept { return account_holder_name; }
    void set_iban(SecretValue value) { iban = std::move(value); }
    [[nodiscard]] const SecretValue& get_iban() const noexcept { return iban; }
    SecretValue account_holder_name{};
    SecretValue iban{};
};

class Passthrough
{
public:
    void set_psp_token(SecretValue value) { psp_token = std::move(value); }
    [[nodiscard]] const SecretValue& get_psp_token() const noexcept { return psp_token; }
    void set_psp_customer_id(std::optional<SecretValue> value) { psp_customer_id = std::move(value); }
    [[nodiscard]] const std::optional<SecretValue>& get_psp_customer_id() const noexcept { return psp_customer_id; }
    void set_token_type(PaymentMethodTypeValue value) { token_type = std::move(value); }
    [[nodiscard]] const PaymentMethodTypeValue& get_token_type() const noexcept { return token_type; }
    SecretValue psp_token{};
    std::optional<SecretValue> psp_customer_id{};
    PaymentMethodTypeValue token_type{};
};

class PaymentMethodTypeInfo
{
public:
    void set_payment_method_type(PaymentMethodTypeValue value) { payment_method_type = std::move(value); }
    [[nodiscard]] const PaymentMethodTypeValue& get_payment_method_type() const noexcept { return payment_method_type; }
    void set_required_fields(std::optional<std::unordered_map<std::string, RequiredFieldInfoValue>> value) { required_fields = std::move(value); }
    [[nodiscard]] const std::optional<std::unordered_map<std::string, RequiredFieldInfoValue>>& get_required_fields() const noexcept { return required_fields; }
    PaymentMethodTypeValue payment_method_type{};
    std::optional<std::unordered_map<std::string, RequiredFieldInfoValue>> required_fields{};
};

class PayoutActionRequest
{
public:
    void set_payout_id(PayoutIdValue value) { payout_id = std::move(value); }
    [[nodiscard]] const PayoutIdValue& get_payout_id() const noexcept { return payout_id; }
    PayoutIdValue payout_id{};
};

class PayoutAttemptResponse
{
public:
    void set_attempt_id(std::string value) { attempt_id = std::move(value); }
    [[nodiscard]] const std::string& get_attempt_id() const noexcept { return attempt_id; }
    void set_status(PayoutStatusValue value) { status = std::move(value); }
    [[nodiscard]] const PayoutStatusValue& get_status() const noexcept { return status; }
    void set_amount(MinorUnitValue value) { amount = std::move(value); }
    [[nodiscard]] const MinorUnitValue& get_amount() const noexcept { return amount; }
    void set_currency(std::optional<CurrencyValue> value) { currency = std::move(value); }
    [[nodiscard]] const std::optional<CurrencyValue>& get_currency() const noexcept { return currency; }
    void set_connector(std::optional<std::string> value) { connector = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_connector() const noexcept { return connector; }
    void set_error_code(std::optional<std::string> value) { error_code = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_error_code() const noexcept { return error_code; }
    void set_error_message(std::optional<std::string> value) { error_message = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_error_message() const noexcept { return error_message; }
    void set_payment_method(std::optional<PayoutTypeValue> value) { payment_method = std::move(value); }
    [[nodiscard]] const std::optional<PayoutTypeValue>& get_payment_method() const noexcept { return payment_method; }
    void set_payout_method_type(std::optional<PaymentMethodTypeValue> value) { payout_method_type = std::move(value); }
    [[nodiscard]] const std::optional<PaymentMethodTypeValue>& get_payout_method_type() const noexcept { return payout_method_type; }
    void set_connector_transaction_id(std::optional<std::string> value) { connector_transaction_id = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_connector_transaction_id() const noexcept { return connector_transaction_id; }
    void set_cancellation_reason(std::optional<std::string> value) { cancellation_reason = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_cancellation_reason() const noexcept { return cancellation_reason; }
    void set_unified_code(std::optional<UnifiedCodeValue> value) { unified_code = std::move(value); }
    [[nodiscard]] const std::optional<UnifiedCodeValue>& get_unified_code() const noexcept { return unified_code; }
    void set_unified_message(std::optional<UnifiedMessageValue> value) { unified_message = std::move(value); }
    [[nodiscard]] const std::optional<UnifiedMessageValue>& get_unified_message() const noexcept { return unified_message; }
    std::string attempt_id{};
    PayoutStatusValue status{};
    MinorUnitValue amount{};
    std::optional<CurrencyValue> currency{};
    std::optional<std::string> connector{};
    std::optional<std::string> error_code{};
    std::optional<std::string> error_message{};
    std::optional<PayoutTypeValue> payment_method{};
    std::optional<PaymentMethodTypeValue> payout_method_type{};
    std::optional<std::string> connector_transaction_id{};
    std::optional<std::string> cancellation_reason{};
    std::optional<UnifiedCodeValue> unified_code{};
    std::optional<UnifiedMessageValue> unified_message{};
};

class PayoutCreatePayoutLinkConfig
{
public:
    void set_payout_link_id(std::optional<std::string> value) { payout_link_id = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_payout_link_id() const noexcept { return payout_link_id; }
    void set_ui_config(std::optional<GenericLinkUiConfigValue> value) { ui_config = std::move(value); }
    [[nodiscard]] const std::optional<GenericLinkUiConfigValue>& get_ui_config() const noexcept { return ui_config; }
    void set_enabled_payment_methods(std::optional<std::vector<EnabledPaymentMethodValue>> value) { enabled_payment_methods = std::move(value); }
    [[nodiscard]] const std::optional<std::vector<EnabledPaymentMethodValue>>& get_enabled_payment_methods() const noexcept { return enabled_payment_methods; }
    void set_form_layout(std::optional<UIWidgetFormLayoutValue> value) { form_layout = std::move(value); }
    [[nodiscard]] const std::optional<UIWidgetFormLayoutValue>& get_form_layout() const noexcept { return form_layout; }
    void set_test_mode(std::optional<bool> value) { test_mode = std::move(value); }
    [[nodiscard]] const std::optional<bool>& get_test_mode() const noexcept { return test_mode; }
    std::optional<std::string> payout_link_id{};
    std::optional<GenericLinkUiConfigValue> ui_config{};
    std::optional<std::vector<EnabledPaymentMethodValue>> enabled_payment_methods{};
    std::optional<UIWidgetFormLayoutValue> form_layout{};
    std::optional<bool> test_mode{};
};

class PayoutIndividualDetails
{
public:
    void set_tos_acceptance_date(std::optional<std::int64_t> value) { tos_acceptance_date = std::move(value); }
    [[nodiscard]] const std::optional<std::int64_t>& get_tos_acceptance_date() const noexcept { return tos_acceptance_date; }
    void set_tos_acceptance_ip(std::optional<SecretValue> value) { tos_acceptance_ip = std::move(value); }
    [[nodiscard]] const std::optional<SecretValue>& get_tos_acceptance_ip() const noexcept { return tos_acceptance_ip; }
    void set_individual_dob_day(std::optional<SecretValue> value) { individual_dob_day = std::move(value); }
    [[nodiscard]] const std::optional<SecretValue>& get_individual_dob_day() const noexcept { return individual_dob_day; }
    void set_individual_dob_month(std::optional<SecretValue> value) { individual_dob_month = std::move(value); }
    [[nodiscard]] const std::optional<SecretValue>& get_individual_dob_month() const noexcept { return individual_dob_month; }
    void set_individual_dob_year(std::optional<SecretValue> value) { individual_dob_year = std::move(value); }
    [[nodiscard]] const std::optional<SecretValue>& get_individual_dob_year() const noexcept { return individual_dob_year; }
    void set_date_of_birth(std::optional<SecretValue> value) { date_of_birth = std::move(value); }
    [[nodiscard]] const std::optional<SecretValue>& get_date_of_birth() const noexcept { return date_of_birth; }
    void set_individual_id_number(std::optional<SecretValue> value) { individual_id_number = std::move(value); }
    [[nodiscard]] const std::optional<SecretValue>& get_individual_id_number() const noexcept { return individual_id_number; }
    void set_individual_ssn_last_4(std::optional<SecretValue> value) { individual_ssn_last_4 = std::move(value); }
    [[nodiscard]] const std::optional<SecretValue>& get_individual_ssn_last_4() const noexcept { return individual_ssn_last_4; }
    void set_external_account_account_holder_type(std::optional<std::string> value) { external_account_account_holder_type = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_external_account_account_holder_type() const noexcept { return external_account_account_holder_type; }
    void set_first_name(std::optional<SecretValue> value) { first_name = std::move(value); }
    [[nodiscard]] const std::optional<SecretValue>& get_first_name() const noexcept { return first_name; }
    void set_last_name(std::optional<SecretValue> value) { last_name = std::move(value); }
    [[nodiscard]] const std::optional<SecretValue>& get_last_name() const noexcept { return last_name; }
    void set_address(std::optional<AddressValue> value) { address = std::move(value); }
    [[nodiscard]] const std::optional<AddressValue>& get_address() const noexcept { return address; }
    void set_reference_id(std::optional<SecretValue> value) { reference_id = std::move(value); }
    [[nodiscard]] const std::optional<SecretValue>& get_reference_id() const noexcept { return reference_id; }
    void set_source_of_funds(std::optional<SourceOfFundsValue> value) { source_of_funds = std::move(value); }
    [[nodiscard]] const std::optional<SourceOfFundsValue>& get_source_of_funds() const noexcept { return source_of_funds; }
    std::optional<std::int64_t> tos_acceptance_date{};
    std::optional<SecretValue> tos_acceptance_ip{};
    std::optional<SecretValue> individual_dob_day{};
    std::optional<SecretValue> individual_dob_month{};
    std::optional<SecretValue> individual_dob_year{};
    std::optional<SecretValue> date_of_birth{};
    std::optional<SecretValue> individual_id_number{};
    std::optional<SecretValue> individual_ssn_last_4{};
    std::optional<std::string> external_account_account_holder_type{};
    std::optional<SecretValue> first_name{};
    std::optional<SecretValue> last_name{};
    std::optional<AddressValue> address{};
    std::optional<SecretValue> reference_id{};
    std::optional<SourceOfFundsValue> source_of_funds{};
};

class PayoutLinkInitiateRequest
{
public:
    void set_merchant_id(MerchantIdValue value) { merchant_id = std::move(value); }
    [[nodiscard]] const MerchantIdValue& get_merchant_id() const noexcept { return merchant_id; }
    void set_payout_id(PayoutIdValue value) { payout_id = std::move(value); }
    [[nodiscard]] const PayoutIdValue& get_payout_id() const noexcept { return payout_id; }
    MerchantIdValue merchant_id{};
    PayoutIdValue payout_id{};
};

class PayoutLinkResponse
{
public:
    void set_payout_link_id(std::string value) { payout_link_id = std::move(value); }
    [[nodiscard]] const std::string& get_payout_link_id() const noexcept { return payout_link_id; }
    void set_link(SecretValue value) { link = std::move(value); }
    [[nodiscard]] const SecretValue& get_link() const noexcept { return link; }
    std::string payout_link_id{};
    SecretValue link{};
};

class PayoutLinkStatusDetails
{
public:
    void set_payout_link_id(std::string value) { payout_link_id = std::move(value); }
    [[nodiscard]] const std::string& get_payout_link_id() const noexcept { return payout_link_id; }
    void set_payout_id(PayoutIdValue value) { payout_id = std::move(value); }
    [[nodiscard]] const PayoutIdValue& get_payout_id() const noexcept { return payout_id; }
    void set_customer_id(CustomerIdValue value) { customer_id = std::move(value); }
    [[nodiscard]] const CustomerIdValue& get_customer_id() const noexcept { return customer_id; }
    void set_session_expiry(PrimitiveDateTimeValue value) { session_expiry = std::move(value); }
    [[nodiscard]] const PrimitiveDateTimeValue& get_session_expiry() const noexcept { return session_expiry; }
    void set_return_url(std::optional<UrlValue> value) { return_url = std::move(value); }
    [[nodiscard]] const std::optional<UrlValue>& get_return_url() const noexcept { return return_url; }
    void set_status(PayoutStatusValue value) { status = std::move(value); }
    [[nodiscard]] const PayoutStatusValue& get_status() const noexcept { return status; }
    void set_error_code(std::optional<UnifiedCodeValue> value) { error_code = std::move(value); }
    [[nodiscard]] const std::optional<UnifiedCodeValue>& get_error_code() const noexcept { return error_code; }
    void set_error_message(std::optional<UnifiedMessageValue> value) { error_message = std::move(value); }
    [[nodiscard]] const std::optional<UnifiedMessageValue>& get_error_message() const noexcept { return error_message; }
    void set_ui_config(GenericLinkUiConfigFormDataValue value) { ui_config = std::move(value); }
    [[nodiscard]] const GenericLinkUiConfigFormDataValue& get_ui_config() const noexcept { return ui_config; }
    void set_test_mode(bool value) { test_mode = std::move(value); }
    [[nodiscard]] const bool& get_test_mode() const noexcept { return test_mode; }
    std::string payout_link_id{};
    PayoutIdValue payout_id{};
    CustomerIdValue customer_id{};
    PrimitiveDateTimeValue session_expiry{};
    std::optional<UrlValue> return_url{};
    PayoutStatusValue status{};
    std::optional<UnifiedCodeValue> error_code{};
    std::optional<UnifiedMessageValue> error_message{};
    GenericLinkUiConfigFormDataValue ui_config{};
    bool test_mode{};
};

class PayoutListConstraints
{
public:
    void set_customer_id(std::optional<CustomerIdValue> value) { customer_id = std::move(value); }
    [[nodiscard]] const std::optional<CustomerIdValue>& get_customer_id() const noexcept { return customer_id; }
    void set_starting_after(std::optional<PayoutIdValue> value) { starting_after = std::move(value); }
    [[nodiscard]] const std::optional<PayoutIdValue>& get_starting_after() const noexcept { return starting_after; }
    void set_ending_before(std::optional<PayoutIdValue> value) { ending_before = std::move(value); }
    [[nodiscard]] const std::optional<PayoutIdValue>& get_ending_before() const noexcept { return ending_before; }
    void set_limit(PageSizeValue value) { limit = std::move(value); }
    [[nodiscard]] const PageSizeValue& get_limit() const noexcept { return limit; }
    void set_created(std::optional<PrimitiveDateTimeValue> value) { created = std::move(value); }
    [[nodiscard]] const std::optional<PrimitiveDateTimeValue>& get_created() const noexcept { return created; }
    void set_time_range(std::optional<TimeRangeValue> value) { time_range = std::move(value); }
    [[nodiscard]] const std::optional<TimeRangeValue>& get_time_range() const noexcept { return time_range; }
    std::optional<CustomerIdValue> customer_id{};
    std::optional<PayoutIdValue> starting_after{};
    std::optional<PayoutIdValue> ending_before{};
    PageSizeValue limit{};
    std::optional<PrimitiveDateTimeValue> created{};
    std::optional<TimeRangeValue> time_range{};
};

class PayoutListFilterConstraints
{
public:
    void set_payout_id(std::optional<PayoutIdValue> value) { payout_id = std::move(value); }
    [[nodiscard]] const std::optional<PayoutIdValue>& get_payout_id() const noexcept { return payout_id; }
    void set_merchant_order_reference_id(std::optional<std::string> value) { merchant_order_reference_id = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_merchant_order_reference_id() const noexcept { return merchant_order_reference_id; }
    void set_profile_id(std::optional<ProfileIdValue> value) { profile_id = std::move(value); }
    [[nodiscard]] const std::optional<ProfileIdValue>& get_profile_id() const noexcept { return profile_id; }
    void set_customer_id(std::optional<CustomerIdValue> value) { customer_id = std::move(value); }
    [[nodiscard]] const std::optional<CustomerIdValue>& get_customer_id() const noexcept { return customer_id; }
    void set_limit(PageSizeValue value) { limit = std::move(value); }
    [[nodiscard]] const PageSizeValue& get_limit() const noexcept { return limit; }
    void set_offset(PageOffsetValue value) { offset = std::move(value); }
    [[nodiscard]] const PageOffsetValue& get_offset() const noexcept { return offset; }
    void set_time_range(std::optional<TimeRangeValue> value) { time_range = std::move(value); }
    [[nodiscard]] const std::optional<TimeRangeValue>& get_time_range() const noexcept { return time_range; }
    void set_connector(std::optional<std::vector<PayoutConnectorsValue>> value) { connector = std::move(value); }
    [[nodiscard]] const std::optional<std::vector<PayoutConnectorsValue>>& get_connector() const noexcept { return connector; }
    void set_currency(std::optional<std::vector<CurrencyValue>> value) { currency = std::move(value); }
    [[nodiscard]] const std::optional<std::vector<CurrencyValue>>& get_currency() const noexcept { return currency; }
    void set_status(std::optional<std::vector<PayoutStatusValue>> value) { status = std::move(value); }
    [[nodiscard]] const std::optional<std::vector<PayoutStatusValue>>& get_status() const noexcept { return status; }
    void set_payout_method(std::optional<std::vector<PayoutTypeValue>> value) { payout_method = std::move(value); }
    [[nodiscard]] const std::optional<std::vector<PayoutTypeValue>>& get_payout_method() const noexcept { return payout_method; }
    void set_entity_type(std::optional<PayoutEntityTypeValue> value) { entity_type = std::move(value); }
    [[nodiscard]] const std::optional<PayoutEntityTypeValue>& get_entity_type() const noexcept { return entity_type; }
    std::optional<PayoutIdValue> payout_id{};
    std::optional<std::string> merchant_order_reference_id{};
    std::optional<ProfileIdValue> profile_id{};
    std::optional<CustomerIdValue> customer_id{};
    PageSizeValue limit{};
    PageOffsetValue offset{};
    std::optional<TimeRangeValue> time_range{};
    std::optional<std::vector<PayoutConnectorsValue>> connector{};
    std::optional<std::vector<CurrencyValue>> currency{};
    std::optional<std::vector<PayoutStatusValue>> status{};
    std::optional<std::vector<PayoutTypeValue>> payout_method{};
    std::optional<PayoutEntityTypeValue> entity_type{};
};

class PayoutListFilters
{
public:
    void set_connector(std::vector<PayoutConnectorsValue> value) { connector = std::move(value); }
    [[nodiscard]] const std::vector<PayoutConnectorsValue>& get_connector() const noexcept { return connector; }
    void set_currency(std::vector<CurrencyValue> value) { currency = std::move(value); }
    [[nodiscard]] const std::vector<CurrencyValue>& get_currency() const noexcept { return currency; }
    void set_status(std::vector<PayoutStatusValue> value) { status = std::move(value); }
    [[nodiscard]] const std::vector<PayoutStatusValue>& get_status() const noexcept { return status; }
    void set_payout_method(std::vector<PayoutTypeValue> value) { payout_method = std::move(value); }
    [[nodiscard]] const std::vector<PayoutTypeValue>& get_payout_method() const noexcept { return payout_method; }
    std::vector<PayoutConnectorsValue> connector{};
    std::vector<CurrencyValue> currency{};
    std::vector<PayoutStatusValue> status{};
    std::vector<PayoutTypeValue> payout_method{};
};

class PayoutListFiltersV2
{
public:
    void set_connector(std::unordered_map<std::string, std::vector<MerchantConnectorInfo>> value) { connector = std::move(value); }
    [[nodiscard]] const std::unordered_map<std::string, std::vector<MerchantConnectorInfo>>& get_connector() const noexcept { return connector; }
    void set_currency(std::vector<CurrencyValue> value) { currency = std::move(value); }
    [[nodiscard]] const std::vector<CurrencyValue>& get_currency() const noexcept { return currency; }
    void set_status(std::vector<PayoutStatusValue> value) { status = std::move(value); }
    [[nodiscard]] const std::vector<PayoutStatusValue>& get_status() const noexcept { return status; }
    void set_payout_method(std::vector<PayoutTypeValue> value) { payout_method = std::move(value); }
    [[nodiscard]] const std::vector<PayoutTypeValue>& get_payout_method() const noexcept { return payout_method; }
    std::unordered_map<std::string, std::vector<MerchantConnectorInfo>> connector{};
    std::vector<CurrencyValue> currency{};
    std::vector<PayoutStatusValue> status{};
    std::vector<PayoutTypeValue> payout_method{};
};

class PayoutRetrieveBody
{
public:
    void set_force_sync(std::optional<bool> value) { force_sync = std::move(value); }
    [[nodiscard]] const std::optional<bool>& get_force_sync() const noexcept { return force_sync; }
    void set_merchant_id(std::optional<MerchantIdValue> value) { merchant_id = std::move(value); }
    [[nodiscard]] const std::optional<MerchantIdValue>& get_merchant_id() const noexcept { return merchant_id; }
    void set_expand_attempts(std::optional<bool> value) { expand_attempts = std::move(value); }
    [[nodiscard]] const std::optional<bool>& get_expand_attempts() const noexcept { return expand_attempts; }
    std::optional<bool> force_sync{};
    std::optional<MerchantIdValue> merchant_id{};
    std::optional<bool> expand_attempts{};
};

class PayoutRetrieveRequest
{
public:
    void set_payout_id(PayoutIdValue value) { payout_id = std::move(value); }
    [[nodiscard]] const PayoutIdValue& get_payout_id() const noexcept { return payout_id; }
    void set_force_sync(std::optional<bool> value) { force_sync = std::move(value); }
    [[nodiscard]] const std::optional<bool>& get_force_sync() const noexcept { return force_sync; }
    void set_merchant_id(std::optional<MerchantIdValue> value) { merchant_id = std::move(value); }
    [[nodiscard]] const std::optional<MerchantIdValue>& get_merchant_id() const noexcept { return merchant_id; }
    void set_expand_attempts(std::optional<bool> value) { expand_attempts = std::move(value); }
    [[nodiscard]] const std::optional<bool>& get_expand_attempts() const noexcept { return expand_attempts; }
    PayoutIdValue payout_id{};
    std::optional<bool> force_sync{};
    std::optional<MerchantIdValue> merchant_id{};
    std::optional<bool> expand_attempts{};
};

class PayoutVendorDetails
{
public:
    void set_account_type(std::string value) { account_type = std::move(value); }
    [[nodiscard]] const std::string& get_account_type() const noexcept { return account_type; }
    void set_business_type(std::string value) { business_type = std::move(value); }
    [[nodiscard]] const std::string& get_business_type() const noexcept { return business_type; }
    void set_business_profile_mcc(std::optional<std::int32_t> value) { business_profile_mcc = std::move(value); }
    [[nodiscard]] const std::optional<std::int32_t>& get_business_profile_mcc() const noexcept { return business_profile_mcc; }
    void set_business_profile_url(std::optional<std::string> value) { business_profile_url = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_business_profile_url() const noexcept { return business_profile_url; }
    void set_business_profile_name(std::optional<SecretValue> value) { business_profile_name = std::move(value); }
    [[nodiscard]] const std::optional<SecretValue>& get_business_profile_name() const noexcept { return business_profile_name; }
    void set_company_address_line1(std::optional<SecretValue> value) { company_address_line1 = std::move(value); }
    [[nodiscard]] const std::optional<SecretValue>& get_company_address_line1() const noexcept { return company_address_line1; }
    void set_company_address_line2(std::optional<SecretValue> value) { company_address_line2 = std::move(value); }
    [[nodiscard]] const std::optional<SecretValue>& get_company_address_line2() const noexcept { return company_address_line2; }
    void set_company_address_postal_code(std::optional<SecretValue> value) { company_address_postal_code = std::move(value); }
    [[nodiscard]] const std::optional<SecretValue>& get_company_address_postal_code() const noexcept { return company_address_postal_code; }
    void set_company_address_city(std::optional<SecretValue> value) { company_address_city = std::move(value); }
    [[nodiscard]] const std::optional<SecretValue>& get_company_address_city() const noexcept { return company_address_city; }
    void set_company_address_state(std::optional<SecretValue> value) { company_address_state = std::move(value); }
    [[nodiscard]] const std::optional<SecretValue>& get_company_address_state() const noexcept { return company_address_state; }
    void set_company_phone(std::optional<SecretValue> value) { company_phone = std::move(value); }
    [[nodiscard]] const std::optional<SecretValue>& get_company_phone() const noexcept { return company_phone; }
    void set_company_tax_id(std::optional<SecretValue> value) { company_tax_id = std::move(value); }
    [[nodiscard]] const std::optional<SecretValue>& get_company_tax_id() const noexcept { return company_tax_id; }
    void set_company_owners_provided(std::optional<bool> value) { company_owners_provided = std::move(value); }
    [[nodiscard]] const std::optional<bool>& get_company_owners_provided() const noexcept { return company_owners_provided; }
    void set_capabilities_card_payments(std::optional<bool> value) { capabilities_card_payments = std::move(value); }
    [[nodiscard]] const std::optional<bool>& get_capabilities_card_payments() const noexcept { return capabilities_card_payments; }
    void set_capabilities_transfers(std::optional<bool> value) { capabilities_transfers = std::move(value); }
    [[nodiscard]] const std::optional<bool>& get_capabilities_transfers() const noexcept { return capabilities_transfers; }
    void set_company_address_country(std::optional<CountryAlpha2Value> value) { company_address_country = std::move(value); }
    [[nodiscard]] const std::optional<CountryAlpha2Value>& get_company_address_country() const noexcept { return company_address_country; }
    void set_email(std::optional<EmailValue> value) { email = std::move(value); }
    [[nodiscard]] const std::optional<EmailValue>& get_email() const noexcept { return email; }
    void set_phone_country_code(std::optional<std::string> value) { phone_country_code = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_phone_country_code() const noexcept { return phone_country_code; }
    void set_reference_id(std::optional<SecretValue> value) { reference_id = std::move(value); }
    [[nodiscard]] const std::optional<SecretValue>& get_reference_id() const noexcept { return reference_id; }
    void set_source_of_funds(std::optional<SourceOfFundsValue> value) { source_of_funds = std::move(value); }
    [[nodiscard]] const std::optional<SourceOfFundsValue>& get_source_of_funds() const noexcept { return source_of_funds; }
    std::string account_type{};
    std::string business_type{};
    std::optional<std::int32_t> business_profile_mcc{};
    std::optional<std::string> business_profile_url{};
    std::optional<SecretValue> business_profile_name{};
    std::optional<SecretValue> company_address_line1{};
    std::optional<SecretValue> company_address_line2{};
    std::optional<SecretValue> company_address_postal_code{};
    std::optional<SecretValue> company_address_city{};
    std::optional<SecretValue> company_address_state{};
    std::optional<SecretValue> company_phone{};
    std::optional<SecretValue> company_tax_id{};
    std::optional<bool> company_owners_provided{};
    std::optional<bool> capabilities_card_payments{};
    std::optional<bool> capabilities_transfers{};
    std::optional<CountryAlpha2Value> company_address_country{};
    std::optional<EmailValue> email{};
    std::optional<std::string> phone_country_code{};
    std::optional<SecretValue> reference_id{};
    std::optional<SourceOfFundsValue> source_of_funds{};
};

class PayoutsManualUpdateRequest
{
public:
    void set_payout_id(PayoutIdValue value) { payout_id = std::move(value); }
    [[nodiscard]] const PayoutIdValue& get_payout_id() const noexcept { return payout_id; }
    void set_payout_attempt_id(std::string value) { payout_attempt_id = std::move(value); }
    [[nodiscard]] const std::string& get_payout_attempt_id() const noexcept { return payout_attempt_id; }
    void set_merchant_id(MerchantIdValue value) { merchant_id = std::move(value); }
    [[nodiscard]] const MerchantIdValue& get_merchant_id() const noexcept { return merchant_id; }
    void set_status(std::optional<PayoutStatusValue> value) { status = std::move(value); }
    [[nodiscard]] const std::optional<PayoutStatusValue>& get_status() const noexcept { return status; }
    void set_error_code(std::optional<std::string> value) { error_code = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_error_code() const noexcept { return error_code; }
    void set_error_message(std::optional<std::string> value) { error_message = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_error_message() const noexcept { return error_message; }
    void set_connector_payout_id(std::optional<std::string> value) { connector_payout_id = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_connector_payout_id() const noexcept { return connector_payout_id; }
    PayoutIdValue payout_id{};
    std::string payout_attempt_id{};
    MerchantIdValue merchant_id{};
    std::optional<PayoutStatusValue> status{};
    std::optional<std::string> error_code{};
    std::optional<std::string> error_message{};
    std::optional<std::string> connector_payout_id{};
};

class PayoutsManualUpdateResponse
{
public:
    void set_payout_id(PayoutIdValue value) { payout_id = std::move(value); }
    [[nodiscard]] const PayoutIdValue& get_payout_id() const noexcept { return payout_id; }
    void set_payout_attempt_id(std::string value) { payout_attempt_id = std::move(value); }
    [[nodiscard]] const std::string& get_payout_attempt_id() const noexcept { return payout_attempt_id; }
    void set_merchant_id(MerchantIdValue value) { merchant_id = std::move(value); }
    [[nodiscard]] const MerchantIdValue& get_merchant_id() const noexcept { return merchant_id; }
    void set_attempt_status(PayoutStatusValue value) { attempt_status = std::move(value); }
    [[nodiscard]] const PayoutStatusValue& get_attempt_status() const noexcept { return attempt_status; }
    void set_error_code(std::optional<std::string> value) { error_code = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_error_code() const noexcept { return error_code; }
    void set_error_message(std::optional<std::string> value) { error_message = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_error_message() const noexcept { return error_message; }
    void set_connector_payout_id(std::optional<std::string> value) { connector_payout_id = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_connector_payout_id() const noexcept { return connector_payout_id; }
    PayoutIdValue payout_id{};
    std::string payout_attempt_id{};
    MerchantIdValue merchant_id{};
    PayoutStatusValue attempt_status{};
    std::optional<std::string> error_code{};
    std::optional<std::string> error_message{};
    std::optional<std::string> connector_payout_id{};
};

class Paypal
{
public:
    void set_email(std::optional<EmailValue> value) { email = std::move(value); }
    [[nodiscard]] const std::optional<EmailValue>& get_email() const noexcept { return email; }
    void set_telephone_number(std::optional<SecretValue> value) { telephone_number = std::move(value); }
    [[nodiscard]] const std::optional<SecretValue>& get_telephone_number() const noexcept { return telephone_number; }
    void set_paypal_id(std::optional<SecretValue> value) { paypal_id = std::move(value); }
    [[nodiscard]] const std::optional<SecretValue>& get_paypal_id() const noexcept { return paypal_id; }
    std::optional<EmailValue> email{};
    std::optional<SecretValue> telephone_number{};
    std::optional<SecretValue> paypal_id{};
};

class PayshapBankTransfer
{
public:
    void set_bank_account_number(SecretValue value) { bank_account_number = std::move(value); }
    [[nodiscard]] const SecretValue& get_bank_account_number() const noexcept { return bank_account_number; }
    void set_account_holder_name(std::optional<SecretValue> value) { account_holder_name = std::move(value); }
    [[nodiscard]] const std::optional<SecretValue>& get_account_holder_name() const noexcept { return account_holder_name; }
    void set_bank_name(std::optional<BankNamesValue> value) { bank_name = std::move(value); }
    [[nodiscard]] const std::optional<BankNamesValue>& get_bank_name() const noexcept { return bank_name; }
    SecretValue bank_account_number{};
    std::optional<SecretValue> account_holder_name{};
    std::optional<BankNamesValue> bank_name{};
};

class PayshapProxyBankTransfer
{
public:
    void set_cellphone(std::optional<SecretValue> value) { cellphone = std::move(value); }
    [[nodiscard]] const std::optional<SecretValue>& get_cellphone() const noexcept { return cellphone; }
    void set_shap_id(std::optional<SecretValue> value) { shap_id = std::move(value); }
    [[nodiscard]] const std::optional<SecretValue>& get_shap_id() const noexcept { return shap_id; }
    std::optional<SecretValue> cellphone{};
    std::optional<SecretValue> shap_id{};
};

class PixAccountBankTransfer
{
public:
    void set_bank_name(std::optional<std::string> value) { bank_name = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_bank_name() const noexcept { return bank_name; }
    void set_bank_branch(std::optional<std::string> value) { bank_branch = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_bank_branch() const noexcept { return bank_branch; }
    void set_bank_account_number(SecretValue value) { bank_account_number = std::move(value); }
    [[nodiscard]] const SecretValue& get_bank_account_number() const noexcept { return bank_account_number; }
    void set_tax_id(std::optional<SecretValue> value) { tax_id = std::move(value); }
    [[nodiscard]] const std::optional<SecretValue>& get_tax_id() const noexcept { return tax_id; }
    void set_ispb(std::optional<std::string> value) { ispb = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_ispb() const noexcept { return ispb; }
    void set_bank_code(std::optional<std::string> value) { bank_code = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_bank_code() const noexcept { return bank_code; }
    void set_bank_account_type(std::optional<BankTypeValue> value) { bank_account_type = std::move(value); }
    [[nodiscard]] const std::optional<BankTypeValue>& get_bank_account_type() const noexcept { return bank_account_type; }
    void set_account_holder_name(std::optional<SecretValue> value) { account_holder_name = std::move(value); }
    [[nodiscard]] const std::optional<SecretValue>& get_account_holder_name() const noexcept { return account_holder_name; }
    std::optional<std::string> bank_name{};
    std::optional<std::string> bank_branch{};
    SecretValue bank_account_number{};
    std::optional<SecretValue> tax_id{};
    std::optional<std::string> ispb{};
    std::optional<std::string> bank_code{};
    std::optional<BankTypeValue> bank_account_type{};
    std::optional<SecretValue> account_holder_name{};
};

class PixBankTransfer
{
public:
    void set_bank_name(std::optional<std::string> value) { bank_name = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_bank_name() const noexcept { return bank_name; }
    void set_bank_branch(std::optional<std::string> value) { bank_branch = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_bank_branch() const noexcept { return bank_branch; }
    void set_bank_account_number(std::optional<SecretValue> value) { bank_account_number = std::move(value); }
    [[nodiscard]] const std::optional<SecretValue>& get_bank_account_number() const noexcept { return bank_account_number; }
    void set_pix_key(std::optional<SecretValue> value) { pix_key = std::move(value); }
    [[nodiscard]] const std::optional<SecretValue>& get_pix_key() const noexcept { return pix_key; }
    void set_tax_id(std::optional<SecretValue> value) { tax_id = std::move(value); }
    [[nodiscard]] const std::optional<SecretValue>& get_tax_id() const noexcept { return tax_id; }
    void set_emv(std::optional<SecretValue> value) { emv = std::move(value); }
    [[nodiscard]] const std::optional<SecretValue>& get_emv() const noexcept { return emv; }
    void set_ispb(std::optional<std::string> value) { ispb = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_ispb() const noexcept { return ispb; }
    void set_bank_code(std::optional<std::string> value) { bank_code = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_bank_code() const noexcept { return bank_code; }
    void set_bank_account_type(std::optional<BankTypeValue> value) { bank_account_type = std::move(value); }
    [[nodiscard]] const std::optional<BankTypeValue>& get_bank_account_type() const noexcept { return bank_account_type; }
    void set_account_holder_name(std::optional<SecretValue> value) { account_holder_name = std::move(value); }
    [[nodiscard]] const std::optional<SecretValue>& get_account_holder_name() const noexcept { return account_holder_name; }
    std::optional<std::string> bank_name{};
    std::optional<std::string> bank_branch{};
    std::optional<SecretValue> bank_account_number{};
    std::optional<SecretValue> pix_key{};
    std::optional<SecretValue> tax_id{};
    std::optional<SecretValue> emv{};
    std::optional<std::string> ispb{};
    std::optional<std::string> bank_code{};
    std::optional<BankTypeValue> bank_account_type{};
    std::optional<SecretValue> account_holder_name{};
};

class PixEmvBankTransfer
{
public:
    void set_emv(SecretValue value) { emv = std::move(value); }
    [[nodiscard]] const SecretValue& get_emv() const noexcept { return emv; }
    SecretValue emv{};
};

class PixKeyBankTransfer
{
public:
    void set_pix_key(SecretValue value) { pix_key = std::move(value); }
    [[nodiscard]] const SecretValue& get_pix_key() const noexcept { return pix_key; }
    SecretValue pix_key{};
};

class RequiredFieldsOverrideRequest
{
public:
    void set_billing(std::optional<AddressValue> value) { billing = std::move(value); }
    [[nodiscard]] const std::optional<AddressValue>& get_billing() const noexcept { return billing; }
    std::optional<AddressValue> billing{};
};

class SepaBankTransfer
{
public:
    void set_bank_name(std::optional<std::string> value) { bank_name = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_bank_name() const noexcept { return bank_name; }
    void set_bank_country_code(std::optional<CountryAlpha2Value> value) { bank_country_code = std::move(value); }
    [[nodiscard]] const std::optional<CountryAlpha2Value>& get_bank_country_code() const noexcept { return bank_country_code; }
    void set_bank_city(std::optional<std::string> value) { bank_city = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_bank_city() const noexcept { return bank_city; }
    void set_iban(SecretValue value) { iban = std::move(value); }
    [[nodiscard]] const SecretValue& get_iban() const noexcept { return iban; }
    void set_bic(std::optional<SecretValue> value) { bic = std::move(value); }
    [[nodiscard]] const std::optional<SecretValue>& get_bic() const noexcept { return bic; }
    void set_account_holder_name(std::optional<SecretValue> value) { account_holder_name = std::move(value); }
    [[nodiscard]] const std::optional<SecretValue>& get_account_holder_name() const noexcept { return account_holder_name; }
    std::optional<std::string> bank_name{};
    std::optional<CountryAlpha2Value> bank_country_code{};
    std::optional<std::string> bank_city{};
    SecretValue iban{};
    std::optional<SecretValue> bic{};
    std::optional<SecretValue> account_holder_name{};
};

class TedBankTransfer
{
public:
    void set_bank_name(std::optional<BankNamesValue> value) { bank_name = std::move(value); }
    [[nodiscard]] const std::optional<BankNamesValue>& get_bank_name() const noexcept { return bank_name; }
    void set_bank_code(std::optional<std::string> value) { bank_code = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_bank_code() const noexcept { return bank_code; }
    void set_ispb(std::optional<std::string> value) { ispb = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_ispb() const noexcept { return ispb; }
    void set_bank_branch(std::optional<std::string> value) { bank_branch = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_bank_branch() const noexcept { return bank_branch; }
    void set_bank_account_number(SecretValue value) { bank_account_number = std::move(value); }
    [[nodiscard]] const SecretValue& get_bank_account_number() const noexcept { return bank_account_number; }
    void set_bank_account_type(std::optional<BankTypeValue> value) { bank_account_type = std::move(value); }
    [[nodiscard]] const std::optional<BankTypeValue>& get_bank_account_type() const noexcept { return bank_account_type; }
    void set_tax_id(std::optional<SecretValue> value) { tax_id = std::move(value); }
    [[nodiscard]] const std::optional<SecretValue>& get_tax_id() const noexcept { return tax_id; }
    void set_account_holder_name(std::optional<SecretValue> value) { account_holder_name = std::move(value); }
    [[nodiscard]] const std::optional<SecretValue>& get_account_holder_name() const noexcept { return account_holder_name; }
    std::optional<BankNamesValue> bank_name{};
    std::optional<std::string> bank_code{};
    std::optional<std::string> ispb{};
    std::optional<std::string> bank_branch{};
    SecretValue bank_account_number{};
    std::optional<BankTypeValue> bank_account_type{};
    std::optional<SecretValue> tax_id{};
    std::optional<SecretValue> account_holder_name{};
};

class TrustlyBankTransfer
{
public:
    void set_iban(std::optional<SecretValue> value) { iban = std::move(value); }
    [[nodiscard]] const std::optional<SecretValue>& get_iban() const noexcept { return iban; }
    void set_country_code(CountryAlpha2Value value) { country_code = std::move(value); }
    [[nodiscard]] const CountryAlpha2Value& get_country_code() const noexcept { return country_code; }
    void set_account_number(std::optional<SecretValue> value) { account_number = std::move(value); }
    [[nodiscard]] const std::optional<SecretValue>& get_account_number() const noexcept { return account_number; }
    void set_bank_number(std::optional<SecretValue> value) { bank_number = std::move(value); }
    [[nodiscard]] const std::optional<SecretValue>& get_bank_number() const noexcept { return bank_number; }
    std::optional<SecretValue> iban{};
    CountryAlpha2Value country_code{};
    std::optional<SecretValue> account_number{};
    std::optional<SecretValue> bank_number{};
};

class TrustlyBankTransferData
{
public:
    void set_iban(std::optional<SecretValue> value) { iban = std::move(value); }
    [[nodiscard]] const std::optional<SecretValue>& get_iban() const noexcept { return iban; }
    void set_bank_country_code(CountryAlpha2Value value) { bank_country_code = std::move(value); }
    [[nodiscard]] const CountryAlpha2Value& get_bank_country_code() const noexcept { return bank_country_code; }
    void set_bank_account_number(std::optional<SecretValue> value) { bank_account_number = std::move(value); }
    [[nodiscard]] const std::optional<SecretValue>& get_bank_account_number() const noexcept { return bank_account_number; }
    void set_bank_number(std::optional<SecretValue> value) { bank_number = std::move(value); }
    [[nodiscard]] const std::optional<SecretValue>& get_bank_number() const noexcept { return bank_number; }
    std::optional<SecretValue> iban{};
    CountryAlpha2Value bank_country_code{};
    std::optional<SecretValue> bank_account_number{};
    std::optional<SecretValue> bank_number{};
};

class Venmo
{
public:
    void set_telephone_number(std::optional<SecretValue> value) { telephone_number = std::move(value); }
    [[nodiscard]] const std::optional<SecretValue>& get_telephone_number() const noexcept { return telephone_number; }
    std::optional<SecretValue> telephone_number{};
};

enum class Bank
{
    Ach,
    Bacs,
    Trustly,
    Sepa,
    Pix,
    OpenBanking,
    Payshap,
    PayshapProxy,
    Ted,
};

enum class BankRedirect
{
    Interac,
    OpenBankingUk,
};

enum class BankTransfer
{
    Ach,
    Bacs,
    Sepa,
    Pix,
    PixKey,
    PixEmv,
    Trustly,
    OpenBanking,
    Payshap,
    PayshapProxy,
    Ted,
};

class PayoutEnabledPaymentMethodsInfo
{
public:
    void set_payment_method(PaymentMethodValue value) { payment_method = std::move(value); }
    [[nodiscard]] const PaymentMethodValue& get_payment_method() const noexcept { return payment_method; }
    void set_payment_method_types_info(std::vector<PaymentMethodTypeInfo> value) { payment_method_types_info = std::move(value); }
    [[nodiscard]] const std::vector<PaymentMethodTypeInfo>& get_payment_method_types_info() const noexcept { return payment_method_types_info; }
    PaymentMethodValue payment_method{};
    std::vector<PaymentMethodTypeInfo> payment_method_types_info{};
};

class PayoutVendorAccountDetails
{
public:
    void set_vendor_details(PayoutVendorDetails value) { vendor_details = std::move(value); }
    [[nodiscard]] const PayoutVendorDetails& get_vendor_details() const noexcept { return vendor_details; }
    void set_individual_details(PayoutIndividualDetails value) { individual_details = std::move(value); }
    [[nodiscard]] const PayoutIndividualDetails& get_individual_details() const noexcept { return individual_details; }
    PayoutVendorDetails vendor_details{};
    PayoutIndividualDetails individual_details{};
};

enum class Wallet
{
    ApplePayDecrypt,
    GooglePayDecrypt,
    Mifinity,
    Paypal,
    Venmo,
};

class PayoutLinkDetails
{
public:
    void set_publishable_key(SecretValue value) { publishable_key = std::move(value); }
    [[nodiscard]] const SecretValue& get_publishable_key() const noexcept { return publishable_key; }
    void set_client_secret(SecretValue value) { client_secret = std::move(value); }
    [[nodiscard]] const SecretValue& get_client_secret() const noexcept { return client_secret; }
    void set_payout_link_id(std::string value) { payout_link_id = std::move(value); }
    [[nodiscard]] const std::string& get_payout_link_id() const noexcept { return payout_link_id; }
    void set_payout_id(PayoutIdValue value) { payout_id = std::move(value); }
    [[nodiscard]] const PayoutIdValue& get_payout_id() const noexcept { return payout_id; }
    void set_customer_id(CustomerIdValue value) { customer_id = std::move(value); }
    [[nodiscard]] const CustomerIdValue& get_customer_id() const noexcept { return customer_id; }
    void set_session_expiry(PrimitiveDateTimeValue value) { session_expiry = std::move(value); }
    [[nodiscard]] const PrimitiveDateTimeValue& get_session_expiry() const noexcept { return session_expiry; }
    void set_return_url(std::optional<UrlValue> value) { return_url = std::move(value); }
    [[nodiscard]] const std::optional<UrlValue>& get_return_url() const noexcept { return return_url; }
    void set_ui_config(GenericLinkUiConfigFormDataValue value) { ui_config = std::move(value); }
    [[nodiscard]] const GenericLinkUiConfigFormDataValue& get_ui_config() const noexcept { return ui_config; }
    void set_enabled_payment_methods(std::vector<EnabledPaymentMethodValue> value) { enabled_payment_methods = std::move(value); }
    [[nodiscard]] const std::vector<EnabledPaymentMethodValue>& get_enabled_payment_methods() const noexcept { return enabled_payment_methods; }
    void set_enabled_payment_methods_with_required_fields(std::vector<PayoutEnabledPaymentMethodsInfo> value) { enabled_payment_methods_with_required_fields = std::move(value); }
    [[nodiscard]] const std::vector<PayoutEnabledPaymentMethodsInfo>& get_enabled_payment_methods_with_required_fields() const noexcept { return enabled_payment_methods_with_required_fields; }
    void set_amount(StringMajorUnitValue value) { amount = std::move(value); }
    [[nodiscard]] const StringMajorUnitValue& get_amount() const noexcept { return amount; }
    void set_currency(CurrencyValue value) { currency = std::move(value); }
    [[nodiscard]] const CurrencyValue& get_currency() const noexcept { return currency; }
    void set_locale(std::string value) { locale = std::move(value); }
    [[nodiscard]] const std::string& get_locale() const noexcept { return locale; }
    void set_form_layout(std::optional<UIWidgetFormLayoutValue> value) { form_layout = std::move(value); }
    [[nodiscard]] const std::optional<UIWidgetFormLayoutValue>& get_form_layout() const noexcept { return form_layout; }
    void set_test_mode(bool value) { test_mode = std::move(value); }
    [[nodiscard]] const bool& get_test_mode() const noexcept { return test_mode; }
    SecretValue publishable_key{};
    SecretValue client_secret{};
    std::string payout_link_id{};
    PayoutIdValue payout_id{};
    CustomerIdValue customer_id{};
    PrimitiveDateTimeValue session_expiry{};
    std::optional<UrlValue> return_url{};
    GenericLinkUiConfigFormDataValue ui_config{};
    std::vector<EnabledPaymentMethodValue> enabled_payment_methods{};
    std::vector<PayoutEnabledPaymentMethodsInfo> enabled_payment_methods_with_required_fields{};
    StringMajorUnitValue amount{};
    CurrencyValue currency{};
    std::string locale{};
    std::optional<UIWidgetFormLayoutValue> form_layout{};
    bool test_mode{};
};

enum class PayoutMethodData
{
    Card,
    Bank,
    Wallet,
    BankRedirect,
    Passthrough,
    BankTransfer,
};

enum class PayoutMethodDataResponse
{
    Card,
    Bank,
    Wallet,
    BankRedirect,
    Passthrough,
};

class PayoutCreateRequest
{
public:
    void set_payout_id(std::optional<PayoutIdValue> value) { payout_id = std::move(value); }
    [[nodiscard]] const std::optional<PayoutIdValue>& get_payout_id() const noexcept { return payout_id; }
    void set_merchant_id(std::optional<MerchantIdValue> value) { merchant_id = std::move(value); }
    [[nodiscard]] const std::optional<MerchantIdValue>& get_merchant_id() const noexcept { return merchant_id; }
    void set_merchant_order_reference_id(std::optional<std::string> value) { merchant_order_reference_id = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_merchant_order_reference_id() const noexcept { return merchant_order_reference_id; }
    void set_amount(std::optional<AmountValue> value) { amount = std::move(value); }
    [[nodiscard]] const std::optional<AmountValue>& get_amount() const noexcept { return amount; }
    void set_currency(std::optional<CurrencyValue> value) { currency = std::move(value); }
    [[nodiscard]] const std::optional<CurrencyValue>& get_currency() const noexcept { return currency; }
    void set_routing(std::optional<serde::Value> value) { routing = std::move(value); }
    [[nodiscard]] const std::optional<serde::Value>& get_routing() const noexcept { return routing; }
    void set_connector(std::optional<std::vector<PayoutConnectorsValue>> value) { connector = std::move(value); }
    [[nodiscard]] const std::optional<std::vector<PayoutConnectorsValue>>& get_connector() const noexcept { return connector; }
    void set_confirm(std::optional<bool> value) { confirm = std::move(value); }
    [[nodiscard]] const std::optional<bool>& get_confirm() const noexcept { return confirm; }
    void set_payout_type(std::optional<PayoutTypeValue> value) { payout_type = std::move(value); }
    [[nodiscard]] const std::optional<PayoutTypeValue>& get_payout_type() const noexcept { return payout_type; }
    void set_payout_method_data(std::optional<PayoutMethodData> value) { payout_method_data = std::move(value); }
    [[nodiscard]] const std::optional<PayoutMethodData>& get_payout_method_data() const noexcept { return payout_method_data; }
    void set_source_bank_data(std::optional<BankTransfer> value) { source_bank_data = std::move(value); }
    [[nodiscard]] const std::optional<BankTransfer>& get_source_bank_data() const noexcept { return source_bank_data; }
    void set_billing(std::optional<AddressValue> value) { billing = std::move(value); }
    [[nodiscard]] const std::optional<AddressValue>& get_billing() const noexcept { return billing; }
    void set_auto_fulfill(std::optional<bool> value) { auto_fulfill = std::move(value); }
    [[nodiscard]] const std::optional<bool>& get_auto_fulfill() const noexcept { return auto_fulfill; }
    void set_customer_id(std::optional<CustomerIdValue> value) { customer_id = std::move(value); }
    [[nodiscard]] const std::optional<CustomerIdValue>& get_customer_id() const noexcept { return customer_id; }
    void set_customer(std::optional<CustomerDetailsValue> value) { customer = std::move(value); }
    [[nodiscard]] const std::optional<CustomerDetailsValue>& get_customer() const noexcept { return customer; }
    void set_client_secret(std::optional<std::string> value) { client_secret = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_client_secret() const noexcept { return client_secret; }
    void set_return_url(std::optional<std::string> value) { return_url = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_return_url() const noexcept { return return_url; }
    void set_business_country(std::optional<CountryAlpha2Value> value) { business_country = std::move(value); }
    [[nodiscard]] const std::optional<CountryAlpha2Value>& get_business_country() const noexcept { return business_country; }
    void set_business_label(std::optional<std::string> value) { business_label = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_business_label() const noexcept { return business_label; }
    void set_description(std::optional<std::string> value) { description = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_description() const noexcept { return description; }
    void set_billing_descriptor(std::optional<PayoutsBillingDescriptorValue> value) { billing_descriptor = std::move(value); }
    [[nodiscard]] const std::optional<PayoutsBillingDescriptorValue>& get_billing_descriptor() const noexcept { return billing_descriptor; }
    void set_entity_type(std::optional<PayoutEntityTypeValue> value) { entity_type = std::move(value); }
    [[nodiscard]] const std::optional<PayoutEntityTypeValue>& get_entity_type() const noexcept { return entity_type; }
    void set_recurring(std::optional<bool> value) { recurring = std::move(value); }
    [[nodiscard]] const std::optional<bool>& get_recurring() const noexcept { return recurring; }
    void set_metadata(std::optional<SecretSerdeValueValue> value) { metadata = std::move(value); }
    [[nodiscard]] const std::optional<SecretSerdeValueValue>& get_metadata() const noexcept { return metadata; }
    void set_funds_transfer_type(std::optional<PayoutFundsTransferTypeValue> value) { funds_transfer_type = std::move(value); }
    [[nodiscard]] const std::optional<PayoutFundsTransferTypeValue>& get_funds_transfer_type() const noexcept { return funds_transfer_type; }
    void set_purpose(std::optional<PayoutPurposeValue> value) { purpose = std::move(value); }
    [[nodiscard]] const std::optional<PayoutPurposeValue>& get_purpose() const noexcept { return purpose; }
    void set_sender_details(std::optional<SenderDetailsValue> value) { sender_details = std::move(value); }
    [[nodiscard]] const std::optional<SenderDetailsValue>& get_sender_details() const noexcept { return sender_details; }
    void set_payout_token(std::optional<std::string> value) { payout_token = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_payout_token() const noexcept { return payout_token; }
    void set_profile_id(std::optional<ProfileIdValue> value) { profile_id = std::move(value); }
    [[nodiscard]] const std::optional<ProfileIdValue>& get_profile_id() const noexcept { return profile_id; }
    void set_priority(std::optional<PayoutSendPriorityValue> value) { priority = std::move(value); }
    [[nodiscard]] const std::optional<PayoutSendPriorityValue>& get_priority() const noexcept { return priority; }
    void set_payout_link(std::optional<bool> value) { payout_link = std::move(value); }
    [[nodiscard]] const std::optional<bool>& get_payout_link() const noexcept { return payout_link; }
    void set_payout_link_config(std::optional<PayoutCreatePayoutLinkConfig> value) { payout_link_config = std::move(value); }
    [[nodiscard]] const std::optional<PayoutCreatePayoutLinkConfig>& get_payout_link_config() const noexcept { return payout_link_config; }
    void set_session_expiry(std::optional<std::uint32_t> value) { session_expiry = std::move(value); }
    [[nodiscard]] const std::optional<std::uint32_t>& get_session_expiry() const noexcept { return session_expiry; }
    void set_email(std::optional<EmailValue> value) { email = std::move(value); }
    [[nodiscard]] const std::optional<EmailValue>& get_email() const noexcept { return email; }
    void set_name(std::optional<SecretValue> value) { name = std::move(value); }
    [[nodiscard]] const std::optional<SecretValue>& get_name() const noexcept { return name; }
    void set_phone(std::optional<SecretValue> value) { phone = std::move(value); }
    [[nodiscard]] const std::optional<SecretValue>& get_phone() const noexcept { return phone; }
    void set_phone_country_code(std::optional<std::string> value) { phone_country_code = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_phone_country_code() const noexcept { return phone_country_code; }
    void set_payout_method_id(std::optional<std::string> value) { payout_method_id = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_payout_method_id() const noexcept { return payout_method_id; }
    void set_browser_info(std::optional<BrowserInformationValue> value) { browser_info = std::move(value); }
    [[nodiscard]] const std::optional<BrowserInformationValue>& get_browser_info() const noexcept { return browser_info; }
    std::optional<PayoutIdValue> payout_id{};
    std::optional<MerchantIdValue> merchant_id{};
    std::optional<std::string> merchant_order_reference_id{};
    std::optional<AmountValue> amount{};
    std::optional<CurrencyValue> currency{};
    std::optional<serde::Value> routing{};
    std::optional<std::vector<PayoutConnectorsValue>> connector{};
    std::optional<bool> confirm{};
    std::optional<PayoutTypeValue> payout_type{};
    std::optional<PayoutMethodData> payout_method_data{};
    std::optional<BankTransfer> source_bank_data{};
    std::optional<AddressValue> billing{};
    std::optional<bool> auto_fulfill{};
    std::optional<CustomerIdValue> customer_id{};
    std::optional<CustomerDetailsValue> customer{};
    std::optional<std::string> client_secret{};
    std::optional<std::string> return_url{};
    std::optional<CountryAlpha2Value> business_country{};
    std::optional<std::string> business_label{};
    std::optional<std::string> description{};
    std::optional<PayoutsBillingDescriptorValue> billing_descriptor{};
    std::optional<PayoutEntityTypeValue> entity_type{};
    std::optional<bool> recurring{};
    std::optional<SecretSerdeValueValue> metadata{};
    std::optional<PayoutFundsTransferTypeValue> funds_transfer_type{};
    std::optional<PayoutPurposeValue> purpose{};
    std::optional<SenderDetailsValue> sender_details{};
    std::optional<std::string> payout_token{};
    std::optional<ProfileIdValue> profile_id{};
    std::optional<PayoutSendPriorityValue> priority{};
    std::optional<bool> payout_link{};
    std::optional<PayoutCreatePayoutLinkConfig> payout_link_config{};
    std::optional<std::uint32_t> session_expiry{};
    std::optional<EmailValue> email{};
    std::optional<SecretValue> name{};
    std::optional<SecretValue> phone{};
    std::optional<std::string> phone_country_code{};
    std::optional<std::string> payout_method_id{};
    std::optional<BrowserInformationValue> browser_info{};
};

class PayoutCreateResponse
{
public:
    void set_payout_id(PayoutIdValue value) { payout_id = std::move(value); }
    [[nodiscard]] const PayoutIdValue& get_payout_id() const noexcept { return payout_id; }
    void set_merchant_id(MerchantIdValue value) { merchant_id = std::move(value); }
    [[nodiscard]] const MerchantIdValue& get_merchant_id() const noexcept { return merchant_id; }
    void set_merchant_order_reference_id(std::optional<std::string> value) { merchant_order_reference_id = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_merchant_order_reference_id() const noexcept { return merchant_order_reference_id; }
    void set_amount(MinorUnitValue value) { amount = std::move(value); }
    [[nodiscard]] const MinorUnitValue& get_amount() const noexcept { return amount; }
    void set_currency(CurrencyValue value) { currency = std::move(value); }
    [[nodiscard]] const CurrencyValue& get_currency() const noexcept { return currency; }
    void set_connector(std::optional<std::string> value) { connector = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_connector() const noexcept { return connector; }
    void set_payout_type(std::optional<PayoutTypeValue> value) { payout_type = std::move(value); }
    [[nodiscard]] const std::optional<PayoutTypeValue>& get_payout_type() const noexcept { return payout_type; }
    void set_payout_method_data(std::optional<PayoutMethodDataResponse> value) { payout_method_data = std::move(value); }
    [[nodiscard]] const std::optional<PayoutMethodDataResponse>& get_payout_method_data() const noexcept { return payout_method_data; }
    void set_source_bank_data(std::optional<BankAdditionalDataValue> value) { source_bank_data = std::move(value); }
    [[nodiscard]] const std::optional<BankAdditionalDataValue>& get_source_bank_data() const noexcept { return source_bank_data; }
    void set_billing(std::optional<AddressValue> value) { billing = std::move(value); }
    [[nodiscard]] const std::optional<AddressValue>& get_billing() const noexcept { return billing; }
    void set_auto_fulfill(bool value) { auto_fulfill = std::move(value); }
    [[nodiscard]] const bool& get_auto_fulfill() const noexcept { return auto_fulfill; }
    void set_customer_id(std::optional<CustomerIdValue> value) { customer_id = std::move(value); }
    [[nodiscard]] const std::optional<CustomerIdValue>& get_customer_id() const noexcept { return customer_id; }
    void set_customer(std::optional<CustomerDetailsResponseValue> value) { customer = std::move(value); }
    [[nodiscard]] const std::optional<CustomerDetailsResponseValue>& get_customer() const noexcept { return customer; }
    void set_client_secret(std::optional<std::string> value) { client_secret = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_client_secret() const noexcept { return client_secret; }
    void set_return_url(std::optional<std::string> value) { return_url = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_return_url() const noexcept { return return_url; }
    void set_business_country(std::optional<CountryAlpha2Value> value) { business_country = std::move(value); }
    [[nodiscard]] const std::optional<CountryAlpha2Value>& get_business_country() const noexcept { return business_country; }
    void set_business_label(std::optional<std::string> value) { business_label = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_business_label() const noexcept { return business_label; }
    void set_description(std::optional<std::string> value) { description = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_description() const noexcept { return description; }
    void set_billing_descriptor(std::optional<PayoutsBillingDescriptorValue> value) { billing_descriptor = std::move(value); }
    [[nodiscard]] const std::optional<PayoutsBillingDescriptorValue>& get_billing_descriptor() const noexcept { return billing_descriptor; }
    void set_entity_type(PayoutEntityTypeValue value) { entity_type = std::move(value); }
    [[nodiscard]] const PayoutEntityTypeValue& get_entity_type() const noexcept { return entity_type; }
    void set_recurring(bool value) { recurring = std::move(value); }
    [[nodiscard]] const bool& get_recurring() const noexcept { return recurring; }
    void set_metadata(std::optional<SecretSerdeValueValue> value) { metadata = std::move(value); }
    [[nodiscard]] const std::optional<SecretSerdeValueValue>& get_metadata() const noexcept { return metadata; }
    void set_funds_transfer_type(std::optional<PayoutFundsTransferTypeValue> value) { funds_transfer_type = std::move(value); }
    [[nodiscard]] const std::optional<PayoutFundsTransferTypeValue>& get_funds_transfer_type() const noexcept { return funds_transfer_type; }
    void set_purpose(std::optional<PayoutPurposeValue> value) { purpose = std::move(value); }
    [[nodiscard]] const std::optional<PayoutPurposeValue>& get_purpose() const noexcept { return purpose; }
    void set_sender_details(std::optional<SenderDetailsValue> value) { sender_details = std::move(value); }
    [[nodiscard]] const std::optional<SenderDetailsValue>& get_sender_details() const noexcept { return sender_details; }
    void set_merchant_connector_id(std::optional<MerchantConnectorAccountIdValue> value) { merchant_connector_id = std::move(value); }
    [[nodiscard]] const std::optional<MerchantConnectorAccountIdValue>& get_merchant_connector_id() const noexcept { return merchant_connector_id; }
    void set_status(PayoutStatusValue value) { status = std::move(value); }
    [[nodiscard]] const PayoutStatusValue& get_status() const noexcept { return status; }
    void set_error_message(std::optional<std::string> value) { error_message = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_error_message() const noexcept { return error_message; }
    void set_error_code(std::optional<std::string> value) { error_code = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_error_code() const noexcept { return error_code; }
    void set_frm_message(std::optional<FrmMessageValue> value) { frm_message = std::move(value); }
    [[nodiscard]] const std::optional<FrmMessageValue>& get_frm_message() const noexcept { return frm_message; }
    void set_profile_id(ProfileIdValue value) { profile_id = std::move(value); }
    [[nodiscard]] const ProfileIdValue& get_profile_id() const noexcept { return profile_id; }
    void set_created(std::optional<PrimitiveDateTimeValue> value) { created = std::move(value); }
    [[nodiscard]] const std::optional<PrimitiveDateTimeValue>& get_created() const noexcept { return created; }
    void set_connector_transaction_id(std::optional<std::string> value) { connector_transaction_id = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_connector_transaction_id() const noexcept { return connector_transaction_id; }
    void set_connector_eligibility_reference_id(std::optional<std::string> value) { connector_eligibility_reference_id = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_connector_eligibility_reference_id() const noexcept { return connector_eligibility_reference_id; }
    void set_priority(std::optional<PayoutSendPriorityValue> value) { priority = std::move(value); }
    [[nodiscard]] const std::optional<PayoutSendPriorityValue>& get_priority() const noexcept { return priority; }
    void set_attempts(std::optional<std::vector<PayoutAttemptResponse>> value) { attempts = std::move(value); }
    [[nodiscard]] const std::optional<std::vector<PayoutAttemptResponse>>& get_attempts() const noexcept { return attempts; }
    void set_payout_link(std::optional<PayoutLinkResponse> value) { payout_link = std::move(value); }
    [[nodiscard]] const std::optional<PayoutLinkResponse>& get_payout_link() const noexcept { return payout_link; }
    void set_email(OptionalEncryptableEmailValue value) { email = std::move(value); }
    [[nodiscard]] const OptionalEncryptableEmailValue& get_email() const noexcept { return email; }
    void set_name(OptionalEncryptableNameValue value) { name = std::move(value); }
    [[nodiscard]] const OptionalEncryptableNameValue& get_name() const noexcept { return name; }
    void set_phone(OptionalEncryptablePhoneValue value) { phone = std::move(value); }
    [[nodiscard]] const OptionalEncryptablePhoneValue& get_phone() const noexcept { return phone; }
    void set_phone_country_code(std::optional<std::string> value) { phone_country_code = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_phone_country_code() const noexcept { return phone_country_code; }
    void set_unified_code(std::optional<UnifiedCodeValue> value) { unified_code = std::move(value); }
    [[nodiscard]] const std::optional<UnifiedCodeValue>& get_unified_code() const noexcept { return unified_code; }
    void set_unified_message(std::optional<UnifiedMessageValue> value) { unified_message = std::move(value); }
    [[nodiscard]] const std::optional<UnifiedMessageValue>& get_unified_message() const noexcept { return unified_message; }
    void set_payout_method_id(std::optional<std::string> value) { payout_method_id = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_payout_method_id() const noexcept { return payout_method_id; }
    PayoutIdValue payout_id{};
    MerchantIdValue merchant_id{};
    std::optional<std::string> merchant_order_reference_id{};
    MinorUnitValue amount{};
    CurrencyValue currency{};
    std::optional<std::string> connector{};
    std::optional<PayoutTypeValue> payout_type{};
    std::optional<PayoutMethodDataResponse> payout_method_data{};
    std::optional<BankAdditionalDataValue> source_bank_data{};
    std::optional<AddressValue> billing{};
    bool auto_fulfill{};
    std::optional<CustomerIdValue> customer_id{};
    std::optional<CustomerDetailsResponseValue> customer{};
    std::optional<std::string> client_secret{};
    std::optional<std::string> return_url{};
    std::optional<CountryAlpha2Value> business_country{};
    std::optional<std::string> business_label{};
    std::optional<std::string> description{};
    std::optional<PayoutsBillingDescriptorValue> billing_descriptor{};
    PayoutEntityTypeValue entity_type{};
    bool recurring{};
    std::optional<SecretSerdeValueValue> metadata{};
    std::optional<PayoutFundsTransferTypeValue> funds_transfer_type{};
    std::optional<PayoutPurposeValue> purpose{};
    std::optional<SenderDetailsValue> sender_details{};
    std::optional<MerchantConnectorAccountIdValue> merchant_connector_id{};
    PayoutStatusValue status{};
    std::optional<std::string> error_message{};
    std::optional<std::string> error_code{};
    std::optional<FrmMessageValue> frm_message{};
    ProfileIdValue profile_id{};
    std::optional<PrimitiveDateTimeValue> created{};
    std::optional<std::string> connector_transaction_id{};
    std::optional<std::string> connector_eligibility_reference_id{};
    std::optional<PayoutSendPriorityValue> priority{};
    std::optional<std::vector<PayoutAttemptResponse>> attempts{};
    std::optional<PayoutLinkResponse> payout_link{};
    OptionalEncryptableEmailValue email{};
    OptionalEncryptableNameValue name{};
    OptionalEncryptablePhoneValue phone{};
    std::optional<std::string> phone_country_code{};
    std::optional<UnifiedCodeValue> unified_code{};
    std::optional<UnifiedMessageValue> unified_message{};
    std::optional<std::string> payout_method_id{};
};

class PayoutListResponse
{
public:
    void set_size(std::size_t value) { size = std::move(value); }
    [[nodiscard]] const std::size_t& get_size() const noexcept { return size; }
    void set_data(std::vector<PayoutCreateResponse> value) { data = std::move(value); }
    [[nodiscard]] const std::vector<PayoutCreateResponse>& get_data() const noexcept { return data; }
    void set_total_count(std::optional<std::int64_t> value) { total_count = std::move(value); }
    [[nodiscard]] const std::optional<std::int64_t>& get_total_count() const noexcept { return total_count; }
    std::size_t size{};
    std::vector<PayoutCreateResponse> data{};
    std::optional<std::int64_t> total_count{};
};

enum class PayoutRequest
{
    PayoutActionRequest,
    PayoutCreateRequest,
    PayoutRetrieveRequest,
};
} // namespace app::model::payouts

template<>
struct serde::Serializable<app::model::payouts::PayoutCreatePayoutLinkConfig>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"payout_link_id", &app::model::payouts::PayoutCreatePayoutLinkConfig::get_payout_link_id, &app::model::payouts::PayoutCreatePayoutLinkConfig::set_payout_link_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"ui_config", &app::model::payouts::PayoutCreatePayoutLinkConfig::get_ui_config, &app::model::payouts::PayoutCreatePayoutLinkConfig::set_ui_config, serde::FieldOptions{}>{},
            serde::FieldDesc<"enabled_payment_methods", &app::model::payouts::PayoutCreatePayoutLinkConfig::get_enabled_payment_methods, &app::model::payouts::PayoutCreatePayoutLinkConfig::set_enabled_payment_methods, serde::FieldOptions{}>{},
            serde::FieldDesc<"form_layout", &app::model::payouts::PayoutCreatePayoutLinkConfig::get_form_layout, &app::model::payouts::PayoutCreatePayoutLinkConfig::set_form_layout, serde::FieldOptions{}>{},
            serde::FieldDesc<"test_mode", &app::model::payouts::PayoutCreatePayoutLinkConfig::get_test_mode, &app::model::payouts::PayoutCreatePayoutLinkConfig::set_test_mode, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "payout_create_payout_link_config"; }
};

template<>
struct serde::Serializable<app::model::payouts::CardPayout>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"card_number", &app::model::payouts::CardPayout::get_card_number, &app::model::payouts::CardPayout::set_card_number, serde::FieldOptions{}>{},
            serde::FieldDesc<"expiry_month", &app::model::payouts::CardPayout::get_expiry_month, &app::model::payouts::CardPayout::set_expiry_month, serde::FieldOptions{}>{},
            serde::FieldDesc<"expiry_year", &app::model::payouts::CardPayout::get_expiry_year, &app::model::payouts::CardPayout::set_expiry_year, serde::FieldOptions{}>{},
            serde::FieldDesc<"card_holder_name", &app::model::payouts::CardPayout::get_card_holder_name, &app::model::payouts::CardPayout::set_card_holder_name, serde::FieldOptions{}>{},
            serde::FieldDesc<"card_network", &app::model::payouts::CardPayout::get_card_network, &app::model::payouts::CardPayout::set_card_network, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "card_payout"; }
};

template<>
struct serde::Serializable<app::model::payouts::AchBankTransfer>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"bank_name", &app::model::payouts::AchBankTransfer::get_bank_name, &app::model::payouts::AchBankTransfer::set_bank_name, serde::FieldOptions{}>{},
            serde::FieldDesc<"bank_country_code", &app::model::payouts::AchBankTransfer::get_bank_country_code, &app::model::payouts::AchBankTransfer::set_bank_country_code, serde::FieldOptions{}>{},
            serde::FieldDesc<"bank_city", &app::model::payouts::AchBankTransfer::get_bank_city, &app::model::payouts::AchBankTransfer::set_bank_city, serde::FieldOptions{}>{},
            serde::FieldDesc<"bank_account_number", &app::model::payouts::AchBankTransfer::get_bank_account_number, &app::model::payouts::AchBankTransfer::set_bank_account_number, serde::FieldOptions{}>{},
            serde::FieldDesc<"bank_routing_number", &app::model::payouts::AchBankTransfer::get_bank_routing_number, &app::model::payouts::AchBankTransfer::set_bank_routing_number, serde::FieldOptions{}>{},
            serde::FieldDesc<"account_holder_name", &app::model::payouts::AchBankTransfer::get_account_holder_name, &app::model::payouts::AchBankTransfer::set_account_holder_name, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "ach_bank_transfer"; }
};

template<>
struct serde::Serializable<app::model::payouts::BacsBankTransfer>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"bank_name", &app::model::payouts::BacsBankTransfer::get_bank_name, &app::model::payouts::BacsBankTransfer::set_bank_name, serde::FieldOptions{}>{},
            serde::FieldDesc<"bank_country_code", &app::model::payouts::BacsBankTransfer::get_bank_country_code, &app::model::payouts::BacsBankTransfer::set_bank_country_code, serde::FieldOptions{}>{},
            serde::FieldDesc<"bank_city", &app::model::payouts::BacsBankTransfer::get_bank_city, &app::model::payouts::BacsBankTransfer::set_bank_city, serde::FieldOptions{}>{},
            serde::FieldDesc<"bank_account_number", &app::model::payouts::BacsBankTransfer::get_bank_account_number, &app::model::payouts::BacsBankTransfer::set_bank_account_number, serde::FieldOptions{}>{},
            serde::FieldDesc<"bank_sort_code", &app::model::payouts::BacsBankTransfer::get_bank_sort_code, &app::model::payouts::BacsBankTransfer::set_bank_sort_code, serde::FieldOptions{}>{},
            serde::FieldDesc<"account_holder_name", &app::model::payouts::BacsBankTransfer::get_account_holder_name, &app::model::payouts::BacsBankTransfer::set_account_holder_name, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "bacs_bank_transfer"; }
};

template<>
struct serde::Serializable<app::model::payouts::SepaBankTransfer>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"bank_name", &app::model::payouts::SepaBankTransfer::get_bank_name, &app::model::payouts::SepaBankTransfer::set_bank_name, serde::FieldOptions{}>{},
            serde::FieldDesc<"bank_country_code", &app::model::payouts::SepaBankTransfer::get_bank_country_code, &app::model::payouts::SepaBankTransfer::set_bank_country_code, serde::FieldOptions{}>{},
            serde::FieldDesc<"bank_city", &app::model::payouts::SepaBankTransfer::get_bank_city, &app::model::payouts::SepaBankTransfer::set_bank_city, serde::FieldOptions{}>{},
            serde::FieldDesc<"iban", &app::model::payouts::SepaBankTransfer::get_iban, &app::model::payouts::SepaBankTransfer::set_iban, serde::FieldOptions{}>{},
            serde::FieldDesc<"bic", &app::model::payouts::SepaBankTransfer::get_bic, &app::model::payouts::SepaBankTransfer::set_bic, serde::FieldOptions{}>{},
            serde::FieldDesc<"account_holder_name", &app::model::payouts::SepaBankTransfer::get_account_holder_name, &app::model::payouts::SepaBankTransfer::set_account_holder_name, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "sepa_bank_transfer"; }
};

template<>
struct serde::Serializable<app::model::payouts::PixBankTransfer>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"bank_name", &app::model::payouts::PixBankTransfer::get_bank_name, &app::model::payouts::PixBankTransfer::set_bank_name, serde::FieldOptions{}>{},
            serde::FieldDesc<"bank_branch", &app::model::payouts::PixBankTransfer::get_bank_branch, &app::model::payouts::PixBankTransfer::set_bank_branch, serde::FieldOptions{}>{},
            serde::FieldDesc<"bank_account_number", &app::model::payouts::PixBankTransfer::get_bank_account_number, &app::model::payouts::PixBankTransfer::set_bank_account_number, serde::FieldOptions{}>{},
            serde::FieldDesc<"pix_key", &app::model::payouts::PixBankTransfer::get_pix_key, &app::model::payouts::PixBankTransfer::set_pix_key, serde::FieldOptions{}>{},
            serde::FieldDesc<"tax_id", &app::model::payouts::PixBankTransfer::get_tax_id, &app::model::payouts::PixBankTransfer::set_tax_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"emv", &app::model::payouts::PixBankTransfer::get_emv, &app::model::payouts::PixBankTransfer::set_emv, serde::FieldOptions{}>{},
            serde::FieldDesc<"ispb", &app::model::payouts::PixBankTransfer::get_ispb, &app::model::payouts::PixBankTransfer::set_ispb, serde::FieldOptions{}>{},
            serde::FieldDesc<"bank_code", &app::model::payouts::PixBankTransfer::get_bank_code, &app::model::payouts::PixBankTransfer::set_bank_code, serde::FieldOptions{}>{},
            serde::FieldDesc<"bank_account_type", &app::model::payouts::PixBankTransfer::get_bank_account_type, &app::model::payouts::PixBankTransfer::set_bank_account_type, serde::FieldOptions{}>{},
            serde::FieldDesc<"account_holder_name", &app::model::payouts::PixBankTransfer::get_account_holder_name, &app::model::payouts::PixBankTransfer::set_account_holder_name, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "pix_bank_transfer"; }
};

template<>
struct serde::Serializable<app::model::payouts::PixAccountBankTransfer>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"bank_name", &app::model::payouts::PixAccountBankTransfer::get_bank_name, &app::model::payouts::PixAccountBankTransfer::set_bank_name, serde::FieldOptions{}>{},
            serde::FieldDesc<"bank_branch", &app::model::payouts::PixAccountBankTransfer::get_bank_branch, &app::model::payouts::PixAccountBankTransfer::set_bank_branch, serde::FieldOptions{}>{},
            serde::FieldDesc<"bank_account_number", &app::model::payouts::PixAccountBankTransfer::get_bank_account_number, &app::model::payouts::PixAccountBankTransfer::set_bank_account_number, serde::FieldOptions{}>{},
            serde::FieldDesc<"tax_id", &app::model::payouts::PixAccountBankTransfer::get_tax_id, &app::model::payouts::PixAccountBankTransfer::set_tax_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"ispb", &app::model::payouts::PixAccountBankTransfer::get_ispb, &app::model::payouts::PixAccountBankTransfer::set_ispb, serde::FieldOptions{}>{},
            serde::FieldDesc<"bank_code", &app::model::payouts::PixAccountBankTransfer::get_bank_code, &app::model::payouts::PixAccountBankTransfer::set_bank_code, serde::FieldOptions{}>{},
            serde::FieldDesc<"bank_account_type", &app::model::payouts::PixAccountBankTransfer::get_bank_account_type, &app::model::payouts::PixAccountBankTransfer::set_bank_account_type, serde::FieldOptions{}>{},
            serde::FieldDesc<"account_holder_name", &app::model::payouts::PixAccountBankTransfer::get_account_holder_name, &app::model::payouts::PixAccountBankTransfer::set_account_holder_name, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "pix_account_bank_transfer"; }
};

template<>
struct serde::Serializable<app::model::payouts::PixKeyBankTransfer>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"pix_key", &app::model::payouts::PixKeyBankTransfer::get_pix_key, &app::model::payouts::PixKeyBankTransfer::set_pix_key, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "pix_key_bank_transfer"; }
};

template<>
struct serde::Serializable<app::model::payouts::PixEmvBankTransfer>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"emv", &app::model::payouts::PixEmvBankTransfer::get_emv, &app::model::payouts::PixEmvBankTransfer::set_emv, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "pix_emv_bank_transfer"; }
};

template<>
struct serde::Serializable<app::model::payouts::PayshapBankTransfer>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"bank_account_number", &app::model::payouts::PayshapBankTransfer::get_bank_account_number, &app::model::payouts::PayshapBankTransfer::set_bank_account_number, serde::FieldOptions{}>{},
            serde::FieldDesc<"account_holder_name", &app::model::payouts::PayshapBankTransfer::get_account_holder_name, &app::model::payouts::PayshapBankTransfer::set_account_holder_name, serde::FieldOptions{}>{},
            serde::FieldDesc<"bank_name", &app::model::payouts::PayshapBankTransfer::get_bank_name, &app::model::payouts::PayshapBankTransfer::set_bank_name, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "payshap_bank_transfer"; }
};

template<>
struct serde::Serializable<app::model::payouts::PayshapProxyBankTransfer>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"cellphone", &app::model::payouts::PayshapProxyBankTransfer::get_cellphone, &app::model::payouts::PayshapProxyBankTransfer::set_cellphone, serde::FieldOptions{}>{},
            serde::FieldDesc<"shap_id", &app::model::payouts::PayshapProxyBankTransfer::get_shap_id, &app::model::payouts::PayshapProxyBankTransfer::set_shap_id, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "payshap_proxy_bank_transfer"; }
};

template<>
struct serde::Serializable<app::model::payouts::TedBankTransfer>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"bank_name", &app::model::payouts::TedBankTransfer::get_bank_name, &app::model::payouts::TedBankTransfer::set_bank_name, serde::FieldOptions{}>{},
            serde::FieldDesc<"bank_code", &app::model::payouts::TedBankTransfer::get_bank_code, &app::model::payouts::TedBankTransfer::set_bank_code, serde::FieldOptions{}>{},
            serde::FieldDesc<"ispb", &app::model::payouts::TedBankTransfer::get_ispb, &app::model::payouts::TedBankTransfer::set_ispb, serde::FieldOptions{}>{},
            serde::FieldDesc<"bank_branch", &app::model::payouts::TedBankTransfer::get_bank_branch, &app::model::payouts::TedBankTransfer::set_bank_branch, serde::FieldOptions{}>{},
            serde::FieldDesc<"bank_account_number", &app::model::payouts::TedBankTransfer::get_bank_account_number, &app::model::payouts::TedBankTransfer::set_bank_account_number, serde::FieldOptions{}>{},
            serde::FieldDesc<"bank_account_type", &app::model::payouts::TedBankTransfer::get_bank_account_type, &app::model::payouts::TedBankTransfer::set_bank_account_type, serde::FieldOptions{}>{},
            serde::FieldDesc<"tax_id", &app::model::payouts::TedBankTransfer::get_tax_id, &app::model::payouts::TedBankTransfer::set_tax_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"account_holder_name", &app::model::payouts::TedBankTransfer::get_account_holder_name, &app::model::payouts::TedBankTransfer::set_account_holder_name, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "ted_bank_transfer"; }
};

template<>
struct serde::Serializable<app::model::payouts::Mifinity>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"destination_account", &app::model::payouts::Mifinity::get_destination_account, &app::model::payouts::Mifinity::set_destination_account, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "mifinity"; }
};

template<>
struct serde::Serializable<app::model::payouts::Interac>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"email", &app::model::payouts::Interac::get_email, &app::model::payouts::Interac::set_email, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "interac"; }
};

template<>
struct serde::Serializable<app::model::payouts::OpenBankingUk>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"account_holder_name", &app::model::payouts::OpenBankingUk::get_account_holder_name, &app::model::payouts::OpenBankingUk::set_account_holder_name, serde::FieldOptions{}>{},
            serde::FieldDesc<"iban", &app::model::payouts::OpenBankingUk::get_iban, &app::model::payouts::OpenBankingUk::set_iban, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "open_banking_uk"; }
};

template<>
struct serde::Serializable<app::model::payouts::OpenBanking>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"account_holder_name", &app::model::payouts::OpenBanking::get_account_holder_name, &app::model::payouts::OpenBanking::set_account_holder_name, serde::FieldOptions{}>{},
            serde::FieldDesc<"iban", &app::model::payouts::OpenBanking::get_iban, &app::model::payouts::OpenBanking::set_iban, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "open_banking"; }
};

template<>
struct serde::Serializable<app::model::payouts::TrustlyBankTransfer>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"iban", &app::model::payouts::TrustlyBankTransfer::get_iban, &app::model::payouts::TrustlyBankTransfer::set_iban, serde::FieldOptions{}>{},
            serde::FieldDesc<"country_code", &app::model::payouts::TrustlyBankTransfer::get_country_code, &app::model::payouts::TrustlyBankTransfer::set_country_code, serde::FieldOptions{}>{},
            serde::FieldDesc<"account_number", &app::model::payouts::TrustlyBankTransfer::get_account_number, &app::model::payouts::TrustlyBankTransfer::set_account_number, serde::FieldOptions{}>{},
            serde::FieldDesc<"bank_number", &app::model::payouts::TrustlyBankTransfer::get_bank_number, &app::model::payouts::TrustlyBankTransfer::set_bank_number, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "trustly_bank_transfer"; }
};

template<>
struct serde::Serializable<app::model::payouts::TrustlyBankTransferData>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"iban", &app::model::payouts::TrustlyBankTransferData::get_iban, &app::model::payouts::TrustlyBankTransferData::set_iban, serde::FieldOptions{}>{},
            serde::FieldDesc<"bank_country_code", &app::model::payouts::TrustlyBankTransferData::get_bank_country_code, &app::model::payouts::TrustlyBankTransferData::set_bank_country_code, serde::FieldOptions{}>{},
            serde::FieldDesc<"bank_account_number", &app::model::payouts::TrustlyBankTransferData::get_bank_account_number, &app::model::payouts::TrustlyBankTransferData::set_bank_account_number, serde::FieldOptions{}>{},
            serde::FieldDesc<"bank_number", &app::model::payouts::TrustlyBankTransferData::get_bank_number, &app::model::payouts::TrustlyBankTransferData::set_bank_number, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "trustly_bank_transfer_data"; }
};

template<>
struct serde::Serializable<app::model::payouts::Passthrough>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"psp_token", &app::model::payouts::Passthrough::get_psp_token, &app::model::payouts::Passthrough::set_psp_token, serde::FieldOptions{}>{},
            serde::FieldDesc<"psp_customer_id", &app::model::payouts::Passthrough::get_psp_customer_id, &app::model::payouts::Passthrough::set_psp_customer_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"token_type", &app::model::payouts::Passthrough::get_token_type, &app::model::payouts::Passthrough::set_token_type, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "passthrough"; }
};

template<>
struct serde::Serializable<app::model::payouts::Paypal>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"email", &app::model::payouts::Paypal::get_email, &app::model::payouts::Paypal::set_email, serde::FieldOptions{}>{},
            serde::FieldDesc<"telephone_number", &app::model::payouts::Paypal::get_telephone_number, &app::model::payouts::Paypal::set_telephone_number, serde::FieldOptions{}>{},
            serde::FieldDesc<"paypal_id", &app::model::payouts::Paypal::get_paypal_id, &app::model::payouts::Paypal::set_paypal_id, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "paypal"; }
};

template<>
struct serde::Serializable<app::model::payouts::Venmo>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"telephone_number", &app::model::payouts::Venmo::get_telephone_number, &app::model::payouts::Venmo::set_telephone_number, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "venmo"; }
};

template<>
struct serde::Serializable<app::model::payouts::ApplePayDecrypt>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"dpan", &app::model::payouts::ApplePayDecrypt::get_dpan, &app::model::payouts::ApplePayDecrypt::set_dpan, serde::FieldOptions{}>{},
            serde::FieldDesc<"expiry_month", &app::model::payouts::ApplePayDecrypt::get_expiry_month, &app::model::payouts::ApplePayDecrypt::set_expiry_month, serde::FieldOptions{}>{},
            serde::FieldDesc<"expiry_year", &app::model::payouts::ApplePayDecrypt::get_expiry_year, &app::model::payouts::ApplePayDecrypt::set_expiry_year, serde::FieldOptions{}>{},
            serde::FieldDesc<"card_holder_name", &app::model::payouts::ApplePayDecrypt::get_card_holder_name, &app::model::payouts::ApplePayDecrypt::set_card_holder_name, serde::FieldOptions{}>{},
            serde::FieldDesc<"card_network", &app::model::payouts::ApplePayDecrypt::get_card_network, &app::model::payouts::ApplePayDecrypt::set_card_network, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "apple_pay_decrypt"; }
};

template<>
struct serde::Serializable<app::model::payouts::GooglePayDecrypt>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"application_primary_account_number", &app::model::payouts::GooglePayDecrypt::get_application_primary_account_number, &app::model::payouts::GooglePayDecrypt::set_application_primary_account_number, serde::FieldOptions{}>{},
            serde::FieldDesc<"expiry_month", &app::model::payouts::GooglePayDecrypt::get_expiry_month, &app::model::payouts::GooglePayDecrypt::set_expiry_month, serde::FieldOptions{}>{},
            serde::FieldDesc<"expiry_year", &app::model::payouts::GooglePayDecrypt::get_expiry_year, &app::model::payouts::GooglePayDecrypt::set_expiry_year, serde::FieldOptions{}>{},
            serde::FieldDesc<"card_holder_name", &app::model::payouts::GooglePayDecrypt::get_card_holder_name, &app::model::payouts::GooglePayDecrypt::set_card_holder_name, serde::FieldOptions{}>{},
            serde::FieldDesc<"card_network", &app::model::payouts::GooglePayDecrypt::get_card_network, &app::model::payouts::GooglePayDecrypt::set_card_network, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "google_pay_decrypt"; }
};

template<>
struct serde::Serializable<app::model::payouts::PayoutActionRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"payout_id", &app::model::payouts::PayoutActionRequest::get_payout_id, &app::model::payouts::PayoutActionRequest::set_payout_id, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "payout_action_request"; }
};

template<>
struct serde::Serializable<app::model::payouts::PayoutRetrieveRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"payout_id", &app::model::payouts::PayoutRetrieveRequest::get_payout_id, &app::model::payouts::PayoutRetrieveRequest::set_payout_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"force_sync", &app::model::payouts::PayoutRetrieveRequest::get_force_sync, &app::model::payouts::PayoutRetrieveRequest::set_force_sync, serde::FieldOptions{}>{},
            serde::FieldDesc<"merchant_id", &app::model::payouts::PayoutRetrieveRequest::get_merchant_id, &app::model::payouts::PayoutRetrieveRequest::set_merchant_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"expand_attempts", &app::model::payouts::PayoutRetrieveRequest::get_expand_attempts, &app::model::payouts::PayoutRetrieveRequest::set_expand_attempts, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "payout_retrieve_request"; }
};

template<>
struct serde::Serializable<app::model::payouts::PayoutRetrieveBody>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"force_sync", &app::model::payouts::PayoutRetrieveBody::get_force_sync, &app::model::payouts::PayoutRetrieveBody::set_force_sync, serde::FieldOptions{}>{},
            serde::FieldDesc<"merchant_id", &app::model::payouts::PayoutRetrieveBody::get_merchant_id, &app::model::payouts::PayoutRetrieveBody::set_merchant_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"expand_attempts", &app::model::payouts::PayoutRetrieveBody::get_expand_attempts, &app::model::payouts::PayoutRetrieveBody::set_expand_attempts, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "payout_retrieve_body"; }
};

template<>
struct serde::Serializable<app::model::payouts::PayoutVendorDetails>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"account_type", &app::model::payouts::PayoutVendorDetails::get_account_type, &app::model::payouts::PayoutVendorDetails::set_account_type, serde::FieldOptions{}>{},
            serde::FieldDesc<"business_type", &app::model::payouts::PayoutVendorDetails::get_business_type, &app::model::payouts::PayoutVendorDetails::set_business_type, serde::FieldOptions{}>{},
            serde::FieldDesc<"business_profile_mcc", &app::model::payouts::PayoutVendorDetails::get_business_profile_mcc, &app::model::payouts::PayoutVendorDetails::set_business_profile_mcc, serde::FieldOptions{}>{},
            serde::FieldDesc<"business_profile_url", &app::model::payouts::PayoutVendorDetails::get_business_profile_url, &app::model::payouts::PayoutVendorDetails::set_business_profile_url, serde::FieldOptions{}>{},
            serde::FieldDesc<"business_profile_name", &app::model::payouts::PayoutVendorDetails::get_business_profile_name, &app::model::payouts::PayoutVendorDetails::set_business_profile_name, serde::FieldOptions{}>{},
            serde::FieldDesc<"company_address_line1", &app::model::payouts::PayoutVendorDetails::get_company_address_line1, &app::model::payouts::PayoutVendorDetails::set_company_address_line1, serde::FieldOptions{}>{},
            serde::FieldDesc<"company_address_line2", &app::model::payouts::PayoutVendorDetails::get_company_address_line2, &app::model::payouts::PayoutVendorDetails::set_company_address_line2, serde::FieldOptions{}>{},
            serde::FieldDesc<"company_address_postal_code", &app::model::payouts::PayoutVendorDetails::get_company_address_postal_code, &app::model::payouts::PayoutVendorDetails::set_company_address_postal_code, serde::FieldOptions{}>{},
            serde::FieldDesc<"company_address_city", &app::model::payouts::PayoutVendorDetails::get_company_address_city, &app::model::payouts::PayoutVendorDetails::set_company_address_city, serde::FieldOptions{}>{},
            serde::FieldDesc<"company_address_state", &app::model::payouts::PayoutVendorDetails::get_company_address_state, &app::model::payouts::PayoutVendorDetails::set_company_address_state, serde::FieldOptions{}>{},
            serde::FieldDesc<"company_phone", &app::model::payouts::PayoutVendorDetails::get_company_phone, &app::model::payouts::PayoutVendorDetails::set_company_phone, serde::FieldOptions{}>{},
            serde::FieldDesc<"company_tax_id", &app::model::payouts::PayoutVendorDetails::get_company_tax_id, &app::model::payouts::PayoutVendorDetails::set_company_tax_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"company_owners_provided", &app::model::payouts::PayoutVendorDetails::get_company_owners_provided, &app::model::payouts::PayoutVendorDetails::set_company_owners_provided, serde::FieldOptions{}>{},
            serde::FieldDesc<"capabilities_card_payments", &app::model::payouts::PayoutVendorDetails::get_capabilities_card_payments, &app::model::payouts::PayoutVendorDetails::set_capabilities_card_payments, serde::FieldOptions{}>{},
            serde::FieldDesc<"capabilities_transfers", &app::model::payouts::PayoutVendorDetails::get_capabilities_transfers, &app::model::payouts::PayoutVendorDetails::set_capabilities_transfers, serde::FieldOptions{}>{},
            serde::FieldDesc<"company_address_country", &app::model::payouts::PayoutVendorDetails::get_company_address_country, &app::model::payouts::PayoutVendorDetails::set_company_address_country, serde::FieldOptions{}>{},
            serde::FieldDesc<"email", &app::model::payouts::PayoutVendorDetails::get_email, &app::model::payouts::PayoutVendorDetails::set_email, serde::FieldOptions{}>{},
            serde::FieldDesc<"phone_country_code", &app::model::payouts::PayoutVendorDetails::get_phone_country_code, &app::model::payouts::PayoutVendorDetails::set_phone_country_code, serde::FieldOptions{}>{},
            serde::FieldDesc<"reference_id", &app::model::payouts::PayoutVendorDetails::get_reference_id, &app::model::payouts::PayoutVendorDetails::set_reference_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"source_of_funds", &app::model::payouts::PayoutVendorDetails::get_source_of_funds, &app::model::payouts::PayoutVendorDetails::set_source_of_funds, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "payout_vendor_details"; }
};

template<>
struct serde::Serializable<app::model::payouts::PayoutIndividualDetails>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"tos_acceptance_date", &app::model::payouts::PayoutIndividualDetails::get_tos_acceptance_date, &app::model::payouts::PayoutIndividualDetails::set_tos_acceptance_date, serde::FieldOptions{}>{},
            serde::FieldDesc<"tos_acceptance_ip", &app::model::payouts::PayoutIndividualDetails::get_tos_acceptance_ip, &app::model::payouts::PayoutIndividualDetails::set_tos_acceptance_ip, serde::FieldOptions{}>{},
            serde::FieldDesc<"individual_dob_day", &app::model::payouts::PayoutIndividualDetails::get_individual_dob_day, &app::model::payouts::PayoutIndividualDetails::set_individual_dob_day, serde::FieldOptions{}>{},
            serde::FieldDesc<"individual_dob_month", &app::model::payouts::PayoutIndividualDetails::get_individual_dob_month, &app::model::payouts::PayoutIndividualDetails::set_individual_dob_month, serde::FieldOptions{}>{},
            serde::FieldDesc<"individual_dob_year", &app::model::payouts::PayoutIndividualDetails::get_individual_dob_year, &app::model::payouts::PayoutIndividualDetails::set_individual_dob_year, serde::FieldOptions{}>{},
            serde::FieldDesc<"date_of_birth", &app::model::payouts::PayoutIndividualDetails::get_date_of_birth, &app::model::payouts::PayoutIndividualDetails::set_date_of_birth, serde::FieldOptions{}>{},
            serde::FieldDesc<"individual_id_number", &app::model::payouts::PayoutIndividualDetails::get_individual_id_number, &app::model::payouts::PayoutIndividualDetails::set_individual_id_number, serde::FieldOptions{}>{},
            serde::FieldDesc<"individual_ssn_last_4", &app::model::payouts::PayoutIndividualDetails::get_individual_ssn_last_4, &app::model::payouts::PayoutIndividualDetails::set_individual_ssn_last_4, serde::FieldOptions{}>{},
            serde::FieldDesc<"external_account_account_holder_type", &app::model::payouts::PayoutIndividualDetails::get_external_account_account_holder_type, &app::model::payouts::PayoutIndividualDetails::set_external_account_account_holder_type, serde::FieldOptions{}>{},
            serde::FieldDesc<"first_name", &app::model::payouts::PayoutIndividualDetails::get_first_name, &app::model::payouts::PayoutIndividualDetails::set_first_name, serde::FieldOptions{}>{},
            serde::FieldDesc<"last_name", &app::model::payouts::PayoutIndividualDetails::get_last_name, &app::model::payouts::PayoutIndividualDetails::set_last_name, serde::FieldOptions{}>{},
            serde::FieldDesc<"address", &app::model::payouts::PayoutIndividualDetails::get_address, &app::model::payouts::PayoutIndividualDetails::set_address, serde::FieldOptions{}>{},
            serde::FieldDesc<"reference_id", &app::model::payouts::PayoutIndividualDetails::get_reference_id, &app::model::payouts::PayoutIndividualDetails::set_reference_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"source_of_funds", &app::model::payouts::PayoutIndividualDetails::get_source_of_funds, &app::model::payouts::PayoutIndividualDetails::set_source_of_funds, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "payout_individual_details"; }
};

template<>
struct serde::Serializable<app::model::payouts::PayoutVendorAccountDetails>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"vendor_details", &app::model::payouts::PayoutVendorAccountDetails::get_vendor_details, &app::model::payouts::PayoutVendorAccountDetails::set_vendor_details, serde::FieldOptions{}>{},
            serde::FieldDesc<"individual_details", &app::model::payouts::PayoutVendorAccountDetails::get_individual_details, &app::model::payouts::PayoutVendorAccountDetails::set_individual_details, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "payout_vendor_account_details"; }
};

template<>
struct serde::Serializable<app::model::payouts::PaymentMethodTypeInfo>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"payment_method_type", &app::model::payouts::PaymentMethodTypeInfo::get_payment_method_type, &app::model::payouts::PaymentMethodTypeInfo::set_payment_method_type, serde::FieldOptions{}>{},
            serde::FieldDesc<"required_fields", &app::model::payouts::PaymentMethodTypeInfo::get_required_fields, &app::model::payouts::PaymentMethodTypeInfo::set_required_fields, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "payment_method_type_info"; }
};

template<>
struct serde::Serializable<app::model::payouts::PayoutEnabledPaymentMethodsInfo>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"payment_method", &app::model::payouts::PayoutEnabledPaymentMethodsInfo::get_payment_method, &app::model::payouts::PayoutEnabledPaymentMethodsInfo::set_payment_method, serde::FieldOptions{}>{},
            serde::FieldDesc<"payment_method_types_info", &app::model::payouts::PayoutEnabledPaymentMethodsInfo::get_payment_method_types_info, &app::model::payouts::PayoutEnabledPaymentMethodsInfo::set_payment_method_types_info, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "payout_enabled_payment_methods_info"; }
};

template<>
struct serde::Serializable<app::model::payouts::PayoutLinkDetails>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"publishable_key", &app::model::payouts::PayoutLinkDetails::get_publishable_key, &app::model::payouts::PayoutLinkDetails::set_publishable_key, serde::FieldOptions{}>{},
            serde::FieldDesc<"client_secret", &app::model::payouts::PayoutLinkDetails::get_client_secret, &app::model::payouts::PayoutLinkDetails::set_client_secret, serde::FieldOptions{}>{},
            serde::FieldDesc<"payout_link_id", &app::model::payouts::PayoutLinkDetails::get_payout_link_id, &app::model::payouts::PayoutLinkDetails::set_payout_link_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"payout_id", &app::model::payouts::PayoutLinkDetails::get_payout_id, &app::model::payouts::PayoutLinkDetails::set_payout_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"customer_id", &app::model::payouts::PayoutLinkDetails::get_customer_id, &app::model::payouts::PayoutLinkDetails::set_customer_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"session_expiry", &app::model::payouts::PayoutLinkDetails::get_session_expiry, &app::model::payouts::PayoutLinkDetails::set_session_expiry, serde::FieldOptions{}>{},
            serde::FieldDesc<"return_url", &app::model::payouts::PayoutLinkDetails::get_return_url, &app::model::payouts::PayoutLinkDetails::set_return_url, serde::FieldOptions{}>{},
            serde::FieldDesc<"ui_config", &app::model::payouts::PayoutLinkDetails::get_ui_config, &app::model::payouts::PayoutLinkDetails::set_ui_config, serde::FieldOptions{}>{},
            serde::FieldDesc<"enabled_payment_methods", &app::model::payouts::PayoutLinkDetails::get_enabled_payment_methods, &app::model::payouts::PayoutLinkDetails::set_enabled_payment_methods, serde::FieldOptions{}>{},
            serde::FieldDesc<"enabled_payment_methods_with_required_fields", &app::model::payouts::PayoutLinkDetails::get_enabled_payment_methods_with_required_fields, &app::model::payouts::PayoutLinkDetails::set_enabled_payment_methods_with_required_fields, serde::FieldOptions{}>{},
            serde::FieldDesc<"amount", &app::model::payouts::PayoutLinkDetails::get_amount, &app::model::payouts::PayoutLinkDetails::set_amount, serde::FieldOptions{}>{},
            serde::FieldDesc<"currency", &app::model::payouts::PayoutLinkDetails::get_currency, &app::model::payouts::PayoutLinkDetails::set_currency, serde::FieldOptions{}>{},
            serde::FieldDesc<"locale", &app::model::payouts::PayoutLinkDetails::get_locale, &app::model::payouts::PayoutLinkDetails::set_locale, serde::FieldOptions{}>{},
            serde::FieldDesc<"form_layout", &app::model::payouts::PayoutLinkDetails::get_form_layout, &app::model::payouts::PayoutLinkDetails::set_form_layout, serde::FieldOptions{}>{},
            serde::FieldDesc<"test_mode", &app::model::payouts::PayoutLinkDetails::get_test_mode, &app::model::payouts::PayoutLinkDetails::set_test_mode, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "payout_link_details"; }
};

template<>
struct serde::Serializable<app::model::payouts::PayoutLinkResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"payout_link_id", &app::model::payouts::PayoutLinkResponse::get_payout_link_id, &app::model::payouts::PayoutLinkResponse::set_payout_link_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"link", &app::model::payouts::PayoutLinkResponse::get_link, &app::model::payouts::PayoutLinkResponse::set_link, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "payout_link_response"; }
};

template<>
struct serde::Serializable<app::model::payouts::PayoutLinkInitiateRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"merchant_id", &app::model::payouts::PayoutLinkInitiateRequest::get_merchant_id, &app::model::payouts::PayoutLinkInitiateRequest::set_merchant_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"payout_id", &app::model::payouts::PayoutLinkInitiateRequest::get_payout_id, &app::model::payouts::PayoutLinkInitiateRequest::set_payout_id, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "payout_link_initiate_request"; }
};

template<>
struct serde::Serializable<app::model::payouts::PayoutLinkStatusDetails>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"payout_link_id", &app::model::payouts::PayoutLinkStatusDetails::get_payout_link_id, &app::model::payouts::PayoutLinkStatusDetails::set_payout_link_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"payout_id", &app::model::payouts::PayoutLinkStatusDetails::get_payout_id, &app::model::payouts::PayoutLinkStatusDetails::set_payout_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"customer_id", &app::model::payouts::PayoutLinkStatusDetails::get_customer_id, &app::model::payouts::PayoutLinkStatusDetails::set_customer_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"session_expiry", &app::model::payouts::PayoutLinkStatusDetails::get_session_expiry, &app::model::payouts::PayoutLinkStatusDetails::set_session_expiry, serde::FieldOptions{}>{},
            serde::FieldDesc<"return_url", &app::model::payouts::PayoutLinkStatusDetails::get_return_url, &app::model::payouts::PayoutLinkStatusDetails::set_return_url, serde::FieldOptions{}>{},
            serde::FieldDesc<"status", &app::model::payouts::PayoutLinkStatusDetails::get_status, &app::model::payouts::PayoutLinkStatusDetails::set_status, serde::FieldOptions{}>{},
            serde::FieldDesc<"error_code", &app::model::payouts::PayoutLinkStatusDetails::get_error_code, &app::model::payouts::PayoutLinkStatusDetails::set_error_code, serde::FieldOptions{}>{},
            serde::FieldDesc<"error_message", &app::model::payouts::PayoutLinkStatusDetails::get_error_message, &app::model::payouts::PayoutLinkStatusDetails::set_error_message, serde::FieldOptions{}>{},
            serde::FieldDesc<"ui_config", &app::model::payouts::PayoutLinkStatusDetails::get_ui_config, &app::model::payouts::PayoutLinkStatusDetails::set_ui_config, serde::FieldOptions{}>{},
            serde::FieldDesc<"test_mode", &app::model::payouts::PayoutLinkStatusDetails::get_test_mode, &app::model::payouts::PayoutLinkStatusDetails::set_test_mode, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "payout_link_status_details"; }
};

template<>
struct serde::Serializable<app::model::payouts::PayoutAttemptResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"attempt_id", &app::model::payouts::PayoutAttemptResponse::get_attempt_id, &app::model::payouts::PayoutAttemptResponse::set_attempt_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"status", &app::model::payouts::PayoutAttemptResponse::get_status, &app::model::payouts::PayoutAttemptResponse::set_status, serde::FieldOptions{}>{},
            serde::FieldDesc<"amount", &app::model::payouts::PayoutAttemptResponse::get_amount, &app::model::payouts::PayoutAttemptResponse::set_amount, serde::FieldOptions{}>{},
            serde::FieldDesc<"currency", &app::model::payouts::PayoutAttemptResponse::get_currency, &app::model::payouts::PayoutAttemptResponse::set_currency, serde::FieldOptions{}>{},
            serde::FieldDesc<"connector", &app::model::payouts::PayoutAttemptResponse::get_connector, &app::model::payouts::PayoutAttemptResponse::set_connector, serde::FieldOptions{}>{},
            serde::FieldDesc<"error_code", &app::model::payouts::PayoutAttemptResponse::get_error_code, &app::model::payouts::PayoutAttemptResponse::set_error_code, serde::FieldOptions{}>{},
            serde::FieldDesc<"error_message", &app::model::payouts::PayoutAttemptResponse::get_error_message, &app::model::payouts::PayoutAttemptResponse::set_error_message, serde::FieldOptions{}>{},
            serde::FieldDesc<"payment_method", &app::model::payouts::PayoutAttemptResponse::get_payment_method, &app::model::payouts::PayoutAttemptResponse::set_payment_method, serde::FieldOptions{}>{},
            serde::FieldDesc<"payout_method_type", &app::model::payouts::PayoutAttemptResponse::get_payout_method_type, &app::model::payouts::PayoutAttemptResponse::set_payout_method_type, serde::FieldOptions{}>{},
            serde::FieldDesc<"connector_transaction_id", &app::model::payouts::PayoutAttemptResponse::get_connector_transaction_id, &app::model::payouts::PayoutAttemptResponse::set_connector_transaction_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"cancellation_reason", &app::model::payouts::PayoutAttemptResponse::get_cancellation_reason, &app::model::payouts::PayoutAttemptResponse::set_cancellation_reason, serde::FieldOptions{}>{},
            serde::FieldDesc<"unified_code", &app::model::payouts::PayoutAttemptResponse::get_unified_code, &app::model::payouts::PayoutAttemptResponse::set_unified_code, serde::FieldOptions{}>{},
            serde::FieldDesc<"unified_message", &app::model::payouts::PayoutAttemptResponse::get_unified_message, &app::model::payouts::PayoutAttemptResponse::set_unified_message, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "payout_attempt_response"; }
};

template<>
struct serde::Serializable<app::model::payouts::PayoutCreateResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"payout_id", &app::model::payouts::PayoutCreateResponse::get_payout_id, &app::model::payouts::PayoutCreateResponse::set_payout_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"merchant_id", &app::model::payouts::PayoutCreateResponse::get_merchant_id, &app::model::payouts::PayoutCreateResponse::set_merchant_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"merchant_order_reference_id", &app::model::payouts::PayoutCreateResponse::get_merchant_order_reference_id, &app::model::payouts::PayoutCreateResponse::set_merchant_order_reference_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"amount", &app::model::payouts::PayoutCreateResponse::get_amount, &app::model::payouts::PayoutCreateResponse::set_amount, serde::FieldOptions{}>{},
            serde::FieldDesc<"currency", &app::model::payouts::PayoutCreateResponse::get_currency, &app::model::payouts::PayoutCreateResponse::set_currency, serde::FieldOptions{}>{},
            serde::FieldDesc<"connector", &app::model::payouts::PayoutCreateResponse::get_connector, &app::model::payouts::PayoutCreateResponse::set_connector, serde::FieldOptions{}>{},
            serde::FieldDesc<"payout_type", &app::model::payouts::PayoutCreateResponse::get_payout_type, &app::model::payouts::PayoutCreateResponse::set_payout_type, serde::FieldOptions{}>{},
            serde::FieldDesc<"payout_method_data", &app::model::payouts::PayoutCreateResponse::get_payout_method_data, &app::model::payouts::PayoutCreateResponse::set_payout_method_data, serde::FieldOptions{}>{},
            serde::FieldDesc<"source_bank_data", &app::model::payouts::PayoutCreateResponse::get_source_bank_data, &app::model::payouts::PayoutCreateResponse::set_source_bank_data, serde::FieldOptions{}>{},
            serde::FieldDesc<"billing", &app::model::payouts::PayoutCreateResponse::get_billing, &app::model::payouts::PayoutCreateResponse::set_billing, serde::FieldOptions{}>{},
            serde::FieldDesc<"auto_fulfill", &app::model::payouts::PayoutCreateResponse::get_auto_fulfill, &app::model::payouts::PayoutCreateResponse::set_auto_fulfill, serde::FieldOptions{}>{},
            serde::FieldDesc<"customer_id", &app::model::payouts::PayoutCreateResponse::get_customer_id, &app::model::payouts::PayoutCreateResponse::set_customer_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"customer", &app::model::payouts::PayoutCreateResponse::get_customer, &app::model::payouts::PayoutCreateResponse::set_customer, serde::FieldOptions{}>{},
            serde::FieldDesc<"client_secret", &app::model::payouts::PayoutCreateResponse::get_client_secret, &app::model::payouts::PayoutCreateResponse::set_client_secret, serde::FieldOptions{}>{},
            serde::FieldDesc<"return_url", &app::model::payouts::PayoutCreateResponse::get_return_url, &app::model::payouts::PayoutCreateResponse::set_return_url, serde::FieldOptions{}>{},
            serde::FieldDesc<"business_country", &app::model::payouts::PayoutCreateResponse::get_business_country, &app::model::payouts::PayoutCreateResponse::set_business_country, serde::FieldOptions{}>{},
            serde::FieldDesc<"business_label", &app::model::payouts::PayoutCreateResponse::get_business_label, &app::model::payouts::PayoutCreateResponse::set_business_label, serde::FieldOptions{}>{},
            serde::FieldDesc<"description", &app::model::payouts::PayoutCreateResponse::get_description, &app::model::payouts::PayoutCreateResponse::set_description, serde::FieldOptions{}>{},
            serde::FieldDesc<"billing_descriptor", &app::model::payouts::PayoutCreateResponse::get_billing_descriptor, &app::model::payouts::PayoutCreateResponse::set_billing_descriptor, serde::FieldOptions{}>{},
            serde::FieldDesc<"entity_type", &app::model::payouts::PayoutCreateResponse::get_entity_type, &app::model::payouts::PayoutCreateResponse::set_entity_type, serde::FieldOptions{}>{},
            serde::FieldDesc<"recurring", &app::model::payouts::PayoutCreateResponse::get_recurring, &app::model::payouts::PayoutCreateResponse::set_recurring, serde::FieldOptions{}>{},
            serde::FieldDesc<"metadata", &app::model::payouts::PayoutCreateResponse::get_metadata, &app::model::payouts::PayoutCreateResponse::set_metadata, serde::FieldOptions{}>{},
            serde::FieldDesc<"funds_transfer_type", &app::model::payouts::PayoutCreateResponse::get_funds_transfer_type, &app::model::payouts::PayoutCreateResponse::set_funds_transfer_type, serde::FieldOptions{}>{},
            serde::FieldDesc<"purpose", &app::model::payouts::PayoutCreateResponse::get_purpose, &app::model::payouts::PayoutCreateResponse::set_purpose, serde::FieldOptions{}>{},
            serde::FieldDesc<"sender_details", &app::model::payouts::PayoutCreateResponse::get_sender_details, &app::model::payouts::PayoutCreateResponse::set_sender_details, serde::FieldOptions{}>{},
            serde::FieldDesc<"merchant_connector_id", &app::model::payouts::PayoutCreateResponse::get_merchant_connector_id, &app::model::payouts::PayoutCreateResponse::set_merchant_connector_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"status", &app::model::payouts::PayoutCreateResponse::get_status, &app::model::payouts::PayoutCreateResponse::set_status, serde::FieldOptions{}>{},
            serde::FieldDesc<"error_message", &app::model::payouts::PayoutCreateResponse::get_error_message, &app::model::payouts::PayoutCreateResponse::set_error_message, serde::FieldOptions{}>{},
            serde::FieldDesc<"error_code", &app::model::payouts::PayoutCreateResponse::get_error_code, &app::model::payouts::PayoutCreateResponse::set_error_code, serde::FieldOptions{}>{},
            serde::FieldDesc<"frm_message", &app::model::payouts::PayoutCreateResponse::get_frm_message, &app::model::payouts::PayoutCreateResponse::set_frm_message, serde::FieldOptions{}>{},
            serde::FieldDesc<"profile_id", &app::model::payouts::PayoutCreateResponse::get_profile_id, &app::model::payouts::PayoutCreateResponse::set_profile_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"created", &app::model::payouts::PayoutCreateResponse::get_created, &app::model::payouts::PayoutCreateResponse::set_created, serde::FieldOptions{}>{},
            serde::FieldDesc<"connector_transaction_id", &app::model::payouts::PayoutCreateResponse::get_connector_transaction_id, &app::model::payouts::PayoutCreateResponse::set_connector_transaction_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"connector_eligibility_reference_id", &app::model::payouts::PayoutCreateResponse::get_connector_eligibility_reference_id, &app::model::payouts::PayoutCreateResponse::set_connector_eligibility_reference_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"priority", &app::model::payouts::PayoutCreateResponse::get_priority, &app::model::payouts::PayoutCreateResponse::set_priority, serde::FieldOptions{}>{},
            serde::FieldDesc<"attempts", &app::model::payouts::PayoutCreateResponse::get_attempts, &app::model::payouts::PayoutCreateResponse::set_attempts, serde::FieldOptions{}>{},
            serde::FieldDesc<"payout_link", &app::model::payouts::PayoutCreateResponse::get_payout_link, &app::model::payouts::PayoutCreateResponse::set_payout_link, serde::FieldOptions{}>{},
            serde::FieldDesc<"email", &app::model::payouts::PayoutCreateResponse::get_email, &app::model::payouts::PayoutCreateResponse::set_email, serde::FieldOptions{}>{},
            serde::FieldDesc<"name", &app::model::payouts::PayoutCreateResponse::get_name, &app::model::payouts::PayoutCreateResponse::set_name, serde::FieldOptions{}>{},
            serde::FieldDesc<"phone", &app::model::payouts::PayoutCreateResponse::get_phone, &app::model::payouts::PayoutCreateResponse::set_phone, serde::FieldOptions{}>{},
            serde::FieldDesc<"phone_country_code", &app::model::payouts::PayoutCreateResponse::get_phone_country_code, &app::model::payouts::PayoutCreateResponse::set_phone_country_code, serde::FieldOptions{}>{},
            serde::FieldDesc<"unified_code", &app::model::payouts::PayoutCreateResponse::get_unified_code, &app::model::payouts::PayoutCreateResponse::set_unified_code, serde::FieldOptions{}>{},
            serde::FieldDesc<"unified_message", &app::model::payouts::PayoutCreateResponse::get_unified_message, &app::model::payouts::PayoutCreateResponse::set_unified_message, serde::FieldOptions{}>{},
            serde::FieldDesc<"payout_method_id", &app::model::payouts::PayoutCreateResponse::get_payout_method_id, &app::model::payouts::PayoutCreateResponse::set_payout_method_id, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "payout_create_response"; }
};

template<>
struct serde::Serializable<app::model::payouts::PayoutCreateRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"payout_id", &app::model::payouts::PayoutCreateRequest::get_payout_id, &app::model::payouts::PayoutCreateRequest::set_payout_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"merchant_id", &app::model::payouts::PayoutCreateRequest::get_merchant_id, &app::model::payouts::PayoutCreateRequest::set_merchant_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"merchant_order_reference_id", &app::model::payouts::PayoutCreateRequest::get_merchant_order_reference_id, &app::model::payouts::PayoutCreateRequest::set_merchant_order_reference_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"amount", &app::model::payouts::PayoutCreateRequest::get_amount, &app::model::payouts::PayoutCreateRequest::set_amount, serde::FieldOptions{}>{},
            serde::FieldDesc<"currency", &app::model::payouts::PayoutCreateRequest::get_currency, &app::model::payouts::PayoutCreateRequest::set_currency, serde::FieldOptions{}>{},
            serde::FieldDesc<"routing", &app::model::payouts::PayoutCreateRequest::get_routing, &app::model::payouts::PayoutCreateRequest::set_routing, serde::FieldOptions{}>{},
            serde::FieldDesc<"connector", &app::model::payouts::PayoutCreateRequest::get_connector, &app::model::payouts::PayoutCreateRequest::set_connector, serde::FieldOptions{}>{},
            serde::FieldDesc<"confirm", &app::model::payouts::PayoutCreateRequest::get_confirm, &app::model::payouts::PayoutCreateRequest::set_confirm, serde::FieldOptions{}>{},
            serde::FieldDesc<"payout_type", &app::model::payouts::PayoutCreateRequest::get_payout_type, &app::model::payouts::PayoutCreateRequest::set_payout_type, serde::FieldOptions{}>{},
            serde::FieldDesc<"payout_method_data", &app::model::payouts::PayoutCreateRequest::get_payout_method_data, &app::model::payouts::PayoutCreateRequest::set_payout_method_data, serde::FieldOptions{}>{},
            serde::FieldDesc<"source_bank_data", &app::model::payouts::PayoutCreateRequest::get_source_bank_data, &app::model::payouts::PayoutCreateRequest::set_source_bank_data, serde::FieldOptions{}>{},
            serde::FieldDesc<"billing", &app::model::payouts::PayoutCreateRequest::get_billing, &app::model::payouts::PayoutCreateRequest::set_billing, serde::FieldOptions{}>{},
            serde::FieldDesc<"auto_fulfill", &app::model::payouts::PayoutCreateRequest::get_auto_fulfill, &app::model::payouts::PayoutCreateRequest::set_auto_fulfill, serde::FieldOptions{}>{},
            serde::FieldDesc<"customer_id", &app::model::payouts::PayoutCreateRequest::get_customer_id, &app::model::payouts::PayoutCreateRequest::set_customer_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"customer", &app::model::payouts::PayoutCreateRequest::get_customer, &app::model::payouts::PayoutCreateRequest::set_customer, serde::FieldOptions{}>{},
            serde::FieldDesc<"client_secret", &app::model::payouts::PayoutCreateRequest::get_client_secret, &app::model::payouts::PayoutCreateRequest::set_client_secret, serde::FieldOptions{}>{},
            serde::FieldDesc<"return_url", &app::model::payouts::PayoutCreateRequest::get_return_url, &app::model::payouts::PayoutCreateRequest::set_return_url, serde::FieldOptions{}>{},
            serde::FieldDesc<"business_country", &app::model::payouts::PayoutCreateRequest::get_business_country, &app::model::payouts::PayoutCreateRequest::set_business_country, serde::FieldOptions{}>{},
            serde::FieldDesc<"business_label", &app::model::payouts::PayoutCreateRequest::get_business_label, &app::model::payouts::PayoutCreateRequest::set_business_label, serde::FieldOptions{}>{},
            serde::FieldDesc<"description", &app::model::payouts::PayoutCreateRequest::get_description, &app::model::payouts::PayoutCreateRequest::set_description, serde::FieldOptions{}>{},
            serde::FieldDesc<"billing_descriptor", &app::model::payouts::PayoutCreateRequest::get_billing_descriptor, &app::model::payouts::PayoutCreateRequest::set_billing_descriptor, serde::FieldOptions{}>{},
            serde::FieldDesc<"entity_type", &app::model::payouts::PayoutCreateRequest::get_entity_type, &app::model::payouts::PayoutCreateRequest::set_entity_type, serde::FieldOptions{}>{},
            serde::FieldDesc<"recurring", &app::model::payouts::PayoutCreateRequest::get_recurring, &app::model::payouts::PayoutCreateRequest::set_recurring, serde::FieldOptions{}>{},
            serde::FieldDesc<"metadata", &app::model::payouts::PayoutCreateRequest::get_metadata, &app::model::payouts::PayoutCreateRequest::set_metadata, serde::FieldOptions{}>{},
            serde::FieldDesc<"funds_transfer_type", &app::model::payouts::PayoutCreateRequest::get_funds_transfer_type, &app::model::payouts::PayoutCreateRequest::set_funds_transfer_type, serde::FieldOptions{}>{},
            serde::FieldDesc<"purpose", &app::model::payouts::PayoutCreateRequest::get_purpose, &app::model::payouts::PayoutCreateRequest::set_purpose, serde::FieldOptions{}>{},
            serde::FieldDesc<"sender_details", &app::model::payouts::PayoutCreateRequest::get_sender_details, &app::model::payouts::PayoutCreateRequest::set_sender_details, serde::FieldOptions{}>{},
            serde::FieldDesc<"payout_token", &app::model::payouts::PayoutCreateRequest::get_payout_token, &app::model::payouts::PayoutCreateRequest::set_payout_token, serde::FieldOptions{}>{},
            serde::FieldDesc<"profile_id", &app::model::payouts::PayoutCreateRequest::get_profile_id, &app::model::payouts::PayoutCreateRequest::set_profile_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"priority", &app::model::payouts::PayoutCreateRequest::get_priority, &app::model::payouts::PayoutCreateRequest::set_priority, serde::FieldOptions{}>{},
            serde::FieldDesc<"payout_link", &app::model::payouts::PayoutCreateRequest::get_payout_link, &app::model::payouts::PayoutCreateRequest::set_payout_link, serde::FieldOptions{}>{},
            serde::FieldDesc<"payout_link_config", &app::model::payouts::PayoutCreateRequest::get_payout_link_config, &app::model::payouts::PayoutCreateRequest::set_payout_link_config, serde::FieldOptions{}>{},
            serde::FieldDesc<"session_expiry", &app::model::payouts::PayoutCreateRequest::get_session_expiry, &app::model::payouts::PayoutCreateRequest::set_session_expiry, serde::FieldOptions{}>{},
            serde::FieldDesc<"email", &app::model::payouts::PayoutCreateRequest::get_email, &app::model::payouts::PayoutCreateRequest::set_email, serde::FieldOptions{}>{},
            serde::FieldDesc<"name", &app::model::payouts::PayoutCreateRequest::get_name, &app::model::payouts::PayoutCreateRequest::set_name, serde::FieldOptions{}>{},
            serde::FieldDesc<"phone", &app::model::payouts::PayoutCreateRequest::get_phone, &app::model::payouts::PayoutCreateRequest::set_phone, serde::FieldOptions{}>{},
            serde::FieldDesc<"phone_country_code", &app::model::payouts::PayoutCreateRequest::get_phone_country_code, &app::model::payouts::PayoutCreateRequest::set_phone_country_code, serde::FieldOptions{}>{},
            serde::FieldDesc<"payout_method_id", &app::model::payouts::PayoutCreateRequest::get_payout_method_id, &app::model::payouts::PayoutCreateRequest::set_payout_method_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"browser_info", &app::model::payouts::PayoutCreateRequest::get_browser_info, &app::model::payouts::PayoutCreateRequest::set_browser_info, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "payout_create_request"; }
};

template<>
struct serde::Serializable<app::model::payouts::PayoutListConstraints>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"customer_id", &app::model::payouts::PayoutListConstraints::get_customer_id, &app::model::payouts::PayoutListConstraints::set_customer_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"starting_after", &app::model::payouts::PayoutListConstraints::get_starting_after, &app::model::payouts::PayoutListConstraints::set_starting_after, serde::FieldOptions{}>{},
            serde::FieldDesc<"ending_before", &app::model::payouts::PayoutListConstraints::get_ending_before, &app::model::payouts::PayoutListConstraints::set_ending_before, serde::FieldOptions{}>{},
            serde::FieldDesc<"limit", &app::model::payouts::PayoutListConstraints::get_limit, &app::model::payouts::PayoutListConstraints::set_limit, serde::FieldOptions{}>{},
            serde::FieldDesc<"created", &app::model::payouts::PayoutListConstraints::get_created, &app::model::payouts::PayoutListConstraints::set_created, serde::FieldOptions{}>{},
            serde::FieldDesc<"time_range", &app::model::payouts::PayoutListConstraints::get_time_range, &app::model::payouts::PayoutListConstraints::set_time_range, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "payout_list_constraints"; }
};

template<>
struct serde::Serializable<app::model::payouts::PayoutListFilterConstraints>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"payout_id", &app::model::payouts::PayoutListFilterConstraints::get_payout_id, &app::model::payouts::PayoutListFilterConstraints::set_payout_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"merchant_order_reference_id", &app::model::payouts::PayoutListFilterConstraints::get_merchant_order_reference_id, &app::model::payouts::PayoutListFilterConstraints::set_merchant_order_reference_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"profile_id", &app::model::payouts::PayoutListFilterConstraints::get_profile_id, &app::model::payouts::PayoutListFilterConstraints::set_profile_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"customer_id", &app::model::payouts::PayoutListFilterConstraints::get_customer_id, &app::model::payouts::PayoutListFilterConstraints::set_customer_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"limit", &app::model::payouts::PayoutListFilterConstraints::get_limit, &app::model::payouts::PayoutListFilterConstraints::set_limit, serde::FieldOptions{}>{},
            serde::FieldDesc<"offset", &app::model::payouts::PayoutListFilterConstraints::get_offset, &app::model::payouts::PayoutListFilterConstraints::set_offset, serde::FieldOptions{}>{},
            serde::FieldDesc<"time_range", &app::model::payouts::PayoutListFilterConstraints::get_time_range, &app::model::payouts::PayoutListFilterConstraints::set_time_range, serde::FieldOptions{}>{},
            serde::FieldDesc<"connector", &app::model::payouts::PayoutListFilterConstraints::get_connector, &app::model::payouts::PayoutListFilterConstraints::set_connector, serde::FieldOptions{}>{},
            serde::FieldDesc<"currency", &app::model::payouts::PayoutListFilterConstraints::get_currency, &app::model::payouts::PayoutListFilterConstraints::set_currency, serde::FieldOptions{}>{},
            serde::FieldDesc<"status", &app::model::payouts::PayoutListFilterConstraints::get_status, &app::model::payouts::PayoutListFilterConstraints::set_status, serde::FieldOptions{}>{},
            serde::FieldDesc<"payout_method", &app::model::payouts::PayoutListFilterConstraints::get_payout_method, &app::model::payouts::PayoutListFilterConstraints::set_payout_method, serde::FieldOptions{}>{},
            serde::FieldDesc<"entity_type", &app::model::payouts::PayoutListFilterConstraints::get_entity_type, &app::model::payouts::PayoutListFilterConstraints::set_entity_type, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "payout_list_filter_constraints"; }
};

template<>
struct serde::Serializable<app::model::payouts::PayoutListResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"size", &app::model::payouts::PayoutListResponse::get_size, &app::model::payouts::PayoutListResponse::set_size, serde::FieldOptions{}>{},
            serde::FieldDesc<"data", &app::model::payouts::PayoutListResponse::get_data, &app::model::payouts::PayoutListResponse::set_data, serde::FieldOptions{}>{},
            serde::FieldDesc<"total_count", &app::model::payouts::PayoutListResponse::get_total_count, &app::model::payouts::PayoutListResponse::set_total_count, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "payout_list_response"; }
};

template<>
struct serde::Serializable<app::model::payouts::PayoutListFilters>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"connector", &app::model::payouts::PayoutListFilters::get_connector, &app::model::payouts::PayoutListFilters::set_connector, serde::FieldOptions{}>{},
            serde::FieldDesc<"currency", &app::model::payouts::PayoutListFilters::get_currency, &app::model::payouts::PayoutListFilters::set_currency, serde::FieldOptions{}>{},
            serde::FieldDesc<"status", &app::model::payouts::PayoutListFilters::get_status, &app::model::payouts::PayoutListFilters::set_status, serde::FieldOptions{}>{},
            serde::FieldDesc<"payout_method", &app::model::payouts::PayoutListFilters::get_payout_method, &app::model::payouts::PayoutListFilters::set_payout_method, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "payout_list_filters"; }
};

template<>
struct serde::Serializable<app::model::payouts::MerchantConnectorInfo>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"connector_label", &app::model::payouts::MerchantConnectorInfo::get_connector_label, &app::model::payouts::MerchantConnectorInfo::set_connector_label, serde::FieldOptions{}>{},
            serde::FieldDesc<"merchant_connector_id", &app::model::payouts::MerchantConnectorInfo::get_merchant_connector_id, &app::model::payouts::MerchantConnectorInfo::set_merchant_connector_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"connector_type", &app::model::payouts::MerchantConnectorInfo::get_connector_type, &app::model::payouts::MerchantConnectorInfo::set_connector_type, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "merchant_connector_info"; }
};

template<>
struct serde::Serializable<app::model::payouts::PayoutListFiltersV2>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"connector", &app::model::payouts::PayoutListFiltersV2::get_connector, &app::model::payouts::PayoutListFiltersV2::set_connector, serde::FieldOptions{}>{},
            serde::FieldDesc<"currency", &app::model::payouts::PayoutListFiltersV2::get_currency, &app::model::payouts::PayoutListFiltersV2::set_currency, serde::FieldOptions{}>{},
            serde::FieldDesc<"status", &app::model::payouts::PayoutListFiltersV2::get_status, &app::model::payouts::PayoutListFiltersV2::set_status, serde::FieldOptions{}>{},
            serde::FieldDesc<"payout_method", &app::model::payouts::PayoutListFiltersV2::get_payout_method, &app::model::payouts::PayoutListFiltersV2::set_payout_method, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "payout_list_filters_v2"; }
};

template<>
struct serde::Serializable<app::model::payouts::RequiredFieldsOverrideRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"billing", &app::model::payouts::RequiredFieldsOverrideRequest::get_billing, &app::model::payouts::RequiredFieldsOverrideRequest::set_billing, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "required_fields_override_request"; }
};

template<>
struct serde::Serializable<app::model::payouts::PayoutsAggregateResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"status_with_count", &app::model::payouts::PayoutsAggregateResponse::get_status_with_count, &app::model::payouts::PayoutsAggregateResponse::set_status_with_count, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "payouts_aggregate_response"; }
};

template<>
struct serde::Serializable<app::model::payouts::PayoutsManualUpdateRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"payout_id", &app::model::payouts::PayoutsManualUpdateRequest::get_payout_id, &app::model::payouts::PayoutsManualUpdateRequest::set_payout_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"payout_attempt_id", &app::model::payouts::PayoutsManualUpdateRequest::get_payout_attempt_id, &app::model::payouts::PayoutsManualUpdateRequest::set_payout_attempt_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"merchant_id", &app::model::payouts::PayoutsManualUpdateRequest::get_merchant_id, &app::model::payouts::PayoutsManualUpdateRequest::set_merchant_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"status", &app::model::payouts::PayoutsManualUpdateRequest::get_status, &app::model::payouts::PayoutsManualUpdateRequest::set_status, serde::FieldOptions{}>{},
            serde::FieldDesc<"error_code", &app::model::payouts::PayoutsManualUpdateRequest::get_error_code, &app::model::payouts::PayoutsManualUpdateRequest::set_error_code, serde::FieldOptions{}>{},
            serde::FieldDesc<"error_message", &app::model::payouts::PayoutsManualUpdateRequest::get_error_message, &app::model::payouts::PayoutsManualUpdateRequest::set_error_message, serde::FieldOptions{}>{},
            serde::FieldDesc<"connector_payout_id", &app::model::payouts::PayoutsManualUpdateRequest::get_connector_payout_id, &app::model::payouts::PayoutsManualUpdateRequest::set_connector_payout_id, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "payouts_manual_update_request"; }
};

template<>
struct serde::Serializable<app::model::payouts::PayoutsManualUpdateResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"payout_id", &app::model::payouts::PayoutsManualUpdateResponse::get_payout_id, &app::model::payouts::PayoutsManualUpdateResponse::set_payout_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"payout_attempt_id", &app::model::payouts::PayoutsManualUpdateResponse::get_payout_attempt_id, &app::model::payouts::PayoutsManualUpdateResponse::set_payout_attempt_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"merchant_id", &app::model::payouts::PayoutsManualUpdateResponse::get_merchant_id, &app::model::payouts::PayoutsManualUpdateResponse::set_merchant_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"attempt_status", &app::model::payouts::PayoutsManualUpdateResponse::get_attempt_status, &app::model::payouts::PayoutsManualUpdateResponse::set_attempt_status, serde::FieldOptions{}>{},
            serde::FieldDesc<"error_code", &app::model::payouts::PayoutsManualUpdateResponse::get_error_code, &app::model::payouts::PayoutsManualUpdateResponse::set_error_code, serde::FieldOptions{}>{},
            serde::FieldDesc<"error_message", &app::model::payouts::PayoutsManualUpdateResponse::get_error_message, &app::model::payouts::PayoutsManualUpdateResponse::set_error_message, serde::FieldOptions{}>{},
            serde::FieldDesc<"connector_payout_id", &app::model::payouts::PayoutsManualUpdateResponse::get_connector_payout_id, &app::model::payouts::PayoutsManualUpdateResponse::set_connector_payout_id, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "payouts_manual_update_response"; }
};

#ifdef CONGELADO_TEST
namespace app::model::payouts::tests {
using namespace boost::ut;
suite<"app_model_payouts"> app_model_payouts_suite = [] {
    "reflects converted types"_test = [] {
        expect(std::tuple_size_v<decltype(serde::Serializable<app::model::payouts::PayoutCreateRequest>::fields())> > 0);
    };
};
}
#endif
