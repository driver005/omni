module;
export module app_dto:authentication;

import std;
import core_serde;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace app::dto::authentication {

class AcquirerDetails
{
public:
    void set_acquirer_bin(std::optional<std::string> value) { acquirer_bin = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_acquirer_bin() const noexcept { return acquirer_bin; }
    void set_acquirer_merchant_id(std::optional<std::string> value) { acquirer_merchant_id = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_acquirer_merchant_id() const noexcept { return acquirer_merchant_id; }
    void set_merchant_country_code(std::optional<std::string> value) { merchant_country_code = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_merchant_country_code() const noexcept { return merchant_country_code; }
    std::optional<std::string> acquirer_bin{};
    std::optional<std::string> acquirer_merchant_id{};
    std::optional<std::string> merchant_country_code{};
};

class AddressValue
{
public:
    void set_value(std::string v) { value = std::move(v); }
    [[nodiscard]] const std::string& get_value() const noexcept { return value; }
    std::string value{};
};

class AuthenticationConnectorsValue
{
public:
    void set_value(std::string v) { value = std::move(v); }
    [[nodiscard]] const std::string& get_value() const noexcept { return value; }
    std::string value{};
};

enum class AuthenticationEligibilityCheckData
{
    ClickToPay,
};

enum class AuthenticationEligibilityCheckResponseData
{
    ClickToPayEnrollmentStatus,
};

class AuthenticationIdValue
{
public:
    void set_value(std::string v) { value = std::move(v); }
    [[nodiscard]] const std::string& get_value() const noexcept { return value; }
    std::string value{};
};

enum class AuthenticationPaymentMethodDataResponse
{
    CardData,
    NetworkTokenData,
};

enum class AuthenticationPaymentMethodType
{
    ClickToPay,
};

enum class AuthenticationSdkNextAction
{
    AwaitMerchantCallback,
    Deny,
    Proceed,
};

enum class AuthenticationSessionToken
{
    ClickToPay,
    NoSessionTokenReceived,
};

class AuthenticationStatusValue
{
public:
    void set_value(std::string v) { value = std::move(v); }
    [[nodiscard]] const std::string& get_value() const noexcept { return value; }
    std::string value{};
};

enum class AuthenticationVaultTokenData
{
    CardData,
    NetworkTokenData,
};

class BrowserInformationValue
{
public:
    void set_value(std::string v) { value = std::move(v); }
    [[nodiscard]] const std::string& get_value() const noexcept { return value; }
    std::string value{};
};

class ClickToPayEligibilityCheckResponseData
{
public:
    void set_visa(std::optional<bool> value) { visa = std::move(value); }
    [[nodiscard]] const std::optional<bool>& get_visa() const noexcept { return visa; }
    void set_mastercard(std::optional<bool> value) { mastercard = std::move(value); }
    [[nodiscard]] const std::optional<bool>& get_mastercard() const noexcept { return mastercard; }
    std::optional<bool> visa{};
    std::optional<bool> mastercard{};
};

enum class Cryptogram
{
    Cavv,
};

class CtpServiceProviderValue
{
public:
    void set_value(std::string v) { value = std::move(v); }
    [[nodiscard]] const std::string& get_value() const noexcept { return value; }
    std::string value{};
};

class CurrencyValue
{
public:
    void set_value(std::string v) { value = std::move(v); }
    [[nodiscard]] const std::string& get_value() const noexcept { return value; }
    std::string value{};
};

class CustomerDetailsValue
{
public:
    void set_value(std::string v) { value = std::move(v); }
    [[nodiscard]] const std::string& get_value() const noexcept { return value; }
    std::string value{};
};

class DeviceChannelValue
{
public:
    void set_value(std::string v) { value = std::move(v); }
    [[nodiscard]] const std::string& get_value() const noexcept { return value; }
    std::string value{};
};

class EmailValue
{
public:
    void set_value(std::string v) { value = std::move(v); }
    [[nodiscard]] const std::string& get_value() const noexcept { return value; }
    std::string value{};
};

class MasterCardEligibilityCheckData
{
public:
    void set_consumer_present(bool value) { consumer_present = std::move(value); }
    [[nodiscard]] const bool& get_consumer_present() const noexcept { return consumer_present; }
    void set_id_lookup_session_id(std::optional<std::string> value) { id_lookup_session_id = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_id_lookup_session_id() const noexcept { return id_lookup_session_id; }
    void set_last_used_card_timestamp(std::optional<std::string> value) { last_used_card_timestamp = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_last_used_card_timestamp() const noexcept { return last_used_card_timestamp; }
    bool consumer_present{};
    std::optional<std::string> id_lookup_session_id{};
    std::optional<std::string> last_used_card_timestamp{};
};

class MerchantIdValue
{
public:
    void set_value(std::string v) { value = std::move(v); }
    [[nodiscard]] const std::string& get_value() const noexcept { return value; }
    std::string value{};
};

class MethodValue
{
public:
    void set_value(std::string v) { value = std::move(v); }
    [[nodiscard]] const std::string& get_value() const noexcept { return value; }
    std::string value{};
};

class MinorUnitValue
{
public:
    void set_value(std::string v) { value = std::move(v); }
    [[nodiscard]] const std::string& get_value() const noexcept { return value; }
    std::string value{};
};

class OptionalEncryptableEmailValue
{
public:
    void set_value(std::string v) { value = std::move(v); }
    [[nodiscard]] const std::string& get_value() const noexcept { return value; }
    std::string value{};
};

class PaymentMethodDataValue
{
public:
    void set_value(std::string v) { value = std::move(v); }
    [[nodiscard]] const std::string& get_value() const noexcept { return value; }
    std::string value{};
};

class PaymentMethodTypeValue
{
public:
    void set_value(std::string v) { value = std::move(v); }
    [[nodiscard]] const std::string& get_value() const noexcept { return value; }
    std::string value{};
};

class PaymentMethodValue
{
public:
    void set_value(std::string v) { value = std::move(v); }
    [[nodiscard]] const std::string& get_value() const noexcept { return value; }
    std::string value{};
};

class PrimitiveDateTimeValue
{
public:
    void set_value(std::string v) { value = std::move(v); }
    [[nodiscard]] const std::string& get_value() const noexcept { return value; }
    std::string value{};
};

class ProfileAcquirerIdValue
{
public:
    void set_value(std::string v) { value = std::move(v); }
    [[nodiscard]] const std::string& get_value() const noexcept { return value; }
    std::string value{};
};

class ProfileIdValue
{
public:
    void set_value(std::string v) { value = std::move(v); }
    [[nodiscard]] const std::string& get_value() const noexcept { return value; }
    std::string value{};
};

class ScaExemptionTypeValue
{
public:
    void set_value(std::string v) { value = std::move(v); }
    [[nodiscard]] const std::string& get_value() const noexcept { return value; }
    std::string value{};
};

class SdkInformationValue
{
public:
    void set_value(std::string v) { value = std::move(v); }
    [[nodiscard]] const std::string& get_value() const noexcept { return value; }
    std::string value{};
};

class SecretSerdeValueValue
{
public:
    void set_value(std::string v) { value = std::move(v); }
    [[nodiscard]] const std::string& get_value() const noexcept { return value; }
    std::string value{};
};

class SemanticVersionValue
{
public:
    void set_value(std::string v) { value = std::move(v); }
    [[nodiscard]] const std::string& get_value() const noexcept { return value; }
    std::string value{};
};

class ThreeDsCompletionIndicatorValue
{
public:
    void set_value(std::string v) { value = std::move(v); }
    [[nodiscard]] const std::string& get_value() const noexcept { return value; }
    std::string value{};
};

class TransactionStatusValue
{
public:
    void set_value(std::string v) { value = std::move(v); }
    [[nodiscard]] const std::string& get_value() const noexcept { return value; }
    std::string value{};
};

class UrlValue
{
public:
    void set_value(std::string v) { value = std::move(v); }
    [[nodiscard]] const std::string& get_value() const noexcept { return value; }
    std::string value{};
};

class AuthenticationAuthenticateRequest
{
public:
    void set_authentication_id(AuthenticationIdValue value) { authentication_id = std::move(value); }
    [[nodiscard]] const AuthenticationIdValue& get_authentication_id() const noexcept { return authentication_id; }
    void set_client_secret(std::optional<std::string> value) { client_secret = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_client_secret() const noexcept { return client_secret; }
    void set_sdk_information(std::optional<SdkInformationValue> value) { sdk_information = std::move(value); }
    [[nodiscard]] const std::optional<SdkInformationValue>& get_sdk_information() const noexcept { return sdk_information; }
    void set_device_channel(DeviceChannelValue value) { device_channel = std::move(value); }
    [[nodiscard]] const DeviceChannelValue& get_device_channel() const noexcept { return device_channel; }
    void set_threeds_method_comp_ind(ThreeDsCompletionIndicatorValue value) { threeds_method_comp_ind = std::move(value); }
    [[nodiscard]] const ThreeDsCompletionIndicatorValue& get_threeds_method_comp_ind() const noexcept { return threeds_method_comp_ind; }
    AuthenticationIdValue authentication_id{};
    std::optional<std::string> client_secret{};
    std::optional<SdkInformationValue> sdk_information{};
    DeviceChannelValue device_channel{};
    ThreeDsCompletionIndicatorValue threeds_method_comp_ind{};
};

class AuthenticationAuthenticateResponse
{
public:
    void set_transaction_status(std::optional<TransactionStatusValue> value) { transaction_status = std::move(value); }
    [[nodiscard]] const std::optional<TransactionStatusValue>& get_transaction_status() const noexcept { return transaction_status; }
    void set_acs_url(std::optional<UrlValue> value) { acs_url = std::move(value); }
    [[nodiscard]] const std::optional<UrlValue>& get_acs_url() const noexcept { return acs_url; }
    void set_challenge_request(std::optional<std::string> value) { challenge_request = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_challenge_request() const noexcept { return challenge_request; }
    void set_acs_reference_number(std::optional<std::string> value) { acs_reference_number = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_acs_reference_number() const noexcept { return acs_reference_number; }
    void set_acs_trans_id(std::optional<std::string> value) { acs_trans_id = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_acs_trans_id() const noexcept { return acs_trans_id; }
    void set_three_ds_server_transaction_id(std::optional<std::string> value) { three_ds_server_transaction_id = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_three_ds_server_transaction_id() const noexcept { return three_ds_server_transaction_id; }
    void set_acs_signed_content(std::optional<std::string> value) { acs_signed_content = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_acs_signed_content() const noexcept { return acs_signed_content; }
    void set_three_ds_requestor_url(std::string value) { three_ds_requestor_url = std::move(value); }
    [[nodiscard]] const std::string& get_three_ds_requestor_url() const noexcept { return three_ds_requestor_url; }
    void set_three_ds_requestor_app_url(std::optional<std::string> value) { three_ds_requestor_app_url = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_three_ds_requestor_app_url() const noexcept { return three_ds_requestor_app_url; }
    void set_error_message(std::optional<std::string> value) { error_message = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_error_message() const noexcept { return error_message; }
    void set_error_code(std::optional<std::string> value) { error_code = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_error_code() const noexcept { return error_code; }
    void set_authentication_value(std::optional<std::string> value) { authentication_value = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_authentication_value() const noexcept { return authentication_value; }
    void set_status(AuthenticationStatusValue value) { status = std::move(value); }
    [[nodiscard]] const AuthenticationStatusValue& get_status() const noexcept { return status; }
    void set_authentication_connector(std::optional<AuthenticationConnectorsValue> value) { authentication_connector = std::move(value); }
    [[nodiscard]] const std::optional<AuthenticationConnectorsValue>& get_authentication_connector() const noexcept { return authentication_connector; }
    void set_authentication_id(AuthenticationIdValue value) { authentication_id = std::move(value); }
    [[nodiscard]] const AuthenticationIdValue& get_authentication_id() const noexcept { return authentication_id; }
    void set_eci(std::optional<std::string> value) { eci = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_eci() const noexcept { return eci; }
    void set_acquirer_details(std::optional<AcquirerDetails> value) { acquirer_details = std::move(value); }
    [[nodiscard]] const std::optional<AcquirerDetails>& get_acquirer_details() const noexcept { return acquirer_details; }
    std::optional<TransactionStatusValue> transaction_status{};
    std::optional<UrlValue> acs_url{};
    std::optional<std::string> challenge_request{};
    std::optional<std::string> acs_reference_number{};
    std::optional<std::string> acs_trans_id{};
    std::optional<std::string> three_ds_server_transaction_id{};
    std::optional<std::string> acs_signed_content{};
    std::string three_ds_requestor_url{};
    std::optional<std::string> three_ds_requestor_app_url{};
    std::optional<std::string> error_message{};
    std::optional<std::string> error_code{};
    std::optional<std::string> authentication_value{};
    AuthenticationStatusValue status{};
    std::optional<AuthenticationConnectorsValue> authentication_connector{};
    AuthenticationIdValue authentication_id{};
    std::optional<std::string> eci{};
    std::optional<AcquirerDetails> acquirer_details{};
};

class AuthenticationCreateRequest
{
public:
    void set_authentication_id(std::optional<AuthenticationIdValue> value) { authentication_id = std::move(value); }
    [[nodiscard]] const std::optional<AuthenticationIdValue>& get_authentication_id() const noexcept { return authentication_id; }
    void set_profile_id(std::optional<ProfileIdValue> value) { profile_id = std::move(value); }
    [[nodiscard]] const std::optional<ProfileIdValue>& get_profile_id() const noexcept { return profile_id; }
    void set_amount(MinorUnitValue value) { amount = std::move(value); }
    [[nodiscard]] const MinorUnitValue& get_amount() const noexcept { return amount; }
    void set_authentication_connector(std::optional<AuthenticationConnectorsValue> value) { authentication_connector = std::move(value); }
    [[nodiscard]] const std::optional<AuthenticationConnectorsValue>& get_authentication_connector() const noexcept { return authentication_connector; }
    void set_currency(CurrencyValue value) { currency = std::move(value); }
    [[nodiscard]] const CurrencyValue& get_currency() const noexcept { return currency; }
    void set_return_url(std::optional<std::string> value) { return_url = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_return_url() const noexcept { return return_url; }
    void set_force_3ds_challenge(std::optional<bool> value) { force_3ds_challenge = std::move(value); }
    [[nodiscard]] const std::optional<bool>& get_force_3ds_challenge() const noexcept { return force_3ds_challenge; }
    void set_psd2_sca_exemption_type(std::optional<ScaExemptionTypeValue> value) { psd2_sca_exemption_type = std::move(value); }
    [[nodiscard]] const std::optional<ScaExemptionTypeValue>& get_psd2_sca_exemption_type() const noexcept { return psd2_sca_exemption_type; }
    void set_profile_acquirer_id(std::optional<ProfileAcquirerIdValue> value) { profile_acquirer_id = std::move(value); }
    [[nodiscard]] const std::optional<ProfileAcquirerIdValue>& get_profile_acquirer_id() const noexcept { return profile_acquirer_id; }
    void set_acquirer_details(std::optional<AcquirerDetails> value) { acquirer_details = std::move(value); }
    [[nodiscard]] const std::optional<AcquirerDetails>& get_acquirer_details() const noexcept { return acquirer_details; }
    void set_customer_details(std::optional<CustomerDetailsValue> value) { customer_details = std::move(value); }
    [[nodiscard]] const std::optional<CustomerDetailsValue>& get_customer_details() const noexcept { return customer_details; }
    std::optional<AuthenticationIdValue> authentication_id{};
    std::optional<ProfileIdValue> profile_id{};
    MinorUnitValue amount{};
    std::optional<AuthenticationConnectorsValue> authentication_connector{};
    CurrencyValue currency{};
    std::optional<std::string> return_url{};
    std::optional<bool> force_3ds_challenge{};
    std::optional<ScaExemptionTypeValue> psd2_sca_exemption_type{};
    std::optional<ProfileAcquirerIdValue> profile_acquirer_id{};
    std::optional<AcquirerDetails> acquirer_details{};
    std::optional<CustomerDetailsValue> customer_details{};
};

class AuthenticationEligibilityCheckRequest
{
public:
    void set_authentication_id(AuthenticationIdValue value) { authentication_id = std::move(value); }
    [[nodiscard]] const AuthenticationIdValue& get_authentication_id() const noexcept { return authentication_id; }
    void set_client_secret(std::optional<std::string> value) { client_secret = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_client_secret() const noexcept { return client_secret; }
    void set_eligibility_check_data(AuthenticationEligibilityCheckData value) { eligibility_check_data = std::move(value); }
    [[nodiscard]] const AuthenticationEligibilityCheckData& get_eligibility_check_data() const noexcept { return eligibility_check_data; }
    AuthenticationIdValue authentication_id{};
    std::optional<std::string> client_secret{};
    AuthenticationEligibilityCheckData eligibility_check_data{};
};

class AuthenticationEligibilityCheckResponse
{
public:
    void set_authentication_id(AuthenticationIdValue value) { authentication_id = std::move(value); }
    [[nodiscard]] const AuthenticationIdValue& get_authentication_id() const noexcept { return authentication_id; }
    void set_sdk_next_action(AuthenticationSdkNextAction value) { sdk_next_action = std::move(value); }
    [[nodiscard]] const AuthenticationSdkNextAction& get_sdk_next_action() const noexcept { return sdk_next_action; }
    AuthenticationIdValue authentication_id{};
    AuthenticationSdkNextAction sdk_next_action{};
};

class AuthenticationEligibilityRequest
{
public:
    void set_payment_method_data(PaymentMethodDataValue value) { payment_method_data = std::move(value); }
    [[nodiscard]] const PaymentMethodDataValue& get_payment_method_data() const noexcept { return payment_method_data; }
    void set_payment_method(PaymentMethodValue value) { payment_method = std::move(value); }
    [[nodiscard]] const PaymentMethodValue& get_payment_method() const noexcept { return payment_method; }
    void set_payment_method_type(std::optional<PaymentMethodTypeValue> value) { payment_method_type = std::move(value); }
    [[nodiscard]] const std::optional<PaymentMethodTypeValue>& get_payment_method_type() const noexcept { return payment_method_type; }
    void set_client_secret(std::optional<std::string> value) { client_secret = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_client_secret() const noexcept { return client_secret; }
    void set_profile_id(std::optional<ProfileIdValue> value) { profile_id = std::move(value); }
    [[nodiscard]] const std::optional<ProfileIdValue>& get_profile_id() const noexcept { return profile_id; }
    void set_billing(std::optional<AddressValue> value) { billing = std::move(value); }
    [[nodiscard]] const std::optional<AddressValue>& get_billing() const noexcept { return billing; }
    void set_shipping(std::optional<AddressValue> value) { shipping = std::move(value); }
    [[nodiscard]] const std::optional<AddressValue>& get_shipping() const noexcept { return shipping; }
    void set_browser_information(std::optional<BrowserInformationValue> value) { browser_information = std::move(value); }
    [[nodiscard]] const std::optional<BrowserInformationValue>& get_browser_information() const noexcept { return browser_information; }
    void set_email(std::optional<EmailValue> value) { email = std::move(value); }
    [[nodiscard]] const std::optional<EmailValue>& get_email() const noexcept { return email; }
    PaymentMethodDataValue payment_method_data{};
    PaymentMethodValue payment_method{};
    std::optional<PaymentMethodTypeValue> payment_method_type{};
    std::optional<std::string> client_secret{};
    std::optional<ProfileIdValue> profile_id{};
    std::optional<AddressValue> billing{};
    std::optional<AddressValue> shipping{};
    std::optional<BrowserInformationValue> browser_information{};
    std::optional<EmailValue> email{};
};

class AuthenticationResponse
{
public:
    void set_authentication_id(AuthenticationIdValue value) { authentication_id = std::move(value); }
    [[nodiscard]] const AuthenticationIdValue& get_authentication_id() const noexcept { return authentication_id; }
    void set_merchant_id(MerchantIdValue value) { merchant_id = std::move(value); }
    [[nodiscard]] const MerchantIdValue& get_merchant_id() const noexcept { return merchant_id; }
    void set_status(AuthenticationStatusValue value) { status = std::move(value); }
    [[nodiscard]] const AuthenticationStatusValue& get_status() const noexcept { return status; }
    void set_client_secret(std::optional<std::string> value) { client_secret = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_client_secret() const noexcept { return client_secret; }
    void set_amount(MinorUnitValue value) { amount = std::move(value); }
    [[nodiscard]] const MinorUnitValue& get_amount() const noexcept { return amount; }
    void set_currency(CurrencyValue value) { currency = std::move(value); }
    [[nodiscard]] const CurrencyValue& get_currency() const noexcept { return currency; }
    void set_authentication_connector(std::optional<AuthenticationConnectorsValue> value) { authentication_connector = std::move(value); }
    [[nodiscard]] const std::optional<AuthenticationConnectorsValue>& get_authentication_connector() const noexcept { return authentication_connector; }
    void set_force_3ds_challenge(std::optional<bool> value) { force_3ds_challenge = std::move(value); }
    [[nodiscard]] const std::optional<bool>& get_force_3ds_challenge() const noexcept { return force_3ds_challenge; }
    void set_return_url(std::optional<std::string> value) { return_url = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_return_url() const noexcept { return return_url; }
    void set_created_at(std::optional<PrimitiveDateTimeValue> value) { created_at = std::move(value); }
    [[nodiscard]] const std::optional<PrimitiveDateTimeValue>& get_created_at() const noexcept { return created_at; }
    void set_error_code(std::optional<std::string> value) { error_code = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_error_code() const noexcept { return error_code; }
    void set_error_message(std::optional<std::string> value) { error_message = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_error_message() const noexcept { return error_message; }
    void set_profile_id(std::optional<ProfileIdValue> value) { profile_id = std::move(value); }
    [[nodiscard]] const std::optional<ProfileIdValue>& get_profile_id() const noexcept { return profile_id; }
    void set_psd2_sca_exemption_type(std::optional<ScaExemptionTypeValue> value) { psd2_sca_exemption_type = std::move(value); }
    [[nodiscard]] const std::optional<ScaExemptionTypeValue>& get_psd2_sca_exemption_type() const noexcept { return psd2_sca_exemption_type; }
    void set_acquirer_details(std::optional<AcquirerDetails> value) { acquirer_details = std::move(value); }
    [[nodiscard]] const std::optional<AcquirerDetails>& get_acquirer_details() const noexcept { return acquirer_details; }
    void set_profile_acquirer_id(std::optional<ProfileAcquirerIdValue> value) { profile_acquirer_id = std::move(value); }
    [[nodiscard]] const std::optional<ProfileAcquirerIdValue>& get_profile_acquirer_id() const noexcept { return profile_acquirer_id; }
    void set_customer_details(std::optional<CustomerDetailsValue> value) { customer_details = std::move(value); }
    [[nodiscard]] const std::optional<CustomerDetailsValue>& get_customer_details() const noexcept { return customer_details; }
    AuthenticationIdValue authentication_id{};
    MerchantIdValue merchant_id{};
    AuthenticationStatusValue status{};
    std::optional<std::string> client_secret{};
    MinorUnitValue amount{};
    CurrencyValue currency{};
    std::optional<AuthenticationConnectorsValue> authentication_connector{};
    std::optional<bool> force_3ds_challenge{};
    std::optional<std::string> return_url{};
    std::optional<PrimitiveDateTimeValue> created_at{};
    std::optional<std::string> error_code{};
    std::optional<std::string> error_message{};
    std::optional<ProfileIdValue> profile_id{};
    std::optional<ScaExemptionTypeValue> psd2_sca_exemption_type{};
    std::optional<AcquirerDetails> acquirer_details{};
    std::optional<ProfileAcquirerIdValue> profile_acquirer_id{};
    std::optional<CustomerDetailsValue> customer_details{};
};

class AuthenticationRetrieveEligibilityCheckRequest
{
public:
    void set_authentication_id(AuthenticationIdValue value) { authentication_id = std::move(value); }
    [[nodiscard]] const AuthenticationIdValue& get_authentication_id() const noexcept { return authentication_id; }
    AuthenticationIdValue authentication_id{};
};

class AuthenticationRetrieveEligibilityCheckResponse
{
public:
    void set_authentication_id(AuthenticationIdValue value) { authentication_id = std::move(value); }
    [[nodiscard]] const AuthenticationIdValue& get_authentication_id() const noexcept { return authentication_id; }
    void set_eligibility_check_data(AuthenticationEligibilityCheckResponseData value) { eligibility_check_data = std::move(value); }
    [[nodiscard]] const AuthenticationEligibilityCheckResponseData& get_eligibility_check_data() const noexcept { return eligibility_check_data; }
    AuthenticationIdValue authentication_id{};
    AuthenticationEligibilityCheckResponseData eligibility_check_data{};
};

class AuthenticationSessionResponse
{
public:
    void set_authentication_id(AuthenticationIdValue value) { authentication_id = std::move(value); }
    [[nodiscard]] const AuthenticationIdValue& get_authentication_id() const noexcept { return authentication_id; }
    void set_session_token(std::vector<AuthenticationSessionToken> value) { session_token = std::move(value); }
    [[nodiscard]] const std::vector<AuthenticationSessionToken>& get_session_token() const noexcept { return session_token; }
    AuthenticationIdValue authentication_id{};
    std::vector<AuthenticationSessionToken> session_token{};
};

class AuthenticationSessionTokenRequest
{
public:
    void set_authentication_id(AuthenticationIdValue value) { authentication_id = std::move(value); }
    [[nodiscard]] const AuthenticationIdValue& get_authentication_id() const noexcept { return authentication_id; }
    void set_client_secret(std::optional<std::string> value) { client_secret = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_client_secret() const noexcept { return client_secret; }
    AuthenticationIdValue authentication_id{};
    std::optional<std::string> client_secret{};
};

class AuthenticationSyncPostUpdateRequest
{
public:
    void set_authentication_id(AuthenticationIdValue value) { authentication_id = std::move(value); }
    [[nodiscard]] const AuthenticationIdValue& get_authentication_id() const noexcept { return authentication_id; }
    AuthenticationIdValue authentication_id{};
};

class ClickToPayDetails
{
public:
    void set_merchant_transaction_id(std::optional<std::string> value) { merchant_transaction_id = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_merchant_transaction_id() const noexcept { return merchant_transaction_id; }
    void set_correlation_id(std::optional<std::string> value) { correlation_id = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_correlation_id() const noexcept { return correlation_id; }
    void set_x_src_flow_id(std::optional<std::string> value) { x_src_flow_id = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_x_src_flow_id() const noexcept { return x_src_flow_id; }
    void set_provider(std::optional<CtpServiceProviderValue> value) { provider = std::move(value); }
    [[nodiscard]] const std::optional<CtpServiceProviderValue>& get_provider() const noexcept { return provider; }
    void set_encrypted_payload(std::optional<std::string> value) { encrypted_payload = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_encrypted_payload() const noexcept { return encrypted_payload; }
    std::optional<std::string> merchant_transaction_id{};
    std::optional<std::string> correlation_id{};
    std::optional<std::string> x_src_flow_id{};
    std::optional<CtpServiceProviderValue> provider{};
    std::optional<std::string> encrypted_payload{};
};

class ExternalThreeDsData
{
public:
    void set_authentication_cryptogram(std::optional<Cryptogram> value) { authentication_cryptogram = std::move(value); }
    [[nodiscard]] const std::optional<Cryptogram>& get_authentication_cryptogram() const noexcept { return authentication_cryptogram; }
    void set_ds_trans_id(std::optional<std::string> value) { ds_trans_id = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_ds_trans_id() const noexcept { return ds_trans_id; }
    void set_version(std::optional<SemanticVersionValue> value) { version = std::move(value); }
    [[nodiscard]] const std::optional<SemanticVersionValue>& get_version() const noexcept { return version; }
    void set_eci(std::optional<std::string> value) { eci = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_eci() const noexcept { return eci; }
    void set_transaction_status(TransactionStatusValue value) { transaction_status = std::move(value); }
    [[nodiscard]] const TransactionStatusValue& get_transaction_status() const noexcept { return transaction_status; }
    std::optional<Cryptogram> authentication_cryptogram{};
    std::optional<std::string> ds_trans_id{};
    std::optional<SemanticVersionValue> version{};
    std::optional<std::string> eci{};
    TransactionStatusValue transaction_status{};
};

class NextAction
{
public:
    void set_url(UrlValue value) { url = std::move(value); }
    [[nodiscard]] const UrlValue& get_url() const noexcept { return url; }
    void set_http_method(MethodValue value) { http_method = std::move(value); }
    [[nodiscard]] const MethodValue& get_http_method() const noexcept { return http_method; }
    UrlValue url{};
    MethodValue http_method{};
};

class ThreeDsData
{
public:
    void set_three_ds_server_transaction_id(std::optional<std::string> value) { three_ds_server_transaction_id = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_three_ds_server_transaction_id() const noexcept { return three_ds_server_transaction_id; }
    void set_maximum_supported_3ds_version(std::optional<SemanticVersionValue> value) { maximum_supported_3ds_version = std::move(value); }
    [[nodiscard]] const std::optional<SemanticVersionValue>& get_maximum_supported_3ds_version() const noexcept { return maximum_supported_3ds_version; }
    void set_connector_authentication_id(std::optional<std::string> value) { connector_authentication_id = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_connector_authentication_id() const noexcept { return connector_authentication_id; }
    void set_three_ds_method_data(std::optional<std::string> value) { three_ds_method_data = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_three_ds_method_data() const noexcept { return three_ds_method_data; }
    void set_three_ds_method_url(std::optional<UrlValue> value) { three_ds_method_url = std::move(value); }
    [[nodiscard]] const std::optional<UrlValue>& get_three_ds_method_url() const noexcept { return three_ds_method_url; }
    void set_message_version(std::optional<SemanticVersionValue> value) { message_version = std::move(value); }
    [[nodiscard]] const std::optional<SemanticVersionValue>& get_message_version() const noexcept { return message_version; }
    void set_directory_server_id(std::optional<std::string> value) { directory_server_id = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_directory_server_id() const noexcept { return directory_server_id; }
    std::optional<std::string> three_ds_server_transaction_id{};
    std::optional<SemanticVersionValue> maximum_supported_3ds_version{};
    std::optional<std::string> connector_authentication_id{};
    std::optional<std::string> three_ds_method_data{};
    std::optional<UrlValue> three_ds_method_url{};
    std::optional<SemanticVersionValue> message_version{};
    std::optional<std::string> directory_server_id{};
};

class VisaEligibilityCheckData
{
public:
    void set_consumer_present(bool value) { consumer_present = std::move(value); }
    [[nodiscard]] const bool& get_consumer_present() const noexcept { return consumer_present; }
    void set_consumer_status(std::optional<std::string> value) { consumer_status = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_consumer_status() const noexcept { return consumer_status; }
    void set_custom_data(std::optional<SecretSerdeValueValue> value) { custom_data = std::move(value); }
    [[nodiscard]] const std::optional<SecretSerdeValueValue>& get_custom_data() const noexcept { return custom_data; }
    bool consumer_present{};
    std::optional<std::string> consumer_status{};
    std::optional<SecretSerdeValueValue> custom_data{};
};

class AuthenticationDetails
{
public:
    void set_three_ds_data(std::optional<ExternalThreeDsData> value) { three_ds_data = std::move(value); }
    [[nodiscard]] const std::optional<ExternalThreeDsData>& get_three_ds_data() const noexcept { return three_ds_data; }
    std::optional<ExternalThreeDsData> three_ds_data{};
};

enum class AuthenticationPaymentMethodData
{
    ClickToPayDetails,
};

class ClickToPayEligibilityCheckData
{
public:
    void set_visa(std::optional<VisaEligibilityCheckData> value) { visa = std::move(value); }
    [[nodiscard]] const std::optional<VisaEligibilityCheckData>& get_visa() const noexcept { return visa; }
    void set_mastercard(std::optional<MasterCardEligibilityCheckData> value) { mastercard = std::move(value); }
    [[nodiscard]] const std::optional<MasterCardEligibilityCheckData>& get_mastercard() const noexcept { return mastercard; }
    std::optional<VisaEligibilityCheckData> visa{};
    std::optional<MasterCardEligibilityCheckData> mastercard{};
};

enum class EligibilityResponseParams
{
    ThreeDsData,
};

class AuthenticationEligibilityResponse
{
public:
    void set_authentication_id(AuthenticationIdValue value) { authentication_id = std::move(value); }
    [[nodiscard]] const AuthenticationIdValue& get_authentication_id() const noexcept { return authentication_id; }
    void set_next_action(NextAction value) { next_action = std::move(value); }
    [[nodiscard]] const NextAction& get_next_action() const noexcept { return next_action; }
    void set_status(AuthenticationStatusValue value) { status = std::move(value); }
    [[nodiscard]] const AuthenticationStatusValue& get_status() const noexcept { return status; }
    void set_eligibility_response_params(std::optional<EligibilityResponseParams> value) { eligibility_response_params = std::move(value); }
    [[nodiscard]] const std::optional<EligibilityResponseParams>& get_eligibility_response_params() const noexcept { return eligibility_response_params; }
    void set_connector_metadata(std::optional<serde::Value> value) { connector_metadata = std::move(value); }
    [[nodiscard]] const std::optional<serde::Value>& get_connector_metadata() const noexcept { return connector_metadata; }
    void set_profile_id(ProfileIdValue value) { profile_id = std::move(value); }
    [[nodiscard]] const ProfileIdValue& get_profile_id() const noexcept { return profile_id; }
    void set_error_message(std::optional<std::string> value) { error_message = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_error_message() const noexcept { return error_message; }
    void set_error_code(std::optional<std::string> value) { error_code = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_error_code() const noexcept { return error_code; }
    void set_authentication_connector(std::optional<AuthenticationConnectorsValue> value) { authentication_connector = std::move(value); }
    [[nodiscard]] const std::optional<AuthenticationConnectorsValue>& get_authentication_connector() const noexcept { return authentication_connector; }
    void set_billing(std::optional<AddressValue> value) { billing = std::move(value); }
    [[nodiscard]] const std::optional<AddressValue>& get_billing() const noexcept { return billing; }
    void set_shipping(std::optional<AddressValue> value) { shipping = std::move(value); }
    [[nodiscard]] const std::optional<AddressValue>& get_shipping() const noexcept { return shipping; }
    void set_browser_information(std::optional<BrowserInformationValue> value) { browser_information = std::move(value); }
    [[nodiscard]] const std::optional<BrowserInformationValue>& get_browser_information() const noexcept { return browser_information; }
    void set_email(OptionalEncryptableEmailValue value) { email = std::move(value); }
    [[nodiscard]] const OptionalEncryptableEmailValue& get_email() const noexcept { return email; }
    void set_acquirer_details(std::optional<AcquirerDetails> value) { acquirer_details = std::move(value); }
    [[nodiscard]] const std::optional<AcquirerDetails>& get_acquirer_details() const noexcept { return acquirer_details; }
    AuthenticationIdValue authentication_id{};
    NextAction next_action{};
    AuthenticationStatusValue status{};
    std::optional<EligibilityResponseParams> eligibility_response_params{};
    std::optional<serde::Value> connector_metadata{};
    ProfileIdValue profile_id{};
    std::optional<std::string> error_message{};
    std::optional<std::string> error_code{};
    std::optional<AuthenticationConnectorsValue> authentication_connector{};
    std::optional<AddressValue> billing{};
    std::optional<AddressValue> shipping{};
    std::optional<BrowserInformationValue> browser_information{};
    OptionalEncryptableEmailValue email{};
    std::optional<AcquirerDetails> acquirer_details{};
};

class AuthenticationSyncResponse
{
public:
    void set_authentication_id(AuthenticationIdValue value) { authentication_id = std::move(value); }
    [[nodiscard]] const AuthenticationIdValue& get_authentication_id() const noexcept { return authentication_id; }
    void set_merchant_id(MerchantIdValue value) { merchant_id = std::move(value); }
    [[nodiscard]] const MerchantIdValue& get_merchant_id() const noexcept { return merchant_id; }
    void set_status(AuthenticationStatusValue value) { status = std::move(value); }
    [[nodiscard]] const AuthenticationStatusValue& get_status() const noexcept { return status; }
    void set_client_secret(std::optional<std::string> value) { client_secret = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_client_secret() const noexcept { return client_secret; }
    void set_amount(MinorUnitValue value) { amount = std::move(value); }
    [[nodiscard]] const MinorUnitValue& get_amount() const noexcept { return amount; }
    void set_currency(CurrencyValue value) { currency = std::move(value); }
    [[nodiscard]] const CurrencyValue& get_currency() const noexcept { return currency; }
    void set_authentication_connector(std::optional<AuthenticationConnectorsValue> value) { authentication_connector = std::move(value); }
    [[nodiscard]] const std::optional<AuthenticationConnectorsValue>& get_authentication_connector() const noexcept { return authentication_connector; }
    void set_force_3ds_challenge(std::optional<bool> value) { force_3ds_challenge = std::move(value); }
    [[nodiscard]] const std::optional<bool>& get_force_3ds_challenge() const noexcept { return force_3ds_challenge; }
    void set_return_url(std::optional<std::string> value) { return_url = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_return_url() const noexcept { return return_url; }
    void set_created_at(PrimitiveDateTimeValue value) { created_at = std::move(value); }
    [[nodiscard]] const PrimitiveDateTimeValue& get_created_at() const noexcept { return created_at; }
    void set_profile_id(ProfileIdValue value) { profile_id = std::move(value); }
    [[nodiscard]] const ProfileIdValue& get_profile_id() const noexcept { return profile_id; }
    void set_psd2_sca_exemption_type(std::optional<ScaExemptionTypeValue> value) { psd2_sca_exemption_type = std::move(value); }
    [[nodiscard]] const std::optional<ScaExemptionTypeValue>& get_psd2_sca_exemption_type() const noexcept { return psd2_sca_exemption_type; }
    void set_acquirer_details(std::optional<AcquirerDetails> value) { acquirer_details = std::move(value); }
    [[nodiscard]] const std::optional<AcquirerDetails>& get_acquirer_details() const noexcept { return acquirer_details; }
    void set_threeds_server_transaction_id(std::optional<std::string> value) { threeds_server_transaction_id = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_threeds_server_transaction_id() const noexcept { return threeds_server_transaction_id; }
    void set_maximum_supported_3ds_version(std::optional<SemanticVersionValue> value) { maximum_supported_3ds_version = std::move(value); }
    [[nodiscard]] const std::optional<SemanticVersionValue>& get_maximum_supported_3ds_version() const noexcept { return maximum_supported_3ds_version; }
    void set_connector_authentication_id(std::optional<std::string> value) { connector_authentication_id = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_connector_authentication_id() const noexcept { return connector_authentication_id; }
    void set_three_ds_method_data(std::optional<std::string> value) { three_ds_method_data = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_three_ds_method_data() const noexcept { return three_ds_method_data; }
    void set_three_ds_method_url(std::optional<std::string> value) { three_ds_method_url = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_three_ds_method_url() const noexcept { return three_ds_method_url; }
    void set_message_version(std::optional<SemanticVersionValue> value) { message_version = std::move(value); }
    [[nodiscard]] const std::optional<SemanticVersionValue>& get_message_version() const noexcept { return message_version; }
    void set_connector_metadata(std::optional<serde::Value> value) { connector_metadata = std::move(value); }
    [[nodiscard]] const std::optional<serde::Value>& get_connector_metadata() const noexcept { return connector_metadata; }
    void set_directory_server_id(std::optional<std::string> value) { directory_server_id = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_directory_server_id() const noexcept { return directory_server_id; }
    void set_payment_method_data(std::optional<AuthenticationPaymentMethodDataResponse> value) { payment_method_data = std::move(value); }
    [[nodiscard]] const std::optional<AuthenticationPaymentMethodDataResponse>& get_payment_method_data() const noexcept { return payment_method_data; }
    void set_vault_token_data(std::optional<AuthenticationVaultTokenData> value) { vault_token_data = std::move(value); }
    [[nodiscard]] const std::optional<AuthenticationVaultTokenData>& get_vault_token_data() const noexcept { return vault_token_data; }
    void set_authentication_details(std::optional<AuthenticationDetails> value) { authentication_details = std::move(value); }
    [[nodiscard]] const std::optional<AuthenticationDetails>& get_authentication_details() const noexcept { return authentication_details; }
    void set_billing(std::optional<AddressValue> value) { billing = std::move(value); }
    [[nodiscard]] const std::optional<AddressValue>& get_billing() const noexcept { return billing; }
    void set_shipping(std::optional<AddressValue> value) { shipping = std::move(value); }
    [[nodiscard]] const std::optional<AddressValue>& get_shipping() const noexcept { return shipping; }
    void set_browser_information(std::optional<BrowserInformationValue> value) { browser_information = std::move(value); }
    [[nodiscard]] const std::optional<BrowserInformationValue>& get_browser_information() const noexcept { return browser_information; }
    void set_email(OptionalEncryptableEmailValue value) { email = std::move(value); }
    [[nodiscard]] const OptionalEncryptableEmailValue& get_email() const noexcept { return email; }
    void set_transaction_status(std::optional<TransactionStatusValue> value) { transaction_status = std::move(value); }
    [[nodiscard]] const std::optional<TransactionStatusValue>& get_transaction_status() const noexcept { return transaction_status; }
    void set_acs_url(std::optional<std::string> value) { acs_url = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_acs_url() const noexcept { return acs_url; }
    void set_challenge_request(std::optional<std::string> value) { challenge_request = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_challenge_request() const noexcept { return challenge_request; }
    void set_acs_reference_number(std::optional<std::string> value) { acs_reference_number = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_acs_reference_number() const noexcept { return acs_reference_number; }
    void set_acs_trans_id(std::optional<std::string> value) { acs_trans_id = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_acs_trans_id() const noexcept { return acs_trans_id; }
    void set_acs_signed_content(std::optional<std::string> value) { acs_signed_content = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_acs_signed_content() const noexcept { return acs_signed_content; }
    void set_three_ds_requestor_url(std::optional<std::string> value) { three_ds_requestor_url = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_three_ds_requestor_url() const noexcept { return three_ds_requestor_url; }
    void set_three_ds_requestor_app_url(std::optional<std::string> value) { three_ds_requestor_app_url = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_three_ds_requestor_app_url() const noexcept { return three_ds_requestor_app_url; }
    void set_eci(std::optional<std::string> value) { eci = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_eci() const noexcept { return eci; }
    void set_error_message(std::optional<std::string> value) { error_message = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_error_message() const noexcept { return error_message; }
    void set_error_code(std::optional<std::string> value) { error_code = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_error_code() const noexcept { return error_code; }
    void set_profile_acquirer_id(std::optional<ProfileAcquirerIdValue> value) { profile_acquirer_id = std::move(value); }
    [[nodiscard]] const std::optional<ProfileAcquirerIdValue>& get_profile_acquirer_id() const noexcept { return profile_acquirer_id; }
    AuthenticationIdValue authentication_id{};
    MerchantIdValue merchant_id{};
    AuthenticationStatusValue status{};
    std::optional<std::string> client_secret{};
    MinorUnitValue amount{};
    CurrencyValue currency{};
    std::optional<AuthenticationConnectorsValue> authentication_connector{};
    std::optional<bool> force_3ds_challenge{};
    std::optional<std::string> return_url{};
    PrimitiveDateTimeValue created_at{};
    ProfileIdValue profile_id{};
    std::optional<ScaExemptionTypeValue> psd2_sca_exemption_type{};
    std::optional<AcquirerDetails> acquirer_details{};
    std::optional<std::string> threeds_server_transaction_id{};
    std::optional<SemanticVersionValue> maximum_supported_3ds_version{};
    std::optional<std::string> connector_authentication_id{};
    std::optional<std::string> three_ds_method_data{};
    std::optional<std::string> three_ds_method_url{};
    std::optional<SemanticVersionValue> message_version{};
    std::optional<serde::Value> connector_metadata{};
    std::optional<std::string> directory_server_id{};
    std::optional<AuthenticationPaymentMethodDataResponse> payment_method_data{};
    std::optional<AuthenticationVaultTokenData> vault_token_data{};
    std::optional<AuthenticationDetails> authentication_details{};
    std::optional<AddressValue> billing{};
    std::optional<AddressValue> shipping{};
    std::optional<BrowserInformationValue> browser_information{};
    OptionalEncryptableEmailValue email{};
    std::optional<TransactionStatusValue> transaction_status{};
    std::optional<std::string> acs_url{};
    std::optional<std::string> challenge_request{};
    std::optional<std::string> acs_reference_number{};
    std::optional<std::string> acs_trans_id{};
    std::optional<std::string> acs_signed_content{};
    std::optional<std::string> three_ds_requestor_url{};
    std::optional<std::string> three_ds_requestor_app_url{};
    std::optional<std::string> eci{};
    std::optional<std::string> error_message{};
    std::optional<std::string> error_code{};
    std::optional<ProfileAcquirerIdValue> profile_acquirer_id{};
};

class PostAuthenticationRequestPaymentMethodData
{
public:
    void set_payment_method_type(AuthenticationPaymentMethodType value) { payment_method_type = std::move(value); }
    [[nodiscard]] const AuthenticationPaymentMethodType& get_payment_method_type() const noexcept { return payment_method_type; }
    void set_payment_method_data(AuthenticationPaymentMethodData value) { payment_method_data = std::move(value); }
    [[nodiscard]] const AuthenticationPaymentMethodData& get_payment_method_data() const noexcept { return payment_method_data; }
    AuthenticationPaymentMethodType payment_method_type{};
    AuthenticationPaymentMethodData payment_method_data{};
};

class AuthenticationSyncRequest
{
public:
    void set_client_secret(std::optional<std::string> value) { client_secret = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_client_secret() const noexcept { return client_secret; }
    void set_payment_method_details(std::optional<PostAuthenticationRequestPaymentMethodData> value) { payment_method_details = std::move(value); }
    [[nodiscard]] const std::optional<PostAuthenticationRequestPaymentMethodData>& get_payment_method_details() const noexcept { return payment_method_details; }
    void set_authentication_id(AuthenticationIdValue value) { authentication_id = std::move(value); }
    [[nodiscard]] const AuthenticationIdValue& get_authentication_id() const noexcept { return authentication_id; }
    std::optional<std::string> client_secret{};
    std::optional<PostAuthenticationRequestPaymentMethodData> payment_method_details{};
    AuthenticationIdValue authentication_id{};
};
}

template<>
struct serde::Serializable<app::dto::authentication::AcquirerDetails>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"acquirer_bin", &app::dto::authentication::AcquirerDetails::get_acquirer_bin, &app::dto::authentication::AcquirerDetails::set_acquirer_bin, serde::FieldOptions{}>{},
            serde::FieldDesc<"acquirer_merchant_id", &app::dto::authentication::AcquirerDetails::get_acquirer_merchant_id, &app::dto::authentication::AcquirerDetails::set_acquirer_merchant_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"merchant_country_code", &app::dto::authentication::AcquirerDetails::get_merchant_country_code, &app::dto::authentication::AcquirerDetails::set_merchant_country_code, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "acquirer_details"; }
};

template<>
struct serde::Serializable<app::dto::authentication::AuthenticationCreateRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"authentication_id", &app::dto::authentication::AuthenticationCreateRequest::get_authentication_id, &app::dto::authentication::AuthenticationCreateRequest::set_authentication_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"profile_id", &app::dto::authentication::AuthenticationCreateRequest::get_profile_id, &app::dto::authentication::AuthenticationCreateRequest::set_profile_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"amount", &app::dto::authentication::AuthenticationCreateRequest::get_amount, &app::dto::authentication::AuthenticationCreateRequest::set_amount, serde::FieldOptions{}>{},
            serde::FieldDesc<"authentication_connector", &app::dto::authentication::AuthenticationCreateRequest::get_authentication_connector, &app::dto::authentication::AuthenticationCreateRequest::set_authentication_connector, serde::FieldOptions{}>{},
            serde::FieldDesc<"currency", &app::dto::authentication::AuthenticationCreateRequest::get_currency, &app::dto::authentication::AuthenticationCreateRequest::set_currency, serde::FieldOptions{}>{},
            serde::FieldDesc<"return_url", &app::dto::authentication::AuthenticationCreateRequest::get_return_url, &app::dto::authentication::AuthenticationCreateRequest::set_return_url, serde::FieldOptions{}>{},
            serde::FieldDesc<"force_3ds_challenge", &app::dto::authentication::AuthenticationCreateRequest::get_force_3ds_challenge, &app::dto::authentication::AuthenticationCreateRequest::set_force_3ds_challenge, serde::FieldOptions{}>{},
            serde::FieldDesc<"psd2_sca_exemption_type", &app::dto::authentication::AuthenticationCreateRequest::get_psd2_sca_exemption_type, &app::dto::authentication::AuthenticationCreateRequest::set_psd2_sca_exemption_type, serde::FieldOptions{}>{},
            serde::FieldDesc<"profile_acquirer_id", &app::dto::authentication::AuthenticationCreateRequest::get_profile_acquirer_id, &app::dto::authentication::AuthenticationCreateRequest::set_profile_acquirer_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"acquirer_details", &app::dto::authentication::AuthenticationCreateRequest::get_acquirer_details, &app::dto::authentication::AuthenticationCreateRequest::set_acquirer_details, serde::FieldOptions{}>{},
            serde::FieldDesc<"customer_details", &app::dto::authentication::AuthenticationCreateRequest::get_customer_details, &app::dto::authentication::AuthenticationCreateRequest::set_customer_details, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "authentication_create_request"; }
};

template<>
struct serde::Serializable<app::dto::authentication::AuthenticationResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"authentication_id", &app::dto::authentication::AuthenticationResponse::get_authentication_id, &app::dto::authentication::AuthenticationResponse::set_authentication_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"merchant_id", &app::dto::authentication::AuthenticationResponse::get_merchant_id, &app::dto::authentication::AuthenticationResponse::set_merchant_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"status", &app::dto::authentication::AuthenticationResponse::get_status, &app::dto::authentication::AuthenticationResponse::set_status, serde::FieldOptions{}>{},
            serde::FieldDesc<"client_secret", &app::dto::authentication::AuthenticationResponse::get_client_secret, &app::dto::authentication::AuthenticationResponse::set_client_secret, serde::FieldOptions{}>{},
            serde::FieldDesc<"amount", &app::dto::authentication::AuthenticationResponse::get_amount, &app::dto::authentication::AuthenticationResponse::set_amount, serde::FieldOptions{}>{},
            serde::FieldDesc<"currency", &app::dto::authentication::AuthenticationResponse::get_currency, &app::dto::authentication::AuthenticationResponse::set_currency, serde::FieldOptions{}>{},
            serde::FieldDesc<"authentication_connector", &app::dto::authentication::AuthenticationResponse::get_authentication_connector, &app::dto::authentication::AuthenticationResponse::set_authentication_connector, serde::FieldOptions{}>{},
            serde::FieldDesc<"force_3ds_challenge", &app::dto::authentication::AuthenticationResponse::get_force_3ds_challenge, &app::dto::authentication::AuthenticationResponse::set_force_3ds_challenge, serde::FieldOptions{}>{},
            serde::FieldDesc<"return_url", &app::dto::authentication::AuthenticationResponse::get_return_url, &app::dto::authentication::AuthenticationResponse::set_return_url, serde::FieldOptions{}>{},
            serde::FieldDesc<"created_at", &app::dto::authentication::AuthenticationResponse::get_created_at, &app::dto::authentication::AuthenticationResponse::set_created_at, serde::FieldOptions{}>{},
            serde::FieldDesc<"error_code", &app::dto::authentication::AuthenticationResponse::get_error_code, &app::dto::authentication::AuthenticationResponse::set_error_code, serde::FieldOptions{}>{},
            serde::FieldDesc<"error_message", &app::dto::authentication::AuthenticationResponse::get_error_message, &app::dto::authentication::AuthenticationResponse::set_error_message, serde::FieldOptions{}>{},
            serde::FieldDesc<"profile_id", &app::dto::authentication::AuthenticationResponse::get_profile_id, &app::dto::authentication::AuthenticationResponse::set_profile_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"psd2_sca_exemption_type", &app::dto::authentication::AuthenticationResponse::get_psd2_sca_exemption_type, &app::dto::authentication::AuthenticationResponse::set_psd2_sca_exemption_type, serde::FieldOptions{}>{},
            serde::FieldDesc<"acquirer_details", &app::dto::authentication::AuthenticationResponse::get_acquirer_details, &app::dto::authentication::AuthenticationResponse::set_acquirer_details, serde::FieldOptions{}>{},
            serde::FieldDesc<"profile_acquirer_id", &app::dto::authentication::AuthenticationResponse::get_profile_acquirer_id, &app::dto::authentication::AuthenticationResponse::set_profile_acquirer_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"customer_details", &app::dto::authentication::AuthenticationResponse::get_customer_details, &app::dto::authentication::AuthenticationResponse::set_customer_details, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "authentication_response"; }
};

template<>
struct serde::Serializable<app::dto::authentication::AuthenticationEligibilityRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"payment_method_data", &app::dto::authentication::AuthenticationEligibilityRequest::get_payment_method_data, &app::dto::authentication::AuthenticationEligibilityRequest::set_payment_method_data, serde::FieldOptions{}>{},
            serde::FieldDesc<"payment_method", &app::dto::authentication::AuthenticationEligibilityRequest::get_payment_method, &app::dto::authentication::AuthenticationEligibilityRequest::set_payment_method, serde::FieldOptions{}>{},
            serde::FieldDesc<"payment_method_type", &app::dto::authentication::AuthenticationEligibilityRequest::get_payment_method_type, &app::dto::authentication::AuthenticationEligibilityRequest::set_payment_method_type, serde::FieldOptions{}>{},
            serde::FieldDesc<"client_secret", &app::dto::authentication::AuthenticationEligibilityRequest::get_client_secret, &app::dto::authentication::AuthenticationEligibilityRequest::set_client_secret, serde::FieldOptions{}>{},
            serde::FieldDesc<"profile_id", &app::dto::authentication::AuthenticationEligibilityRequest::get_profile_id, &app::dto::authentication::AuthenticationEligibilityRequest::set_profile_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"billing", &app::dto::authentication::AuthenticationEligibilityRequest::get_billing, &app::dto::authentication::AuthenticationEligibilityRequest::set_billing, serde::FieldOptions{}>{},
            serde::FieldDesc<"shipping", &app::dto::authentication::AuthenticationEligibilityRequest::get_shipping, &app::dto::authentication::AuthenticationEligibilityRequest::set_shipping, serde::FieldOptions{}>{},
            serde::FieldDesc<"browser_information", &app::dto::authentication::AuthenticationEligibilityRequest::get_browser_information, &app::dto::authentication::AuthenticationEligibilityRequest::set_browser_information, serde::FieldOptions{}>{},
            serde::FieldDesc<"email", &app::dto::authentication::AuthenticationEligibilityRequest::get_email, &app::dto::authentication::AuthenticationEligibilityRequest::set_email, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "authentication_eligibility_request"; }
};

template<>
struct serde::Serializable<app::dto::authentication::NextAction>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"url", &app::dto::authentication::NextAction::get_url, &app::dto::authentication::NextAction::set_url, serde::FieldOptions{}>{},
            serde::FieldDesc<"http_method", &app::dto::authentication::NextAction::get_http_method, &app::dto::authentication::NextAction::set_http_method, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "next_action"; }
};

template<>
struct serde::Serializable<app::dto::authentication::AuthenticationEligibilityResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"authentication_id", &app::dto::authentication::AuthenticationEligibilityResponse::get_authentication_id, &app::dto::authentication::AuthenticationEligibilityResponse::set_authentication_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"next_action", &app::dto::authentication::AuthenticationEligibilityResponse::get_next_action, &app::dto::authentication::AuthenticationEligibilityResponse::set_next_action, serde::FieldOptions{}>{},
            serde::FieldDesc<"status", &app::dto::authentication::AuthenticationEligibilityResponse::get_status, &app::dto::authentication::AuthenticationEligibilityResponse::set_status, serde::FieldOptions{}>{},
            serde::FieldDesc<"eligibility_response_params", &app::dto::authentication::AuthenticationEligibilityResponse::get_eligibility_response_params, &app::dto::authentication::AuthenticationEligibilityResponse::set_eligibility_response_params, serde::FieldOptions{}>{},
            serde::FieldDesc<"connector_metadata", &app::dto::authentication::AuthenticationEligibilityResponse::get_connector_metadata, &app::dto::authentication::AuthenticationEligibilityResponse::set_connector_metadata, serde::FieldOptions{}>{},
            serde::FieldDesc<"profile_id", &app::dto::authentication::AuthenticationEligibilityResponse::get_profile_id, &app::dto::authentication::AuthenticationEligibilityResponse::set_profile_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"error_message", &app::dto::authentication::AuthenticationEligibilityResponse::get_error_message, &app::dto::authentication::AuthenticationEligibilityResponse::set_error_message, serde::FieldOptions{}>{},
            serde::FieldDesc<"error_code", &app::dto::authentication::AuthenticationEligibilityResponse::get_error_code, &app::dto::authentication::AuthenticationEligibilityResponse::set_error_code, serde::FieldOptions{}>{},
            serde::FieldDesc<"authentication_connector", &app::dto::authentication::AuthenticationEligibilityResponse::get_authentication_connector, &app::dto::authentication::AuthenticationEligibilityResponse::set_authentication_connector, serde::FieldOptions{}>{},
            serde::FieldDesc<"billing", &app::dto::authentication::AuthenticationEligibilityResponse::get_billing, &app::dto::authentication::AuthenticationEligibilityResponse::set_billing, serde::FieldOptions{}>{},
            serde::FieldDesc<"shipping", &app::dto::authentication::AuthenticationEligibilityResponse::get_shipping, &app::dto::authentication::AuthenticationEligibilityResponse::set_shipping, serde::FieldOptions{}>{},
            serde::FieldDesc<"browser_information", &app::dto::authentication::AuthenticationEligibilityResponse::get_browser_information, &app::dto::authentication::AuthenticationEligibilityResponse::set_browser_information, serde::FieldOptions{}>{},
            serde::FieldDesc<"email", &app::dto::authentication::AuthenticationEligibilityResponse::get_email, &app::dto::authentication::AuthenticationEligibilityResponse::set_email, serde::FieldOptions{}>{},
            serde::FieldDesc<"acquirer_details", &app::dto::authentication::AuthenticationEligibilityResponse::get_acquirer_details, &app::dto::authentication::AuthenticationEligibilityResponse::set_acquirer_details, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "authentication_eligibility_response"; }
};

template<>
struct serde::Serializable<app::dto::authentication::AuthenticationEligibilityCheckRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"authentication_id", &app::dto::authentication::AuthenticationEligibilityCheckRequest::get_authentication_id, &app::dto::authentication::AuthenticationEligibilityCheckRequest::set_authentication_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"client_secret", &app::dto::authentication::AuthenticationEligibilityCheckRequest::get_client_secret, &app::dto::authentication::AuthenticationEligibilityCheckRequest::set_client_secret, serde::FieldOptions{}>{},
            serde::FieldDesc<"eligibility_check_data", &app::dto::authentication::AuthenticationEligibilityCheckRequest::get_eligibility_check_data, &app::dto::authentication::AuthenticationEligibilityCheckRequest::set_eligibility_check_data, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "authentication_eligibility_check_request"; }
};

template<>
struct serde::Serializable<app::dto::authentication::AuthenticationEligibilityCheckResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"authentication_id", &app::dto::authentication::AuthenticationEligibilityCheckResponse::get_authentication_id, &app::dto::authentication::AuthenticationEligibilityCheckResponse::set_authentication_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"sdk_next_action", &app::dto::authentication::AuthenticationEligibilityCheckResponse::get_sdk_next_action, &app::dto::authentication::AuthenticationEligibilityCheckResponse::set_sdk_next_action, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "authentication_eligibility_check_response"; }
};

template<>
struct serde::Serializable<app::dto::authentication::AuthenticationRetrieveEligibilityCheckRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"authentication_id", &app::dto::authentication::AuthenticationRetrieveEligibilityCheckRequest::get_authentication_id, &app::dto::authentication::AuthenticationRetrieveEligibilityCheckRequest::set_authentication_id, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "authentication_retrieve_eligibility_check_request"; }
};

template<>
struct serde::Serializable<app::dto::authentication::AuthenticationRetrieveEligibilityCheckResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"authentication_id", &app::dto::authentication::AuthenticationRetrieveEligibilityCheckResponse::get_authentication_id, &app::dto::authentication::AuthenticationRetrieveEligibilityCheckResponse::set_authentication_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"eligibility_check_data", &app::dto::authentication::AuthenticationRetrieveEligibilityCheckResponse::get_eligibility_check_data, &app::dto::authentication::AuthenticationRetrieveEligibilityCheckResponse::set_eligibility_check_data, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "authentication_retrieve_eligibility_check_response"; }
};

template<>
struct serde::Serializable<app::dto::authentication::VisaEligibilityCheckData>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"consumerPresent", &app::dto::authentication::VisaEligibilityCheckData::get_consumer_present, &app::dto::authentication::VisaEligibilityCheckData::set_consumer_present, serde::FieldOptions{}>{},
            serde::FieldDesc<"consumerStatus", &app::dto::authentication::VisaEligibilityCheckData::get_consumer_status, &app::dto::authentication::VisaEligibilityCheckData::set_consumer_status, serde::FieldOptions{}>{},
            serde::FieldDesc<"customData", &app::dto::authentication::VisaEligibilityCheckData::get_custom_data, &app::dto::authentication::VisaEligibilityCheckData::set_custom_data, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "visa_eligibility_check_data"; }
};

template<>
struct serde::Serializable<app::dto::authentication::MasterCardEligibilityCheckData>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"consumerPresent", &app::dto::authentication::MasterCardEligibilityCheckData::get_consumer_present, &app::dto::authentication::MasterCardEligibilityCheckData::set_consumer_present, serde::FieldOptions{}>{},
            serde::FieldDesc<"idLookupSessionId", &app::dto::authentication::MasterCardEligibilityCheckData::get_id_lookup_session_id, &app::dto::authentication::MasterCardEligibilityCheckData::set_id_lookup_session_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"lastUsedCardTimestamp", &app::dto::authentication::MasterCardEligibilityCheckData::get_last_used_card_timestamp, &app::dto::authentication::MasterCardEligibilityCheckData::set_last_used_card_timestamp, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "master_card_eligibility_check_data"; }
};

template<>
struct serde::Serializable<app::dto::authentication::ClickToPayEligibilityCheckData>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"visa", &app::dto::authentication::ClickToPayEligibilityCheckData::get_visa, &app::dto::authentication::ClickToPayEligibilityCheckData::set_visa, serde::FieldOptions{}>{},
            serde::FieldDesc<"mastercard", &app::dto::authentication::ClickToPayEligibilityCheckData::get_mastercard, &app::dto::authentication::ClickToPayEligibilityCheckData::set_mastercard, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "click_to_pay_eligibility_check_data"; }
};

template<>
struct serde::Serializable<app::dto::authentication::ClickToPayEligibilityCheckResponseData>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"visa", &app::dto::authentication::ClickToPayEligibilityCheckResponseData::get_visa, &app::dto::authentication::ClickToPayEligibilityCheckResponseData::set_visa, serde::FieldOptions{}>{},
            serde::FieldDesc<"mastercard", &app::dto::authentication::ClickToPayEligibilityCheckResponseData::get_mastercard, &app::dto::authentication::ClickToPayEligibilityCheckResponseData::set_mastercard, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "click_to_pay_eligibility_check_response_data"; }
};

template<>
struct serde::Serializable<app::dto::authentication::ThreeDsData>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"three_ds_server_transaction_id", &app::dto::authentication::ThreeDsData::get_three_ds_server_transaction_id, &app::dto::authentication::ThreeDsData::set_three_ds_server_transaction_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"maximum_supported_3ds_version", &app::dto::authentication::ThreeDsData::get_maximum_supported_3ds_version, &app::dto::authentication::ThreeDsData::set_maximum_supported_3ds_version, serde::FieldOptions{}>{},
            serde::FieldDesc<"connector_authentication_id", &app::dto::authentication::ThreeDsData::get_connector_authentication_id, &app::dto::authentication::ThreeDsData::set_connector_authentication_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"three_ds_method_data", &app::dto::authentication::ThreeDsData::get_three_ds_method_data, &app::dto::authentication::ThreeDsData::set_three_ds_method_data, serde::FieldOptions{}>{},
            serde::FieldDesc<"three_ds_method_url", &app::dto::authentication::ThreeDsData::get_three_ds_method_url, &app::dto::authentication::ThreeDsData::set_three_ds_method_url, serde::FieldOptions{}>{},
            serde::FieldDesc<"message_version", &app::dto::authentication::ThreeDsData::get_message_version, &app::dto::authentication::ThreeDsData::set_message_version, serde::FieldOptions{}>{},
            serde::FieldDesc<"directory_server_id", &app::dto::authentication::ThreeDsData::get_directory_server_id, &app::dto::authentication::ThreeDsData::set_directory_server_id, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "three_ds_data"; }
};

template<>
struct serde::Serializable<app::dto::authentication::AuthenticationAuthenticateRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"authentication_id", &app::dto::authentication::AuthenticationAuthenticateRequest::get_authentication_id, &app::dto::authentication::AuthenticationAuthenticateRequest::set_authentication_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"client_secret", &app::dto::authentication::AuthenticationAuthenticateRequest::get_client_secret, &app::dto::authentication::AuthenticationAuthenticateRequest::set_client_secret, serde::FieldOptions{}>{},
            serde::FieldDesc<"sdk_information", &app::dto::authentication::AuthenticationAuthenticateRequest::get_sdk_information, &app::dto::authentication::AuthenticationAuthenticateRequest::set_sdk_information, serde::FieldOptions{}>{},
            serde::FieldDesc<"device_channel", &app::dto::authentication::AuthenticationAuthenticateRequest::get_device_channel, &app::dto::authentication::AuthenticationAuthenticateRequest::set_device_channel, serde::FieldOptions{}>{},
            serde::FieldDesc<"threeds_method_comp_ind", &app::dto::authentication::AuthenticationAuthenticateRequest::get_threeds_method_comp_ind, &app::dto::authentication::AuthenticationAuthenticateRequest::set_threeds_method_comp_ind, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "authentication_authenticate_request"; }
};

template<>
struct serde::Serializable<app::dto::authentication::AuthenticationAuthenticateResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"trans_status", &app::dto::authentication::AuthenticationAuthenticateResponse::get_transaction_status, &app::dto::authentication::AuthenticationAuthenticateResponse::set_transaction_status, serde::FieldOptions{}>{},
            serde::FieldDesc<"acs_url", &app::dto::authentication::AuthenticationAuthenticateResponse::get_acs_url, &app::dto::authentication::AuthenticationAuthenticateResponse::set_acs_url, serde::FieldOptions{}>{},
            serde::FieldDesc<"challenge_request", &app::dto::authentication::AuthenticationAuthenticateResponse::get_challenge_request, &app::dto::authentication::AuthenticationAuthenticateResponse::set_challenge_request, serde::FieldOptions{}>{},
            serde::FieldDesc<"acs_reference_number", &app::dto::authentication::AuthenticationAuthenticateResponse::get_acs_reference_number, &app::dto::authentication::AuthenticationAuthenticateResponse::set_acs_reference_number, serde::FieldOptions{}>{},
            serde::FieldDesc<"acs_trans_id", &app::dto::authentication::AuthenticationAuthenticateResponse::get_acs_trans_id, &app::dto::authentication::AuthenticationAuthenticateResponse::set_acs_trans_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"three_ds_server_transaction_id", &app::dto::authentication::AuthenticationAuthenticateResponse::get_three_ds_server_transaction_id, &app::dto::authentication::AuthenticationAuthenticateResponse::set_three_ds_server_transaction_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"acs_signed_content", &app::dto::authentication::AuthenticationAuthenticateResponse::get_acs_signed_content, &app::dto::authentication::AuthenticationAuthenticateResponse::set_acs_signed_content, serde::FieldOptions{}>{},
            serde::FieldDesc<"three_ds_requestor_url", &app::dto::authentication::AuthenticationAuthenticateResponse::get_three_ds_requestor_url, &app::dto::authentication::AuthenticationAuthenticateResponse::set_three_ds_requestor_url, serde::FieldOptions{}>{},
            serde::FieldDesc<"three_ds_requestor_app_url", &app::dto::authentication::AuthenticationAuthenticateResponse::get_three_ds_requestor_app_url, &app::dto::authentication::AuthenticationAuthenticateResponse::set_three_ds_requestor_app_url, serde::FieldOptions{}>{},
            serde::FieldDesc<"error_message", &app::dto::authentication::AuthenticationAuthenticateResponse::get_error_message, &app::dto::authentication::AuthenticationAuthenticateResponse::set_error_message, serde::FieldOptions{}>{},
            serde::FieldDesc<"error_code", &app::dto::authentication::AuthenticationAuthenticateResponse::get_error_code, &app::dto::authentication::AuthenticationAuthenticateResponse::set_error_code, serde::FieldOptions{}>{},
            serde::FieldDesc<"authentication_value", &app::dto::authentication::AuthenticationAuthenticateResponse::get_authentication_value, &app::dto::authentication::AuthenticationAuthenticateResponse::set_authentication_value, serde::FieldOptions{}>{},
            serde::FieldDesc<"status", &app::dto::authentication::AuthenticationAuthenticateResponse::get_status, &app::dto::authentication::AuthenticationAuthenticateResponse::set_status, serde::FieldOptions{}>{},
            serde::FieldDesc<"authentication_connector", &app::dto::authentication::AuthenticationAuthenticateResponse::get_authentication_connector, &app::dto::authentication::AuthenticationAuthenticateResponse::set_authentication_connector, serde::FieldOptions{}>{},
            serde::FieldDesc<"authentication_id", &app::dto::authentication::AuthenticationAuthenticateResponse::get_authentication_id, &app::dto::authentication::AuthenticationAuthenticateResponse::set_authentication_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"eci", &app::dto::authentication::AuthenticationAuthenticateResponse::get_eci, &app::dto::authentication::AuthenticationAuthenticateResponse::set_eci, serde::FieldOptions{}>{},
            serde::FieldDesc<"acquirer_details", &app::dto::authentication::AuthenticationAuthenticateResponse::get_acquirer_details, &app::dto::authentication::AuthenticationAuthenticateResponse::set_acquirer_details, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "authentication_authenticate_response"; }
};

template<>
struct serde::Serializable<app::dto::authentication::ExternalThreeDsData>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"authentication_cryptogram", &app::dto::authentication::ExternalThreeDsData::get_authentication_cryptogram, &app::dto::authentication::ExternalThreeDsData::set_authentication_cryptogram, serde::FieldOptions{}>{},
            serde::FieldDesc<"ds_trans_id", &app::dto::authentication::ExternalThreeDsData::get_ds_trans_id, &app::dto::authentication::ExternalThreeDsData::set_ds_trans_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"version", &app::dto::authentication::ExternalThreeDsData::get_version, &app::dto::authentication::ExternalThreeDsData::set_version, serde::FieldOptions{}>{},
            serde::FieldDesc<"eci", &app::dto::authentication::ExternalThreeDsData::get_eci, &app::dto::authentication::ExternalThreeDsData::set_eci, serde::FieldOptions{}>{},
            serde::FieldDesc<"transaction_status", &app::dto::authentication::ExternalThreeDsData::get_transaction_status, &app::dto::authentication::ExternalThreeDsData::set_transaction_status, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "external_three_ds_data"; }
};

template<>
struct serde::Serializable<app::dto::authentication::AuthenticationDetails>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"three_ds_data", &app::dto::authentication::AuthenticationDetails::get_three_ds_data, &app::dto::authentication::AuthenticationDetails::set_three_ds_data, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "authentication_details"; }
};

template<>
struct serde::Serializable<app::dto::authentication::AuthenticationSyncResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"authentication_id", &app::dto::authentication::AuthenticationSyncResponse::get_authentication_id, &app::dto::authentication::AuthenticationSyncResponse::set_authentication_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"merchant_id", &app::dto::authentication::AuthenticationSyncResponse::get_merchant_id, &app::dto::authentication::AuthenticationSyncResponse::set_merchant_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"status", &app::dto::authentication::AuthenticationSyncResponse::get_status, &app::dto::authentication::AuthenticationSyncResponse::set_status, serde::FieldOptions{}>{},
            serde::FieldDesc<"client_secret", &app::dto::authentication::AuthenticationSyncResponse::get_client_secret, &app::dto::authentication::AuthenticationSyncResponse::set_client_secret, serde::FieldOptions{}>{},
            serde::FieldDesc<"amount", &app::dto::authentication::AuthenticationSyncResponse::get_amount, &app::dto::authentication::AuthenticationSyncResponse::set_amount, serde::FieldOptions{}>{},
            serde::FieldDesc<"currency", &app::dto::authentication::AuthenticationSyncResponse::get_currency, &app::dto::authentication::AuthenticationSyncResponse::set_currency, serde::FieldOptions{}>{},
            serde::FieldDesc<"authentication_connector", &app::dto::authentication::AuthenticationSyncResponse::get_authentication_connector, &app::dto::authentication::AuthenticationSyncResponse::set_authentication_connector, serde::FieldOptions{}>{},
            serde::FieldDesc<"force_3ds_challenge", &app::dto::authentication::AuthenticationSyncResponse::get_force_3ds_challenge, &app::dto::authentication::AuthenticationSyncResponse::set_force_3ds_challenge, serde::FieldOptions{}>{},
            serde::FieldDesc<"return_url", &app::dto::authentication::AuthenticationSyncResponse::get_return_url, &app::dto::authentication::AuthenticationSyncResponse::set_return_url, serde::FieldOptions{}>{},
            serde::FieldDesc<"created_at", &app::dto::authentication::AuthenticationSyncResponse::get_created_at, &app::dto::authentication::AuthenticationSyncResponse::set_created_at, serde::FieldOptions{}>{},
            serde::FieldDesc<"profile_id", &app::dto::authentication::AuthenticationSyncResponse::get_profile_id, &app::dto::authentication::AuthenticationSyncResponse::set_profile_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"psd2_sca_exemption_type", &app::dto::authentication::AuthenticationSyncResponse::get_psd2_sca_exemption_type, &app::dto::authentication::AuthenticationSyncResponse::set_psd2_sca_exemption_type, serde::FieldOptions{}>{},
            serde::FieldDesc<"acquirer_details", &app::dto::authentication::AuthenticationSyncResponse::get_acquirer_details, &app::dto::authentication::AuthenticationSyncResponse::set_acquirer_details, serde::FieldOptions{}>{},
            serde::FieldDesc<"threeds_server_transaction_id", &app::dto::authentication::AuthenticationSyncResponse::get_threeds_server_transaction_id, &app::dto::authentication::AuthenticationSyncResponse::set_threeds_server_transaction_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"maximum_supported_3ds_version", &app::dto::authentication::AuthenticationSyncResponse::get_maximum_supported_3ds_version, &app::dto::authentication::AuthenticationSyncResponse::set_maximum_supported_3ds_version, serde::FieldOptions{}>{},
            serde::FieldDesc<"connector_authentication_id", &app::dto::authentication::AuthenticationSyncResponse::get_connector_authentication_id, &app::dto::authentication::AuthenticationSyncResponse::set_connector_authentication_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"three_ds_method_data", &app::dto::authentication::AuthenticationSyncResponse::get_three_ds_method_data, &app::dto::authentication::AuthenticationSyncResponse::set_three_ds_method_data, serde::FieldOptions{}>{},
            serde::FieldDesc<"three_ds_method_url", &app::dto::authentication::AuthenticationSyncResponse::get_three_ds_method_url, &app::dto::authentication::AuthenticationSyncResponse::set_three_ds_method_url, serde::FieldOptions{}>{},
            serde::FieldDesc<"message_version", &app::dto::authentication::AuthenticationSyncResponse::get_message_version, &app::dto::authentication::AuthenticationSyncResponse::set_message_version, serde::FieldOptions{}>{},
            serde::FieldDesc<"connector_metadata", &app::dto::authentication::AuthenticationSyncResponse::get_connector_metadata, &app::dto::authentication::AuthenticationSyncResponse::set_connector_metadata, serde::FieldOptions{}>{},
            serde::FieldDesc<"directory_server_id", &app::dto::authentication::AuthenticationSyncResponse::get_directory_server_id, &app::dto::authentication::AuthenticationSyncResponse::set_directory_server_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"payment_method_data", &app::dto::authentication::AuthenticationSyncResponse::get_payment_method_data, &app::dto::authentication::AuthenticationSyncResponse::set_payment_method_data, serde::FieldOptions{}>{},
            serde::FieldDesc<"vault_token_data", &app::dto::authentication::AuthenticationSyncResponse::get_vault_token_data, &app::dto::authentication::AuthenticationSyncResponse::set_vault_token_data, serde::FieldOptions{}>{},
            serde::FieldDesc<"authentication_details", &app::dto::authentication::AuthenticationSyncResponse::get_authentication_details, &app::dto::authentication::AuthenticationSyncResponse::set_authentication_details, serde::FieldOptions{}>{},
            serde::FieldDesc<"billing", &app::dto::authentication::AuthenticationSyncResponse::get_billing, &app::dto::authentication::AuthenticationSyncResponse::set_billing, serde::FieldOptions{}>{},
            serde::FieldDesc<"shipping", &app::dto::authentication::AuthenticationSyncResponse::get_shipping, &app::dto::authentication::AuthenticationSyncResponse::set_shipping, serde::FieldOptions{}>{},
            serde::FieldDesc<"browser_information", &app::dto::authentication::AuthenticationSyncResponse::get_browser_information, &app::dto::authentication::AuthenticationSyncResponse::set_browser_information, serde::FieldOptions{}>{},
            serde::FieldDesc<"email", &app::dto::authentication::AuthenticationSyncResponse::get_email, &app::dto::authentication::AuthenticationSyncResponse::set_email, serde::FieldOptions{}>{},
            serde::FieldDesc<"trans_status", &app::dto::authentication::AuthenticationSyncResponse::get_transaction_status, &app::dto::authentication::AuthenticationSyncResponse::set_transaction_status, serde::FieldOptions{}>{},
            serde::FieldDesc<"acs_url", &app::dto::authentication::AuthenticationSyncResponse::get_acs_url, &app::dto::authentication::AuthenticationSyncResponse::set_acs_url, serde::FieldOptions{}>{},
            serde::FieldDesc<"challenge_request", &app::dto::authentication::AuthenticationSyncResponse::get_challenge_request, &app::dto::authentication::AuthenticationSyncResponse::set_challenge_request, serde::FieldOptions{}>{},
            serde::FieldDesc<"acs_reference_number", &app::dto::authentication::AuthenticationSyncResponse::get_acs_reference_number, &app::dto::authentication::AuthenticationSyncResponse::set_acs_reference_number, serde::FieldOptions{}>{},
            serde::FieldDesc<"acs_trans_id", &app::dto::authentication::AuthenticationSyncResponse::get_acs_trans_id, &app::dto::authentication::AuthenticationSyncResponse::set_acs_trans_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"acs_signed_content", &app::dto::authentication::AuthenticationSyncResponse::get_acs_signed_content, &app::dto::authentication::AuthenticationSyncResponse::set_acs_signed_content, serde::FieldOptions{}>{},
            serde::FieldDesc<"three_ds_requestor_url", &app::dto::authentication::AuthenticationSyncResponse::get_three_ds_requestor_url, &app::dto::authentication::AuthenticationSyncResponse::set_three_ds_requestor_url, serde::FieldOptions{}>{},
            serde::FieldDesc<"three_ds_requestor_app_url", &app::dto::authentication::AuthenticationSyncResponse::get_three_ds_requestor_app_url, &app::dto::authentication::AuthenticationSyncResponse::set_three_ds_requestor_app_url, serde::FieldOptions{}>{},
            serde::FieldDesc<"eci", &app::dto::authentication::AuthenticationSyncResponse::get_eci, &app::dto::authentication::AuthenticationSyncResponse::set_eci, serde::FieldOptions{}>{},
            serde::FieldDesc<"error_message", &app::dto::authentication::AuthenticationSyncResponse::get_error_message, &app::dto::authentication::AuthenticationSyncResponse::set_error_message, serde::FieldOptions{}>{},
            serde::FieldDesc<"error_code", &app::dto::authentication::AuthenticationSyncResponse::get_error_code, &app::dto::authentication::AuthenticationSyncResponse::set_error_code, serde::FieldOptions{}>{},
            serde::FieldDesc<"profile_acquirer_id", &app::dto::authentication::AuthenticationSyncResponse::get_profile_acquirer_id, &app::dto::authentication::AuthenticationSyncResponse::set_profile_acquirer_id, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "authentication_sync_response"; }
};

template<>
struct serde::Serializable<app::dto::authentication::ClickToPayDetails>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"merchant_transaction_id", &app::dto::authentication::ClickToPayDetails::get_merchant_transaction_id, &app::dto::authentication::ClickToPayDetails::set_merchant_transaction_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"correlation_id", &app::dto::authentication::ClickToPayDetails::get_correlation_id, &app::dto::authentication::ClickToPayDetails::set_correlation_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"x_src_flow_id", &app::dto::authentication::ClickToPayDetails::get_x_src_flow_id, &app::dto::authentication::ClickToPayDetails::set_x_src_flow_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"provider", &app::dto::authentication::ClickToPayDetails::get_provider, &app::dto::authentication::ClickToPayDetails::set_provider, serde::FieldOptions{}>{},
            serde::FieldDesc<"encrypted_payload", &app::dto::authentication::ClickToPayDetails::get_encrypted_payload, &app::dto::authentication::ClickToPayDetails::set_encrypted_payload, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "click_to_pay_details"; }
};

template<>
struct serde::Serializable<app::dto::authentication::PostAuthenticationRequestPaymentMethodData>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"payment_method_type", &app::dto::authentication::PostAuthenticationRequestPaymentMethodData::get_payment_method_type, &app::dto::authentication::PostAuthenticationRequestPaymentMethodData::set_payment_method_type, serde::FieldOptions{}>{},
            serde::FieldDesc<"payment_method_data", &app::dto::authentication::PostAuthenticationRequestPaymentMethodData::get_payment_method_data, &app::dto::authentication::PostAuthenticationRequestPaymentMethodData::set_payment_method_data, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "post_authentication_request_payment_method_data"; }
};

template<>
struct serde::Serializable<app::dto::authentication::AuthenticationSyncRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"client_secret", &app::dto::authentication::AuthenticationSyncRequest::get_client_secret, &app::dto::authentication::AuthenticationSyncRequest::set_client_secret, serde::FieldOptions{}>{},
            serde::FieldDesc<"payment_method_details", &app::dto::authentication::AuthenticationSyncRequest::get_payment_method_details, &app::dto::authentication::AuthenticationSyncRequest::set_payment_method_details, serde::FieldOptions{}>{},
            serde::FieldDesc<"authentication_id", &app::dto::authentication::AuthenticationSyncRequest::get_authentication_id, &app::dto::authentication::AuthenticationSyncRequest::set_authentication_id, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "authentication_sync_request"; }
};

template<>
struct serde::Serializable<app::dto::authentication::AuthenticationSyncPostUpdateRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"authentication_id", &app::dto::authentication::AuthenticationSyncPostUpdateRequest::get_authentication_id, &app::dto::authentication::AuthenticationSyncPostUpdateRequest::set_authentication_id, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "authentication_sync_post_update_request"; }
};

template<>
struct serde::Serializable<app::dto::authentication::AuthenticationSessionTokenRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"authentication_id", &app::dto::authentication::AuthenticationSessionTokenRequest::get_authentication_id, &app::dto::authentication::AuthenticationSessionTokenRequest::set_authentication_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"client_secret", &app::dto::authentication::AuthenticationSessionTokenRequest::get_client_secret, &app::dto::authentication::AuthenticationSessionTokenRequest::set_client_secret, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "authentication_session_token_request"; }
};

template<>
struct serde::Serializable<app::dto::authentication::AuthenticationSessionResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"authentication_id", &app::dto::authentication::AuthenticationSessionResponse::get_authentication_id, &app::dto::authentication::AuthenticationSessionResponse::set_authentication_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"session_token", &app::dto::authentication::AuthenticationSessionResponse::get_session_token, &app::dto::authentication::AuthenticationSessionResponse::set_session_token, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "authentication_session_response"; }
};

#ifdef CONGELADO_TEST
namespace app::dto::authentication::tests {
using namespace boost::ut;
suite<"app_dto_authentication"> app_dto_authentication_suite = [] {
    "reflects converted types"_test = [] {
        expect(std::tuple_size_v<decltype(serde::Serializable<app::dto::authentication::AuthenticationCreateRequest>::fields())> > 0);
    };
};
}
#endif
