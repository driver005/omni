module;
export module app_model:payments;

import core_serde;
import core_behavior;

import std;
import core_serde;
import core_behavior;
import app_connector;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace app::model::payments {


enum class Amount
{
    Value,
    Zero,
};

class AmountFilter : public core::behavior::Serializer<AmountFilter>
{
public:
    void set_start_amount(std::optional<std::int64_t> value)
    {
        start_amount = std::move(value);
    }

    [[nodiscard]] const std::optional<std::int64_t>& get_start_amount() const noexcept
    {
        return start_amount;
    }

    void set_end_amount(std::optional<std::int64_t> value)
    {
        end_amount = std::move(value);
    }

    [[nodiscard]] const std::optional<std::int64_t>& get_end_amount() const noexcept
    {
        return end_amount;
    }

    std::optional<std::int64_t> start_amount{};
    std::optional<std::int64_t> end_amount{};
};

class AttemptStatusValue : public core::behavior::Serializer<AttemptStatusValue>
{
public:
    void set_value(std::string v)
    {
        value = std::move(v);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

class AuthenticationStatusValue : public core::behavior::Serializer<AuthenticationStatusValue>
{
public:
    void set_value(std::string v)
    {
        value = std::move(v);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

class AuthenticationTypeValue : public core::behavior::Serializer<AuthenticationTypeValue>
{
public:
    void set_value(std::string v)
    {
        value = std::move(v);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

class AuthorizationStatusValue : public core::behavior::Serializer<AuthorizationStatusValue>
{
public:
    void set_value(std::string v)
    {
        value = std::move(v);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

class BillingDescriptorValue : public core::behavior::Serializer<BillingDescriptorValue>
{
public:
    void set_value(std::string v)
    {
        value = std::move(v);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

class BillingFrequencyValue : public core::behavior::Serializer<BillingFrequencyValue>
{
public:
    void set_value(std::string v)
    {
        value = std::move(v);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};


enum class CalculationType
{
    Percentage,
    FlatAmount,
};

class CaptureMethodValue : public core::behavior::Serializer<CaptureMethodValue>
{
public:
    void set_value(std::string v)
    {
        value = std::move(v);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

class CaptureStatusValue : public core::behavior::Serializer<CaptureStatusValue>
{
public:
    void set_value(std::string v)
    {
        value = std::move(v);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

class CardBinValue : public core::behavior::Serializer<CardBinValue>
{
public:
    void set_value(std::string v)
    {
        value = std::move(v);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

class CardDiscoveryValue : public core::behavior::Serializer<CardDiscoveryValue>
{
public:
    void set_value(std::string v)
    {
        value = std::move(v);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

class CavvAlgorithmValue : public core::behavior::Serializer<CavvAlgorithmValue>
{
public:
    void set_value(std::string v)
    {
        value = std::move(v);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

class CryptoData : public core::behavior::Serializer<CryptoData>
{
public:
    void set_pay_currency(std::optional<std::string> value)
    {
        pay_currency = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_pay_currency() const noexcept
    {
        return pay_currency;
    }

    void set_network(std::optional<std::string> value)
    {
        network = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_network() const noexcept
    {
        return network;
    }

    std::optional<std::string> pay_currency{};
    std::optional<std::string> network{};
};

enum class Cryptogram
{
    Cavv,
};

class CtpServiceProviderValue : public core::behavior::Serializer<CtpServiceProviderValue>
{
public:
    void set_value(std::string v)
    {
        value = std::move(v);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

class CustomerAcceptanceValue : public core::behavior::Serializer<CustomerAcceptanceValue>
{
public:
    void set_value(std::string v)
    {
        value = std::move(v);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

class CustomerDocumentDetailsValue : public core::behavior::Serializer<CustomerDocumentDetailsValue>
{
public:
    void set_value(std::string v)
    {
        value = std::move(v);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

class CustomerPaymentMethodResponseItemValue :
    public core::behavior::Serializer<CustomerPaymentMethodResponseItemValue>
{
public:
    void set_value(std::string v)
    {
        value = std::move(v);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

class DecoupledAuthenticationTypeValue :
    public core::behavior::Serializer<DecoupledAuthenticationTypeValue>
{
public:
    void set_value(std::string v)
    {
        value = std::move(v);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

class DescriptionValue : public core::behavior::Serializer<DescriptionValue>
{
public:
    void set_value(std::string v)
    {
        value = std::move(v);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

enum class DeviceChannel
{
    App,
    Browser,
};

class DeviceDetails : public core::behavior::Serializer<DeviceDetails>
{
public:
    void set_device_type(std::optional<std::string> value)
    {
        device_type = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_device_type() const noexcept
    {
        return device_type;
    }

    void set_device_brand(std::optional<std::string> value)
    {
        device_brand = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_device_brand() const noexcept
    {
        return device_brand;
    }

    void set_device_os(std::optional<std::string> value)
    {
        device_os = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_device_os() const noexcept
    {
        return device_os;
    }

    void set_device_display(std::optional<std::string> value)
    {
        device_display = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_device_display() const noexcept
    {
        return device_display;
    }

    std::optional<std::string> device_type{};
    std::optional<std::string> device_brand{};
    std::optional<std::string> device_os{};
    std::optional<std::string> device_display{};
};

class DiscountPercentageValue : public core::behavior::Serializer<DiscountPercentageValue>
{
public:
    void set_value(std::string v)
    {
        value = std::move(v);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

class DisputeResponsePaymentsRetrieveValue :
    public core::behavior::Serializer<DisputeResponsePaymentsRetrieveValue>
{
public:
    void set_value(std::string v)
    {
        value = std::move(v);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

class EnableOvercaptureBoolValue : public core::behavior::Serializer<EnableOvercaptureBoolValue>
{
public:
    void set_value(std::string v)
    {
        value = std::move(v);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

class EnablePartialAuthorizationBoolValue :
    public core::behavior::Serializer<EnablePartialAuthorizationBoolValue>
{
public:
    void set_value(std::string v)
    {
        value = std::move(v);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

class ErrorDetails : public core::behavior::Serializer<ErrorDetails>
{
public:
    void set_code(std::string value)
    {
        code = std::move(value);
    }

    [[nodiscard]] const std::string& get_code() const noexcept
    {
        return code;
    }

    void set_message(std::string value)
    {
        message = std::move(value);
    }

    [[nodiscard]] const std::string& get_message() const noexcept
    {
        return message;
    }

    void set_reason(std::optional<std::string> value)
    {
        reason = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_reason() const noexcept
    {
        return reason;
    }

    void set_unified_code(std::optional<std::string> value)
    {
        unified_code = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_unified_code() const noexcept
    {
        return unified_code;
    }

    void set_unified_message(std::optional<std::string> value)
    {
        unified_message = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_unified_message() const noexcept
    {
        return unified_message;
    }

    void set_network_advice_code(std::optional<std::string> value)
    {
        network_advice_code = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_network_advice_code() const noexcept
    {
        return network_advice_code;
    }

    void set_network_decline_code(std::optional<std::string> value)
    {
        network_decline_code = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_network_decline_code() const noexcept
    {
        return network_decline_code;
    }

    void set_network_error_message(std::optional<std::string> value)
    {
        network_error_message = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_network_error_message() const noexcept
    {
        return network_error_message;
    }

    std::string code{};
    std::string message{};
    std::optional<std::string> reason{};
    std::optional<std::string> unified_code{};
    std::optional<std::string> unified_message{};
    std::optional<std::string> network_advice_code{};
    std::optional<std::string> network_decline_code{};
    std::optional<std::string> network_error_message{};
};

class ExemptionIndicatorValue : public core::behavior::Serializer<ExemptionIndicatorValue>
{
public:
    void set_value(std::string v)
    {
        value = std::move(v);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

class ExtendedAuthorizationAppliedBoolValue :
    public core::behavior::Serializer<ExtendedAuthorizationAppliedBoolValue>
{
public:
    void set_value(std::string v)
    {
        value = std::move(v);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

class ExtendedCardInfoResponse : public core::behavior::Serializer<ExtendedCardInfoResponse>
{
public:
    void set_payload(std::string value)
    {
        payload = std::move(value);
    }

    [[nodiscard]] const std::string& get_payload() const noexcept
    {
        return payload;
    }

    std::string payload{};
};

class FingerprintTypeValue : public core::behavior::Serializer<FingerprintTypeValue>
{
public:
    void set_value(std::string v)
    {
        value = std::move(v);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

class FrmMessage : public core::behavior::Serializer<FrmMessage>
{
public:
    void set_frm_name(std::string value)
    {
        frm_name = std::move(value);
    }

    [[nodiscard]] const std::string& get_frm_name() const noexcept
    {
        return frm_name;
    }

    void set_frm_transaction_id(std::optional<std::string> value)
    {
        frm_transaction_id = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_frm_transaction_id() const noexcept
    {
        return frm_transaction_id;
    }

    void set_frm_transaction_type(std::optional<std::string> value)
    {
        frm_transaction_type = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_frm_transaction_type() const noexcept
    {
        return frm_transaction_type;
    }

    void set_frm_status(std::optional<std::string> value)
    {
        frm_status = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_frm_status() const noexcept
    {
        return frm_status;
    }

    void set_frm_score(std::optional<std::int32_t> value)
    {
        frm_score = std::move(value);
    }

    [[nodiscard]] const std::optional<std::int32_t>& get_frm_score() const noexcept
    {
        return frm_score;
    }

    void set_frm_reason(std::optional<serde::Value> value)
    {
        frm_reason = std::move(value);
    }

    [[nodiscard]] const std::optional<serde::Value>& get_frm_reason() const noexcept
    {
        return frm_reason;
    }

    void set_frm_error(std::optional<std::string> value)
    {
        frm_error = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_frm_error() const noexcept
    {
        return frm_error;
    }

    std::string frm_name{};
    std::optional<std::string> frm_transaction_id{};
    std::optional<std::string> frm_transaction_type{};
    std::optional<std::string> frm_status{};
    std::optional<std::int32_t> frm_score{};
    std::optional<serde::Value> frm_reason{};
    std::optional<std::string> frm_error{};
};

class GlobalAttemptIdValue : public core::behavior::Serializer<GlobalAttemptIdValue>
{
public:
    void set_value(std::string v)
    {
        value = std::move(v);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

class GlobalCustomerIdValue : public core::behavior::Serializer<GlobalCustomerIdValue>
{
public:
    void set_value(std::string v)
    {
        value = std::move(v);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

class GlobalPaymentMethodIdValue : public core::behavior::Serializer<GlobalPaymentMethodIdValue>
{
public:
    void set_value(std::string v)
    {
        value = std::move(v);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};


enum class GpaySessionTokenResponse
{
    ThirdPartyResponse,
    GooglePaySession,
};

class HyperswitchVaultData : public core::behavior::Serializer<HyperswitchVaultData>
{
public:
    void set_sdk_authorization(std::string value)
    {
        sdk_authorization = std::move(value);
    }

    [[nodiscard]] const std::string& get_sdk_authorization() const noexcept
    {
        return sdk_authorization;
    }

    std::string sdk_authorization{};
};

enum class IframeData
{
    ThreedsInvokeAndCompleteAutorize,
};

class InitiatorValue : public core::behavior::Serializer<InitiatorValue>
{
public:
    void set_value(std::string v)
    {
        value = std::move(v);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

class InstallmentDataValue : public core::behavior::Serializer<InstallmentDataValue>
{
public:
    void set_value(std::string v)
    {
        value = std::move(v);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

class InstallmentOptionValue : public core::behavior::Serializer<InstallmentOptionValue>
{
public:
    void set_value(std::string v)
    {
        value = std::move(v);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

enum class IntegrationType
{
    Client,
    Server,
};

class InternalVaultSessionDetails : public core::behavior::Serializer<InternalVaultSessionDetails>
{
public:
    void set_sdk_authorization(std::string value)
    {
        sdk_authorization = std::move(value);
    }

    [[nodiscard]] const std::string& get_sdk_authorization() const noexcept
    {
        return sdk_authorization;
    }

    std::string sdk_authorization{};
};

class IpAddrValue : public core::behavior::Serializer<IpAddrValue>
{
public:
    void set_value(std::string v)
    {
        value = std::move(v);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

class KlarnaSdkPaymentMethod : public core::behavior::Serializer<KlarnaSdkPaymentMethod>
{
public:
    void set_payment_type(std::optional<std::string> value)
    {
        payment_type = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_payment_type() const noexcept
    {
        return payment_type;
    }

    std::optional<std::string> payment_type{};
};

class KlarnaSdkPaymentMethodResponse :
    public core::behavior::Serializer<KlarnaSdkPaymentMethodResponse>
{
public:
    void set_payment_type(std::optional<std::string> value)
    {
        payment_type = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_payment_type() const noexcept
    {
        return payment_type;
    }

    std::optional<std::string> payment_type{};
};

class KlarnaSessionTokenResponse : public core::behavior::Serializer<KlarnaSessionTokenResponse>
{
public:
    void set_session_token(std::string value)
    {
        session_token = std::move(value);
    }

    [[nodiscard]] const std::string& get_session_token() const noexcept
    {
        return session_token;
    }

    void set_session_id(std::string value)
    {
        session_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_session_id() const noexcept
    {
        return session_id;
    }

    std::string session_token{};
    std::string session_id{};
};

enum class MandateType
{
    SingleUse,
    MultiUse,
};

class ManualUpdateIntentStatusValue :
    public core::behavior::Serializer<ManualUpdateIntentStatusValue>
{
public:
    void set_value(std::string v)
    {
        value = std::move(v);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

class MaskedRecipientDetailsValue : public core::behavior::Serializer<MaskedRecipientDetailsValue>
{
public:
    void set_value(std::string v)
    {
        value = std::move(v);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

class MitCategoryValue : public core::behavior::Serializer<MitCategoryValue>
{
public:
    void set_value(std::string v)
    {
        value = std::move(v);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

class MitExemptionRequestValue : public core::behavior::Serializer<MitExemptionRequestValue>
{
public:
    void set_value(std::string v)
    {
        value = std::move(v);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};


enum class MobilePaymentConsent
{
    ConsentRequired,
    ConsentNotRequired,
    ConsentOptional,
};

enum class MobilePaymentData
{
    DirectCarrierBilling,
};

class NetellerData : public core::behavior::Serializer<NetellerData>
{
public:
};

class NetworkDetails : public core::behavior::Serializer<NetworkDetails>
{
public:
    void set_network_advice_code(std::optional<std::string> value)
    {
        network_advice_code = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_network_advice_code() const noexcept
    {
        return network_advice_code;
    }

    std::optional<std::string> network_advice_code{};
};

class NetworkTokenValue : public core::behavior::Serializer<NetworkTokenValue>
{
public:
    void set_value(std::string v)
    {
        value = std::move(v);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};


enum class NextActionType
{
    RedirectToUrl,
    DisplayQrCode,
    InvokeSdkClient,
    TriggerApi,
    DisplayBankTransferInformation,
    DisplayWaitScreen,
    CollectOtp,
    RedirectInsidePopup,
    InvokeUpiIntentSdk,
    InvokeUpiQrFlow,
};

class NonZeroU8Value : public core::behavior::Serializer<NonZeroU8Value>
{
public:
    void set_value(std::string v)
    {
        value = std::move(v);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

class NullObject : public core::behavior::Serializer<NullObject>
{
public:
};

class OfferSelection : public core::behavior::Serializer<OfferSelection>
{
public:
    void set_offer_quote_ids(std::vector<std::string> value)
    {
        offer_quote_ids = std::move(value);
    }

    [[nodiscard]] const std::vector<std::string>& get_offer_quote_ids() const noexcept
    {
        return offer_quote_ids;
    }

    std::vector<std::string> offer_quote_ids{};
};

enum class OpenBankingData
{
    OpenBankingPIS,
};

class OpenBankingSessionToken : public core::behavior::Serializer<OpenBankingSessionToken>
{
public:
    void set_open_banking_session_token(std::string value)
    {
        open_banking_session_token = std::move(value);
    }

    [[nodiscard]] const std::string& get_open_banking_session_token() const noexcept
    {
        return open_banking_session_token;
    }

    std::string open_banking_session_token{};
};

class OptionalEncryptableEmailValue :
    public core::behavior::Serializer<OptionalEncryptableEmailValue>
{
public:
    void set_value(std::string v)
    {
        value = std::move(v);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

class OptionalEncryptableSecretStringValue :
    public core::behavior::Serializer<OptionalEncryptableSecretStringValue>
{
public:
    void set_value(std::string v)
    {
        value = std::move(v);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

class OvercaptureEnabledBoolValue : public core::behavior::Serializer<OvercaptureEnabledBoolValue>
{
public:
    void set_value(std::string v)
    {
        value = std::move(v);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

class PMBalanceCheckFailureResponse :
    public core::behavior::Serializer<PMBalanceCheckFailureResponse>
{
public:
    void set_error(std::string value)
    {
        error = std::move(value);
    }

    [[nodiscard]] const std::string& get_error() const noexcept
    {
        return error;
    }

    std::string error{};
};

class PageOffsetValue : public core::behavior::Serializer<PageOffsetValue>
{
public:
    void set_value(std::string v)
    {
        value = std::move(v);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

class PageSizeValue : public core::behavior::Serializer<PageSizeValue>
{
public:
    void set_value(std::string v)
    {
        value = std::move(v);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

class PartnerMerchantIdentifierDetailsValue :
    public core::behavior::Serializer<PartnerMerchantIdentifierDetailsValue>
{
public:
    void set_value(std::string v)
    {
        value = std::move(v);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

enum class PayLaterData
{
    KlarnaRedirect,
    KlarnaSdk,
    AffirmRedirect,
    AfterpayClearpayRedirect,
    PayBrightRedirect,
    FlexitiRedirect,
    WalleyRedirect,
    AlmaRedirect,
    AtomeRedirect,
    BreadpayRedirect,
    PayjustnowRedirect,
};


enum class PaymentAllowanceType
{
    Exact,
    Partial,
    Flexible,
};

class PaymentChannelValue : public core::behavior::Serializer<PaymentChannelValue>
{
public:
    void set_value(std::string v)
    {
        value = std::move(v);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

class PaymentExperienceValue : public core::behavior::Serializer<PaymentExperienceValue>
{
public:
    void set_value(std::string v)
    {
        value = std::move(v);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

enum class PaymentIdType
{
    PaymentIntentId,
    ConnectorTransactionId,
    PaymentAttemptId,
};

class PaymentLinkBackgroundImageConfigValue :
    public core::behavior::Serializer<PaymentLinkBackgroundImageConfigValue>
{
public:
    void set_value(std::string v)
    {
        value = std::move(v);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

class PaymentLinkConfigRequestValue :
    public core::behavior::Serializer<PaymentLinkConfigRequestValue>
{
public:
    void set_value(std::string v)
    {
        value = std::move(v);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

class PaymentLinkDetailsLayoutValue :
    public core::behavior::Serializer<PaymentLinkDetailsLayoutValue>
{
public:
    void set_value(std::string v)
    {
        value = std::move(v);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

class PaymentLinkResponse : public core::behavior::Serializer<PaymentLinkResponse>
{
public:
    void set_link(std::string value)
    {
        link = std::move(value);
    }

    [[nodiscard]] const std::string& get_link() const noexcept
    {
        return link;
    }

    void set_secure_link(std::optional<std::string> value)
    {
        secure_link = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_secure_link() const noexcept
    {
        return secure_link;
    }

    void set_payment_link_id(std::string value)
    {
        payment_link_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_payment_link_id() const noexcept
    {
        return payment_link_id;
    }

    std::string link{};
    std::optional<std::string> secure_link{};
    std::string payment_link_id{};
};

class PaymentLinkSdkLabelTypeValue : public core::behavior::Serializer<PaymentLinkSdkLabelTypeValue>
{
public:
    void set_value(std::string v)
    {
        value = std::move(v);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

class PaymentLinkShowSdkTermsValue : public core::behavior::Serializer<PaymentLinkShowSdkTermsValue>
{
public:
    void set_value(std::string v)
    {
        value = std::move(v);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

enum class PaymentLinkStatus
{
    Active,
    Expired,
};

class PaymentLinkTransactionDetailsValue :
    public core::behavior::Serializer<PaymentLinkTransactionDetailsValue>
{
public:
    void set_value(std::string v)
    {
        value = std::move(v);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

class PaymentMethodListResultValue : public core::behavior::Serializer<PaymentMethodListResultValue>
{
public:
    void set_value(std::string v)
    {
        value = std::move(v);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

class PaymentMethodStatusValue : public core::behavior::Serializer<PaymentMethodStatusValue>
{
public:
    void set_value(std::string v)
    {
        value = std::move(v);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

class PaymentMethodSubtypeSpecificDataValue :
    public core::behavior::Serializer<PaymentMethodSubtypeSpecificDataValue>
{
public:
    void set_value(std::string v)
    {
        value = std::move(v);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

class PaymentMethodsConfigValue : public core::behavior::Serializer<PaymentMethodsConfigValue>
{
public:
    void set_value(std::string v)
    {
        value = std::move(v);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

enum class PaymentOp
{
    Create,
    Update,
    Confirm,
};

class PaymentReferenceIdValue : public core::behavior::Serializer<PaymentReferenceIdValue>
{
public:
    void set_value(std::string v)
    {
        value = std::move(v);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

class PaymentsAggregateResponse : public core::behavior::Serializer<PaymentsAggregateResponse>
{
public:
    void set_status_with_count(std::unordered_map<std::string, std::int64_t> value)
    {
        status_with_count = std::move(value);
    }

    [[nodiscard]] const std::unordered_map<std::string, std::int64_t>&
    get_status_with_count() const noexcept
    {
        return status_with_count;
    }

    std::unordered_map<std::string, std::int64_t> status_with_count{};
};

class PaymentsStatusRequest : public core::behavior::Serializer<PaymentsStatusRequest>
{
public:
    void set_force_sync(bool value)
    {
        force_sync = std::move(value);
    }

    [[nodiscard]] const bool& get_force_sync() const noexcept
    {
        return force_sync;
    }

    void set_expand_attempts(bool value)
    {
        expand_attempts = std::move(value);
    }

    [[nodiscard]] const bool& get_expand_attempts() const noexcept
    {
        return expand_attempts;
    }

    void set_param(std::optional<std::string> value)
    {
        param = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_param() const noexcept
    {
        return param;
    }

    void set_return_raw_connector_response(std::optional<bool> value)
    {
        return_raw_connector_response = std::move(value);
    }

    [[nodiscard]] const std::optional<bool>& get_return_raw_connector_response() const noexcept
    {
        return return_raw_connector_response;
    }

    bool force_sync{};
    bool expand_attempts{};
    std::optional<std::string> param{};
    std::optional<bool> return_raw_connector_response{};
};

class PayseraData : public core::behavior::Serializer<PayseraData>
{
public:
};

class PollConfig : public core::behavior::Serializer<PollConfig>
{
public:
    void set_delay_in_secs(std::uint16_t value)
    {
        delay_in_secs = std::move(value);
    }

    [[nodiscard]] const std::uint16_t& get_delay_in_secs() const noexcept
    {
        return delay_in_secs;
    }

    void set_frequency(std::uint16_t value)
    {
        frequency = std::move(value);
    }

    [[nodiscard]] const std::uint16_t& get_frequency() const noexcept
    {
        return frequency;
    }

    std::uint16_t delay_in_secs{};
    std::uint16_t frequency{};
};

class PollConfigResponse : public core::behavior::Serializer<PollConfigResponse>
{
public:
    void set_poll_id(std::string value)
    {
        poll_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_poll_id() const noexcept
    {
        return poll_id;
    }

    void set_delay_in_secs(std::int8_t value)
    {
        delay_in_secs = std::move(value);
    }

    [[nodiscard]] const std::int8_t& get_delay_in_secs() const noexcept
    {
        return delay_in_secs;
    }

    void set_frequency(std::int8_t value)
    {
        frequency = std::move(value);
    }

    [[nodiscard]] const std::int8_t& get_frequency() const noexcept
    {
        return frequency;
    }

    std::string poll_id{};
    std::int8_t delay_in_secs{};
    std::int8_t frequency{};
};

class ProcessorPaymentTokenValue : public core::behavior::Serializer<ProcessorPaymentTokenValue>
{
public:
    void set_value(std::string v)
    {
        value = std::move(v);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

class ProductTypeValue : public core::behavior::Serializer<ProductTypeValue>
{
public:
    void set_value(std::string v)
    {
        value = std::move(v);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};


enum class RealTimePaymentData
{
    Fps,
    DuitNow,
    PromptPay,
    VietQr,
    Qris,
};

class RecommendedActionValue : public core::behavior::Serializer<RecommendedActionValue>
{
public:
    void set_value(std::string v)
    {
        value = std::move(v);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

class RecordAttemptErrorDetails : public core::behavior::Serializer<RecordAttemptErrorDetails>
{
public:
    void set_code(std::string value)
    {
        code = std::move(value);
    }

    [[nodiscard]] const std::string& get_code() const noexcept
    {
        return code;
    }

    void set_message(std::string value)
    {
        message = std::move(value);
    }

    [[nodiscard]] const std::string& get_message() const noexcept
    {
        return message;
    }

    void set_network_advice_code(std::optional<std::string> value)
    {
        network_advice_code = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_network_advice_code() const noexcept
    {
        return network_advice_code;
    }

    void set_network_decline_code(std::optional<std::string> value)
    {
        network_decline_code = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_network_decline_code() const noexcept
    {
        return network_decline_code;
    }

    void set_network_error_message(std::optional<std::string> value)
    {
        network_error_message = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_network_error_message() const noexcept
    {
        return network_error_message;
    }

    std::string code{};
    std::string message{};
    std::optional<std::string> network_advice_code{};
    std::optional<std::string> network_decline_code{};
    std::optional<std::string> network_error_message{};
};

class RecoveryNextRetryValue : public core::behavior::Serializer<RecoveryNextRetryValue>
{
public:
    void set_value(std::string v)
    {
        value = std::move(v);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

enum class RecoveryPaymentsCreateResponse
{
    Recorded,
    Accepted,
};

class RecoveryStatusValue : public core::behavior::Serializer<RecoveryStatusValue>
{
public:
    void set_value(std::string v)
    {
        value = std::move(v);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

class RecoveryTransactionStatusValue :
    public core::behavior::Serializer<RecoveryTransactionStatusValue>
{
public:
    void set_value(std::string v)
    {
        value = std::move(v);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

class RecurringDetailsValue : public core::behavior::Serializer<RecurringDetailsValue>
{
public:
    void set_value(std::string v)
    {
        value = std::move(v);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

class RefundResponseValue : public core::behavior::Serializer<RefundResponseValue>
{
public:
    void set_value(std::string v)
    {
        value = std::move(v);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

class RequestExtendedAuthorizationBoolValue :
    public core::behavior::Serializer<RequestExtendedAuthorizationBoolValue>
{
public:
    void set_value(std::string v)
    {
        value = std::move(v);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

class RequiredFieldInfoValue : public core::behavior::Serializer<RequiredFieldInfoValue>
{
public:
    void set_value(std::string v)
    {
        value = std::move(v);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

class RetrievePaymentLinkRequest : public core::behavior::Serializer<RetrievePaymentLinkRequest>
{
public:
    void set_client_secret(std::optional<std::string> value)
    {
        client_secret = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_client_secret() const noexcept
    {
        return client_secret;
    }

    std::optional<std::string> client_secret{};
};

class RevolutPayData : public core::behavior::Serializer<RevolutPayData>
{
public:
};

class Revolv3Data : public core::behavior::Serializer<Revolv3Data>
{
public:
    void set_processor(std::optional<std::string> value)
    {
        processor = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_processor() const noexcept
    {
        return processor;
    }

    std::optional<std::string> processor{};
};

class RoutingApproachValue : public core::behavior::Serializer<RoutingApproachValue>
{
public:
    void set_value(std::string v)
    {
        value = std::move(v);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

class RoutingIdValue : public core::behavior::Serializer<RoutingIdValue>
{
public:
    void set_value(std::string v)
    {
        value = std::move(v);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};


enum class SdkType
{
    DefaultSdk,
    SplitSdk,
    LimitedSdk,
    BrowserSdk,
    ShellSdk,
};

class SdkUpiUriInformation : public core::behavior::Serializer<SdkUpiUriInformation>
{
public:
    void set_sdk_uri(std::string value)
    {
        sdk_uri = std::move(value);
    }

    [[nodiscard]] const std::string& get_sdk_uri() const noexcept
    {
        return sdk_uri;
    }

    std::string sdk_uri{};
};

class SemanticVersionValue : public core::behavior::Serializer<SemanticVersionValue>
{
public:
    void set_value(std::string v)
    {
        value = std::move(v);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

enum class SessionToken
{
    GooglePay,
    SamsungPay,
    Klarna,
    Paypal,
    ApplePay,
    OpenBanking,
    Paze,
    ClickToPay,
    AmazonPay,
    NoSessionTokenReceived,
};

enum class SessionTokensResult
{
    Success,
    Failed,
};

class SkrillData : public core::behavior::Serializer<SkrillData>
{
public:
};

class SofortBilling : public core::behavior::Serializer<SofortBilling>
{
public:
    void set_billing_country(std::string value)
    {
        billing_country = std::move(value);
    }

    [[nodiscard]] const std::string& get_billing_country() const noexcept
    {
        return billing_country;
    }

    std::string billing_country{};
};

enum class SortBy
{
    Asc,
    Desc,
};

class StatementDescriptorValue : public core::behavior::Serializer<StatementDescriptorValue>
{
public:
    void set_value(std::string v)
    {
        value = std::move(v);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

class SurchargeCalculationOverrideValue :
    public core::behavior::Serializer<SurchargeCalculationOverrideValue>
{
public:
    void set_value(std::string v)
    {
        value = std::move(v);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

class SurchargeErrorDetailsValue : public core::behavior::Serializer<SurchargeErrorDetailsValue>
{
public:
    void set_value(std::string v)
    {
        value = std::move(v);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

class SwishQrData : public core::behavior::Serializer<SwishQrData>
{
public:
};

class TaxCalculationOverrideValue : public core::behavior::Serializer<TaxCalculationOverrideValue>
{
public:
    void set_value(std::string v)
    {
        value = std::move(v);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

enum class ThreeDsCompletionIndicator
{
    Success,
    Failure,
    NotAvailable,
};

enum class ThreeDsMethodData
{
    AcsThreeDsMethodData,
};

class TimeRangeValue : public core::behavior::Serializer<TimeRangeValue>
{
public:
    void set_value(std::string v)
    {
        value = std::move(v);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

class TokenizationValue : public core::behavior::Serializer<TokenizationValue>
{
public:
    void set_value(std::string v)
    {
        value = std::move(v);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

class TransactionStatusValue : public core::behavior::Serializer<TransactionStatusValue>
{
public:
    void set_value(std::string v)
    {
        value = std::move(v);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

class UpiAdditionalDataValue : public core::behavior::Serializer<UpiAdditionalDataValue>
{
public:
    void set_value(std::string v)
    {
        value = std::move(v);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

enum class UpiData
{
    UpiCollect,
    UpiIntent,
    UpiQr,
};

enum class UpiSource
{
    UpiCc,
    UpiCl,
    UpiAccount,
    UpiCcCl,
    UpiPpi,
    UpiVoucher,
};

class UrlDetails : public core::behavior::Serializer<UrlDetails>
{
public:
    void set_url(std::string value)
    {
        url = std::move(value);
    }

    [[nodiscard]] const std::string& get_url() const noexcept
    {
        return url;
    }

    void set_method(std::string value)
    {
        method = std::move(value);
    }

    [[nodiscard]] const std::string& get_method() const noexcept
    {
        return method;
    }

    std::string url{};
    std::string method{};
};


enum class VaultDetailsResponse
{
    Hyperswitch,
    Vgs,
};

enum class VaultSessionDetails
{
    Vgs,
    HyperswitchVault,
};

enum class VoucherData
{
    Boleto,
    Efecty,
    PagoEfectivo,
    RedCompra,
    RedPagos,
    Alfamart,
    Indomaret,
    Oxxo,
    SevenEleven,
    Lawson,
    MiniStop,
    FamilyMart,
    Seicomart,
    PayEasy,
};

enum class WalletResponseData
{
    ApplePay,
    GooglePay,
    SamsungPay,
    Paypal,
};

class WeChatPay : public core::behavior::Serializer<WeChatPay>
{
public:
};

class AchBillingDetails : public core::behavior::Serializer<AchBillingDetails>
{
public:
    void set_email(std::optional<EmailValue> value)
    {
        email = std::move(value);
    }

    [[nodiscard]] const std::optional<EmailValue>& get_email() const noexcept
    {
        return email;
    }

    std::optional<EmailValue> email{};
};

class AchTransfer : public core::behavior::Serializer<AchTransfer>
{
public:
    void set_account_number(SecretValue value)
    {
        account_number = std::move(value);
    }

    [[nodiscard]] const SecretValue& get_account_number() const noexcept
    {
        return account_number;
    }

    void set_bank_name(std::string value)
    {
        bank_name = std::move(value);
    }

    [[nodiscard]] const std::string& get_bank_name() const noexcept
    {
        return bank_name;
    }

    void set_routing_number(SecretValue value)
    {
        routing_number = std::move(value);
    }

    [[nodiscard]] const SecretValue& get_routing_number() const noexcept
    {
        return routing_number;
    }

    void set_swift_code(SecretValue value)
    {
        swift_code = std::move(value);
    }

    [[nodiscard]] const SecretValue& get_swift_code() const noexcept
    {
        return swift_code;
    }

    SecretValue account_number{};
    std::string bank_name{};
    SecretValue routing_number{};
    SecretValue swift_code{};
};

class AdditionalNetworkTokenInfo : public core::behavior::Serializer<AdditionalNetworkTokenInfo>
{
public:
    void set_card_issuer(std::optional<std::string> value)
    {
        card_issuer = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_card_issuer() const noexcept
    {
        return card_issuer;
    }

    void set_card_network(std::optional<CardNetworkValue> value)
    {
        card_network = std::move(value);
    }

    [[nodiscard]] const std::optional<CardNetworkValue>& get_card_network() const noexcept
    {
        return card_network;
    }

    void set_card_type(std::optional<std::string> value)
    {
        card_type = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_card_type() const noexcept
    {
        return card_type;
    }

    void set_card_issuing_country(std::optional<std::string> value)
    {
        card_issuing_country = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_card_issuing_country() const noexcept
    {
        return card_issuing_country;
    }

    void set_bank_code(std::optional<std::string> value)
    {
        bank_code = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_bank_code() const noexcept
    {
        return bank_code;
    }

    void set_last4(std::optional<std::string> value)
    {
        last4 = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_last4() const noexcept
    {
        return last4;
    }

    void set_token_isin(std::optional<std::string> value)
    {
        token_isin = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_token_isin() const noexcept
    {
        return token_isin;
    }

    void set_token_exp_month(std::optional<SecretValue> value)
    {
        token_exp_month = std::move(value);
    }

    [[nodiscard]] const std::optional<SecretValue>& get_token_exp_month() const noexcept
    {
        return token_exp_month;
    }

    void set_token_exp_year(std::optional<SecretValue> value)
    {
        token_exp_year = std::move(value);
    }

    [[nodiscard]] const std::optional<SecretValue>& get_token_exp_year() const noexcept
    {
        return token_exp_year;
    }

    void set_card_holder_name(std::optional<SecretValue> value)
    {
        card_holder_name = std::move(value);
    }

    [[nodiscard]] const std::optional<SecretValue>& get_card_holder_name() const noexcept
    {
        return card_holder_name;
    }

    void set_par(std::optional<SecretValue> value)
    {
        par = std::move(value);
    }

    [[nodiscard]] const std::optional<SecretValue>& get_par() const noexcept
    {
        return par;
    }

    std::optional<std::string> card_issuer{};
    std::optional<CardNetworkValue> card_network{};
    std::optional<std::string> card_type{};
    std::optional<std::string> card_issuing_country{};
    std::optional<std::string> bank_code{};
    std::optional<std::string> last4{};
    std::optional<std::string> token_isin{};
    std::optional<SecretValue> token_exp_month{};
    std::optional<SecretValue> token_exp_year{};
    std::optional<SecretValue> card_holder_name{};
    std::optional<SecretValue> par{};
};

class AlfamartVoucherData : public core::behavior::Serializer<AlfamartVoucherData>
{
public:
    void set_first_name(std::optional<SecretValue> value)
    {
        first_name = std::move(value);
    }

    [[nodiscard]] const std::optional<SecretValue>& get_first_name() const noexcept
    {
        return first_name;
    }

    void set_last_name(std::optional<SecretValue> value)
    {
        last_name = std::move(value);
    }

    [[nodiscard]] const std::optional<SecretValue>& get_last_name() const noexcept
    {
        return last_name;
    }

    void set_email(std::optional<EmailValue> value)
    {
        email = std::move(value);
    }

    [[nodiscard]] const std::optional<EmailValue>& get_email() const noexcept
    {
        return email;
    }

    std::optional<SecretValue> first_name{};
    std::optional<SecretValue> last_name{};
    std::optional<EmailValue> email{};
};

class AmountDetails : public core::behavior::Serializer<AmountDetails>
{
public:
    void set_order_amount(Amount value)
    {
        order_amount = std::move(value);
    }

    [[nodiscard]] const Amount& get_order_amount() const noexcept
    {
        return order_amount;
    }

    void set_currency(CurrencyValue value)
    {
        currency = std::move(value);
    }

    [[nodiscard]] const CurrencyValue& get_currency() const noexcept
    {
        return currency;
    }

    void set_shipping_cost(std::optional<MinorUnitValue> value)
    {
        shipping_cost = std::move(value);
    }

    [[nodiscard]] const std::optional<MinorUnitValue>& get_shipping_cost() const noexcept
    {
        return shipping_cost;
    }

    void set_order_tax_amount(std::optional<MinorUnitValue> value)
    {
        order_tax_amount = std::move(value);
    }

    [[nodiscard]] const std::optional<MinorUnitValue>& get_order_tax_amount() const noexcept
    {
        return order_tax_amount;
    }

    void set_skip_external_tax_calculation(TaxCalculationOverrideValue value)
    {
        skip_external_tax_calculation = std::move(value);
    }

    [[nodiscard]] const TaxCalculationOverrideValue&
    get_skip_external_tax_calculation() const noexcept
    {
        return skip_external_tax_calculation;
    }

    void set_skip_surcharge_calculation(SurchargeCalculationOverrideValue value)
    {
        skip_surcharge_calculation = std::move(value);
    }

    [[nodiscard]] const SurchargeCalculationOverrideValue&
    get_skip_surcharge_calculation() const noexcept
    {
        return skip_surcharge_calculation;
    }

    void set_surcharge_amount(std::optional<MinorUnitValue> value)
    {
        surcharge_amount = std::move(value);
    }

    [[nodiscard]] const std::optional<MinorUnitValue>& get_surcharge_amount() const noexcept
    {
        return surcharge_amount;
    }

    void set_tax_on_surcharge(std::optional<MinorUnitValue> value)
    {
        tax_on_surcharge = std::move(value);
    }

    [[nodiscard]] const std::optional<MinorUnitValue>& get_tax_on_surcharge() const noexcept
    {
        return tax_on_surcharge;
    }

    Amount order_amount{};
    CurrencyValue currency{};
    std::optional<MinorUnitValue> shipping_cost{};
    std::optional<MinorUnitValue> order_tax_amount{};
    TaxCalculationOverrideValue skip_external_tax_calculation{};
    SurchargeCalculationOverrideValue skip_surcharge_calculation{};
    std::optional<MinorUnitValue> surcharge_amount{};
    std::optional<MinorUnitValue> tax_on_surcharge{};
};

class AmountDetailsResponse : public core::behavior::Serializer<AmountDetailsResponse>
{
public:
    void set_order_amount(MinorUnitValue value)
    {
        order_amount = std::move(value);
    }

    [[nodiscard]] const MinorUnitValue& get_order_amount() const noexcept
    {
        return order_amount;
    }

    void set_currency(CurrencyValue value)
    {
        currency = std::move(value);
    }

    [[nodiscard]] const CurrencyValue& get_currency() const noexcept
    {
        return currency;
    }

    void set_shipping_cost(std::optional<MinorUnitValue> value)
    {
        shipping_cost = std::move(value);
    }

    [[nodiscard]] const std::optional<MinorUnitValue>& get_shipping_cost() const noexcept
    {
        return shipping_cost;
    }

    void set_order_tax_amount(std::optional<MinorUnitValue> value)
    {
        order_tax_amount = std::move(value);
    }

    [[nodiscard]] const std::optional<MinorUnitValue>& get_order_tax_amount() const noexcept
    {
        return order_tax_amount;
    }

    void set_external_tax_calculation(TaxCalculationOverrideValue value)
    {
        external_tax_calculation = std::move(value);
    }

    [[nodiscard]] const TaxCalculationOverrideValue& get_external_tax_calculation() const noexcept
    {
        return external_tax_calculation;
    }

    void set_surcharge_calculation(SurchargeCalculationOverrideValue value)
    {
        surcharge_calculation = std::move(value);
    }

    [[nodiscard]] const SurchargeCalculationOverrideValue&
    get_surcharge_calculation() const noexcept
    {
        return surcharge_calculation;
    }

    void set_surcharge_amount(std::optional<MinorUnitValue> value)
    {
        surcharge_amount = std::move(value);
    }

    [[nodiscard]] const std::optional<MinorUnitValue>& get_surcharge_amount() const noexcept
    {
        return surcharge_amount;
    }

    void set_tax_on_surcharge(std::optional<MinorUnitValue> value)
    {
        tax_on_surcharge = std::move(value);
    }

    [[nodiscard]] const std::optional<MinorUnitValue>& get_tax_on_surcharge() const noexcept
    {
        return tax_on_surcharge;
    }

    void set_amount_captured(std::optional<MinorUnitValue> value)
    {
        amount_captured = std::move(value);
    }

    [[nodiscard]] const std::optional<MinorUnitValue>& get_amount_captured() const noexcept
    {
        return amount_captured;
    }

    MinorUnitValue order_amount{};
    CurrencyValue currency{};
    std::optional<MinorUnitValue> shipping_cost{};
    std::optional<MinorUnitValue> order_tax_amount{};
    TaxCalculationOverrideValue external_tax_calculation{};
    SurchargeCalculationOverrideValue surcharge_calculation{};
    std::optional<MinorUnitValue> surcharge_amount{};
    std::optional<MinorUnitValue> tax_on_surcharge{};
    std::optional<MinorUnitValue> amount_captured{};
};

class AmountDetailsSetter : public core::behavior::Serializer<AmountDetailsSetter>
{
public:
    void set_order_amount(Amount value)
    {
        order_amount = std::move(value);
    }

    [[nodiscard]] const Amount& get_order_amount() const noexcept
    {
        return order_amount;
    }

    void set_currency(CurrencyValue value)
    {
        currency = std::move(value);
    }

    [[nodiscard]] const CurrencyValue& get_currency() const noexcept
    {
        return currency;
    }

    void set_shipping_cost(std::optional<MinorUnitValue> value)
    {
        shipping_cost = std::move(value);
    }

    [[nodiscard]] const std::optional<MinorUnitValue>& get_shipping_cost() const noexcept
    {
        return shipping_cost;
    }

    void set_order_tax_amount(std::optional<MinorUnitValue> value)
    {
        order_tax_amount = std::move(value);
    }

    [[nodiscard]] const std::optional<MinorUnitValue>& get_order_tax_amount() const noexcept
    {
        return order_tax_amount;
    }

    void set_skip_external_tax_calculation(TaxCalculationOverrideValue value)
    {
        skip_external_tax_calculation = std::move(value);
    }

    [[nodiscard]] const TaxCalculationOverrideValue&
    get_skip_external_tax_calculation() const noexcept
    {
        return skip_external_tax_calculation;
    }

    void set_skip_surcharge_calculation(SurchargeCalculationOverrideValue value)
    {
        skip_surcharge_calculation = std::move(value);
    }

    [[nodiscard]] const SurchargeCalculationOverrideValue&
    get_skip_surcharge_calculation() const noexcept
    {
        return skip_surcharge_calculation;
    }

    void set_surcharge_amount(std::optional<MinorUnitValue> value)
    {
        surcharge_amount = std::move(value);
    }

    [[nodiscard]] const std::optional<MinorUnitValue>& get_surcharge_amount() const noexcept
    {
        return surcharge_amount;
    }

    void set_tax_on_surcharge(std::optional<MinorUnitValue> value)
    {
        tax_on_surcharge = std::move(value);
    }

    [[nodiscard]] const std::optional<MinorUnitValue>& get_tax_on_surcharge() const noexcept
    {
        return tax_on_surcharge;
    }

    Amount order_amount{};
    CurrencyValue currency{};
    std::optional<MinorUnitValue> shipping_cost{};
    std::optional<MinorUnitValue> order_tax_amount{};
    TaxCalculationOverrideValue skip_external_tax_calculation{};
    SurchargeCalculationOverrideValue skip_surcharge_calculation{};
    std::optional<MinorUnitValue> surcharge_amount{};
    std::optional<MinorUnitValue> tax_on_surcharge{};
};

class AmountDetailsUpdate : public core::behavior::Serializer<AmountDetailsUpdate>
{
public:
    void set_order_amount(std::optional<Amount> value)
    {
        order_amount = std::move(value);
    }

    [[nodiscard]] const std::optional<Amount>& get_order_amount() const noexcept
    {
        return order_amount;
    }

    void set_currency(std::optional<CurrencyValue> value)
    {
        currency = std::move(value);
    }

    [[nodiscard]] const std::optional<CurrencyValue>& get_currency() const noexcept
    {
        return currency;
    }

    void set_shipping_cost(std::optional<MinorUnitValue> value)
    {
        shipping_cost = std::move(value);
    }

    [[nodiscard]] const std::optional<MinorUnitValue>& get_shipping_cost() const noexcept
    {
        return shipping_cost;
    }

    void set_order_tax_amount(std::optional<MinorUnitValue> value)
    {
        order_tax_amount = std::move(value);
    }

    [[nodiscard]] const std::optional<MinorUnitValue>& get_order_tax_amount() const noexcept
    {
        return order_tax_amount;
    }

    void set_skip_external_tax_calculation(std::optional<TaxCalculationOverrideValue> value)
    {
        skip_external_tax_calculation = std::move(value);
    }

    [[nodiscard]] const std::optional<TaxCalculationOverrideValue>&
    get_skip_external_tax_calculation() const noexcept
    {
        return skip_external_tax_calculation;
    }

    void set_skip_surcharge_calculation(std::optional<SurchargeCalculationOverrideValue> value)
    {
        skip_surcharge_calculation = std::move(value);
    }

    [[nodiscard]] const std::optional<SurchargeCalculationOverrideValue>&
    get_skip_surcharge_calculation() const noexcept
    {
        return skip_surcharge_calculation;
    }

    void set_surcharge_amount(std::optional<MinorUnitValue> value)
    {
        surcharge_amount = std::move(value);
    }

    [[nodiscard]] const std::optional<MinorUnitValue>& get_surcharge_amount() const noexcept
    {
        return surcharge_amount;
    }

    void set_tax_on_surcharge(std::optional<MinorUnitValue> value)
    {
        tax_on_surcharge = std::move(value);
    }

    [[nodiscard]] const std::optional<MinorUnitValue>& get_tax_on_surcharge() const noexcept
    {
        return tax_on_surcharge;
    }

    std::optional<Amount> order_amount{};
    std::optional<CurrencyValue> currency{};
    std::optional<MinorUnitValue> shipping_cost{};
    std::optional<MinorUnitValue> order_tax_amount{};
    std::optional<TaxCalculationOverrideValue> skip_external_tax_calculation{};
    std::optional<SurchargeCalculationOverrideValue> skip_surcharge_calculation{};
    std::optional<MinorUnitValue> surcharge_amount{};
    std::optional<MinorUnitValue> tax_on_surcharge{};
};

class ApiNetworkErrorDetails : public core::behavior::Serializer<ApiNetworkErrorDetails>
{
public:
    void set_name(std::optional<CardNetworkValue> value)
    {
        name = std::move(value);
    }

    [[nodiscard]] const std::optional<CardNetworkValue>& get_name() const noexcept
    {
        return name;
    }

    void set_advice_code(std::optional<std::string> value)
    {
        advice_code = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_advice_code() const noexcept
    {
        return advice_code;
    }

    void set_advice_message(std::optional<std::string> value)
    {
        advice_message = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_advice_message() const noexcept
    {
        return advice_message;
    }

    std::optional<CardNetworkValue> name{};
    std::optional<std::string> advice_code{};
    std::optional<std::string> advice_message{};
};

class ApiUnifiedErrorDetails : public core::behavior::Serializer<ApiUnifiedErrorDetails>
{
public:
    void set_category(std::optional<std::string> value)
    {
        category = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_category() const noexcept
    {
        return category;
    }

    void set_message(std::optional<std::string> value)
    {
        message = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_message() const noexcept
    {
        return message;
    }

    void set_standardised_code(std::optional<StandardisedCodeValue> value)
    {
        standardised_code = std::move(value);
    }

    [[nodiscard]] const std::optional<StandardisedCodeValue>& get_standardised_code() const noexcept
    {
        return standardised_code;
    }

    void set_description(std::optional<std::string> value)
    {
        description = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_description() const noexcept
    {
        return description;
    }

    void set_user_guidance_message(std::optional<std::string> value)
    {
        user_guidance_message = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_user_guidance_message() const noexcept
    {
        return user_guidance_message;
    }

    void set_recommended_action(std::optional<RecommendedActionValue> value)
    {
        recommended_action = std::move(value);
    }

    [[nodiscard]] const std::optional<RecommendedActionValue>&
    get_recommended_action() const noexcept
    {
        return recommended_action;
    }

    std::optional<std::string> category{};
    std::optional<std::string> message{};
    std::optional<StandardisedCodeValue> standardised_code{};
    std::optional<std::string> description{};
    std::optional<std::string> user_guidance_message{};
    std::optional<RecommendedActionValue> recommended_action{};
};

class AppliedOffer : public core::behavior::Serializer<AppliedOffer>
{
public:
    void set_offer_engine_merchant_id(std::string value)
    {
        offer_engine_merchant_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_offer_engine_merchant_id() const noexcept
    {
        return offer_engine_merchant_id;
    }

    void set_offer_engine_txn_id(std::string value)
    {
        offer_engine_txn_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_offer_engine_txn_id() const noexcept
    {
        return offer_engine_txn_id;
    }

    void set_offer_id(std::string value)
    {
        offer_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_offer_id() const noexcept
    {
        return offer_id;
    }

    void set_offer_amount(MinorUnitValue value)
    {
        offer_amount = std::move(value);
    }

    [[nodiscard]] const MinorUnitValue& get_offer_amount() const noexcept
    {
        return offer_amount;
    }

    void set_currency(CurrencyValue value)
    {
        currency = std::move(value);
    }

    [[nodiscard]] const CurrencyValue& get_currency() const noexcept
    {
        return currency;
    }

    std::string offer_engine_merchant_id{};
    std::string offer_engine_txn_id{};
    std::string offer_id{};
    MinorUnitValue offer_amount{};
    CurrencyValue currency{};
};

class AuthenticationForStartResponse :
    public core::behavior::Serializer<AuthenticationForStartResponse>
{
public:
    void set_authentication(UrlDetails value)
    {
        authentication = std::move(value);
    }

    [[nodiscard]] const UrlDetails& get_authentication() const noexcept
    {
        return authentication;
    }

    UrlDetails authentication{};
};

class BrowserInformation : public core::behavior::Serializer<BrowserInformation>
{
public:
    void set_color_depth(std::optional<std::uint8_t> value)
    {
        color_depth = std::move(value);
    }

    [[nodiscard]] const std::optional<std::uint8_t>& get_color_depth() const noexcept
    {
        return color_depth;
    }

    void set_java_enabled(std::optional<bool> value)
    {
        java_enabled = std::move(value);
    }

    [[nodiscard]] const std::optional<bool>& get_java_enabled() const noexcept
    {
        return java_enabled;
    }

    void set_java_script_enabled(std::optional<bool> value)
    {
        java_script_enabled = std::move(value);
    }

    [[nodiscard]] const std::optional<bool>& get_java_script_enabled() const noexcept
    {
        return java_script_enabled;
    }

    void set_language(std::optional<std::string> value)
    {
        language = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_language() const noexcept
    {
        return language;
    }

    void set_screen_height(std::optional<std::uint32_t> value)
    {
        screen_height = std::move(value);
    }

    [[nodiscard]] const std::optional<std::uint32_t>& get_screen_height() const noexcept
    {
        return screen_height;
    }

    void set_screen_width(std::optional<std::uint32_t> value)
    {
        screen_width = std::move(value);
    }

    [[nodiscard]] const std::optional<std::uint32_t>& get_screen_width() const noexcept
    {
        return screen_width;
    }

    void set_time_zone(std::optional<std::int32_t> value)
    {
        time_zone = std::move(value);
    }

    [[nodiscard]] const std::optional<std::int32_t>& get_time_zone() const noexcept
    {
        return time_zone;
    }

    void set_ip_address(std::optional<IpAddrValue> value)
    {
        ip_address = std::move(value);
    }

    [[nodiscard]] const std::optional<IpAddrValue>& get_ip_address() const noexcept
    {
        return ip_address;
    }

    void set_accept_header(std::optional<std::string> value)
    {
        accept_header = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_accept_header() const noexcept
    {
        return accept_header;
    }

    void set_user_agent(std::optional<std::string> value)
    {
        user_agent = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_user_agent() const noexcept
    {
        return user_agent;
    }

    void set_os_type(std::optional<std::string> value)
    {
        os_type = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_os_type() const noexcept
    {
        return os_type;
    }

    void set_os_version(std::optional<std::string> value)
    {
        os_version = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_os_version() const noexcept
    {
        return os_version;
    }

    void set_device_model(std::optional<std::string> value)
    {
        device_model = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_device_model() const noexcept
    {
        return device_model;
    }

    void set_accept_language(std::optional<std::string> value)
    {
        accept_language = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_accept_language() const noexcept
    {
        return accept_language;
    }

    void set_referer(std::optional<std::string> value)
    {
        referer = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_referer() const noexcept
    {
        return referer;
    }

    std::optional<std::uint8_t> color_depth{};
    std::optional<bool> java_enabled{};
    std::optional<bool> java_script_enabled{};
    std::optional<std::string> language{};
    std::optional<std::uint32_t> screen_height{};
    std::optional<std::uint32_t> screen_width{};
    std::optional<std::int32_t> time_zone{};
    std::optional<IpAddrValue> ip_address{};
    std::optional<std::string> accept_header{};
    std::optional<std::string> user_agent{};
    std::optional<std::string> os_type{};
    std::optional<std::string> os_version{};
    std::optional<std::string> device_model{};
    std::optional<std::string> accept_language{};
    std::optional<std::string> referer{};
};

class CaptureResponse : public core::behavior::Serializer<CaptureResponse>
{
public:
    void set_capture_id(std::string value)
    {
        capture_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_capture_id() const noexcept
    {
        return capture_id;
    }

    void set_status(CaptureStatusValue value)
    {
        status = std::move(value);
    }

    [[nodiscard]] const CaptureStatusValue& get_status() const noexcept
    {
        return status;
    }

    void set_amount(MinorUnitValue value)
    {
        amount = std::move(value);
    }

    [[nodiscard]] const MinorUnitValue& get_amount() const noexcept
    {
        return amount;
    }

    void set_currency(std::optional<CurrencyValue> value)
    {
        currency = std::move(value);
    }

    [[nodiscard]] const std::optional<CurrencyValue>& get_currency() const noexcept
    {
        return currency;
    }

    void set_connector(std::string value)
    {
        connector = std::move(value);
    }

    [[nodiscard]] const std::string& get_connector() const noexcept
    {
        return connector;
    }

    void set_authorized_attempt_id(std::string value)
    {
        authorized_attempt_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_authorized_attempt_id() const noexcept
    {
        return authorized_attempt_id;
    }

    void set_connector_capture_id(std::optional<std::string> value)
    {
        connector_capture_id = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_connector_capture_id() const noexcept
    {
        return connector_capture_id;
    }

    void set_capture_sequence(std::int16_t value)
    {
        capture_sequence = std::move(value);
    }

    [[nodiscard]] const std::int16_t& get_capture_sequence() const noexcept
    {
        return capture_sequence;
    }

    void set_error_message(std::optional<std::string> value)
    {
        error_message = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_error_message() const noexcept
    {
        return error_message;
    }

    void set_error_code(std::optional<std::string> value)
    {
        error_code = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_error_code() const noexcept
    {
        return error_code;
    }

    void set_error_reason(std::optional<std::string> value)
    {
        error_reason = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_error_reason() const noexcept
    {
        return error_reason;
    }

    void set_reference_id(std::optional<std::string> value)
    {
        reference_id = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_reference_id() const noexcept
    {
        return reference_id;
    }

    std::string capture_id{};
    CaptureStatusValue status{};
    MinorUnitValue amount{};
    std::optional<CurrencyValue> currency{};
    std::string connector{};
    std::string authorized_attempt_id{};
    std::optional<std::string> connector_capture_id{};
    std::int16_t capture_sequence{};
    std::optional<std::string> error_message{};
    std::optional<std::string> error_code{};
    std::optional<std::string> error_reason{};
    std::optional<std::string> reference_id{};
};

class CardResponse : public core::behavior::Serializer<CardResponse>
{
public:
    void set_last4(std::optional<std::string> value)
    {
        last4 = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_last4() const noexcept
    {
        return last4;
    }

    void set_card_type(std::optional<std::string> value)
    {
        card_type = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_card_type() const noexcept
    {
        return card_type;
    }

    void set_card_subtype(std::optional<std::string> value)
    {
        card_subtype = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_card_subtype() const noexcept
    {
        return card_subtype;
    }

    void set_card_segment_type(std::optional<CardSegmentTypeValue> value)
    {
        card_segment_type = std::move(value);
    }

    [[nodiscard]] const std::optional<CardSegmentTypeValue>& get_card_segment_type() const noexcept
    {
        return card_segment_type;
    }

    void set_funding_source(std::optional<FundingSourceValue> value)
    {
        funding_source = std::move(value);
    }

    [[nodiscard]] const std::optional<FundingSourceValue>& get_funding_source() const noexcept
    {
        return funding_source;
    }

    void set_card_network(std::optional<CardNetworkValue> value)
    {
        card_network = std::move(value);
    }

    [[nodiscard]] const std::optional<CardNetworkValue>& get_card_network() const noexcept
    {
        return card_network;
    }

    void set_card_issuer(std::optional<std::string> value)
    {
        card_issuer = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_card_issuer() const noexcept
    {
        return card_issuer;
    }

    void set_card_issuing_country(std::optional<std::string> value)
    {
        card_issuing_country = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_card_issuing_country() const noexcept
    {
        return card_issuing_country;
    }

    void set_card_isin(std::optional<std::string> value)
    {
        card_isin = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_card_isin() const noexcept
    {
        return card_isin;
    }

    void set_card_extended_bin(std::optional<std::string> value)
    {
        card_extended_bin = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_card_extended_bin() const noexcept
    {
        return card_extended_bin;
    }

    void set_card_exp_month(std::optional<SecretValue> value)
    {
        card_exp_month = std::move(value);
    }

    [[nodiscard]] const std::optional<SecretValue>& get_card_exp_month() const noexcept
    {
        return card_exp_month;
    }

    void set_card_exp_year(std::optional<SecretValue> value)
    {
        card_exp_year = std::move(value);
    }

    [[nodiscard]] const std::optional<SecretValue>& get_card_exp_year() const noexcept
    {
        return card_exp_year;
    }

    void set_card_holder_name(std::optional<SecretValue> value)
    {
        card_holder_name = std::move(value);
    }

    [[nodiscard]] const std::optional<SecretValue>& get_card_holder_name() const noexcept
    {
        return card_holder_name;
    }

    void set_payment_checks(std::optional<serde::Value> value)
    {
        payment_checks = std::move(value);
    }

    [[nodiscard]] const std::optional<serde::Value>& get_payment_checks() const noexcept
    {
        return payment_checks;
    }

    void set_authentication_data(std::optional<serde::Value> value)
    {
        authentication_data = std::move(value);
    }

    [[nodiscard]] const std::optional<serde::Value>& get_authentication_data() const noexcept
    {
        return authentication_data;
    }

    void set_auth_code(std::optional<std::string> value)
    {
        auth_code = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_auth_code() const noexcept
    {
        return auth_code;
    }

    std::optional<std::string> last4{};
    std::optional<std::string> card_type{};
    std::optional<std::string> card_subtype{};
    std::optional<CardSegmentTypeValue> card_segment_type{};
    std::optional<FundingSourceValue> funding_source{};
    std::optional<CardNetworkValue> card_network{};
    std::optional<std::string> card_issuer{};
    std::optional<std::string> card_issuing_country{};
    std::optional<std::string> card_isin{};
    std::optional<std::string> card_extended_bin{};
    std::optional<SecretValue> card_exp_month{};
    std::optional<SecretValue> card_exp_year{};
    std::optional<SecretValue> card_holder_name{};
    std::optional<serde::Value> payment_checks{};
    std::optional<serde::Value> authentication_data{};
    std::optional<std::string> auth_code{};
};

class CartesBancairesParams : public core::behavior::Serializer<CartesBancairesParams>
{
public:
    void set_cavv_algorithm(CavvAlgorithmValue value)
    {
        cavv_algorithm = std::move(value);
    }

    [[nodiscard]] const CavvAlgorithmValue& get_cavv_algorithm() const noexcept
    {
        return cavv_algorithm;
    }

    void set_cb_exemption(std::string value)
    {
        cb_exemption = std::move(value);
    }

    [[nodiscard]] const std::string& get_cb_exemption() const noexcept
    {
        return cb_exemption;
    }

    void set_cb_score(std::int32_t value)
    {
        cb_score = std::move(value);
    }

    [[nodiscard]] const std::int32_t& get_cb_score() const noexcept
    {
        return cb_score;
    }

    CavvAlgorithmValue cavv_algorithm{};
    std::string cb_exemption{};
    std::int32_t cb_score{};
};

class ClickToPaySessionResponse : public core::behavior::Serializer<ClickToPaySessionResponse>
{
public:
    void set_dpa_id(std::string value)
    {
        dpa_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_dpa_id() const noexcept
    {
        return dpa_id;
    }

    void set_dpa_name(std::string value)
    {
        dpa_name = std::move(value);
    }

    [[nodiscard]] const std::string& get_dpa_name() const noexcept
    {
        return dpa_name;
    }

    void set_locale(std::string value)
    {
        locale = std::move(value);
    }

    [[nodiscard]] const std::string& get_locale() const noexcept
    {
        return locale;
    }

    void set_card_brands(std::vector<CardNetworkValue> value)
    {
        card_brands = std::move(value);
    }

    [[nodiscard]] const std::vector<CardNetworkValue>& get_card_brands() const noexcept
    {
        return card_brands;
    }

    void set_acquirer_bin(std::string value)
    {
        acquirer_bin = std::move(value);
    }

    [[nodiscard]] const std::string& get_acquirer_bin() const noexcept
    {
        return acquirer_bin;
    }

    void set_acquirer_merchant_id(std::string value)
    {
        acquirer_merchant_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_acquirer_merchant_id() const noexcept
    {
        return acquirer_merchant_id;
    }

    void set_merchant_category_code(std::optional<std::string> value)
    {
        merchant_category_code = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_merchant_category_code() const noexcept
    {
        return merchant_category_code;
    }

    void set_merchant_country_code(std::string value)
    {
        merchant_country_code = std::move(value);
    }

    [[nodiscard]] const std::string& get_merchant_country_code() const noexcept
    {
        return merchant_country_code;
    }

    void set_transaction_amount(StringMajorUnitValue value)
    {
        transaction_amount = std::move(value);
    }

    [[nodiscard]] const StringMajorUnitValue& get_transaction_amount() const noexcept
    {
        return transaction_amount;
    }

    void set_transaction_currency_code(CurrencyValue value)
    {
        transaction_currency_code = std::move(value);
    }

    [[nodiscard]] const CurrencyValue& get_transaction_currency_code() const noexcept
    {
        return transaction_currency_code;
    }

    void set_phone_number(std::optional<SecretValue> value)
    {
        phone_number = std::move(value);
    }

    [[nodiscard]] const std::optional<SecretValue>& get_phone_number() const noexcept
    {
        return phone_number;
    }

    void set_email(std::optional<EmailValue> value)
    {
        email = std::move(value);
    }

    [[nodiscard]] const std::optional<EmailValue>& get_email() const noexcept
    {
        return email;
    }

    void set_phone_country_code(std::optional<std::string> value)
    {
        phone_country_code = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_phone_country_code() const noexcept
    {
        return phone_country_code;
    }

    void set_provider(std::optional<CtpServiceProviderValue> value)
    {
        provider = std::move(value);
    }

    [[nodiscard]] const std::optional<CtpServiceProviderValue>& get_provider() const noexcept
    {
        return provider;
    }

    void set_dpa_client_id(std::optional<std::string> value)
    {
        dpa_client_id = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_dpa_client_id() const noexcept
    {
        return dpa_client_id;
    }

    std::string dpa_id{};
    std::string dpa_name{};
    std::string locale{};
    std::vector<CardNetworkValue> card_brands{};
    std::string acquirer_bin{};
    std::string acquirer_merchant_id{};
    std::optional<std::string> merchant_category_code{};
    std::string merchant_country_code{};
    StringMajorUnitValue transaction_amount{};
    CurrencyValue transaction_currency_code{};
    std::optional<SecretValue> phone_number{};
    std::optional<EmailValue> email{};
    std::optional<std::string> phone_country_code{};
    std::optional<CtpServiceProviderValue> provider{};
    std::optional<std::string> dpa_client_id{};
};

class CryptoResponse : public core::behavior::Serializer<CryptoResponse>
{
public:
    void set_details(std::optional<CryptoData> value)
    {
        details = std::move(value);
    }

    [[nodiscard]] const std::optional<CryptoData>& get_details() const noexcept
    {
        return details;
    }

    std::optional<CryptoData> details{};
};

class CtpServiceDetails : public core::behavior::Serializer<CtpServiceDetails>
{
public:
    void set_merchant_transaction_id(std::optional<std::string> value)
    {
        merchant_transaction_id = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_merchant_transaction_id() const noexcept
    {
        return merchant_transaction_id;
    }

    void set_correlation_id(std::optional<std::string> value)
    {
        correlation_id = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_correlation_id() const noexcept
    {
        return correlation_id;
    }

    void set_x_src_flow_id(std::optional<std::string> value)
    {
        x_src_flow_id = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_x_src_flow_id() const noexcept
    {
        return x_src_flow_id;
    }

    void set_provider(std::optional<CtpServiceProviderValue> value)
    {
        provider = std::move(value);
    }

    [[nodiscard]] const std::optional<CtpServiceProviderValue>& get_provider() const noexcept
    {
        return provider;
    }

    void set_encrypted_payload(std::optional<SecretValue> value)
    {
        encrypted_payload = std::move(value);
    }

    [[nodiscard]] const std::optional<SecretValue>& get_encrypted_payload() const noexcept
    {
        return encrypted_payload;
    }

    std::optional<std::string> merchant_transaction_id{};
    std::optional<std::string> correlation_id{};
    std::optional<std::string> x_src_flow_id{};
    std::optional<CtpServiceProviderValue> provider{};
    std::optional<SecretValue> encrypted_payload{};
};

class CustomerDetails : public core::behavior::Serializer<CustomerDetails>
{
public:
    void set_id(std::optional<CustomerIdValue> value)
    {
        id = std::move(value);
    }

    [[nodiscard]] const std::optional<CustomerIdValue>& get_id() const noexcept
    {
        return id;
    }

    void set_name(std::optional<SecretValue> value)
    {
        name = std::move(value);
    }

    [[nodiscard]] const std::optional<SecretValue>& get_name() const noexcept
    {
        return name;
    }

    void set_email(std::optional<EmailValue> value)
    {
        email = std::move(value);
    }

    [[nodiscard]] const std::optional<EmailValue>& get_email() const noexcept
    {
        return email;
    }

    void set_phone(std::optional<SecretValue> value)
    {
        phone = std::move(value);
    }

    [[nodiscard]] const std::optional<SecretValue>& get_phone() const noexcept
    {
        return phone;
    }

    void set_phone_country_code(std::optional<std::string> value)
    {
        phone_country_code = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_phone_country_code() const noexcept
    {
        return phone_country_code;
    }

    void set_tax_registration_id(std::optional<SecretValue> value)
    {
        tax_registration_id = std::move(value);
    }

    [[nodiscard]] const std::optional<SecretValue>& get_tax_registration_id() const noexcept
    {
        return tax_registration_id;
    }

    void set_document_details(std::optional<CustomerDocumentDetailsValue> value)
    {
        document_details = std::move(value);
    }

    [[nodiscard]] const std::optional<CustomerDocumentDetailsValue>&
    get_document_details() const noexcept
    {
        return document_details;
    }

    void set_date_of_birth(std::optional<SecretValue> value)
    {
        date_of_birth = std::move(value);
    }

    [[nodiscard]] const std::optional<SecretValue>& get_date_of_birth() const noexcept
    {
        return date_of_birth;
    }

    std::optional<CustomerIdValue> id{};
    std::optional<SecretValue> name{};
    std::optional<EmailValue> email{};
    std::optional<SecretValue> phone{};
    std::optional<std::string> phone_country_code{};
    std::optional<SecretValue> tax_registration_id{};
    std::optional<CustomerDocumentDetailsValue> document_details{};
    std::optional<SecretValue> date_of_birth{};
};

class CustomerDetailsResponse : public core::behavior::Serializer<CustomerDetailsResponse>
{
public:
    void set_id(std::optional<CustomerIdValue> value)
    {
        id = std::move(value);
    }

    [[nodiscard]] const std::optional<CustomerIdValue>& get_id() const noexcept
    {
        return id;
    }

    void set_name(std::optional<SecretValue> value)
    {
        name = std::move(value);
    }

    [[nodiscard]] const std::optional<SecretValue>& get_name() const noexcept
    {
        return name;
    }

    void set_email(std::optional<EmailValue> value)
    {
        email = std::move(value);
    }

    [[nodiscard]] const std::optional<EmailValue>& get_email() const noexcept
    {
        return email;
    }

    void set_phone(std::optional<SecretValue> value)
    {
        phone = std::move(value);
    }

    [[nodiscard]] const std::optional<SecretValue>& get_phone() const noexcept
    {
        return phone;
    }

    void set_phone_country_code(std::optional<std::string> value)
    {
        phone_country_code = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_phone_country_code() const noexcept
    {
        return phone_country_code;
    }

    void set_customer_document_details(std::optional<CustomerDocumentDetailsValue> value)
    {
        customer_document_details = std::move(value);
    }

    [[nodiscard]] const std::optional<CustomerDocumentDetailsValue>&
    get_customer_document_details() const noexcept
    {
        return customer_document_details;
    }

    std::optional<CustomerIdValue> id{};
    std::optional<SecretValue> name{};
    std::optional<EmailValue> email{};
    std::optional<SecretValue> phone{};
    std::optional<std::string> phone_country_code{};
    std::optional<CustomerDocumentDetailsValue> customer_document_details{};
};

class DDCData : public core::behavior::Serializer<DDCData>
{
public:
    void set_iframe_url(UrlValue value)
    {
        iframe_url = std::move(value);
    }

    [[nodiscard]] const UrlValue& get_iframe_url() const noexcept
    {
        return iframe_url;
    }

    void set_timeout_ms(std::optional<std::int32_t> value)
    {
        timeout_ms = std::move(value);
    }

    [[nodiscard]] const std::optional<std::int32_t>& get_timeout_ms() const noexcept
    {
        return timeout_ms;
    }

    UrlValue iframe_url{};
    std::optional<std::int32_t> timeout_ms{};
};

class DisplayAmountOnSdk : public core::behavior::Serializer<DisplayAmountOnSdk>
{
public:
    void set_net_amount(StringMajorUnitValue value)
    {
        net_amount = std::move(value);
    }

    [[nodiscard]] const StringMajorUnitValue& get_net_amount() const noexcept
    {
        return net_amount;
    }

    void set_order_tax_amount(std::optional<StringMajorUnitValue> value)
    {
        order_tax_amount = std::move(value);
    }

    [[nodiscard]] const std::optional<StringMajorUnitValue>& get_order_tax_amount() const noexcept
    {
        return order_tax_amount;
    }

    void set_shipping_cost(std::optional<StringMajorUnitValue> value)
    {
        shipping_cost = std::move(value);
    }

    [[nodiscard]] const std::optional<StringMajorUnitValue>& get_shipping_cost() const noexcept
    {
        return shipping_cost;
    }

    StringMajorUnitValue net_amount{};
    std::optional<StringMajorUnitValue> order_tax_amount{};
    std::optional<StringMajorUnitValue> shipping_cost{};
};

class DocumentDetails : public core::behavior::Serializer<DocumentDetails>
{
public:
    void set_document_type(DocumentKindValue value)
    {
        document_type = std::move(value);
    }

    [[nodiscard]] const DocumentKindValue& get_document_type() const noexcept
    {
        return document_type;
    }

    void set_document_number(SecretValue value)
    {
        document_number = std::move(value);
    }

    [[nodiscard]] const SecretValue& get_document_number() const noexcept
    {
        return document_number;
    }

    DocumentKindValue document_type{};
    SecretValue document_number{};
};

class DokuBillingDetails : public core::behavior::Serializer<DokuBillingDetails>
{
public:
    void set_first_name(std::optional<SecretValue> value)
    {
        first_name = std::move(value);
    }

    [[nodiscard]] const std::optional<SecretValue>& get_first_name() const noexcept
    {
        return first_name;
    }

    void set_last_name(std::optional<SecretValue> value)
    {
        last_name = std::move(value);
    }

    [[nodiscard]] const std::optional<SecretValue>& get_last_name() const noexcept
    {
        return last_name;
    }

    void set_email(std::optional<EmailValue> value)
    {
        email = std::move(value);
    }

    [[nodiscard]] const std::optional<EmailValue>& get_email() const noexcept
    {
        return email;
    }

    std::optional<SecretValue> first_name{};
    std::optional<SecretValue> last_name{};
    std::optional<EmailValue> email{};
};

class EligibilityAmountDetails : public core::behavior::Serializer<EligibilityAmountDetails>
{
public:
    void set_total_amount(MinorUnitValue value)
    {
        total_amount = std::move(value);
    }

    [[nodiscard]] const MinorUnitValue& get_total_amount() const noexcept
    {
        return total_amount;
    }

    void set_net_amount(MinorUnitValue value)
    {
        net_amount = std::move(value);
    }

    [[nodiscard]] const MinorUnitValue& get_net_amount() const noexcept
    {
        return net_amount;
    }

    void set_currency(CurrencyValue value)
    {
        currency = std::move(value);
    }

    [[nodiscard]] const CurrencyValue& get_currency() const noexcept
    {
        return currency;
    }

    MinorUnitValue total_amount{};
    MinorUnitValue net_amount{};
    CurrencyValue currency{};
};

class EligibilityCard : public core::behavior::Serializer<EligibilityCard>
{
public:
    void set_card_number(CardNumberValue value)
    {
        card_number = std::move(value);
    }

    [[nodiscard]] const CardNumberValue& get_card_number() const noexcept
    {
        return card_number;
    }

    void set_card_exp_month(std::optional<SecretValue> value)
    {
        card_exp_month = std::move(value);
    }

    [[nodiscard]] const std::optional<SecretValue>& get_card_exp_month() const noexcept
    {
        return card_exp_month;
    }

    void set_card_exp_year(std::optional<SecretValue> value)
    {
        card_exp_year = std::move(value);
    }

    [[nodiscard]] const std::optional<SecretValue>& get_card_exp_year() const noexcept
    {
        return card_exp_year;
    }

    void set_card_cvc(std::optional<SecretValue> value)
    {
        card_cvc = std::move(value);
    }

    [[nodiscard]] const std::optional<SecretValue>& get_card_cvc() const noexcept
    {
        return card_cvc;
    }

    void set_card_holder_name(std::optional<SecretValue> value)
    {
        card_holder_name = std::move(value);
    }

    [[nodiscard]] const std::optional<SecretValue>& get_card_holder_name() const noexcept
    {
        return card_holder_name;
    }

    void set_card_issuer(std::optional<std::string> value)
    {
        card_issuer = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_card_issuer() const noexcept
    {
        return card_issuer;
    }

    void set_card_network(std::optional<CardNetworkValue> value)
    {
        card_network = std::move(value);
    }

    [[nodiscard]] const std::optional<CardNetworkValue>& get_card_network() const noexcept
    {
        return card_network;
    }

    void set_card_type(std::optional<std::string> value)
    {
        card_type = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_card_type() const noexcept
    {
        return card_type;
    }

    void set_card_issuing_country(std::optional<std::string> value)
    {
        card_issuing_country = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_card_issuing_country() const noexcept
    {
        return card_issuing_country;
    }

    void set_card_issuing_country_code(std::optional<std::string> value)
    {
        card_issuing_country_code = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_card_issuing_country_code() const noexcept
    {
        return card_issuing_country_code;
    }

    void set_bank_code(std::optional<std::string> value)
    {
        bank_code = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_bank_code() const noexcept
    {
        return bank_code;
    }

    void set_nick_name(std::optional<SecretValue> value)
    {
        nick_name = std::move(value);
    }

    [[nodiscard]] const std::optional<SecretValue>& get_nick_name() const noexcept
    {
        return nick_name;
    }

    CardNumberValue card_number{};
    std::optional<SecretValue> card_exp_month{};
    std::optional<SecretValue> card_exp_year{};
    std::optional<SecretValue> card_cvc{};
    std::optional<SecretValue> card_holder_name{};
    std::optional<std::string> card_issuer{};
    std::optional<CardNetworkValue> card_network{};
    std::optional<std::string> card_type{};
    std::optional<std::string> card_issuing_country{};
    std::optional<std::string> card_issuing_country_code{};
    std::optional<std::string> bank_code{};
    std::optional<SecretValue> nick_name{};
};

class EligibilityCardBin : public core::behavior::Serializer<EligibilityCardBin>
{
public:
    void set_card_bin(CardBinValue value)
    {
        card_bin = std::move(value);
    }

    [[nodiscard]] const CardBinValue& get_card_bin() const noexcept
    {
        return card_bin;
    }

    CardBinValue card_bin{};
};

class EligibleOffer : public core::behavior::Serializer<EligibleOffer>
{
public:
    void set_offer_quote_id(std::string value)
    {
        offer_quote_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_offer_quote_id() const noexcept
    {
        return offer_quote_id;
    }

    void set_offer_amount(MinorUnitValue value)
    {
        offer_amount = std::move(value);
    }

    [[nodiscard]] const MinorUnitValue& get_offer_amount() const noexcept
    {
        return offer_amount;
    }

    void set_currency(CurrencyValue value)
    {
        currency = std::move(value);
    }

    [[nodiscard]] const CurrencyValue& get_currency() const noexcept
    {
        return currency;
    }

    void set_code(std::string value)
    {
        code = std::move(value);
    }

    [[nodiscard]] const std::string& get_code() const noexcept
    {
        return code;
    }

    void set_title(std::optional<std::string> value)
    {
        title = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_title() const noexcept
    {
        return title;
    }

    void set_description(std::optional<std::string> value)
    {
        description = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_description() const noexcept
    {
        return description;
    }

    std::string offer_quote_id{};
    MinorUnitValue offer_amount{};
    CurrencyValue currency{};
    std::string code{};
    std::optional<std::string> title{};
    std::optional<std::string> description{};
};

class EncryptableAddressDetails : public core::behavior::Serializer<EncryptableAddressDetails>
{
public:
    void set_line1(OptionalEncryptableSecretStringValue value)
    {
        line1 = std::move(value);
    }

    [[nodiscard]] const OptionalEncryptableSecretStringValue& get_line1() const noexcept
    {
        return line1;
    }

    void set_line2(OptionalEncryptableSecretStringValue value)
    {
        line2 = std::move(value);
    }

    [[nodiscard]] const OptionalEncryptableSecretStringValue& get_line2() const noexcept
    {
        return line2;
    }

    void set_line3(OptionalEncryptableSecretStringValue value)
    {
        line3 = std::move(value);
    }

    [[nodiscard]] const OptionalEncryptableSecretStringValue& get_line3() const noexcept
    {
        return line3;
    }

    void set_state(OptionalEncryptableSecretStringValue value)
    {
        state = std::move(value);
    }

    [[nodiscard]] const OptionalEncryptableSecretStringValue& get_state() const noexcept
    {
        return state;
    }

    void set_zip(OptionalEncryptableSecretStringValue value)
    {
        zip = std::move(value);
    }

    [[nodiscard]] const OptionalEncryptableSecretStringValue& get_zip() const noexcept
    {
        return zip;
    }

    void set_first_name(OptionalEncryptableSecretStringValue value)
    {
        first_name = std::move(value);
    }

    [[nodiscard]] const OptionalEncryptableSecretStringValue& get_first_name() const noexcept
    {
        return first_name;
    }

    void set_last_name(OptionalEncryptableSecretStringValue value)
    {
        last_name = std::move(value);
    }

    [[nodiscard]] const OptionalEncryptableSecretStringValue& get_last_name() const noexcept
    {
        return last_name;
    }

    void set_phone_number(OptionalEncryptableSecretStringValue value)
    {
        phone_number = std::move(value);
    }

    [[nodiscard]] const OptionalEncryptableSecretStringValue& get_phone_number() const noexcept
    {
        return phone_number;
    }

    void set_email(OptionalEncryptableEmailValue value)
    {
        email = std::move(value);
    }

    [[nodiscard]] const OptionalEncryptableEmailValue& get_email() const noexcept
    {
        return email;
    }

    OptionalEncryptableSecretStringValue line1{};
    OptionalEncryptableSecretStringValue line2{};
    OptionalEncryptableSecretStringValue line3{};
    OptionalEncryptableSecretStringValue state{};
    OptionalEncryptableSecretStringValue zip{};
    OptionalEncryptableSecretStringValue first_name{};
    OptionalEncryptableSecretStringValue last_name{};
    OptionalEncryptableSecretStringValue phone_number{};
    OptionalEncryptableEmailValue email{};
};

class ExtendedCardInfo : public core::behavior::Serializer<ExtendedCardInfo>
{
public:
    void set_card_number(CardNumberValue value)
    {
        card_number = std::move(value);
    }

    [[nodiscard]] const CardNumberValue& get_card_number() const noexcept
    {
        return card_number;
    }

    void set_card_exp_month(SecretValue value)
    {
        card_exp_month = std::move(value);
    }

    [[nodiscard]] const SecretValue& get_card_exp_month() const noexcept
    {
        return card_exp_month;
    }

    void set_card_exp_year(SecretValue value)
    {
        card_exp_year = std::move(value);
    }

    [[nodiscard]] const SecretValue& get_card_exp_year() const noexcept
    {
        return card_exp_year;
    }

    void set_card_holder_name(std::optional<SecretValue> value)
    {
        card_holder_name = std::move(value);
    }

    [[nodiscard]] const std::optional<SecretValue>& get_card_holder_name() const noexcept
    {
        return card_holder_name;
    }

    void set_card_cvc(SecretValue value)
    {
        card_cvc = std::move(value);
    }

    [[nodiscard]] const SecretValue& get_card_cvc() const noexcept
    {
        return card_cvc;
    }

    void set_card_issuer(std::optional<std::string> value)
    {
        card_issuer = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_card_issuer() const noexcept
    {
        return card_issuer;
    }

    void set_card_network(std::optional<CardNetworkValue> value)
    {
        card_network = std::move(value);
    }

    [[nodiscard]] const std::optional<CardNetworkValue>& get_card_network() const noexcept
    {
        return card_network;
    }

    void set_card_type(std::optional<std::string> value)
    {
        card_type = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_card_type() const noexcept
    {
        return card_type;
    }

    void set_card_issuing_country(std::optional<std::string> value)
    {
        card_issuing_country = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_card_issuing_country() const noexcept
    {
        return card_issuing_country;
    }

    void set_bank_code(std::optional<std::string> value)
    {
        bank_code = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_bank_code() const noexcept
    {
        return bank_code;
    }

    CardNumberValue card_number{};
    SecretValue card_exp_month{};
    SecretValue card_exp_year{};
    std::optional<SecretValue> card_holder_name{};
    SecretValue card_cvc{};
    std::optional<std::string> card_issuer{};
    std::optional<CardNetworkValue> card_network{};
    std::optional<std::string> card_type{};
    std::optional<std::string> card_issuing_country{};
    std::optional<std::string> bank_code{};
};

class ExternalAuthenticationDetailsResponse :
    public core::behavior::Serializer<ExternalAuthenticationDetailsResponse>
{
public:
    void set_authentication_flow(std::optional<DecoupledAuthenticationTypeValue> value)
    {
        authentication_flow = std::move(value);
    }

    [[nodiscard]] const std::optional<DecoupledAuthenticationTypeValue>&
    get_authentication_flow() const noexcept
    {
        return authentication_flow;
    }

    void set_electronic_commerce_indicator(std::optional<std::string> value)
    {
        electronic_commerce_indicator = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>&
    get_electronic_commerce_indicator() const noexcept
    {
        return electronic_commerce_indicator;
    }

    void set_status(AuthenticationStatusValue value)
    {
        status = std::move(value);
    }

    [[nodiscard]] const AuthenticationStatusValue& get_status() const noexcept
    {
        return status;
    }

    void set_ds_transaction_id(std::optional<std::string> value)
    {
        ds_transaction_id = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_ds_transaction_id() const noexcept
    {
        return ds_transaction_id;
    }

    void set_version(std::optional<std::string> value)
    {
        version = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_version() const noexcept
    {
        return version;
    }

    void set_error_code(std::optional<std::string> value)
    {
        error_code = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_error_code() const noexcept
    {
        return error_code;
    }

    void set_error_message(std::optional<std::string> value)
    {
        error_message = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_error_message() const noexcept
    {
        return error_message;
    }

    void set_challenge_cancel_code(std::optional<std::string> value)
    {
        challenge_cancel_code = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_challenge_cancel_code() const noexcept
    {
        return challenge_cancel_code;
    }

    void set_trans_status_reason(std::optional<std::string> value)
    {
        trans_status_reason = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_trans_status_reason() const noexcept
    {
        return trans_status_reason;
    }

    std::optional<DecoupledAuthenticationTypeValue> authentication_flow{};
    std::optional<std::string> electronic_commerce_indicator{};
    AuthenticationStatusValue status{};
    std::optional<std::string> ds_transaction_id{};
    std::optional<std::string> version{};
    std::optional<std::string> error_code{};
    std::optional<std::string> error_message{};
    std::optional<std::string> challenge_cancel_code{};
    std::optional<std::string> trans_status_reason{};
};

class FlexibleAmountDetails : public core::behavior::Serializer<FlexibleAmountDetails>
{
public:
    void set_min_value(std::optional<StringMajorUnitValue> value)
    {
        min_value = std::move(value);
    }

    [[nodiscard]] const std::optional<StringMajorUnitValue>& get_min_value() const noexcept
    {
        return min_value;
    }

    void set_max_value(std::optional<StringMajorUnitValue> value)
    {
        max_value = std::move(value);
    }

    [[nodiscard]] const std::optional<StringMajorUnitValue>& get_max_value() const noexcept
    {
        return max_value;
    }

    void set_value_type(std::optional<CalculationType> value)
    {
        value_type = std::move(value);
    }

    [[nodiscard]] const std::optional<CalculationType>& get_value_type() const noexcept
    {
        return value_type;
    }

    std::optional<StringMajorUnitValue> min_value{};
    std::optional<StringMajorUnitValue> max_value{};
    std::optional<CalculationType> value_type{};
};

class HyperswitchVaultSessionDetails :
    public core::behavior::Serializer<HyperswitchVaultSessionDetails>
{
public:
    void set_sdk_authorization(SecretValue value)
    {
        sdk_authorization = std::move(value);
    }

    [[nodiscard]] const SecretValue& get_sdk_authorization() const noexcept
    {
        return sdk_authorization;
    }

    SecretValue sdk_authorization{};
};

class ImmediateExpirationTime : public core::behavior::Serializer<ImmediateExpirationTime>
{
public:
    void set_time(std::uint32_t value)
    {
        time = std::move(value);
    }

    [[nodiscard]] const std::uint32_t& get_time() const noexcept
    {
        return time;
    }

    void set_pix_key(std::optional<PixKeyValue> value)
    {
        pix_key = std::move(value);
    }

    [[nodiscard]] const std::optional<PixKeyValue>& get_pix_key() const noexcept
    {
        return pix_key;
    }

    std::uint32_t time{};
    std::optional<PixKeyValue> pix_key{};
};

class IncrementalAuthorizationResponse :
    public core::behavior::Serializer<IncrementalAuthorizationResponse>
{
public:
    void set_authorization_id(std::string value)
    {
        authorization_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_authorization_id() const noexcept
    {
        return authorization_id;
    }

    void set_amount(MinorUnitValue value)
    {
        amount = std::move(value);
    }

    [[nodiscard]] const MinorUnitValue& get_amount() const noexcept
    {
        return amount;
    }

    void set_status(AuthorizationStatusValue value)
    {
        status = std::move(value);
    }

    [[nodiscard]] const AuthorizationStatusValue& get_status() const noexcept
    {
        return status;
    }

    void set_error_code(std::optional<std::string> value)
    {
        error_code = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_error_code() const noexcept
    {
        return error_code;
    }

    void set_error_message(std::optional<std::string> value)
    {
        error_message = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_error_message() const noexcept
    {
        return error_message;
    }

    void set_previously_authorized_amount(MinorUnitValue value)
    {
        previously_authorized_amount = std::move(value);
    }

    [[nodiscard]] const MinorUnitValue& get_previously_authorized_amount() const noexcept
    {
        return previously_authorized_amount;
    }

    std::string authorization_id{};
    MinorUnitValue amount{};
    AuthorizationStatusValue status{};
    std::optional<std::string> error_code{};
    std::optional<std::string> error_message{};
    MinorUnitValue previously_authorized_amount{};
};

class IndomaretVoucherData : public core::behavior::Serializer<IndomaretVoucherData>
{
public:
    void set_first_name(std::optional<SecretValue> value)
    {
        first_name = std::move(value);
    }

    [[nodiscard]] const std::optional<SecretValue>& get_first_name() const noexcept
    {
        return first_name;
    }

    void set_last_name(std::optional<SecretValue> value)
    {
        last_name = std::move(value);
    }

    [[nodiscard]] const std::optional<SecretValue>& get_last_name() const noexcept
    {
        return last_name;
    }

    void set_email(std::optional<EmailValue> value)
    {
        email = std::move(value);
    }

    [[nodiscard]] const std::optional<EmailValue>& get_email() const noexcept
    {
        return email;
    }

    std::optional<SecretValue> first_name{};
    std::optional<SecretValue> last_name{};
    std::optional<EmailValue> email{};
};

class InstallmentDetails : public core::behavior::Serializer<InstallmentDetails>
{
public:
    void set_max_partial_payments(std::optional<std::uint32_t> value)
    {
        max_partial_payments = std::move(value);
    }

    [[nodiscard]] const std::optional<std::uint32_t>& get_max_partial_payments() const noexcept
    {
        return max_partial_payments;
    }

    void set_value_type(std::optional<CalculationType> value)
    {
        value_type = std::move(value);
    }

    [[nodiscard]] const std::optional<CalculationType>& get_value_type() const noexcept
    {
        return value_type;
    }

    std::optional<std::uint32_t> max_partial_payments{};
    std::optional<CalculationType> value_type{};
};

class InstallmentRequest : public core::behavior::Serializer<InstallmentRequest>
{
public:
    void set_number_of_installments(NonZeroU8Value value)
    {
        number_of_installments = std::move(value);
    }

    [[nodiscard]] const NonZeroU8Value& get_number_of_installments() const noexcept
    {
        return number_of_installments;
    }

    void set_billing_frequency(BillingFrequencyValue value)
    {
        billing_frequency = std::move(value);
    }

    [[nodiscard]] const BillingFrequencyValue& get_billing_frequency() const noexcept
    {
        return billing_frequency;
    }

    NonZeroU8Value number_of_installments{};
    BillingFrequencyValue billing_frequency{};
};

class JCSVoucherData : public core::behavior::Serializer<JCSVoucherData>
{
public:
    void set_first_name(std::optional<SecretValue> value)
    {
        first_name = std::move(value);
    }

    [[nodiscard]] const std::optional<SecretValue>& get_first_name() const noexcept
    {
        return first_name;
    }

    void set_last_name(std::optional<SecretValue> value)
    {
        last_name = std::move(value);
    }

    [[nodiscard]] const std::optional<SecretValue>& get_last_name() const noexcept
    {
        return last_name;
    }

    void set_email(std::optional<EmailValue> value)
    {
        email = std::move(value);
    }

    [[nodiscard]] const std::optional<EmailValue>& get_email() const noexcept
    {
        return email;
    }

    void set_phone_number(std::optional<std::string> value)
    {
        phone_number = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_phone_number() const noexcept
    {
        return phone_number;
    }

    std::optional<SecretValue> first_name{};
    std::optional<SecretValue> last_name{};
    std::optional<EmailValue> email{};
    std::optional<std::string> phone_number{};
};

class ListMethodsForPaymentsRequest :
    public core::behavior::Serializer<ListMethodsForPaymentsRequest>
{
public:
    void set_client_secret(std::optional<std::string> value)
    {
        client_secret = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_client_secret() const noexcept
    {
        return client_secret;
    }

    void set_accepted_countries(std::optional<std::vector<CountryAlpha2Value>> value)
    {
        accepted_countries = std::move(value);
    }

    [[nodiscard]] const std::optional<std::vector<CountryAlpha2Value>>&
    get_accepted_countries() const noexcept
    {
        return accepted_countries;
    }

    void set_amount(std::optional<MinorUnitValue> value)
    {
        amount = std::move(value);
    }

    [[nodiscard]] const std::optional<MinorUnitValue>& get_amount() const noexcept
    {
        return amount;
    }

    void set_accepted_currencies(std::optional<std::vector<CurrencyValue>> value)
    {
        accepted_currencies = std::move(value);
    }

    [[nodiscard]] const std::optional<std::vector<CurrencyValue>>&
    get_accepted_currencies() const noexcept
    {
        return accepted_currencies;
    }

    void set_recurring_enabled(std::optional<bool> value)
    {
        recurring_enabled = std::move(value);
    }

    [[nodiscard]] const std::optional<bool>& get_recurring_enabled() const noexcept
    {
        return recurring_enabled;
    }

    void set_card_networks(std::optional<std::vector<CardNetworkValue>> value)
    {
        card_networks = std::move(value);
    }

    [[nodiscard]] const std::optional<std::vector<CardNetworkValue>>&
    get_card_networks() const noexcept
    {
        return card_networks;
    }

    void set_limit(std::optional<std::int64_t> value)
    {
        limit = std::move(value);
    }

    [[nodiscard]] const std::optional<std::int64_t>& get_limit() const noexcept
    {
        return limit;
    }

    std::optional<std::string> client_secret{};
    std::optional<std::vector<CountryAlpha2Value>> accepted_countries{};
    std::optional<MinorUnitValue> amount{};
    std::optional<std::vector<CurrencyValue>> accepted_currencies{};
    std::optional<bool> recurring_enabled{};
    std::optional<std::vector<CardNetworkValue>> card_networks{};
    std::optional<std::int64_t> limit{};
};

class MandateAmountData : public core::behavior::Serializer<MandateAmountData>
{
public:
    void set_amount(MinorUnitValue value)
    {
        amount = std::move(value);
    }

    [[nodiscard]] const MinorUnitValue& get_amount() const noexcept
    {
        return amount;
    }

    void set_currency(CurrencyValue value)
    {
        currency = std::move(value);
    }

    [[nodiscard]] const CurrencyValue& get_currency() const noexcept
    {
        return currency;
    }

    void set_start_date(std::optional<PrimitiveDateTimeValue> value)
    {
        start_date = std::move(value);
    }

    [[nodiscard]] const std::optional<PrimitiveDateTimeValue>& get_start_date() const noexcept
    {
        return start_date;
    }

    void set_end_date(std::optional<PrimitiveDateTimeValue> value)
    {
        end_date = std::move(value);
    }

    [[nodiscard]] const std::optional<PrimitiveDateTimeValue>& get_end_date() const noexcept
    {
        return end_date;
    }

    void set_metadata(std::optional<SecretSerdeValueValue> value)
    {
        metadata = std::move(value);
    }

    [[nodiscard]] const std::optional<SecretSerdeValueValue>& get_metadata() const noexcept
    {
        return metadata;
    }

    MinorUnitValue amount{};
    CurrencyValue currency{};
    std::optional<PrimitiveDateTimeValue> start_date{};
    std::optional<PrimitiveDateTimeValue> end_date{};
    std::optional<SecretSerdeValueValue> metadata{};
};

class MandateData : public core::behavior::Serializer<MandateData>
{
public:
    void set_update_mandate_id(std::optional<std::string> value)
    {
        update_mandate_id = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_update_mandate_id() const noexcept
    {
        return update_mandate_id;
    }

    void set_customer_acceptance(std::optional<CustomerAcceptanceValue> value)
    {
        customer_acceptance = std::move(value);
    }

    [[nodiscard]] const std::optional<CustomerAcceptanceValue>&
    get_customer_acceptance() const noexcept
    {
        return customer_acceptance;
    }

    void set_mandate_type(std::optional<MandateType> value)
    {
        mandate_type = std::move(value);
    }

    [[nodiscard]] const std::optional<MandateType>& get_mandate_type() const noexcept
    {
        return mandate_type;
    }

    std::optional<std::string> update_mandate_id{};
    std::optional<CustomerAcceptanceValue> customer_acceptance{};
    std::optional<MandateType> mandate_type{};
};

class MifinityData : public core::behavior::Serializer<MifinityData>
{
public:
    void set_date_of_birth(SecretValue value)
    {
        date_of_birth = std::move(value);
    }

    [[nodiscard]] const SecretValue& get_date_of_birth() const noexcept
    {
        return date_of_birth;
    }

    void set_language_preference(std::optional<std::string> value)
    {
        language_preference = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_language_preference() const noexcept
    {
        return language_preference;
    }

    SecretValue date_of_birth{};
    std::optional<std::string> language_preference{};
};

class MobilePaymentNextStepData : public core::behavior::Serializer<MobilePaymentNextStepData>
{
public:
    void set_consent_data_required(MobilePaymentConsent value)
    {
        consent_data_required = std::move(value);
    }

    [[nodiscard]] const MobilePaymentConsent& get_consent_data_required() const noexcept
    {
        return consent_data_required;
    }

    MobilePaymentConsent consent_data_required{};
};

class MobilePaymentResponse : public core::behavior::Serializer<MobilePaymentResponse>
{
public:
    void set_details(std::optional<MobilePaymentData> value)
    {
        details = std::move(value);
    }

    [[nodiscard]] const std::optional<MobilePaymentData>& get_details() const noexcept
    {
        return details;
    }

    std::optional<MobilePaymentData> details{};
};

class MultibancoBillingDetails : public core::behavior::Serializer<MultibancoBillingDetails>
{
public:
    void set_email(std::optional<EmailValue> value)
    {
        email = std::move(value);
    }

    [[nodiscard]] const std::optional<EmailValue>& get_email() const noexcept
    {
        return email;
    }

    std::optional<EmailValue> email{};
};

class MultibancoTransferInstructions :
    public core::behavior::Serializer<MultibancoTransferInstructions>
{
public:
    void set_reference(SecretValue value)
    {
        reference = std::move(value);
    }

    [[nodiscard]] const SecretValue& get_reference() const noexcept
    {
        return reference;
    }

    void set_entity(std::string value)
    {
        entity = std::move(value);
    }

    [[nodiscard]] const std::string& get_entity() const noexcept
    {
        return entity;
    }

    SecretValue reference{};
    std::string entity{};
};

class NetworkTokenData : public core::behavior::Serializer<NetworkTokenData>
{
public:
    void set_network_token(NetworkTokenValue value)
    {
        network_token = std::move(value);
    }

    [[nodiscard]] const NetworkTokenValue& get_network_token() const noexcept
    {
        return network_token;
    }

    void set_token_exp_month(SecretValue value)
    {
        token_exp_month = std::move(value);
    }

    [[nodiscard]] const SecretValue& get_token_exp_month() const noexcept
    {
        return token_exp_month;
    }

    void set_token_exp_year(SecretValue value)
    {
        token_exp_year = std::move(value);
    }

    [[nodiscard]] const SecretValue& get_token_exp_year() const noexcept
    {
        return token_exp_year;
    }

    void set_token_cryptogram(SecretValue value)
    {
        token_cryptogram = std::move(value);
    }

    [[nodiscard]] const SecretValue& get_token_cryptogram() const noexcept
    {
        return token_cryptogram;
    }

    void set_card_network(std::optional<CardNetworkValue> value)
    {
        card_network = std::move(value);
    }

    [[nodiscard]] const std::optional<CardNetworkValue>& get_card_network() const noexcept
    {
        return card_network;
    }

    void set_card_type(std::optional<std::string> value)
    {
        card_type = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_card_type() const noexcept
    {
        return card_type;
    }

    void set_card_issuing_country(std::optional<std::string> value)
    {
        card_issuing_country = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_card_issuing_country() const noexcept
    {
        return card_issuing_country;
    }

    void set_bank_code(std::optional<std::string> value)
    {
        bank_code = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_bank_code() const noexcept
    {
        return bank_code;
    }

    void set_card_holder_name(std::optional<SecretValue> value)
    {
        card_holder_name = std::move(value);
    }

    [[nodiscard]] const std::optional<SecretValue>& get_card_holder_name() const noexcept
    {
        return card_holder_name;
    }

    void set_card_issuer(std::optional<std::string> value)
    {
        card_issuer = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_card_issuer() const noexcept
    {
        return card_issuer;
    }

    void set_nick_name(std::optional<SecretValue> value)
    {
        nick_name = std::move(value);
    }

    [[nodiscard]] const std::optional<SecretValue>& get_nick_name() const noexcept
    {
        return nick_name;
    }

    void set_eci(std::optional<std::string> value)
    {
        eci = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_eci() const noexcept
    {
        return eci;
    }

    void set_par(std::optional<SecretValue> value)
    {
        par = std::move(value);
    }

    [[nodiscard]] const std::optional<SecretValue>& get_par() const noexcept
    {
        return par;
    }

    NetworkTokenValue network_token{};
    SecretValue token_exp_month{};
    SecretValue token_exp_year{};
    SecretValue token_cryptogram{};
    std::optional<CardNetworkValue> card_network{};
    std::optional<std::string> card_type{};
    std::optional<std::string> card_issuing_country{};
    std::optional<std::string> bank_code{};
    std::optional<SecretValue> card_holder_name{};
    std::optional<std::string> card_issuer{};
    std::optional<SecretValue> nick_name{};
    std::optional<std::string> eci{};
    std::optional<SecretValue> par{};
};

class NetworkTokenResponse : public core::behavior::Serializer<NetworkTokenResponse>
{
public:
    void set_last4(std::optional<std::string> value)
    {
        last4 = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_last4() const noexcept
    {
        return last4;
    }

    void set_card_type(std::optional<std::string> value)
    {
        card_type = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_card_type() const noexcept
    {
        return card_type;
    }

    void set_card_network(std::optional<CardNetworkValue> value)
    {
        card_network = std::move(value);
    }

    [[nodiscard]] const std::optional<CardNetworkValue>& get_card_network() const noexcept
    {
        return card_network;
    }

    void set_token_isin(std::optional<std::string> value)
    {
        token_isin = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_token_isin() const noexcept
    {
        return token_isin;
    }

    void set_card_issuer(std::optional<std::string> value)
    {
        card_issuer = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_card_issuer() const noexcept
    {
        return card_issuer;
    }

    void set_card_issuing_country(std::optional<std::string> value)
    {
        card_issuing_country = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_card_issuing_country() const noexcept
    {
        return card_issuing_country;
    }

    void set_token_exp_month(std::optional<SecretValue> value)
    {
        token_exp_month = std::move(value);
    }

    [[nodiscard]] const std::optional<SecretValue>& get_token_exp_month() const noexcept
    {
        return token_exp_month;
    }

    void set_token_exp_year(std::optional<SecretValue> value)
    {
        token_exp_year = std::move(value);
    }

    [[nodiscard]] const std::optional<SecretValue>& get_token_exp_year() const noexcept
    {
        return token_exp_year;
    }

    void set_card_holder_name(std::optional<SecretValue> value)
    {
        card_holder_name = std::move(value);
    }

    [[nodiscard]] const std::optional<SecretValue>& get_card_holder_name() const noexcept
    {
        return card_holder_name;
    }

    void set_par(std::optional<SecretValue> value)
    {
        par = std::move(value);
    }

    [[nodiscard]] const std::optional<SecretValue>& get_par() const noexcept
    {
        return par;
    }

    std::optional<std::string> last4{};
    std::optional<std::string> card_type{};
    std::optional<CardNetworkValue> card_network{};
    std::optional<std::string> token_isin{};
    std::optional<std::string> card_issuer{};
    std::optional<std::string> card_issuing_country{};
    std::optional<SecretValue> token_exp_month{};
    std::optional<SecretValue> token_exp_year{};
    std::optional<SecretValue> card_holder_name{};
    std::optional<SecretValue> par{};
};

class OpenBankingResponse : public core::behavior::Serializer<OpenBankingResponse>
{
public:
    void set_details(std::optional<OpenBankingData> value)
    {
        details = std::move(value);
    }

    [[nodiscard]] const std::optional<OpenBankingData>& get_details() const noexcept
    {
        return details;
    }

    std::optional<OpenBankingData> details{};
};

class OrderDetailsWithAmount : public core::behavior::Serializer<OrderDetailsWithAmount>
{
public:
    void set_product_name(std::string value)
    {
        product_name = std::move(value);
    }

    [[nodiscard]] const std::string& get_product_name() const noexcept
    {
        return product_name;
    }

    void set_quantity(std::uint16_t value)
    {
        quantity = std::move(value);
    }

    [[nodiscard]] const std::uint16_t& get_quantity() const noexcept
    {
        return quantity;
    }

    void set_amount(MinorUnitValue value)
    {
        amount = std::move(value);
    }

    [[nodiscard]] const MinorUnitValue& get_amount() const noexcept
    {
        return amount;
    }

    void set_tax_rate(std::optional<double> value)
    {
        tax_rate = std::move(value);
    }

    [[nodiscard]] const std::optional<double>& get_tax_rate() const noexcept
    {
        return tax_rate;
    }

    void set_total_tax_amount(std::optional<MinorUnitValue> value)
    {
        total_tax_amount = std::move(value);
    }

    [[nodiscard]] const std::optional<MinorUnitValue>& get_total_tax_amount() const noexcept
    {
        return total_tax_amount;
    }

    void set_requires_shipping(std::optional<bool> value)
    {
        requires_shipping = std::move(value);
    }

    [[nodiscard]] const std::optional<bool>& get_requires_shipping() const noexcept
    {
        return requires_shipping;
    }

    void set_product_img_link(std::optional<std::string> value)
    {
        product_img_link = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_product_img_link() const noexcept
    {
        return product_img_link;
    }

    void set_product_id(std::optional<std::string> value)
    {
        product_id = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_product_id() const noexcept
    {
        return product_id;
    }

    void set_category(std::optional<std::string> value)
    {
        category = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_category() const noexcept
    {
        return category;
    }

    void set_sub_category(std::optional<std::string> value)
    {
        sub_category = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_sub_category() const noexcept
    {
        return sub_category;
    }

    void set_brand(std::optional<std::string> value)
    {
        brand = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_brand() const noexcept
    {
        return brand;
    }

    void set_product_type(std::optional<ProductTypeValue> value)
    {
        product_type = std::move(value);
    }

    [[nodiscard]] const std::optional<ProductTypeValue>& get_product_type() const noexcept
    {
        return product_type;
    }

    void set_product_tax_code(std::optional<std::string> value)
    {
        product_tax_code = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_product_tax_code() const noexcept
    {
        return product_tax_code;
    }

    void set_description(std::optional<std::string> value)
    {
        description = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_description() const noexcept
    {
        return description;
    }

    void set_sku(std::optional<std::string> value)
    {
        sku = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_sku() const noexcept
    {
        return sku;
    }

    void set_upc(std::optional<std::string> value)
    {
        upc = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_upc() const noexcept
    {
        return upc;
    }

    void set_commodity_code(std::optional<std::string> value)
    {
        commodity_code = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_commodity_code() const noexcept
    {
        return commodity_code;
    }

    void set_unit_of_measure(std::optional<std::string> value)
    {
        unit_of_measure = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_unit_of_measure() const noexcept
    {
        return unit_of_measure;
    }

    void set_total_amount(std::optional<MinorUnitValue> value)
    {
        total_amount = std::move(value);
    }

    [[nodiscard]] const std::optional<MinorUnitValue>& get_total_amount() const noexcept
    {
        return total_amount;
    }

    void set_discount_name(std::optional<std::string> value)
    {
        discount_name = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_discount_name() const noexcept
    {
        return discount_name;
    }

    void set_unit_discount_amount(std::optional<MinorUnitValue> value)
    {
        unit_discount_amount = std::move(value);
    }

    [[nodiscard]] const std::optional<MinorUnitValue>& get_unit_discount_amount() const noexcept
    {
        return unit_discount_amount;
    }

    void set_discount_percentage(std::optional<DiscountPercentageValue> value)
    {
        discount_percentage = std::move(value);
    }

    [[nodiscard]] const std::optional<DiscountPercentageValue>&
    get_discount_percentage() const noexcept
    {
        return discount_percentage;
    }

    void set_discount_type(std::optional<std::string> value)
    {
        discount_type = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_discount_type() const noexcept
    {
        return discount_type;
    }

    std::string product_name{};
    std::uint16_t quantity{};
    MinorUnitValue amount{};
    std::optional<double> tax_rate{};
    std::optional<MinorUnitValue> total_tax_amount{};
    std::optional<bool> requires_shipping{};
    std::optional<std::string> product_img_link{};
    std::optional<std::string> product_id{};
    std::optional<std::string> category{};
    std::optional<std::string> sub_category{};
    std::optional<std::string> brand{};
    std::optional<ProductTypeValue> product_type{};
    std::optional<std::string> product_tax_code{};
    std::optional<std::string> description{};
    std::optional<std::string> sku{};
    std::optional<std::string> upc{};
    std::optional<std::string> commodity_code{};
    std::optional<std::string> unit_of_measure{};
    std::optional<MinorUnitValue> total_amount{};
    std::optional<std::string> discount_name{};
    std::optional<MinorUnitValue> unit_discount_amount{};
    std::optional<DiscountPercentageValue> discount_percentage{};
    std::optional<std::string> discount_type{};
};

class OrderDetailsWithStringAmount : public core::behavior::Serializer<OrderDetailsWithStringAmount>
{
public:
    void set_product_name(std::string value)
    {
        product_name = std::move(value);
    }

    [[nodiscard]] const std::string& get_product_name() const noexcept
    {
        return product_name;
    }

    void set_quantity(std::uint16_t value)
    {
        quantity = std::move(value);
    }

    [[nodiscard]] const std::uint16_t& get_quantity() const noexcept
    {
        return quantity;
    }

    void set_amount(StringMajorUnitValue value)
    {
        amount = std::move(value);
    }

    [[nodiscard]] const StringMajorUnitValue& get_amount() const noexcept
    {
        return amount;
    }

    void set_product_img_link(std::optional<std::string> value)
    {
        product_img_link = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_product_img_link() const noexcept
    {
        return product_img_link;
    }

    std::string product_name{};
    std::uint16_t quantity{};
    StringMajorUnitValue amount{};
    std::optional<std::string> product_img_link{};
};

class PMBalanceCheckSuccessResponse :
    public core::behavior::Serializer<PMBalanceCheckSuccessResponse>
{
public:
    void set_balance(MinorUnitValue value)
    {
        balance = std::move(value);
    }

    [[nodiscard]] const MinorUnitValue& get_balance() const noexcept
    {
        return balance;
    }

    void set_applicable_amount(MinorUnitValue value)
    {
        applicable_amount = std::move(value);
    }

    [[nodiscard]] const MinorUnitValue& get_applicable_amount() const noexcept
    {
        return applicable_amount;
    }

    void set_currency(CurrencyValue value)
    {
        currency = std::move(value);
    }

    [[nodiscard]] const CurrencyValue& get_currency() const noexcept
    {
        return currency;
    }

    MinorUnitValue balance{};
    MinorUnitValue applicable_amount{};
    CurrencyValue currency{};
};

class PaylaterResponse : public core::behavior::Serializer<PaylaterResponse>
{
public:
    void set_klarna_sdk(std::optional<KlarnaSdkPaymentMethodResponse> value)
    {
        klarna_sdk = std::move(value);
    }

    [[nodiscard]] const std::optional<KlarnaSdkPaymentMethodResponse>&
    get_klarna_sdk() const noexcept
    {
        return klarna_sdk;
    }

    std::optional<KlarnaSdkPaymentMethodResponse> klarna_sdk{};
};

class PaymentAmountDetailsResponse : public core::behavior::Serializer<PaymentAmountDetailsResponse>
{
public:
    void set_order_amount(MinorUnitValue value)
    {
        order_amount = std::move(value);
    }

    [[nodiscard]] const MinorUnitValue& get_order_amount() const noexcept
    {
        return order_amount;
    }

    void set_currency(CurrencyValue value)
    {
        currency = std::move(value);
    }

    [[nodiscard]] const CurrencyValue& get_currency() const noexcept
    {
        return currency;
    }

    void set_shipping_cost(std::optional<MinorUnitValue> value)
    {
        shipping_cost = std::move(value);
    }

    [[nodiscard]] const std::optional<MinorUnitValue>& get_shipping_cost() const noexcept
    {
        return shipping_cost;
    }

    void set_order_tax_amount(std::optional<MinorUnitValue> value)
    {
        order_tax_amount = std::move(value);
    }

    [[nodiscard]] const std::optional<MinorUnitValue>& get_order_tax_amount() const noexcept
    {
        return order_tax_amount;
    }

    void set_external_tax_calculation(TaxCalculationOverrideValue value)
    {
        external_tax_calculation = std::move(value);
    }

    [[nodiscard]] const TaxCalculationOverrideValue& get_external_tax_calculation() const noexcept
    {
        return external_tax_calculation;
    }

    void set_surcharge_calculation(SurchargeCalculationOverrideValue value)
    {
        surcharge_calculation = std::move(value);
    }

    [[nodiscard]] const SurchargeCalculationOverrideValue&
    get_surcharge_calculation() const noexcept
    {
        return surcharge_calculation;
    }

    void set_surcharge_amount(std::optional<MinorUnitValue> value)
    {
        surcharge_amount = std::move(value);
    }

    [[nodiscard]] const std::optional<MinorUnitValue>& get_surcharge_amount() const noexcept
    {
        return surcharge_amount;
    }

    void set_tax_on_surcharge(std::optional<MinorUnitValue> value)
    {
        tax_on_surcharge = std::move(value);
    }

    [[nodiscard]] const std::optional<MinorUnitValue>& get_tax_on_surcharge() const noexcept
    {
        return tax_on_surcharge;
    }

    void set_net_amount(MinorUnitValue value)
    {
        net_amount = std::move(value);
    }

    [[nodiscard]] const MinorUnitValue& get_net_amount() const noexcept
    {
        return net_amount;
    }

    void set_amount_to_capture(std::optional<MinorUnitValue> value)
    {
        amount_to_capture = std::move(value);
    }

    [[nodiscard]] const std::optional<MinorUnitValue>& get_amount_to_capture() const noexcept
    {
        return amount_to_capture;
    }

    void set_amount_capturable(MinorUnitValue value)
    {
        amount_capturable = std::move(value);
    }

    [[nodiscard]] const MinorUnitValue& get_amount_capturable() const noexcept
    {
        return amount_capturable;
    }

    void set_amount_captured(std::optional<MinorUnitValue> value)
    {
        amount_captured = std::move(value);
    }

    [[nodiscard]] const std::optional<MinorUnitValue>& get_amount_captured() const noexcept
    {
        return amount_captured;
    }

    MinorUnitValue order_amount{};
    CurrencyValue currency{};
    std::optional<MinorUnitValue> shipping_cost{};
    std::optional<MinorUnitValue> order_tax_amount{};
    TaxCalculationOverrideValue external_tax_calculation{};
    SurchargeCalculationOverrideValue surcharge_calculation{};
    std::optional<MinorUnitValue> surcharge_amount{};
    std::optional<MinorUnitValue> tax_on_surcharge{};
    MinorUnitValue net_amount{};
    std::optional<MinorUnitValue> amount_to_capture{};
    MinorUnitValue amount_capturable{};
    std::optional<MinorUnitValue> amount_captured{};
};

class PaymentAttemptAmountDetails : public core::behavior::Serializer<PaymentAttemptAmountDetails>
{
public:
    void set_net_amount(MinorUnitValue value)
    {
        net_amount = std::move(value);
    }

    [[nodiscard]] const MinorUnitValue& get_net_amount() const noexcept
    {
        return net_amount;
    }

    void set_amount_to_capture(std::optional<MinorUnitValue> value)
    {
        amount_to_capture = std::move(value);
    }

    [[nodiscard]] const std::optional<MinorUnitValue>& get_amount_to_capture() const noexcept
    {
        return amount_to_capture;
    }

    void set_surcharge_amount(std::optional<MinorUnitValue> value)
    {
        surcharge_amount = std::move(value);
    }

    [[nodiscard]] const std::optional<MinorUnitValue>& get_surcharge_amount() const noexcept
    {
        return surcharge_amount;
    }

    void set_tax_on_surcharge(std::optional<MinorUnitValue> value)
    {
        tax_on_surcharge = std::move(value);
    }

    [[nodiscard]] const std::optional<MinorUnitValue>& get_tax_on_surcharge() const noexcept
    {
        return tax_on_surcharge;
    }

    void set_amount_capturable(MinorUnitValue value)
    {
        amount_capturable = std::move(value);
    }

    [[nodiscard]] const MinorUnitValue& get_amount_capturable() const noexcept
    {
        return amount_capturable;
    }

    void set_shipping_cost(std::optional<MinorUnitValue> value)
    {
        shipping_cost = std::move(value);
    }

    [[nodiscard]] const std::optional<MinorUnitValue>& get_shipping_cost() const noexcept
    {
        return shipping_cost;
    }

    void set_order_tax_amount(std::optional<MinorUnitValue> value)
    {
        order_tax_amount = std::move(value);
    }

    [[nodiscard]] const std::optional<MinorUnitValue>& get_order_tax_amount() const noexcept
    {
        return order_tax_amount;
    }

    void set_amount_captured(std::optional<MinorUnitValue> value)
    {
        amount_captured = std::move(value);
    }

    [[nodiscard]] const std::optional<MinorUnitValue>& get_amount_captured() const noexcept
    {
        return amount_captured;
    }

    MinorUnitValue net_amount{};
    std::optional<MinorUnitValue> amount_to_capture{};
    std::optional<MinorUnitValue> surcharge_amount{};
    std::optional<MinorUnitValue> tax_on_surcharge{};
    MinorUnitValue amount_capturable{};
    std::optional<MinorUnitValue> shipping_cost{};
    std::optional<MinorUnitValue> order_tax_amount{};
    std::optional<MinorUnitValue> amount_captured{};
};

class PaymentAttemptListRequest : public core::behavior::Serializer<PaymentAttemptListRequest>
{
public:
    void set_payment_intent_id(GlobalPaymentIdValue value)
    {
        payment_intent_id = std::move(value);
    }

    [[nodiscard]] const GlobalPaymentIdValue& get_payment_intent_id() const noexcept
    {
        return payment_intent_id;
    }

    GlobalPaymentIdValue payment_intent_id{};
};

class PaymentCreatePaymentLinkConfig :
    public core::behavior::Serializer<PaymentCreatePaymentLinkConfig>
{
public:
    void set_theme_config(PaymentLinkConfigRequestValue value)
    {
        theme_config = std::move(value);
    }

    [[nodiscard]] const PaymentLinkConfigRequestValue& get_theme_config() const noexcept
    {
        return theme_config;
    }

    PaymentLinkConfigRequestValue theme_config{};
};

class PaymentLinkInitiateRequest : public core::behavior::Serializer<PaymentLinkInitiateRequest>
{
public:
    void set_merchant_id(MerchantIdValue value)
    {
        merchant_id = std::move(value);
    }

    [[nodiscard]] const MerchantIdValue& get_merchant_id() const noexcept
    {
        return merchant_id;
    }

    void set_payment_id(PaymentIdValue value)
    {
        payment_id = std::move(value);
    }

    [[nodiscard]] const PaymentIdValue& get_payment_id() const noexcept
    {
        return payment_id;
    }

    MerchantIdValue merchant_id{};
    PaymentIdValue payment_id{};
};

class PaymentLinkListConstraints : public core::behavior::Serializer<PaymentLinkListConstraints>
{
public:
    void set_limit(PageSizeValue value)
    {
        limit = std::move(value);
    }

    [[nodiscard]] const PageSizeValue& get_limit() const noexcept
    {
        return limit;
    }

    void set_offset(PageOffsetValue value)
    {
        offset = std::move(value);
    }

    [[nodiscard]] const PageOffsetValue& get_offset() const noexcept
    {
        return offset;
    }

    void set_time_range(std::optional<TimeRangeValue> value)
    {
        time_range = std::move(value);
    }

    [[nodiscard]] const std::optional<TimeRangeValue>& get_time_range() const noexcept
    {
        return time_range;
    }

    PageSizeValue limit{};
    PageOffsetValue offset{};
    std::optional<TimeRangeValue> time_range{};
};

enum class PaymentLinkStatusWrap
{
    PaymentLinkStatus,
    IntentStatus,
};

class PaymentListFilters : public core::behavior::Serializer<PaymentListFilters>
{
public:
    void set_connector(std::vector<std::string> value)
    {
        connector = std::move(value);
    }

    [[nodiscard]] const std::vector<std::string>& get_connector() const noexcept
    {
        return connector;
    }

    void set_currency(std::vector<CurrencyValue> value)
    {
        currency = std::move(value);
    }

    [[nodiscard]] const std::vector<CurrencyValue>& get_currency() const noexcept
    {
        return currency;
    }

    void set_status(std::vector<IntentStatusValue> value)
    {
        status = std::move(value);
    }

    [[nodiscard]] const std::vector<IntentStatusValue>& get_status() const noexcept
    {
        return status;
    }

    void set_payment_method(std::vector<PaymentMethodValue> value)
    {
        payment_method = std::move(value);
    }

    [[nodiscard]] const std::vector<PaymentMethodValue>& get_payment_method() const noexcept
    {
        return payment_method;
    }

    void set_payment_method_type(std::vector<PaymentMethodTypeValue> value)
    {
        payment_method_type = std::move(value);
    }

    [[nodiscard]] const std::vector<PaymentMethodTypeValue>&
    get_payment_method_type() const noexcept
    {
        return payment_method_type;
    }

    void set_authentication_type(std::vector<AuthenticationTypeValue> value)
    {
        authentication_type = std::move(value);
    }

    [[nodiscard]] const std::vector<AuthenticationTypeValue>&
    get_authentication_type() const noexcept
    {
        return authentication_type;
    }

    std::vector<std::string> connector{};
    std::vector<CurrencyValue> currency{};
    std::vector<IntentStatusValue> status{};
    std::vector<PaymentMethodValue> payment_method{};
    std::vector<PaymentMethodTypeValue> payment_method_type{};
    std::vector<AuthenticationTypeValue> authentication_type{};
};

class PaymentListFiltersV2 : public core::behavior::Serializer<PaymentListFiltersV2>
{
public:
    void set_connector(
        std::unordered_map<std::string, std::vector<MerchantConnectorInfoValue>> value
    )
    {
        connector = std::move(value);
    }

    [[nodiscard]] const std::unordered_map<std::string, std::vector<MerchantConnectorInfoValue>>&
    get_connector() const noexcept
    {
        return connector;
    }

    void set_currency(std::vector<CurrencyValue> value)
    {
        currency = std::move(value);
    }

    [[nodiscard]] const std::vector<CurrencyValue>& get_currency() const noexcept
    {
        return currency;
    }

    void set_status(std::vector<IntentStatusValue> value)
    {
        status = std::move(value);
    }

    [[nodiscard]] const std::vector<IntentStatusValue>& get_status() const noexcept
    {
        return status;
    }

    void set_payment_method(
        std::unordered_map<std::string, std::vector<PaymentMethodTypeValue>> value
    )
    {
        payment_method = std::move(value);
    }

    [[nodiscard]] const std::unordered_map<std::string, std::vector<PaymentMethodTypeValue>>&
    get_payment_method() const noexcept
    {
        return payment_method;
    }

    void set_authentication_type(std::vector<AuthenticationTypeValue> value)
    {
        authentication_type = std::move(value);
    }

    [[nodiscard]] const std::vector<AuthenticationTypeValue>&
    get_authentication_type() const noexcept
    {
        return authentication_type;
    }

    void set_card_network(std::vector<CardNetworkValue> value)
    {
        card_network = std::move(value);
    }

    [[nodiscard]] const std::vector<CardNetworkValue>& get_card_network() const noexcept
    {
        return card_network;
    }

    void set_card_discovery(std::vector<CardDiscoveryValue> value)
    {
        card_discovery = std::move(value);
    }

    [[nodiscard]] const std::vector<CardDiscoveryValue>& get_card_discovery() const noexcept
    {
        return card_discovery;
    }

    std::unordered_map<std::string, std::vector<MerchantConnectorInfoValue>> connector{};
    std::vector<CurrencyValue> currency{};
    std::vector<IntentStatusValue> status{};
    std::unordered_map<std::string, std::vector<PaymentMethodTypeValue>> payment_method{};
    std::vector<AuthenticationTypeValue> authentication_type{};
    std::vector<CardNetworkValue> card_network{};
    std::vector<CardDiscoveryValue> card_discovery{};
};

class PaymentMethodTokenizationDetails :
    public core::behavior::Serializer<PaymentMethodTokenizationDetails>
{
public:
    void set_payment_method_id(std::string value)
    {
        payment_method_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_payment_method_id() const noexcept
    {
        return payment_method_id;
    }

    void set_payment_method_status(PaymentMethodStatusValue value)
    {
        payment_method_status = std::move(value);
    }

    [[nodiscard]] const PaymentMethodStatusValue& get_payment_method_status() const noexcept
    {
        return payment_method_status;
    }

    void set_psp_tokenization(bool value)
    {
        psp_tokenization = std::move(value);
    }

    [[nodiscard]] const bool& get_psp_tokenization() const noexcept
    {
        return psp_tokenization;
    }

    void set_network_tokenization(bool value)
    {
        network_tokenization = std::move(value);
    }

    [[nodiscard]] const bool& get_network_tokenization() const noexcept
    {
        return network_tokenization;
    }

    void set_network_transaction_id(std::optional<std::string> value)
    {
        network_transaction_id = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_network_transaction_id() const noexcept
    {
        return network_transaction_id;
    }

    void set_network_transaction_link_id(std::optional<std::string> value)
    {
        network_transaction_link_id = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_network_transaction_link_id() const noexcept
    {
        return network_transaction_link_id;
    }

    void set_is_eligible_for_mit_payment(bool value)
    {
        is_eligible_for_mit_payment = std::move(value);
    }

    [[nodiscard]] const bool& get_is_eligible_for_mit_payment() const noexcept
    {
        return is_eligible_for_mit_payment;
    }

    std::string payment_method_id{};
    PaymentMethodStatusValue payment_method_status{};
    bool psp_tokenization{};
    bool network_tokenization{};
    std::optional<std::string> network_transaction_id{};
    std::optional<std::string> network_transaction_link_id{};
    bool is_eligible_for_mit_payment{};
};

class PaymentProcessingDetails : public core::behavior::Serializer<PaymentProcessingDetails>
{
public:
    void set_payment_processing_certificate(SecretValue value)
    {
        payment_processing_certificate = std::move(value);
    }

    [[nodiscard]] const SecretValue& get_payment_processing_certificate() const noexcept
    {
        return payment_processing_certificate;
    }

    void set_payment_processing_certificate_key(SecretValue value)
    {
        payment_processing_certificate_key = std::move(value);
    }

    [[nodiscard]] const SecretValue& get_payment_processing_certificate_key() const noexcept
    {
        return payment_processing_certificate_key;
    }

    SecretValue payment_processing_certificate{};
    SecretValue payment_processing_certificate_key{};
};

class PaymentRetrieveBody : public core::behavior::Serializer<PaymentRetrieveBody>
{
public:
    void set_merchant_id(std::optional<MerchantIdValue> value)
    {
        merchant_id = std::move(value);
    }

    [[nodiscard]] const std::optional<MerchantIdValue>& get_merchant_id() const noexcept
    {
        return merchant_id;
    }

    void set_force_sync(std::optional<bool> value)
    {
        force_sync = std::move(value);
    }

    [[nodiscard]] const std::optional<bool>& get_force_sync() const noexcept
    {
        return force_sync;
    }

    void set_client_secret(std::optional<std::string> value)
    {
        client_secret = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_client_secret() const noexcept
    {
        return client_secret;
    }

    void set_expand_captures(std::optional<bool> value)
    {
        expand_captures = std::move(value);
    }

    [[nodiscard]] const std::optional<bool>& get_expand_captures() const noexcept
    {
        return expand_captures;
    }

    void set_expand_attempts(std::optional<bool> value)
    {
        expand_attempts = std::move(value);
    }

    [[nodiscard]] const std::optional<bool>& get_expand_attempts() const noexcept
    {
        return expand_attempts;
    }

    void set_all_keys_required(std::optional<bool> value)
    {
        all_keys_required = std::move(value);
    }

    [[nodiscard]] const std::optional<bool>& get_all_keys_required() const noexcept
    {
        return all_keys_required;
    }

    std::optional<MerchantIdValue> merchant_id{};
    std::optional<bool> force_sync{};
    std::optional<std::string> client_secret{};
    std::optional<bool> expand_captures{};
    std::optional<bool> expand_attempts{};
    std::optional<bool> all_keys_required{};
};

class PaymentRetrieveBodyWithCredentials :
    public core::behavior::Serializer<PaymentRetrieveBodyWithCredentials>
{
public:
    void set_payment_id(PaymentIdValue value)
    {
        payment_id = std::move(value);
    }

    [[nodiscard]] const PaymentIdValue& get_payment_id() const noexcept
    {
        return payment_id;
    }

    void set_merchant_id(std::optional<MerchantIdValue> value)
    {
        merchant_id = std::move(value);
    }

    [[nodiscard]] const std::optional<MerchantIdValue>& get_merchant_id() const noexcept
    {
        return merchant_id;
    }

    void set_force_sync(std::optional<bool> value)
    {
        force_sync = std::move(value);
    }

    [[nodiscard]] const std::optional<bool>& get_force_sync() const noexcept
    {
        return force_sync;
    }

    void set_merchant_connector_details(std::optional<MerchantConnectorDetailsWrapValue> value)
    {
        merchant_connector_details = std::move(value);
    }

    [[nodiscard]] const std::optional<MerchantConnectorDetailsWrapValue>&
    get_merchant_connector_details() const noexcept
    {
        return merchant_connector_details;
    }

    PaymentIdValue payment_id{};
    std::optional<MerchantIdValue> merchant_id{};
    std::optional<bool> force_sync{};
    std::optional<MerchantConnectorDetailsWrapValue> merchant_connector_details{};
};

class PaymentSurchargeDetailsResponse :
    public core::behavior::Serializer<PaymentSurchargeDetailsResponse>
{
public:
    void set_surcharge_amount(MinorUnitValue value)
    {
        surcharge_amount = std::move(value);
    }

    [[nodiscard]] const MinorUnitValue& get_surcharge_amount() const noexcept
    {
        return surcharge_amount;
    }

    void set_tax_amount(std::optional<MinorUnitValue> value)
    {
        tax_amount = std::move(value);
    }

    [[nodiscard]] const std::optional<MinorUnitValue>& get_tax_amount() const noexcept
    {
        return tax_amount;
    }

    void set_surcharge_percentage(std::optional<double> value)
    {
        surcharge_percentage = std::move(value);
    }

    [[nodiscard]] const std::optional<double>& get_surcharge_percentage() const noexcept
    {
        return surcharge_percentage;
    }

    void set_error_details(std::optional<SurchargeErrorDetailsValue> value)
    {
        error_details = std::move(value);
    }

    [[nodiscard]] const std::optional<SurchargeErrorDetailsValue>&
    get_error_details() const noexcept
    {
        return error_details;
    }

    MinorUnitValue surcharge_amount{};
    std::optional<MinorUnitValue> tax_amount{};
    std::optional<double> surcharge_percentage{};
    std::optional<SurchargeErrorDetailsValue> error_details{};
};

class PaymentsApproveRequest : public core::behavior::Serializer<PaymentsApproveRequest>
{
public:
    void set_payment_id(PaymentIdValue value)
    {
        payment_id = std::move(value);
    }

    [[nodiscard]] const PaymentIdValue& get_payment_id() const noexcept
    {
        return payment_id;
    }

    PaymentIdValue payment_id{};
};

class PaymentsExtendAuthorizationRequest :
    public core::behavior::Serializer<PaymentsExtendAuthorizationRequest>
{
public:
    void set_payment_id(PaymentIdValue value)
    {
        payment_id = std::move(value);
    }

    [[nodiscard]] const PaymentIdValue& get_payment_id() const noexcept
    {
        return payment_id;
    }

    PaymentIdValue payment_id{};
};

class PaymentsExternalAuthenticationResponse :
    public core::behavior::Serializer<PaymentsExternalAuthenticationResponse>
{
public:
    void set_transaction_status(TransactionStatusValue value)
    {
        transaction_status = std::move(value);
    }

    [[nodiscard]] const TransactionStatusValue& get_transaction_status() const noexcept
    {
        return transaction_status;
    }

    void set_acs_url(std::optional<std::string> value)
    {
        acs_url = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_acs_url() const noexcept
    {
        return acs_url;
    }

    void set_challenge_request(std::optional<std::string> value)
    {
        challenge_request = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_challenge_request() const noexcept
    {
        return challenge_request;
    }

    void set_challenge_request_key(std::optional<std::string> value)
    {
        challenge_request_key = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_challenge_request_key() const noexcept
    {
        return challenge_request_key;
    }

    void set_acs_reference_number(std::optional<std::string> value)
    {
        acs_reference_number = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_acs_reference_number() const noexcept
    {
        return acs_reference_number;
    }

    void set_acs_trans_id(std::optional<std::string> value)
    {
        acs_trans_id = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_acs_trans_id() const noexcept
    {
        return acs_trans_id;
    }

    void set_three_dsserver_trans_id(std::optional<std::string> value)
    {
        three_dsserver_trans_id = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_three_dsserver_trans_id() const noexcept
    {
        return three_dsserver_trans_id;
    }

    void set_acs_signed_content(std::optional<std::string> value)
    {
        acs_signed_content = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_acs_signed_content() const noexcept
    {
        return acs_signed_content;
    }

    void set_three_ds_requestor_url(std::string value)
    {
        three_ds_requestor_url = std::move(value);
    }

    [[nodiscard]] const std::string& get_three_ds_requestor_url() const noexcept
    {
        return three_ds_requestor_url;
    }

    void set_three_ds_requestor_app_url(std::optional<std::string> value)
    {
        three_ds_requestor_app_url = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_three_ds_requestor_app_url() const noexcept
    {
        return three_ds_requestor_app_url;
    }

    void set_error_message(std::optional<std::string> value)
    {
        error_message = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_error_message() const noexcept
    {
        return error_message;
    }

    TransactionStatusValue transaction_status{};
    std::optional<std::string> acs_url{};
    std::optional<std::string> challenge_request{};
    std::optional<std::string> challenge_request_key{};
    std::optional<std::string> acs_reference_number{};
    std::optional<std::string> acs_trans_id{};
    std::optional<std::string> three_dsserver_trans_id{};
    std::optional<std::string> acs_signed_content{};
    std::string three_ds_requestor_url{};
    std::optional<std::string> three_ds_requestor_app_url{};
    std::optional<std::string> error_message{};
};

class PaymentsGetIntentRequest : public core::behavior::Serializer<PaymentsGetIntentRequest>
{
public:
    void set_id(GlobalPaymentIdValue value)
    {
        id = std::move(value);
    }

    [[nodiscard]] const GlobalPaymentIdValue& get_id() const noexcept
    {
        return id;
    }

    GlobalPaymentIdValue id{};
};

class PaymentsManualStatusUpdateEligibleStatusesResponse :
    public core::behavior::Serializer<PaymentsManualStatusUpdateEligibleStatusesResponse>
{
public:
    void set_payment_id(PaymentIdValue value)
    {
        payment_id = std::move(value);
    }

    [[nodiscard]] const PaymentIdValue& get_payment_id() const noexcept
    {
        return payment_id;
    }

    void set_eligible_statuses(std::vector<ManualUpdateIntentStatusValue> value)
    {
        eligible_statuses = std::move(value);
    }

    [[nodiscard]] const std::vector<ManualUpdateIntentStatusValue>&
    get_eligible_statuses() const noexcept
    {
        return eligible_statuses;
    }

    PaymentIdValue payment_id{};
    std::vector<ManualUpdateIntentStatusValue> eligible_statuses{};
};

class PaymentsManualStatusUpdateRequest :
    public core::behavior::Serializer<PaymentsManualStatusUpdateRequest>
{
public:
    void set_intent_status(ManualUpdateIntentStatusValue value)
    {
        intent_status = std::move(value);
    }

    [[nodiscard]] const ManualUpdateIntentStatusValue& get_intent_status() const noexcept
    {
        return intent_status;
    }

    ManualUpdateIntentStatusValue intent_status{};
};

class PaymentsManualStatusUpdateResponse :
    public core::behavior::Serializer<PaymentsManualStatusUpdateResponse>
{
public:
    void set_payment_id(PaymentIdValue value)
    {
        payment_id = std::move(value);
    }

    [[nodiscard]] const PaymentIdValue& get_payment_id() const noexcept
    {
        return payment_id;
    }

    void set_attempt_id(std::string value)
    {
        attempt_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_attempt_id() const noexcept
    {
        return attempt_id;
    }

    void set_intent_status(IntentStatusValue value)
    {
        intent_status = std::move(value);
    }

    [[nodiscard]] const IntentStatusValue& get_intent_status() const noexcept
    {
        return intent_status;
    }

    void set_attempt_status(AttemptStatusValue value)
    {
        attempt_status = std::move(value);
    }

    [[nodiscard]] const AttemptStatusValue& get_attempt_status() const noexcept
    {
        return attempt_status;
    }

    PaymentIdValue payment_id{};
    std::string attempt_id{};
    IntentStatusValue intent_status{};
    AttemptStatusValue attempt_status{};
};

class PaymentsManualUpdateRequest : public core::behavior::Serializer<PaymentsManualUpdateRequest>
{
public:
    void set_payment_id(PaymentIdValue value)
    {
        payment_id = std::move(value);
    }

    [[nodiscard]] const PaymentIdValue& get_payment_id() const noexcept
    {
        return payment_id;
    }

    void set_attempt_id(std::string value)
    {
        attempt_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_attempt_id() const noexcept
    {
        return attempt_id;
    }

    void set_merchant_id(MerchantIdValue value)
    {
        merchant_id = std::move(value);
    }

    [[nodiscard]] const MerchantIdValue& get_merchant_id() const noexcept
    {
        return merchant_id;
    }

    void set_attempt_status(std::optional<AttemptStatusValue> value)
    {
        attempt_status = std::move(value);
    }

    [[nodiscard]] const std::optional<AttemptStatusValue>& get_attempt_status() const noexcept
    {
        return attempt_status;
    }

    void set_error_code(std::optional<std::string> value)
    {
        error_code = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_error_code() const noexcept
    {
        return error_code;
    }

    void set_error_message(std::optional<std::string> value)
    {
        error_message = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_error_message() const noexcept
    {
        return error_message;
    }

    void set_error_reason(std::optional<std::string> value)
    {
        error_reason = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_error_reason() const noexcept
    {
        return error_reason;
    }

    void set_connector_transaction_id(std::optional<std::string> value)
    {
        connector_transaction_id = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_connector_transaction_id() const noexcept
    {
        return connector_transaction_id;
    }

    void set_amount_capturable(std::optional<MinorUnitValue> value)
    {
        amount_capturable = std::move(value);
    }

    [[nodiscard]] const std::optional<MinorUnitValue>& get_amount_capturable() const noexcept
    {
        return amount_capturable;
    }

    void set_update_amount_captured(std::optional<bool> value)
    {
        update_amount_captured = std::move(value);
    }

    [[nodiscard]] const std::optional<bool>& get_update_amount_captured() const noexcept
    {
        return update_amount_captured;
    }

    void set_amount_captured(std::optional<MinorUnitValue> value)
    {
        amount_captured = std::move(value);
    }

    [[nodiscard]] const std::optional<MinorUnitValue>& get_amount_captured() const noexcept
    {
        return amount_captured;
    }

    PaymentIdValue payment_id{};
    std::string attempt_id{};
    MerchantIdValue merchant_id{};
    std::optional<AttemptStatusValue> attempt_status{};
    std::optional<std::string> error_code{};
    std::optional<std::string> error_message{};
    std::optional<std::string> error_reason{};
    std::optional<std::string> connector_transaction_id{};
    std::optional<MinorUnitValue> amount_capturable{};
    std::optional<bool> update_amount_captured{};
    std::optional<MinorUnitValue> amount_captured{};
};

class PaymentsManualUpdateResponse : public core::behavior::Serializer<PaymentsManualUpdateResponse>
{
public:
    void set_payment_id(PaymentIdValue value)
    {
        payment_id = std::move(value);
    }

    [[nodiscard]] const PaymentIdValue& get_payment_id() const noexcept
    {
        return payment_id;
    }

    void set_attempt_id(std::string value)
    {
        attempt_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_attempt_id() const noexcept
    {
        return attempt_id;
    }

    void set_merchant_id(MerchantIdValue value)
    {
        merchant_id = std::move(value);
    }

    [[nodiscard]] const MerchantIdValue& get_merchant_id() const noexcept
    {
        return merchant_id;
    }

    void set_attempt_status(AttemptStatusValue value)
    {
        attempt_status = std::move(value);
    }

    [[nodiscard]] const AttemptStatusValue& get_attempt_status() const noexcept
    {
        return attempt_status;
    }

    void set_error_code(std::optional<std::string> value)
    {
        error_code = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_error_code() const noexcept
    {
        return error_code;
    }

    void set_error_message(std::optional<std::string> value)
    {
        error_message = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_error_message() const noexcept
    {
        return error_message;
    }

    void set_error_reason(std::optional<std::string> value)
    {
        error_reason = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_error_reason() const noexcept
    {
        return error_reason;
    }

    void set_connector_transaction_id(std::optional<std::string> value)
    {
        connector_transaction_id = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_connector_transaction_id() const noexcept
    {
        return connector_transaction_id;
    }

    void set_amount_capturable(std::optional<MinorUnitValue> value)
    {
        amount_capturable = std::move(value);
    }

    [[nodiscard]] const std::optional<MinorUnitValue>& get_amount_capturable() const noexcept
    {
        return amount_capturable;
    }

    void set_amount_captured(std::optional<MinorUnitValue> value)
    {
        amount_captured = std::move(value);
    }

    [[nodiscard]] const std::optional<MinorUnitValue>& get_amount_captured() const noexcept
    {
        return amount_captured;
    }

    PaymentIdValue payment_id{};
    std::string attempt_id{};
    MerchantIdValue merchant_id{};
    AttemptStatusValue attempt_status{};
    std::optional<std::string> error_code{};
    std::optional<std::string> error_message{};
    std::optional<std::string> error_reason{};
    std::optional<std::string> connector_transaction_id{};
    std::optional<MinorUnitValue> amount_capturable{};
    std::optional<MinorUnitValue> amount_captured{};
};

class PaymentsPostCaptureVoidSyncTrackingData :
    public core::behavior::Serializer<PaymentsPostCaptureVoidSyncTrackingData>
{
public:
    void set_payment_id(PaymentIdValue value)
    {
        payment_id = std::move(value);
    }

    [[nodiscard]] const PaymentIdValue& get_payment_id() const noexcept
    {
        return payment_id;
    }

    void set_merchant_id(MerchantIdValue value)
    {
        merchant_id = std::move(value);
    }

    [[nodiscard]] const MerchantIdValue& get_merchant_id() const noexcept
    {
        return merchant_id;
    }

    PaymentIdValue payment_id{};
    MerchantIdValue merchant_id{};
};

class PaymentsPostSessionTokensRequest :
    public core::behavior::Serializer<PaymentsPostSessionTokensRequest>
{
public:
    void set_payment_id(PaymentIdValue value)
    {
        payment_id = std::move(value);
    }

    [[nodiscard]] const PaymentIdValue& get_payment_id() const noexcept
    {
        return payment_id;
    }

    void set_client_secret(std::optional<SecretValue> value)
    {
        client_secret = std::move(value);
    }

    [[nodiscard]] const std::optional<SecretValue>& get_client_secret() const noexcept
    {
        return client_secret;
    }

    void set_payment_method_type(PaymentMethodTypeValue value)
    {
        payment_method_type = std::move(value);
    }

    [[nodiscard]] const PaymentMethodTypeValue& get_payment_method_type() const noexcept
    {
        return payment_method_type;
    }

    void set_payment_method(PaymentMethodValue value)
    {
        payment_method = std::move(value);
    }

    [[nodiscard]] const PaymentMethodValue& get_payment_method() const noexcept
    {
        return payment_method;
    }

    PaymentIdValue payment_id{};
    std::optional<SecretValue> client_secret{};
    PaymentMethodTypeValue payment_method_type{};
    PaymentMethodValue payment_method{};
};

class PaymentsRejectRequest : public core::behavior::Serializer<PaymentsRejectRequest>
{
public:
    void set_payment_id(PaymentIdValue value)
    {
        payment_id = std::move(value);
    }

    [[nodiscard]] const PaymentIdValue& get_payment_id() const noexcept
    {
        return payment_id;
    }

    PaymentIdValue payment_id{};
};

class PaymentsResponseForm : public core::behavior::Serializer<PaymentsResponseForm>
{
public:
    void set_transaction_id(std::string value)
    {
        transaction_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_transaction_id() const noexcept
    {
        return transaction_id;
    }

    void set_merchant_id(MerchantIdValue value)
    {
        merchant_id = std::move(value);
    }

    [[nodiscard]] const MerchantIdValue& get_merchant_id() const noexcept
    {
        return merchant_id;
    }

    void set_order_id(std::string value)
    {
        order_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_order_id() const noexcept
    {
        return order_id;
    }

    std::string transaction_id{};
    MerchantIdValue merchant_id{};
    std::string order_id{};
};

class PaymentsRetrieveRequest : public core::behavior::Serializer<PaymentsRetrieveRequest>
{
public:
    void set_resource_id(PaymentIdType value)
    {
        resource_id = std::move(value);
    }

    [[nodiscard]] const PaymentIdType& get_resource_id() const noexcept
    {
        return resource_id;
    }

    void set_merchant_id(std::optional<MerchantIdValue> value)
    {
        merchant_id = std::move(value);
    }

    [[nodiscard]] const std::optional<MerchantIdValue>& get_merchant_id() const noexcept
    {
        return merchant_id;
    }

    void set_force_sync(bool value)
    {
        force_sync = std::move(value);
    }

    [[nodiscard]] const bool& get_force_sync() const noexcept
    {
        return force_sync;
    }

    void set_param(std::optional<std::string> value)
    {
        param = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_param() const noexcept
    {
        return param;
    }

    void set_connector(std::optional<std::string> value)
    {
        connector = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_connector() const noexcept
    {
        return connector;
    }

    void set_merchant_connector_details(std::optional<MerchantConnectorDetailsWrapValue> value)
    {
        merchant_connector_details = std::move(value);
    }

    [[nodiscard]] const std::optional<MerchantConnectorDetailsWrapValue>&
    get_merchant_connector_details() const noexcept
    {
        return merchant_connector_details;
    }

    void set_client_secret(std::optional<std::string> value)
    {
        client_secret = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_client_secret() const noexcept
    {
        return client_secret;
    }

    void set_expand_captures(std::optional<bool> value)
    {
        expand_captures = std::move(value);
    }

    [[nodiscard]] const std::optional<bool>& get_expand_captures() const noexcept
    {
        return expand_captures;
    }

    void set_expand_attempts(std::optional<bool> value)
    {
        expand_attempts = std::move(value);
    }

    [[nodiscard]] const std::optional<bool>& get_expand_attempts() const noexcept
    {
        return expand_attempts;
    }

    void set_all_keys_required(std::optional<bool> value)
    {
        all_keys_required = std::move(value);
    }

    [[nodiscard]] const std::optional<bool>& get_all_keys_required() const noexcept
    {
        return all_keys_required;
    }

    PaymentIdType resource_id{};
    std::optional<MerchantIdValue> merchant_id{};
    bool force_sync{};
    std::optional<std::string> param{};
    std::optional<std::string> connector{};
    std::optional<MerchantConnectorDetailsWrapValue> merchant_connector_details{};
    std::optional<std::string> client_secret{};
    std::optional<bool> expand_captures{};
    std::optional<bool> expand_attempts{};
    std::optional<bool> all_keys_required{};
};

class PaymentsSessionRequest : public core::behavior::Serializer<PaymentsSessionRequest>
{
public:
    void set_payment_id(PaymentIdValue value)
    {
        payment_id = std::move(value);
    }

    [[nodiscard]] const PaymentIdValue& get_payment_id() const noexcept
    {
        return payment_id;
    }

    void set_client_secret(std::optional<std::string> value)
    {
        client_secret = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_client_secret() const noexcept
    {
        return client_secret;
    }

    void set_wallets(std::vector<PaymentMethodTypeValue> value)
    {
        wallets = std::move(value);
    }

    [[nodiscard]] const std::vector<PaymentMethodTypeValue>& get_wallets() const noexcept
    {
        return wallets;
    }

    void set_merchant_connector_details(std::optional<MerchantConnectorDetailsWrapValue> value)
    {
        merchant_connector_details = std::move(value);
    }

    [[nodiscard]] const std::optional<MerchantConnectorDetailsWrapValue>&
    get_merchant_connector_details() const noexcept
    {
        return merchant_connector_details;
    }

    PaymentIdValue payment_id{};
    std::optional<std::string> client_secret{};
    std::vector<PaymentMethodTypeValue> wallets{};
    std::optional<MerchantConnectorDetailsWrapValue> merchant_connector_details{};
};

class PaymentsSessionResponse : public core::behavior::Serializer<PaymentsSessionResponse>
{
public:
    void set_payment_id(PaymentIdValue value)
    {
        payment_id = std::move(value);
    }

    [[nodiscard]] const PaymentIdValue& get_payment_id() const noexcept
    {
        return payment_id;
    }

    void set_client_secret(SecretValue value)
    {
        client_secret = std::move(value);
    }

    [[nodiscard]] const SecretValue& get_client_secret() const noexcept
    {
        return client_secret;
    }

    void set_session_token(std::vector<SessionToken> value)
    {
        session_token = std::move(value);
    }

    [[nodiscard]] const std::vector<SessionToken>& get_session_token() const noexcept
    {
        return session_token;
    }

    void set_vault_details(std::optional<VaultDetailsResponse> value)
    {
        vault_details = std::move(value);
    }

    [[nodiscard]] const std::optional<VaultDetailsResponse>& get_vault_details() const noexcept
    {
        return vault_details;
    }

    PaymentIdValue payment_id{};
    SecretValue client_secret{};
    std::vector<SessionToken> session_token{};
    std::optional<VaultDetailsResponse> vault_details{};
};

class PaymentsStartRequest : public core::behavior::Serializer<PaymentsStartRequest>
{
public:
    void set_payment_id(PaymentIdValue value)
    {
        payment_id = std::move(value);
    }

    [[nodiscard]] const PaymentIdValue& get_payment_id() const noexcept
    {
        return payment_id;
    }

    void set_merchant_id(MerchantIdValue value)
    {
        merchant_id = std::move(value);
    }

    [[nodiscard]] const MerchantIdValue& get_merchant_id() const noexcept
    {
        return merchant_id;
    }

    void set_attempt_id(std::string value)
    {
        attempt_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_attempt_id() const noexcept
    {
        return attempt_id;
    }

    PaymentIdValue payment_id{};
    MerchantIdValue merchant_id{};
    std::string attempt_id{};
};

class PlatformPaymentListFilters : public core::behavior::Serializer<PlatformPaymentListFilters>
{
public:
    void set_connector(
        std::unordered_map<std::string, std::vector<MerchantConnectorInfoValue>> value
    )
    {
        connector = std::move(value);
    }

    [[nodiscard]] const std::unordered_map<std::string, std::vector<MerchantConnectorInfoValue>>&
    get_connector() const noexcept
    {
        return connector;
    }

    void set_currency(std::vector<CurrencyValue> value)
    {
        currency = std::move(value);
    }

    [[nodiscard]] const std::vector<CurrencyValue>& get_currency() const noexcept
    {
        return currency;
    }

    void set_status(std::vector<IntentStatusValue> value)
    {
        status = std::move(value);
    }

    [[nodiscard]] const std::vector<IntentStatusValue>& get_status() const noexcept
    {
        return status;
    }

    void set_payment_method(
        std::unordered_map<std::string, std::vector<PaymentMethodTypeValue>> value
    )
    {
        payment_method = std::move(value);
    }

    [[nodiscard]] const std::unordered_map<std::string, std::vector<PaymentMethodTypeValue>>&
    get_payment_method() const noexcept
    {
        return payment_method;
    }

    void set_authentication_type(std::vector<AuthenticationTypeValue> value)
    {
        authentication_type = std::move(value);
    }

    [[nodiscard]] const std::vector<AuthenticationTypeValue>&
    get_authentication_type() const noexcept
    {
        return authentication_type;
    }

    void set_card_network(std::vector<CardNetworkValue> value)
    {
        card_network = std::move(value);
    }

    [[nodiscard]] const std::vector<CardNetworkValue>& get_card_network() const noexcept
    {
        return card_network;
    }

    void set_card_discovery(std::vector<CardDiscoveryValue> value)
    {
        card_discovery = std::move(value);
    }

    [[nodiscard]] const std::vector<CardDiscoveryValue>& get_card_discovery() const noexcept
    {
        return card_discovery;
    }

    std::unordered_map<std::string, std::vector<MerchantConnectorInfoValue>> connector{};
    std::vector<CurrencyValue> currency{};
    std::vector<IntentStatusValue> status{};
    std::unordered_map<std::string, std::vector<PaymentMethodTypeValue>> payment_method{};
    std::vector<AuthenticationTypeValue> authentication_type{};
    std::vector<CardNetworkValue> card_network{};
    std::vector<CardDiscoveryValue> card_discovery{};
};

class ProxyCardData : public core::behavior::Serializer<ProxyCardData>
{
public:
    void set_card_number(SecretValue value)
    {
        card_number = std::move(value);
    }

    [[nodiscard]] const SecretValue& get_card_number() const noexcept
    {
        return card_number;
    }

    void set_card_exp_month(SecretValue value)
    {
        card_exp_month = std::move(value);
    }

    [[nodiscard]] const SecretValue& get_card_exp_month() const noexcept
    {
        return card_exp_month;
    }

    void set_card_exp_year(SecretValue value)
    {
        card_exp_year = std::move(value);
    }

    [[nodiscard]] const SecretValue& get_card_exp_year() const noexcept
    {
        return card_exp_year;
    }

    void set_card_holder_name(std::optional<SecretValue> value)
    {
        card_holder_name = std::move(value);
    }

    [[nodiscard]] const std::optional<SecretValue>& get_card_holder_name() const noexcept
    {
        return card_holder_name;
    }

    void set_card_cvc(SecretValue value)
    {
        card_cvc = std::move(value);
    }

    [[nodiscard]] const SecretValue& get_card_cvc() const noexcept
    {
        return card_cvc;
    }

    void set_card_issuer(std::optional<std::string> value)
    {
        card_issuer = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_card_issuer() const noexcept
    {
        return card_issuer;
    }

    void set_card_network(std::optional<CardNetworkValue> value)
    {
        card_network = std::move(value);
    }

    [[nodiscard]] const std::optional<CardNetworkValue>& get_card_network() const noexcept
    {
        return card_network;
    }

    void set_card_type(std::optional<std::string> value)
    {
        card_type = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_card_type() const noexcept
    {
        return card_type;
    }

    void set_card_issuing_country(std::optional<std::string> value)
    {
        card_issuing_country = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_card_issuing_country() const noexcept
    {
        return card_issuing_country;
    }

    void set_bank_code(std::optional<std::string> value)
    {
        bank_code = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_bank_code() const noexcept
    {
        return bank_code;
    }

    void set_nick_name(std::optional<SecretValue> value)
    {
        nick_name = std::move(value);
    }

    [[nodiscard]] const std::optional<SecretValue>& get_nick_name() const noexcept
    {
        return nick_name;
    }

    void set_bin_number(std::optional<std::string> value)
    {
        bin_number = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_bin_number() const noexcept
    {
        return bin_number;
    }

    void set_last_four(std::optional<std::string> value)
    {
        last_four = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_last_four() const noexcept
    {
        return last_four;
    }

    SecretValue card_number{};
    SecretValue card_exp_month{};
    SecretValue card_exp_year{};
    std::optional<SecretValue> card_holder_name{};
    SecretValue card_cvc{};
    std::optional<std::string> card_issuer{};
    std::optional<CardNetworkValue> card_network{};
    std::optional<std::string> card_type{};
    std::optional<std::string> card_issuing_country{};
    std::optional<std::string> bank_code{};
    std::optional<SecretValue> nick_name{};
    std::optional<std::string> bin_number{};
    std::optional<std::string> last_four{};
};

class RealTimePaymentDataResponse : public core::behavior::Serializer<RealTimePaymentDataResponse>
{
public:
    void set_details(std::optional<RealTimePaymentData> value)
    {
        details = std::move(value);
    }

    [[nodiscard]] const std::optional<RealTimePaymentData>& get_details() const noexcept
    {
        return details;
    }

    std::optional<RealTimePaymentData> details{};
};

class RecoveryPaymentsResponse : public core::behavior::Serializer<RecoveryPaymentsResponse>
{
public:
    void set_id(GlobalPaymentIdValue value)
    {
        id = std::move(value);
    }

    [[nodiscard]] const GlobalPaymentIdValue& get_id() const noexcept
    {
        return id;
    }

    void set_intent_status(IntentStatusValue value)
    {
        intent_status = std::move(value);
    }

    [[nodiscard]] const IntentStatusValue& get_intent_status() const noexcept
    {
        return intent_status;
    }

    void set_merchant_reference_id(std::optional<PaymentReferenceIdValue> value)
    {
        merchant_reference_id = std::move(value);
    }

    [[nodiscard]] const std::optional<PaymentReferenceIdValue>&
    get_merchant_reference_id() const noexcept
    {
        return merchant_reference_id;
    }

    GlobalPaymentIdValue id{};
    IntentStatusValue intent_status{};
    std::optional<PaymentReferenceIdValue> merchant_reference_id{};
};

class RecoveryRequestAccepted : public core::behavior::Serializer<RecoveryRequestAccepted>
{
public:
    void set_merchant_reference_id(PaymentReferenceIdValue value)
    {
        merchant_reference_id = std::move(value);
    }

    [[nodiscard]] const PaymentReferenceIdValue& get_merchant_reference_id() const noexcept
    {
        return merchant_reference_id;
    }

    void set_connector_transaction_id(std::string value)
    {
        connector_transaction_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_connector_transaction_id() const noexcept
    {
        return connector_transaction_id;
    }

    PaymentReferenceIdValue merchant_reference_id{};
    std::string connector_transaction_id{};
};

class RecoveryRetryWebhookDetails : public core::behavior::Serializer<RecoveryRetryWebhookDetails>
{
public:
    void set_id(GlobalPaymentIdValue value)
    {
        id = std::move(value);
    }

    [[nodiscard]] const GlobalPaymentIdValue& get_id() const noexcept
    {
        return id;
    }

    void set_merchant_reference_id(PaymentReferenceIdValue value)
    {
        merchant_reference_id = std::move(value);
    }

    [[nodiscard]] const PaymentReferenceIdValue& get_merchant_reference_id() const noexcept
    {
        return merchant_reference_id;
    }

    void set_next_retry(RecoveryNextRetryValue value)
    {
        next_retry = std::move(value);
    }

    [[nodiscard]] const RecoveryNextRetryValue& get_next_retry() const noexcept
    {
        return next_retry;
    }

    GlobalPaymentIdValue id{};
    PaymentReferenceIdValue merchant_reference_id{};
    RecoveryNextRetryValue next_retry{};
};

class RequestSurchargeDetails : public core::behavior::Serializer<RequestSurchargeDetails>
{
public:
    void set_surcharge_amount(MinorUnitValue value)
    {
        surcharge_amount = std::move(value);
    }

    [[nodiscard]] const MinorUnitValue& get_surcharge_amount() const noexcept
    {
        return surcharge_amount;
    }

    void set_tax_amount(std::optional<MinorUnitValue> value)
    {
        tax_amount = std::move(value);
    }

    [[nodiscard]] const std::optional<MinorUnitValue>& get_tax_amount() const noexcept
    {
        return tax_amount;
    }

    void set_surcharge_percentage(std::optional<double> value)
    {
        surcharge_percentage = std::move(value);
    }

    [[nodiscard]] const std::optional<double>& get_surcharge_percentage() const noexcept
    {
        return surcharge_percentage;
    }

    MinorUnitValue surcharge_amount{};
    std::optional<MinorUnitValue> tax_amount{};
    std::optional<double> surcharge_percentage{};
};

class ResponsePaymentMethodTypesForPayments :
    public core::behavior::Serializer<ResponsePaymentMethodTypesForPayments>
{
public:
    void set_payment_method_type(PaymentMethodValue value)
    {
        payment_method_type = std::move(value);
    }

    [[nodiscard]] const PaymentMethodValue& get_payment_method_type() const noexcept
    {
        return payment_method_type;
    }

    void set_payment_method_subtype(PaymentMethodTypeValue value)
    {
        payment_method_subtype = std::move(value);
    }

    [[nodiscard]] const PaymentMethodTypeValue& get_payment_method_subtype() const noexcept
    {
        return payment_method_subtype;
    }

    void set_payment_experience(std::optional<std::vector<PaymentExperienceValue>> value)
    {
        payment_experience = std::move(value);
    }

    [[nodiscard]] const std::optional<std::vector<PaymentExperienceValue>>&
    get_payment_experience() const noexcept
    {
        return payment_experience;
    }

    void set_extra_information(std::optional<PaymentMethodSubtypeSpecificDataValue> value)
    {
        extra_information = std::move(value);
    }

    [[nodiscard]] const std::optional<PaymentMethodSubtypeSpecificDataValue>&
    get_extra_information() const noexcept
    {
        return extra_information;
    }

    void set_required_fields(std::vector<RequiredFieldInfoValue> value)
    {
        required_fields = std::move(value);
    }

    [[nodiscard]] const std::vector<RequiredFieldInfoValue>& get_required_fields() const noexcept
    {
        return required_fields;
    }

    void set_surcharge_details(std::optional<SurchargeDetailsResponseValue> value)
    {
        surcharge_details = std::move(value);
    }

    [[nodiscard]] const std::optional<SurchargeDetailsResponseValue>&
    get_surcharge_details() const noexcept
    {
        return surcharge_details;
    }

    PaymentMethodValue payment_method_type{};
    PaymentMethodTypeValue payment_method_subtype{};
    std::optional<std::vector<PaymentExperienceValue>> payment_experience{};
    std::optional<PaymentMethodSubtypeSpecificDataValue> extra_information{};
    std::vector<RequiredFieldInfoValue> required_fields{};
    std::optional<SurchargeDetailsResponseValue> surcharge_details{};
};

class RetrievePaymentLinkResponse : public core::behavior::Serializer<RetrievePaymentLinkResponse>
{
public:
    void set_payment_link_id(std::string value)
    {
        payment_link_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_payment_link_id() const noexcept
    {
        return payment_link_id;
    }

    void set_payment_id(PaymentIdValue value)
    {
        payment_id = std::move(value);
    }

    [[nodiscard]] const PaymentIdValue& get_payment_id() const noexcept
    {
        return payment_id;
    }

    void set_merchant_id(MerchantIdValue value)
    {
        merchant_id = std::move(value);
    }

    [[nodiscard]] const MerchantIdValue& get_merchant_id() const noexcept
    {
        return merchant_id;
    }

    void set_processor_merchant_id(std::optional<MerchantIdValue> value)
    {
        processor_merchant_id = std::move(value);
    }

    [[nodiscard]] const std::optional<MerchantIdValue>& get_processor_merchant_id() const noexcept
    {
        return processor_merchant_id;
    }

    void set_profile_id(std::optional<ProfileIdValue> value)
    {
        profile_id = std::move(value);
    }

    [[nodiscard]] const std::optional<ProfileIdValue>& get_profile_id() const noexcept
    {
        return profile_id;
    }

    void set_link_to_pay(std::string value)
    {
        link_to_pay = std::move(value);
    }

    [[nodiscard]] const std::string& get_link_to_pay() const noexcept
    {
        return link_to_pay;
    }

    void set_amount(MinorUnitValue value)
    {
        amount = std::move(value);
    }

    [[nodiscard]] const MinorUnitValue& get_amount() const noexcept
    {
        return amount;
    }

    void set_created_at(PrimitiveDateTimeValue value)
    {
        created_at = std::move(value);
    }

    [[nodiscard]] const PrimitiveDateTimeValue& get_created_at() const noexcept
    {
        return created_at;
    }

    void set_expiry(std::optional<PrimitiveDateTimeValue> value)
    {
        expiry = std::move(value);
    }

    [[nodiscard]] const std::optional<PrimitiveDateTimeValue>& get_expiry() const noexcept
    {
        return expiry;
    }

    void set_description(std::optional<std::string> value)
    {
        description = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_description() const noexcept
    {
        return description;
    }

    void set_status(PaymentLinkStatus value)
    {
        status = std::move(value);
    }

    [[nodiscard]] const PaymentLinkStatus& get_status() const noexcept
    {
        return status;
    }

    void set_currency(std::optional<CurrencyValue> value)
    {
        currency = std::move(value);
    }

    [[nodiscard]] const std::optional<CurrencyValue>& get_currency() const noexcept
    {
        return currency;
    }

    void set_secure_link(std::optional<std::string> value)
    {
        secure_link = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_secure_link() const noexcept
    {
        return secure_link;
    }

    std::string payment_link_id{};
    PaymentIdValue payment_id{};
    MerchantIdValue merchant_id{};
    std::optional<MerchantIdValue> processor_merchant_id{};
    std::optional<ProfileIdValue> profile_id{};
    std::string link_to_pay{};
    MinorUnitValue amount{};
    PrimitiveDateTimeValue created_at{};
    std::optional<PrimitiveDateTimeValue> expiry{};
    std::optional<std::string> description{};
    PaymentLinkStatus status{};
    std::optional<CurrencyValue> currency{};
    std::optional<std::string> secure_link{};
};

class RewardData : public core::behavior::Serializer<RewardData>
{
public:
    void set_merchant_id(MerchantIdValue value)
    {
        merchant_id = std::move(value);
    }

    [[nodiscard]] const MerchantIdValue& get_merchant_id() const noexcept
    {
        return merchant_id;
    }

    MerchantIdValue merchant_id{};
};

class ScheduledExpirationTime : public core::behavior::Serializer<ScheduledExpirationTime>
{
public:
    void set_date(PrimitiveDateTimeValue value)
    {
        date = std::move(value);
    }

    [[nodiscard]] const PrimitiveDateTimeValue& get_date() const noexcept
    {
        return date;
    }

    void set_validity_after_expiration(std::optional<std::uint32_t> value)
    {
        validity_after_expiration = std::move(value);
    }

    [[nodiscard]] const std::optional<std::uint32_t>& get_validity_after_expiration() const noexcept
    {
        return validity_after_expiration;
    }

    void set_pix_key(std::optional<PixKeyValue> value)
    {
        pix_key = std::move(value);
    }

    [[nodiscard]] const std::optional<PixKeyValue>& get_pix_key() const noexcept
    {
        return pix_key;
    }

    PrimitiveDateTimeValue date{};
    std::optional<std::uint32_t> validity_after_expiration{};
    std::optional<PixKeyValue> pix_key{};
};

class SdkInformation : public core::behavior::Serializer<SdkInformation>
{
public:
    void set_sdk_app_id(std::string value)
    {
        sdk_app_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_sdk_app_id() const noexcept
    {
        return sdk_app_id;
    }

    void set_sdk_enc_data(std::string value)
    {
        sdk_enc_data = std::move(value);
    }

    [[nodiscard]] const std::string& get_sdk_enc_data() const noexcept
    {
        return sdk_enc_data;
    }

    void set_sdk_ephem_pub_key(std::unordered_map<std::string, std::string> value)
    {
        sdk_ephem_pub_key = std::move(value);
    }

    [[nodiscard]] const std::unordered_map<std::string, std::string>&
    get_sdk_ephem_pub_key() const noexcept
    {
        return sdk_ephem_pub_key;
    }

    void set_sdk_trans_id(std::string value)
    {
        sdk_trans_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_sdk_trans_id() const noexcept
    {
        return sdk_trans_id;
    }

    void set_sdk_reference_number(std::string value)
    {
        sdk_reference_number = std::move(value);
    }

    [[nodiscard]] const std::string& get_sdk_reference_number() const noexcept
    {
        return sdk_reference_number;
    }

    void set_sdk_max_timeout(std::uint8_t value)
    {
        sdk_max_timeout = std::move(value);
    }

    [[nodiscard]] const std::uint8_t& get_sdk_max_timeout() const noexcept
    {
        return sdk_max_timeout;
    }

    void set_sdk_type(std::optional<SdkType> value)
    {
        sdk_type = std::move(value);
    }

    [[nodiscard]] const std::optional<SdkType>& get_sdk_type() const noexcept
    {
        return sdk_type;
    }

    void set_device_details(std::optional<DeviceDetails> value)
    {
        device_details = std::move(value);
    }

    [[nodiscard]] const std::optional<DeviceDetails>& get_device_details() const noexcept
    {
        return device_details;
    }

    std::string sdk_app_id{};
    std::string sdk_enc_data{};
    std::unordered_map<std::string, std::string> sdk_ephem_pub_key{};
    std::string sdk_trans_id{};
    std::string sdk_reference_number{};
    std::uint8_t sdk_max_timeout{};
    std::optional<SdkType> sdk_type{};
    std::optional<DeviceDetails> device_details{};
};

class SdkNextActionData : public core::behavior::Serializer<SdkNextActionData>
{
public:
    void set_next_action(NextActionCall value)
    {
        next_action = std::move(value);
    }

    [[nodiscard]] const NextActionCall& get_next_action() const noexcept
    {
        return next_action;
    }

    void set_order_id(std::optional<std::string> value)
    {
        order_id = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_order_id() const noexcept
    {
        return order_id;
    }

    NextActionCall next_action{};
    std::optional<std::string> order_id{};
};

class SepaAndBacsBillingDetails : public core::behavior::Serializer<SepaAndBacsBillingDetails>
{
public:
    void set_email(std::optional<EmailValue> value)
    {
        email = std::move(value);
    }

    [[nodiscard]] const std::optional<EmailValue>& get_email() const noexcept
    {
        return email;
    }

    void set_name(std::optional<SecretValue> value)
    {
        name = std::move(value);
    }

    [[nodiscard]] const std::optional<SecretValue>& get_name() const noexcept
    {
        return name;
    }

    std::optional<EmailValue> email{};
    std::optional<SecretValue> name{};
};

class SingleUseMandate : public core::behavior::Serializer<SingleUseMandate>
{
public:
    void set_amount(MinorUnitValue value)
    {
        amount = std::move(value);
    }

    [[nodiscard]] const MinorUnitValue& get_amount() const noexcept
    {
        return amount;
    }

    void set_currency(CurrencyValue value)
    {
        currency = std::move(value);
    }

    [[nodiscard]] const CurrencyValue& get_currency() const noexcept
    {
        return currency;
    }

    MinorUnitValue amount{};
    CurrencyValue currency{};
};

enum class SortOn
{
    Amount,
    AttemptCount,
    Created,
    Modified,
};

class ThreeDsData : public core::behavior::Serializer<ThreeDsData>
{
public:
    void set_three_ds_authentication_url(SecretValue value)
    {
        three_ds_authentication_url = std::move(value);
    }

    [[nodiscard]] const SecretValue& get_three_ds_authentication_url() const noexcept
    {
        return three_ds_authentication_url;
    }

    void set_three_ds_authorize_url(SecretValue value)
    {
        three_ds_authorize_url = std::move(value);
    }

    [[nodiscard]] const SecretValue& get_three_ds_authorize_url() const noexcept
    {
        return three_ds_authorize_url;
    }

    void set_three_ds_method_details(ThreeDsMethodData value)
    {
        three_ds_method_details = std::move(value);
    }

    [[nodiscard]] const ThreeDsMethodData& get_three_ds_method_details() const noexcept
    {
        return three_ds_method_details;
    }

    void set_poll_config(PollConfigResponse value)
    {
        poll_config = std::move(value);
    }

    [[nodiscard]] const PollConfigResponse& get_poll_config() const noexcept
    {
        return poll_config;
    }

    void set_message_version(std::optional<std::string> value)
    {
        message_version = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_message_version() const noexcept
    {
        return message_version;
    }

    void set_directory_server_id(std::optional<std::string> value)
    {
        directory_server_id = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_directory_server_id() const noexcept
    {
        return directory_server_id;
    }

    void set_card_network(std::optional<CardNetworkValue> value)
    {
        card_network = std::move(value);
    }

    [[nodiscard]] const std::optional<CardNetworkValue>& get_card_network() const noexcept
    {
        return card_network;
    }

    void set_three_ds_connector(std::optional<std::string> value)
    {
        three_ds_connector = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_three_ds_connector() const noexcept
    {
        return three_ds_connector;
    }

    SecretValue three_ds_authentication_url{};
    SecretValue three_ds_authorize_url{};
    ThreeDsMethodData three_ds_method_details{};
    PollConfigResponse poll_config{};
    std::optional<std::string> message_version{};
    std::optional<std::string> directory_server_id{};
    std::optional<CardNetworkValue> card_network{};
    std::optional<std::string> three_ds_connector{};
};

enum class ThreeDsMethodKey
{
    ThreeDsMethodData,
    JWT,
};

class UpiCollectData : public core::behavior::Serializer<UpiCollectData>
{
public:
    void set_vpa_id(std::optional<SecretValue> value)
    {
        vpa_id = std::move(value);
    }

    [[nodiscard]] const std::optional<SecretValue>& get_vpa_id() const noexcept
    {
        return vpa_id;
    }

    void set_upi_source(std::optional<UpiSource> value)
    {
        upi_source = std::move(value);
    }

    [[nodiscard]] const std::optional<UpiSource>& get_upi_source() const noexcept
    {
        return upi_source;
    }

    std::optional<SecretValue> vpa_id{};
    std::optional<UpiSource> upi_source{};
};

class UpiIntentData : public core::behavior::Serializer<UpiIntentData>
{
public:
    void set_upi_source(std::optional<UpiSource> value)
    {
        upi_source = std::move(value);
    }

    [[nodiscard]] const std::optional<UpiSource>& get_upi_source() const noexcept
    {
        return upi_source;
    }

    void set_app_name(std::optional<std::string> value)
    {
        app_name = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_app_name() const noexcept
    {
        return app_name;
    }

    std::optional<UpiSource> upi_source{};
    std::optional<std::string> app_name{};
};

class UpiQrData : public core::behavior::Serializer<UpiQrData>
{
public:
    void set_upi_source(std::optional<UpiSource> value)
    {
        upi_source = std::move(value);
    }

    [[nodiscard]] const std::optional<UpiSource>& get_upi_source() const noexcept
    {
        return upi_source;
    }

    std::optional<UpiSource> upi_source{};
};

class UpiResponse : public core::behavior::Serializer<UpiResponse>
{
public:
    void set_details(std::optional<UpiAdditionalDataValue> value)
    {
        details = std::move(value);
    }

    [[nodiscard]] const std::optional<UpiAdditionalDataValue>& get_details() const noexcept
    {
        return details;
    }

    std::optional<UpiAdditionalDataValue> details{};
};

class VaultDetails : public core::behavior::Serializer<VaultDetails>
{
public:
    void set_internal_vault(std::optional<InternalVaultSessionDetails> value)
    {
        internal_vault = std::move(value);
    }

    [[nodiscard]] const std::optional<InternalVaultSessionDetails>&
    get_internal_vault() const noexcept
    {
        return internal_vault;
    }

    void set_external_vault_details(std::optional<VaultSessionDetails> value)
    {
        external_vault_details = std::move(value);
    }

    [[nodiscard]] const std::optional<VaultSessionDetails>&
    get_external_vault_details() const noexcept
    {
        return external_vault_details;
    }

    std::optional<InternalVaultSessionDetails> internal_vault{};
    std::optional<VaultSessionDetails> external_vault_details{};
};

class VgsSessionDetails : public core::behavior::Serializer<VgsSessionDetails>
{
public:
    void set_external_vault_id(SecretValue value)
    {
        external_vault_id = std::move(value);
    }

    [[nodiscard]] const SecretValue& get_external_vault_id() const noexcept
    {
        return external_vault_id;
    }

    void set_sdk_env(std::string value)
    {
        sdk_env = std::move(value);
    }

    [[nodiscard]] const std::string& get_sdk_env() const noexcept
    {
        return sdk_env;
    }

    SecretValue external_vault_id{};
    std::string sdk_env{};
};

class VgsVaultData : public core::behavior::Serializer<VgsVaultData>
{
public:
    void set_vault_id(SecretValue value)
    {
        vault_id = std::move(value);
    }

    [[nodiscard]] const SecretValue& get_vault_id() const noexcept
    {
        return vault_id;
    }

    void set_environment(std::string value)
    {
        environment = std::move(value);
    }

    [[nodiscard]] const std::string& get_environment() const noexcept
    {
        return environment;
    }

    SecretValue vault_id{};
    std::string environment{};
};

class VoucherNextStepData : public core::behavior::Serializer<VoucherNextStepData>
{
public:
    void set_entry_date(std::optional<std::string> value)
    {
        entry_date = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_entry_date() const noexcept
    {
        return entry_date;
    }

    void set_expires_at(std::optional<std::int64_t> value)
    {
        expires_at = std::move(value);
    }

    [[nodiscard]] const std::optional<std::int64_t>& get_expires_at() const noexcept
    {
        return expires_at;
    }

    void set_expiry_date(std::optional<PrimitiveDateTimeValue> value)
    {
        expiry_date = std::move(value);
    }

    [[nodiscard]] const std::optional<PrimitiveDateTimeValue>& get_expiry_date() const noexcept
    {
        return expiry_date;
    }

    void set_reference(std::string value)
    {
        reference = std::move(value);
    }

    [[nodiscard]] const std::string& get_reference() const noexcept
    {
        return reference;
    }

    void set_download_url(std::optional<UrlValue> value)
    {
        download_url = std::move(value);
    }

    [[nodiscard]] const std::optional<UrlValue>& get_download_url() const noexcept
    {
        return download_url;
    }

    void set_instructions_url(std::optional<UrlValue> value)
    {
        instructions_url = std::move(value);
    }

    [[nodiscard]] const std::optional<UrlValue>& get_instructions_url() const noexcept
    {
        return instructions_url;
    }

    void set_digitable_line(std::optional<SecretValue> value)
    {
        digitable_line = std::move(value);
    }

    [[nodiscard]] const std::optional<SecretValue>& get_digitable_line() const noexcept
    {
        return digitable_line;
    }

    void set_barcode(std::optional<SecretValue> value)
    {
        barcode = std::move(value);
    }

    [[nodiscard]] const std::optional<SecretValue>& get_barcode() const noexcept
    {
        return barcode;
    }

    void set_qr_code_url(std::optional<UrlValue> value)
    {
        qr_code_url = std::move(value);
    }

    [[nodiscard]] const std::optional<UrlValue>& get_qr_code_url() const noexcept
    {
        return qr_code_url;
    }

    void set_raw_qr_data(std::optional<std::string> value)
    {
        raw_qr_data = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_raw_qr_data() const noexcept
    {
        return raw_qr_data;
    }

    std::optional<std::string> entry_date{};
    std::optional<std::int64_t> expires_at{};
    std::optional<PrimitiveDateTimeValue> expiry_date{};
    std::string reference{};
    std::optional<UrlValue> download_url{};
    std::optional<UrlValue> instructions_url{};
    std::optional<SecretValue> digitable_line{};
    std::optional<SecretValue> barcode{};
    std::optional<UrlValue> qr_code_url{};
    std::optional<std::string> raw_qr_data{};
};

class VoucherResponse : public core::behavior::Serializer<VoucherResponse>
{
public:
    void set_details(std::optional<VoucherData> value)
    {
        details = std::move(value);
    }

    [[nodiscard]] const std::optional<VoucherData>& get_details() const noexcept
    {
        return details;
    }

    std::optional<VoucherData> details{};
};

class WaitScreenInstructions : public core::behavior::Serializer<WaitScreenInstructions>
{
public:
    void set_display_from_timestamp(std::int64_t value)
    {
        display_from_timestamp = std::move(value);
    }

    [[nodiscard]] const std::int64_t& get_display_from_timestamp() const noexcept
    {
        return display_from_timestamp;
    }

    void set_display_to_timestamp(std::optional<std::int64_t> value)
    {
        display_to_timestamp = std::move(value);
    }

    [[nodiscard]] const std::optional<std::int64_t>& get_display_to_timestamp() const noexcept
    {
        return display_to_timestamp;
    }

    void set_poll_config(std::optional<PollConfig> value)
    {
        poll_config = std::move(value);
    }

    [[nodiscard]] const std::optional<PollConfig>& get_poll_config() const noexcept
    {
        return poll_config;
    }

    std::int64_t display_from_timestamp{};
    std::optional<std::int64_t> display_to_timestamp{};
    std::optional<PollConfig> poll_config{};
};

class WalletResponse : public core::behavior::Serializer<WalletResponse>
{
public:
    void set_details(std::optional<WalletResponseData> value)
    {
        details = std::move(value);
    }

    [[nodiscard]] const std::optional<WalletResponseData>& get_details() const noexcept
    {
        return details;
    }

    std::optional<WalletResponseData> details{};
};

class AddressDetailsWithPhone : public core::behavior::Serializer<AddressDetailsWithPhone>
{
public:
    void set_address(std::optional<AddressDetails> value)
    {
        address = std::move(value);
    }

    [[nodiscard]] const std::optional<AddressDetails>& get_address() const noexcept
    {
        return address;
    }

    void set_phone_number(std::optional<SecretValue> value)
    {
        phone_number = std::move(value);
    }

    [[nodiscard]] const std::optional<SecretValue>& get_phone_number() const noexcept
    {
        return phone_number;
    }

    void set_email(std::optional<EmailValue> value)
    {
        email = std::move(value);
    }

    [[nodiscard]] const std::optional<EmailValue>& get_email() const noexcept
    {
        return email;
    }

    std::optional<AddressDetails> address{};
    std::optional<SecretValue> phone_number{};
    std::optional<EmailValue> email{};
};

class ApiIssuerErrorDetails : public core::behavior::Serializer<ApiIssuerErrorDetails>
{
public:
    void set_code(std::optional<std::string> value)
    {
        code = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_code() const noexcept
    {
        return code;
    }

    void set_message(std::optional<std::string> value)
    {
        message = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_message() const noexcept
    {
        return message;
    }

    void set_network_details(std::optional<ApiNetworkErrorDetails> value)
    {
        network_details = std::move(value);
    }

    [[nodiscard]] const std::optional<ApiNetworkErrorDetails>& get_network_details() const noexcept
    {
        return network_details;
    }

    std::optional<std::string> code{};
    std::optional<std::string> message{};
    std::optional<ApiNetworkErrorDetails> network_details{};
};

class EligibilityOfferDetails : public core::behavior::Serializer<EligibilityOfferDetails>
{
public:
    void set_uplifted_offer_quote_ids(std::vector<std::string> value)
    {
        uplifted_offer_quote_ids = std::move(value);
    }

    [[nodiscard]] const std::vector<std::string>& get_uplifted_offer_quote_ids() const noexcept
    {
        return uplifted_offer_quote_ids;
    }

    void set_eligible_offers(std::vector<EligibleOffer> value)
    {
        eligible_offers = std::move(value);
    }

    [[nodiscard]] const std::vector<EligibleOffer>& get_eligible_offers() const noexcept
    {
        return eligible_offers;
    }

    std::vector<std::string> uplifted_offer_quote_ids{};
    std::vector<EligibleOffer> eligible_offers{};
};

class MandateValidationFields : public core::behavior::Serializer<MandateValidationFields>
{
public:
    void set_recurring_details(std::optional<RecurringDetailsValue> value)
    {
        recurring_details = std::move(value);
    }

    [[nodiscard]] const std::optional<RecurringDetailsValue>& get_recurring_details() const noexcept
    {
        return recurring_details;
    }

    void set_confirm(std::optional<bool> value)
    {
        confirm = std::move(value);
    }

    [[nodiscard]] const std::optional<bool>& get_confirm() const noexcept
    {
        return confirm;
    }

    void set_customer_id(std::optional<CustomerIdValue> value)
    {
        customer_id = std::move(value);
    }

    [[nodiscard]] const std::optional<CustomerIdValue>& get_customer_id() const noexcept
    {
        return customer_id;
    }

    void set_mandate_data(std::optional<MandateData> value)
    {
        mandate_data = std::move(value);
    }

    [[nodiscard]] const std::optional<MandateData>& get_mandate_data() const noexcept
    {
        return mandate_data;
    }

    void set_setup_future_usage(std::optional<FutureUsageValue> value)
    {
        setup_future_usage = std::move(value);
    }

    [[nodiscard]] const std::optional<FutureUsageValue>& get_setup_future_usage() const noexcept
    {
        return setup_future_usage;
    }

    void set_off_session(std::optional<bool> value)
    {
        off_session = std::move(value);
    }

    [[nodiscard]] const std::optional<bool>& get_off_session() const noexcept
    {
        return off_session;
    }

    std::optional<RecurringDetailsValue> recurring_details{};
    std::optional<bool> confirm{};
    std::optional<CustomerIdValue> customer_id{};
    std::optional<MandateData> mandate_data{};
    std::optional<FutureUsageValue> setup_future_usage{};
    std::optional<bool> off_session{};
};

class NetworkParams : public core::behavior::Serializer<NetworkParams>
{
public:
    void set_cartes_bancaires(std::optional<CartesBancairesParams> value)
    {
        cartes_bancaires = std::move(value);
    }

    [[nodiscard]] const std::optional<CartesBancairesParams>& get_cartes_bancaires() const noexcept
    {
        return cartes_bancaires;
    }

    std::optional<CartesBancairesParams> cartes_bancaires{};
};

enum class NextActionData
{
    RedirectToUrl,
    RedirectInsidePopup,
    DisplayBankTransferInformation,
    ThirdPartySdkSessionToken,
    QrCodeInformation,
    FetchQrCodeInformation,
    InvokeUpiIntentSdk,
    InvokeUpiQrFlow,
    DisplayVoucherInformation,
    WaitScreenInformation,
    ThreeDsInvoke,
    InvokeSdkClient,
    CollectOtp,
    InvokeHiddenIframe,
    InvokeDdc,
};

class Order : public core::behavior::Serializer<Order>
{
public:
    void set_on(SortOn value)
    {
        on = std::move(value);
    }

    [[nodiscard]] const SortOn& get_on() const noexcept
    {
        return on;
    }

    void set_by(SortBy value)
    {
        by = std::move(value);
    }

    [[nodiscard]] const SortBy& get_by() const noexcept
    {
        return by;
    }

    SortOn on{};
    SortBy by{};
};

class PaymentLinkDetails : public core::behavior::Serializer<PaymentLinkDetails>
{
public:
    void set_amount(StringMajorUnitValue value)
    {
        amount = std::move(value);
    }

    [[nodiscard]] const StringMajorUnitValue& get_amount() const noexcept
    {
        return amount;
    }

    void set_currency(CurrencyValue value)
    {
        currency = std::move(value);
    }

    [[nodiscard]] const CurrencyValue& get_currency() const noexcept
    {
        return currency;
    }

    void set_pub_key(std::string value)
    {
        pub_key = std::move(value);
    }

    [[nodiscard]] const std::string& get_pub_key() const noexcept
    {
        return pub_key;
    }

    void set_client_secret(std::string value)
    {
        client_secret = std::move(value);
    }

    [[nodiscard]] const std::string& get_client_secret() const noexcept
    {
        return client_secret;
    }

    void set_payment_id(PaymentIdValue value)
    {
        payment_id = std::move(value);
    }

    [[nodiscard]] const PaymentIdValue& get_payment_id() const noexcept
    {
        return payment_id;
    }

    void set_session_expiry(PrimitiveDateTimeValue value)
    {
        session_expiry = std::move(value);
    }

    [[nodiscard]] const PrimitiveDateTimeValue& get_session_expiry() const noexcept
    {
        return session_expiry;
    }

    void set_merchant_logo(std::string value)
    {
        merchant_logo = std::move(value);
    }

    [[nodiscard]] const std::string& get_merchant_logo() const noexcept
    {
        return merchant_logo;
    }

    void set_return_url(std::string value)
    {
        return_url = std::move(value);
    }

    [[nodiscard]] const std::string& get_return_url() const noexcept
    {
        return return_url;
    }

    void set_merchant_name(std::string value)
    {
        merchant_name = std::move(value);
    }

    [[nodiscard]] const std::string& get_merchant_name() const noexcept
    {
        return merchant_name;
    }

    void set_order_details(std::optional<std::vector<OrderDetailsWithStringAmount>> value)
    {
        order_details = std::move(value);
    }

    [[nodiscard]] const std::optional<std::vector<OrderDetailsWithStringAmount>>&
    get_order_details() const noexcept
    {
        return order_details;
    }

    void set_max_items_visible_after_collapse(std::int8_t value)
    {
        max_items_visible_after_collapse = std::move(value);
    }

    [[nodiscard]] const std::int8_t& get_max_items_visible_after_collapse() const noexcept
    {
        return max_items_visible_after_collapse;
    }

    void set_theme(std::string value)
    {
        theme = std::move(value);
    }

    [[nodiscard]] const std::string& get_theme() const noexcept
    {
        return theme;
    }

    void set_merchant_description(std::optional<std::string> value)
    {
        merchant_description = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_merchant_description() const noexcept
    {
        return merchant_description;
    }

    void set_sdk_layout(std::string value)
    {
        sdk_layout = std::move(value);
    }

    [[nodiscard]] const std::string& get_sdk_layout() const noexcept
    {
        return sdk_layout;
    }

    void set_display_sdk_only(bool value)
    {
        display_sdk_only = std::move(value);
    }

    [[nodiscard]] const bool& get_display_sdk_only() const noexcept
    {
        return display_sdk_only;
    }

    void set_hide_card_nickname_field(bool value)
    {
        hide_card_nickname_field = std::move(value);
    }

    [[nodiscard]] const bool& get_hide_card_nickname_field() const noexcept
    {
        return hide_card_nickname_field;
    }

    void set_show_card_form_by_default(bool value)
    {
        show_card_form_by_default = std::move(value);
    }

    [[nodiscard]] const bool& get_show_card_form_by_default() const noexcept
    {
        return show_card_form_by_default;
    }

    void set_locale(std::optional<std::string> value)
    {
        locale = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_locale() const noexcept
    {
        return locale;
    }

    void set_transaction_details(
        std::optional<std::vector<PaymentLinkTransactionDetailsValue>> value
    )
    {
        transaction_details = std::move(value);
    }

    [[nodiscard]] const std::optional<std::vector<PaymentLinkTransactionDetailsValue>>&
    get_transaction_details() const noexcept
    {
        return transaction_details;
    }

    void set_background_image(std::optional<PaymentLinkBackgroundImageConfigValue> value)
    {
        background_image = std::move(value);
    }

    [[nodiscard]] const std::optional<PaymentLinkBackgroundImageConfigValue>&
    get_background_image() const noexcept
    {
        return background_image;
    }

    void set_details_layout(std::optional<PaymentLinkDetailsLayoutValue> value)
    {
        details_layout = std::move(value);
    }

    [[nodiscard]] const std::optional<PaymentLinkDetailsLayoutValue>&
    get_details_layout() const noexcept
    {
        return details_layout;
    }

    void set_branding_visibility(std::optional<bool> value)
    {
        branding_visibility = std::move(value);
    }

    [[nodiscard]] const std::optional<bool>& get_branding_visibility() const noexcept
    {
        return branding_visibility;
    }

    void set_payment_button_text(std::optional<std::string> value)
    {
        payment_button_text = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_payment_button_text() const noexcept
    {
        return payment_button_text;
    }

    void set_skip_status_screen(std::optional<bool> value)
    {
        skip_status_screen = std::move(value);
    }

    [[nodiscard]] const std::optional<bool>& get_skip_status_screen() const noexcept
    {
        return skip_status_screen;
    }

    void set_custom_message_for_card_terms(std::optional<std::string> value)
    {
        custom_message_for_card_terms = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>&
    get_custom_message_for_card_terms() const noexcept
    {
        return custom_message_for_card_terms;
    }

    void set_custom_message_for_payment_method_types(std::optional<PaymentMethodsConfigValue> value)
    {
        custom_message_for_payment_method_types = std::move(value);
    }

    [[nodiscard]] const std::optional<PaymentMethodsConfigValue>&
    get_custom_message_for_payment_method_types() const noexcept
    {
        return custom_message_for_payment_method_types;
    }

    void set_payment_button_colour(std::optional<std::string> value)
    {
        payment_button_colour = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_payment_button_colour() const noexcept
    {
        return payment_button_colour;
    }

    void set_payment_button_text_colour(std::optional<std::string> value)
    {
        payment_button_text_colour = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_payment_button_text_colour() const noexcept
    {
        return payment_button_text_colour;
    }

    void set_background_colour(std::optional<std::string> value)
    {
        background_colour = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_background_colour() const noexcept
    {
        return background_colour;
    }

    void set_sdk_ui_rules(
        std::optional<std::unordered_map<std::string, std::unordered_map<std::string, std::string>>>
            value
    )
    {
        sdk_ui_rules = std::move(value);
    }

    [[nodiscard]] const std::optional<
        std::unordered_map<std::string, std::unordered_map<std::string, std::string>>>&
    get_sdk_ui_rules() const noexcept
    {
        return sdk_ui_rules;
    }

    void set_status(IntentStatusValue value)
    {
        status = std::move(value);
    }

    [[nodiscard]] const IntentStatusValue& get_status() const noexcept
    {
        return status;
    }

    void set_enable_button_only_on_form_ready(bool value)
    {
        enable_button_only_on_form_ready = std::move(value);
    }

    [[nodiscard]] const bool& get_enable_button_only_on_form_ready() const noexcept
    {
        return enable_button_only_on_form_ready;
    }

    void set_payment_form_header_text(std::optional<std::string> value)
    {
        payment_form_header_text = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_payment_form_header_text() const noexcept
    {
        return payment_form_header_text;
    }

    void set_payment_form_label_type(std::optional<PaymentLinkSdkLabelTypeValue> value)
    {
        payment_form_label_type = std::move(value);
    }

    [[nodiscard]] const std::optional<PaymentLinkSdkLabelTypeValue>&
    get_payment_form_label_type() const noexcept
    {
        return payment_form_label_type;
    }

    void set_show_card_terms(std::optional<PaymentLinkShowSdkTermsValue> value)
    {
        show_card_terms = std::move(value);
    }

    [[nodiscard]] const std::optional<PaymentLinkShowSdkTermsValue>&
    get_show_card_terms() const noexcept
    {
        return show_card_terms;
    }

    void set_is_setup_mandate_flow(std::optional<bool> value)
    {
        is_setup_mandate_flow = std::move(value);
    }

    [[nodiscard]] const std::optional<bool>& get_is_setup_mandate_flow() const noexcept
    {
        return is_setup_mandate_flow;
    }

    void set_capture_method(std::optional<CaptureMethodValue> value)
    {
        capture_method = std::move(value);
    }

    [[nodiscard]] const std::optional<CaptureMethodValue>& get_capture_method() const noexcept
    {
        return capture_method;
    }

    void set_setup_future_usage_applied(std::optional<FutureUsageValue> value)
    {
        setup_future_usage_applied = std::move(value);
    }

    [[nodiscard]] const std::optional<FutureUsageValue>&
    get_setup_future_usage_applied() const noexcept
    {
        return setup_future_usage_applied;
    }

    void set_color_icon_card_cvc_error(std::optional<std::string> value)
    {
        color_icon_card_cvc_error = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_color_icon_card_cvc_error() const noexcept
    {
        return color_icon_card_cvc_error;
    }

    void set_show_merchant_name(std::optional<bool> value)
    {
        show_merchant_name = std::move(value);
    }

    [[nodiscard]] const std::optional<bool>& get_show_merchant_name() const noexcept
    {
        return show_merchant_name;
    }

    void set_payment_methods_separator_text(std::optional<std::string> value)
    {
        payment_methods_separator_text = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>&
    get_payment_methods_separator_text() const noexcept
    {
        return payment_methods_separator_text;
    }

    StringMajorUnitValue amount{};
    CurrencyValue currency{};
    std::string pub_key{};
    std::string client_secret{};
    PaymentIdValue payment_id{};
    PrimitiveDateTimeValue session_expiry{};
    std::string merchant_logo{};
    std::string return_url{};
    std::string merchant_name{};
    std::optional<std::vector<OrderDetailsWithStringAmount>> order_details{};
    std::int8_t max_items_visible_after_collapse{};
    std::string theme{};
    std::optional<std::string> merchant_description{};
    std::string sdk_layout{};
    bool display_sdk_only{};
    bool hide_card_nickname_field{};
    bool show_card_form_by_default{};
    std::optional<std::string> locale{};
    std::optional<std::vector<PaymentLinkTransactionDetailsValue>> transaction_details{};
    std::optional<PaymentLinkBackgroundImageConfigValue> background_image{};
    std::optional<PaymentLinkDetailsLayoutValue> details_layout{};
    std::optional<bool> branding_visibility{};
    std::optional<std::string> payment_button_text{};
    std::optional<bool> skip_status_screen{};
    std::optional<std::string> custom_message_for_card_terms{};
    std::optional<PaymentMethodsConfigValue> custom_message_for_payment_method_types{};
    std::optional<std::string> payment_button_colour{};
    std::optional<std::string> payment_button_text_colour{};
    std::optional<std::string> background_colour{};
    std::optional<std::unordered_map<std::string, std::unordered_map<std::string, std::string>>>
        sdk_ui_rules{};
    IntentStatusValue status{};
    bool enable_button_only_on_form_ready{};
    std::optional<std::string> payment_form_header_text{};
    std::optional<PaymentLinkSdkLabelTypeValue> payment_form_label_type{};
    std::optional<PaymentLinkShowSdkTermsValue> show_card_terms{};
    std::optional<bool> is_setup_mandate_flow{};
    std::optional<CaptureMethodValue> capture_method{};
    std::optional<FutureUsageValue> setup_future_usage_applied{};
    std::optional<std::string> color_icon_card_cvc_error{};
    std::optional<bool> show_merchant_name{};
    std::optional<std::string> payment_methods_separator_text{};
};

class PaymentLinkListResponse : public core::behavior::Serializer<PaymentLinkListResponse>
{
public:
    void set_size(std::size_t value)
    {
        size = std::move(value);
    }

    [[nodiscard]] const std::size_t& get_size() const noexcept
    {
        return size;
    }

    void set_total_count(std::int64_t value)
    {
        total_count = std::move(value);
    }

    [[nodiscard]] const std::int64_t& get_total_count() const noexcept
    {
        return total_count;
    }

    void set_data(std::vector<RetrievePaymentLinkResponse> value)
    {
        data = std::move(value);
    }

    [[nodiscard]] const std::vector<RetrievePaymentLinkResponse>& get_data() const noexcept
    {
        return data;
    }

    std::size_t size{};
    std::int64_t total_count{};
    std::vector<RetrievePaymentLinkResponse> data{};
};

class PaymentLinkStatusDetails : public core::behavior::Serializer<PaymentLinkStatusDetails>
{
public:
    void set_amount(StringMajorUnitValue value)
    {
        amount = std::move(value);
    }

    [[nodiscard]] const StringMajorUnitValue& get_amount() const noexcept
    {
        return amount;
    }

    void set_currency(CurrencyValue value)
    {
        currency = std::move(value);
    }

    [[nodiscard]] const CurrencyValue& get_currency() const noexcept
    {
        return currency;
    }

    void set_payment_id(PaymentIdValue value)
    {
        payment_id = std::move(value);
    }

    [[nodiscard]] const PaymentIdValue& get_payment_id() const noexcept
    {
        return payment_id;
    }

    void set_merchant_logo(std::string value)
    {
        merchant_logo = std::move(value);
    }

    [[nodiscard]] const std::string& get_merchant_logo() const noexcept
    {
        return merchant_logo;
    }

    void set_merchant_name(std::string value)
    {
        merchant_name = std::move(value);
    }

    [[nodiscard]] const std::string& get_merchant_name() const noexcept
    {
        return merchant_name;
    }

    void set_created(PrimitiveDateTimeValue value)
    {
        created = std::move(value);
    }

    [[nodiscard]] const PrimitiveDateTimeValue& get_created() const noexcept
    {
        return created;
    }

    void set_status(PaymentLinkStatusWrap value)
    {
        status = std::move(value);
    }

    [[nodiscard]] const PaymentLinkStatusWrap& get_status() const noexcept
    {
        return status;
    }

    void set_error_code(std::optional<std::string> value)
    {
        error_code = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_error_code() const noexcept
    {
        return error_code;
    }

    void set_error_message(std::optional<std::string> value)
    {
        error_message = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_error_message() const noexcept
    {
        return error_message;
    }

    void set_redirect(bool value)
    {
        redirect = std::move(value);
    }

    [[nodiscard]] const bool& get_redirect() const noexcept
    {
        return redirect;
    }

    void set_theme(std::string value)
    {
        theme = std::move(value);
    }

    [[nodiscard]] const std::string& get_theme() const noexcept
    {
        return theme;
    }

    void set_return_url(std::string value)
    {
        return_url = std::move(value);
    }

    [[nodiscard]] const std::string& get_return_url() const noexcept
    {
        return return_url;
    }

    void set_locale(std::optional<std::string> value)
    {
        locale = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_locale() const noexcept
    {
        return locale;
    }

    void set_transaction_details(
        std::optional<std::vector<PaymentLinkTransactionDetailsValue>> value
    )
    {
        transaction_details = std::move(value);
    }

    [[nodiscard]] const std::optional<std::vector<PaymentLinkTransactionDetailsValue>>&
    get_transaction_details() const noexcept
    {
        return transaction_details;
    }

    void set_unified_code(std::optional<std::string> value)
    {
        unified_code = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_unified_code() const noexcept
    {
        return unified_code;
    }

    void set_unified_message(std::optional<std::string> value)
    {
        unified_message = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_unified_message() const noexcept
    {
        return unified_message;
    }

    void set_capture_method(std::optional<CaptureMethodValue> value)
    {
        capture_method = std::move(value);
    }

    [[nodiscard]] const std::optional<CaptureMethodValue>& get_capture_method() const noexcept
    {
        return capture_method;
    }

    void set_setup_future_usage_applied(std::optional<FutureUsageValue> value)
    {
        setup_future_usage_applied = std::move(value);
    }

    [[nodiscard]] const std::optional<FutureUsageValue>&
    get_setup_future_usage_applied() const noexcept
    {
        return setup_future_usage_applied;
    }

    void set_redirect_delay_seconds(std::optional<std::uint32_t> value)
    {
        redirect_delay_seconds = std::move(value);
    }

    [[nodiscard]] const std::optional<std::uint32_t>& get_redirect_delay_seconds() const noexcept
    {
        return redirect_delay_seconds;
    }

    StringMajorUnitValue amount{};
    CurrencyValue currency{};
    PaymentIdValue payment_id{};
    std::string merchant_logo{};
    std::string merchant_name{};
    PrimitiveDateTimeValue created{};
    PaymentLinkStatusWrap status{};
    std::optional<std::string> error_code{};
    std::optional<std::string> error_message{};
    bool redirect{};
    std::string theme{};
    std::string return_url{};
    std::optional<std::string> locale{};
    std::optional<std::vector<PaymentLinkTransactionDetailsValue>> transaction_details{};
    std::optional<std::string> unified_code{};
    std::optional<std::string> unified_message{};
    std::optional<CaptureMethodValue> capture_method{};
    std::optional<FutureUsageValue> setup_future_usage_applied{};
    std::optional<std::uint32_t> redirect_delay_seconds{};
};

class PaymentListConstraints : public core::behavior::Serializer<PaymentListConstraints>
{
public:
    void set_payment_id(std::optional<GlobalPaymentIdValue> value)
    {
        payment_id = std::move(value);
    }

    [[nodiscard]] const std::optional<GlobalPaymentIdValue>& get_payment_id() const noexcept
    {
        return payment_id;
    }

    void set_profile_id(std::optional<ProfileIdValue> value)
    {
        profile_id = std::move(value);
    }

    [[nodiscard]] const std::optional<ProfileIdValue>& get_profile_id() const noexcept
    {
        return profile_id;
    }

    void set_customer_id(std::optional<GlobalCustomerIdValue> value)
    {
        customer_id = std::move(value);
    }

    [[nodiscard]] const std::optional<GlobalCustomerIdValue>& get_customer_id() const noexcept
    {
        return customer_id;
    }

    void set_starting_after(std::optional<GlobalPaymentIdValue> value)
    {
        starting_after = std::move(value);
    }

    [[nodiscard]] const std::optional<GlobalPaymentIdValue>& get_starting_after() const noexcept
    {
        return starting_after;
    }

    void set_ending_before(std::optional<GlobalPaymentIdValue> value)
    {
        ending_before = std::move(value);
    }

    [[nodiscard]] const std::optional<GlobalPaymentIdValue>& get_ending_before() const noexcept
    {
        return ending_before;
    }

    void set_limit(PageSizeValue value)
    {
        limit = std::move(value);
    }

    [[nodiscard]] const PageSizeValue& get_limit() const noexcept
    {
        return limit;
    }

    void set_offset(PageOffsetValue value)
    {
        offset = std::move(value);
    }

    [[nodiscard]] const PageOffsetValue& get_offset() const noexcept
    {
        return offset;
    }

    void set_created(std::optional<PrimitiveDateTimeValue> value)
    {
        created = std::move(value);
    }

    [[nodiscard]] const std::optional<PrimitiveDateTimeValue>& get_created() const noexcept
    {
        return created;
    }

    void set_created_lt(std::optional<PrimitiveDateTimeValue> value)
    {
        created_lt = std::move(value);
    }

    [[nodiscard]] const std::optional<PrimitiveDateTimeValue>& get_created_lt() const noexcept
    {
        return created_lt;
    }

    void set_created_gt(std::optional<PrimitiveDateTimeValue> value)
    {
        created_gt = std::move(value);
    }

    [[nodiscard]] const std::optional<PrimitiveDateTimeValue>& get_created_gt() const noexcept
    {
        return created_gt;
    }

    void set_created_lte(std::optional<PrimitiveDateTimeValue> value)
    {
        created_lte = std::move(value);
    }

    [[nodiscard]] const std::optional<PrimitiveDateTimeValue>& get_created_lte() const noexcept
    {
        return created_lte;
    }

    void set_created_gte(std::optional<PrimitiveDateTimeValue> value)
    {
        created_gte = std::move(value);
    }

    [[nodiscard]] const std::optional<PrimitiveDateTimeValue>& get_created_gte() const noexcept
    {
        return created_gte;
    }

    void set_start_amount(std::optional<std::int64_t> value)
    {
        start_amount = std::move(value);
    }

    [[nodiscard]] const std::optional<std::int64_t>& get_start_amount() const noexcept
    {
        return start_amount;
    }

    void set_end_amount(std::optional<std::int64_t> value)
    {
        end_amount = std::move(value);
    }

    [[nodiscard]] const std::optional<std::int64_t>& get_end_amount() const noexcept
    {
        return end_amount;
    }

    void set_connector(std::optional<std::vector<ConnectorValue>> value)
    {
        connector = std::move(value);
    }

    [[nodiscard]] const std::optional<std::vector<ConnectorValue>>& get_connector() const noexcept
    {
        return connector;
    }

    void set_currency(std::optional<std::vector<CurrencyValue>> value)
    {
        currency = std::move(value);
    }

    [[nodiscard]] const std::optional<std::vector<CurrencyValue>>& get_currency() const noexcept
    {
        return currency;
    }

    void set_status(std::optional<std::vector<IntentStatusValue>> value)
    {
        status = std::move(value);
    }

    [[nodiscard]] const std::optional<std::vector<IntentStatusValue>>& get_status() const noexcept
    {
        return status;
    }

    void set_payment_method_type(std::optional<std::vector<PaymentMethodValue>> value)
    {
        payment_method_type = std::move(value);
    }

    [[nodiscard]] const std::optional<std::vector<PaymentMethodValue>>&
    get_payment_method_type() const noexcept
    {
        return payment_method_type;
    }

    void set_payment_method_subtype(std::optional<std::vector<PaymentMethodTypeValue>> value)
    {
        payment_method_subtype = std::move(value);
    }

    [[nodiscard]] const std::optional<std::vector<PaymentMethodTypeValue>>&
    get_payment_method_subtype() const noexcept
    {
        return payment_method_subtype;
    }

    void set_authentication_type(std::optional<std::vector<AuthenticationTypeValue>> value)
    {
        authentication_type = std::move(value);
    }

    [[nodiscard]] const std::optional<std::vector<AuthenticationTypeValue>>&
    get_authentication_type() const noexcept
    {
        return authentication_type;
    }

    void set_merchant_connector_id(
        std::optional<std::vector<MerchantConnectorAccountIdValue>> value
    )
    {
        merchant_connector_id = std::move(value);
    }

    [[nodiscard]] const std::optional<std::vector<MerchantConnectorAccountIdValue>>&
    get_merchant_connector_id() const noexcept
    {
        return merchant_connector_id;
    }

    void set_order_on(SortOn value)
    {
        order_on = std::move(value);
    }

    [[nodiscard]] const SortOn& get_order_on() const noexcept
    {
        return order_on;
    }

    void set_order_by(SortBy value)
    {
        order_by = std::move(value);
    }

    [[nodiscard]] const SortBy& get_order_by() const noexcept
    {
        return order_by;
    }

    void set_card_network(std::optional<std::vector<CardNetworkValue>> value)
    {
        card_network = std::move(value);
    }

    [[nodiscard]] const std::optional<std::vector<CardNetworkValue>>&
    get_card_network() const noexcept
    {
        return card_network;
    }

    void set_merchant_order_reference_id(std::optional<std::string> value)
    {
        merchant_order_reference_id = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_merchant_order_reference_id() const noexcept
    {
        return merchant_order_reference_id;
    }

    std::optional<GlobalPaymentIdValue> payment_id{};
    std::optional<ProfileIdValue> profile_id{};
    std::optional<GlobalCustomerIdValue> customer_id{};
    std::optional<GlobalPaymentIdValue> starting_after{};
    std::optional<GlobalPaymentIdValue> ending_before{};
    PageSizeValue limit{};
    PageOffsetValue offset{};
    std::optional<PrimitiveDateTimeValue> created{};
    std::optional<PrimitiveDateTimeValue> created_lt{};
    std::optional<PrimitiveDateTimeValue> created_gt{};
    std::optional<PrimitiveDateTimeValue> created_lte{};
    std::optional<PrimitiveDateTimeValue> created_gte{};
    std::optional<std::int64_t> start_amount{};
    std::optional<std::int64_t> end_amount{};
    std::optional<std::vector<ConnectorValue>> connector{};
    std::optional<std::vector<CurrencyValue>> currency{};
    std::optional<std::vector<IntentStatusValue>> status{};
    std::optional<std::vector<PaymentMethodValue>> payment_method_type{};
    std::optional<std::vector<PaymentMethodTypeValue>> payment_method_subtype{};
    std::optional<std::vector<AuthenticationTypeValue>> authentication_type{};
    std::optional<std::vector<MerchantConnectorAccountIdValue>> merchant_connector_id{};
    SortOn order_on{};
    SortBy order_by{};
    std::optional<std::vector<CardNetworkValue>> card_network{};
    std::optional<std::string> merchant_order_reference_id{};
};

class PaymentMethodListResponseForPayments :
    public core::behavior::Serializer<PaymentMethodListResponseForPayments>
{
public:
    void set_payment_methods_enabled(std::vector<ResponsePaymentMethodTypesForPayments> value)
    {
        payment_methods_enabled = std::move(value);
    }

    [[nodiscard]] const std::vector<ResponsePaymentMethodTypesForPayments>&
    get_payment_methods_enabled() const noexcept
    {
        return payment_methods_enabled;
    }

    void set_customer_payment_methods(
        std::optional<std::vector<CustomerPaymentMethodResponseItemValue>> value
    )
    {
        customer_payment_methods = std::move(value);
    }

    [[nodiscard]] const std::optional<std::vector<CustomerPaymentMethodResponseItemValue>>&
    get_customer_payment_methods() const noexcept
    {
        return customer_payment_methods;
    }

    std::vector<ResponsePaymentMethodTypesForPayments> payment_methods_enabled{};
    std::optional<std::vector<CustomerPaymentMethodResponseItemValue>> customer_payment_methods{};
};

class PaymentsCaptureResponse : public core::behavior::Serializer<PaymentsCaptureResponse>
{
public:
    void set_id(GlobalPaymentIdValue value)
    {
        id = std::move(value);
    }

    [[nodiscard]] const GlobalPaymentIdValue& get_id() const noexcept
    {
        return id;
    }

    void set_status(IntentStatusValue value)
    {
        status = std::move(value);
    }

    [[nodiscard]] const IntentStatusValue& get_status() const noexcept
    {
        return status;
    }

    void set_amount(PaymentAmountDetailsResponse value)
    {
        amount = std::move(value);
    }

    [[nodiscard]] const PaymentAmountDetailsResponse& get_amount() const noexcept
    {
        return amount;
    }

    GlobalPaymentIdValue id{};
    IntentStatusValue status{};
    PaymentAmountDetailsResponse amount{};
};

class PaymentsDynamicTaxCalculationResponse :
    public core::behavior::Serializer<PaymentsDynamicTaxCalculationResponse>
{
public:
    void set_payment_id(PaymentIdValue value)
    {
        payment_id = std::move(value);
    }

    [[nodiscard]] const PaymentIdValue& get_payment_id() const noexcept
    {
        return payment_id;
    }

    void set_net_amount(MinorUnitValue value)
    {
        net_amount = std::move(value);
    }

    [[nodiscard]] const MinorUnitValue& get_net_amount() const noexcept
    {
        return net_amount;
    }

    void set_order_tax_amount(std::optional<MinorUnitValue> value)
    {
        order_tax_amount = std::move(value);
    }

    [[nodiscard]] const std::optional<MinorUnitValue>& get_order_tax_amount() const noexcept
    {
        return order_tax_amount;
    }

    void set_shipping_cost(std::optional<MinorUnitValue> value)
    {
        shipping_cost = std::move(value);
    }

    [[nodiscard]] const std::optional<MinorUnitValue>& get_shipping_cost() const noexcept
    {
        return shipping_cost;
    }

    void set_display_amount(DisplayAmountOnSdk value)
    {
        display_amount = std::move(value);
    }

    [[nodiscard]] const DisplayAmountOnSdk& get_display_amount() const noexcept
    {
        return display_amount;
    }

    PaymentIdValue payment_id{};
    MinorUnitValue net_amount{};
    std::optional<MinorUnitValue> order_tax_amount{};
    std::optional<MinorUnitValue> shipping_cost{};
    DisplayAmountOnSdk display_amount{};
};

class PaymentsEligibilityCheckResponse :
    public core::behavior::Serializer<PaymentsEligibilityCheckResponse>
{
public:
    void set_payment_id(PaymentIdValue value)
    {
        payment_id = std::move(value);
    }

    [[nodiscard]] const PaymentIdValue& get_payment_id() const noexcept
    {
        return payment_id;
    }

    void set_sdk_next_action(SdkNextAction value)
    {
        sdk_next_action = std::move(value);
    }

    [[nodiscard]] const SdkNextAction& get_sdk_next_action() const noexcept
    {
        return sdk_next_action;
    }

    PaymentIdValue payment_id{};
    SdkNextAction sdk_next_action{};
};

class PaymentsExternalAuthenticationRequest :
    public core::behavior::Serializer<PaymentsExternalAuthenticationRequest>
{
public:
    void set_payment_id(PaymentIdValue value)
    {
        payment_id = std::move(value);
    }

    [[nodiscard]] const PaymentIdValue& get_payment_id() const noexcept
    {
        return payment_id;
    }

    void set_client_secret(std::optional<SecretValue> value)
    {
        client_secret = std::move(value);
    }

    [[nodiscard]] const std::optional<SecretValue>& get_client_secret() const noexcept
    {
        return client_secret;
    }

    void set_sdk_information(std::optional<SdkInformation> value)
    {
        sdk_information = std::move(value);
    }

    [[nodiscard]] const std::optional<SdkInformation>& get_sdk_information() const noexcept
    {
        return sdk_information;
    }

    void set_device_channel(DeviceChannel value)
    {
        device_channel = std::move(value);
    }

    [[nodiscard]] const DeviceChannel& get_device_channel() const noexcept
    {
        return device_channel;
    }

    void set_threeds_method_comp_ind(ThreeDsCompletionIndicator value)
    {
        threeds_method_comp_ind = std::move(value);
    }

    [[nodiscard]] const ThreeDsCompletionIndicator& get_threeds_method_comp_ind() const noexcept
    {
        return threeds_method_comp_ind;
    }

    PaymentIdValue payment_id{};
    std::optional<SecretValue> client_secret{};
    std::optional<SdkInformation> sdk_information{};
    DeviceChannel device_channel{};
    ThreeDsCompletionIndicator threeds_method_comp_ind{};
};

class PaymentsListResponseItem : public core::behavior::Serializer<PaymentsListResponseItem>
{
public:
    void set_id(GlobalPaymentIdValue value)
    {
        id = std::move(value);
    }

    [[nodiscard]] const GlobalPaymentIdValue& get_id() const noexcept
    {
        return id;
    }

    void set_merchant_id(MerchantIdValue value)
    {
        merchant_id = std::move(value);
    }

    [[nodiscard]] const MerchantIdValue& get_merchant_id() const noexcept
    {
        return merchant_id;
    }

    void set_profile_id(ProfileIdValue value)
    {
        profile_id = std::move(value);
    }

    [[nodiscard]] const ProfileIdValue& get_profile_id() const noexcept
    {
        return profile_id;
    }

    void set_customer_id(std::optional<GlobalCustomerIdValue> value)
    {
        customer_id = std::move(value);
    }

    [[nodiscard]] const std::optional<GlobalCustomerIdValue>& get_customer_id() const noexcept
    {
        return customer_id;
    }

    void set_payment_method_id(std::optional<GlobalPaymentMethodIdValue> value)
    {
        payment_method_id = std::move(value);
    }

    [[nodiscard]] const std::optional<GlobalPaymentMethodIdValue>&
    get_payment_method_id() const noexcept
    {
        return payment_method_id;
    }

    void set_status(IntentStatusValue value)
    {
        status = std::move(value);
    }

    [[nodiscard]] const IntentStatusValue& get_status() const noexcept
    {
        return status;
    }

    void set_amount(PaymentAmountDetailsResponse value)
    {
        amount = std::move(value);
    }

    [[nodiscard]] const PaymentAmountDetailsResponse& get_amount() const noexcept
    {
        return amount;
    }

    void set_created(PrimitiveDateTimeValue value)
    {
        created = std::move(value);
    }

    [[nodiscard]] const PrimitiveDateTimeValue& get_created() const noexcept
    {
        return created;
    }

    void set_payment_method_type(std::optional<PaymentMethodValue> value)
    {
        payment_method_type = std::move(value);
    }

    [[nodiscard]] const std::optional<PaymentMethodValue>& get_payment_method_type() const noexcept
    {
        return payment_method_type;
    }

    void set_payment_method_subtype(std::optional<PaymentMethodTypeValue> value)
    {
        payment_method_subtype = std::move(value);
    }

    [[nodiscard]] const std::optional<PaymentMethodTypeValue>&
    get_payment_method_subtype() const noexcept
    {
        return payment_method_subtype;
    }

    void set_connector(std::optional<std::string> value)
    {
        connector = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_connector() const noexcept
    {
        return connector;
    }

    void set_merchant_connector_id(std::optional<MerchantConnectorAccountIdValue> value)
    {
        merchant_connector_id = std::move(value);
    }

    [[nodiscard]] const std::optional<MerchantConnectorAccountIdValue>&
    get_merchant_connector_id() const noexcept
    {
        return merchant_connector_id;
    }

    void set_customer(std::optional<CustomerDetailsResponse> value)
    {
        customer = std::move(value);
    }

    [[nodiscard]] const std::optional<CustomerDetailsResponse>& get_customer() const noexcept
    {
        return customer;
    }

    void set_merchant_reference_id(std::optional<PaymentReferenceIdValue> value)
    {
        merchant_reference_id = std::move(value);
    }

    [[nodiscard]] const std::optional<PaymentReferenceIdValue>&
    get_merchant_reference_id() const noexcept
    {
        return merchant_reference_id;
    }

    void set_connector_payment_id(std::optional<std::string> value)
    {
        connector_payment_id = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_connector_payment_id() const noexcept
    {
        return connector_payment_id;
    }

    void set_connector_response_reference_id(std::optional<std::string> value)
    {
        connector_response_reference_id = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>&
    get_connector_response_reference_id() const noexcept
    {
        return connector_response_reference_id;
    }

    void set_metadata(std::optional<SecretValue> value)
    {
        metadata = std::move(value);
    }

    [[nodiscard]] const std::optional<SecretValue>& get_metadata() const noexcept
    {
        return metadata;
    }

    void set_description(std::optional<std::string> value)
    {
        description = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_description() const noexcept
    {
        return description;
    }

    void set_authentication_type(std::optional<AuthenticationTypeValue> value)
    {
        authentication_type = std::move(value);
    }

    [[nodiscard]] const std::optional<AuthenticationTypeValue>&
    get_authentication_type() const noexcept
    {
        return authentication_type;
    }

    void set_capture_method(std::optional<CaptureMethodValue> value)
    {
        capture_method = std::move(value);
    }

    [[nodiscard]] const std::optional<CaptureMethodValue>& get_capture_method() const noexcept
    {
        return capture_method;
    }

    void set_setup_future_usage(std::optional<FutureUsageValue> value)
    {
        setup_future_usage = std::move(value);
    }

    [[nodiscard]] const std::optional<FutureUsageValue>& get_setup_future_usage() const noexcept
    {
        return setup_future_usage;
    }

    void set_attempt_count(std::int16_t value)
    {
        attempt_count = std::move(value);
    }

    [[nodiscard]] const std::int16_t& get_attempt_count() const noexcept
    {
        return attempt_count;
    }

    void set_error(std::optional<ErrorDetails> value)
    {
        error = std::move(value);
    }

    [[nodiscard]] const std::optional<ErrorDetails>& get_error() const noexcept
    {
        return error;
    }

    void set_cancellation_reason(std::optional<std::string> value)
    {
        cancellation_reason = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_cancellation_reason() const noexcept
    {
        return cancellation_reason;
    }

    void set_order_details(std::optional<std::vector<SecretValue>> value)
    {
        order_details = std::move(value);
    }

    [[nodiscard]] const std::optional<std::vector<SecretValue>>& get_order_details() const noexcept
    {
        return order_details;
    }

    void set_return_url(std::optional<UrlValue> value)
    {
        return_url = std::move(value);
    }

    [[nodiscard]] const std::optional<UrlValue>& get_return_url() const noexcept
    {
        return return_url;
    }

    void set_statement_descriptor(std::optional<StatementDescriptorValue> value)
    {
        statement_descriptor = std::move(value);
    }

    [[nodiscard]] const std::optional<StatementDescriptorValue>&
    get_statement_descriptor() const noexcept
    {
        return statement_descriptor;
    }

    void set_allowed_payment_method_types(std::optional<std::vector<PaymentMethodTypeValue>> value)
    {
        allowed_payment_method_types = std::move(value);
    }

    [[nodiscard]] const std::optional<std::vector<PaymentMethodTypeValue>>&
    get_allowed_payment_method_types() const noexcept
    {
        return allowed_payment_method_types;
    }

    void set_authorization_count(std::optional<std::int32_t> value)
    {
        authorization_count = std::move(value);
    }

    [[nodiscard]] const std::optional<std::int32_t>& get_authorization_count() const noexcept
    {
        return authorization_count;
    }

    void set_modified_at(std::optional<PrimitiveDateTimeValue> value)
    {
        modified_at = std::move(value);
    }

    [[nodiscard]] const std::optional<PrimitiveDateTimeValue>& get_modified_at() const noexcept
    {
        return modified_at;
    }

    void set_is_split_payment(bool value)
    {
        is_split_payment = std::move(value);
    }

    [[nodiscard]] const bool& get_is_split_payment() const noexcept
    {
        return is_split_payment;
    }

    GlobalPaymentIdValue id{};
    MerchantIdValue merchant_id{};
    ProfileIdValue profile_id{};
    std::optional<GlobalCustomerIdValue> customer_id{};
    std::optional<GlobalPaymentMethodIdValue> payment_method_id{};
    IntentStatusValue status{};
    PaymentAmountDetailsResponse amount{};
    PrimitiveDateTimeValue created{};
    std::optional<PaymentMethodValue> payment_method_type{};
    std::optional<PaymentMethodTypeValue> payment_method_subtype{};
    std::optional<std::string> connector{};
    std::optional<MerchantConnectorAccountIdValue> merchant_connector_id{};
    std::optional<CustomerDetailsResponse> customer{};
    std::optional<PaymentReferenceIdValue> merchant_reference_id{};
    std::optional<std::string> connector_payment_id{};
    std::optional<std::string> connector_response_reference_id{};
    std::optional<SecretValue> metadata{};
    std::optional<std::string> description{};
    std::optional<AuthenticationTypeValue> authentication_type{};
    std::optional<CaptureMethodValue> capture_method{};
    std::optional<FutureUsageValue> setup_future_usage{};
    std::int16_t attempt_count{};
    std::optional<ErrorDetails> error{};
    std::optional<std::string> cancellation_reason{};
    std::optional<std::vector<SecretValue>> order_details{};
    std::optional<UrlValue> return_url{};
    std::optional<StatementDescriptorValue> statement_descriptor{};
    std::optional<std::vector<PaymentMethodTypeValue>> allowed_payment_method_types{};
    std::optional<std::int32_t> authorization_count{};
    std::optional<PrimitiveDateTimeValue> modified_at{};
    bool is_split_payment{};
};

class PixAutomaticoMitData : public core::behavior::Serializer<PixAutomaticoMitData>
{
public:
    void set_receiver_details(std::optional<SantanderPixAutomaticoReceiverDetails> value)
    {
        receiver_details = std::move(value);
    }

    [[nodiscard]] const std::optional<SantanderPixAutomaticoReceiverDetails>&
    get_receiver_details() const noexcept
    {
        return receiver_details;
    }

    void set_mandate_execution_date(std::optional<PrimitiveDateTimeValue> value)
    {
        mandate_execution_date = std::move(value);
    }

    [[nodiscard]] const std::optional<PrimitiveDateTimeValue>&
    get_mandate_execution_date() const noexcept
    {
        return mandate_execution_date;
    }

    void set_auto_adjust_date(std::optional<bool> value)
    {
        auto_adjust_date = std::move(value);
    }

    [[nodiscard]] const std::optional<bool>& get_auto_adjust_date() const noexcept
    {
        return auto_adjust_date;
    }

    std::optional<SantanderPixAutomaticoReceiverDetails> receiver_details{};
    std::optional<PrimitiveDateTimeValue> mandate_execution_date{};
    std::optional<bool> auto_adjust_date{};
};

class PixAutomaticoPushData : public core::behavior::Serializer<PixAutomaticoPushData>
{
public:
    void set_time(std::uint32_t value)
    {
        time = std::move(value);
    }

    [[nodiscard]] const std::uint32_t& get_time() const noexcept
    {
        return time;
    }

    void set_retry_policy(std::optional<bool> value)
    {
        retry_policy = std::move(value);
    }

    [[nodiscard]] const std::optional<bool>& get_retry_policy() const noexcept
    {
        return retry_policy;
    }

    void set_mandate_details(std::optional<SantanderMandateDetails> value)
    {
        mandate_details = std::move(value);
    }

    [[nodiscard]] const std::optional<SantanderMandateDetails>& get_mandate_details() const noexcept
    {
        return mandate_details;
    }

    std::uint32_t time{};
    std::optional<bool> retry_policy{};
    std::optional<SantanderMandateDetails> mandate_details{};
};

class PixAutomaticoQrData : public core::behavior::Serializer<PixAutomaticoQrData>
{
public:
    void set_retry_policy(std::optional<bool> value)
    {
        retry_policy = std::move(value);
    }

    [[nodiscard]] const std::optional<bool>& get_retry_policy() const noexcept
    {
        return retry_policy;
    }

    void set_mandate_details(std::optional<SantanderMandateDetails> value)
    {
        mandate_details = std::move(value);
    }

    [[nodiscard]] const std::optional<SantanderMandateDetails>& get_mandate_details() const noexcept
    {
        return mandate_details;
    }

    std::optional<bool> retry_policy{};
    std::optional<SantanderMandateDetails> mandate_details{};
};

class PlatformPaymentListItem : public core::behavior::Serializer<PlatformPaymentListItem>
{
public:
    void set_payment_id(PaymentIdValue value)
    {
        payment_id = std::move(value);
    }

    [[nodiscard]] const PaymentIdValue& get_payment_id() const noexcept
    {
        return payment_id;
    }

    void set_merchant_id(MerchantIdValue value)
    {
        merchant_id = std::move(value);
    }

    [[nodiscard]] const MerchantIdValue& get_merchant_id() const noexcept
    {
        return merchant_id;
    }

    void set_processor_merchant_id(std::optional<MerchantIdValue> value)
    {
        processor_merchant_id = std::move(value);
    }

    [[nodiscard]] const std::optional<MerchantIdValue>& get_processor_merchant_id() const noexcept
    {
        return processor_merchant_id;
    }

    void set_status(IntentStatusValue value)
    {
        status = std::move(value);
    }

    [[nodiscard]] const IntentStatusValue& get_status() const noexcept
    {
        return status;
    }

    void set_amount(MinorUnitValue value)
    {
        amount = std::move(value);
    }

    [[nodiscard]] const MinorUnitValue& get_amount() const noexcept
    {
        return amount;
    }

    void set_net_amount(std::optional<MinorUnitValue> value)
    {
        net_amount = std::move(value);
    }

    [[nodiscard]] const std::optional<MinorUnitValue>& get_net_amount() const noexcept
    {
        return net_amount;
    }

    void set_amount_capturable(MinorUnitValue value)
    {
        amount_capturable = std::move(value);
    }

    [[nodiscard]] const MinorUnitValue& get_amount_capturable() const noexcept
    {
        return amount_capturable;
    }

    void set_state_metadata(std::optional<PaymentIntentStateMetadataValue> value)
    {
        state_metadata = std::move(value);
    }

    [[nodiscard]] const std::optional<PaymentIntentStateMetadataValue>&
    get_state_metadata() const noexcept
    {
        return state_metadata;
    }

    void set_client_secret(std::optional<SecretValue> value)
    {
        client_secret = std::move(value);
    }

    [[nodiscard]] const std::optional<SecretValue>& get_client_secret() const noexcept
    {
        return client_secret;
    }

    void set_created(std::optional<PrimitiveDateTimeValue> value)
    {
        created = std::move(value);
    }

    [[nodiscard]] const std::optional<PrimitiveDateTimeValue>& get_created() const noexcept
    {
        return created;
    }

    void set_modified_at(std::optional<PrimitiveDateTimeValue> value)
    {
        modified_at = std::move(value);
    }

    [[nodiscard]] const std::optional<PrimitiveDateTimeValue>& get_modified_at() const noexcept
    {
        return modified_at;
    }

    void set_currency(std::optional<CurrencyValue> value)
    {
        currency = std::move(value);
    }

    [[nodiscard]] const std::optional<CurrencyValue>& get_currency() const noexcept
    {
        return currency;
    }

    void set_customer_id(std::optional<CustomerIdValue> value)
    {
        customer_id = std::move(value);
    }

    [[nodiscard]] const std::optional<CustomerIdValue>& get_customer_id() const noexcept
    {
        return customer_id;
    }

    void set_description(std::optional<std::string> value)
    {
        description = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_description() const noexcept
    {
        return description;
    }

    void set_order_details(std::optional<std::vector<SecretSerdeValueValue>> value)
    {
        order_details = std::move(value);
    }

    [[nodiscard]] const std::optional<std::vector<SecretSerdeValueValue>>&
    get_order_details() const noexcept
    {
        return order_details;
    }

    void set_connector(std::optional<std::string> value)
    {
        connector = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_connector() const noexcept
    {
        return connector;
    }

    void set_payment_method(std::optional<PaymentMethodValue> value)
    {
        payment_method = std::move(value);
    }

    [[nodiscard]] const std::optional<PaymentMethodValue>& get_payment_method() const noexcept
    {
        return payment_method;
    }

    void set_payment_method_type(std::optional<PaymentMethodTypeValue> value)
    {
        payment_method_type = std::move(value);
    }

    [[nodiscard]] const std::optional<PaymentMethodTypeValue>&
    get_payment_method_type() const noexcept
    {
        return payment_method_type;
    }

    void set_business_label(std::optional<std::string> value)
    {
        business_label = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_business_label() const noexcept
    {
        return business_label;
    }

    void set_business_country(std::optional<CountryAlpha2Value> value)
    {
        business_country = std::move(value);
    }

    [[nodiscard]] const std::optional<CountryAlpha2Value>& get_business_country() const noexcept
    {
        return business_country;
    }

    void set_business_sub_label(std::optional<std::string> value)
    {
        business_sub_label = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_business_sub_label() const noexcept
    {
        return business_sub_label;
    }

    void set_setup_future_usage(std::optional<FutureUsageValue> value)
    {
        setup_future_usage = std::move(value);
    }

    [[nodiscard]] const std::optional<FutureUsageValue>& get_setup_future_usage() const noexcept
    {
        return setup_future_usage;
    }

    void set_capture_method(std::optional<CaptureMethodValue> value)
    {
        capture_method = std::move(value);
    }

    [[nodiscard]] const std::optional<CaptureMethodValue>& get_capture_method() const noexcept
    {
        return capture_method;
    }

    void set_authentication_type(std::optional<AuthenticationTypeValue> value)
    {
        authentication_type = std::move(value);
    }

    [[nodiscard]] const std::optional<AuthenticationTypeValue>&
    get_authentication_type() const noexcept
    {
        return authentication_type;
    }

    void set_connector_transaction_id(std::optional<std::string> value)
    {
        connector_transaction_id = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_connector_transaction_id() const noexcept
    {
        return connector_transaction_id;
    }

    void set_attempt_count(std::int16_t value)
    {
        attempt_count = std::move(value);
    }

    [[nodiscard]] const std::int16_t& get_attempt_count() const noexcept
    {
        return attempt_count;
    }

    void set_profile_id(std::optional<ProfileIdValue> value)
    {
        profile_id = std::move(value);
    }

    [[nodiscard]] const std::optional<ProfileIdValue>& get_profile_id() const noexcept
    {
        return profile_id;
    }

    void set_merchant_connector_id(std::optional<MerchantConnectorAccountIdValue> value)
    {
        merchant_connector_id = std::move(value);
    }

    [[nodiscard]] const std::optional<MerchantConnectorAccountIdValue>&
    get_merchant_connector_id() const noexcept
    {
        return merchant_connector_id;
    }

    void set_merchant_order_reference_id(std::optional<std::string> value)
    {
        merchant_order_reference_id = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_merchant_order_reference_id() const noexcept
    {
        return merchant_order_reference_id;
    }

    void set_metadata(std::optional<serde::Value> value)
    {
        metadata = std::move(value);
    }

    [[nodiscard]] const std::optional<serde::Value>& get_metadata() const noexcept
    {
        return metadata;
    }

    void set_error_message(std::optional<std::string> value)
    {
        error_message = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_error_message() const noexcept
    {
        return error_message;
    }

    void set_updated(std::optional<PrimitiveDateTimeValue> value)
    {
        updated = std::move(value);
    }

    [[nodiscard]] const std::optional<PrimitiveDateTimeValue>& get_updated() const noexcept
    {
        return updated;
    }

    void set_extended_authorization_applied(
        std::optional<ExtendedAuthorizationAppliedBoolValue> value
    )
    {
        extended_authorization_applied = std::move(value);
    }

    [[nodiscard]] const std::optional<ExtendedAuthorizationAppliedBoolValue>&
    get_extended_authorization_applied() const noexcept
    {
        return extended_authorization_applied;
    }

    void set_extended_authorization_last_applied_at(std::optional<PrimitiveDateTimeValue> value)
    {
        extended_authorization_last_applied_at = std::move(value);
    }

    [[nodiscard]] const std::optional<PrimitiveDateTimeValue>&
    get_extended_authorization_last_applied_at() const noexcept
    {
        return extended_authorization_last_applied_at;
    }

    void set_capture_before(std::optional<PrimitiveDateTimeValue> value)
    {
        capture_before = std::move(value);
    }

    [[nodiscard]] const std::optional<PrimitiveDateTimeValue>& get_capture_before() const noexcept
    {
        return capture_before;
    }

    void set_card_discovery(std::optional<CardDiscoveryValue> value)
    {
        card_discovery = std::move(value);
    }

    [[nodiscard]] const std::optional<CardDiscoveryValue>& get_card_discovery() const noexcept
    {
        return card_discovery;
    }

    void set_mit_category(std::optional<MitCategoryValue> value)
    {
        mit_category = std::move(value);
    }

    [[nodiscard]] const std::optional<MitCategoryValue>& get_mit_category() const noexcept
    {
        return mit_category;
    }

    void set_tokenization(std::optional<TokenizationValue> value)
    {
        tokenization = std::move(value);
    }

    [[nodiscard]] const std::optional<TokenizationValue>& get_tokenization() const noexcept
    {
        return tokenization;
    }

    void set_force_3ds_challenge(std::optional<bool> value)
    {
        force_3ds_challenge = std::move(value);
    }

    [[nodiscard]] const std::optional<bool>& get_force_3ds_challenge() const noexcept
    {
        return force_3ds_challenge;
    }

    void set_force_3ds_challenge_trigger(std::optional<bool> value)
    {
        force_3ds_challenge_trigger = std::move(value);
    }

    [[nodiscard]] const std::optional<bool>& get_force_3ds_challenge_trigger() const noexcept
    {
        return force_3ds_challenge_trigger;
    }

    void set_issuer_error_code(std::optional<std::string> value)
    {
        issuer_error_code = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_issuer_error_code() const noexcept
    {
        return issuer_error_code;
    }

    void set_issuer_error_message(std::optional<std::string> value)
    {
        issuer_error_message = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_issuer_error_message() const noexcept
    {
        return issuer_error_message;
    }

    void set_is_iframe_redirection_enabled(std::optional<bool> value)
    {
        is_iframe_redirection_enabled = std::move(value);
    }

    [[nodiscard]] const std::optional<bool>& get_is_iframe_redirection_enabled() const noexcept
    {
        return is_iframe_redirection_enabled;
    }

    void set_payment_channel(std::optional<PaymentChannelValue> value)
    {
        payment_channel = std::move(value);
    }

    [[nodiscard]] const std::optional<PaymentChannelValue>& get_payment_channel() const noexcept
    {
        return payment_channel;
    }

    void set_enable_partial_authorization(std::optional<EnablePartialAuthorizationBoolValue> value)
    {
        enable_partial_authorization = std::move(value);
    }

    [[nodiscard]] const std::optional<EnablePartialAuthorizationBoolValue>&
    get_enable_partial_authorization() const noexcept
    {
        return enable_partial_authorization;
    }

    void set_enable_overcapture(std::optional<EnableOvercaptureBoolValue> value)
    {
        enable_overcapture = std::move(value);
    }

    [[nodiscard]] const std::optional<EnableOvercaptureBoolValue>&
    get_enable_overcapture() const noexcept
    {
        return enable_overcapture;
    }

    void set_is_overcapture_enabled(std::optional<OvercaptureEnabledBoolValue> value)
    {
        is_overcapture_enabled = std::move(value);
    }

    [[nodiscard]] const std::optional<OvercaptureEnabledBoolValue>&
    get_is_overcapture_enabled() const noexcept
    {
        return is_overcapture_enabled;
    }

    void set_network_details(std::optional<NetworkDetails> value)
    {
        network_details = std::move(value);
    }

    [[nodiscard]] const std::optional<NetworkDetails>& get_network_details() const noexcept
    {
        return network_details;
    }

    void set_is_stored_credential(std::optional<bool> value)
    {
        is_stored_credential = std::move(value);
    }

    [[nodiscard]] const std::optional<bool>& get_is_stored_credential() const noexcept
    {
        return is_stored_credential;
    }

    void set_request_extended_authorization(
        std::optional<RequestExtendedAuthorizationBoolValue> value
    )
    {
        request_extended_authorization = std::move(value);
    }

    [[nodiscard]] const std::optional<RequestExtendedAuthorizationBoolValue>&
    get_request_extended_authorization() const noexcept
    {
        return request_extended_authorization;
    }

    void set_billing_descriptor(std::optional<BillingDescriptorValue> value)
    {
        billing_descriptor = std::move(value);
    }

    [[nodiscard]] const std::optional<BillingDescriptorValue>&
    get_billing_descriptor() const noexcept
    {
        return billing_descriptor;
    }

    void set_partner_merchant_identifier_details(
        std::optional<PartnerMerchantIdentifierDetailsValue> value
    )
    {
        partner_merchant_identifier_details = std::move(value);
    }

    [[nodiscard]] const std::optional<PartnerMerchantIdentifierDetailsValue>&
    get_partner_merchant_identifier_details() const noexcept
    {
        return partner_merchant_identifier_details;
    }

    void set_installment_data(std::optional<InstallmentDataValue> value)
    {
        installment_data = std::move(value);
    }

    [[nodiscard]] const std::optional<InstallmentDataValue>& get_installment_data() const noexcept
    {
        return installment_data;
    }

    void set_sender_payment_instrument_id(std::optional<std::string> value)
    {
        sender_payment_instrument_id = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>&
    get_sender_payment_instrument_id() const noexcept
    {
        return sender_payment_instrument_id;
    }

    void set_surcharge_details(std::optional<PaymentSurchargeDetailsResponse> value)
    {
        surcharge_details = std::move(value);
    }

    [[nodiscard]] const std::optional<PaymentSurchargeDetailsResponse>&
    get_surcharge_details() const noexcept
    {
        return surcharge_details;
    }

    void set_installment_options(std::optional<std::vector<InstallmentOptionValue>> value)
    {
        installment_options = std::move(value);
    }

    [[nodiscard]] const std::optional<std::vector<InstallmentOptionValue>>&
    get_installment_options() const noexcept
    {
        return installment_options;
    }

    PaymentIdValue payment_id{};
    MerchantIdValue merchant_id{};
    std::optional<MerchantIdValue> processor_merchant_id{};
    IntentStatusValue status{};
    MinorUnitValue amount{};
    std::optional<MinorUnitValue> net_amount{};
    MinorUnitValue amount_capturable{};
    std::optional<PaymentIntentStateMetadataValue> state_metadata{};
    std::optional<SecretValue> client_secret{};
    std::optional<PrimitiveDateTimeValue> created{};
    std::optional<PrimitiveDateTimeValue> modified_at{};
    std::optional<CurrencyValue> currency{};
    std::optional<CustomerIdValue> customer_id{};
    std::optional<std::string> description{};
    std::optional<std::vector<SecretSerdeValueValue>> order_details{};
    std::optional<std::string> connector{};
    std::optional<PaymentMethodValue> payment_method{};
    std::optional<PaymentMethodTypeValue> payment_method_type{};
    std::optional<std::string> business_label{};
    std::optional<CountryAlpha2Value> business_country{};
    std::optional<std::string> business_sub_label{};
    std::optional<FutureUsageValue> setup_future_usage{};
    std::optional<CaptureMethodValue> capture_method{};
    std::optional<AuthenticationTypeValue> authentication_type{};
    std::optional<std::string> connector_transaction_id{};
    std::int16_t attempt_count{};
    std::optional<ProfileIdValue> profile_id{};
    std::optional<MerchantConnectorAccountIdValue> merchant_connector_id{};
    std::optional<std::string> merchant_order_reference_id{};
    std::optional<serde::Value> metadata{};
    std::optional<std::string> error_message{};
    std::optional<PrimitiveDateTimeValue> updated{};
    std::optional<ExtendedAuthorizationAppliedBoolValue> extended_authorization_applied{};
    std::optional<PrimitiveDateTimeValue> extended_authorization_last_applied_at{};
    std::optional<PrimitiveDateTimeValue> capture_before{};
    std::optional<CardDiscoveryValue> card_discovery{};
    std::optional<MitCategoryValue> mit_category{};
    std::optional<TokenizationValue> tokenization{};
    std::optional<bool> force_3ds_challenge{};
    std::optional<bool> force_3ds_challenge_trigger{};
    std::optional<std::string> issuer_error_code{};
    std::optional<std::string> issuer_error_message{};
    std::optional<bool> is_iframe_redirection_enabled{};
    std::optional<PaymentChannelValue> payment_channel{};
    std::optional<EnablePartialAuthorizationBoolValue> enable_partial_authorization{};
    std::optional<EnableOvercaptureBoolValue> enable_overcapture{};
    std::optional<OvercaptureEnabledBoolValue> is_overcapture_enabled{};
    std::optional<NetworkDetails> network_details{};
    std::optional<bool> is_stored_credential{};
    std::optional<RequestExtendedAuthorizationBoolValue> request_extended_authorization{};
    std::optional<BillingDescriptorValue> billing_descriptor{};
    std::optional<PartnerMerchantIdentifierDetailsValue> partner_merchant_identifier_details{};
    std::optional<InstallmentDataValue> installment_data{};
    std::optional<std::string> sender_payment_instrument_id{};
    std::optional<PaymentSurchargeDetailsResponse> surcharge_details{};
    std::optional<std::vector<InstallmentOptionValue>> installment_options{};
};

class RecoveryPaymentsListResponseItem :
    public core::behavior::Serializer<RecoveryPaymentsListResponseItem>
{
public:
    void set_id(GlobalPaymentIdValue value)
    {
        id = std::move(value);
    }

    [[nodiscard]] const GlobalPaymentIdValue& get_id() const noexcept
    {
        return id;
    }

    void set_merchant_id(MerchantIdValue value)
    {
        merchant_id = std::move(value);
    }

    [[nodiscard]] const MerchantIdValue& get_merchant_id() const noexcept
    {
        return merchant_id;
    }

    void set_profile_id(ProfileIdValue value)
    {
        profile_id = std::move(value);
    }

    [[nodiscard]] const ProfileIdValue& get_profile_id() const noexcept
    {
        return profile_id;
    }

    void set_customer_id(std::optional<GlobalCustomerIdValue> value)
    {
        customer_id = std::move(value);
    }

    [[nodiscard]] const std::optional<GlobalCustomerIdValue>& get_customer_id() const noexcept
    {
        return customer_id;
    }

    void set_status(RecoveryStatusValue value)
    {
        status = std::move(value);
    }

    [[nodiscard]] const RecoveryStatusValue& get_status() const noexcept
    {
        return status;
    }

    void set_amount(PaymentAmountDetailsResponse value)
    {
        amount = std::move(value);
    }

    [[nodiscard]] const PaymentAmountDetailsResponse& get_amount() const noexcept
    {
        return amount;
    }

    void set_created(PrimitiveDateTimeValue value)
    {
        created = std::move(value);
    }

    [[nodiscard]] const PrimitiveDateTimeValue& get_created() const noexcept
    {
        return created;
    }

    void set_payment_method_type(std::optional<PaymentMethodValue> value)
    {
        payment_method_type = std::move(value);
    }

    [[nodiscard]] const std::optional<PaymentMethodValue>& get_payment_method_type() const noexcept
    {
        return payment_method_type;
    }

    void set_payment_method_subtype(std::optional<PaymentMethodTypeValue> value)
    {
        payment_method_subtype = std::move(value);
    }

    [[nodiscard]] const std::optional<PaymentMethodTypeValue>&
    get_payment_method_subtype() const noexcept
    {
        return payment_method_subtype;
    }

    void set_connector(std::optional<std::string> value)
    {
        connector = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_connector() const noexcept
    {
        return connector;
    }

    void set_merchant_connector_id(std::optional<MerchantConnectorAccountIdValue> value)
    {
        merchant_connector_id = std::move(value);
    }

    [[nodiscard]] const std::optional<MerchantConnectorAccountIdValue>&
    get_merchant_connector_id() const noexcept
    {
        return merchant_connector_id;
    }

    void set_customer(std::optional<CustomerDetailsResponse> value)
    {
        customer = std::move(value);
    }

    [[nodiscard]] const std::optional<CustomerDetailsResponse>& get_customer() const noexcept
    {
        return customer;
    }

    void set_merchant_reference_id(std::optional<PaymentReferenceIdValue> value)
    {
        merchant_reference_id = std::move(value);
    }

    [[nodiscard]] const std::optional<PaymentReferenceIdValue>&
    get_merchant_reference_id() const noexcept
    {
        return merchant_reference_id;
    }

    void set_description(std::optional<std::string> value)
    {
        description = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_description() const noexcept
    {
        return description;
    }

    void set_attempt_count(std::int16_t value)
    {
        attempt_count = std::move(value);
    }

    [[nodiscard]] const std::int16_t& get_attempt_count() const noexcept
    {
        return attempt_count;
    }

    void set_error(std::optional<ErrorDetails> value)
    {
        error = std::move(value);
    }

    [[nodiscard]] const std::optional<ErrorDetails>& get_error() const noexcept
    {
        return error;
    }

    void set_cancellation_reason(std::optional<std::string> value)
    {
        cancellation_reason = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_cancellation_reason() const noexcept
    {
        return cancellation_reason;
    }

    void set_modified_at(std::optional<PrimitiveDateTimeValue> value)
    {
        modified_at = std::move(value);
    }

    [[nodiscard]] const std::optional<PrimitiveDateTimeValue>& get_modified_at() const noexcept
    {
        return modified_at;
    }

    void set_last_attempt_at(std::optional<PrimitiveDateTimeValue> value)
    {
        last_attempt_at = std::move(value);
    }

    [[nodiscard]] const std::optional<PrimitiveDateTimeValue>& get_last_attempt_at() const noexcept
    {
        return last_attempt_at;
    }

    GlobalPaymentIdValue id{};
    MerchantIdValue merchant_id{};
    ProfileIdValue profile_id{};
    std::optional<GlobalCustomerIdValue> customer_id{};
    RecoveryStatusValue status{};
    PaymentAmountDetailsResponse amount{};
    PrimitiveDateTimeValue created{};
    std::optional<PaymentMethodValue> payment_method_type{};
    std::optional<PaymentMethodTypeValue> payment_method_subtype{};
    std::optional<std::string> connector{};
    std::optional<MerchantConnectorAccountIdValue> merchant_connector_id{};
    std::optional<CustomerDetailsResponse> customer{};
    std::optional<PaymentReferenceIdValue> merchant_reference_id{};
    std::optional<std::string> description{};
    std::int16_t attempt_count{};
    std::optional<ErrorDetails> error{};
    std::optional<std::string> cancellation_reason{};
    std::optional<PrimitiveDateTimeValue> modified_at{};
    std::optional<PrimitiveDateTimeValue> last_attempt_at{};
};

class ThirdPartySdkSessionResponse : public core::behavior::Serializer<ThirdPartySdkSessionResponse>
{
public:
    void set_secrets(SecretInfoToInitiateSdk value)
    {
        secrets = std::move(value);
    }

    [[nodiscard]] const SecretInfoToInitiateSdk& get_secrets() const noexcept
    {
        return secrets;
    }

    SecretInfoToInitiateSdk secrets{};
};

class ExternalThreeDsData : public core::behavior::Serializer<ExternalThreeDsData>
{
public:
    void set_authentication_cryptogram(Cryptogram value)
    {
        authentication_cryptogram = std::move(value);
    }

    [[nodiscard]] const Cryptogram& get_authentication_cryptogram() const noexcept
    {
        return authentication_cryptogram;
    }

    void set_ds_trans_id(std::string value)
    {
        ds_trans_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_ds_trans_id() const noexcept
    {
        return ds_trans_id;
    }

    void set_version(SemanticVersionValue value)
    {
        version = std::move(value);
    }

    [[nodiscard]] const SemanticVersionValue& get_version() const noexcept
    {
        return version;
    }

    void set_eci(std::string value)
    {
        eci = std::move(value);
    }

    [[nodiscard]] const std::string& get_eci() const noexcept
    {
        return eci;
    }

    void set_transaction_status(TransactionStatusValue value)
    {
        transaction_status = std::move(value);
    }

    [[nodiscard]] const TransactionStatusValue& get_transaction_status() const noexcept
    {
        return transaction_status;
    }

    void set_exemption_indicator(std::optional<ExemptionIndicatorValue> value)
    {
        exemption_indicator = std::move(value);
    }

    [[nodiscard]] const std::optional<ExemptionIndicatorValue>&
    get_exemption_indicator() const noexcept
    {
        return exemption_indicator;
    }

    void set_network_params(std::optional<NetworkParams> value)
    {
        network_params = std::move(value);
    }

    [[nodiscard]] const std::optional<NetworkParams>& get_network_params() const noexcept
    {
        return network_params;
    }

    Cryptogram authentication_cryptogram{};
    std::string ds_trans_id{};
    SemanticVersionValue version{};
    std::string eci{};
    TransactionStatusValue transaction_status{};
    std::optional<ExemptionIndicatorValue> exemption_indicator{};
    std::optional<NetworkParams> network_params{};
};

class PaymentErrorDetails : public core::behavior::Serializer<PaymentErrorDetails>
{
public:
    void set_unified_details(std::optional<ApiUnifiedErrorDetails> value)
    {
        unified_details = std::move(value);
    }

    [[nodiscard]] const std::optional<ApiUnifiedErrorDetails>& get_unified_details() const noexcept
    {
        return unified_details;
    }

    void set_issuer_details(std::optional<ApiIssuerErrorDetails> value)
    {
        issuer_details = std::move(value);
    }

    [[nodiscard]] const std::optional<ApiIssuerErrorDetails>& get_issuer_details() const noexcept
    {
        return issuer_details;
    }

    void set_connector_details(std::optional<ApiConnectorErrorDetails> value)
    {
        connector_details = std::move(value);
    }

    [[nodiscard]] const std::optional<ApiConnectorErrorDetails>&
    get_connector_details() const noexcept
    {
        return connector_details;
    }

    std::optional<ApiUnifiedErrorDetails> unified_details{};
    std::optional<ApiIssuerErrorDetails> issuer_details{};
    std::optional<ApiConnectorErrorDetails> connector_details{};
};

enum class PaymentLinkData
{
    PaymentLinkDetails,
    PaymentLinkStatusDetails,
};

class PaymentListFilterConstraints : public core::behavior::Serializer<PaymentListFilterConstraints>
{
public:
    void set_query(std::optional<std::string> value)
    {
        query = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_query() const noexcept
    {
        return query;
    }

    void set_payment_id(std::optional<PaymentIdValue> value)
    {
        payment_id = std::move(value);
    }

    [[nodiscard]] const std::optional<PaymentIdValue>& get_payment_id() const noexcept
    {
        return payment_id;
    }

    void set_profile_id(std::optional<ProfileIdValue> value)
    {
        profile_id = std::move(value);
    }

    [[nodiscard]] const std::optional<ProfileIdValue>& get_profile_id() const noexcept
    {
        return profile_id;
    }

    void set_customer_id(std::optional<CustomerIdValue> value)
    {
        customer_id = std::move(value);
    }

    [[nodiscard]] const std::optional<CustomerIdValue>& get_customer_id() const noexcept
    {
        return customer_id;
    }

    void set_limit(PageSizeValue value)
    {
        limit = std::move(value);
    }

    [[nodiscard]] const PageSizeValue& get_limit() const noexcept
    {
        return limit;
    }

    void set_offset(PageOffsetValue value)
    {
        offset = std::move(value);
    }

    [[nodiscard]] const PageOffsetValue& get_offset() const noexcept
    {
        return offset;
    }

    void set_amount_filter(std::optional<AmountFilter> value)
    {
        amount_filter = std::move(value);
    }

    [[nodiscard]] const std::optional<AmountFilter>& get_amount_filter() const noexcept
    {
        return amount_filter;
    }

    void set_time_range(std::optional<TimeRangeValue> value)
    {
        time_range = std::move(value);
    }

    [[nodiscard]] const std::optional<TimeRangeValue>& get_time_range() const noexcept
    {
        return time_range;
    }

    void set_connector(std::optional<std::vector<ConnectorValue>> value)
    {
        connector = std::move(value);
    }

    [[nodiscard]] const std::optional<std::vector<ConnectorValue>>& get_connector() const noexcept
    {
        return connector;
    }

    void set_currency(std::optional<std::vector<CurrencyValue>> value)
    {
        currency = std::move(value);
    }

    [[nodiscard]] const std::optional<std::vector<CurrencyValue>>& get_currency() const noexcept
    {
        return currency;
    }

    void set_status(std::optional<std::vector<IntentStatusValue>> value)
    {
        status = std::move(value);
    }

    [[nodiscard]] const std::optional<std::vector<IntentStatusValue>>& get_status() const noexcept
    {
        return status;
    }

    void set_payment_method(std::optional<std::vector<PaymentMethodValue>> value)
    {
        payment_method = std::move(value);
    }

    [[nodiscard]] const std::optional<std::vector<PaymentMethodValue>>&
    get_payment_method() const noexcept
    {
        return payment_method;
    }

    void set_payment_method_type(std::optional<std::vector<PaymentMethodTypeValue>> value)
    {
        payment_method_type = std::move(value);
    }

    [[nodiscard]] const std::optional<std::vector<PaymentMethodTypeValue>>&
    get_payment_method_type() const noexcept
    {
        return payment_method_type;
    }

    void set_authentication_type(std::optional<std::vector<AuthenticationTypeValue>> value)
    {
        authentication_type = std::move(value);
    }

    [[nodiscard]] const std::optional<std::vector<AuthenticationTypeValue>>&
    get_authentication_type() const noexcept
    {
        return authentication_type;
    }

    void set_merchant_connector_id(
        std::optional<std::vector<MerchantConnectorAccountIdValue>> value
    )
    {
        merchant_connector_id = std::move(value);
    }

    [[nodiscard]] const std::optional<std::vector<MerchantConnectorAccountIdValue>>&
    get_merchant_connector_id() const noexcept
    {
        return merchant_connector_id;
    }

    void set_order(Order value)
    {
        order = std::move(value);
    }

    [[nodiscard]] const Order& get_order() const noexcept
    {
        return order;
    }

    void set_card_network(std::optional<std::vector<CardNetworkValue>> value)
    {
        card_network = std::move(value);
    }

    [[nodiscard]] const std::optional<std::vector<CardNetworkValue>>&
    get_card_network() const noexcept
    {
        return card_network;
    }

    void set_card_last_4(std::optional<std::vector<std::string>> value)
    {
        card_last_4 = std::move(value);
    }

    [[nodiscard]] const std::optional<std::vector<std::string>>& get_card_last_4() const noexcept
    {
        return card_last_4;
    }

    void set_active_attempt_id(std::optional<std::vector<std::string>> value)
    {
        active_attempt_id = std::move(value);
    }

    [[nodiscard]] const std::optional<std::vector<std::string>>&
    get_active_attempt_id() const noexcept
    {
        return active_attempt_id;
    }

    void set_card_issuer(std::optional<std::vector<std::string>> value)
    {
        card_issuer = std::move(value);
    }

    [[nodiscard]] const std::optional<std::vector<std::string>>& get_card_issuer() const noexcept
    {
        return card_issuer;
    }

    void set_routing_approach(std::optional<std::vector<RoutingApproachValue>> value)
    {
        routing_approach = std::move(value);
    }

    [[nodiscard]] const std::optional<std::vector<RoutingApproachValue>>&
    get_routing_approach() const noexcept
    {
        return routing_approach;
    }

    void set_refunds_status(std::optional<std::vector<std::string>> value)
    {
        refunds_status = std::move(value);
    }

    [[nodiscard]] const std::optional<std::vector<std::string>>& get_refunds_status() const noexcept
    {
        return refunds_status;
    }

    void set_dispute_status(std::optional<std::vector<std::string>> value)
    {
        dispute_status = std::move(value);
    }

    [[nodiscard]] const std::optional<std::vector<std::string>>& get_dispute_status() const noexcept
    {
        return dispute_status;
    }

    void set_client_source(std::optional<std::vector<std::string>> value)
    {
        client_source = std::move(value);
    }

    [[nodiscard]] const std::optional<std::vector<std::string>>& get_client_source() const noexcept
    {
        return client_source;
    }

    void set_client_version(std::optional<std::vector<std::string>> value)
    {
        client_version = std::move(value);
    }

    [[nodiscard]] const std::optional<std::vector<std::string>>& get_client_version() const noexcept
    {
        return client_version;
    }

    void set_first_attempt(std::optional<std::vector<bool>> value)
    {
        first_attempt = std::move(value);
    }

    [[nodiscard]] const std::optional<std::vector<bool>>& get_first_attempt() const noexcept
    {
        return first_attempt;
    }

    void set_merchant_order_reference_id(std::optional<std::string> value)
    {
        merchant_order_reference_id = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_merchant_order_reference_id() const noexcept
    {
        return merchant_order_reference_id;
    }

    void set_card_discovery(std::optional<std::vector<CardDiscoveryValue>> value)
    {
        card_discovery = std::move(value);
    }

    [[nodiscard]] const std::optional<std::vector<CardDiscoveryValue>>&
    get_card_discovery() const noexcept
    {
        return card_discovery;
    }

    void set_customer_email(std::optional<EmailValue> value)
    {
        customer_email = std::move(value);
    }

    [[nodiscard]] const std::optional<EmailValue>& get_customer_email() const noexcept
    {
        return customer_email;
    }

    std::optional<std::string> query{};
    std::optional<PaymentIdValue> payment_id{};
    std::optional<ProfileIdValue> profile_id{};
    std::optional<CustomerIdValue> customer_id{};
    PageSizeValue limit{};
    PageOffsetValue offset{};
    std::optional<AmountFilter> amount_filter{};
    std::optional<TimeRangeValue> time_range{};
    std::optional<std::vector<ConnectorValue>> connector{};
    std::optional<std::vector<CurrencyValue>> currency{};
    std::optional<std::vector<IntentStatusValue>> status{};
    std::optional<std::vector<PaymentMethodValue>> payment_method{};
    std::optional<std::vector<PaymentMethodTypeValue>> payment_method_type{};
    std::optional<std::vector<AuthenticationTypeValue>> authentication_type{};
    std::optional<std::vector<MerchantConnectorAccountIdValue>> merchant_connector_id{};
    Order order{};
    std::optional<std::vector<CardNetworkValue>> card_network{};
    std::optional<std::vector<std::string>> card_last_4{};
    std::optional<std::vector<std::string>> active_attempt_id{};
    std::optional<std::vector<std::string>> card_issuer{};
    std::optional<std::vector<RoutingApproachValue>> routing_approach{};
    std::optional<std::vector<std::string>> refunds_status{};
    std::optional<std::vector<std::string>> dispute_status{};
    std::optional<std::vector<std::string>> client_source{};
    std::optional<std::vector<std::string>> client_version{};
    std::optional<std::vector<bool>> first_attempt{};
    std::optional<std::string> merchant_order_reference_id{};
    std::optional<std::vector<CardDiscoveryValue>> card_discovery{};
    std::optional<EmailValue> customer_email{};
};

class PaymentListResponse : public core::behavior::Serializer<PaymentListResponse>
{
public:
    void set_count(std::size_t value)
    {
        count = std::move(value);
    }

    [[nodiscard]] const std::size_t& get_count() const noexcept
    {
        return count;
    }

    void set_total_count(std::int64_t value)
    {
        total_count = std::move(value);
    }

    [[nodiscard]] const std::int64_t& get_total_count() const noexcept
    {
        return total_count;
    }

    void set_data(std::vector<PaymentsListResponseItem> value)
    {
        data = std::move(value);
    }

    [[nodiscard]] const std::vector<PaymentsListResponseItem>& get_data() const noexcept
    {
        return data;
    }

    std::size_t count{};
    std::int64_t total_count{};
    std::vector<PaymentsListResponseItem> data{};
};

class PaymentsCancelPostCaptureRequest :
    public core::behavior::Serializer<PaymentsCancelPostCaptureRequest>
{
public:
    void set_payment_id(PaymentIdValue value)
    {
        payment_id = std::move(value);
    }

    [[nodiscard]] const PaymentIdValue& get_payment_id() const noexcept
    {
        return payment_id;
    }

    void set_cancellation_reason(std::optional<std::string> value)
    {
        cancellation_reason = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_cancellation_reason() const noexcept
    {
        return cancellation_reason;
    }

    void set_payment_method_data(std::optional<PaymentMethodData> value)
    {
        payment_method_data = std::move(value);
    }

    [[nodiscard]] const std::optional<PaymentMethodData>& get_payment_method_data() const noexcept
    {
        return payment_method_data;
    }

    PaymentIdValue payment_id{};
    std::optional<std::string> cancellation_reason{};
    std::optional<PaymentMethodData> payment_method_data{};
};

class PaymentsCancelRequest : public core::behavior::Serializer<PaymentsCancelRequest>
{
public:
    void set_payment_id(PaymentIdValue value)
    {
        payment_id = std::move(value);
    }

    [[nodiscard]] const PaymentIdValue& get_payment_id() const noexcept
    {
        return payment_id;
    }

    void set_cancellation_reason(std::optional<std::string> value)
    {
        cancellation_reason = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_cancellation_reason() const noexcept
    {
        return cancellation_reason;
    }

    void set_merchant_connector_details(std::optional<MerchantConnectorDetailsWrapValue> value)
    {
        merchant_connector_details = std::move(value);
    }

    [[nodiscard]] const std::optional<MerchantConnectorDetailsWrapValue>&
    get_merchant_connector_details() const noexcept
    {
        return merchant_connector_details;
    }

    void set_all_keys_required(std::optional<bool> value)
    {
        all_keys_required = std::move(value);
    }

    [[nodiscard]] const std::optional<bool>& get_all_keys_required() const noexcept
    {
        return all_keys_required;
    }

    void set_payment_method_data(std::optional<PaymentMethodData> value)
    {
        payment_method_data = std::move(value);
    }

    [[nodiscard]] const std::optional<PaymentMethodData>& get_payment_method_data() const noexcept
    {
        return payment_method_data;
    }

    PaymentIdValue payment_id{};
    std::optional<std::string> cancellation_reason{};
    std::optional<MerchantConnectorDetailsWrapValue> merchant_connector_details{};
    std::optional<bool> all_keys_required{};
    std::optional<PaymentMethodData> payment_method_data{};
};

class PaymentsCaptureRequest : public core::behavior::Serializer<PaymentsCaptureRequest>
{
public:
    void set_payment_id(PaymentIdValue value)
    {
        payment_id = std::move(value);
    }

    [[nodiscard]] const PaymentIdValue& get_payment_id() const noexcept
    {
        return payment_id;
    }

    void set_merchant_id(std::optional<MerchantIdValue> value)
    {
        merchant_id = std::move(value);
    }

    [[nodiscard]] const std::optional<MerchantIdValue>& get_merchant_id() const noexcept
    {
        return merchant_id;
    }

    void set_amount_to_capture(std::optional<MinorUnitValue> value)
    {
        amount_to_capture = std::move(value);
    }

    [[nodiscard]] const std::optional<MinorUnitValue>& get_amount_to_capture() const noexcept
    {
        return amount_to_capture;
    }

    void set_refund_uncaptured_amount(std::optional<bool> value)
    {
        refund_uncaptured_amount = std::move(value);
    }

    [[nodiscard]] const std::optional<bool>& get_refund_uncaptured_amount() const noexcept
    {
        return refund_uncaptured_amount;
    }

    void set_statement_descriptor_suffix(std::optional<std::string> value)
    {
        statement_descriptor_suffix = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_statement_descriptor_suffix() const noexcept
    {
        return statement_descriptor_suffix;
    }

    void set_statement_descriptor_prefix(std::optional<std::string> value)
    {
        statement_descriptor_prefix = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_statement_descriptor_prefix() const noexcept
    {
        return statement_descriptor_prefix;
    }

    void set_merchant_connector_details(std::optional<MerchantConnectorDetailsWrapValue> value)
    {
        merchant_connector_details = std::move(value);
    }

    [[nodiscard]] const std::optional<MerchantConnectorDetailsWrapValue>&
    get_merchant_connector_details() const noexcept
    {
        return merchant_connector_details;
    }

    void set_all_keys_required(std::optional<bool> value)
    {
        all_keys_required = std::move(value);
    }

    [[nodiscard]] const std::optional<bool>& get_all_keys_required() const noexcept
    {
        return all_keys_required;
    }

    void set_payment_method_data(std::optional<PaymentMethodData> value)
    {
        payment_method_data = std::move(value);
    }

    [[nodiscard]] const std::optional<PaymentMethodData>& get_payment_method_data() const noexcept
    {
        return payment_method_data;
    }

    void set_feature_metadata(std::optional<CaptureFeatureMetadataValue> value)
    {
        feature_metadata = std::move(value);
    }

    [[nodiscard]] const std::optional<CaptureFeatureMetadataValue>&
    get_feature_metadata() const noexcept
    {
        return feature_metadata;
    }

    PaymentIdValue payment_id{};
    std::optional<MerchantIdValue> merchant_id{};
    std::optional<MinorUnitValue> amount_to_capture{};
    std::optional<bool> refund_uncaptured_amount{};
    std::optional<std::string> statement_descriptor_suffix{};
    std::optional<std::string> statement_descriptor_prefix{};
    std::optional<MerchantConnectorDetailsWrapValue> merchant_connector_details{};
    std::optional<bool> all_keys_required{};
    std::optional<PaymentMethodData> payment_method_data{};
    std::optional<CaptureFeatureMetadataValue> feature_metadata{};
};

class PaymentsCompleteAuthorizeRequest :
    public core::behavior::Serializer<PaymentsCompleteAuthorizeRequest>
{
public:
    void set_payment_id(PaymentIdValue value)
    {
        payment_id = std::move(value);
    }

    [[nodiscard]] const PaymentIdValue& get_payment_id() const noexcept
    {
        return payment_id;
    }

    void set_shipping(std::optional<Address> value)
    {
        shipping = std::move(value);
    }

    [[nodiscard]] const std::optional<Address>& get_shipping() const noexcept
    {
        return shipping;
    }

    void set_client_secret(std::optional<SecretValue> value)
    {
        client_secret = std::move(value);
    }

    [[nodiscard]] const std::optional<SecretValue>& get_client_secret() const noexcept
    {
        return client_secret;
    }

    void set_threeds_method_comp_ind(std::optional<ThreeDsCompletionIndicator> value)
    {
        threeds_method_comp_ind = std::move(value);
    }

    [[nodiscard]] const std::optional<ThreeDsCompletionIndicator>&
    get_threeds_method_comp_ind() const noexcept
    {
        return threeds_method_comp_ind;
    }

    PaymentIdValue payment_id{};
    std::optional<Address> shipping{};
    std::optional<SecretValue> client_secret{};
    std::optional<ThreeDsCompletionIndicator> threeds_method_comp_ind{};
};

class PaymentsCreateIntentRequest : public core::behavior::Serializer<PaymentsCreateIntentRequest>
{
public:
    void set_amount_details(AmountDetails value)
    {
        amount_details = std::move(value);
    }

    [[nodiscard]] const AmountDetails& get_amount_details() const noexcept
    {
        return amount_details;
    }

    void set_merchant_reference_id(std::optional<PaymentReferenceIdValue> value)
    {
        merchant_reference_id = std::move(value);
    }

    [[nodiscard]] const std::optional<PaymentReferenceIdValue>&
    get_merchant_reference_id() const noexcept
    {
        return merchant_reference_id;
    }

    void set_routing_algorithm_id(std::optional<RoutingIdValue> value)
    {
        routing_algorithm_id = std::move(value);
    }

    [[nodiscard]] const std::optional<RoutingIdValue>& get_routing_algorithm_id() const noexcept
    {
        return routing_algorithm_id;
    }

    void set_capture_method(std::optional<CaptureMethodValue> value)
    {
        capture_method = std::move(value);
    }

    [[nodiscard]] const std::optional<CaptureMethodValue>& get_capture_method() const noexcept
    {
        return capture_method;
    }

    void set_authentication_type(std::optional<AuthenticationTypeValue> value)
    {
        authentication_type = std::move(value);
    }

    [[nodiscard]] const std::optional<AuthenticationTypeValue>&
    get_authentication_type() const noexcept
    {
        return authentication_type;
    }

    void set_billing(std::optional<Address> value)
    {
        billing = std::move(value);
    }

    [[nodiscard]] const std::optional<Address>& get_billing() const noexcept
    {
        return billing;
    }

    void set_customer_details(std::optional<CustomerDetails> value)
    {
        customer_details = std::move(value);
    }

    [[nodiscard]] const std::optional<CustomerDetails>& get_customer_details() const noexcept
    {
        return customer_details;
    }

    void set_shipping(std::optional<Address> value)
    {
        shipping = std::move(value);
    }

    [[nodiscard]] const std::optional<Address>& get_shipping() const noexcept
    {
        return shipping;
    }

    void set_customer_id(std::optional<GlobalCustomerIdValue> value)
    {
        customer_id = std::move(value);
    }

    [[nodiscard]] const std::optional<GlobalCustomerIdValue>& get_customer_id() const noexcept
    {
        return customer_id;
    }

    void set_customer_present(std::optional<PresenceOfCustomerDuringPaymentValue> value)
    {
        customer_present = std::move(value);
    }

    [[nodiscard]] const std::optional<PresenceOfCustomerDuringPaymentValue>&
    get_customer_present() const noexcept
    {
        return customer_present;
    }

    void set_description(std::optional<DescriptionValue> value)
    {
        description = std::move(value);
    }

    [[nodiscard]] const std::optional<DescriptionValue>& get_description() const noexcept
    {
        return description;
    }

    void set_return_url(std::optional<UrlValue> value)
    {
        return_url = std::move(value);
    }

    [[nodiscard]] const std::optional<UrlValue>& get_return_url() const noexcept
    {
        return return_url;
    }

    void set_setup_future_usage(std::optional<FutureUsageValue> value)
    {
        setup_future_usage = std::move(value);
    }

    [[nodiscard]] const std::optional<FutureUsageValue>& get_setup_future_usage() const noexcept
    {
        return setup_future_usage;
    }

    void set_apply_mit_exemption(std::optional<MitExemptionRequestValue> value)
    {
        apply_mit_exemption = std::move(value);
    }

    [[nodiscard]] const std::optional<MitExemptionRequestValue>&
    get_apply_mit_exemption() const noexcept
    {
        return apply_mit_exemption;
    }

    void set_statement_descriptor(std::optional<StatementDescriptorValue> value)
    {
        statement_descriptor = std::move(value);
    }

    [[nodiscard]] const std::optional<StatementDescriptorValue>&
    get_statement_descriptor() const noexcept
    {
        return statement_descriptor;
    }

    AmountDetails amount_details{};
    std::optional<PaymentReferenceIdValue> merchant_reference_id{};
    std::optional<RoutingIdValue> routing_algorithm_id{};
    std::optional<CaptureMethodValue> capture_method{};
    std::optional<AuthenticationTypeValue> authentication_type{};
    std::optional<Address> billing{};
    std::optional<CustomerDetails> customer_details{};
    std::optional<Address> shipping{};
    std::optional<GlobalCustomerIdValue> customer_id{};
    std::optional<PresenceOfCustomerDuringPaymentValue> customer_present{};
    std::optional<DescriptionValue> description{};
    std::optional<UrlValue> return_url{};
    std::optional<FutureUsageValue> setup_future_usage{};
    std::optional<MitExemptionRequestValue> apply_mit_exemption{};
    std::optional<StatementDescriptorValue> statement_descriptor{};
};

class PaymentsDynamicTaxCalculationRequest :
    public core::behavior::Serializer<PaymentsDynamicTaxCalculationRequest>
{
public:
    void set_payment_id(PaymentIdValue value)
    {
        payment_id = std::move(value);
    }

    [[nodiscard]] const PaymentIdValue& get_payment_id() const noexcept
    {
        return payment_id;
    }

    void set_shipping(Address value)
    {
        shipping = std::move(value);
    }

    [[nodiscard]] const Address& get_shipping() const noexcept
    {
        return shipping;
    }

    void set_client_secret(std::optional<SecretValue> value)
    {
        client_secret = std::move(value);
    }

    [[nodiscard]] const std::optional<SecretValue>& get_client_secret() const noexcept
    {
        return client_secret;
    }

    void set_payment_method_type(PaymentMethodTypeValue value)
    {
        payment_method_type = std::move(value);
    }

    [[nodiscard]] const PaymentMethodTypeValue& get_payment_method_type() const noexcept
    {
        return payment_method_type;
    }

    void set_session_id(std::optional<std::string> value)
    {
        session_id = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_session_id() const noexcept
    {
        return session_id;
    }

    PaymentIdValue payment_id{};
    Address shipping{};
    std::optional<SecretValue> client_secret{};
    PaymentMethodTypeValue payment_method_type{};
    std::optional<std::string> session_id{};
};

class PaymentsEligibilityResponse : public core::behavior::Serializer<PaymentsEligibilityResponse>
{
public:
    void set_payment_id(PaymentIdValue value)
    {
        payment_id = std::move(value);
    }

    [[nodiscard]] const PaymentIdValue& get_payment_id() const noexcept
    {
        return payment_id;
    }

    void set_sdk_next_action(SdkNextAction value)
    {
        sdk_next_action = std::move(value);
    }

    [[nodiscard]] const SdkNextAction& get_sdk_next_action() const noexcept
    {
        return sdk_next_action;
    }

    void set_surcharge_details(std::optional<SurchargeDetailsResponseValue> value)
    {
        surcharge_details = std::move(value);
    }

    [[nodiscard]] const std::optional<SurchargeDetailsResponseValue>&
    get_surcharge_details() const noexcept
    {
        return surcharge_details;
    }

    void set_amount_details(std::optional<EligibilityAmountDetails> value)
    {
        amount_details = std::move(value);
    }

    [[nodiscard]] const std::optional<EligibilityAmountDetails>& get_amount_details() const noexcept
    {
        return amount_details;
    }

    void set_offer_details(std::optional<EligibilityOfferDetails> value)
    {
        offer_details = std::move(value);
    }

    [[nodiscard]] const std::optional<EligibilityOfferDetails>& get_offer_details() const noexcept
    {
        return offer_details;
    }

    PaymentIdValue payment_id{};
    SdkNextAction sdk_next_action{};
    std::optional<SurchargeDetailsResponseValue> surcharge_details{};
    std::optional<EligibilityAmountDetails> amount_details{};
    std::optional<EligibilityOfferDetails> offer_details{};
};

class PaymentsIncrementalAuthorizationRequest :
    public core::behavior::Serializer<PaymentsIncrementalAuthorizationRequest>
{
public:
    void set_payment_id(PaymentIdValue value)
    {
        payment_id = std::move(value);
    }

    [[nodiscard]] const PaymentIdValue& get_payment_id() const noexcept
    {
        return payment_id;
    }

    void set_amount(MinorUnitValue value)
    {
        amount = std::move(value);
    }

    [[nodiscard]] const MinorUnitValue& get_amount() const noexcept
    {
        return amount;
    }

    void set_reason(std::optional<std::string> value)
    {
        reason = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_reason() const noexcept
    {
        return reason;
    }

    void set_payment_method_data(std::optional<PaymentMethodData> value)
    {
        payment_method_data = std::move(value);
    }

    [[nodiscard]] const std::optional<PaymentMethodData>& get_payment_method_data() const noexcept
    {
        return payment_method_data;
    }

    PaymentIdValue payment_id{};
    MinorUnitValue amount{};
    std::optional<std::string> reason{};
    std::optional<PaymentMethodData> payment_method_data{};
};

class PaymentsIntentResponse : public core::behavior::Serializer<PaymentsIntentResponse>
{
public:
    void set_id(GlobalPaymentIdValue value)
    {
        id = std::move(value);
    }

    [[nodiscard]] const GlobalPaymentIdValue& get_id() const noexcept
    {
        return id;
    }

    void set_status(IntentStatusValue value)
    {
        status = std::move(value);
    }

    [[nodiscard]] const IntentStatusValue& get_status() const noexcept
    {
        return status;
    }

    void set_amount_details(AmountDetailsResponse value)
    {
        amount_details = std::move(value);
    }

    [[nodiscard]] const AmountDetailsResponse& get_amount_details() const noexcept
    {
        return amount_details;
    }

    void set_client_secret(std::optional<SecretValue> value)
    {
        client_secret = std::move(value);
    }

    [[nodiscard]] const std::optional<SecretValue>& get_client_secret() const noexcept
    {
        return client_secret;
    }

    void set_profile_id(ProfileIdValue value)
    {
        profile_id = std::move(value);
    }

    [[nodiscard]] const ProfileIdValue& get_profile_id() const noexcept
    {
        return profile_id;
    }

    void set_merchant_reference_id(std::optional<PaymentReferenceIdValue> value)
    {
        merchant_reference_id = std::move(value);
    }

    [[nodiscard]] const std::optional<PaymentReferenceIdValue>&
    get_merchant_reference_id() const noexcept
    {
        return merchant_reference_id;
    }

    void set_routing_algorithm_id(std::optional<RoutingIdValue> value)
    {
        routing_algorithm_id = std::move(value);
    }

    [[nodiscard]] const std::optional<RoutingIdValue>& get_routing_algorithm_id() const noexcept
    {
        return routing_algorithm_id;
    }

    void set_capture_method(CaptureMethodValue value)
    {
        capture_method = std::move(value);
    }

    [[nodiscard]] const CaptureMethodValue& get_capture_method() const noexcept
    {
        return capture_method;
    }

    void set_authentication_type(std::optional<AuthenticationTypeValue> value)
    {
        authentication_type = std::move(value);
    }

    [[nodiscard]] const std::optional<AuthenticationTypeValue>&
    get_authentication_type() const noexcept
    {
        return authentication_type;
    }

    void set_billing(std::optional<Address> value)
    {
        billing = std::move(value);
    }

    [[nodiscard]] const std::optional<Address>& get_billing() const noexcept
    {
        return billing;
    }

    void set_shipping(std::optional<Address> value)
    {
        shipping = std::move(value);
    }

    [[nodiscard]] const std::optional<Address>& get_shipping() const noexcept
    {
        return shipping;
    }

    void set_customer_id(std::optional<GlobalCustomerIdValue> value)
    {
        customer_id = std::move(value);
    }

    [[nodiscard]] const std::optional<GlobalCustomerIdValue>& get_customer_id() const noexcept
    {
        return customer_id;
    }

    void set_customer_present(PresenceOfCustomerDuringPaymentValue value)
    {
        customer_present = std::move(value);
    }

    [[nodiscard]] const PresenceOfCustomerDuringPaymentValue& get_customer_present() const noexcept
    {
        return customer_present;
    }

    void set_description(std::optional<DescriptionValue> value)
    {
        description = std::move(value);
    }

    [[nodiscard]] const std::optional<DescriptionValue>& get_description() const noexcept
    {
        return description;
    }

    void set_return_url(std::optional<UrlValue> value)
    {
        return_url = std::move(value);
    }

    [[nodiscard]] const std::optional<UrlValue>& get_return_url() const noexcept
    {
        return return_url;
    }

    void set_setup_future_usage(FutureUsageValue value)
    {
        setup_future_usage = std::move(value);
    }

    [[nodiscard]] const FutureUsageValue& get_setup_future_usage() const noexcept
    {
        return setup_future_usage;
    }

    void set_apply_mit_exemption(MitExemptionRequestValue value)
    {
        apply_mit_exemption = std::move(value);
    }

    [[nodiscard]] const MitExemptionRequestValue& get_apply_mit_exemption() const noexcept
    {
        return apply_mit_exemption;
    }

    void set_statement_descriptor(std::optional<StatementDescriptorValue> value)
    {
        statement_descriptor = std::move(value);
    }

    [[nodiscard]] const std::optional<StatementDescriptorValue>&
    get_statement_descriptor() const noexcept
    {
        return statement_descriptor;
    }

    GlobalPaymentIdValue id{};
    IntentStatusValue status{};
    AmountDetailsResponse amount_details{};
    std::optional<SecretValue> client_secret{};
    ProfileIdValue profile_id{};
    std::optional<PaymentReferenceIdValue> merchant_reference_id{};
    std::optional<RoutingIdValue> routing_algorithm_id{};
    CaptureMethodValue capture_method{};
    std::optional<AuthenticationTypeValue> authentication_type{};
    std::optional<Address> billing{};
    std::optional<Address> shipping{};
    std::optional<GlobalCustomerIdValue> customer_id{};
    PresenceOfCustomerDuringPaymentValue customer_present{};
    std::optional<DescriptionValue> description{};
    std::optional<UrlValue> return_url{};
    FutureUsageValue setup_future_usage{};
    MitExemptionRequestValue apply_mit_exemption{};
    std::optional<StatementDescriptorValue> statement_descriptor{};
};

class PaymentsPostSessionTokensResponse :
    public core::behavior::Serializer<PaymentsPostSessionTokensResponse>
{
public:
    void set_payment_id(PaymentIdValue value)
    {
        payment_id = std::move(value);
    }

    [[nodiscard]] const PaymentIdValue& get_payment_id() const noexcept
    {
        return payment_id;
    }

    void set_next_action(std::optional<NextActionData> value)
    {
        next_action = std::move(value);
    }

    [[nodiscard]] const std::optional<NextActionData>& get_next_action() const noexcept
    {
        return next_action;
    }

    void set_status(IntentStatusValue value)
    {
        status = std::move(value);
    }

    [[nodiscard]] const IntentStatusValue& get_status() const noexcept
    {
        return status;
    }

    PaymentIdValue payment_id{};
    std::optional<NextActionData> next_action{};
    IntentStatusValue status{};
};

class PaymentsUpdateIntentRequest : public core::behavior::Serializer<PaymentsUpdateIntentRequest>
{
public:
    void set_amount_details(std::optional<AmountDetailsUpdate> value)
    {
        amount_details = std::move(value);
    }

    [[nodiscard]] const std::optional<AmountDetailsUpdate>& get_amount_details() const noexcept
    {
        return amount_details;
    }

    void set_routing_algorithm_id(std::optional<RoutingIdValue> value)
    {
        routing_algorithm_id = std::move(value);
    }

    [[nodiscard]] const std::optional<RoutingIdValue>& get_routing_algorithm_id() const noexcept
    {
        return routing_algorithm_id;
    }

    void set_capture_method(std::optional<CaptureMethodValue> value)
    {
        capture_method = std::move(value);
    }

    [[nodiscard]] const std::optional<CaptureMethodValue>& get_capture_method() const noexcept
    {
        return capture_method;
    }

    void set_authentication_type(std::optional<AuthenticationTypeValue> value)
    {
        authentication_type = std::move(value);
    }

    [[nodiscard]] const std::optional<AuthenticationTypeValue>&
    get_authentication_type() const noexcept
    {
        return authentication_type;
    }

    void set_billing(std::optional<Address> value)
    {
        billing = std::move(value);
    }

    [[nodiscard]] const std::optional<Address>& get_billing() const noexcept
    {
        return billing;
    }

    void set_shipping(std::optional<Address> value)
    {
        shipping = std::move(value);
    }

    [[nodiscard]] const std::optional<Address>& get_shipping() const noexcept
    {
        return shipping;
    }

    void set_customer_present(std::optional<PresenceOfCustomerDuringPaymentValue> value)
    {
        customer_present = std::move(value);
    }

    [[nodiscard]] const std::optional<PresenceOfCustomerDuringPaymentValue>&
    get_customer_present() const noexcept
    {
        return customer_present;
    }

    void set_description(std::optional<DescriptionValue> value)
    {
        description = std::move(value);
    }

    [[nodiscard]] const std::optional<DescriptionValue>& get_description() const noexcept
    {
        return description;
    }

    void set_return_url(std::optional<UrlValue> value)
    {
        return_url = std::move(value);
    }

    [[nodiscard]] const std::optional<UrlValue>& get_return_url() const noexcept
    {
        return return_url;
    }

    void set_setup_future_usage(std::optional<FutureUsageValue> value)
    {
        setup_future_usage = std::move(value);
    }

    [[nodiscard]] const std::optional<FutureUsageValue>& get_setup_future_usage() const noexcept
    {
        return setup_future_usage;
    }

    void set_apply_mit_exemption(std::optional<MitExemptionRequestValue> value)
    {
        apply_mit_exemption = std::move(value);
    }

    [[nodiscard]] const std::optional<MitExemptionRequestValue>&
    get_apply_mit_exemption() const noexcept
    {
        return apply_mit_exemption;
    }

    void set_statement_descriptor(std::optional<StatementDescriptorValue> value)
    {
        statement_descriptor = std::move(value);
    }

    [[nodiscard]] const std::optional<StatementDescriptorValue>&
    get_statement_descriptor() const noexcept
    {
        return statement_descriptor;
    }

    std::optional<AmountDetailsUpdate> amount_details{};
    std::optional<RoutingIdValue> routing_algorithm_id{};
    std::optional<CaptureMethodValue> capture_method{};
    std::optional<AuthenticationTypeValue> authentication_type{};
    std::optional<Address> billing{};
    std::optional<Address> shipping{};
    std::optional<PresenceOfCustomerDuringPaymentValue> customer_present{};
    std::optional<DescriptionValue> description{};
    std::optional<UrlValue> return_url{};
    std::optional<FutureUsageValue> setup_future_usage{};
    std::optional<MitExemptionRequestValue> apply_mit_exemption{};
    std::optional<StatementDescriptorValue> statement_descriptor{};
};

class PlatformPaymentListConstraints :
    public core::behavior::Serializer<PlatformPaymentListConstraints>
{
public:
    void set_payment_id(std::optional<PaymentIdValue> value)
    {
        payment_id = std::move(value);
    }

    [[nodiscard]] const std::optional<PaymentIdValue>& get_payment_id() const noexcept
    {
        return payment_id;
    }

    void set_profile_id(std::optional<ProfileIdValue> value)
    {
        profile_id = std::move(value);
    }

    [[nodiscard]] const std::optional<ProfileIdValue>& get_profile_id() const noexcept
    {
        return profile_id;
    }

    void set_processor_merchant_id(std::optional<MerchantIdValue> value)
    {
        processor_merchant_id = std::move(value);
    }

    [[nodiscard]] const std::optional<MerchantIdValue>& get_processor_merchant_id() const noexcept
    {
        return processor_merchant_id;
    }

    void set_customer_id(std::optional<CustomerIdValue> value)
    {
        customer_id = std::move(value);
    }

    [[nodiscard]] const std::optional<CustomerIdValue>& get_customer_id() const noexcept
    {
        return customer_id;
    }

    void set_limit(PageSizeValue value)
    {
        limit = std::move(value);
    }

    [[nodiscard]] const PageSizeValue& get_limit() const noexcept
    {
        return limit;
    }

    void set_offset(PageOffsetValue value)
    {
        offset = std::move(value);
    }

    [[nodiscard]] const PageOffsetValue& get_offset() const noexcept
    {
        return offset;
    }

    void set_time_range(std::optional<TimeRangeValue> value)
    {
        time_range = std::move(value);
    }

    [[nodiscard]] const std::optional<TimeRangeValue>& get_time_range() const noexcept
    {
        return time_range;
    }

    void set_start_amount(std::optional<std::int64_t> value)
    {
        start_amount = std::move(value);
    }

    [[nodiscard]] const std::optional<std::int64_t>& get_start_amount() const noexcept
    {
        return start_amount;
    }

    void set_end_amount(std::optional<std::int64_t> value)
    {
        end_amount = std::move(value);
    }

    [[nodiscard]] const std::optional<std::int64_t>& get_end_amount() const noexcept
    {
        return end_amount;
    }

    void set_connector(std::optional<std::vector<ConnectorValue>> value)
    {
        connector = std::move(value);
    }

    [[nodiscard]] const std::optional<std::vector<ConnectorValue>>& get_connector() const noexcept
    {
        return connector;
    }

    void set_currency(std::optional<std::vector<CurrencyValue>> value)
    {
        currency = std::move(value);
    }

    [[nodiscard]] const std::optional<std::vector<CurrencyValue>>& get_currency() const noexcept
    {
        return currency;
    }

    void set_status(std::optional<std::vector<IntentStatusValue>> value)
    {
        status = std::move(value);
    }

    [[nodiscard]] const std::optional<std::vector<IntentStatusValue>>& get_status() const noexcept
    {
        return status;
    }

    void set_payment_method(std::optional<std::vector<PaymentMethodValue>> value)
    {
        payment_method = std::move(value);
    }

    [[nodiscard]] const std::optional<std::vector<PaymentMethodValue>>&
    get_payment_method() const noexcept
    {
        return payment_method;
    }

    void set_payment_method_type(std::optional<std::vector<PaymentMethodTypeValue>> value)
    {
        payment_method_type = std::move(value);
    }

    [[nodiscard]] const std::optional<std::vector<PaymentMethodTypeValue>>&
    get_payment_method_type() const noexcept
    {
        return payment_method_type;
    }

    void set_authentication_type(std::optional<std::vector<AuthenticationTypeValue>> value)
    {
        authentication_type = std::move(value);
    }

    [[nodiscard]] const std::optional<std::vector<AuthenticationTypeValue>>&
    get_authentication_type() const noexcept
    {
        return authentication_type;
    }

    void set_merchant_connector_id(
        std::optional<std::vector<MerchantConnectorAccountIdValue>> value
    )
    {
        merchant_connector_id = std::move(value);
    }

    [[nodiscard]] const std::optional<std::vector<MerchantConnectorAccountIdValue>>&
    get_merchant_connector_id() const noexcept
    {
        return merchant_connector_id;
    }

    void set_card_network(std::optional<std::vector<CardNetworkValue>> value)
    {
        card_network = std::move(value);
    }

    [[nodiscard]] const std::optional<std::vector<CardNetworkValue>>&
    get_card_network() const noexcept
    {
        return card_network;
    }

    void set_card_discovery(std::optional<std::vector<CardDiscoveryValue>> value)
    {
        card_discovery = std::move(value);
    }

    [[nodiscard]] const std::optional<std::vector<CardDiscoveryValue>>&
    get_card_discovery() const noexcept
    {
        return card_discovery;
    }

    void set_merchant_order_reference_id(std::optional<std::string> value)
    {
        merchant_order_reference_id = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_merchant_order_reference_id() const noexcept
    {
        return merchant_order_reference_id;
    }

    void set_order(std::optional<Order> value)
    {
        order = std::move(value);
    }

    [[nodiscard]] const std::optional<Order>& get_order() const noexcept
    {
        return order;
    }

    std::optional<PaymentIdValue> payment_id{};
    std::optional<ProfileIdValue> profile_id{};
    std::optional<MerchantIdValue> processor_merchant_id{};
    std::optional<CustomerIdValue> customer_id{};
    PageSizeValue limit{};
    PageOffsetValue offset{};
    std::optional<TimeRangeValue> time_range{};
    std::optional<std::int64_t> start_amount{};
    std::optional<std::int64_t> end_amount{};
    std::optional<std::vector<ConnectorValue>> connector{};
    std::optional<std::vector<CurrencyValue>> currency{};
    std::optional<std::vector<IntentStatusValue>> status{};
    std::optional<std::vector<PaymentMethodValue>> payment_method{};
    std::optional<std::vector<PaymentMethodTypeValue>> payment_method_type{};
    std::optional<std::vector<AuthenticationTypeValue>> authentication_type{};
    std::optional<std::vector<MerchantConnectorAccountIdValue>> merchant_connector_id{};
    std::optional<std::vector<CardNetworkValue>> card_network{};
    std::optional<std::vector<CardDiscoveryValue>> card_discovery{};
    std::optional<std::string> merchant_order_reference_id{};
    std::optional<Order> order{};
};

class PlatformPaymentListResponse : public core::behavior::Serializer<PlatformPaymentListResponse>
{
public:
    void set_count(std::size_t value)
    {
        count = std::move(value);
    }

    [[nodiscard]] const std::size_t& get_count() const noexcept
    {
        return count;
    }

    void set_total_count(std::int64_t value)
    {
        total_count = std::move(value);
    }

    [[nodiscard]] const std::int64_t& get_total_count() const noexcept
    {
        return total_count;
    }

    void set_data(std::vector<PlatformPaymentListItem> value)
    {
        data = std::move(value);
    }

    [[nodiscard]] const std::vector<PlatformPaymentListItem>& get_data() const noexcept
    {
        return data;
    }

    std::size_t count{};
    std::int64_t total_count{};
    std::vector<PlatformPaymentListItem> data{};
};

class ProxyPaymentsRequest : public core::behavior::Serializer<ProxyPaymentsRequest>
{
public:
    void set_return_url(std::optional<UrlValue> value)
    {
        return_url = std::move(value);
    }

    [[nodiscard]] const std::optional<UrlValue>& get_return_url() const noexcept
    {
        return return_url;
    }

    void set_amount(AmountDetails value)
    {
        amount = std::move(value);
    }

    [[nodiscard]] const AmountDetails& get_amount() const noexcept
    {
        return amount;
    }

    void set_recurring_details(ProcessorPaymentTokenValue value)
    {
        recurring_details = std::move(value);
    }

    [[nodiscard]] const ProcessorPaymentTokenValue& get_recurring_details() const noexcept
    {
        return recurring_details;
    }

    void set_shipping(std::optional<Address> value)
    {
        shipping = std::move(value);
    }

    [[nodiscard]] const std::optional<Address>& get_shipping() const noexcept
    {
        return shipping;
    }

    void set_browser_info(std::optional<BrowserInformation> value)
    {
        browser_info = std::move(value);
    }

    [[nodiscard]] const std::optional<BrowserInformation>& get_browser_info() const noexcept
    {
        return browser_info;
    }

    void set_connector(std::string value)
    {
        connector = std::move(value);
    }

    [[nodiscard]] const std::string& get_connector() const noexcept
    {
        return connector;
    }

    void set_merchant_connector_id(MerchantConnectorAccountIdValue value)
    {
        merchant_connector_id = std::move(value);
    }

    [[nodiscard]] const MerchantConnectorAccountIdValue& get_merchant_connector_id() const noexcept
    {
        return merchant_connector_id;
    }

    std::optional<UrlValue> return_url{};
    AmountDetails amount{};
    ProcessorPaymentTokenValue recurring_details{};
    std::optional<Address> shipping{};
    std::optional<BrowserInformation> browser_info{};
    std::string connector{};
    MerchantConnectorAccountIdValue merchant_connector_id{};
};

class RecoveryPaymentListResponse : public core::behavior::Serializer<RecoveryPaymentListResponse>
{
public:
    void set_count(std::size_t value)
    {
        count = std::move(value);
    }

    [[nodiscard]] const std::size_t& get_count() const noexcept
    {
        return count;
    }

    void set_total_count(std::int64_t value)
    {
        total_count = std::move(value);
    }

    [[nodiscard]] const std::int64_t& get_total_count() const noexcept
    {
        return total_count;
    }

    void set_data(std::vector<RecoveryPaymentsListResponseItem> value)
    {
        data = std::move(value);
    }

    [[nodiscard]] const std::vector<RecoveryPaymentsListResponseItem>& get_data() const noexcept
    {
        return data;
    }

    std::size_t count{};
    std::int64_t total_count{};
    std::vector<RecoveryPaymentsListResponseItem> data{};
};

class RecoveryPaymentsCreate : public core::behavior::Serializer<RecoveryPaymentsCreate>
{
public:
    void set_amount_details(AmountDetails value)
    {
        amount_details = std::move(value);
    }

    [[nodiscard]] const AmountDetails& get_amount_details() const noexcept
    {
        return amount_details;
    }

    void set_merchant_reference_id(PaymentReferenceIdValue value)
    {
        merchant_reference_id = std::move(value);
    }

    [[nodiscard]] const PaymentReferenceIdValue& get_merchant_reference_id() const noexcept
    {
        return merchant_reference_id;
    }

    void set_error(std::optional<ErrorDetails> value)
    {
        error = std::move(value);
    }

    [[nodiscard]] const std::optional<ErrorDetails>& get_error() const noexcept
    {
        return error;
    }

    void set_billing_merchant_connector_id(std::optional<MerchantConnectorAccountIdValue> value)
    {
        billing_merchant_connector_id = std::move(value);
    }

    [[nodiscard]] const std::optional<MerchantConnectorAccountIdValue>&
    get_billing_merchant_connector_id() const noexcept
    {
        return billing_merchant_connector_id;
    }

    void set_payment_merchant_connector_id(std::optional<MerchantConnectorAccountIdValue> value)
    {
        payment_merchant_connector_id = std::move(value);
    }

    [[nodiscard]] const std::optional<MerchantConnectorAccountIdValue>&
    get_payment_merchant_connector_id() const noexcept
    {
        return payment_merchant_connector_id;
    }

    void set_merchant_reference_billing_connector_id(std::optional<std::string> value)
    {
        merchant_reference_billing_connector_id = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>&
    get_merchant_reference_billing_connector_id() const noexcept
    {
        return merchant_reference_billing_connector_id;
    }

    void set_merchant_reference_payment_connector_id(std::optional<std::string> value)
    {
        merchant_reference_payment_connector_id = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>&
    get_merchant_reference_payment_connector_id() const noexcept
    {
        return merchant_reference_payment_connector_id;
    }

    void set_connector(std::optional<RecoveryConnectorValue> value)
    {
        connector = std::move(value);
    }

    [[nodiscard]] const std::optional<RecoveryConnectorValue>& get_connector() const noexcept
    {
        return connector;
    }

    void set_transaction_status(RecoveryTransactionStatusValue value)
    {
        transaction_status = std::move(value);
    }

    [[nodiscard]] const RecoveryTransactionStatusValue& get_transaction_status() const noexcept
    {
        return transaction_status;
    }

    void set_amount_captured(std::optional<MinorUnitValue> value)
    {
        amount_captured = std::move(value);
    }

    [[nodiscard]] const std::optional<MinorUnitValue>& get_amount_captured() const noexcept
    {
        return amount_captured;
    }

    void set_mit_category(std::optional<MitCategoryValue> value)
    {
        mit_category = std::move(value);
    }

    [[nodiscard]] const std::optional<MitCategoryValue>& get_mit_category() const noexcept
    {
        return mit_category;
    }

    void set_customer_present(std::optional<PresenceOfCustomerDuringPaymentValue> value)
    {
        customer_present = std::move(value);
    }

    [[nodiscard]] const std::optional<PresenceOfCustomerDuringPaymentValue>&
    get_customer_present() const noexcept
    {
        return customer_present;
    }

    void set_setup_future_usage(std::optional<FutureUsageValue> value)
    {
        setup_future_usage = std::move(value);
    }

    [[nodiscard]] const std::optional<FutureUsageValue>& get_setup_future_usage() const noexcept
    {
        return setup_future_usage;
    }

    void set_billing(std::optional<Address> value)
    {
        billing = std::move(value);
    }

    [[nodiscard]] const std::optional<Address>& get_billing() const noexcept
    {
        return billing;
    }

    void set_customer_details(std::optional<CustomerDetails> value)
    {
        customer_details = std::move(value);
    }

    [[nodiscard]] const std::optional<CustomerDetails>& get_customer_details() const noexcept
    {
        return customer_details;
    }

    void set_payment_method_type(PaymentMethodValue value)
    {
        payment_method_type = std::move(value);
    }

    [[nodiscard]] const PaymentMethodValue& get_payment_method_type() const noexcept
    {
        return payment_method_type;
    }

    void set_payment_method_sub_type(PaymentMethodTypeValue value)
    {
        payment_method_sub_type = std::move(value);
    }

    [[nodiscard]] const PaymentMethodTypeValue& get_payment_method_sub_type() const noexcept
    {
        return payment_method_sub_type;
    }

    void set_transaction_created_at(PrimitiveDateTimeValue value)
    {
        transaction_created_at = std::move(value);
    }

    [[nodiscard]] const PrimitiveDateTimeValue& get_transaction_created_at() const noexcept
    {
        return transaction_created_at;
    }

    void set_connector_customer_id(SecretValue value)
    {
        connector_customer_id = std::move(value);
    }

    [[nodiscard]] const SecretValue& get_connector_customer_id() const noexcept
    {
        return connector_customer_id;
    }

    void set_billing_started_at(PrimitiveDateTimeValue value)
    {
        billing_started_at = std::move(value);
    }

    [[nodiscard]] const PrimitiveDateTimeValue& get_billing_started_at() const noexcept
    {
        return billing_started_at;
    }

    void set_connector_transaction_id(std::string value)
    {
        connector_transaction_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_connector_transaction_id() const noexcept
    {
        return connector_transaction_id;
    }

    void set_payment_method_data(std::optional<CustomRecoveryPaymentMethodData> value)
    {
        payment_method_data = std::move(value);
    }

    [[nodiscard]] const std::optional<CustomRecoveryPaymentMethodData>&
    get_payment_method_data() const noexcept
    {
        return payment_method_data;
    }

    void set_enable_partial_authorization(std::optional<EnablePartialAuthorizationBoolValue> value)
    {
        enable_partial_authorization = std::move(value);
    }

    [[nodiscard]] const std::optional<EnablePartialAuthorizationBoolValue>&
    get_enable_partial_authorization() const noexcept
    {
        return enable_partial_authorization;
    }

    void set_metadata(std::optional<SecretSerdeValueValue> value)
    {
        metadata = std::move(value);
    }

    [[nodiscard]] const std::optional<SecretSerdeValueValue>& get_metadata() const noexcept
    {
        return metadata;
    }

    AmountDetails amount_details{};
    PaymentReferenceIdValue merchant_reference_id{};
    std::optional<ErrorDetails> error{};
    std::optional<MerchantConnectorAccountIdValue> billing_merchant_connector_id{};
    std::optional<MerchantConnectorAccountIdValue> payment_merchant_connector_id{};
    std::optional<std::string> merchant_reference_billing_connector_id{};
    std::optional<std::string> merchant_reference_payment_connector_id{};
    std::optional<RecoveryConnectorValue> connector{};
    RecoveryTransactionStatusValue transaction_status{};
    std::optional<MinorUnitValue> amount_captured{};
    std::optional<MitCategoryValue> mit_category{};
    std::optional<PresenceOfCustomerDuringPaymentValue> customer_present{};
    std::optional<FutureUsageValue> setup_future_usage{};
    std::optional<Address> billing{};
    std::optional<CustomerDetails> customer_details{};
    PaymentMethodValue payment_method_type{};
    PaymentMethodTypeValue payment_method_sub_type{};
    PrimitiveDateTimeValue transaction_created_at{};
    SecretValue connector_customer_id{};
    PrimitiveDateTimeValue billing_started_at{};
    std::string connector_transaction_id{};
    std::optional<CustomRecoveryPaymentMethodData> payment_method_data{};
    std::optional<EnablePartialAuthorizationBoolValue> enable_partial_authorization{};
    std::optional<SecretSerdeValueValue> metadata{};
};

class RevenueRecoveryGetIntentResponse :
    public core::behavior::Serializer<RevenueRecoveryGetIntentResponse>
{
public:
    void set_id(GlobalPaymentIdValue value)
    {
        id = std::move(value);
    }

    [[nodiscard]] const GlobalPaymentIdValue& get_id() const noexcept
    {
        return id;
    }

    void set_status(RecoveryStatusValue value)
    {
        status = std::move(value);
    }

    [[nodiscard]] const RecoveryStatusValue& get_status() const noexcept
    {
        return status;
    }

    void set_next_retry(std::optional<RecoveryNextRetryValue> value)
    {
        next_retry = std::move(value);
    }

    [[nodiscard]] const std::optional<RecoveryNextRetryValue>& get_next_retry() const noexcept
    {
        return next_retry;
    }

    void set_amount_details(AmountDetailsResponse value)
    {
        amount_details = std::move(value);
    }

    [[nodiscard]] const AmountDetailsResponse& get_amount_details() const noexcept
    {
        return amount_details;
    }

    void set_client_secret(std::optional<SecretValue> value)
    {
        client_secret = std::move(value);
    }

    [[nodiscard]] const std::optional<SecretValue>& get_client_secret() const noexcept
    {
        return client_secret;
    }

    void set_profile_id(ProfileIdValue value)
    {
        profile_id = std::move(value);
    }

    [[nodiscard]] const ProfileIdValue& get_profile_id() const noexcept
    {
        return profile_id;
    }

    void set_merchant_reference_id(std::optional<PaymentReferenceIdValue> value)
    {
        merchant_reference_id = std::move(value);
    }

    [[nodiscard]] const std::optional<PaymentReferenceIdValue>&
    get_merchant_reference_id() const noexcept
    {
        return merchant_reference_id;
    }

    void set_routing_algorithm_id(std::optional<RoutingIdValue> value)
    {
        routing_algorithm_id = std::move(value);
    }

    [[nodiscard]] const std::optional<RoutingIdValue>& get_routing_algorithm_id() const noexcept
    {
        return routing_algorithm_id;
    }

    void set_capture_method(CaptureMethodValue value)
    {
        capture_method = std::move(value);
    }

    [[nodiscard]] const CaptureMethodValue& get_capture_method() const noexcept
    {
        return capture_method;
    }

    void set_authentication_type(std::optional<AuthenticationTypeValue> value)
    {
        authentication_type = std::move(value);
    }

    [[nodiscard]] const std::optional<AuthenticationTypeValue>&
    get_authentication_type() const noexcept
    {
        return authentication_type;
    }

    void set_billing(std::optional<Address> value)
    {
        billing = std::move(value);
    }

    [[nodiscard]] const std::optional<Address>& get_billing() const noexcept
    {
        return billing;
    }

    void set_shipping(std::optional<Address> value)
    {
        shipping = std::move(value);
    }

    [[nodiscard]] const std::optional<Address>& get_shipping() const noexcept
    {
        return shipping;
    }

    void set_customer_id(std::optional<GlobalCustomerIdValue> value)
    {
        customer_id = std::move(value);
    }

    [[nodiscard]] const std::optional<GlobalCustomerIdValue>& get_customer_id() const noexcept
    {
        return customer_id;
    }

    void set_customer_present(PresenceOfCustomerDuringPaymentValue value)
    {
        customer_present = std::move(value);
    }

    [[nodiscard]] const PresenceOfCustomerDuringPaymentValue& get_customer_present() const noexcept
    {
        return customer_present;
    }

    void set_description(std::optional<DescriptionValue> value)
    {
        description = std::move(value);
    }

    [[nodiscard]] const std::optional<DescriptionValue>& get_description() const noexcept
    {
        return description;
    }

    void set_return_url(std::optional<UrlValue> value)
    {
        return_url = std::move(value);
    }

    [[nodiscard]] const std::optional<UrlValue>& get_return_url() const noexcept
    {
        return return_url;
    }

    void set_setup_future_usage(FutureUsageValue value)
    {
        setup_future_usage = std::move(value);
    }

    [[nodiscard]] const FutureUsageValue& get_setup_future_usage() const noexcept
    {
        return setup_future_usage;
    }

    void set_apply_mit_exemption(MitExemptionRequestValue value)
    {
        apply_mit_exemption = std::move(value);
    }

    [[nodiscard]] const MitExemptionRequestValue& get_apply_mit_exemption() const noexcept
    {
        return apply_mit_exemption;
    }

    void set_statement_descriptor(std::optional<StatementDescriptorValue> value)
    {
        statement_descriptor = std::move(value);
    }

    [[nodiscard]] const std::optional<StatementDescriptorValue>&
    get_statement_descriptor() const noexcept
    {
        return statement_descriptor;
    }

    GlobalPaymentIdValue id{};
    RecoveryStatusValue status{};
    std::optional<RecoveryNextRetryValue> next_retry{};
    AmountDetailsResponse amount_details{};
    std::optional<SecretValue> client_secret{};
    ProfileIdValue profile_id{};
    std::optional<PaymentReferenceIdValue> merchant_reference_id{};
    std::optional<RoutingIdValue> routing_algorithm_id{};
    CaptureMethodValue capture_method{};
    std::optional<AuthenticationTypeValue> authentication_type{};
    std::optional<Address> billing{};
    std::optional<Address> shipping{};
    std::optional<GlobalCustomerIdValue> customer_id{};
    PresenceOfCustomerDuringPaymentValue customer_present{};
    std::optional<DescriptionValue> description{};
    std::optional<UrlValue> return_url{};
    FutureUsageValue setup_future_usage{};
    MitExemptionRequestValue apply_mit_exemption{};
    std::optional<StatementDescriptorValue> statement_descriptor{};
};

class SecurePaymentLinkDetails : public core::behavior::Serializer<SecurePaymentLinkDetails>
{
public:
    void set_enabled_saved_payment_method(bool value)
    {
        enabled_saved_payment_method = std::move(value);
    }

    [[nodiscard]] const bool& get_enabled_saved_payment_method() const noexcept
    {
        return enabled_saved_payment_method;
    }

    void set_hide_card_nickname_field(bool value)
    {
        hide_card_nickname_field = std::move(value);
    }

    [[nodiscard]] const bool& get_hide_card_nickname_field() const noexcept
    {
        return hide_card_nickname_field;
    }

    void set_show_card_form_by_default(bool value)
    {
        show_card_form_by_default = std::move(value);
    }

    [[nodiscard]] const bool& get_show_card_form_by_default() const noexcept
    {
        return show_card_form_by_default;
    }

    void set_payment_link_details(PaymentLinkDetails value)
    {
        payment_link_details = std::move(value);
    }

    [[nodiscard]] const PaymentLinkDetails& get_payment_link_details() const noexcept
    {
        return payment_link_details;
    }

    void set_payment_button_text(std::optional<std::string> value)
    {
        payment_button_text = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_payment_button_text() const noexcept
    {
        return payment_button_text;
    }

    void set_skip_status_screen(std::optional<bool> value)
    {
        skip_status_screen = std::move(value);
    }

    [[nodiscard]] const std::optional<bool>& get_skip_status_screen() const noexcept
    {
        return skip_status_screen;
    }

    void set_custom_message_for_card_terms(std::optional<std::string> value)
    {
        custom_message_for_card_terms = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>&
    get_custom_message_for_card_terms() const noexcept
    {
        return custom_message_for_card_terms;
    }

    void set_custom_message_for_payment_method_types(std::optional<PaymentMethodsConfigValue> value)
    {
        custom_message_for_payment_method_types = std::move(value);
    }

    [[nodiscard]] const std::optional<PaymentMethodsConfigValue>&
    get_custom_message_for_payment_method_types() const noexcept
    {
        return custom_message_for_payment_method_types;
    }

    void set_payment_button_colour(std::optional<std::string> value)
    {
        payment_button_colour = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_payment_button_colour() const noexcept
    {
        return payment_button_colour;
    }

    void set_payment_button_text_colour(std::optional<std::string> value)
    {
        payment_button_text_colour = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_payment_button_text_colour() const noexcept
    {
        return payment_button_text_colour;
    }

    void set_background_colour(std::optional<std::string> value)
    {
        background_colour = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_background_colour() const noexcept
    {
        return background_colour;
    }

    void set_sdk_ui_rules(
        std::optional<std::unordered_map<std::string, std::unordered_map<std::string, std::string>>>
            value
    )
    {
        sdk_ui_rules = std::move(value);
    }

    [[nodiscard]] const std::optional<
        std::unordered_map<std::string, std::unordered_map<std::string, std::string>>>&
    get_sdk_ui_rules() const noexcept
    {
        return sdk_ui_rules;
    }

    void set_enable_button_only_on_form_ready(bool value)
    {
        enable_button_only_on_form_ready = std::move(value);
    }

    [[nodiscard]] const bool& get_enable_button_only_on_form_ready() const noexcept
    {
        return enable_button_only_on_form_ready;
    }

    void set_payment_form_header_text(std::optional<std::string> value)
    {
        payment_form_header_text = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_payment_form_header_text() const noexcept
    {
        return payment_form_header_text;
    }

    void set_payment_form_label_type(std::optional<PaymentLinkSdkLabelTypeValue> value)
    {
        payment_form_label_type = std::move(value);
    }

    [[nodiscard]] const std::optional<PaymentLinkSdkLabelTypeValue>&
    get_payment_form_label_type() const noexcept
    {
        return payment_form_label_type;
    }

    void set_show_card_terms(std::optional<PaymentLinkShowSdkTermsValue> value)
    {
        show_card_terms = std::move(value);
    }

    [[nodiscard]] const std::optional<PaymentLinkShowSdkTermsValue>&
    get_show_card_terms() const noexcept
    {
        return show_card_terms;
    }

    void set_color_icon_card_cvc_error(std::optional<std::string> value)
    {
        color_icon_card_cvc_error = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_color_icon_card_cvc_error() const noexcept
    {
        return color_icon_card_cvc_error;
    }

    void set_payment_methods_separator_text(std::optional<std::string> value)
    {
        payment_methods_separator_text = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>&
    get_payment_methods_separator_text() const noexcept
    {
        return payment_methods_separator_text;
    }

    bool enabled_saved_payment_method{};
    bool hide_card_nickname_field{};
    bool show_card_form_by_default{};
    PaymentLinkDetails payment_link_details{};
    std::optional<std::string> payment_button_text{};
    std::optional<bool> skip_status_screen{};
    std::optional<std::string> custom_message_for_card_terms{};
    std::optional<PaymentMethodsConfigValue> custom_message_for_payment_method_types{};
    std::optional<std::string> payment_button_colour{};
    std::optional<std::string> payment_button_text_colour{};
    std::optional<std::string> background_colour{};
    std::optional<std::unordered_map<std::string, std::unordered_map<std::string, std::string>>>
        sdk_ui_rules{};
    bool enable_button_only_on_form_ready{};
    std::optional<std::string> payment_form_header_text{};
    std::optional<PaymentLinkSdkLabelTypeValue> payment_form_label_type{};
    std::optional<PaymentLinkShowSdkTermsValue> show_card_terms{};
    std::optional<std::string> color_icon_card_cvc_error{};
    std::optional<std::string> payment_methods_separator_text{};
};

class VerifyRequest : public core::behavior::Serializer<VerifyRequest>
{
public:
    void set_merchant_id(std::optional<std::string> value)
    {
        merchant_id = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_merchant_id() const noexcept
    {
        return merchant_id;
    }

    void set_customer_id(std::optional<CustomerIdValue> value)
    {
        customer_id = std::move(value);
    }

    [[nodiscard]] const std::optional<CustomerIdValue>& get_customer_id() const noexcept
    {
        return customer_id;
    }

    void set_email(std::optional<EmailValue> value)
    {
        email = std::move(value);
    }

    [[nodiscard]] const std::optional<EmailValue>& get_email() const noexcept
    {
        return email;
    }

    void set_name(std::optional<SecretValue> value)
    {
        name = std::move(value);
    }

    [[nodiscard]] const std::optional<SecretValue>& get_name() const noexcept
    {
        return name;
    }

    void set_phone(std::optional<SecretValue> value)
    {
        phone = std::move(value);
    }

    [[nodiscard]] const std::optional<SecretValue>& get_phone() const noexcept
    {
        return phone;
    }

    void set_phone_country_code(std::optional<std::string> value)
    {
        phone_country_code = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_phone_country_code() const noexcept
    {
        return phone_country_code;
    }

    void set_payment_method(std::optional<PaymentMethodValue> value)
    {
        payment_method = std::move(value);
    }

    [[nodiscard]] const std::optional<PaymentMethodValue>& get_payment_method() const noexcept
    {
        return payment_method;
    }

    void set_payment_method_data(std::optional<PaymentMethodData> value)
    {
        payment_method_data = std::move(value);
    }

    [[nodiscard]] const std::optional<PaymentMethodData>& get_payment_method_data() const noexcept
    {
        return payment_method_data;
    }

    void set_payment_token(std::optional<std::string> value)
    {
        payment_token = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_payment_token() const noexcept
    {
        return payment_token;
    }

    void set_mandate_data(std::optional<MandateData> value)
    {
        mandate_data = std::move(value);
    }

    [[nodiscard]] const std::optional<MandateData>& get_mandate_data() const noexcept
    {
        return mandate_data;
    }

    void set_setup_future_usage(std::optional<FutureUsageValue> value)
    {
        setup_future_usage = std::move(value);
    }

    [[nodiscard]] const std::optional<FutureUsageValue>& get_setup_future_usage() const noexcept
    {
        return setup_future_usage;
    }

    void set_off_session(std::optional<bool> value)
    {
        off_session = std::move(value);
    }

    [[nodiscard]] const std::optional<bool>& get_off_session() const noexcept
    {
        return off_session;
    }

    void set_client_secret(std::optional<std::string> value)
    {
        client_secret = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_client_secret() const noexcept
    {
        return client_secret;
    }

    void set_merchant_connector_details(std::optional<MerchantConnectorDetailsWrapValue> value)
    {
        merchant_connector_details = std::move(value);
    }

    [[nodiscard]] const std::optional<MerchantConnectorDetailsWrapValue>&
    get_merchant_connector_details() const noexcept
    {
        return merchant_connector_details;
    }

    std::optional<std::string> merchant_id{};
    std::optional<CustomerIdValue> customer_id{};
    std::optional<EmailValue> email{};
    std::optional<SecretValue> name{};
    std::optional<SecretValue> phone{};
    std::optional<std::string> phone_country_code{};
    std::optional<PaymentMethodValue> payment_method{};
    std::optional<PaymentMethodData> payment_method_data{};
    std::optional<std::string> payment_token{};
    std::optional<MandateData> mandate_data{};
    std::optional<FutureUsageValue> setup_future_usage{};
    std::optional<bool> off_session{};
    std::optional<std::string> client_secret{};
    std::optional<MerchantConnectorDetailsWrapValue> merchant_connector_details{};
};

class VerifyResponse : public core::behavior::Serializer<VerifyResponse>
{
public:
    void set_verify_id(std::optional<PaymentIdValue> value)
    {
        verify_id = std::move(value);
    }

    [[nodiscard]] const std::optional<PaymentIdValue>& get_verify_id() const noexcept
    {
        return verify_id;
    }

    void set_merchant_id(std::optional<MerchantIdValue> value)
    {
        merchant_id = std::move(value);
    }

    [[nodiscard]] const std::optional<MerchantIdValue>& get_merchant_id() const noexcept
    {
        return merchant_id;
    }

    void set_client_secret(std::optional<SecretValue> value)
    {
        client_secret = std::move(value);
    }

    [[nodiscard]] const std::optional<SecretValue>& get_client_secret() const noexcept
    {
        return client_secret;
    }

    void set_customer_id(std::optional<CustomerIdValue> value)
    {
        customer_id = std::move(value);
    }

    [[nodiscard]] const std::optional<CustomerIdValue>& get_customer_id() const noexcept
    {
        return customer_id;
    }

    void set_email(std::optional<EmailValue> value)
    {
        email = std::move(value);
    }

    [[nodiscard]] const std::optional<EmailValue>& get_email() const noexcept
    {
        return email;
    }

    void set_name(std::optional<SecretValue> value)
    {
        name = std::move(value);
    }

    [[nodiscard]] const std::optional<SecretValue>& get_name() const noexcept
    {
        return name;
    }

    void set_phone(std::optional<SecretValue> value)
    {
        phone = std::move(value);
    }

    [[nodiscard]] const std::optional<SecretValue>& get_phone() const noexcept
    {
        return phone;
    }

    void set_mandate_id(std::optional<std::string> value)
    {
        mandate_id = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_mandate_id() const noexcept
    {
        return mandate_id;
    }

    void set_payment_method(std::optional<PaymentMethodValue> value)
    {
        payment_method = std::move(value);
    }

    [[nodiscard]] const std::optional<PaymentMethodValue>& get_payment_method() const noexcept
    {
        return payment_method;
    }

    void set_payment_method_data(std::optional<PaymentMethodDataResponse> value)
    {
        payment_method_data = std::move(value);
    }

    [[nodiscard]] const std::optional<PaymentMethodDataResponse>&
    get_payment_method_data() const noexcept
    {
        return payment_method_data;
    }

    void set_payment_token(std::optional<std::string> value)
    {
        payment_token = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_payment_token() const noexcept
    {
        return payment_token;
    }

    void set_error_code(std::optional<std::string> value)
    {
        error_code = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_error_code() const noexcept
    {
        return error_code;
    }

    void set_error_message(std::optional<std::string> value)
    {
        error_message = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_error_message() const noexcept
    {
        return error_message;
    }

    std::optional<PaymentIdValue> verify_id{};
    std::optional<MerchantIdValue> merchant_id{};
    std::optional<SecretValue> client_secret{};
    std::optional<CustomerIdValue> customer_id{};
    std::optional<EmailValue> email{};
    std::optional<SecretValue> name{};
    std::optional<SecretValue> phone{};
    std::optional<std::string> mandate_id{};
    std::optional<PaymentMethodValue> payment_method{};
    std::optional<PaymentMethodDataResponse> payment_method_data{};
    std::optional<std::string> payment_token{};
    std::optional<std::string> error_code{};
    std::optional<std::string> error_message{};
};

class ExternalVaultProxyPaymentsRequest :
    public core::behavior::Serializer<ExternalVaultProxyPaymentsRequest>
{
public:
    void set_return_url(std::optional<UrlValue> value)
    {
        return_url = std::move(value);
    }

    [[nodiscard]] const std::optional<UrlValue>& get_return_url() const noexcept
    {
        return return_url;
    }

    void set_payment_method_data(ProxyPaymentMethodDataRequest value)
    {
        payment_method_data = std::move(value);
    }

    [[nodiscard]] const ProxyPaymentMethodDataRequest& get_payment_method_data() const noexcept
    {
        return payment_method_data;
    }

    void set_payment_method_type(PaymentMethodValue value)
    {
        payment_method_type = std::move(value);
    }

    [[nodiscard]] const PaymentMethodValue& get_payment_method_type() const noexcept
    {
        return payment_method_type;
    }

    void set_payment_method_subtype(PaymentMethodTypeValue value)
    {
        payment_method_subtype = std::move(value);
    }

    [[nodiscard]] const PaymentMethodTypeValue& get_payment_method_subtype() const noexcept
    {
        return payment_method_subtype;
    }

    void set_shipping(std::optional<Address> value)
    {
        shipping = std::move(value);
    }

    [[nodiscard]] const std::optional<Address>& get_shipping() const noexcept
    {
        return shipping;
    }

    void set_customer_acceptance(std::optional<CustomerAcceptanceValue> value)
    {
        customer_acceptance = std::move(value);
    }

    [[nodiscard]] const std::optional<CustomerAcceptanceValue>&
    get_customer_acceptance() const noexcept
    {
        return customer_acceptance;
    }

    void set_browser_info(std::optional<BrowserInformation> value)
    {
        browser_info = std::move(value);
    }

    [[nodiscard]] const std::optional<BrowserInformation>& get_browser_info() const noexcept
    {
        return browser_info;
    }

    void set_payment_method_id(std::optional<GlobalPaymentMethodIdValue> value)
    {
        payment_method_id = std::move(value);
    }

    [[nodiscard]] const std::optional<GlobalPaymentMethodIdValue>&
    get_payment_method_id() const noexcept
    {
        return payment_method_id;
    }

    void set_payment_token(std::optional<std::string> value)
    {
        payment_token = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_payment_token() const noexcept
    {
        return payment_token;
    }

    void set_merchant_connector_details(std::optional<MerchantConnectorAuthDetailsValue> value)
    {
        merchant_connector_details = std::move(value);
    }

    [[nodiscard]] const std::optional<MerchantConnectorAuthDetailsValue>&
    get_merchant_connector_details() const noexcept
    {
        return merchant_connector_details;
    }

    void set_return_raw_connector_response(std::optional<bool> value)
    {
        return_raw_connector_response = std::move(value);
    }

    [[nodiscard]] const std::optional<bool>& get_return_raw_connector_response() const noexcept
    {
        return return_raw_connector_response;
    }

    std::optional<UrlValue> return_url{};
    ProxyPaymentMethodDataRequest payment_method_data{};
    PaymentMethodValue payment_method_type{};
    PaymentMethodTypeValue payment_method_subtype{};
    std::optional<Address> shipping{};
    std::optional<CustomerAcceptanceValue> customer_acceptance{};
    std::optional<BrowserInformation> browser_info{};
    std::optional<GlobalPaymentMethodIdValue> payment_method_id{};
    std::optional<std::string> payment_token{};
    std::optional<MerchantConnectorAuthDetailsValue> merchant_connector_details{};
    std::optional<bool> return_raw_connector_response{};
};

class PaymentAttemptResponse : public core::behavior::Serializer<PaymentAttemptResponse>
{
public:
    void set_attempt_id(std::string value)
    {
        attempt_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_attempt_id() const noexcept
    {
        return attempt_id;
    }

    void set_status(AttemptStatusValue value)
    {
        status = std::move(value);
    }

    [[nodiscard]] const AttemptStatusValue& get_status() const noexcept
    {
        return status;
    }

    void set_amount(MinorUnitValue value)
    {
        amount = std::move(value);
    }

    [[nodiscard]] const MinorUnitValue& get_amount() const noexcept
    {
        return amount;
    }

    void set_order_tax_amount(std::optional<MinorUnitValue> value)
    {
        order_tax_amount = std::move(value);
    }

    [[nodiscard]] const std::optional<MinorUnitValue>& get_order_tax_amount() const noexcept
    {
        return order_tax_amount;
    }

    void set_currency(std::optional<CurrencyValue> value)
    {
        currency = std::move(value);
    }

    [[nodiscard]] const std::optional<CurrencyValue>& get_currency() const noexcept
    {
        return currency;
    }

    void set_connector(std::optional<std::string> value)
    {
        connector = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_connector() const noexcept
    {
        return connector;
    }

    void set_error_message(std::optional<std::string> value)
    {
        error_message = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_error_message() const noexcept
    {
        return error_message;
    }

    void set_payment_method(std::optional<PaymentMethodValue> value)
    {
        payment_method = std::move(value);
    }

    [[nodiscard]] const std::optional<PaymentMethodValue>& get_payment_method() const noexcept
    {
        return payment_method;
    }

    void set_connector_transaction_id(std::optional<std::string> value)
    {
        connector_transaction_id = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_connector_transaction_id() const noexcept
    {
        return connector_transaction_id;
    }

    void set_capture_method(std::optional<CaptureMethodValue> value)
    {
        capture_method = std::move(value);
    }

    [[nodiscard]] const std::optional<CaptureMethodValue>& get_capture_method() const noexcept
    {
        return capture_method;
    }

    void set_authentication_type(std::optional<AuthenticationTypeValue> value)
    {
        authentication_type = std::move(value);
    }

    [[nodiscard]] const std::optional<AuthenticationTypeValue>&
    get_authentication_type() const noexcept
    {
        return authentication_type;
    }

    void set_created_at(PrimitiveDateTimeValue value)
    {
        created_at = std::move(value);
    }

    [[nodiscard]] const PrimitiveDateTimeValue& get_created_at() const noexcept
    {
        return created_at;
    }

    void set_modified_at(PrimitiveDateTimeValue value)
    {
        modified_at = std::move(value);
    }

    [[nodiscard]] const PrimitiveDateTimeValue& get_modified_at() const noexcept
    {
        return modified_at;
    }

    void set_cancellation_reason(std::optional<std::string> value)
    {
        cancellation_reason = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_cancellation_reason() const noexcept
    {
        return cancellation_reason;
    }

    void set_mandate_id(std::optional<std::string> value)
    {
        mandate_id = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_mandate_id() const noexcept
    {
        return mandate_id;
    }

    void set_error_code(std::optional<std::string> value)
    {
        error_code = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_error_code() const noexcept
    {
        return error_code;
    }

    void set_payment_token(std::optional<std::string> value)
    {
        payment_token = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_payment_token() const noexcept
    {
        return payment_token;
    }

    void set_connector_metadata(std::optional<serde::Value> value)
    {
        connector_metadata = std::move(value);
    }

    [[nodiscard]] const std::optional<serde::Value>& get_connector_metadata() const noexcept
    {
        return connector_metadata;
    }

    void set_payment_experience(std::optional<PaymentExperienceValue> value)
    {
        payment_experience = std::move(value);
    }

    [[nodiscard]] const std::optional<PaymentExperienceValue>&
    get_payment_experience() const noexcept
    {
        return payment_experience;
    }

    void set_payment_method_type(std::optional<PaymentMethodTypeValue> value)
    {
        payment_method_type = std::move(value);
    }

    [[nodiscard]] const std::optional<PaymentMethodTypeValue>&
    get_payment_method_type() const noexcept
    {
        return payment_method_type;
    }

    void set_reference_id(std::optional<std::string> value)
    {
        reference_id = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_reference_id() const noexcept
    {
        return reference_id;
    }

    void set_unified_code(std::optional<std::string> value)
    {
        unified_code = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_unified_code() const noexcept
    {
        return unified_code;
    }

    void set_unified_message(std::optional<std::string> value)
    {
        unified_message = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_unified_message() const noexcept
    {
        return unified_message;
    }

    void set_client_source(std::optional<std::string> value)
    {
        client_source = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_client_source() const noexcept
    {
        return client_source;
    }

    void set_client_version(std::optional<std::string> value)
    {
        client_version = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_client_version() const noexcept
    {
        return client_version;
    }

    void set_error_details(std::optional<PaymentErrorDetails> value)
    {
        error_details = std::move(value);
    }

    [[nodiscard]] const std::optional<PaymentErrorDetails>& get_error_details() const noexcept
    {
        return error_details;
    }

    std::string attempt_id{};
    AttemptStatusValue status{};
    MinorUnitValue amount{};
    std::optional<MinorUnitValue> order_tax_amount{};
    std::optional<CurrencyValue> currency{};
    std::optional<std::string> connector{};
    std::optional<std::string> error_message{};
    std::optional<PaymentMethodValue> payment_method{};
    std::optional<std::string> connector_transaction_id{};
    std::optional<CaptureMethodValue> capture_method{};
    std::optional<AuthenticationTypeValue> authentication_type{};
    PrimitiveDateTimeValue created_at{};
    PrimitiveDateTimeValue modified_at{};
    std::optional<std::string> cancellation_reason{};
    std::optional<std::string> mandate_id{};
    std::optional<std::string> error_code{};
    std::optional<std::string> payment_token{};
    std::optional<serde::Value> connector_metadata{};
    std::optional<PaymentExperienceValue> payment_experience{};
    std::optional<PaymentMethodTypeValue> payment_method_type{};
    std::optional<std::string> reference_id{};
    std::optional<std::string> unified_code{};
    std::optional<std::string> unified_message{};
    std::optional<std::string> client_source{};
    std::optional<std::string> client_version{};
    std::optional<PaymentErrorDetails> error_details{};
};

class PaymentsAttemptRecordRequest : public core::behavior::Serializer<PaymentsAttemptRecordRequest>
{
public:
    void set_amount_details(PaymentAttemptAmountDetails value)
    {
        amount_details = std::move(value);
    }

    [[nodiscard]] const PaymentAttemptAmountDetails& get_amount_details() const noexcept
    {
        return amount_details;
    }

    void set_status(AttemptStatusValue value)
    {
        status = std::move(value);
    }

    [[nodiscard]] const AttemptStatusValue& get_status() const noexcept
    {
        return status;
    }

    void set_billing(std::optional<Address> value)
    {
        billing = std::move(value);
    }

    [[nodiscard]] const std::optional<Address>& get_billing() const noexcept
    {
        return billing;
    }

    void set_shipping(std::optional<Address> value)
    {
        shipping = std::move(value);
    }

    [[nodiscard]] const std::optional<Address>& get_shipping() const noexcept
    {
        return shipping;
    }

    void set_error(std::optional<RecordAttemptErrorDetails> value)
    {
        error = std::move(value);
    }

    [[nodiscard]] const std::optional<RecordAttemptErrorDetails>& get_error() const noexcept
    {
        return error;
    }

    void set_description(std::optional<DescriptionValue> value)
    {
        description = std::move(value);
    }

    [[nodiscard]] const std::optional<DescriptionValue>& get_description() const noexcept
    {
        return description;
    }

    void set_connector_transaction_id(std::optional<ConnectorTransactionIdValue> value)
    {
        connector_transaction_id = std::move(value);
    }

    [[nodiscard]] const std::optional<ConnectorTransactionIdValue>&
    get_connector_transaction_id() const noexcept
    {
        return connector_transaction_id;
    }

    void set_payment_method_type(PaymentMethodValue value)
    {
        payment_method_type = std::move(value);
    }

    [[nodiscard]] const PaymentMethodValue& get_payment_method_type() const noexcept
    {
        return payment_method_type;
    }

    void set_connector_details(RecoveryConnectorDetailsValue value)
    {
        connector_details = std::move(value);
    }

    [[nodiscard]] const RecoveryConnectorDetailsValue& get_connector_details() const noexcept
    {
        return connector_details;
    }

    void set_payment_method_subtype(PaymentMethodTypeValue value)
    {
        payment_method_subtype = std::move(value);
    }

    [[nodiscard]] const PaymentMethodTypeValue& get_payment_method_subtype() const noexcept
    {
        return payment_method_subtype;
    }

    void set_payment_method_data(std::optional<RecordAttemptPaymentMethodDataRequest> value)
    {
        payment_method_data = std::move(value);
    }

    [[nodiscard]] const std::optional<RecordAttemptPaymentMethodDataRequest>&
    get_payment_method_data() const noexcept
    {
        return payment_method_data;
    }

    void set_metadata(std::optional<SecretSerdeValueValue> value)
    {
        metadata = std::move(value);
    }

    [[nodiscard]] const std::optional<SecretSerdeValueValue>& get_metadata() const noexcept
    {
        return metadata;
    }

    void set_feature_metadata(std::optional<PaymentAttemptFeatureMetadata> value)
    {
        feature_metadata = std::move(value);
    }

    [[nodiscard]] const std::optional<PaymentAttemptFeatureMetadata>&
    get_feature_metadata() const noexcept
    {
        return feature_metadata;
    }

    void set_transaction_created_at(std::optional<PrimitiveDateTimeValue> value)
    {
        transaction_created_at = std::move(value);
    }

    [[nodiscard]] const std::optional<PrimitiveDateTimeValue>&
    get_transaction_created_at() const noexcept
    {
        return transaction_created_at;
    }

    void set_processor_payment_method_token(std::optional<std::string> value)
    {
        processor_payment_method_token = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>&
    get_processor_payment_method_token() const noexcept
    {
        return processor_payment_method_token;
    }

    void set_connector_customer_id(std::string value)
    {
        connector_customer_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_connector_customer_id() const noexcept
    {
        return connector_customer_id;
    }

    void set_retry_count(std::optional<std::uint16_t> value)
    {
        retry_count = std::move(value);
    }

    [[nodiscard]] const std::optional<std::uint16_t>& get_retry_count() const noexcept
    {
        return retry_count;
    }

    void set_invoice_next_billing_time(std::optional<PrimitiveDateTimeValue> value)
    {
        invoice_next_billing_time = std::move(value);
    }

    [[nodiscard]] const std::optional<PrimitiveDateTimeValue>&
    get_invoice_next_billing_time() const noexcept
    {
        return invoice_next_billing_time;
    }

    void set_invoice_billing_started_at_time(std::optional<PrimitiveDateTimeValue> value)
    {
        invoice_billing_started_at_time = std::move(value);
    }

    [[nodiscard]] const std::optional<PrimitiveDateTimeValue>&
    get_invoice_billing_started_at_time() const noexcept
    {
        return invoice_billing_started_at_time;
    }

    void set_triggered_by(TriggeredByValue value)
    {
        triggered_by = std::move(value);
    }

    [[nodiscard]] const TriggeredByValue& get_triggered_by() const noexcept
    {
        return triggered_by;
    }

    void set_card_network(std::optional<CardNetworkValue> value)
    {
        card_network = std::move(value);
    }

    [[nodiscard]] const std::optional<CardNetworkValue>& get_card_network() const noexcept
    {
        return card_network;
    }

    void set_card_issuer(std::optional<std::string> value)
    {
        card_issuer = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_card_issuer() const noexcept
    {
        return card_issuer;
    }

    void set_standardised_error_code(std::optional<StandardisedCodeValue> value)
    {
        standardised_error_code = std::move(value);
    }

    [[nodiscard]] const std::optional<StandardisedCodeValue>&
    get_standardised_error_code() const noexcept
    {
        return standardised_error_code;
    }

    PaymentAttemptAmountDetails amount_details{};
    AttemptStatusValue status{};
    std::optional<Address> billing{};
    std::optional<Address> shipping{};
    std::optional<RecordAttemptErrorDetails> error{};
    std::optional<DescriptionValue> description{};
    std::optional<ConnectorTransactionIdValue> connector_transaction_id{};
    PaymentMethodValue payment_method_type{};
    RecoveryConnectorDetailsValue connector_details{};
    PaymentMethodTypeValue payment_method_subtype{};
    std::optional<RecordAttemptPaymentMethodDataRequest> payment_method_data{};
    std::optional<SecretSerdeValueValue> metadata{};
    std::optional<PaymentAttemptFeatureMetadata> feature_metadata{};
    std::optional<PrimitiveDateTimeValue> transaction_created_at{};
    std::optional<std::string> processor_payment_method_token{};
    std::string connector_customer_id{};
    std::optional<std::uint16_t> retry_count{};
    std::optional<PrimitiveDateTimeValue> invoice_next_billing_time{};
    std::optional<PrimitiveDateTimeValue> invoice_billing_started_at_time{};
    TriggeredByValue triggered_by{};
    std::optional<CardNetworkValue> card_network{};
    std::optional<std::string> card_issuer{};
    std::optional<StandardisedCodeValue> standardised_error_code{};
};

class PaymentsConfirmIntentRequest : public core::behavior::Serializer<PaymentsConfirmIntentRequest>
{
public:
    void set_return_url(std::optional<UrlValue> value)
    {
        return_url = std::move(value);
    }

    [[nodiscard]] const std::optional<UrlValue>& get_return_url() const noexcept
    {
        return return_url;
    }

    void set_payment_method_data(PaymentMethodDataRequest value)
    {
        payment_method_data = std::move(value);
    }

    [[nodiscard]] const PaymentMethodDataRequest& get_payment_method_data() const noexcept
    {
        return payment_method_data;
    }

    void set_split_payment_method_data(
        std::optional<std::vector<SplitPaymentMethodDataRequest>> value
    )
    {
        split_payment_method_data = std::move(value);
    }

    [[nodiscard]] const std::optional<std::vector<SplitPaymentMethodDataRequest>>&
    get_split_payment_method_data() const noexcept
    {
        return split_payment_method_data;
    }

    void set_payment_method_type(PaymentMethodValue value)
    {
        payment_method_type = std::move(value);
    }

    [[nodiscard]] const PaymentMethodValue& get_payment_method_type() const noexcept
    {
        return payment_method_type;
    }

    void set_payment_method_subtype(std::optional<PaymentMethodTypeValue> value)
    {
        payment_method_subtype = std::move(value);
    }

    [[nodiscard]] const std::optional<PaymentMethodTypeValue>&
    get_payment_method_subtype() const noexcept
    {
        return payment_method_subtype;
    }

    void set_shipping(std::optional<Address> value)
    {
        shipping = std::move(value);
    }

    [[nodiscard]] const std::optional<Address>& get_shipping() const noexcept
    {
        return shipping;
    }

    void set_customer_acceptance(std::optional<CustomerAcceptanceValue> value)
    {
        customer_acceptance = std::move(value);
    }

    [[nodiscard]] const std::optional<CustomerAcceptanceValue>&
    get_customer_acceptance() const noexcept
    {
        return customer_acceptance;
    }

    void set_browser_info(std::optional<BrowserInformation> value)
    {
        browser_info = std::move(value);
    }

    [[nodiscard]] const std::optional<BrowserInformation>& get_browser_info() const noexcept
    {
        return browser_info;
    }

    void set_payment_method_id(std::optional<GlobalPaymentMethodIdValue> value)
    {
        payment_method_id = std::move(value);
    }

    [[nodiscard]] const std::optional<GlobalPaymentMethodIdValue>&
    get_payment_method_id() const noexcept
    {
        return payment_method_id;
    }

    void set_payment_token(std::optional<std::string> value)
    {
        payment_token = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_payment_token() const noexcept
    {
        return payment_token;
    }

    void set_merchant_connector_details(std::optional<MerchantConnectorAuthDetailsValue> value)
    {
        merchant_connector_details = std::move(value);
    }

    [[nodiscard]] const std::optional<MerchantConnectorAuthDetailsValue>&
    get_merchant_connector_details() const noexcept
    {
        return merchant_connector_details;
    }

    void set_return_raw_connector_response(std::optional<bool> value)
    {
        return_raw_connector_response = std::move(value);
    }

    [[nodiscard]] const std::optional<bool>& get_return_raw_connector_response() const noexcept
    {
        return return_raw_connector_response;
    }

    void set_webhook_url(std::optional<UrlValue> value)
    {
        webhook_url = std::move(value);
    }

    [[nodiscard]] const std::optional<UrlValue>& get_webhook_url() const noexcept
    {
        return webhook_url;
    }

    void set_enable_avs_check(std::optional<bool> value)
    {
        enable_avs_check = std::move(value);
    }

    [[nodiscard]] const std::optional<bool>& get_enable_avs_check() const noexcept
    {
        return enable_avs_check;
    }

    void set_enable_account_name_verification(std::optional<bool> value)
    {
        enable_account_name_verification = std::move(value);
    }

    [[nodiscard]] const std::optional<bool>& get_enable_account_name_verification() const noexcept
    {
        return enable_account_name_verification;
    }

    std::optional<UrlValue> return_url{};
    PaymentMethodDataRequest payment_method_data{};
    std::optional<std::vector<SplitPaymentMethodDataRequest>> split_payment_method_data{};
    PaymentMethodValue payment_method_type{};
    std::optional<PaymentMethodTypeValue> payment_method_subtype{};
    std::optional<Address> shipping{};
    std::optional<CustomerAcceptanceValue> customer_acceptance{};
    std::optional<BrowserInformation> browser_info{};
    std::optional<GlobalPaymentMethodIdValue> payment_method_id{};
    std::optional<std::string> payment_token{};
    std::optional<MerchantConnectorAuthDetailsValue> merchant_connector_details{};
    std::optional<bool> return_raw_connector_response{};
    std::optional<UrlValue> webhook_url{};
    std::optional<bool> enable_avs_check{};
    std::optional<bool> enable_account_name_verification{};
};

class PaymentsEligibilityCheckRequest :
    public core::behavior::Serializer<PaymentsEligibilityCheckRequest>
{
public:
    void set_payment_id(PaymentIdValue value)
    {
        payment_id = std::move(value);
    }

    [[nodiscard]] const PaymentIdValue& get_payment_id() const noexcept
    {
        return payment_id;
    }

    void set_client_secret(std::optional<SecretValue> value)
    {
        client_secret = std::move(value);
    }

    [[nodiscard]] const std::optional<SecretValue>& get_client_secret() const noexcept
    {
        return client_secret;
    }

    void set_payment_method_type(PaymentMethodValue value)
    {
        payment_method_type = std::move(value);
    }

    [[nodiscard]] const PaymentMethodValue& get_payment_method_type() const noexcept
    {
        return payment_method_type;
    }

    void set_payment_method_subtype(std::optional<PaymentMethodTypeValue> value)
    {
        payment_method_subtype = std::move(value);
    }

    [[nodiscard]] const std::optional<PaymentMethodTypeValue>&
    get_payment_method_subtype() const noexcept
    {
        return payment_method_subtype;
    }

    void set_payment_method_data(std::optional<EligibilityPaymentMethodDataRequest> value)
    {
        payment_method_data = std::move(value);
    }

    [[nodiscard]] const std::optional<EligibilityPaymentMethodDataRequest>&
    get_payment_method_data() const noexcept
    {
        return payment_method_data;
    }

    void set_browser_info(std::optional<BrowserInformation> value)
    {
        browser_info = std::move(value);
    }

    [[nodiscard]] const std::optional<BrowserInformation>& get_browser_info() const noexcept
    {
        return browser_info;
    }

    void set_payment_token(std::optional<SecretValue> value)
    {
        payment_token = std::move(value);
    }

    [[nodiscard]] const std::optional<SecretValue>& get_payment_token() const noexcept
    {
        return payment_token;
    }

    PaymentIdValue payment_id{};
    std::optional<SecretValue> client_secret{};
    PaymentMethodValue payment_method_type{};
    std::optional<PaymentMethodTypeValue> payment_method_subtype{};
    std::optional<EligibilityPaymentMethodDataRequest> payment_method_data{};
    std::optional<BrowserInformation> browser_info{};
    std::optional<SecretValue> payment_token{};
};

class PaymentsEligibilityRequest : public core::behavior::Serializer<PaymentsEligibilityRequest>
{
public:
    void set_payment_id(PaymentIdValue value)
    {
        payment_id = std::move(value);
    }

    [[nodiscard]] const PaymentIdValue& get_payment_id() const noexcept
    {
        return payment_id;
    }

    void set_client_secret(std::optional<SecretValue> value)
    {
        client_secret = std::move(value);
    }

    [[nodiscard]] const std::optional<SecretValue>& get_client_secret() const noexcept
    {
        return client_secret;
    }

    void set_payment_method_type(PaymentMethodValue value)
    {
        payment_method_type = std::move(value);
    }

    [[nodiscard]] const PaymentMethodValue& get_payment_method_type() const noexcept
    {
        return payment_method_type;
    }

    void set_payment_method_subtype(std::optional<PaymentMethodTypeValue> value)
    {
        payment_method_subtype = std::move(value);
    }

    [[nodiscard]] const std::optional<PaymentMethodTypeValue>&
    get_payment_method_subtype() const noexcept
    {
        return payment_method_subtype;
    }

    void set_payment_method_data(std::optional<EligibilityPaymentMethodDataRequest> value)
    {
        payment_method_data = std::move(value);
    }

    [[nodiscard]] const std::optional<EligibilityPaymentMethodDataRequest>&
    get_payment_method_data() const noexcept
    {
        return payment_method_data;
    }

    void set_browser_info(std::optional<BrowserInformation> value)
    {
        browser_info = std::move(value);
    }

    [[nodiscard]] const std::optional<BrowserInformation>& get_browser_info() const noexcept
    {
        return browser_info;
    }

    void set_payment_token(std::optional<SecretValue> value)
    {
        payment_token = std::move(value);
    }

    [[nodiscard]] const std::optional<SecretValue>& get_payment_token() const noexcept
    {
        return payment_token;
    }

    PaymentIdValue payment_id{};
    std::optional<SecretValue> client_secret{};
    PaymentMethodValue payment_method_type{};
    std::optional<PaymentMethodTypeValue> payment_method_subtype{};
    std::optional<EligibilityPaymentMethodDataRequest> payment_method_data{};
    std::optional<BrowserInformation> browser_info{};
    std::optional<SecretValue> payment_token{};
};

class PaymentsRequest : public core::behavior::Serializer<PaymentsRequest>
{
public:
    void set_amount(std::optional<Amount> value)
    {
        amount = std::move(value);
    }

    [[nodiscard]] const std::optional<Amount>& get_amount() const noexcept
    {
        return amount;
    }

    void set_order_tax_amount(std::optional<MinorUnitValue> value)
    {
        order_tax_amount = std::move(value);
    }

    [[nodiscard]] const std::optional<MinorUnitValue>& get_order_tax_amount() const noexcept
    {
        return order_tax_amount;
    }

    void set_currency(std::optional<CurrencyValue> value)
    {
        currency = std::move(value);
    }

    [[nodiscard]] const std::optional<CurrencyValue>& get_currency() const noexcept
    {
        return currency;
    }

    void set_amount_to_capture(std::optional<MinorUnitValue> value)
    {
        amount_to_capture = std::move(value);
    }

    [[nodiscard]] const std::optional<MinorUnitValue>& get_amount_to_capture() const noexcept
    {
        return amount_to_capture;
    }

    void set_shipping_cost(std::optional<MinorUnitValue> value)
    {
        shipping_cost = std::move(value);
    }

    [[nodiscard]] const std::optional<MinorUnitValue>& get_shipping_cost() const noexcept
    {
        return shipping_cost;
    }

    void set_payment_id(std::optional<PaymentIdType> value)
    {
        payment_id = std::move(value);
    }

    [[nodiscard]] const std::optional<PaymentIdType>& get_payment_id() const noexcept
    {
        return payment_id;
    }

    void set_merchant_id(std::optional<MerchantIdValue> value)
    {
        merchant_id = std::move(value);
    }

    [[nodiscard]] const std::optional<MerchantIdValue>& get_merchant_id() const noexcept
    {
        return merchant_id;
    }

    void set_routing(std::optional<serde::Value> value)
    {
        routing = std::move(value);
    }

    [[nodiscard]] const std::optional<serde::Value>& get_routing() const noexcept
    {
        return routing;
    }

    void set_connector(std::optional<std::vector<ConnectorValue>> value)
    {
        connector = std::move(value);
    }

    [[nodiscard]] const std::optional<std::vector<ConnectorValue>>& get_connector() const noexcept
    {
        return connector;
    }

    void set_capture_method(std::optional<CaptureMethodValue> value)
    {
        capture_method = std::move(value);
    }

    [[nodiscard]] const std::optional<CaptureMethodValue>& get_capture_method() const noexcept
    {
        return capture_method;
    }

    void set_authentication_type(std::optional<AuthenticationTypeValue> value)
    {
        authentication_type = std::move(value);
    }

    [[nodiscard]] const std::optional<AuthenticationTypeValue>&
    get_authentication_type() const noexcept
    {
        return authentication_type;
    }

    void set_billing(std::optional<Address> value)
    {
        billing = std::move(value);
    }

    [[nodiscard]] const std::optional<Address>& get_billing() const noexcept
    {
        return billing;
    }

    void set_capture_on(std::optional<PrimitiveDateTimeValue> value)
    {
        capture_on = std::move(value);
    }

    [[nodiscard]] const std::optional<PrimitiveDateTimeValue>& get_capture_on() const noexcept
    {
        return capture_on;
    }

    void set_confirm(std::optional<bool> value)
    {
        confirm = std::move(value);
    }

    [[nodiscard]] const std::optional<bool>& get_confirm() const noexcept
    {
        return confirm;
    }

    void set_customer(std::optional<CustomerDetails> value)
    {
        customer = std::move(value);
    }

    [[nodiscard]] const std::optional<CustomerDetails>& get_customer() const noexcept
    {
        return customer;
    }

    void set_customer_id(std::optional<CustomerIdValue> value)
    {
        customer_id = std::move(value);
    }

    [[nodiscard]] const std::optional<CustomerIdValue>& get_customer_id() const noexcept
    {
        return customer_id;
    }

    void set_email(std::optional<EmailValue> value)
    {
        email = std::move(value);
    }

    [[nodiscard]] const std::optional<EmailValue>& get_email() const noexcept
    {
        return email;
    }

    void set_name(std::optional<SecretValue> value)
    {
        name = std::move(value);
    }

    [[nodiscard]] const std::optional<SecretValue>& get_name() const noexcept
    {
        return name;
    }

    void set_phone(std::optional<SecretValue> value)
    {
        phone = std::move(value);
    }

    [[nodiscard]] const std::optional<SecretValue>& get_phone() const noexcept
    {
        return phone;
    }

    void set_phone_country_code(std::optional<std::string> value)
    {
        phone_country_code = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_phone_country_code() const noexcept
    {
        return phone_country_code;
    }

    void set_off_session(std::optional<bool> value)
    {
        off_session = std::move(value);
    }

    [[nodiscard]] const std::optional<bool>& get_off_session() const noexcept
    {
        return off_session;
    }

    void set_description(std::optional<std::string> value)
    {
        description = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_description() const noexcept
    {
        return description;
    }

    void set_return_url(std::optional<UrlValue> value)
    {
        return_url = std::move(value);
    }

    [[nodiscard]] const std::optional<UrlValue>& get_return_url() const noexcept
    {
        return return_url;
    }

    void set_setup_future_usage(std::optional<FutureUsageValue> value)
    {
        setup_future_usage = std::move(value);
    }

    [[nodiscard]] const std::optional<FutureUsageValue>& get_setup_future_usage() const noexcept
    {
        return setup_future_usage;
    }

    void set_payment_method_data(std::optional<PaymentMethodDataRequest> value)
    {
        payment_method_data = std::move(value);
    }

    [[nodiscard]] const std::optional<PaymentMethodDataRequest>&
    get_payment_method_data() const noexcept
    {
        return payment_method_data;
    }

    void set_payment_method(std::optional<PaymentMethodValue> value)
    {
        payment_method = std::move(value);
    }

    [[nodiscard]] const std::optional<PaymentMethodValue>& get_payment_method() const noexcept
    {
        return payment_method;
    }

    void set_payment_token(std::optional<std::string> value)
    {
        payment_token = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_payment_token() const noexcept
    {
        return payment_token;
    }

    void set_card_cvc(std::optional<SecretValue> value)
    {
        card_cvc = std::move(value);
    }

    [[nodiscard]] const std::optional<SecretValue>& get_card_cvc() const noexcept
    {
        return card_cvc;
    }

    void set_shipping(std::optional<Address> value)
    {
        shipping = std::move(value);
    }

    [[nodiscard]] const std::optional<Address>& get_shipping() const noexcept
    {
        return shipping;
    }

    void set_statement_descriptor_name(std::optional<std::string> value)
    {
        statement_descriptor_name = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_statement_descriptor_name() const noexcept
    {
        return statement_descriptor_name;
    }

    void set_statement_descriptor_suffix(std::optional<std::string> value)
    {
        statement_descriptor_suffix = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_statement_descriptor_suffix() const noexcept
    {
        return statement_descriptor_suffix;
    }

    std::optional<Amount> amount{};
    std::optional<MinorUnitValue> order_tax_amount{};
    std::optional<CurrencyValue> currency{};
    std::optional<MinorUnitValue> amount_to_capture{};
    std::optional<MinorUnitValue> shipping_cost{};
    std::optional<PaymentIdType> payment_id{};
    std::optional<MerchantIdValue> merchant_id{};
    std::optional<serde::Value> routing{};
    std::optional<std::vector<ConnectorValue>> connector{};
    std::optional<CaptureMethodValue> capture_method{};
    std::optional<AuthenticationTypeValue> authentication_type{};
    std::optional<Address> billing{};
    std::optional<PrimitiveDateTimeValue> capture_on{};
    std::optional<bool> confirm{};
    std::optional<CustomerDetails> customer{};
    std::optional<CustomerIdValue> customer_id{};
    std::optional<EmailValue> email{};
    std::optional<SecretValue> name{};
    std::optional<SecretValue> phone{};
    std::optional<std::string> phone_country_code{};
    std::optional<bool> off_session{};
    std::optional<std::string> description{};
    std::optional<UrlValue> return_url{};
    std::optional<FutureUsageValue> setup_future_usage{};
    std::optional<PaymentMethodDataRequest> payment_method_data{};
    std::optional<PaymentMethodValue> payment_method{};
    std::optional<std::string> payment_token{};
    std::optional<SecretValue> card_cvc{};
    std::optional<Address> shipping{};
    std::optional<std::string> statement_descriptor_name{};
    std::optional<std::string> statement_descriptor_suffix{};
};

class PaymentAttemptListResponse : public core::behavior::Serializer<PaymentAttemptListResponse>
{
public:
    void set_payment_attempt_list(std::vector<PaymentAttemptResponse> value)
    {
        payment_attempt_list = std::move(value);
    }

    [[nodiscard]] const std::vector<PaymentAttemptResponse>&
    get_payment_attempt_list() const noexcept
    {
        return payment_attempt_list;
    }

    std::vector<PaymentAttemptResponse> payment_attempt_list{};
};

class PaymentAttemptRecordResponse : public core::behavior::Serializer<PaymentAttemptRecordResponse>
{
public:
    void set_id(GlobalAttemptIdValue value)
    {
        id = std::move(value);
    }

    [[nodiscard]] const GlobalAttemptIdValue& get_id() const noexcept
    {
        return id;
    }

    void set_status(AttemptStatusValue value)
    {
        status = std::move(value);
    }

    [[nodiscard]] const AttemptStatusValue& get_status() const noexcept
    {
        return status;
    }

    void set_amount(MinorUnitValue value)
    {
        amount = std::move(value);
    }

    [[nodiscard]] const MinorUnitValue& get_amount() const noexcept
    {
        return amount;
    }

    void set_error_details(std::optional<RecordAttemptErrorDetails> value)
    {
        error_details = std::move(value);
    }

    [[nodiscard]] const std::optional<RecordAttemptErrorDetails>& get_error_details() const noexcept
    {
        return error_details;
    }

    void set_payment_intent_feature_metadata(std::optional<FeatureMetadata> value)
    {
        payment_intent_feature_metadata = std::move(value);
    }

    [[nodiscard]] const std::optional<FeatureMetadata>&
    get_payment_intent_feature_metadata() const noexcept
    {
        return payment_intent_feature_metadata;
    }

    void set_payment_attempt_feature_metadata(std::optional<PaymentAttemptFeatureMetadata> value)
    {
        payment_attempt_feature_metadata = std::move(value);
    }

    [[nodiscard]] const std::optional<PaymentAttemptFeatureMetadata>&
    get_payment_attempt_feature_metadata() const noexcept
    {
        return payment_attempt_feature_metadata;
    }

    void set_created_at(PrimitiveDateTimeValue value)
    {
        created_at = std::move(value);
    }

    [[nodiscard]] const PrimitiveDateTimeValue& get_created_at() const noexcept
    {
        return created_at;
    }

    GlobalAttemptIdValue id{};
    AttemptStatusValue status{};
    MinorUnitValue amount{};
    std::optional<RecordAttemptErrorDetails> error_details{};
    std::optional<FeatureMetadata> payment_intent_feature_metadata{};
    std::optional<PaymentAttemptFeatureMetadata> payment_attempt_feature_metadata{};
    PrimitiveDateTimeValue created_at{};
};

class PaymentsCancelResponse : public core::behavior::Serializer<PaymentsCancelResponse>
{
public:
    void set_id(GlobalPaymentIdValue value)
    {
        id = std::move(value);
    }

    [[nodiscard]] const GlobalPaymentIdValue& get_id() const noexcept
    {
        return id;
    }

    void set_status(IntentStatusValue value)
    {
        status = std::move(value);
    }

    [[nodiscard]] const IntentStatusValue& get_status() const noexcept
    {
        return status;
    }

    void set_cancellation_reason(std::optional<std::string> value)
    {
        cancellation_reason = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_cancellation_reason() const noexcept
    {
        return cancellation_reason;
    }

    void set_amount(PaymentAmountDetailsResponse value)
    {
        amount = std::move(value);
    }

    [[nodiscard]] const PaymentAmountDetailsResponse& get_amount() const noexcept
    {
        return amount;
    }

    void set_customer_id(std::optional<GlobalCustomerIdValue> value)
    {
        customer_id = std::move(value);
    }

    [[nodiscard]] const std::optional<GlobalCustomerIdValue>& get_customer_id() const noexcept
    {
        return customer_id;
    }

    void set_connector(std::optional<ConnectorValue> value)
    {
        connector = std::move(value);
    }

    [[nodiscard]] const std::optional<ConnectorValue>& get_connector() const noexcept
    {
        return connector;
    }

    void set_created(PrimitiveDateTimeValue value)
    {
        created = std::move(value);
    }

    [[nodiscard]] const PrimitiveDateTimeValue& get_created() const noexcept
    {
        return created;
    }

    void set_payment_method_type(std::optional<PaymentMethodValue> value)
    {
        payment_method_type = std::move(value);
    }

    [[nodiscard]] const std::optional<PaymentMethodValue>& get_payment_method_type() const noexcept
    {
        return payment_method_type;
    }

    void set_payment_method_subtype(std::optional<PaymentMethodTypeValue> value)
    {
        payment_method_subtype = std::move(value);
    }

    [[nodiscard]] const std::optional<PaymentMethodTypeValue>&
    get_payment_method_subtype() const noexcept
    {
        return payment_method_subtype;
    }

    void set_attempts(std::optional<std::vector<PaymentAttemptResponse>> value)
    {
        attempts = std::move(value);
    }

    [[nodiscard]] const std::optional<std::vector<PaymentAttemptResponse>>&
    get_attempts() const noexcept
    {
        return attempts;
    }

    void set_return_url(std::optional<UrlValue> value)
    {
        return_url = std::move(value);
    }

    [[nodiscard]] const std::optional<UrlValue>& get_return_url() const noexcept
    {
        return return_url;
    }

    void set_error(std::optional<ErrorDetails> value)
    {
        error = std::move(value);
    }

    [[nodiscard]] const std::optional<ErrorDetails>& get_error() const noexcept
    {
        return error;
    }

    GlobalPaymentIdValue id{};
    IntentStatusValue status{};
    std::optional<std::string> cancellation_reason{};
    PaymentAmountDetailsResponse amount{};
    std::optional<GlobalCustomerIdValue> customer_id{};
    std::optional<ConnectorValue> connector{};
    PrimitiveDateTimeValue created{};
    std::optional<PaymentMethodValue> payment_method_type{};
    std::optional<PaymentMethodTypeValue> payment_method_subtype{};
    std::optional<std::vector<PaymentAttemptResponse>> attempts{};
    std::optional<UrlValue> return_url{};
    std::optional<ErrorDetails> error{};
};

class PaymentsResponse : public core::behavior::Serializer<PaymentsResponse>
{
public:
    void set_payment_id(PaymentIdValue value)
    {
        payment_id = std::move(value);
    }

    [[nodiscard]] const PaymentIdValue& get_payment_id() const noexcept
    {
        return payment_id;
    }

    void set_merchant_id(MerchantIdValue value)
    {
        merchant_id = std::move(value);
    }

    [[nodiscard]] const MerchantIdValue& get_merchant_id() const noexcept
    {
        return merchant_id;
    }

    void set_status(IntentStatusValue value)
    {
        status = std::move(value);
    }

    [[nodiscard]] const IntentStatusValue& get_status() const noexcept
    {
        return status;
    }

    void set_amount(MinorUnitValue value)
    {
        amount = std::move(value);
    }

    [[nodiscard]] const MinorUnitValue& get_amount() const noexcept
    {
        return amount;
    }

    void set_net_amount(MinorUnitValue value)
    {
        net_amount = std::move(value);
    }

    [[nodiscard]] const MinorUnitValue& get_net_amount() const noexcept
    {
        return net_amount;
    }

    void set_shipping_cost(std::optional<MinorUnitValue> value)
    {
        shipping_cost = std::move(value);
    }

    [[nodiscard]] const std::optional<MinorUnitValue>& get_shipping_cost() const noexcept
    {
        return shipping_cost;
    }

    void set_amount_capturable(MinorUnitValue value)
    {
        amount_capturable = std::move(value);
    }

    [[nodiscard]] const MinorUnitValue& get_amount_capturable() const noexcept
    {
        return amount_capturable;
    }

    void set_amount_received(std::optional<MinorUnitValue> value)
    {
        amount_received = std::move(value);
    }

    [[nodiscard]] const std::optional<MinorUnitValue>& get_amount_received() const noexcept
    {
        return amount_received;
    }

    void set_processor_merchant_id(MerchantIdValue value)
    {
        processor_merchant_id = std::move(value);
    }

    [[nodiscard]] const MerchantIdValue& get_processor_merchant_id() const noexcept
    {
        return processor_merchant_id;
    }

    void set_initiator(std::optional<InitiatorValue> value)
    {
        initiator = std::move(value);
    }

    [[nodiscard]] const std::optional<InitiatorValue>& get_initiator() const noexcept
    {
        return initiator;
    }

    void set_sdk_authorization(std::optional<std::string> value)
    {
        sdk_authorization = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_sdk_authorization() const noexcept
    {
        return sdk_authorization;
    }

    void set_payment_method_list(std::optional<PaymentMethodListResultValue> value)
    {
        payment_method_list = std::move(value);
    }

    [[nodiscard]] const std::optional<PaymentMethodListResultValue>&
    get_payment_method_list() const noexcept
    {
        return payment_method_list;
    }

    void set_session_tokens(std::optional<SessionTokensResult> value)
    {
        session_tokens = std::move(value);
    }

    [[nodiscard]] const std::optional<SessionTokensResult>& get_session_tokens() const noexcept
    {
        return session_tokens;
    }

    void set_connector(std::optional<std::string> value)
    {
        connector = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_connector() const noexcept
    {
        return connector;
    }

    void set_state_metadata(std::optional<PaymentIntentStateMetadataValue> value)
    {
        state_metadata = std::move(value);
    }

    [[nodiscard]] const std::optional<PaymentIntentStateMetadataValue>&
    get_state_metadata() const noexcept
    {
        return state_metadata;
    }

    void set_client_secret(std::optional<SecretValue> value)
    {
        client_secret = std::move(value);
    }

    [[nodiscard]] const std::optional<SecretValue>& get_client_secret() const noexcept
    {
        return client_secret;
    }

    void set_created(std::optional<PrimitiveDateTimeValue> value)
    {
        created = std::move(value);
    }

    [[nodiscard]] const std::optional<PrimitiveDateTimeValue>& get_created() const noexcept
    {
        return created;
    }

    void set_modified_at(std::optional<PrimitiveDateTimeValue> value)
    {
        modified_at = std::move(value);
    }

    [[nodiscard]] const std::optional<PrimitiveDateTimeValue>& get_modified_at() const noexcept
    {
        return modified_at;
    }

    void set_connector_customer_id(std::optional<std::string> value)
    {
        connector_customer_id = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_connector_customer_id() const noexcept
    {
        return connector_customer_id;
    }

    void set_currency(std::string value)
    {
        currency = std::move(value);
    }

    [[nodiscard]] const std::string& get_currency() const noexcept
    {
        return currency;
    }

    void set_customer_id(std::optional<CustomerIdValue> value)
    {
        customer_id = std::move(value);
    }

    [[nodiscard]] const std::optional<CustomerIdValue>& get_customer_id() const noexcept
    {
        return customer_id;
    }

    void set_customer(std::optional<CustomerDetailsResponse> value)
    {
        customer = std::move(value);
    }

    [[nodiscard]] const std::optional<CustomerDetailsResponse>& get_customer() const noexcept
    {
        return customer;
    }

    void set_description(std::optional<std::string> value)
    {
        description = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_description() const noexcept
    {
        return description;
    }

    void set_refunds(std::optional<std::vector<RefundResponseValue>> value)
    {
        refunds = std::move(value);
    }

    [[nodiscard]] const std::optional<std::vector<RefundResponseValue>>&
    get_refunds() const noexcept
    {
        return refunds;
    }

    void set_disputes(std::optional<std::vector<DisputeResponsePaymentsRetrieveValue>> value)
    {
        disputes = std::move(value);
    }

    [[nodiscard]] const std::optional<std::vector<DisputeResponsePaymentsRetrieveValue>>&
    get_disputes() const noexcept
    {
        return disputes;
    }

    void set_attempts(std::optional<std::vector<PaymentAttemptResponse>> value)
    {
        attempts = std::move(value);
    }

    [[nodiscard]] const std::optional<std::vector<PaymentAttemptResponse>>&
    get_attempts() const noexcept
    {
        return attempts;
    }

    void set_captures(std::optional<std::vector<CaptureResponse>> value)
    {
        captures = std::move(value);
    }

    [[nodiscard]] const std::optional<std::vector<CaptureResponse>>& get_captures() const noexcept
    {
        return captures;
    }

    void set_mandate_id(std::optional<std::string> value)
    {
        mandate_id = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_mandate_id() const noexcept
    {
        return mandate_id;
    }

    void set_mandate_data(std::optional<MandateData> value)
    {
        mandate_data = std::move(value);
    }

    [[nodiscard]] const std::optional<MandateData>& get_mandate_data() const noexcept
    {
        return mandate_data;
    }

    void set_setup_future_usage(std::optional<FutureUsageValue> value)
    {
        setup_future_usage = std::move(value);
    }

    [[nodiscard]] const std::optional<FutureUsageValue>& get_setup_future_usage() const noexcept
    {
        return setup_future_usage;
    }

    void set_off_session(std::optional<bool> value)
    {
        off_session = std::move(value);
    }

    [[nodiscard]] const std::optional<bool>& get_off_session() const noexcept
    {
        return off_session;
    }

    void set_capture_on(std::optional<PrimitiveDateTimeValue> value)
    {
        capture_on = std::move(value);
    }

    [[nodiscard]] const std::optional<PrimitiveDateTimeValue>& get_capture_on() const noexcept
    {
        return capture_on;
    }

    void set_capture_method(std::optional<CaptureMethodValue> value)
    {
        capture_method = std::move(value);
    }

    [[nodiscard]] const std::optional<CaptureMethodValue>& get_capture_method() const noexcept
    {
        return capture_method;
    }

    void set_capture_method_applied(std::optional<CaptureMethodValue> value)
    {
        capture_method_applied = std::move(value);
    }

    [[nodiscard]] const std::optional<CaptureMethodValue>&
    get_capture_method_applied() const noexcept
    {
        return capture_method_applied;
    }

    void set_payment_method(std::optional<PaymentMethodValue> value)
    {
        payment_method = std::move(value);
    }

    [[nodiscard]] const std::optional<PaymentMethodValue>& get_payment_method() const noexcept
    {
        return payment_method;
    }

    void set_payment_method_data(std::optional<PaymentMethodDataResponseWithBilling> value)
    {
        payment_method_data = std::move(value);
    }

    [[nodiscard]] const std::optional<PaymentMethodDataResponseWithBilling>&
    get_payment_method_data() const noexcept
    {
        return payment_method_data;
    }

    void set_payment_token(std::optional<std::string> value)
    {
        payment_token = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_payment_token() const noexcept
    {
        return payment_token;
    }

    void set_shipping(std::optional<Address> value)
    {
        shipping = std::move(value);
    }

    [[nodiscard]] const std::optional<Address>& get_shipping() const noexcept
    {
        return shipping;
    }

    void set_billing(std::optional<Address> value)
    {
        billing = std::move(value);
    }

    [[nodiscard]] const std::optional<Address>& get_billing() const noexcept
    {
        return billing;
    }

    void set_order_details(std::optional<std::vector<SecretSerdeValueValue>> value)
    {
        order_details = std::move(value);
    }

    [[nodiscard]] const std::optional<std::vector<SecretSerdeValueValue>>&
    get_order_details() const noexcept
    {
        return order_details;
    }

    void set_email(std::optional<EmailValue> value)
    {
        email = std::move(value);
    }

    [[nodiscard]] const std::optional<EmailValue>& get_email() const noexcept
    {
        return email;
    }

    void set_name(std::optional<SecretValue> value)
    {
        name = std::move(value);
    }

    [[nodiscard]] const std::optional<SecretValue>& get_name() const noexcept
    {
        return name;
    }

    void set_phone(std::optional<SecretValue> value)
    {
        phone = std::move(value);
    }

    [[nodiscard]] const std::optional<SecretValue>& get_phone() const noexcept
    {
        return phone;
    }

    void set_return_url(std::optional<std::string> value)
    {
        return_url = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_return_url() const noexcept
    {
        return return_url;
    }

    void set_authentication_type(std::optional<AuthenticationTypeValue> value)
    {
        authentication_type = std::move(value);
    }

    [[nodiscard]] const std::optional<AuthenticationTypeValue>&
    get_authentication_type() const noexcept
    {
        return authentication_type;
    }

    void set_statement_descriptor_name(std::optional<std::string> value)
    {
        statement_descriptor_name = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_statement_descriptor_name() const noexcept
    {
        return statement_descriptor_name;
    }

    void set_statement_descriptor_suffix(std::optional<std::string> value)
    {
        statement_descriptor_suffix = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_statement_descriptor_suffix() const noexcept
    {
        return statement_descriptor_suffix;
    }

    void set_next_action(std::optional<NextActionData> value)
    {
        next_action = std::move(value);
    }

    [[nodiscard]] const std::optional<NextActionData>& get_next_action() const noexcept
    {
        return next_action;
    }

    void set_cancellation_reason(std::optional<std::string> value)
    {
        cancellation_reason = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_cancellation_reason() const noexcept
    {
        return cancellation_reason;
    }

    void set_error_code(std::optional<std::string> value)
    {
        error_code = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_error_code() const noexcept
    {
        return error_code;
    }

    void set_error_message(std::optional<std::string> value)
    {
        error_message = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_error_message() const noexcept
    {
        return error_message;
    }

    void set_unified_code(std::optional<std::string> value)
    {
        unified_code = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_unified_code() const noexcept
    {
        return unified_code;
    }

    void set_unified_message(std::optional<std::string> value)
    {
        unified_message = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_unified_message() const noexcept
    {
        return unified_message;
    }

    void set_error_details(std::optional<PaymentErrorDetails> value)
    {
        error_details = std::move(value);
    }

    [[nodiscard]] const std::optional<PaymentErrorDetails>& get_error_details() const noexcept
    {
        return error_details;
    }

    void set_payment_experience(std::optional<PaymentExperienceValue> value)
    {
        payment_experience = std::move(value);
    }

    [[nodiscard]] const std::optional<PaymentExperienceValue>&
    get_payment_experience() const noexcept
    {
        return payment_experience;
    }

    void set_payment_method_type(std::optional<PaymentMethodTypeValue> value)
    {
        payment_method_type = std::move(value);
    }

    [[nodiscard]] const std::optional<PaymentMethodTypeValue>&
    get_payment_method_type() const noexcept
    {
        return payment_method_type;
    }

    void set_connector_label(std::optional<std::string> value)
    {
        connector_label = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_connector_label() const noexcept
    {
        return connector_label;
    }

    void set_business_country(std::optional<CountryAlpha2Value> value)
    {
        business_country = std::move(value);
    }

    [[nodiscard]] const std::optional<CountryAlpha2Value>& get_business_country() const noexcept
    {
        return business_country;
    }

    void set_business_label(std::optional<std::string> value)
    {
        business_label = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_business_label() const noexcept
    {
        return business_label;
    }

    void set_business_sub_label(std::optional<std::string> value)
    {
        business_sub_label = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_business_sub_label() const noexcept
    {
        return business_sub_label;
    }

    void set_allowed_payment_method_types(std::optional<serde::Value> value)
    {
        allowed_payment_method_types = std::move(value);
    }

    [[nodiscard]] const std::optional<serde::Value>&
    get_allowed_payment_method_types() const noexcept
    {
        return allowed_payment_method_types;
    }

    void set_manual_retry_allowed(std::optional<bool> value)
    {
        manual_retry_allowed = std::move(value);
    }

    [[nodiscard]] const std::optional<bool>& get_manual_retry_allowed() const noexcept
    {
        return manual_retry_allowed;
    }

    void set_connector_transaction_id(std::optional<std::string> value)
    {
        connector_transaction_id = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_connector_transaction_id() const noexcept
    {
        return connector_transaction_id;
    }

    void set_frm_message(std::optional<FrmMessage> value)
    {
        frm_message = std::move(value);
    }

    [[nodiscard]] const std::optional<FrmMessage>& get_frm_message() const noexcept
    {
        return frm_message;
    }

    void set_metadata(std::optional<serde::Value> value)
    {
        metadata = std::move(value);
    }

    [[nodiscard]] const std::optional<serde::Value>& get_metadata() const noexcept
    {
        return metadata;
    }

    void set_connector_metadata(std::optional<serde::Value> value)
    {
        connector_metadata = std::move(value);
    }

    [[nodiscard]] const std::optional<serde::Value>& get_connector_metadata() const noexcept
    {
        return connector_metadata;
    }

    void set_connector_response_metadata(std::optional<ConnectorMetadataResponse> value)
    {
        connector_response_metadata = std::move(value);
    }

    [[nodiscard]] const std::optional<ConnectorMetadataResponse>&
    get_connector_response_metadata() const noexcept
    {
        return connector_response_metadata;
    }

    void set_feature_metadata(std::optional<FeatureMetadata> value)
    {
        feature_metadata = std::move(value);
    }

    [[nodiscard]] const std::optional<FeatureMetadata>& get_feature_metadata() const noexcept
    {
        return feature_metadata;
    }

    void set_reference_id(std::optional<std::string> value)
    {
        reference_id = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_reference_id() const noexcept
    {
        return reference_id;
    }

    void set_payment_link(std::optional<PaymentLinkResponse> value)
    {
        payment_link = std::move(value);
    }

    [[nodiscard]] const std::optional<PaymentLinkResponse>& get_payment_link() const noexcept
    {
        return payment_link;
    }

    void set_profile_id(std::optional<ProfileIdValue> value)
    {
        profile_id = std::move(value);
    }

    [[nodiscard]] const std::optional<ProfileIdValue>& get_profile_id() const noexcept
    {
        return profile_id;
    }

    void set_surcharge_details(std::optional<PaymentSurchargeDetailsResponse> value)
    {
        surcharge_details = std::move(value);
    }

    [[nodiscard]] const std::optional<PaymentSurchargeDetailsResponse>&
    get_surcharge_details() const noexcept
    {
        return surcharge_details;
    }

    void set_applied_offer(std::optional<AppliedOffer> value)
    {
        applied_offer = std::move(value);
    }

    [[nodiscard]] const std::optional<AppliedOffer>& get_applied_offer() const noexcept
    {
        return applied_offer;
    }

    void set_attempt_count(std::int16_t value)
    {
        attempt_count = std::move(value);
    }

    [[nodiscard]] const std::int16_t& get_attempt_count() const noexcept
    {
        return attempt_count;
    }

    void set_merchant_decision(std::optional<std::string> value)
    {
        merchant_decision = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_merchant_decision() const noexcept
    {
        return merchant_decision;
    }

    void set_merchant_connector_id(std::optional<MerchantConnectorAccountIdValue> value)
    {
        merchant_connector_id = std::move(value);
    }

    [[nodiscard]] const std::optional<MerchantConnectorAccountIdValue>&
    get_merchant_connector_id() const noexcept
    {
        return merchant_connector_id;
    }

    void set_incremental_authorization_allowed(std::optional<bool> value)
    {
        incremental_authorization_allowed = std::move(value);
    }

    [[nodiscard]] const std::optional<bool>& get_incremental_authorization_allowed() const noexcept
    {
        return incremental_authorization_allowed;
    }

    void set_authorization_count(std::optional<std::int32_t> value)
    {
        authorization_count = std::move(value);
    }

    [[nodiscard]] const std::optional<std::int32_t>& get_authorization_count() const noexcept
    {
        return authorization_count;
    }

    void set_incremental_authorizations(
        std::optional<std::vector<IncrementalAuthorizationResponse>> value
    )
    {
        incremental_authorizations = std::move(value);
    }

    [[nodiscard]] const std::optional<std::vector<IncrementalAuthorizationResponse>>&
    get_incremental_authorizations() const noexcept
    {
        return incremental_authorizations;
    }

    void set_external_authentication_details(
        std::optional<ExternalAuthenticationDetailsResponse> value
    )
    {
        external_authentication_details = std::move(value);
    }

    [[nodiscard]] const std::optional<ExternalAuthenticationDetailsResponse>&
    get_external_authentication_details() const noexcept
    {
        return external_authentication_details;
    }

    void set_external_3ds_authentication_attempted(std::optional<bool> value)
    {
        external_3ds_authentication_attempted = std::move(value);
    }

    [[nodiscard]] const std::optional<bool>&
    get_external_3ds_authentication_attempted() const noexcept
    {
        return external_3ds_authentication_attempted;
    }

    void set_expires_on(std::optional<PrimitiveDateTimeValue> value)
    {
        expires_on = std::move(value);
    }

    [[nodiscard]] const std::optional<PrimitiveDateTimeValue>& get_expires_on() const noexcept
    {
        return expires_on;
    }

    void set_fingerprint(std::optional<std::string> value)
    {
        fingerprint = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_fingerprint() const noexcept
    {
        return fingerprint;
    }

    void set_fingerprint_type(std::optional<FingerprintTypeValue> value)
    {
        fingerprint_type = std::move(value);
    }

    [[nodiscard]] const std::optional<FingerprintTypeValue>& get_fingerprint_type() const noexcept
    {
        return fingerprint_type;
    }

    void set_browser_info(std::optional<serde::Value> value)
    {
        browser_info = std::move(value);
    }

    [[nodiscard]] const std::optional<serde::Value>& get_browser_info() const noexcept
    {
        return browser_info;
    }

    void set_payment_channel(std::optional<PaymentChannelValue> value)
    {
        payment_channel = std::move(value);
    }

    [[nodiscard]] const std::optional<PaymentChannelValue>& get_payment_channel() const noexcept
    {
        return payment_channel;
    }

    void set_payment_method_id(std::optional<std::string> value)
    {
        payment_method_id = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_payment_method_id() const noexcept
    {
        return payment_method_id;
    }

    void set_network_transaction_id(std::optional<std::string> value)
    {
        network_transaction_id = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_network_transaction_id() const noexcept
    {
        return network_transaction_id;
    }

    void set_payment_account_reference(std::optional<std::string> value)
    {
        payment_account_reference = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_payment_account_reference() const noexcept
    {
        return payment_account_reference;
    }

    void set_network_transaction_link_id(std::optional<std::string> value)
    {
        network_transaction_link_id = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_network_transaction_link_id() const noexcept
    {
        return network_transaction_link_id;
    }

    void set_payment_method_status(std::optional<PaymentMethodStatusValue> value)
    {
        payment_method_status = std::move(value);
    }

    [[nodiscard]] const std::optional<PaymentMethodStatusValue>&
    get_payment_method_status() const noexcept
    {
        return payment_method_status;
    }

    void set_updated(std::optional<PrimitiveDateTimeValue> value)
    {
        updated = std::move(value);
    }

    [[nodiscard]] const std::optional<PrimitiveDateTimeValue>& get_updated() const noexcept
    {
        return updated;
    }

    void set_split_payments(std::optional<ConnectorChargeResponseDataValue> value)
    {
        split_payments = std::move(value);
    }

    [[nodiscard]] const std::optional<ConnectorChargeResponseDataValue>&
    get_split_payments() const noexcept
    {
        return split_payments;
    }

    void set_frm_metadata(std::optional<SecretSerdeValueValue> value)
    {
        frm_metadata = std::move(value);
    }

    [[nodiscard]] const std::optional<SecretSerdeValueValue>& get_frm_metadata() const noexcept
    {
        return frm_metadata;
    }

    void set_extended_authorization_applied(
        std::optional<ExtendedAuthorizationAppliedBoolValue> value
    )
    {
        extended_authorization_applied = std::move(value);
    }

    [[nodiscard]] const std::optional<ExtendedAuthorizationAppliedBoolValue>&
    get_extended_authorization_applied() const noexcept
    {
        return extended_authorization_applied;
    }

    void set_extended_authorization_last_applied_at(std::optional<PrimitiveDateTimeValue> value)
    {
        extended_authorization_last_applied_at = std::move(value);
    }

    [[nodiscard]] const std::optional<PrimitiveDateTimeValue>&
    get_extended_authorization_last_applied_at() const noexcept
    {
        return extended_authorization_last_applied_at;
    }

    void set_request_extended_authorization(
        std::optional<RequestExtendedAuthorizationBoolValue> value
    )
    {
        request_extended_authorization = std::move(value);
    }

    [[nodiscard]] const std::optional<RequestExtendedAuthorizationBoolValue>&
    get_request_extended_authorization() const noexcept
    {
        return request_extended_authorization;
    }

    void set_capture_before(std::optional<PrimitiveDateTimeValue> value)
    {
        capture_before = std::move(value);
    }

    [[nodiscard]] const std::optional<PrimitiveDateTimeValue>& get_capture_before() const noexcept
    {
        return capture_before;
    }

    void set_merchant_order_reference_id(std::optional<std::string> value)
    {
        merchant_order_reference_id = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_merchant_order_reference_id() const noexcept
    {
        return merchant_order_reference_id;
    }

    void set_order_tax_amount(std::optional<MinorUnitValue> value)
    {
        order_tax_amount = std::move(value);
    }

    [[nodiscard]] const std::optional<MinorUnitValue>& get_order_tax_amount() const noexcept
    {
        return order_tax_amount;
    }

    void set_connector_mandate_id(std::optional<std::string> value)
    {
        connector_mandate_id = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_connector_mandate_id() const noexcept
    {
        return connector_mandate_id;
    }

    void set_card_discovery(std::optional<CardDiscoveryValue> value)
    {
        card_discovery = std::move(value);
    }

    [[nodiscard]] const std::optional<CardDiscoveryValue>& get_card_discovery() const noexcept
    {
        return card_discovery;
    }

    void set_force_3ds_challenge(std::optional<bool> value)
    {
        force_3ds_challenge = std::move(value);
    }

    [[nodiscard]] const std::optional<bool>& get_force_3ds_challenge() const noexcept
    {
        return force_3ds_challenge;
    }

    void set_force_3ds_challenge_trigger(std::optional<bool> value)
    {
        force_3ds_challenge_trigger = std::move(value);
    }

    [[nodiscard]] const std::optional<bool>& get_force_3ds_challenge_trigger() const noexcept
    {
        return force_3ds_challenge_trigger;
    }

    void set_issuer_error_code(std::optional<std::string> value)
    {
        issuer_error_code = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_issuer_error_code() const noexcept
    {
        return issuer_error_code;
    }

    void set_issuer_error_message(std::optional<std::string> value)
    {
        issuer_error_message = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_issuer_error_message() const noexcept
    {
        return issuer_error_message;
    }

    void set_is_iframe_redirection_enabled(std::optional<bool> value)
    {
        is_iframe_redirection_enabled = std::move(value);
    }

    [[nodiscard]] const std::optional<bool>& get_is_iframe_redirection_enabled() const noexcept
    {
        return is_iframe_redirection_enabled;
    }

    void set_whole_connector_response(std::optional<SecretValue> value)
    {
        whole_connector_response = std::move(value);
    }

    [[nodiscard]] const std::optional<SecretValue>& get_whole_connector_response() const noexcept
    {
        return whole_connector_response;
    }

    void set_enable_partial_authorization(std::optional<EnablePartialAuthorizationBoolValue> value)
    {
        enable_partial_authorization = std::move(value);
    }

    [[nodiscard]] const std::optional<EnablePartialAuthorizationBoolValue>&
    get_enable_partial_authorization() const noexcept
    {
        return enable_partial_authorization;
    }

    void set_enable_overcapture(std::optional<EnableOvercaptureBoolValue> value)
    {
        enable_overcapture = std::move(value);
    }

    [[nodiscard]] const std::optional<EnableOvercaptureBoolValue>&
    get_enable_overcapture() const noexcept
    {
        return enable_overcapture;
    }

    void set_is_overcapture_enabled(std::optional<OvercaptureEnabledBoolValue> value)
    {
        is_overcapture_enabled = std::move(value);
    }

    [[nodiscard]] const std::optional<OvercaptureEnabledBoolValue>&
    get_is_overcapture_enabled() const noexcept
    {
        return is_overcapture_enabled;
    }

    void set_network_details(std::optional<NetworkDetails> value)
    {
        network_details = std::move(value);
    }

    [[nodiscard]] const std::optional<NetworkDetails>& get_network_details() const noexcept
    {
        return network_details;
    }

    void set_is_stored_credential(std::optional<bool> value)
    {
        is_stored_credential = std::move(value);
    }

    [[nodiscard]] const std::optional<bool>& get_is_stored_credential() const noexcept
    {
        return is_stored_credential;
    }

    void set_mit_category(std::optional<MitCategoryValue> value)
    {
        mit_category = std::move(value);
    }

    [[nodiscard]] const std::optional<MitCategoryValue>& get_mit_category() const noexcept
    {
        return mit_category;
    }

    void set_billing_descriptor(std::optional<BillingDescriptorValue> value)
    {
        billing_descriptor = std::move(value);
    }

    [[nodiscard]] const std::optional<BillingDescriptorValue>&
    get_billing_descriptor() const noexcept
    {
        return billing_descriptor;
    }

    void set_is_account_funded_transaction(std::optional<bool> value)
    {
        is_account_funded_transaction = std::move(value);
    }

    [[nodiscard]] const std::optional<bool>& get_is_account_funded_transaction() const noexcept
    {
        return is_account_funded_transaction;
    }

    void set_recipient_details(std::optional<MaskedRecipientDetailsValue> value)
    {
        recipient_details = std::move(value);
    }

    [[nodiscard]] const std::optional<MaskedRecipientDetailsValue>&
    get_recipient_details() const noexcept
    {
        return recipient_details;
    }

    void set_tokenization(std::optional<TokenizationValue> value)
    {
        tokenization = std::move(value);
    }

    [[nodiscard]] const std::optional<TokenizationValue>& get_tokenization() const noexcept
    {
        return tokenization;
    }

    void set_partner_merchant_identifier_details(
        std::optional<PartnerMerchantIdentifierDetailsValue> value
    )
    {
        partner_merchant_identifier_details = std::move(value);
    }

    [[nodiscard]] const std::optional<PartnerMerchantIdentifierDetailsValue>&
    get_partner_merchant_identifier_details() const noexcept
    {
        return partner_merchant_identifier_details;
    }

    void set_payment_method_tokenization_details(
        std::optional<PaymentMethodTokenizationDetails> value
    )
    {
        payment_method_tokenization_details = std::move(value);
    }

    [[nodiscard]] const std::optional<PaymentMethodTokenizationDetails>&
    get_payment_method_tokenization_details() const noexcept
    {
        return payment_method_tokenization_details;
    }

    void set_installment_options(std::optional<std::vector<InstallmentOptionValue>> value)
    {
        installment_options = std::move(value);
    }

    [[nodiscard]] const std::optional<std::vector<InstallmentOptionValue>>&
    get_installment_options() const noexcept
    {
        return installment_options;
    }

    void set_installment_data(std::optional<InstallmentDataValue> value)
    {
        installment_data = std::move(value);
    }

    [[nodiscard]] const std::optional<InstallmentDataValue>& get_installment_data() const noexcept
    {
        return installment_data;
    }

    void set_sender_payment_instrument_id(std::optional<std::string> value)
    {
        sender_payment_instrument_id = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>&
    get_sender_payment_instrument_id() const noexcept
    {
        return sender_payment_instrument_id;
    }

    void set_enable_avs_check(std::optional<bool> value)
    {
        enable_avs_check = std::move(value);
    }

    [[nodiscard]] const std::optional<bool>& get_enable_avs_check() const noexcept
    {
        return enable_avs_check;
    }

    void set_enable_account_name_verification(std::optional<bool> value)
    {
        enable_account_name_verification = std::move(value);
    }

    [[nodiscard]] const std::optional<bool>& get_enable_account_name_verification() const noexcept
    {
        return enable_account_name_verification;
    }

    PaymentIdValue payment_id{};
    MerchantIdValue merchant_id{};
    IntentStatusValue status{};
    MinorUnitValue amount{};
    MinorUnitValue net_amount{};
    std::optional<MinorUnitValue> shipping_cost{};
    MinorUnitValue amount_capturable{};
    std::optional<MinorUnitValue> amount_received{};
    MerchantIdValue processor_merchant_id{};
    std::optional<InitiatorValue> initiator{};
    std::optional<std::string> sdk_authorization{};
    std::optional<PaymentMethodListResultValue> payment_method_list{};
    std::optional<SessionTokensResult> session_tokens{};
    std::optional<std::string> connector{};
    std::optional<PaymentIntentStateMetadataValue> state_metadata{};
    std::optional<SecretValue> client_secret{};
    std::optional<PrimitiveDateTimeValue> created{};
    std::optional<PrimitiveDateTimeValue> modified_at{};
    std::optional<std::string> connector_customer_id{};
    std::string currency{};
    std::optional<CustomerIdValue> customer_id{};
    std::optional<CustomerDetailsResponse> customer{};
    std::optional<std::string> description{};
    std::optional<std::vector<RefundResponseValue>> refunds{};
    std::optional<std::vector<DisputeResponsePaymentsRetrieveValue>> disputes{};
    std::optional<std::vector<PaymentAttemptResponse>> attempts{};
    std::optional<std::vector<CaptureResponse>> captures{};
    std::optional<std::string> mandate_id{};
    std::optional<MandateData> mandate_data{};
    std::optional<FutureUsageValue> setup_future_usage{};
    std::optional<bool> off_session{};
    std::optional<PrimitiveDateTimeValue> capture_on{};
    std::optional<CaptureMethodValue> capture_method{};
    std::optional<CaptureMethodValue> capture_method_applied{};
    std::optional<PaymentMethodValue> payment_method{};
    std::optional<PaymentMethodDataResponseWithBilling> payment_method_data{};
    std::optional<std::string> payment_token{};
    std::optional<Address> shipping{};
    std::optional<Address> billing{};
    std::optional<std::vector<SecretSerdeValueValue>> order_details{};
    std::optional<EmailValue> email{};
    std::optional<SecretValue> name{};
    std::optional<SecretValue> phone{};
    std::optional<std::string> return_url{};
    std::optional<AuthenticationTypeValue> authentication_type{};
    std::optional<std::string> statement_descriptor_name{};
    std::optional<std::string> statement_descriptor_suffix{};
    std::optional<NextActionData> next_action{};
    std::optional<std::string> cancellation_reason{};
    std::optional<std::string> error_code{};
    std::optional<std::string> error_message{};
    std::optional<std::string> unified_code{};
    std::optional<std::string> unified_message{};
    std::optional<PaymentErrorDetails> error_details{};
    std::optional<PaymentExperienceValue> payment_experience{};
    std::optional<PaymentMethodTypeValue> payment_method_type{};
    std::optional<std::string> connector_label{};
    std::optional<CountryAlpha2Value> business_country{};
    std::optional<std::string> business_label{};
    std::optional<std::string> business_sub_label{};
    std::optional<serde::Value> allowed_payment_method_types{};
    std::optional<bool> manual_retry_allowed{};
    std::optional<std::string> connector_transaction_id{};
    std::optional<FrmMessage> frm_message{};
    std::optional<serde::Value> metadata{};
    std::optional<serde::Value> connector_metadata{};
    std::optional<ConnectorMetadataResponse> connector_response_metadata{};
    std::optional<FeatureMetadata> feature_metadata{};
    std::optional<std::string> reference_id{};
    std::optional<PaymentLinkResponse> payment_link{};
    std::optional<ProfileIdValue> profile_id{};
    std::optional<PaymentSurchargeDetailsResponse> surcharge_details{};
    std::optional<AppliedOffer> applied_offer{};
    std::int16_t attempt_count{};
    std::optional<std::string> merchant_decision{};
    std::optional<MerchantConnectorAccountIdValue> merchant_connector_id{};
    std::optional<bool> incremental_authorization_allowed{};
    std::optional<std::int32_t> authorization_count{};
    std::optional<std::vector<IncrementalAuthorizationResponse>> incremental_authorizations{};
    std::optional<ExternalAuthenticationDetailsResponse> external_authentication_details{};
    std::optional<bool> external_3ds_authentication_attempted{};
    std::optional<PrimitiveDateTimeValue> expires_on{};
    std::optional<std::string> fingerprint{};
    std::optional<FingerprintTypeValue> fingerprint_type{};
    std::optional<serde::Value> browser_info{};
    std::optional<PaymentChannelValue> payment_channel{};
    std::optional<std::string> payment_method_id{};
    std::optional<std::string> network_transaction_id{};
    std::optional<std::string> payment_account_reference{};
    std::optional<std::string> network_transaction_link_id{};
    std::optional<PaymentMethodStatusValue> payment_method_status{};
    std::optional<PrimitiveDateTimeValue> updated{};
    std::optional<ConnectorChargeResponseDataValue> split_payments{};
    std::optional<SecretSerdeValueValue> frm_metadata{};
    std::optional<ExtendedAuthorizationAppliedBoolValue> extended_authorization_applied{};
    std::optional<PrimitiveDateTimeValue> extended_authorization_last_applied_at{};
    std::optional<RequestExtendedAuthorizationBoolValue> request_extended_authorization{};
    std::optional<PrimitiveDateTimeValue> capture_before{};
    std::optional<std::string> merchant_order_reference_id{};
    std::optional<MinorUnitValue> order_tax_amount{};
    std::optional<std::string> connector_mandate_id{};
    std::optional<CardDiscoveryValue> card_discovery{};
    std::optional<bool> force_3ds_challenge{};
    std::optional<bool> force_3ds_challenge_trigger{};
    std::optional<std::string> issuer_error_code{};
    std::optional<std::string> issuer_error_message{};
    std::optional<bool> is_iframe_redirection_enabled{};
    std::optional<SecretValue> whole_connector_response{};
    std::optional<EnablePartialAuthorizationBoolValue> enable_partial_authorization{};
    std::optional<EnableOvercaptureBoolValue> enable_overcapture{};
    std::optional<OvercaptureEnabledBoolValue> is_overcapture_enabled{};
    std::optional<NetworkDetails> network_details{};
    std::optional<bool> is_stored_credential{};
    std::optional<MitCategoryValue> mit_category{};
    std::optional<BillingDescriptorValue> billing_descriptor{};
    std::optional<bool> is_account_funded_transaction{};
    std::optional<MaskedRecipientDetailsValue> recipient_details{};
    std::optional<TokenizationValue> tokenization{};
    std::optional<PartnerMerchantIdentifierDetailsValue> partner_merchant_identifier_details{};
    std::optional<PaymentMethodTokenizationDetails> payment_method_tokenization_details{};
    std::optional<std::vector<InstallmentOptionValue>> installment_options{};
    std::optional<InstallmentDataValue> installment_data{};
    std::optional<std::string> sender_payment_instrument_id{};
    std::optional<bool> enable_avs_check{};
    std::optional<bool> enable_account_name_verification{};
};

class GpaySessionTokenData : public core::behavior::Serializer<GpaySessionTokenData>
{
public:
    void set_google_pay(GooglePayDetailsWrapper value)
    {
        google_pay = std::move(value);
    }

    [[nodiscard]] const GooglePayDetailsWrapper& get_google_pay() const noexcept
    {
        return google_pay;
    }

    GooglePayDetailsWrapper google_pay{};
};

class PaymentListResponseV2 : public core::behavior::Serializer<PaymentListResponseV2>
{
public:
    void set_count(std::size_t value)
    {
        count = std::move(value);
    }

    [[nodiscard]] const std::size_t& get_count() const noexcept
    {
        return count;
    }

    void set_total_count(std::int64_t value)
    {
        total_count = std::move(value);
    }

    [[nodiscard]] const std::int64_t& get_total_count() const noexcept
    {
        return total_count;
    }

    void set_data(std::vector<PaymentsResponse> value)
    {
        data = std::move(value);
    }

    [[nodiscard]] const std::vector<PaymentsResponse>& get_data() const noexcept
    {
        return data;
    }

    std::size_t count{};
    std::int64_t total_count{};
    std::vector<PaymentsResponse> data{};
};

// ─── Dedicated payments list/query DTOs (parity SR-6) ────────────────────────
// These are the typed, deserializable query/list shapes used by the payments
// handlers instead of decoding query strings into the oversized PaymentsRequest
// body DTO. Field names mirror the upstream `web::Query<T>` / `web::Path<T>`
// structs in docs/hyperswitch/crates/router/src/routes/payments.rs.

/** Typed query for the v1/v2 payments list families (`payments_list`,
 *  `profile_payments_list`, `payments_list_by_filter`, `payments_list_for_platform`). */
class PaymentsListRequest : public core::behavior::Serializer<PaymentsListRequest>
{
public:
    void set_merchant_id(std::optional<std::string> value)
    {
        merchant_id = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_merchant_id() const noexcept
    {
        return merchant_id;
    }

    void set_profile_id(std::optional<std::string> value)
    {
        profile_id = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_profile_id() const noexcept
    {
        return profile_id;
    }

    void set_status(std::optional<std::string> value)
    {
        status = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_status() const noexcept
    {
        return status;
    }

    void set_connector(std::optional<std::string> value)
    {
        connector = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_connector() const noexcept
    {
        return connector;
    }

    void set_payment_method(std::optional<std::string> value)
    {
        payment_method = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_payment_method() const noexcept
    {
        return payment_method;
    }

    void set_payment_method_type(std::optional<std::string> value)
    {
        payment_method_type = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_payment_method_type() const noexcept
    {
        return payment_method_type;
    }

    void set_offset(std::optional<std::int64_t> value)
    {
        offset = std::move(value);
    }

    [[nodiscard]] const std::optional<std::int64_t>& get_offset() const noexcept
    {
        return offset;
    }

    void set_limit(std::optional<std::int64_t> value)
    {
        limit = std::move(value);
    }

    [[nodiscard]] const std::optional<std::int64_t>& get_limit() const noexcept
    {
        return limit;
    }

    void set_force_sync(std::optional<bool> value)
    {
        force_sync = std::move(value);
    }

    [[nodiscard]] const std::optional<bool>& get_force_sync() const noexcept
    {
        return force_sync;
    }

    void set_expand_attempts(std::optional<bool> value)
    {
        expand_attempts = std::move(value);
    }

    [[nodiscard]] const std::optional<bool>& get_expand_attempts() const noexcept
    {
        return expand_attempts;
    }

    std::optional<std::string> merchant_id{};
    std::optional<std::string> profile_id{};
    std::optional<std::string> status{};
    std::optional<std::string> connector{};
    std::optional<std::string> payment_method{};
    std::optional<std::string> payment_method_type{};
    std::optional<std::int64_t> offset{};
    std::optional<std::int64_t> limit{};
    std::optional<bool> force_sync{};
    std::optional<bool> expand_attempts{};
};

/** Typed query for the filter endpoints (`get_payment_filters`,
 *  `get_payment_filters_profile`, `get_filters_for_payments`). */
class PaymentsFilterRequest : public core::behavior::Serializer<PaymentsFilterRequest>
{
public:
    void set_status(std::optional<std::string> value)
    {
        status = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_status() const noexcept
    {
        return status;
    }

    void set_connector(std::optional<std::string> value)
    {
        connector = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_connector() const noexcept
    {
        return connector;
    }

    void set_payment_method(std::optional<std::string> value)
    {
        payment_method = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_payment_method() const noexcept
    {
        return payment_method;
    }

    void set_payment_method_type(std::optional<std::string> value)
    {
        payment_method_type = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_payment_method_type() const noexcept
    {
        return payment_method_type;
    }

    void set_merchant_reference_id(std::optional<std::string> value)
    {
        merchant_reference_id = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_merchant_reference_id() const noexcept
    {
        return merchant_reference_id;
    }

    void set_profile_id(std::optional<std::string> value)
    {
        profile_id = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_profile_id() const noexcept
    {
        return profile_id;
    }

    void set_offset(std::optional<std::int64_t> value)
    {
        offset = std::move(value);
    }

    [[nodiscard]] const std::optional<std::int64_t>& get_offset() const noexcept
    {
        return offset;
    }

    void set_limit(std::optional<std::int64_t> value)
    {
        limit = std::move(value);
    }

    [[nodiscard]] const std::optional<std::int64_t>& get_limit() const noexcept
    {
        return limit;
    }

    std::optional<std::string> status{};
    std::optional<std::string> connector{};
    std::optional<std::string> payment_method{};
    std::optional<std::string> payment_method_type{};
    std::optional<std::string> merchant_reference_id{};
    std::optional<std::string> profile_id{};
    std::optional<std::int64_t> offset{};
    std::optional<std::int64_t> limit{};
};

/** Typed query for the aggregate endpoints (`get_payments_aggregates{,_profile}`). */
class PaymentsAggregateRequest : public core::behavior::Serializer<PaymentsAggregateRequest>
{
public:
    void set_start(std::optional<std::string> value)
    {
        start = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_start() const noexcept
    {
        return start;
    }

    void set_end(std::optional<std::string> value)
    {
        end = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_end() const noexcept
    {
        return end;
    }

    void set_status(std::optional<std::string> value)
    {
        status = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_status() const noexcept
    {
        return status;
    }

    void set_profile_id(std::optional<std::string> value)
    {
        profile_id = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_profile_id() const noexcept
    {
        return profile_id;
    }

    std::optional<std::string> start{};
    std::optional<std::string> end{};
    std::optional<std::string> status{};
    std::optional<std::string> profile_id{};
};

/** Typed query for `list_payment_attempts` (typed attempt-list fields, SR-6). */
class PaymentAttemptListFilterRequest :
    public core::behavior::Serializer<PaymentAttemptListFilterRequest>
{
public:
    void set_payment_id(std::optional<std::string> value)
    {
        payment_id = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_payment_id() const noexcept
    {
        return payment_id;
    }

    void set_status(std::optional<std::string> value)
    {
        status = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_status() const noexcept
    {
        return status;
    }

    void set_connector(std::optional<std::string> value)
    {
        connector = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_connector() const noexcept
    {
        return connector;
    }

    void set_offset(std::optional<std::int64_t> value)
    {
        offset = std::move(value);
    }

    [[nodiscard]] const std::optional<std::int64_t>& get_offset() const noexcept
    {
        return offset;
    }

    void set_limit(std::optional<std::int64_t> value)
    {
        limit = std::move(value);
    }

    [[nodiscard]] const std::optional<std::int64_t>& get_limit() const noexcept
    {
        return limit;
    }

    std::optional<std::string> payment_id{};
    std::optional<std::string> status{};
    std::optional<std::string> connector{};
    std::optional<std::int64_t> offset{};
    std::optional<std::int64_t> limit{};
};

/** Typed body for the OpenSearch advanced-list routes. Mirrors upstream
 *  `PaymentListFilterConstraints` (api_models/src/payments.rs:9888-9951); every
 *  field is optional so a sparse filter body round-trips without synthetic
 *  defaults. */
class PaymentsAdvancedListRequest : public core::behavior::Serializer<PaymentsAdvancedListRequest>
{
public:
    void set_query(std::optional<std::string> value)
    {
        query = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_query() const noexcept
    {
        return query;
    }

    void set_payment_id(std::optional<std::string> value)
    {
        payment_id = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_payment_id() const noexcept
    {
        return payment_id;
    }

    void set_profile_id(std::optional<std::string> value)
    {
        profile_id = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_profile_id() const noexcept
    {
        return profile_id;
    }

    void set_customer_id(std::optional<std::string> value)
    {
        customer_id = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_customer_id() const noexcept
    {
        return customer_id;
    }

    void set_limit(std::optional<std::int64_t> value)
    {
        limit = std::move(value);
    }

    [[nodiscard]] const std::optional<std::int64_t>& get_limit() const noexcept
    {
        return limit;
    }

    void set_offset(std::optional<std::int64_t> value)
    {
        offset = std::move(value);
    }

    [[nodiscard]] const std::optional<std::int64_t>& get_offset() const noexcept
    {
        return offset;
    }

    void set_connector(std::optional<std::vector<std::string>> value)
    {
        connector = std::move(value);
    }

    [[nodiscard]] const std::optional<std::vector<std::string>>& get_connector() const noexcept
    {
        return connector;
    }

    void set_currency(std::optional<std::vector<std::string>> value)
    {
        currency = std::move(value);
    }

    [[nodiscard]] const std::optional<std::vector<std::string>>& get_currency() const noexcept
    {
        return currency;
    }

    void set_status(std::optional<std::vector<std::string>> value)
    {
        status = std::move(value);
    }

    [[nodiscard]] const std::optional<std::vector<std::string>>& get_status() const noexcept
    {
        return status;
    }

    void set_payment_method(std::optional<std::vector<std::string>> value)
    {
        payment_method = std::move(value);
    }

    [[nodiscard]] const std::optional<std::vector<std::string>>& get_payment_method() const noexcept
    {
        return payment_method;
    }

    void set_payment_method_type(std::optional<std::vector<std::string>> value)
    {
        payment_method_type = std::move(value);
    }

    [[nodiscard]] const std::optional<std::vector<std::string>>&
    get_payment_method_type() const noexcept
    {
        return payment_method_type;
    }

    void set_order(std::optional<std::string> value)
    {
        order = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_order() const noexcept
    {
        return order;
    }

    std::optional<std::string> query{};
    std::optional<std::string> payment_id{};
    std::optional<std::string> profile_id{};
    std::optional<std::string> customer_id{};
    std::optional<std::int64_t> limit{};
    std::optional<std::int64_t> offset{};
    std::optional<std::vector<std::string>> connector{};
    std::optional<std::vector<std::string>> currency{};
    std::optional<std::vector<std::string>> status{};
    std::optional<std::vector<std::string>> payment_method{};
    std::optional<std::vector<std::string>> payment_method_type{};
    std::optional<std::string> order{};
};

/** Typed response for the OpenSearch advanced-list routes. Mirrors the upstream
 *  `SearchResponse` envelope (`count` + `hits`) with typed payment rows. */
class PaymentsAdvancedListResponse : public core::behavior::Serializer<PaymentsAdvancedListResponse>
{
public:
    void set_count(std::size_t value)
    {
        count = std::move(value);
    }

    [[nodiscard]] const std::size_t& get_count() const noexcept
    {
        return count;
    }

    void set_total_count(std::int64_t value)
    {
        total_count = std::move(value);
    }

    [[nodiscard]] const std::int64_t& get_total_count() const noexcept
    {
        return total_count;
    }

    void set_data(std::vector<PaymentsResponse> value)
    {
        data = std::move(value);
    }

    [[nodiscard]] const std::vector<PaymentsResponse>& get_data() const noexcept
    {
        return data;
    }

    std::size_t count{};
    std::int64_t total_count{};
    std::vector<PaymentsResponse> data{};
};

/** Typed row of the `/payments/{id}/client` payment-methods listing. */
class PaymentsClientMethod : public core::behavior::Serializer<PaymentsClientMethod>
{
public:
    void set_payment_method(std::optional<std::string> value)
    {
        payment_method = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_payment_method() const noexcept
    {
        return payment_method;
    }

    void set_payment_method_type(std::optional<std::string> value)
    {
        payment_method_type = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_payment_method_type() const noexcept
    {
        return payment_method_type;
    }

    void set_connector(std::optional<std::string> value)
    {
        connector = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_connector() const noexcept
    {
        return connector;
    }

    void set_enabled(std::optional<bool> value)
    {
        enabled = std::move(value);
    }

    [[nodiscard]] const std::optional<bool>& get_enabled() const noexcept
    {
        return enabled;
    }

    std::optional<std::string> payment_method{};
    std::optional<std::string> payment_method_type{};
    std::optional<std::string> connector{};
    std::optional<bool> enabled{};
};

/** Typed response for `list_payment_methods_for_payments_client`. */
class PaymentsClientMethodsResponse :
    public core::behavior::Serializer<PaymentsClientMethodsResponse>
{
public:
    void set_count(std::size_t value)
    {
        count = std::move(value);
    }

    [[nodiscard]] const std::size_t& get_count() const noexcept
    {
        return count;
    }

    void set_data(std::vector<PaymentsClientMethod> value)
    {
        data = std::move(value);
    }

    [[nodiscard]] const std::vector<PaymentsClientMethod>& get_data() const noexcept
    {
        return data;
    }

    std::size_t count{};
    std::vector<PaymentsClientMethod> data{};
};

} // namespace app::model::payments

template<>
struct serde::Serializable<app::model::payments::CustomerDetails>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "id",
                &app::model::payments::CustomerDetails::get_id,
                &app::model::payments::CustomerDetails::set_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "name",
                &app::model::payments::CustomerDetails::get_name,
                &app::model::payments::CustomerDetails::set_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "email",
                &app::model::payments::CustomerDetails::get_email,
                &app::model::payments::CustomerDetails::set_email,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "phone",
                &app::model::payments::CustomerDetails::get_phone,
                &app::model::payments::CustomerDetails::set_phone,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "phone_country_code",
                &app::model::payments::CustomerDetails::get_phone_country_code,
                &app::model::payments::CustomerDetails::set_phone_country_code,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "tax_registration_id",
                &app::model::payments::CustomerDetails::get_tax_registration_id,
                &app::model::payments::CustomerDetails::set_tax_registration_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "document_details",
                &app::model::payments::CustomerDetails::get_document_details,
                &app::model::payments::CustomerDetails::set_document_details,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "date_of_birth",
                &app::model::payments::CustomerDetails::get_date_of_birth,
                &app::model::payments::CustomerDetails::set_date_of_birth,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "customer_details";
    }
};

template<>
struct serde::Serializable<app::model::payments::CustomerDetailsResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "id",
                &app::model::payments::CustomerDetailsResponse::get_id,
                &app::model::payments::CustomerDetailsResponse::set_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "name",
                &app::model::payments::CustomerDetailsResponse::get_name,
                &app::model::payments::CustomerDetailsResponse::set_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "email",
                &app::model::payments::CustomerDetailsResponse::get_email,
                &app::model::payments::CustomerDetailsResponse::set_email,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "phone",
                &app::model::payments::CustomerDetailsResponse::get_phone,
                &app::model::payments::CustomerDetailsResponse::set_phone,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "phone_country_code",
                &app::model::payments::CustomerDetailsResponse::get_phone_country_code,
                &app::model::payments::CustomerDetailsResponse::set_phone_country_code,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "customer_document_details",
                &app::model::payments::CustomerDetailsResponse::get_customer_document_details,
                &app::model::payments::CustomerDetailsResponse::set_customer_document_details,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "customer_details_response";
    }
};

template<>
struct serde::Serializable<app::model::payments::PaymentsCreateIntentRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "amount_details",
                &app::model::payments::PaymentsCreateIntentRequest::get_amount_details,
                &app::model::payments::PaymentsCreateIntentRequest::set_amount_details,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "merchant_reference_id",
                &app::model::payments::PaymentsCreateIntentRequest::get_merchant_reference_id,
                &app::model::payments::PaymentsCreateIntentRequest::set_merchant_reference_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "routing_algorithm_id",
                &app::model::payments::PaymentsCreateIntentRequest::get_routing_algorithm_id,
                &app::model::payments::PaymentsCreateIntentRequest::set_routing_algorithm_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "capture_method",
                &app::model::payments::PaymentsCreateIntentRequest::get_capture_method,
                &app::model::payments::PaymentsCreateIntentRequest::set_capture_method,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "authentication_type",
                &app::model::payments::PaymentsCreateIntentRequest::get_authentication_type,
                &app::model::payments::PaymentsCreateIntentRequest::set_authentication_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "billing",
                &app::model::payments::PaymentsCreateIntentRequest::get_billing,
                &app::model::payments::PaymentsCreateIntentRequest::set_billing,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "customer_details",
                &app::model::payments::PaymentsCreateIntentRequest::get_customer_details,
                &app::model::payments::PaymentsCreateIntentRequest::set_customer_details,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "shipping",
                &app::model::payments::PaymentsCreateIntentRequest::get_shipping,
                &app::model::payments::PaymentsCreateIntentRequest::set_shipping,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "customer_id",
                &app::model::payments::PaymentsCreateIntentRequest::get_customer_id,
                &app::model::payments::PaymentsCreateIntentRequest::set_customer_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "customer_present",
                &app::model::payments::PaymentsCreateIntentRequest::get_customer_present,
                &app::model::payments::PaymentsCreateIntentRequest::set_customer_present,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "description",
                &app::model::payments::PaymentsCreateIntentRequest::get_description,
                &app::model::payments::PaymentsCreateIntentRequest::set_description,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "return_url",
                &app::model::payments::PaymentsCreateIntentRequest::get_return_url,
                &app::model::payments::PaymentsCreateIntentRequest::set_return_url,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "setup_future_usage",
                &app::model::payments::PaymentsCreateIntentRequest::get_setup_future_usage,
                &app::model::payments::PaymentsCreateIntentRequest::set_setup_future_usage,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "apply_mit_exemption",
                &app::model::payments::PaymentsCreateIntentRequest::get_apply_mit_exemption,
                &app::model::payments::PaymentsCreateIntentRequest::set_apply_mit_exemption,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "statement_descriptor",
                &app::model::payments::PaymentsCreateIntentRequest::get_statement_descriptor,
                &app::model::payments::PaymentsCreateIntentRequest::set_statement_descriptor,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "payments_create_intent_request";
    }
};

template<>
struct serde::Serializable<app::model::payments::PaymentAttemptListRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<
            "payment_intent_id",
            &app::model::payments::PaymentAttemptListRequest::get_payment_intent_id,
            &app::model::payments::PaymentAttemptListRequest::set_payment_intent_id,
            serde::FieldOptions{}>{}};
    }

    static constexpr std::string_view table_name()
    {
        return "payment_attempt_list_request";
    }
};

template<>
struct serde::Serializable<app::model::payments::PaymentAttemptListResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<
            "payment_attempt_list",
            &app::model::payments::PaymentAttemptListResponse::get_payment_attempt_list,
            &app::model::payments::PaymentAttemptListResponse::set_payment_attempt_list,
            serde::FieldOptions{}>{}};
    }

    static constexpr std::string_view table_name()
    {
        return "payment_attempt_list_response";
    }
};

template<>
struct serde::Serializable<app::model::payments::PaymentsGetIntentRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<
            "id",
            &app::model::payments::PaymentsGetIntentRequest::get_id,
            &app::model::payments::PaymentsGetIntentRequest::set_id,
            serde::FieldOptions{}>{}};
    }

    static constexpr std::string_view table_name()
    {
        return "payments_get_intent_request";
    }
};

template<>
struct serde::Serializable<app::model::payments::PaymentsUpdateIntentRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "amount_details",
                &app::model::payments::PaymentsUpdateIntentRequest::get_amount_details,
                &app::model::payments::PaymentsUpdateIntentRequest::set_amount_details,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "routing_algorithm_id",
                &app::model::payments::PaymentsUpdateIntentRequest::get_routing_algorithm_id,
                &app::model::payments::PaymentsUpdateIntentRequest::set_routing_algorithm_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "capture_method",
                &app::model::payments::PaymentsUpdateIntentRequest::get_capture_method,
                &app::model::payments::PaymentsUpdateIntentRequest::set_capture_method,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "authentication_type",
                &app::model::payments::PaymentsUpdateIntentRequest::get_authentication_type,
                &app::model::payments::PaymentsUpdateIntentRequest::set_authentication_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "billing",
                &app::model::payments::PaymentsUpdateIntentRequest::get_billing,
                &app::model::payments::PaymentsUpdateIntentRequest::set_billing,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "shipping",
                &app::model::payments::PaymentsUpdateIntentRequest::get_shipping,
                &app::model::payments::PaymentsUpdateIntentRequest::set_shipping,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "customer_present",
                &app::model::payments::PaymentsUpdateIntentRequest::get_customer_present,
                &app::model::payments::PaymentsUpdateIntentRequest::set_customer_present,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "description",
                &app::model::payments::PaymentsUpdateIntentRequest::get_description,
                &app::model::payments::PaymentsUpdateIntentRequest::set_description,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "return_url",
                &app::model::payments::PaymentsUpdateIntentRequest::get_return_url,
                &app::model::payments::PaymentsUpdateIntentRequest::set_return_url,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "setup_future_usage",
                &app::model::payments::PaymentsUpdateIntentRequest::get_setup_future_usage,
                &app::model::payments::PaymentsUpdateIntentRequest::set_setup_future_usage,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "apply_mit_exemption",
                &app::model::payments::PaymentsUpdateIntentRequest::get_apply_mit_exemption,
                &app::model::payments::PaymentsUpdateIntentRequest::set_apply_mit_exemption,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "statement_descriptor",
                &app::model::payments::PaymentsUpdateIntentRequest::get_statement_descriptor,
                &app::model::payments::PaymentsUpdateIntentRequest::set_statement_descriptor,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "payments_update_intent_request";
    }
};

template<>
struct serde::Serializable<app::model::payments::PaymentsIntentResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "id",
                &app::model::payments::PaymentsIntentResponse::get_id,
                &app::model::payments::PaymentsIntentResponse::set_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "status",
                &app::model::payments::PaymentsIntentResponse::get_status,
                &app::model::payments::PaymentsIntentResponse::set_status,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "amount_details",
                &app::model::payments::PaymentsIntentResponse::get_amount_details,
                &app::model::payments::PaymentsIntentResponse::set_amount_details,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "client_secret",
                &app::model::payments::PaymentsIntentResponse::get_client_secret,
                &app::model::payments::PaymentsIntentResponse::set_client_secret,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "profile_id",
                &app::model::payments::PaymentsIntentResponse::get_profile_id,
                &app::model::payments::PaymentsIntentResponse::set_profile_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "merchant_reference_id",
                &app::model::payments::PaymentsIntentResponse::get_merchant_reference_id,
                &app::model::payments::PaymentsIntentResponse::set_merchant_reference_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "routing_algorithm_id",
                &app::model::payments::PaymentsIntentResponse::get_routing_algorithm_id,
                &app::model::payments::PaymentsIntentResponse::set_routing_algorithm_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "capture_method",
                &app::model::payments::PaymentsIntentResponse::get_capture_method,
                &app::model::payments::PaymentsIntentResponse::set_capture_method,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "authentication_type",
                &app::model::payments::PaymentsIntentResponse::get_authentication_type,
                &app::model::payments::PaymentsIntentResponse::set_authentication_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "billing",
                &app::model::payments::PaymentsIntentResponse::get_billing,
                &app::model::payments::PaymentsIntentResponse::set_billing,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "shipping",
                &app::model::payments::PaymentsIntentResponse::get_shipping,
                &app::model::payments::PaymentsIntentResponse::set_shipping,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "customer_id",
                &app::model::payments::PaymentsIntentResponse::get_customer_id,
                &app::model::payments::PaymentsIntentResponse::set_customer_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "customer_present",
                &app::model::payments::PaymentsIntentResponse::get_customer_present,
                &app::model::payments::PaymentsIntentResponse::set_customer_present,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "description",
                &app::model::payments::PaymentsIntentResponse::get_description,
                &app::model::payments::PaymentsIntentResponse::set_description,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "return_url",
                &app::model::payments::PaymentsIntentResponse::get_return_url,
                &app::model::payments::PaymentsIntentResponse::set_return_url,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "setup_future_usage",
                &app::model::payments::PaymentsIntentResponse::get_setup_future_usage,
                &app::model::payments::PaymentsIntentResponse::set_setup_future_usage,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "apply_mit_exemption",
                &app::model::payments::PaymentsIntentResponse::get_apply_mit_exemption,
                &app::model::payments::PaymentsIntentResponse::set_apply_mit_exemption,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "statement_descriptor",
                &app::model::payments::PaymentsIntentResponse::get_statement_descriptor,
                &app::model::payments::PaymentsIntentResponse::set_statement_descriptor,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "payments_intent_response";
    }
};

template<>
struct serde::Serializable<app::model::payments::RevenueRecoveryGetIntentResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "id",
                &app::model::payments::RevenueRecoveryGetIntentResponse::get_id,
                &app::model::payments::RevenueRecoveryGetIntentResponse::set_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "status",
                &app::model::payments::RevenueRecoveryGetIntentResponse::get_status,
                &app::model::payments::RevenueRecoveryGetIntentResponse::set_status,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "next_retry",
                &app::model::payments::RevenueRecoveryGetIntentResponse::get_next_retry,
                &app::model::payments::RevenueRecoveryGetIntentResponse::set_next_retry,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "amount_details",
                &app::model::payments::RevenueRecoveryGetIntentResponse::get_amount_details,
                &app::model::payments::RevenueRecoveryGetIntentResponse::set_amount_details,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "client_secret",
                &app::model::payments::RevenueRecoveryGetIntentResponse::get_client_secret,
                &app::model::payments::RevenueRecoveryGetIntentResponse::set_client_secret,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "profile_id",
                &app::model::payments::RevenueRecoveryGetIntentResponse::get_profile_id,
                &app::model::payments::RevenueRecoveryGetIntentResponse::set_profile_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "merchant_reference_id",
                &app::model::payments::RevenueRecoveryGetIntentResponse::get_merchant_reference_id,
                &app::model::payments::RevenueRecoveryGetIntentResponse::set_merchant_reference_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "routing_algorithm_id",
                &app::model::payments::RevenueRecoveryGetIntentResponse::get_routing_algorithm_id,
                &app::model::payments::RevenueRecoveryGetIntentResponse::set_routing_algorithm_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "capture_method",
                &app::model::payments::RevenueRecoveryGetIntentResponse::get_capture_method,
                &app::model::payments::RevenueRecoveryGetIntentResponse::set_capture_method,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "authentication_type",
                &app::model::payments::RevenueRecoveryGetIntentResponse::get_authentication_type,
                &app::model::payments::RevenueRecoveryGetIntentResponse::set_authentication_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "billing",
                &app::model::payments::RevenueRecoveryGetIntentResponse::get_billing,
                &app::model::payments::RevenueRecoveryGetIntentResponse::set_billing,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "shipping",
                &app::model::payments::RevenueRecoveryGetIntentResponse::get_shipping,
                &app::model::payments::RevenueRecoveryGetIntentResponse::set_shipping,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "customer_id",
                &app::model::payments::RevenueRecoveryGetIntentResponse::get_customer_id,
                &app::model::payments::RevenueRecoveryGetIntentResponse::set_customer_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "customer_present",
                &app::model::payments::RevenueRecoveryGetIntentResponse::get_customer_present,
                &app::model::payments::RevenueRecoveryGetIntentResponse::set_customer_present,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "description",
                &app::model::payments::RevenueRecoveryGetIntentResponse::get_description,
                &app::model::payments::RevenueRecoveryGetIntentResponse::set_description,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "return_url",
                &app::model::payments::RevenueRecoveryGetIntentResponse::get_return_url,
                &app::model::payments::RevenueRecoveryGetIntentResponse::set_return_url,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "setup_future_usage",
                &app::model::payments::RevenueRecoveryGetIntentResponse::get_setup_future_usage,
                &app::model::payments::RevenueRecoveryGetIntentResponse::set_setup_future_usage,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "apply_mit_exemption",
                &app::model::payments::RevenueRecoveryGetIntentResponse::get_apply_mit_exemption,
                &app::model::payments::RevenueRecoveryGetIntentResponse::set_apply_mit_exemption,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "statement_descriptor",
                &app::model::payments::RevenueRecoveryGetIntentResponse::get_statement_descriptor,
                &app::model::payments::RevenueRecoveryGetIntentResponse::set_statement_descriptor,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "revenue_recovery_get_intent_response";
    }
};

template<>
struct serde::Serializable<app::model::payments::AmountDetails>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "order_amount",
                &app::model::payments::AmountDetails::get_order_amount,
                &app::model::payments::AmountDetails::set_order_amount,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "currency",
                &app::model::payments::AmountDetails::get_currency,
                &app::model::payments::AmountDetails::set_currency,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "shipping_cost",
                &app::model::payments::AmountDetails::get_shipping_cost,
                &app::model::payments::AmountDetails::set_shipping_cost,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "order_tax_amount",
                &app::model::payments::AmountDetails::get_order_tax_amount,
                &app::model::payments::AmountDetails::set_order_tax_amount,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "skip_external_tax_calculation",
                &app::model::payments::AmountDetails::get_skip_external_tax_calculation,
                &app::model::payments::AmountDetails::set_skip_external_tax_calculation,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "skip_surcharge_calculation",
                &app::model::payments::AmountDetails::get_skip_surcharge_calculation,
                &app::model::payments::AmountDetails::set_skip_surcharge_calculation,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "surcharge_amount",
                &app::model::payments::AmountDetails::get_surcharge_amount,
                &app::model::payments::AmountDetails::set_surcharge_amount,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "tax_on_surcharge",
                &app::model::payments::AmountDetails::get_tax_on_surcharge,
                &app::model::payments::AmountDetails::set_tax_on_surcharge,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "amount_details";
    }
};

template<>
struct serde::Serializable<app::model::payments::AmountDetailsUpdate>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "order_amount",
                &app::model::payments::AmountDetailsUpdate::get_order_amount,
                &app::model::payments::AmountDetailsUpdate::set_order_amount,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "currency",
                &app::model::payments::AmountDetailsUpdate::get_currency,
                &app::model::payments::AmountDetailsUpdate::set_currency,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "shipping_cost",
                &app::model::payments::AmountDetailsUpdate::get_shipping_cost,
                &app::model::payments::AmountDetailsUpdate::set_shipping_cost,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "order_tax_amount",
                &app::model::payments::AmountDetailsUpdate::get_order_tax_amount,
                &app::model::payments::AmountDetailsUpdate::set_order_tax_amount,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "skip_external_tax_calculation",
                &app::model::payments::AmountDetailsUpdate::get_skip_external_tax_calculation,
                &app::model::payments::AmountDetailsUpdate::set_skip_external_tax_calculation,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "skip_surcharge_calculation",
                &app::model::payments::AmountDetailsUpdate::get_skip_surcharge_calculation,
                &app::model::payments::AmountDetailsUpdate::set_skip_surcharge_calculation,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "surcharge_amount",
                &app::model::payments::AmountDetailsUpdate::get_surcharge_amount,
                &app::model::payments::AmountDetailsUpdate::set_surcharge_amount,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "tax_on_surcharge",
                &app::model::payments::AmountDetailsUpdate::get_tax_on_surcharge,
                &app::model::payments::AmountDetailsUpdate::set_tax_on_surcharge,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "amount_details_update";
    }
};

template<>
struct serde::Serializable<app::model::payments::AmountDetailsSetter>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "order_amount",
                &app::model::payments::AmountDetailsSetter::get_order_amount,
                &app::model::payments::AmountDetailsSetter::set_order_amount,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "currency",
                &app::model::payments::AmountDetailsSetter::get_currency,
                &app::model::payments::AmountDetailsSetter::set_currency,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "shipping_cost",
                &app::model::payments::AmountDetailsSetter::get_shipping_cost,
                &app::model::payments::AmountDetailsSetter::set_shipping_cost,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "order_tax_amount",
                &app::model::payments::AmountDetailsSetter::get_order_tax_amount,
                &app::model::payments::AmountDetailsSetter::set_order_tax_amount,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "skip_external_tax_calculation",
                &app::model::payments::AmountDetailsSetter::get_skip_external_tax_calculation,
                &app::model::payments::AmountDetailsSetter::set_skip_external_tax_calculation,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "skip_surcharge_calculation",
                &app::model::payments::AmountDetailsSetter::get_skip_surcharge_calculation,
                &app::model::payments::AmountDetailsSetter::set_skip_surcharge_calculation,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "surcharge_amount",
                &app::model::payments::AmountDetailsSetter::get_surcharge_amount,
                &app::model::payments::AmountDetailsSetter::set_surcharge_amount,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "tax_on_surcharge",
                &app::model::payments::AmountDetailsSetter::get_tax_on_surcharge,
                &app::model::payments::AmountDetailsSetter::set_tax_on_surcharge,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "amount_details_setter";
    }
};

template<>
struct serde::Serializable<app::model::payments::AmountDetailsResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "order_amount",
                &app::model::payments::AmountDetailsResponse::get_order_amount,
                &app::model::payments::AmountDetailsResponse::set_order_amount,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "currency",
                &app::model::payments::AmountDetailsResponse::get_currency,
                &app::model::payments::AmountDetailsResponse::set_currency,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "shipping_cost",
                &app::model::payments::AmountDetailsResponse::get_shipping_cost,
                &app::model::payments::AmountDetailsResponse::set_shipping_cost,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "order_tax_amount",
                &app::model::payments::AmountDetailsResponse::get_order_tax_amount,
                &app::model::payments::AmountDetailsResponse::set_order_tax_amount,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "external_tax_calculation",
                &app::model::payments::AmountDetailsResponse::get_external_tax_calculation,
                &app::model::payments::AmountDetailsResponse::set_external_tax_calculation,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "surcharge_calculation",
                &app::model::payments::AmountDetailsResponse::get_surcharge_calculation,
                &app::model::payments::AmountDetailsResponse::set_surcharge_calculation,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "surcharge_amount",
                &app::model::payments::AmountDetailsResponse::get_surcharge_amount,
                &app::model::payments::AmountDetailsResponse::set_surcharge_amount,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "tax_on_surcharge",
                &app::model::payments::AmountDetailsResponse::get_tax_on_surcharge,
                &app::model::payments::AmountDetailsResponse::set_tax_on_surcharge,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "amount_captured",
                &app::model::payments::AmountDetailsResponse::get_amount_captured,
                &app::model::payments::AmountDetailsResponse::set_amount_captured,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "amount_details_response";
    }
};

template<>
struct serde::Serializable<app::model::payments::PaymentAmountDetailsResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "order_amount",
                &app::model::payments::PaymentAmountDetailsResponse::get_order_amount,
                &app::model::payments::PaymentAmountDetailsResponse::set_order_amount,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "currency",
                &app::model::payments::PaymentAmountDetailsResponse::get_currency,
                &app::model::payments::PaymentAmountDetailsResponse::set_currency,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "shipping_cost",
                &app::model::payments::PaymentAmountDetailsResponse::get_shipping_cost,
                &app::model::payments::PaymentAmountDetailsResponse::set_shipping_cost,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "order_tax_amount",
                &app::model::payments::PaymentAmountDetailsResponse::get_order_tax_amount,
                &app::model::payments::PaymentAmountDetailsResponse::set_order_tax_amount,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "external_tax_calculation",
                &app::model::payments::PaymentAmountDetailsResponse::get_external_tax_calculation,
                &app::model::payments::PaymentAmountDetailsResponse::set_external_tax_calculation,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "surcharge_calculation",
                &app::model::payments::PaymentAmountDetailsResponse::get_surcharge_calculation,
                &app::model::payments::PaymentAmountDetailsResponse::set_surcharge_calculation,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "surcharge_amount",
                &app::model::payments::PaymentAmountDetailsResponse::get_surcharge_amount,
                &app::model::payments::PaymentAmountDetailsResponse::set_surcharge_amount,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "tax_on_surcharge",
                &app::model::payments::PaymentAmountDetailsResponse::get_tax_on_surcharge,
                &app::model::payments::PaymentAmountDetailsResponse::set_tax_on_surcharge,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "net_amount",
                &app::model::payments::PaymentAmountDetailsResponse::get_net_amount,
                &app::model::payments::PaymentAmountDetailsResponse::set_net_amount,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "amount_to_capture",
                &app::model::payments::PaymentAmountDetailsResponse::get_amount_to_capture,
                &app::model::payments::PaymentAmountDetailsResponse::set_amount_to_capture,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "amount_capturable",
                &app::model::payments::PaymentAmountDetailsResponse::get_amount_capturable,
                &app::model::payments::PaymentAmountDetailsResponse::set_amount_capturable,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "amount_captured",
                &app::model::payments::PaymentAmountDetailsResponse::get_amount_captured,
                &app::model::payments::PaymentAmountDetailsResponse::set_amount_captured,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "payment_amount_details_response";
    }
};

template<>
struct serde::Serializable<app::model::payments::PaymentAttemptAmountDetails>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "net_amount",
                &app::model::payments::PaymentAttemptAmountDetails::get_net_amount,
                &app::model::payments::PaymentAttemptAmountDetails::set_net_amount,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "amount_to_capture",
                &app::model::payments::PaymentAttemptAmountDetails::get_amount_to_capture,
                &app::model::payments::PaymentAttemptAmountDetails::set_amount_to_capture,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "surcharge_amount",
                &app::model::payments::PaymentAttemptAmountDetails::get_surcharge_amount,
                &app::model::payments::PaymentAttemptAmountDetails::set_surcharge_amount,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "tax_on_surcharge",
                &app::model::payments::PaymentAttemptAmountDetails::get_tax_on_surcharge,
                &app::model::payments::PaymentAttemptAmountDetails::set_tax_on_surcharge,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "amount_capturable",
                &app::model::payments::PaymentAttemptAmountDetails::get_amount_capturable,
                &app::model::payments::PaymentAttemptAmountDetails::set_amount_capturable,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "shipping_cost",
                &app::model::payments::PaymentAttemptAmountDetails::get_shipping_cost,
                &app::model::payments::PaymentAttemptAmountDetails::set_shipping_cost,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "order_tax_amount",
                &app::model::payments::PaymentAttemptAmountDetails::get_order_tax_amount,
                &app::model::payments::PaymentAttemptAmountDetails::set_order_tax_amount,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "amount_captured",
                &app::model::payments::PaymentAttemptAmountDetails::get_amount_captured,
                &app::model::payments::PaymentAttemptAmountDetails::set_amount_captured,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "payment_attempt_amount_details";
    }
};

template<>
struct serde::Serializable<app::model::payments::InstallmentRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "number_of_installments",
                &app::model::payments::InstallmentRequest::get_number_of_installments,
                &app::model::payments::InstallmentRequest::set_number_of_installments,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "billing_frequency",
                &app::model::payments::InstallmentRequest::get_billing_frequency,
                &app::model::payments::InstallmentRequest::set_billing_frequency,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "installment_request";
    }
};

template<>
struct serde::Serializable<app::model::payments::PaymentsRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "amount",
                &app::model::payments::PaymentsRequest::get_amount,
                &app::model::payments::PaymentsRequest::set_amount,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "order_tax_amount",
                &app::model::payments::PaymentsRequest::get_order_tax_amount,
                &app::model::payments::PaymentsRequest::set_order_tax_amount,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "currency",
                &app::model::payments::PaymentsRequest::get_currency,
                &app::model::payments::PaymentsRequest::set_currency,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "amount_to_capture",
                &app::model::payments::PaymentsRequest::get_amount_to_capture,
                &app::model::payments::PaymentsRequest::set_amount_to_capture,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "shipping_cost",
                &app::model::payments::PaymentsRequest::get_shipping_cost,
                &app::model::payments::PaymentsRequest::set_shipping_cost,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_id",
                &app::model::payments::PaymentsRequest::get_payment_id,
                &app::model::payments::PaymentsRequest::set_payment_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "merchant_id",
                &app::model::payments::PaymentsRequest::get_merchant_id,
                &app::model::payments::PaymentsRequest::set_merchant_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "routing",
                &app::model::payments::PaymentsRequest::get_routing,
                &app::model::payments::PaymentsRequest::set_routing,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "connector",
                &app::model::payments::PaymentsRequest::get_connector,
                &app::model::payments::PaymentsRequest::set_connector,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "capture_method",
                &app::model::payments::PaymentsRequest::get_capture_method,
                &app::model::payments::PaymentsRequest::set_capture_method,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "authentication_type",
                &app::model::payments::PaymentsRequest::get_authentication_type,
                &app::model::payments::PaymentsRequest::set_authentication_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "billing",
                &app::model::payments::PaymentsRequest::get_billing,
                &app::model::payments::PaymentsRequest::set_billing,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "capture_on",
                &app::model::payments::PaymentsRequest::get_capture_on,
                &app::model::payments::PaymentsRequest::set_capture_on,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "confirm",
                &app::model::payments::PaymentsRequest::get_confirm,
                &app::model::payments::PaymentsRequest::set_confirm,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "customer",
                &app::model::payments::PaymentsRequest::get_customer,
                &app::model::payments::PaymentsRequest::set_customer,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "customer_id",
                &app::model::payments::PaymentsRequest::get_customer_id,
                &app::model::payments::PaymentsRequest::set_customer_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "email",
                &app::model::payments::PaymentsRequest::get_email,
                &app::model::payments::PaymentsRequest::set_email,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "name",
                &app::model::payments::PaymentsRequest::get_name,
                &app::model::payments::PaymentsRequest::set_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "phone",
                &app::model::payments::PaymentsRequest::get_phone,
                &app::model::payments::PaymentsRequest::set_phone,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "phone_country_code",
                &app::model::payments::PaymentsRequest::get_phone_country_code,
                &app::model::payments::PaymentsRequest::set_phone_country_code,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "off_session",
                &app::model::payments::PaymentsRequest::get_off_session,
                &app::model::payments::PaymentsRequest::set_off_session,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "description",
                &app::model::payments::PaymentsRequest::get_description,
                &app::model::payments::PaymentsRequest::set_description,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "return_url",
                &app::model::payments::PaymentsRequest::get_return_url,
                &app::model::payments::PaymentsRequest::set_return_url,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "setup_future_usage",
                &app::model::payments::PaymentsRequest::get_setup_future_usage,
                &app::model::payments::PaymentsRequest::set_setup_future_usage,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_method_data",
                &app::model::payments::PaymentsRequest::get_payment_method_data,
                &app::model::payments::PaymentsRequest::set_payment_method_data,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_method",
                &app::model::payments::PaymentsRequest::get_payment_method,
                &app::model::payments::PaymentsRequest::set_payment_method,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_token",
                &app::model::payments::PaymentsRequest::get_payment_token,
                &app::model::payments::PaymentsRequest::set_payment_token,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_cvc",
                &app::model::payments::PaymentsRequest::get_card_cvc,
                &app::model::payments::PaymentsRequest::set_card_cvc,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "shipping",
                &app::model::payments::PaymentsRequest::get_shipping,
                &app::model::payments::PaymentsRequest::set_shipping,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "statement_descriptor_name",
                &app::model::payments::PaymentsRequest::get_statement_descriptor_name,
                &app::model::payments::PaymentsRequest::set_statement_descriptor_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "statement_descriptor_suffix",
                &app::model::payments::PaymentsRequest::get_statement_descriptor_suffix,
                &app::model::payments::PaymentsRequest::set_statement_descriptor_suffix,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "payments_request";
    }
};

template<>
struct serde::Serializable<app::model::payments::CtpServiceDetails>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "merchant_transaction_id",
                &app::model::payments::CtpServiceDetails::get_merchant_transaction_id,
                &app::model::payments::CtpServiceDetails::set_merchant_transaction_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "correlation_id",
                &app::model::payments::CtpServiceDetails::get_correlation_id,
                &app::model::payments::CtpServiceDetails::set_correlation_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "x_src_flow_id",
                &app::model::payments::CtpServiceDetails::get_x_src_flow_id,
                &app::model::payments::CtpServiceDetails::set_x_src_flow_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "provider",
                &app::model::payments::CtpServiceDetails::get_provider,
                &app::model::payments::CtpServiceDetails::set_provider,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "encrypted_payload",
                &app::model::payments::CtpServiceDetails::get_encrypted_payload,
                &app::model::payments::CtpServiceDetails::set_encrypted_payload,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "ctp_service_details";
    }
};

template<>
struct serde::Serializable<app::model::payments::RequestSurchargeDetails>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "surcharge_amount",
                &app::model::payments::RequestSurchargeDetails::get_surcharge_amount,
                &app::model::payments::RequestSurchargeDetails::set_surcharge_amount,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "tax_amount",
                &app::model::payments::RequestSurchargeDetails::get_tax_amount,
                &app::model::payments::RequestSurchargeDetails::set_tax_amount,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "surcharge_percentage",
                &app::model::payments::RequestSurchargeDetails::get_surcharge_percentage,
                &app::model::payments::RequestSurchargeDetails::set_surcharge_percentage,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "request_surcharge_details";
    }
};

template<>
struct serde::Serializable<app::model::payments::PaymentSurchargeDetailsResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "surcharge_amount",
                &app::model::payments::PaymentSurchargeDetailsResponse::get_surcharge_amount,
                &app::model::payments::PaymentSurchargeDetailsResponse::set_surcharge_amount,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "tax_amount",
                &app::model::payments::PaymentSurchargeDetailsResponse::get_tax_amount,
                &app::model::payments::PaymentSurchargeDetailsResponse::set_tax_amount,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "surcharge_percentage",
                &app::model::payments::PaymentSurchargeDetailsResponse::get_surcharge_percentage,
                &app::model::payments::PaymentSurchargeDetailsResponse::set_surcharge_percentage,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "error_details",
                &app::model::payments::PaymentSurchargeDetailsResponse::get_error_details,
                &app::model::payments::PaymentSurchargeDetailsResponse::set_error_details,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "payment_surcharge_details_response";
    }
};

template<>
struct serde::Serializable<app::model::payments::BrowserInformation>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "color_depth",
                &app::model::payments::BrowserInformation::get_color_depth,
                &app::model::payments::BrowserInformation::set_color_depth,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "java_enabled",
                &app::model::payments::BrowserInformation::get_java_enabled,
                &app::model::payments::BrowserInformation::set_java_enabled,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "java_script_enabled",
                &app::model::payments::BrowserInformation::get_java_script_enabled,
                &app::model::payments::BrowserInformation::set_java_script_enabled,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "language",
                &app::model::payments::BrowserInformation::get_language,
                &app::model::payments::BrowserInformation::set_language,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "screen_height",
                &app::model::payments::BrowserInformation::get_screen_height,
                &app::model::payments::BrowserInformation::set_screen_height,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "screen_width",
                &app::model::payments::BrowserInformation::get_screen_width,
                &app::model::payments::BrowserInformation::set_screen_width,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "time_zone",
                &app::model::payments::BrowserInformation::get_time_zone,
                &app::model::payments::BrowserInformation::set_time_zone,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "ip_address",
                &app::model::payments::BrowserInformation::get_ip_address,
                &app::model::payments::BrowserInformation::set_ip_address,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "accept_header",
                &app::model::payments::BrowserInformation::get_accept_header,
                &app::model::payments::BrowserInformation::set_accept_header,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "user_agent",
                &app::model::payments::BrowserInformation::get_user_agent,
                &app::model::payments::BrowserInformation::set_user_agent,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "os_type",
                &app::model::payments::BrowserInformation::get_os_type,
                &app::model::payments::BrowserInformation::set_os_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "os_version",
                &app::model::payments::BrowserInformation::get_os_version,
                &app::model::payments::BrowserInformation::set_os_version,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "device_model",
                &app::model::payments::BrowserInformation::get_device_model,
                &app::model::payments::BrowserInformation::set_device_model,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "accept_language",
                &app::model::payments::BrowserInformation::get_accept_language,
                &app::model::payments::BrowserInformation::set_accept_language,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "referer",
                &app::model::payments::BrowserInformation::get_referer,
                &app::model::payments::BrowserInformation::set_referer,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "browser_information";
    }
};

template<>
struct serde::Serializable<app::model::payments::PaymentAttemptResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "attempt_id",
                &app::model::payments::PaymentAttemptResponse::get_attempt_id,
                &app::model::payments::PaymentAttemptResponse::set_attempt_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "status",
                &app::model::payments::PaymentAttemptResponse::get_status,
                &app::model::payments::PaymentAttemptResponse::set_status,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "amount",
                &app::model::payments::PaymentAttemptResponse::get_amount,
                &app::model::payments::PaymentAttemptResponse::set_amount,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "order_tax_amount",
                &app::model::payments::PaymentAttemptResponse::get_order_tax_amount,
                &app::model::payments::PaymentAttemptResponse::set_order_tax_amount,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "currency",
                &app::model::payments::PaymentAttemptResponse::get_currency,
                &app::model::payments::PaymentAttemptResponse::set_currency,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "connector",
                &app::model::payments::PaymentAttemptResponse::get_connector,
                &app::model::payments::PaymentAttemptResponse::set_connector,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "error_message",
                &app::model::payments::PaymentAttemptResponse::get_error_message,
                &app::model::payments::PaymentAttemptResponse::set_error_message,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_method",
                &app::model::payments::PaymentAttemptResponse::get_payment_method,
                &app::model::payments::PaymentAttemptResponse::set_payment_method,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "connector_transaction_id",
                &app::model::payments::PaymentAttemptResponse::get_connector_transaction_id,
                &app::model::payments::PaymentAttemptResponse::set_connector_transaction_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "capture_method",
                &app::model::payments::PaymentAttemptResponse::get_capture_method,
                &app::model::payments::PaymentAttemptResponse::set_capture_method,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "authentication_type",
                &app::model::payments::PaymentAttemptResponse::get_authentication_type,
                &app::model::payments::PaymentAttemptResponse::set_authentication_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "created_at",
                &app::model::payments::PaymentAttemptResponse::get_created_at,
                &app::model::payments::PaymentAttemptResponse::set_created_at,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "modified_at",
                &app::model::payments::PaymentAttemptResponse::get_modified_at,
                &app::model::payments::PaymentAttemptResponse::set_modified_at,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "cancellation_reason",
                &app::model::payments::PaymentAttemptResponse::get_cancellation_reason,
                &app::model::payments::PaymentAttemptResponse::set_cancellation_reason,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "mandate_id",
                &app::model::payments::PaymentAttemptResponse::get_mandate_id,
                &app::model::payments::PaymentAttemptResponse::set_mandate_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "error_code",
                &app::model::payments::PaymentAttemptResponse::get_error_code,
                &app::model::payments::PaymentAttemptResponse::set_error_code,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_token",
                &app::model::payments::PaymentAttemptResponse::get_payment_token,
                &app::model::payments::PaymentAttemptResponse::set_payment_token,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "connector_metadata",
                &app::model::payments::PaymentAttemptResponse::get_connector_metadata,
                &app::model::payments::PaymentAttemptResponse::set_connector_metadata,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_experience",
                &app::model::payments::PaymentAttemptResponse::get_payment_experience,
                &app::model::payments::PaymentAttemptResponse::set_payment_experience,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_method_type",
                &app::model::payments::PaymentAttemptResponse::get_payment_method_type,
                &app::model::payments::PaymentAttemptResponse::set_payment_method_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "reference_id",
                &app::model::payments::PaymentAttemptResponse::get_reference_id,
                &app::model::payments::PaymentAttemptResponse::set_reference_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "unified_code",
                &app::model::payments::PaymentAttemptResponse::get_unified_code,
                &app::model::payments::PaymentAttemptResponse::set_unified_code,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "unified_message",
                &app::model::payments::PaymentAttemptResponse::get_unified_message,
                &app::model::payments::PaymentAttemptResponse::set_unified_message,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "client_source",
                &app::model::payments::PaymentAttemptResponse::get_client_source,
                &app::model::payments::PaymentAttemptResponse::set_client_source,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "client_version",
                &app::model::payments::PaymentAttemptResponse::get_client_version,
                &app::model::payments::PaymentAttemptResponse::set_client_version,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "error_details",
                &app::model::payments::PaymentAttemptResponse::get_error_details,
                &app::model::payments::PaymentAttemptResponse::set_error_details,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "payment_attempt_response";
    }
};

template<>
struct serde::Serializable<app::model::payments::PaymentAttemptRecordResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "id",
                &app::model::payments::PaymentAttemptRecordResponse::get_id,
                &app::model::payments::PaymentAttemptRecordResponse::set_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "status",
                &app::model::payments::PaymentAttemptRecordResponse::get_status,
                &app::model::payments::PaymentAttemptRecordResponse::set_status,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "amount",
                &app::model::payments::PaymentAttemptRecordResponse::get_amount,
                &app::model::payments::PaymentAttemptRecordResponse::set_amount,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "error_details",
                &app::model::payments::PaymentAttemptRecordResponse::get_error_details,
                &app::model::payments::PaymentAttemptRecordResponse::set_error_details,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_intent_feature_metadata",
                &app::model::payments::PaymentAttemptRecordResponse::
                    get_payment_intent_feature_metadata,
                &app::model::payments::PaymentAttemptRecordResponse::
                    set_payment_intent_feature_metadata,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_attempt_feature_metadata",
                &app::model::payments::PaymentAttemptRecordResponse::
                    get_payment_attempt_feature_metadata,
                &app::model::payments::PaymentAttemptRecordResponse::
                    set_payment_attempt_feature_metadata,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "created_at",
                &app::model::payments::PaymentAttemptRecordResponse::get_created_at,
                &app::model::payments::PaymentAttemptRecordResponse::set_created_at,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "payment_attempt_record_response";
    }
};

template<>
struct serde::Serializable<app::model::payments::RecoveryPaymentsResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "id",
                &app::model::payments::RecoveryPaymentsResponse::get_id,
                &app::model::payments::RecoveryPaymentsResponse::set_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "intent_status",
                &app::model::payments::RecoveryPaymentsResponse::get_intent_status,
                &app::model::payments::RecoveryPaymentsResponse::set_intent_status,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "merchant_reference_id",
                &app::model::payments::RecoveryPaymentsResponse::get_merchant_reference_id,
                &app::model::payments::RecoveryPaymentsResponse::set_merchant_reference_id,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "recovery_payments_response";
    }
};

template<>
struct serde::Serializable<app::model::payments::RecoveryRequestAccepted>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "merchant_reference_id",
                &app::model::payments::RecoveryRequestAccepted::get_merchant_reference_id,
                &app::model::payments::RecoveryRequestAccepted::set_merchant_reference_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "connector_transaction_id",
                &app::model::payments::RecoveryRequestAccepted::get_connector_transaction_id,
                &app::model::payments::RecoveryRequestAccepted::set_connector_transaction_id,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "recovery_request_accepted";
    }
};

template<>
struct serde::Serializable<app::model::payments::RecoveryRetryWebhookDetails>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "id",
                &app::model::payments::RecoveryRetryWebhookDetails::get_id,
                &app::model::payments::RecoveryRetryWebhookDetails::set_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "merchant_reference_id",
                &app::model::payments::RecoveryRetryWebhookDetails::get_merchant_reference_id,
                &app::model::payments::RecoveryRetryWebhookDetails::set_merchant_reference_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "next_retry",
                &app::model::payments::RecoveryRetryWebhookDetails::get_next_retry,
                &app::model::payments::RecoveryRetryWebhookDetails::set_next_retry,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "recovery_retry_webhook_details";
    }
};

template<>
struct serde::Serializable<app::model::payments::CaptureResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "capture_id",
                &app::model::payments::CaptureResponse::get_capture_id,
                &app::model::payments::CaptureResponse::set_capture_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "status",
                &app::model::payments::CaptureResponse::get_status,
                &app::model::payments::CaptureResponse::set_status,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "amount",
                &app::model::payments::CaptureResponse::get_amount,
                &app::model::payments::CaptureResponse::set_amount,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "currency",
                &app::model::payments::CaptureResponse::get_currency,
                &app::model::payments::CaptureResponse::set_currency,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "connector",
                &app::model::payments::CaptureResponse::get_connector,
                &app::model::payments::CaptureResponse::set_connector,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "authorized_attempt_id",
                &app::model::payments::CaptureResponse::get_authorized_attempt_id,
                &app::model::payments::CaptureResponse::set_authorized_attempt_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "connector_capture_id",
                &app::model::payments::CaptureResponse::get_connector_capture_id,
                &app::model::payments::CaptureResponse::set_connector_capture_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "capture_sequence",
                &app::model::payments::CaptureResponse::get_capture_sequence,
                &app::model::payments::CaptureResponse::set_capture_sequence,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "error_message",
                &app::model::payments::CaptureResponse::get_error_message,
                &app::model::payments::CaptureResponse::set_error_message,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "error_code",
                &app::model::payments::CaptureResponse::get_error_code,
                &app::model::payments::CaptureResponse::set_error_code,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "error_reason",
                &app::model::payments::CaptureResponse::get_error_reason,
                &app::model::payments::CaptureResponse::set_error_reason,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "reference_id",
                &app::model::payments::CaptureResponse::get_reference_id,
                &app::model::payments::CaptureResponse::set_reference_id,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "capture_response";
    }
};

template<>
struct serde::Serializable<app::model::payments::VerifyRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "merchant_id",
                &app::model::payments::VerifyRequest::get_merchant_id,
                &app::model::payments::VerifyRequest::set_merchant_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "customer_id",
                &app::model::payments::VerifyRequest::get_customer_id,
                &app::model::payments::VerifyRequest::set_customer_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "email",
                &app::model::payments::VerifyRequest::get_email,
                &app::model::payments::VerifyRequest::set_email,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "name",
                &app::model::payments::VerifyRequest::get_name,
                &app::model::payments::VerifyRequest::set_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "phone",
                &app::model::payments::VerifyRequest::get_phone,
                &app::model::payments::VerifyRequest::set_phone,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "phone_country_code",
                &app::model::payments::VerifyRequest::get_phone_country_code,
                &app::model::payments::VerifyRequest::set_phone_country_code,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_method",
                &app::model::payments::VerifyRequest::get_payment_method,
                &app::model::payments::VerifyRequest::set_payment_method,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_method_data",
                &app::model::payments::VerifyRequest::get_payment_method_data,
                &app::model::payments::VerifyRequest::set_payment_method_data,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_token",
                &app::model::payments::VerifyRequest::get_payment_token,
                &app::model::payments::VerifyRequest::set_payment_token,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "mandate_data",
                &app::model::payments::VerifyRequest::get_mandate_data,
                &app::model::payments::VerifyRequest::set_mandate_data,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "setup_future_usage",
                &app::model::payments::VerifyRequest::get_setup_future_usage,
                &app::model::payments::VerifyRequest::set_setup_future_usage,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "off_session",
                &app::model::payments::VerifyRequest::get_off_session,
                &app::model::payments::VerifyRequest::set_off_session,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "client_secret",
                &app::model::payments::VerifyRequest::get_client_secret,
                &app::model::payments::VerifyRequest::set_client_secret,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "merchant_connector_details",
                &app::model::payments::VerifyRequest::get_merchant_connector_details,
                &app::model::payments::VerifyRequest::set_merchant_connector_details,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "verify_request";
    }
};

template<>
struct serde::Serializable<app::model::payments::MandateData>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "update_mandate_id",
                &app::model::payments::MandateData::get_update_mandate_id,
                &app::model::payments::MandateData::set_update_mandate_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "customer_acceptance",
                &app::model::payments::MandateData::get_customer_acceptance,
                &app::model::payments::MandateData::set_customer_acceptance,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "mandate_type",
                &app::model::payments::MandateData::get_mandate_type,
                &app::model::payments::MandateData::set_mandate_type,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "mandate_data";
    }
};

template<>
struct serde::Serializable<app::model::payments::SingleUseMandate>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "amount",
                &app::model::payments::SingleUseMandate::get_amount,
                &app::model::payments::SingleUseMandate::set_amount,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "currency",
                &app::model::payments::SingleUseMandate::get_currency,
                &app::model::payments::SingleUseMandate::set_currency,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "single_use_mandate";
    }
};

template<>
struct serde::Serializable<app::model::payments::MandateAmountData>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "amount",
                &app::model::payments::MandateAmountData::get_amount,
                &app::model::payments::MandateAmountData::set_amount,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "currency",
                &app::model::payments::MandateAmountData::get_currency,
                &app::model::payments::MandateAmountData::set_currency,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "start_date",
                &app::model::payments::MandateAmountData::get_start_date,
                &app::model::payments::MandateAmountData::set_start_date,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "end_date",
                &app::model::payments::MandateAmountData::get_end_date,
                &app::model::payments::MandateAmountData::set_end_date,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "metadata",
                &app::model::payments::MandateAmountData::get_metadata,
                &app::model::payments::MandateAmountData::set_metadata,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "mandate_amount_data";
    }
};

template<>
struct serde::Serializable<app::model::payments::NetworkDetails>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<
            "network_advice_code",
            &app::model::payments::NetworkDetails::get_network_advice_code,
            &app::model::payments::NetworkDetails::set_network_advice_code,
            serde::FieldOptions{}>{}};
    }

    static constexpr std::string_view table_name()
    {
        return "network_details";
    }
};

template<>
struct serde::Serializable<app::model::payments::ExtendedCardInfo>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "card_number",
                &app::model::payments::ExtendedCardInfo::get_card_number,
                &app::model::payments::ExtendedCardInfo::set_card_number,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_exp_month",
                &app::model::payments::ExtendedCardInfo::get_card_exp_month,
                &app::model::payments::ExtendedCardInfo::set_card_exp_month,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_exp_year",
                &app::model::payments::ExtendedCardInfo::get_card_exp_year,
                &app::model::payments::ExtendedCardInfo::set_card_exp_year,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_holder_name",
                &app::model::payments::ExtendedCardInfo::get_card_holder_name,
                &app::model::payments::ExtendedCardInfo::set_card_holder_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_cvc",
                &app::model::payments::ExtendedCardInfo::get_card_cvc,
                &app::model::payments::ExtendedCardInfo::set_card_cvc,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_issuer",
                &app::model::payments::ExtendedCardInfo::get_card_issuer,
                &app::model::payments::ExtendedCardInfo::set_card_issuer,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_network",
                &app::model::payments::ExtendedCardInfo::get_card_network,
                &app::model::payments::ExtendedCardInfo::set_card_network,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_type",
                &app::model::payments::ExtendedCardInfo::get_card_type,
                &app::model::payments::ExtendedCardInfo::set_card_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_issuing_country",
                &app::model::payments::ExtendedCardInfo::get_card_issuing_country,
                &app::model::payments::ExtendedCardInfo::set_card_issuing_country,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "bank_code",
                &app::model::payments::ExtendedCardInfo::get_bank_code,
                &app::model::payments::ExtendedCardInfo::set_bank_code,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "extended_card_info";
    }
};

template<>
struct serde::Serializable<app::model::payments::ProxyCardData>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "card_number",
                &app::model::payments::ProxyCardData::get_card_number,
                &app::model::payments::ProxyCardData::set_card_number,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_exp_month",
                &app::model::payments::ProxyCardData::get_card_exp_month,
                &app::model::payments::ProxyCardData::set_card_exp_month,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_exp_year",
                &app::model::payments::ProxyCardData::get_card_exp_year,
                &app::model::payments::ProxyCardData::set_card_exp_year,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_holder_name",
                &app::model::payments::ProxyCardData::get_card_holder_name,
                &app::model::payments::ProxyCardData::set_card_holder_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_cvc",
                &app::model::payments::ProxyCardData::get_card_cvc,
                &app::model::payments::ProxyCardData::set_card_cvc,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_issuer",
                &app::model::payments::ProxyCardData::get_card_issuer,
                &app::model::payments::ProxyCardData::set_card_issuer,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_network",
                &app::model::payments::ProxyCardData::get_card_network,
                &app::model::payments::ProxyCardData::set_card_network,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_type",
                &app::model::payments::ProxyCardData::get_card_type,
                &app::model::payments::ProxyCardData::set_card_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_issuing_country",
                &app::model::payments::ProxyCardData::get_card_issuing_country,
                &app::model::payments::ProxyCardData::set_card_issuing_country,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "bank_code",
                &app::model::payments::ProxyCardData::get_bank_code,
                &app::model::payments::ProxyCardData::set_bank_code,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "nick_name",
                &app::model::payments::ProxyCardData::get_nick_name,
                &app::model::payments::ProxyCardData::set_nick_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "bin_number",
                &app::model::payments::ProxyCardData::get_bin_number,
                &app::model::payments::ProxyCardData::set_bin_number,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "last_four",
                &app::model::payments::ProxyCardData::get_last_four,
                &app::model::payments::ProxyCardData::set_last_four,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "proxy_card_data";
    }
};

template<>
struct serde::Serializable<app::model::payments::NetworkTokenData>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "network_token",
                &app::model::payments::NetworkTokenData::get_network_token,
                &app::model::payments::NetworkTokenData::set_network_token,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "token_exp_month",
                &app::model::payments::NetworkTokenData::get_token_exp_month,
                &app::model::payments::NetworkTokenData::set_token_exp_month,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "token_exp_year",
                &app::model::payments::NetworkTokenData::get_token_exp_year,
                &app::model::payments::NetworkTokenData::set_token_exp_year,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "token_cryptogram",
                &app::model::payments::NetworkTokenData::get_token_cryptogram,
                &app::model::payments::NetworkTokenData::set_token_cryptogram,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_network",
                &app::model::payments::NetworkTokenData::get_card_network,
                &app::model::payments::NetworkTokenData::set_card_network,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_type",
                &app::model::payments::NetworkTokenData::get_card_type,
                &app::model::payments::NetworkTokenData::set_card_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_issuing_country",
                &app::model::payments::NetworkTokenData::get_card_issuing_country,
                &app::model::payments::NetworkTokenData::set_card_issuing_country,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "bank_code",
                &app::model::payments::NetworkTokenData::get_bank_code,
                &app::model::payments::NetworkTokenData::set_bank_code,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_holder_name",
                &app::model::payments::NetworkTokenData::get_card_holder_name,
                &app::model::payments::NetworkTokenData::set_card_holder_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_issuer",
                &app::model::payments::NetworkTokenData::get_card_issuer,
                &app::model::payments::NetworkTokenData::set_card_issuer,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "nick_name",
                &app::model::payments::NetworkTokenData::get_nick_name,
                &app::model::payments::NetworkTokenData::set_nick_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "eci",
                &app::model::payments::NetworkTokenData::get_eci,
                &app::model::payments::NetworkTokenData::set_eci,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "par",
                &app::model::payments::NetworkTokenData::get_par,
                &app::model::payments::NetworkTokenData::set_par,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "network_token_data";
    }
};

template<>
struct serde::Serializable<app::model::payments::NetworkTokenResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "last4",
                &app::model::payments::NetworkTokenResponse::get_last4,
                &app::model::payments::NetworkTokenResponse::set_last4,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_type",
                &app::model::payments::NetworkTokenResponse::get_card_type,
                &app::model::payments::NetworkTokenResponse::set_card_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_network",
                &app::model::payments::NetworkTokenResponse::get_card_network,
                &app::model::payments::NetworkTokenResponse::set_card_network,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "token_isin",
                &app::model::payments::NetworkTokenResponse::get_token_isin,
                &app::model::payments::NetworkTokenResponse::set_token_isin,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_issuer",
                &app::model::payments::NetworkTokenResponse::get_card_issuer,
                &app::model::payments::NetworkTokenResponse::set_card_issuer,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_issuing_country",
                &app::model::payments::NetworkTokenResponse::get_card_issuing_country,
                &app::model::payments::NetworkTokenResponse::set_card_issuing_country,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "token_exp_month",
                &app::model::payments::NetworkTokenResponse::get_token_exp_month,
                &app::model::payments::NetworkTokenResponse::set_token_exp_month,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "token_exp_year",
                &app::model::payments::NetworkTokenResponse::get_token_exp_year,
                &app::model::payments::NetworkTokenResponse::set_token_exp_year,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_holder_name",
                &app::model::payments::NetworkTokenResponse::get_card_holder_name,
                &app::model::payments::NetworkTokenResponse::set_card_holder_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "par",
                &app::model::payments::NetworkTokenResponse::get_par,
                &app::model::payments::NetworkTokenResponse::set_par,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "network_token_response";
    }
};

template<>
struct serde::Serializable<app::model::payments::PMBalanceCheckSuccessResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "balance",
                &app::model::payments::PMBalanceCheckSuccessResponse::get_balance,
                &app::model::payments::PMBalanceCheckSuccessResponse::set_balance,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "applicable_amount",
                &app::model::payments::PMBalanceCheckSuccessResponse::get_applicable_amount,
                &app::model::payments::PMBalanceCheckSuccessResponse::set_applicable_amount,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "currency",
                &app::model::payments::PMBalanceCheckSuccessResponse::get_currency,
                &app::model::payments::PMBalanceCheckSuccessResponse::set_currency,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "pm_balance_check_success_response";
    }
};

template<>
struct serde::Serializable<app::model::payments::PMBalanceCheckFailureResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<
            "error",
            &app::model::payments::PMBalanceCheckFailureResponse::get_error,
            &app::model::payments::PMBalanceCheckFailureResponse::set_error,
            serde::FieldOptions{}>{}};
    }

    static constexpr std::string_view table_name()
    {
        return "pm_balance_check_failure_response";
    }
};

template<>
struct serde::Serializable<app::model::payments::AdditionalNetworkTokenInfo>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "card_issuer",
                &app::model::payments::AdditionalNetworkTokenInfo::get_card_issuer,
                &app::model::payments::AdditionalNetworkTokenInfo::set_card_issuer,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_network",
                &app::model::payments::AdditionalNetworkTokenInfo::get_card_network,
                &app::model::payments::AdditionalNetworkTokenInfo::set_card_network,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_type",
                &app::model::payments::AdditionalNetworkTokenInfo::get_card_type,
                &app::model::payments::AdditionalNetworkTokenInfo::set_card_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_issuing_country",
                &app::model::payments::AdditionalNetworkTokenInfo::get_card_issuing_country,
                &app::model::payments::AdditionalNetworkTokenInfo::set_card_issuing_country,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "bank_code",
                &app::model::payments::AdditionalNetworkTokenInfo::get_bank_code,
                &app::model::payments::AdditionalNetworkTokenInfo::set_bank_code,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "last4",
                &app::model::payments::AdditionalNetworkTokenInfo::get_last4,
                &app::model::payments::AdditionalNetworkTokenInfo::set_last4,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "token_isin",
                &app::model::payments::AdditionalNetworkTokenInfo::get_token_isin,
                &app::model::payments::AdditionalNetworkTokenInfo::set_token_isin,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "token_exp_month",
                &app::model::payments::AdditionalNetworkTokenInfo::get_token_exp_month,
                &app::model::payments::AdditionalNetworkTokenInfo::set_token_exp_month,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "token_exp_year",
                &app::model::payments::AdditionalNetworkTokenInfo::get_token_exp_year,
                &app::model::payments::AdditionalNetworkTokenInfo::set_token_exp_year,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_holder_name",
                &app::model::payments::AdditionalNetworkTokenInfo::get_card_holder_name,
                &app::model::payments::AdditionalNetworkTokenInfo::set_card_holder_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "par",
                &app::model::payments::AdditionalNetworkTokenInfo::get_par,
                &app::model::payments::AdditionalNetworkTokenInfo::set_par,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "additional_network_token_info";
    }
};

template<>
struct serde::Serializable<app::model::payments::KlarnaSdkPaymentMethod>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<
            "payment_type",
            &app::model::payments::KlarnaSdkPaymentMethod::get_payment_type,
            &app::model::payments::KlarnaSdkPaymentMethod::set_payment_type,
            serde::FieldOptions{}>{}};
    }

    static constexpr std::string_view table_name()
    {
        return "klarna_sdk_payment_method";
    }
};

template<>
struct serde::Serializable<app::model::payments::AlfamartVoucherData>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "first_name",
                &app::model::payments::AlfamartVoucherData::get_first_name,
                &app::model::payments::AlfamartVoucherData::set_first_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "last_name",
                &app::model::payments::AlfamartVoucherData::get_last_name,
                &app::model::payments::AlfamartVoucherData::set_last_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "email",
                &app::model::payments::AlfamartVoucherData::get_email,
                &app::model::payments::AlfamartVoucherData::set_email,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "alfamart_voucher_data";
    }
};

template<>
struct serde::Serializable<app::model::payments::IndomaretVoucherData>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "first_name",
                &app::model::payments::IndomaretVoucherData::get_first_name,
                &app::model::payments::IndomaretVoucherData::set_first_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "last_name",
                &app::model::payments::IndomaretVoucherData::get_last_name,
                &app::model::payments::IndomaretVoucherData::set_last_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "email",
                &app::model::payments::IndomaretVoucherData::get_email,
                &app::model::payments::IndomaretVoucherData::set_email,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "indomaret_voucher_data";
    }
};

template<>
struct serde::Serializable<app::model::payments::JCSVoucherData>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "first_name",
                &app::model::payments::JCSVoucherData::get_first_name,
                &app::model::payments::JCSVoucherData::set_first_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "last_name",
                &app::model::payments::JCSVoucherData::get_last_name,
                &app::model::payments::JCSVoucherData::set_last_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "email",
                &app::model::payments::JCSVoucherData::get_email,
                &app::model::payments::JCSVoucherData::set_email,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "phone_number",
                &app::model::payments::JCSVoucherData::get_phone_number,
                &app::model::payments::JCSVoucherData::set_phone_number,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "jcs_voucher_data";
    }
};

template<>
struct serde::Serializable<app::model::payments::AchBillingDetails>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<
            "email",
            &app::model::payments::AchBillingDetails::get_email,
            &app::model::payments::AchBillingDetails::set_email,
            serde::FieldOptions{}>{}};
    }

    static constexpr std::string_view table_name()
    {
        return "ach_billing_details";
    }
};

template<>
struct serde::Serializable<app::model::payments::DokuBillingDetails>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "first_name",
                &app::model::payments::DokuBillingDetails::get_first_name,
                &app::model::payments::DokuBillingDetails::set_first_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "last_name",
                &app::model::payments::DokuBillingDetails::get_last_name,
                &app::model::payments::DokuBillingDetails::set_last_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "email",
                &app::model::payments::DokuBillingDetails::get_email,
                &app::model::payments::DokuBillingDetails::set_email,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "doku_billing_details";
    }
};

template<>
struct serde::Serializable<app::model::payments::MultibancoBillingDetails>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<
            "email",
            &app::model::payments::MultibancoBillingDetails::get_email,
            &app::model::payments::MultibancoBillingDetails::set_email,
            serde::FieldOptions{}>{}};
    }

    static constexpr std::string_view table_name()
    {
        return "multibanco_billing_details";
    }
};

template<>
struct serde::Serializable<app::model::payments::SepaAndBacsBillingDetails>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "email",
                &app::model::payments::SepaAndBacsBillingDetails::get_email,
                &app::model::payments::SepaAndBacsBillingDetails::set_email,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "name",
                &app::model::payments::SepaAndBacsBillingDetails::get_name,
                &app::model::payments::SepaAndBacsBillingDetails::set_name,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "sepa_and_bacs_billing_details";
    }
};

template<>
struct serde::Serializable<app::model::payments::CryptoData>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "pay_currency",
                &app::model::payments::CryptoData::get_pay_currency,
                &app::model::payments::CryptoData::set_pay_currency,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "network",
                &app::model::payments::CryptoData::get_network,
                &app::model::payments::CryptoData::set_network,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "crypto_data";
    }
};

template<>
struct serde::Serializable<app::model::payments::UpiCollectData>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "vpa_id",
                &app::model::payments::UpiCollectData::get_vpa_id,
                &app::model::payments::UpiCollectData::set_vpa_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "upi_source",
                &app::model::payments::UpiCollectData::get_upi_source,
                &app::model::payments::UpiCollectData::set_upi_source,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "upi_collect_data";
    }
};

template<>
struct serde::Serializable<app::model::payments::UpiQrData>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<
            "upi_source",
            &app::model::payments::UpiQrData::get_upi_source,
            &app::model::payments::UpiQrData::set_upi_source,
            serde::FieldOptions{}>{}};
    }

    static constexpr std::string_view table_name()
    {
        return "upi_qr_data";
    }
};

template<>
struct serde::Serializable<app::model::payments::UpiIntentData>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "upi_source",
                &app::model::payments::UpiIntentData::get_upi_source,
                &app::model::payments::UpiIntentData::set_upi_source,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "app_name",
                &app::model::payments::UpiIntentData::get_app_name,
                &app::model::payments::UpiIntentData::set_app_name,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "upi_intent_data";
    }
};

template<>
struct serde::Serializable<app::model::payments::SofortBilling>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<
            "billing_country",
            &app::model::payments::SofortBilling::get_billing_country,
            &app::model::payments::SofortBilling::set_billing_country,
            serde::FieldOptions{}>{}};
    }

    static constexpr std::string_view table_name()
    {
        return "sofort_billing";
    }
};

template<>
struct serde::Serializable<app::model::payments::DocumentDetails>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "document_type",
                &app::model::payments::DocumentDetails::get_document_type,
                &app::model::payments::DocumentDetails::set_document_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "document_number",
                &app::model::payments::DocumentDetails::get_document_number,
                &app::model::payments::DocumentDetails::set_document_number,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "document_details";
    }
};

template<>
struct serde::Serializable<app::model::payments::SkrillData>
{
    static constexpr auto fields()
    {
        return std::tuple{};
    }

    static constexpr std::string_view table_name()
    {
        return "skrill_data";
    }
};

template<>
struct serde::Serializable<app::model::payments::NetellerData>
{
    static constexpr auto fields()
    {
        return std::tuple{};
    }

    static constexpr std::string_view table_name()
    {
        return "neteller_data";
    }
};

template<>
struct serde::Serializable<app::model::payments::PayseraData>
{
    static constexpr auto fields()
    {
        return std::tuple{};
    }

    static constexpr std::string_view table_name()
    {
        return "paysera_data";
    }
};

template<>
struct serde::Serializable<app::model::payments::WeChatPay>
{
    static constexpr auto fields()
    {
        return std::tuple{};
    }

    static constexpr std::string_view table_name()
    {
        return "we_chat_pay";
    }
};

template<>
struct serde::Serializable<app::model::payments::SwishQrData>
{
    static constexpr auto fields()
    {
        return std::tuple{};
    }

    static constexpr std::string_view table_name()
    {
        return "swish_qr_data";
    }
};

template<>
struct serde::Serializable<app::model::payments::RevolutPayData>
{
    static constexpr auto fields()
    {
        return std::tuple{};
    }

    static constexpr std::string_view table_name()
    {
        return "revolut_pay_data";
    }
};

template<>
struct serde::Serializable<app::model::payments::MifinityData>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "date_of_birth",
                &app::model::payments::MifinityData::get_date_of_birth,
                &app::model::payments::MifinityData::set_date_of_birth,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "language_preference",
                &app::model::payments::MifinityData::get_language_preference,
                &app::model::payments::MifinityData::set_language_preference,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "mifinity_data";
    }
};

template<>
struct serde::Serializable<app::model::payments::CardResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "last4",
                &app::model::payments::CardResponse::get_last4,
                &app::model::payments::CardResponse::set_last4,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_type",
                &app::model::payments::CardResponse::get_card_type,
                &app::model::payments::CardResponse::set_card_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_subtype",
                &app::model::payments::CardResponse::get_card_subtype,
                &app::model::payments::CardResponse::set_card_subtype,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_segment_type",
                &app::model::payments::CardResponse::get_card_segment_type,
                &app::model::payments::CardResponse::set_card_segment_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "funding_source",
                &app::model::payments::CardResponse::get_funding_source,
                &app::model::payments::CardResponse::set_funding_source,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_network",
                &app::model::payments::CardResponse::get_card_network,
                &app::model::payments::CardResponse::set_card_network,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_issuer",
                &app::model::payments::CardResponse::get_card_issuer,
                &app::model::payments::CardResponse::set_card_issuer,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_issuing_country",
                &app::model::payments::CardResponse::get_card_issuing_country,
                &app::model::payments::CardResponse::set_card_issuing_country,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_isin",
                &app::model::payments::CardResponse::get_card_isin,
                &app::model::payments::CardResponse::set_card_isin,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_extended_bin",
                &app::model::payments::CardResponse::get_card_extended_bin,
                &app::model::payments::CardResponse::set_card_extended_bin,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_exp_month",
                &app::model::payments::CardResponse::get_card_exp_month,
                &app::model::payments::CardResponse::set_card_exp_month,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_exp_year",
                &app::model::payments::CardResponse::get_card_exp_year,
                &app::model::payments::CardResponse::set_card_exp_year,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_holder_name",
                &app::model::payments::CardResponse::get_card_holder_name,
                &app::model::payments::CardResponse::set_card_holder_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_checks",
                &app::model::payments::CardResponse::get_payment_checks,
                &app::model::payments::CardResponse::set_payment_checks,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "authentication_data",
                &app::model::payments::CardResponse::get_authentication_data,
                &app::model::payments::CardResponse::set_authentication_data,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "auth_code",
                &app::model::payments::CardResponse::get_auth_code,
                &app::model::payments::CardResponse::set_auth_code,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "card_response";
    }
};

template<>
struct serde::Serializable<app::model::payments::RewardData>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<
            "merchant_id",
            &app::model::payments::RewardData::get_merchant_id,
            &app::model::payments::RewardData::set_merchant_id,
            serde::FieldOptions{}>{}};
    }

    static constexpr std::string_view table_name()
    {
        return "reward_data";
    }
};

template<>
struct serde::Serializable<app::model::payments::CryptoResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<
            "details",
            &app::model::payments::CryptoResponse::get_details,
            &app::model::payments::CryptoResponse::set_details,
            serde::FieldOptions{}>{}};
    }

    static constexpr std::string_view table_name()
    {
        return "crypto_response";
    }
};

template<>
struct serde::Serializable<app::model::payments::OpenBankingResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<
            "details",
            &app::model::payments::OpenBankingResponse::get_details,
            &app::model::payments::OpenBankingResponse::set_details,
            serde::FieldOptions{}>{}};
    }

    static constexpr std::string_view table_name()
    {
        return "open_banking_response";
    }
};

template<>
struct serde::Serializable<app::model::payments::MobilePaymentResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<
            "details",
            &app::model::payments::MobilePaymentResponse::get_details,
            &app::model::payments::MobilePaymentResponse::set_details,
            serde::FieldOptions{}>{}};
    }

    static constexpr std::string_view table_name()
    {
        return "mobile_payment_response";
    }
};

template<>
struct serde::Serializable<app::model::payments::RealTimePaymentDataResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<
            "details",
            &app::model::payments::RealTimePaymentDataResponse::get_details,
            &app::model::payments::RealTimePaymentDataResponse::set_details,
            serde::FieldOptions{}>{}};
    }

    static constexpr std::string_view table_name()
    {
        return "real_time_payment_data_response";
    }
};

template<>
struct serde::Serializable<app::model::payments::UpiResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<
            "details",
            &app::model::payments::UpiResponse::get_details,
            &app::model::payments::UpiResponse::set_details,
            serde::FieldOptions{}>{}};
    }

    static constexpr std::string_view table_name()
    {
        return "upi_response";
    }
};

template<>
struct serde::Serializable<app::model::payments::VoucherResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<
            "details",
            &app::model::payments::VoucherResponse::get_details,
            &app::model::payments::VoucherResponse::set_details,
            serde::FieldOptions{}>{}};
    }

    static constexpr std::string_view table_name()
    {
        return "voucher_response";
    }
};

template<>
struct serde::Serializable<app::model::payments::PaylaterResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<
            "klarna_sdk",
            &app::model::payments::PaylaterResponse::get_klarna_sdk,
            &app::model::payments::PaylaterResponse::set_klarna_sdk,
            serde::FieldOptions{}>{}};
    }

    static constexpr std::string_view table_name()
    {
        return "paylater_response";
    }
};

template<>
struct serde::Serializable<app::model::payments::WalletResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<
            "details",
            &app::model::payments::WalletResponse::get_details,
            &app::model::payments::WalletResponse::set_details,
            serde::FieldOptions{}>{}};
    }

    static constexpr std::string_view table_name()
    {
        return "wallet_response";
    }
};

template<>
struct serde::Serializable<app::model::payments::KlarnaSdkPaymentMethodResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<
            "payment_type",
            &app::model::payments::KlarnaSdkPaymentMethodResponse::get_payment_type,
            &app::model::payments::KlarnaSdkPaymentMethodResponse::set_payment_type,
            serde::FieldOptions{}>{}};
    }

    static constexpr std::string_view table_name()
    {
        return "klarna_sdk_payment_method_response";
    }
};

template<>
struct serde::Serializable<app::model::payments::AddressDetailsWithPhone>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "address",
                &app::model::payments::AddressDetailsWithPhone::get_address,
                &app::model::payments::AddressDetailsWithPhone::set_address,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "phone_number",
                &app::model::payments::AddressDetailsWithPhone::get_phone_number,
                &app::model::payments::AddressDetailsWithPhone::set_phone_number,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "email",
                &app::model::payments::AddressDetailsWithPhone::get_email,
                &app::model::payments::AddressDetailsWithPhone::set_email,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "address_details_with_phone";
    }
};

template<>
struct serde::Serializable<app::model::payments::EncryptableAddressDetails>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "line1",
                &app::model::payments::EncryptableAddressDetails::get_line1,
                &app::model::payments::EncryptableAddressDetails::set_line1,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "line2",
                &app::model::payments::EncryptableAddressDetails::get_line2,
                &app::model::payments::EncryptableAddressDetails::set_line2,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "line3",
                &app::model::payments::EncryptableAddressDetails::get_line3,
                &app::model::payments::EncryptableAddressDetails::set_line3,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "state",
                &app::model::payments::EncryptableAddressDetails::get_state,
                &app::model::payments::EncryptableAddressDetails::set_state,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "zip",
                &app::model::payments::EncryptableAddressDetails::get_zip,
                &app::model::payments::EncryptableAddressDetails::set_zip,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "first_name",
                &app::model::payments::EncryptableAddressDetails::get_first_name,
                &app::model::payments::EncryptableAddressDetails::set_first_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "last_name",
                &app::model::payments::EncryptableAddressDetails::get_last_name,
                &app::model::payments::EncryptableAddressDetails::set_last_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "phone_number",
                &app::model::payments::EncryptableAddressDetails::get_phone_number,
                &app::model::payments::EncryptableAddressDetails::set_phone_number,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "email",
                &app::model::payments::EncryptableAddressDetails::get_email,
                &app::model::payments::EncryptableAddressDetails::set_email,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "encryptable_address_details";
    }
};

template<>
struct serde::Serializable<app::model::payments::PaymentsCaptureRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "payment_id",
                &app::model::payments::PaymentsCaptureRequest::get_payment_id,
                &app::model::payments::PaymentsCaptureRequest::set_payment_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "merchant_id",
                &app::model::payments::PaymentsCaptureRequest::get_merchant_id,
                &app::model::payments::PaymentsCaptureRequest::set_merchant_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "amount_to_capture",
                &app::model::payments::PaymentsCaptureRequest::get_amount_to_capture,
                &app::model::payments::PaymentsCaptureRequest::set_amount_to_capture,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "refund_uncaptured_amount",
                &app::model::payments::PaymentsCaptureRequest::get_refund_uncaptured_amount,
                &app::model::payments::PaymentsCaptureRequest::set_refund_uncaptured_amount,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "statement_descriptor_suffix",
                &app::model::payments::PaymentsCaptureRequest::get_statement_descriptor_suffix,
                &app::model::payments::PaymentsCaptureRequest::set_statement_descriptor_suffix,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "statement_descriptor_prefix",
                &app::model::payments::PaymentsCaptureRequest::get_statement_descriptor_prefix,
                &app::model::payments::PaymentsCaptureRequest::set_statement_descriptor_prefix,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "merchant_connector_details",
                &app::model::payments::PaymentsCaptureRequest::get_merchant_connector_details,
                &app::model::payments::PaymentsCaptureRequest::set_merchant_connector_details,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "all_keys_required",
                &app::model::payments::PaymentsCaptureRequest::get_all_keys_required,
                &app::model::payments::PaymentsCaptureRequest::set_all_keys_required,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_method_data",
                &app::model::payments::PaymentsCaptureRequest::get_payment_method_data,
                &app::model::payments::PaymentsCaptureRequest::set_payment_method_data,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "feature_metadata",
                &app::model::payments::PaymentsCaptureRequest::get_feature_metadata,
                &app::model::payments::PaymentsCaptureRequest::set_feature_metadata,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "payments_capture_request";
    }
};

template<>
struct serde::Serializable<app::model::payments::PaymentsCaptureResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "id",
                &app::model::payments::PaymentsCaptureResponse::get_id,
                &app::model::payments::PaymentsCaptureResponse::set_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "status",
                &app::model::payments::PaymentsCaptureResponse::get_status,
                &app::model::payments::PaymentsCaptureResponse::set_status,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "amount",
                &app::model::payments::PaymentsCaptureResponse::get_amount,
                &app::model::payments::PaymentsCaptureResponse::set_amount,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "payments_capture_response";
    }
};

template<>
struct serde::Serializable<app::model::payments::PaymentsCancelRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "payment_id",
                &app::model::payments::PaymentsCancelRequest::get_payment_id,
                &app::model::payments::PaymentsCancelRequest::set_payment_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "cancellation_reason",
                &app::model::payments::PaymentsCancelRequest::get_cancellation_reason,
                &app::model::payments::PaymentsCancelRequest::set_cancellation_reason,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "merchant_connector_details",
                &app::model::payments::PaymentsCancelRequest::get_merchant_connector_details,
                &app::model::payments::PaymentsCancelRequest::set_merchant_connector_details,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "all_keys_required",
                &app::model::payments::PaymentsCancelRequest::get_all_keys_required,
                &app::model::payments::PaymentsCancelRequest::set_all_keys_required,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_method_data",
                &app::model::payments::PaymentsCancelRequest::get_payment_method_data,
                &app::model::payments::PaymentsCancelRequest::set_payment_method_data,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "payments_cancel_request";
    }
};

template<>
struct serde::Serializable<app::model::payments::PaymentsCancelResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "id",
                &app::model::payments::PaymentsCancelResponse::get_id,
                &app::model::payments::PaymentsCancelResponse::set_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "status",
                &app::model::payments::PaymentsCancelResponse::get_status,
                &app::model::payments::PaymentsCancelResponse::set_status,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "cancellation_reason",
                &app::model::payments::PaymentsCancelResponse::get_cancellation_reason,
                &app::model::payments::PaymentsCancelResponse::set_cancellation_reason,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "amount",
                &app::model::payments::PaymentsCancelResponse::get_amount,
                &app::model::payments::PaymentsCancelResponse::set_amount,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "customer_id",
                &app::model::payments::PaymentsCancelResponse::get_customer_id,
                &app::model::payments::PaymentsCancelResponse::set_customer_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "connector",
                &app::model::payments::PaymentsCancelResponse::get_connector,
                &app::model::payments::PaymentsCancelResponse::set_connector,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "created",
                &app::model::payments::PaymentsCancelResponse::get_created,
                &app::model::payments::PaymentsCancelResponse::set_created,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_method_type",
                &app::model::payments::PaymentsCancelResponse::get_payment_method_type,
                &app::model::payments::PaymentsCancelResponse::set_payment_method_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_method_subtype",
                &app::model::payments::PaymentsCancelResponse::get_payment_method_subtype,
                &app::model::payments::PaymentsCancelResponse::set_payment_method_subtype,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "attempts",
                &app::model::payments::PaymentsCancelResponse::get_attempts,
                &app::model::payments::PaymentsCancelResponse::set_attempts,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "return_url",
                &app::model::payments::PaymentsCancelResponse::get_return_url,
                &app::model::payments::PaymentsCancelResponse::set_return_url,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "error",
                &app::model::payments::PaymentsCancelResponse::get_error,
                &app::model::payments::PaymentsCancelResponse::set_error,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "payments_cancel_response";
    }
};

template<>
struct serde::Serializable<app::model::payments::UrlDetails>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "url",
                &app::model::payments::UrlDetails::get_url,
                &app::model::payments::UrlDetails::set_url,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "method",
                &app::model::payments::UrlDetails::get_method,
                &app::model::payments::UrlDetails::set_method,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "url_details";
    }
};

template<>
struct serde::Serializable<app::model::payments::AuthenticationForStartResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<
            "authentication",
            &app::model::payments::AuthenticationForStartResponse::get_authentication,
            &app::model::payments::AuthenticationForStartResponse::set_authentication,
            serde::FieldOptions{}>{}};
    }

    static constexpr std::string_view table_name()
    {
        return "authentication_for_start_response";
    }
};

template<>
struct serde::Serializable<app::model::payments::DDCData>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "iframe_url",
                &app::model::payments::DDCData::get_iframe_url,
                &app::model::payments::DDCData::set_iframe_url,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "timeout_ms",
                &app::model::payments::DDCData::get_timeout_ms,
                &app::model::payments::DDCData::set_timeout_ms,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "ddc_data";
    }
};

template<>
struct serde::Serializable<app::model::payments::ThreeDsData>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "three_ds_authentication_url",
                &app::model::payments::ThreeDsData::get_three_ds_authentication_url,
                &app::model::payments::ThreeDsData::set_three_ds_authentication_url,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "three_ds_authorize_url",
                &app::model::payments::ThreeDsData::get_three_ds_authorize_url,
                &app::model::payments::ThreeDsData::set_three_ds_authorize_url,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "three_ds_method_details",
                &app::model::payments::ThreeDsData::get_three_ds_method_details,
                &app::model::payments::ThreeDsData::set_three_ds_method_details,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "poll_config",
                &app::model::payments::ThreeDsData::get_poll_config,
                &app::model::payments::ThreeDsData::set_poll_config,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "message_version",
                &app::model::payments::ThreeDsData::get_message_version,
                &app::model::payments::ThreeDsData::set_message_version,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "directory_server_id",
                &app::model::payments::ThreeDsData::get_directory_server_id,
                &app::model::payments::ThreeDsData::set_directory_server_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_network",
                &app::model::payments::ThreeDsData::get_card_network,
                &app::model::payments::ThreeDsData::set_card_network,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "three_ds_connector",
                &app::model::payments::ThreeDsData::get_three_ds_connector,
                &app::model::payments::ThreeDsData::set_three_ds_connector,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "three_ds_data";
    }
};

template<>
struct serde::Serializable<app::model::payments::PollConfigResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "poll_id",
                &app::model::payments::PollConfigResponse::get_poll_id,
                &app::model::payments::PollConfigResponse::set_poll_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "delay_in_secs",
                &app::model::payments::PollConfigResponse::get_delay_in_secs,
                &app::model::payments::PollConfigResponse::set_delay_in_secs,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "frequency",
                &app::model::payments::PollConfigResponse::get_frequency,
                &app::model::payments::PollConfigResponse::set_frequency,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "poll_config_response";
    }
};

template<>
struct serde::Serializable<app::model::payments::SdkNextActionData>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "next_action",
                &app::model::payments::SdkNextActionData::get_next_action,
                &app::model::payments::SdkNextActionData::set_next_action,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "order_id",
                &app::model::payments::SdkNextActionData::get_order_id,
                &app::model::payments::SdkNextActionData::set_order_id,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "sdk_next_action_data";
    }
};

template<>
struct serde::Serializable<app::model::payments::VoucherNextStepData>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "entry_date",
                &app::model::payments::VoucherNextStepData::get_entry_date,
                &app::model::payments::VoucherNextStepData::set_entry_date,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "expires_at",
                &app::model::payments::VoucherNextStepData::get_expires_at,
                &app::model::payments::VoucherNextStepData::set_expires_at,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "expiry_date",
                &app::model::payments::VoucherNextStepData::get_expiry_date,
                &app::model::payments::VoucherNextStepData::set_expiry_date,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "reference",
                &app::model::payments::VoucherNextStepData::get_reference,
                &app::model::payments::VoucherNextStepData::set_reference,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "download_url",
                &app::model::payments::VoucherNextStepData::get_download_url,
                &app::model::payments::VoucherNextStepData::set_download_url,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "instructions_url",
                &app::model::payments::VoucherNextStepData::get_instructions_url,
                &app::model::payments::VoucherNextStepData::set_instructions_url,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "digitable_line",
                &app::model::payments::VoucherNextStepData::get_digitable_line,
                &app::model::payments::VoucherNextStepData::set_digitable_line,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "barcode",
                &app::model::payments::VoucherNextStepData::get_barcode,
                &app::model::payments::VoucherNextStepData::set_barcode,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "qr_code_url",
                &app::model::payments::VoucherNextStepData::get_qr_code_url,
                &app::model::payments::VoucherNextStepData::set_qr_code_url,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "raw_qr_data",
                &app::model::payments::VoucherNextStepData::get_raw_qr_data,
                &app::model::payments::VoucherNextStepData::set_raw_qr_data,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "voucher_next_step_data";
    }
};

template<>
struct serde::Serializable<app::model::payments::MobilePaymentNextStepData>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<
            "consent_data_required",
            &app::model::payments::MobilePaymentNextStepData::get_consent_data_required,
            &app::model::payments::MobilePaymentNextStepData::set_consent_data_required,
            serde::FieldOptions{}>{}};
    }

    static constexpr std::string_view table_name()
    {
        return "mobile_payment_next_step_data";
    }
};

template<>
struct serde::Serializable<app::model::payments::WaitScreenInstructions>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "display_from_timestamp",
                &app::model::payments::WaitScreenInstructions::get_display_from_timestamp,
                &app::model::payments::WaitScreenInstructions::set_display_from_timestamp,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "display_to_timestamp",
                &app::model::payments::WaitScreenInstructions::get_display_to_timestamp,
                &app::model::payments::WaitScreenInstructions::set_display_to_timestamp,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "poll_config",
                &app::model::payments::WaitScreenInstructions::get_poll_config,
                &app::model::payments::WaitScreenInstructions::set_poll_config,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "wait_screen_instructions";
    }
};

template<>
struct serde::Serializable<app::model::payments::SdkUpiUriInformation>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<
            "sdk_uri",
            &app::model::payments::SdkUpiUriInformation::get_sdk_uri,
            &app::model::payments::SdkUpiUriInformation::set_sdk_uri,
            serde::FieldOptions{}>{}};
    }

    static constexpr std::string_view table_name()
    {
        return "sdk_upi_uri_information";
    }
};

template<>
struct serde::Serializable<app::model::payments::PollConfig>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "delay_in_secs",
                &app::model::payments::PollConfig::get_delay_in_secs,
                &app::model::payments::PollConfig::set_delay_in_secs,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "frequency",
                &app::model::payments::PollConfig::get_frequency,
                &app::model::payments::PollConfig::set_frequency,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "poll_config";
    }
};

template<>
struct serde::Serializable<app::model::payments::MultibancoTransferInstructions>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "reference",
                &app::model::payments::MultibancoTransferInstructions::get_reference,
                &app::model::payments::MultibancoTransferInstructions::set_reference,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "entity",
                &app::model::payments::MultibancoTransferInstructions::get_entity,
                &app::model::payments::MultibancoTransferInstructions::set_entity,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "multibanco_transfer_instructions";
    }
};

template<>
struct serde::Serializable<app::model::payments::AchTransfer>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "account_number",
                &app::model::payments::AchTransfer::get_account_number,
                &app::model::payments::AchTransfer::set_account_number,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "bank_name",
                &app::model::payments::AchTransfer::get_bank_name,
                &app::model::payments::AchTransfer::set_bank_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "routing_number",
                &app::model::payments::AchTransfer::get_routing_number,
                &app::model::payments::AchTransfer::set_routing_number,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "swift_code",
                &app::model::payments::AchTransfer::get_swift_code,
                &app::model::payments::AchTransfer::set_swift_code,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "ach_transfer";
    }
};

template<>
struct serde::Serializable<app::model::payments::PaymentsResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "payment_id",
                &app::model::payments::PaymentsResponse::get_payment_id,
                &app::model::payments::PaymentsResponse::set_payment_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "merchant_id",
                &app::model::payments::PaymentsResponse::get_merchant_id,
                &app::model::payments::PaymentsResponse::set_merchant_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "status",
                &app::model::payments::PaymentsResponse::get_status,
                &app::model::payments::PaymentsResponse::set_status,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "amount",
                &app::model::payments::PaymentsResponse::get_amount,
                &app::model::payments::PaymentsResponse::set_amount,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "net_amount",
                &app::model::payments::PaymentsResponse::get_net_amount,
                &app::model::payments::PaymentsResponse::set_net_amount,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "shipping_cost",
                &app::model::payments::PaymentsResponse::get_shipping_cost,
                &app::model::payments::PaymentsResponse::set_shipping_cost,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "amount_capturable",
                &app::model::payments::PaymentsResponse::get_amount_capturable,
                &app::model::payments::PaymentsResponse::set_amount_capturable,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "amount_received",
                &app::model::payments::PaymentsResponse::get_amount_received,
                &app::model::payments::PaymentsResponse::set_amount_received,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "processor_merchant_id",
                &app::model::payments::PaymentsResponse::get_processor_merchant_id,
                &app::model::payments::PaymentsResponse::set_processor_merchant_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "initiator",
                &app::model::payments::PaymentsResponse::get_initiator,
                &app::model::payments::PaymentsResponse::set_initiator,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "sdk_authorization",
                &app::model::payments::PaymentsResponse::get_sdk_authorization,
                &app::model::payments::PaymentsResponse::set_sdk_authorization,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_method_list",
                &app::model::payments::PaymentsResponse::get_payment_method_list,
                &app::model::payments::PaymentsResponse::set_payment_method_list,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "session_tokens",
                &app::model::payments::PaymentsResponse::get_session_tokens,
                &app::model::payments::PaymentsResponse::set_session_tokens,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "connector",
                &app::model::payments::PaymentsResponse::get_connector,
                &app::model::payments::PaymentsResponse::set_connector,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "state_metadata",
                &app::model::payments::PaymentsResponse::get_state_metadata,
                &app::model::payments::PaymentsResponse::set_state_metadata,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "client_secret",
                &app::model::payments::PaymentsResponse::get_client_secret,
                &app::model::payments::PaymentsResponse::set_client_secret,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "created",
                &app::model::payments::PaymentsResponse::get_created,
                &app::model::payments::PaymentsResponse::set_created,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "modified_at",
                &app::model::payments::PaymentsResponse::get_modified_at,
                &app::model::payments::PaymentsResponse::set_modified_at,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "connector_customer_id",
                &app::model::payments::PaymentsResponse::get_connector_customer_id,
                &app::model::payments::PaymentsResponse::set_connector_customer_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "currency",
                &app::model::payments::PaymentsResponse::get_currency,
                &app::model::payments::PaymentsResponse::set_currency,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "customer_id",
                &app::model::payments::PaymentsResponse::get_customer_id,
                &app::model::payments::PaymentsResponse::set_customer_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "customer",
                &app::model::payments::PaymentsResponse::get_customer,
                &app::model::payments::PaymentsResponse::set_customer,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "description",
                &app::model::payments::PaymentsResponse::get_description,
                &app::model::payments::PaymentsResponse::set_description,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "refunds",
                &app::model::payments::PaymentsResponse::get_refunds,
                &app::model::payments::PaymentsResponse::set_refunds,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "disputes",
                &app::model::payments::PaymentsResponse::get_disputes,
                &app::model::payments::PaymentsResponse::set_disputes,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "attempts",
                &app::model::payments::PaymentsResponse::get_attempts,
                &app::model::payments::PaymentsResponse::set_attempts,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "captures",
                &app::model::payments::PaymentsResponse::get_captures,
                &app::model::payments::PaymentsResponse::set_captures,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "mandate_id",
                &app::model::payments::PaymentsResponse::get_mandate_id,
                &app::model::payments::PaymentsResponse::set_mandate_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "mandate_data",
                &app::model::payments::PaymentsResponse::get_mandate_data,
                &app::model::payments::PaymentsResponse::set_mandate_data,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "setup_future_usage",
                &app::model::payments::PaymentsResponse::get_setup_future_usage,
                &app::model::payments::PaymentsResponse::set_setup_future_usage,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "off_session",
                &app::model::payments::PaymentsResponse::get_off_session,
                &app::model::payments::PaymentsResponse::set_off_session,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "capture_on",
                &app::model::payments::PaymentsResponse::get_capture_on,
                &app::model::payments::PaymentsResponse::set_capture_on,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "capture_method",
                &app::model::payments::PaymentsResponse::get_capture_method,
                &app::model::payments::PaymentsResponse::set_capture_method,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "capture_method_applied",
                &app::model::payments::PaymentsResponse::get_capture_method_applied,
                &app::model::payments::PaymentsResponse::set_capture_method_applied,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_method",
                &app::model::payments::PaymentsResponse::get_payment_method,
                &app::model::payments::PaymentsResponse::set_payment_method,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_method_data",
                &app::model::payments::PaymentsResponse::get_payment_method_data,
                &app::model::payments::PaymentsResponse::set_payment_method_data,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_token",
                &app::model::payments::PaymentsResponse::get_payment_token,
                &app::model::payments::PaymentsResponse::set_payment_token,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "shipping",
                &app::model::payments::PaymentsResponse::get_shipping,
                &app::model::payments::PaymentsResponse::set_shipping,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "billing",
                &app::model::payments::PaymentsResponse::get_billing,
                &app::model::payments::PaymentsResponse::set_billing,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "order_details",
                &app::model::payments::PaymentsResponse::get_order_details,
                &app::model::payments::PaymentsResponse::set_order_details,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "email",
                &app::model::payments::PaymentsResponse::get_email,
                &app::model::payments::PaymentsResponse::set_email,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "name",
                &app::model::payments::PaymentsResponse::get_name,
                &app::model::payments::PaymentsResponse::set_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "phone",
                &app::model::payments::PaymentsResponse::get_phone,
                &app::model::payments::PaymentsResponse::set_phone,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "return_url",
                &app::model::payments::PaymentsResponse::get_return_url,
                &app::model::payments::PaymentsResponse::set_return_url,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "authentication_type",
                &app::model::payments::PaymentsResponse::get_authentication_type,
                &app::model::payments::PaymentsResponse::set_authentication_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "statement_descriptor_name",
                &app::model::payments::PaymentsResponse::get_statement_descriptor_name,
                &app::model::payments::PaymentsResponse::set_statement_descriptor_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "statement_descriptor_suffix",
                &app::model::payments::PaymentsResponse::get_statement_descriptor_suffix,
                &app::model::payments::PaymentsResponse::set_statement_descriptor_suffix,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "next_action",
                &app::model::payments::PaymentsResponse::get_next_action,
                &app::model::payments::PaymentsResponse::set_next_action,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "cancellation_reason",
                &app::model::payments::PaymentsResponse::get_cancellation_reason,
                &app::model::payments::PaymentsResponse::set_cancellation_reason,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "error_code",
                &app::model::payments::PaymentsResponse::get_error_code,
                &app::model::payments::PaymentsResponse::set_error_code,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "error_message",
                &app::model::payments::PaymentsResponse::get_error_message,
                &app::model::payments::PaymentsResponse::set_error_message,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "unified_code",
                &app::model::payments::PaymentsResponse::get_unified_code,
                &app::model::payments::PaymentsResponse::set_unified_code,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "unified_message",
                &app::model::payments::PaymentsResponse::get_unified_message,
                &app::model::payments::PaymentsResponse::set_unified_message,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "error_details",
                &app::model::payments::PaymentsResponse::get_error_details,
                &app::model::payments::PaymentsResponse::set_error_details,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_experience",
                &app::model::payments::PaymentsResponse::get_payment_experience,
                &app::model::payments::PaymentsResponse::set_payment_experience,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_method_type",
                &app::model::payments::PaymentsResponse::get_payment_method_type,
                &app::model::payments::PaymentsResponse::set_payment_method_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "connector_label",
                &app::model::payments::PaymentsResponse::get_connector_label,
                &app::model::payments::PaymentsResponse::set_connector_label,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "business_country",
                &app::model::payments::PaymentsResponse::get_business_country,
                &app::model::payments::PaymentsResponse::set_business_country,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "business_label",
                &app::model::payments::PaymentsResponse::get_business_label,
                &app::model::payments::PaymentsResponse::set_business_label,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "business_sub_label",
                &app::model::payments::PaymentsResponse::get_business_sub_label,
                &app::model::payments::PaymentsResponse::set_business_sub_label,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "allowed_payment_method_types",
                &app::model::payments::PaymentsResponse::get_allowed_payment_method_types,
                &app::model::payments::PaymentsResponse::set_allowed_payment_method_types,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "manual_retry_allowed",
                &app::model::payments::PaymentsResponse::get_manual_retry_allowed,
                &app::model::payments::PaymentsResponse::set_manual_retry_allowed,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "connector_transaction_id",
                &app::model::payments::PaymentsResponse::get_connector_transaction_id,
                &app::model::payments::PaymentsResponse::set_connector_transaction_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "frm_message",
                &app::model::payments::PaymentsResponse::get_frm_message,
                &app::model::payments::PaymentsResponse::set_frm_message,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "metadata",
                &app::model::payments::PaymentsResponse::get_metadata,
                &app::model::payments::PaymentsResponse::set_metadata,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "connector_metadata",
                &app::model::payments::PaymentsResponse::get_connector_metadata,
                &app::model::payments::PaymentsResponse::set_connector_metadata,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "connector_response_metadata",
                &app::model::payments::PaymentsResponse::get_connector_response_metadata,
                &app::model::payments::PaymentsResponse::set_connector_response_metadata,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "feature_metadata",
                &app::model::payments::PaymentsResponse::get_feature_metadata,
                &app::model::payments::PaymentsResponse::set_feature_metadata,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "reference_id",
                &app::model::payments::PaymentsResponse::get_reference_id,
                &app::model::payments::PaymentsResponse::set_reference_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_link",
                &app::model::payments::PaymentsResponse::get_payment_link,
                &app::model::payments::PaymentsResponse::set_payment_link,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "profile_id",
                &app::model::payments::PaymentsResponse::get_profile_id,
                &app::model::payments::PaymentsResponse::set_profile_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "surcharge_details",
                &app::model::payments::PaymentsResponse::get_surcharge_details,
                &app::model::payments::PaymentsResponse::set_surcharge_details,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "applied_offer",
                &app::model::payments::PaymentsResponse::get_applied_offer,
                &app::model::payments::PaymentsResponse::set_applied_offer,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "attempt_count",
                &app::model::payments::PaymentsResponse::get_attempt_count,
                &app::model::payments::PaymentsResponse::set_attempt_count,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "merchant_decision",
                &app::model::payments::PaymentsResponse::get_merchant_decision,
                &app::model::payments::PaymentsResponse::set_merchant_decision,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "merchant_connector_id",
                &app::model::payments::PaymentsResponse::get_merchant_connector_id,
                &app::model::payments::PaymentsResponse::set_merchant_connector_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "incremental_authorization_allowed",
                &app::model::payments::PaymentsResponse::get_incremental_authorization_allowed,
                &app::model::payments::PaymentsResponse::set_incremental_authorization_allowed,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "authorization_count",
                &app::model::payments::PaymentsResponse::get_authorization_count,
                &app::model::payments::PaymentsResponse::set_authorization_count,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "incremental_authorizations",
                &app::model::payments::PaymentsResponse::get_incremental_authorizations,
                &app::model::payments::PaymentsResponse::set_incremental_authorizations,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "external_authentication_details",
                &app::model::payments::PaymentsResponse::get_external_authentication_details,
                &app::model::payments::PaymentsResponse::set_external_authentication_details,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "external_3ds_authentication_attempted",
                &app::model::payments::PaymentsResponse::get_external_3ds_authentication_attempted,
                &app::model::payments::PaymentsResponse::set_external_3ds_authentication_attempted,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "expires_on",
                &app::model::payments::PaymentsResponse::get_expires_on,
                &app::model::payments::PaymentsResponse::set_expires_on,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "fingerprint",
                &app::model::payments::PaymentsResponse::get_fingerprint,
                &app::model::payments::PaymentsResponse::set_fingerprint,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "fingerprint_type",
                &app::model::payments::PaymentsResponse::get_fingerprint_type,
                &app::model::payments::PaymentsResponse::set_fingerprint_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "browser_info",
                &app::model::payments::PaymentsResponse::get_browser_info,
                &app::model::payments::PaymentsResponse::set_browser_info,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_channel",
                &app::model::payments::PaymentsResponse::get_payment_channel,
                &app::model::payments::PaymentsResponse::set_payment_channel,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_method_id",
                &app::model::payments::PaymentsResponse::get_payment_method_id,
                &app::model::payments::PaymentsResponse::set_payment_method_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "network_transaction_id",
                &app::model::payments::PaymentsResponse::get_network_transaction_id,
                &app::model::payments::PaymentsResponse::set_network_transaction_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_account_reference",
                &app::model::payments::PaymentsResponse::get_payment_account_reference,
                &app::model::payments::PaymentsResponse::set_payment_account_reference,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "network_transaction_link_id",
                &app::model::payments::PaymentsResponse::get_network_transaction_link_id,
                &app::model::payments::PaymentsResponse::set_network_transaction_link_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_method_status",
                &app::model::payments::PaymentsResponse::get_payment_method_status,
                &app::model::payments::PaymentsResponse::set_payment_method_status,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "updated",
                &app::model::payments::PaymentsResponse::get_updated,
                &app::model::payments::PaymentsResponse::set_updated,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "split_payments",
                &app::model::payments::PaymentsResponse::get_split_payments,
                &app::model::payments::PaymentsResponse::set_split_payments,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "frm_metadata",
                &app::model::payments::PaymentsResponse::get_frm_metadata,
                &app::model::payments::PaymentsResponse::set_frm_metadata,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "extended_authorization_applied",
                &app::model::payments::PaymentsResponse::get_extended_authorization_applied,
                &app::model::payments::PaymentsResponse::set_extended_authorization_applied,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "extended_authorization_last_applied_at",
                &app::model::payments::PaymentsResponse::get_extended_authorization_last_applied_at,
                &app::model::payments::PaymentsResponse::set_extended_authorization_last_applied_at,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "request_extended_authorization",
                &app::model::payments::PaymentsResponse::get_request_extended_authorization,
                &app::model::payments::PaymentsResponse::set_request_extended_authorization,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "capture_before",
                &app::model::payments::PaymentsResponse::get_capture_before,
                &app::model::payments::PaymentsResponse::set_capture_before,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "merchant_order_reference_id",
                &app::model::payments::PaymentsResponse::get_merchant_order_reference_id,
                &app::model::payments::PaymentsResponse::set_merchant_order_reference_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "order_tax_amount",
                &app::model::payments::PaymentsResponse::get_order_tax_amount,
                &app::model::payments::PaymentsResponse::set_order_tax_amount,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "connector_mandate_id",
                &app::model::payments::PaymentsResponse::get_connector_mandate_id,
                &app::model::payments::PaymentsResponse::set_connector_mandate_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_discovery",
                &app::model::payments::PaymentsResponse::get_card_discovery,
                &app::model::payments::PaymentsResponse::set_card_discovery,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "force_3ds_challenge",
                &app::model::payments::PaymentsResponse::get_force_3ds_challenge,
                &app::model::payments::PaymentsResponse::set_force_3ds_challenge,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "force_3ds_challenge_trigger",
                &app::model::payments::PaymentsResponse::get_force_3ds_challenge_trigger,
                &app::model::payments::PaymentsResponse::set_force_3ds_challenge_trigger,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "issuer_error_code",
                &app::model::payments::PaymentsResponse::get_issuer_error_code,
                &app::model::payments::PaymentsResponse::set_issuer_error_code,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "issuer_error_message",
                &app::model::payments::PaymentsResponse::get_issuer_error_message,
                &app::model::payments::PaymentsResponse::set_issuer_error_message,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "is_iframe_redirection_enabled",
                &app::model::payments::PaymentsResponse::get_is_iframe_redirection_enabled,
                &app::model::payments::PaymentsResponse::set_is_iframe_redirection_enabled,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "whole_connector_response",
                &app::model::payments::PaymentsResponse::get_whole_connector_response,
                &app::model::payments::PaymentsResponse::set_whole_connector_response,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "enable_partial_authorization",
                &app::model::payments::PaymentsResponse::get_enable_partial_authorization,
                &app::model::payments::PaymentsResponse::set_enable_partial_authorization,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "enable_overcapture",
                &app::model::payments::PaymentsResponse::get_enable_overcapture,
                &app::model::payments::PaymentsResponse::set_enable_overcapture,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "is_overcapture_enabled",
                &app::model::payments::PaymentsResponse::get_is_overcapture_enabled,
                &app::model::payments::PaymentsResponse::set_is_overcapture_enabled,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "network_details",
                &app::model::payments::PaymentsResponse::get_network_details,
                &app::model::payments::PaymentsResponse::set_network_details,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "is_stored_credential",
                &app::model::payments::PaymentsResponse::get_is_stored_credential,
                &app::model::payments::PaymentsResponse::set_is_stored_credential,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "mit_category",
                &app::model::payments::PaymentsResponse::get_mit_category,
                &app::model::payments::PaymentsResponse::set_mit_category,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "billing_descriptor",
                &app::model::payments::PaymentsResponse::get_billing_descriptor,
                &app::model::payments::PaymentsResponse::set_billing_descriptor,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "is_account_funded_transaction",
                &app::model::payments::PaymentsResponse::get_is_account_funded_transaction,
                &app::model::payments::PaymentsResponse::set_is_account_funded_transaction,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "recipient_details",
                &app::model::payments::PaymentsResponse::get_recipient_details,
                &app::model::payments::PaymentsResponse::set_recipient_details,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "tokenization",
                &app::model::payments::PaymentsResponse::get_tokenization,
                &app::model::payments::PaymentsResponse::set_tokenization,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "partner_merchant_identifier_details",
                &app::model::payments::PaymentsResponse::get_partner_merchant_identifier_details,
                &app::model::payments::PaymentsResponse::set_partner_merchant_identifier_details,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_method_tokenization_details",
                &app::model::payments::PaymentsResponse::get_payment_method_tokenization_details,
                &app::model::payments::PaymentsResponse::set_payment_method_tokenization_details,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "installment_options",
                &app::model::payments::PaymentsResponse::get_installment_options,
                &app::model::payments::PaymentsResponse::set_installment_options,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "installment_data",
                &app::model::payments::PaymentsResponse::get_installment_data,
                &app::model::payments::PaymentsResponse::set_installment_data,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "sender_payment_instrument_id",
                &app::model::payments::PaymentsResponse::get_sender_payment_instrument_id,
                &app::model::payments::PaymentsResponse::set_sender_payment_instrument_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "enable_avs_check",
                &app::model::payments::PaymentsResponse::get_enable_avs_check,
                &app::model::payments::PaymentsResponse::set_enable_avs_check,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "enable_account_name_verification",
                &app::model::payments::PaymentsResponse::get_enable_account_name_verification,
                &app::model::payments::PaymentsResponse::set_enable_account_name_verification,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "payments_response";
    }
};

template<>
struct serde::Serializable<app::model::payments::PaymentMethodTokenizationDetails>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "payment_method_id",
                &app::model::payments::PaymentMethodTokenizationDetails::get_payment_method_id,
                &app::model::payments::PaymentMethodTokenizationDetails::set_payment_method_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_method_status",
                &app::model::payments::PaymentMethodTokenizationDetails::get_payment_method_status,
                &app::model::payments::PaymentMethodTokenizationDetails::set_payment_method_status,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "psp_tokenization",
                &app::model::payments::PaymentMethodTokenizationDetails::get_psp_tokenization,
                &app::model::payments::PaymentMethodTokenizationDetails::set_psp_tokenization,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "network_tokenization",
                &app::model::payments::PaymentMethodTokenizationDetails::get_network_tokenization,
                &app::model::payments::PaymentMethodTokenizationDetails::set_network_tokenization,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "network_transaction_id",
                &app::model::payments::PaymentMethodTokenizationDetails::get_network_transaction_id,
                &app::model::payments::PaymentMethodTokenizationDetails::set_network_transaction_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "network_transaction_link_id",
                &app::model::payments::PaymentMethodTokenizationDetails::
                    get_network_transaction_link_id,
                &app::model::payments::PaymentMethodTokenizationDetails::
                    set_network_transaction_link_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "is_eligible_for_mit_payment",
                &app::model::payments::PaymentMethodTokenizationDetails::
                    get_is_eligible_for_mit_payment,
                &app::model::payments::PaymentMethodTokenizationDetails::
                    set_is_eligible_for_mit_payment,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "payment_method_tokenization_details";
    }
};

template<>
struct serde::Serializable<app::model::payments::PaymentsListResponseItem>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "id",
                &app::model::payments::PaymentsListResponseItem::get_id,
                &app::model::payments::PaymentsListResponseItem::set_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "merchant_id",
                &app::model::payments::PaymentsListResponseItem::get_merchant_id,
                &app::model::payments::PaymentsListResponseItem::set_merchant_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "profile_id",
                &app::model::payments::PaymentsListResponseItem::get_profile_id,
                &app::model::payments::PaymentsListResponseItem::set_profile_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "customer_id",
                &app::model::payments::PaymentsListResponseItem::get_customer_id,
                &app::model::payments::PaymentsListResponseItem::set_customer_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_method_id",
                &app::model::payments::PaymentsListResponseItem::get_payment_method_id,
                &app::model::payments::PaymentsListResponseItem::set_payment_method_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "status",
                &app::model::payments::PaymentsListResponseItem::get_status,
                &app::model::payments::PaymentsListResponseItem::set_status,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "amount",
                &app::model::payments::PaymentsListResponseItem::get_amount,
                &app::model::payments::PaymentsListResponseItem::set_amount,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "created",
                &app::model::payments::PaymentsListResponseItem::get_created,
                &app::model::payments::PaymentsListResponseItem::set_created,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_method_type",
                &app::model::payments::PaymentsListResponseItem::get_payment_method_type,
                &app::model::payments::PaymentsListResponseItem::set_payment_method_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_method_subtype",
                &app::model::payments::PaymentsListResponseItem::get_payment_method_subtype,
                &app::model::payments::PaymentsListResponseItem::set_payment_method_subtype,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "connector",
                &app::model::payments::PaymentsListResponseItem::get_connector,
                &app::model::payments::PaymentsListResponseItem::set_connector,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "merchant_connector_id",
                &app::model::payments::PaymentsListResponseItem::get_merchant_connector_id,
                &app::model::payments::PaymentsListResponseItem::set_merchant_connector_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "customer",
                &app::model::payments::PaymentsListResponseItem::get_customer,
                &app::model::payments::PaymentsListResponseItem::set_customer,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "merchant_reference_id",
                &app::model::payments::PaymentsListResponseItem::get_merchant_reference_id,
                &app::model::payments::PaymentsListResponseItem::set_merchant_reference_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "connector_payment_id",
                &app::model::payments::PaymentsListResponseItem::get_connector_payment_id,
                &app::model::payments::PaymentsListResponseItem::set_connector_payment_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "connector_response_reference_id",
                &app::model::payments::PaymentsListResponseItem::
                    get_connector_response_reference_id,
                &app::model::payments::PaymentsListResponseItem::
                    set_connector_response_reference_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "metadata",
                &app::model::payments::PaymentsListResponseItem::get_metadata,
                &app::model::payments::PaymentsListResponseItem::set_metadata,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "description",
                &app::model::payments::PaymentsListResponseItem::get_description,
                &app::model::payments::PaymentsListResponseItem::set_description,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "authentication_type",
                &app::model::payments::PaymentsListResponseItem::get_authentication_type,
                &app::model::payments::PaymentsListResponseItem::set_authentication_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "capture_method",
                &app::model::payments::PaymentsListResponseItem::get_capture_method,
                &app::model::payments::PaymentsListResponseItem::set_capture_method,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "setup_future_usage",
                &app::model::payments::PaymentsListResponseItem::get_setup_future_usage,
                &app::model::payments::PaymentsListResponseItem::set_setup_future_usage,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "attempt_count",
                &app::model::payments::PaymentsListResponseItem::get_attempt_count,
                &app::model::payments::PaymentsListResponseItem::set_attempt_count,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "error",
                &app::model::payments::PaymentsListResponseItem::get_error,
                &app::model::payments::PaymentsListResponseItem::set_error,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "cancellation_reason",
                &app::model::payments::PaymentsListResponseItem::get_cancellation_reason,
                &app::model::payments::PaymentsListResponseItem::set_cancellation_reason,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "order_details",
                &app::model::payments::PaymentsListResponseItem::get_order_details,
                &app::model::payments::PaymentsListResponseItem::set_order_details,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "return_url",
                &app::model::payments::PaymentsListResponseItem::get_return_url,
                &app::model::payments::PaymentsListResponseItem::set_return_url,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "statement_descriptor",
                &app::model::payments::PaymentsListResponseItem::get_statement_descriptor,
                &app::model::payments::PaymentsListResponseItem::set_statement_descriptor,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "allowed_payment_method_types",
                &app::model::payments::PaymentsListResponseItem::get_allowed_payment_method_types,
                &app::model::payments::PaymentsListResponseItem::set_allowed_payment_method_types,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "authorization_count",
                &app::model::payments::PaymentsListResponseItem::get_authorization_count,
                &app::model::payments::PaymentsListResponseItem::set_authorization_count,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "modified_at",
                &app::model::payments::PaymentsListResponseItem::get_modified_at,
                &app::model::payments::PaymentsListResponseItem::set_modified_at,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "is_split_payment",
                &app::model::payments::PaymentsListResponseItem::get_is_split_payment,
                &app::model::payments::PaymentsListResponseItem::set_is_split_payment,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "payments_list_response_item";
    }
};

template<>
struct serde::Serializable<app::model::payments::RecoveryPaymentsListResponseItem>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "id",
                &app::model::payments::RecoveryPaymentsListResponseItem::get_id,
                &app::model::payments::RecoveryPaymentsListResponseItem::set_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "merchant_id",
                &app::model::payments::RecoveryPaymentsListResponseItem::get_merchant_id,
                &app::model::payments::RecoveryPaymentsListResponseItem::set_merchant_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "profile_id",
                &app::model::payments::RecoveryPaymentsListResponseItem::get_profile_id,
                &app::model::payments::RecoveryPaymentsListResponseItem::set_profile_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "customer_id",
                &app::model::payments::RecoveryPaymentsListResponseItem::get_customer_id,
                &app::model::payments::RecoveryPaymentsListResponseItem::set_customer_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "status",
                &app::model::payments::RecoveryPaymentsListResponseItem::get_status,
                &app::model::payments::RecoveryPaymentsListResponseItem::set_status,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "amount",
                &app::model::payments::RecoveryPaymentsListResponseItem::get_amount,
                &app::model::payments::RecoveryPaymentsListResponseItem::set_amount,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "created",
                &app::model::payments::RecoveryPaymentsListResponseItem::get_created,
                &app::model::payments::RecoveryPaymentsListResponseItem::set_created,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_method_type",
                &app::model::payments::RecoveryPaymentsListResponseItem::get_payment_method_type,
                &app::model::payments::RecoveryPaymentsListResponseItem::set_payment_method_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_method_subtype",
                &app::model::payments::RecoveryPaymentsListResponseItem::get_payment_method_subtype,
                &app::model::payments::RecoveryPaymentsListResponseItem::set_payment_method_subtype,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "connector",
                &app::model::payments::RecoveryPaymentsListResponseItem::get_connector,
                &app::model::payments::RecoveryPaymentsListResponseItem::set_connector,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "merchant_connector_id",
                &app::model::payments::RecoveryPaymentsListResponseItem::get_merchant_connector_id,
                &app::model::payments::RecoveryPaymentsListResponseItem::set_merchant_connector_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "customer",
                &app::model::payments::RecoveryPaymentsListResponseItem::get_customer,
                &app::model::payments::RecoveryPaymentsListResponseItem::set_customer,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "merchant_reference_id",
                &app::model::payments::RecoveryPaymentsListResponseItem::get_merchant_reference_id,
                &app::model::payments::RecoveryPaymentsListResponseItem::set_merchant_reference_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "description",
                &app::model::payments::RecoveryPaymentsListResponseItem::get_description,
                &app::model::payments::RecoveryPaymentsListResponseItem::set_description,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "attempt_count",
                &app::model::payments::RecoveryPaymentsListResponseItem::get_attempt_count,
                &app::model::payments::RecoveryPaymentsListResponseItem::set_attempt_count,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "error",
                &app::model::payments::RecoveryPaymentsListResponseItem::get_error,
                &app::model::payments::RecoveryPaymentsListResponseItem::set_error,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "cancellation_reason",
                &app::model::payments::RecoveryPaymentsListResponseItem::get_cancellation_reason,
                &app::model::payments::RecoveryPaymentsListResponseItem::set_cancellation_reason,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "modified_at",
                &app::model::payments::RecoveryPaymentsListResponseItem::get_modified_at,
                &app::model::payments::RecoveryPaymentsListResponseItem::set_modified_at,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "last_attempt_at",
                &app::model::payments::RecoveryPaymentsListResponseItem::get_last_attempt_at,
                &app::model::payments::RecoveryPaymentsListResponseItem::set_last_attempt_at,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "recovery_payments_list_response_item";
    }
};

template<>
struct serde::Serializable<app::model::payments::PaymentsConfirmIntentRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "return_url",
                &app::model::payments::PaymentsConfirmIntentRequest::get_return_url,
                &app::model::payments::PaymentsConfirmIntentRequest::set_return_url,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_method_data",
                &app::model::payments::PaymentsConfirmIntentRequest::get_payment_method_data,
                &app::model::payments::PaymentsConfirmIntentRequest::set_payment_method_data,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "split_payment_method_data",
                &app::model::payments::PaymentsConfirmIntentRequest::get_split_payment_method_data,
                &app::model::payments::PaymentsConfirmIntentRequest::set_split_payment_method_data,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_method_type",
                &app::model::payments::PaymentsConfirmIntentRequest::get_payment_method_type,
                &app::model::payments::PaymentsConfirmIntentRequest::set_payment_method_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_method_subtype",
                &app::model::payments::PaymentsConfirmIntentRequest::get_payment_method_subtype,
                &app::model::payments::PaymentsConfirmIntentRequest::set_payment_method_subtype,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "shipping",
                &app::model::payments::PaymentsConfirmIntentRequest::get_shipping,
                &app::model::payments::PaymentsConfirmIntentRequest::set_shipping,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "customer_acceptance",
                &app::model::payments::PaymentsConfirmIntentRequest::get_customer_acceptance,
                &app::model::payments::PaymentsConfirmIntentRequest::set_customer_acceptance,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "browser_info",
                &app::model::payments::PaymentsConfirmIntentRequest::get_browser_info,
                &app::model::payments::PaymentsConfirmIntentRequest::set_browser_info,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_method_id",
                &app::model::payments::PaymentsConfirmIntentRequest::get_payment_method_id,
                &app::model::payments::PaymentsConfirmIntentRequest::set_payment_method_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_token",
                &app::model::payments::PaymentsConfirmIntentRequest::get_payment_token,
                &app::model::payments::PaymentsConfirmIntentRequest::set_payment_token,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "merchant_connector_details",
                &app::model::payments::PaymentsConfirmIntentRequest::get_merchant_connector_details,
                &app::model::payments::PaymentsConfirmIntentRequest::set_merchant_connector_details,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "return_raw_connector_response",
                &app::model::payments::PaymentsConfirmIntentRequest::
                    get_return_raw_connector_response,
                &app::model::payments::PaymentsConfirmIntentRequest::
                    set_return_raw_connector_response,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "webhook_url",
                &app::model::payments::PaymentsConfirmIntentRequest::get_webhook_url,
                &app::model::payments::PaymentsConfirmIntentRequest::set_webhook_url,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "enable_avs_check",
                &app::model::payments::PaymentsConfirmIntentRequest::get_enable_avs_check,
                &app::model::payments::PaymentsConfirmIntentRequest::set_enable_avs_check,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "enable_account_name_verification",
                &app::model::payments::PaymentsConfirmIntentRequest::
                    get_enable_account_name_verification,
                &app::model::payments::PaymentsConfirmIntentRequest::
                    set_enable_account_name_verification,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "payments_confirm_intent_request";
    }
};

template<>
struct serde::Serializable<app::model::payments::ProxyPaymentsRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "return_url",
                &app::model::payments::ProxyPaymentsRequest::get_return_url,
                &app::model::payments::ProxyPaymentsRequest::set_return_url,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "amount",
                &app::model::payments::ProxyPaymentsRequest::get_amount,
                &app::model::payments::ProxyPaymentsRequest::set_amount,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "recurring_details",
                &app::model::payments::ProxyPaymentsRequest::get_recurring_details,
                &app::model::payments::ProxyPaymentsRequest::set_recurring_details,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "shipping",
                &app::model::payments::ProxyPaymentsRequest::get_shipping,
                &app::model::payments::ProxyPaymentsRequest::set_shipping,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "browser_info",
                &app::model::payments::ProxyPaymentsRequest::get_browser_info,
                &app::model::payments::ProxyPaymentsRequest::set_browser_info,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "connector",
                &app::model::payments::ProxyPaymentsRequest::get_connector,
                &app::model::payments::ProxyPaymentsRequest::set_connector,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "merchant_connector_id",
                &app::model::payments::ProxyPaymentsRequest::get_merchant_connector_id,
                &app::model::payments::ProxyPaymentsRequest::set_merchant_connector_id,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "proxy_payments_request";
    }
};

template<>
struct serde::Serializable<app::model::payments::ExternalVaultProxyPaymentsRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "return_url",
                &app::model::payments::ExternalVaultProxyPaymentsRequest::get_return_url,
                &app::model::payments::ExternalVaultProxyPaymentsRequest::set_return_url,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_method_data",
                &app::model::payments::ExternalVaultProxyPaymentsRequest::get_payment_method_data,
                &app::model::payments::ExternalVaultProxyPaymentsRequest::set_payment_method_data,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_method_type",
                &app::model::payments::ExternalVaultProxyPaymentsRequest::get_payment_method_type,
                &app::model::payments::ExternalVaultProxyPaymentsRequest::set_payment_method_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_method_subtype",
                &app::model::payments::ExternalVaultProxyPaymentsRequest::
                    get_payment_method_subtype,
                &app::model::payments::ExternalVaultProxyPaymentsRequest::
                    set_payment_method_subtype,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "shipping",
                &app::model::payments::ExternalVaultProxyPaymentsRequest::get_shipping,
                &app::model::payments::ExternalVaultProxyPaymentsRequest::set_shipping,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "customer_acceptance",
                &app::model::payments::ExternalVaultProxyPaymentsRequest::get_customer_acceptance,
                &app::model::payments::ExternalVaultProxyPaymentsRequest::set_customer_acceptance,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "browser_info",
                &app::model::payments::ExternalVaultProxyPaymentsRequest::get_browser_info,
                &app::model::payments::ExternalVaultProxyPaymentsRequest::set_browser_info,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_method_id",
                &app::model::payments::ExternalVaultProxyPaymentsRequest::get_payment_method_id,
                &app::model::payments::ExternalVaultProxyPaymentsRequest::set_payment_method_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_token",
                &app::model::payments::ExternalVaultProxyPaymentsRequest::get_payment_token,
                &app::model::payments::ExternalVaultProxyPaymentsRequest::set_payment_token,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "merchant_connector_details",
                &app::model::payments::ExternalVaultProxyPaymentsRequest::
                    get_merchant_connector_details,
                &app::model::payments::ExternalVaultProxyPaymentsRequest::
                    set_merchant_connector_details,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "return_raw_connector_response",
                &app::model::payments::ExternalVaultProxyPaymentsRequest::
                    get_return_raw_connector_response,
                &app::model::payments::ExternalVaultProxyPaymentsRequest::
                    set_return_raw_connector_response,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "external_vault_proxy_payments_request";
    }
};

template<>
struct serde::Serializable<app::model::payments::PaymentsRetrieveRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "resource_id",
                &app::model::payments::PaymentsRetrieveRequest::get_resource_id,
                &app::model::payments::PaymentsRetrieveRequest::set_resource_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "merchant_id",
                &app::model::payments::PaymentsRetrieveRequest::get_merchant_id,
                &app::model::payments::PaymentsRetrieveRequest::set_merchant_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "force_sync",
                &app::model::payments::PaymentsRetrieveRequest::get_force_sync,
                &app::model::payments::PaymentsRetrieveRequest::set_force_sync,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "param",
                &app::model::payments::PaymentsRetrieveRequest::get_param,
                &app::model::payments::PaymentsRetrieveRequest::set_param,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "connector",
                &app::model::payments::PaymentsRetrieveRequest::get_connector,
                &app::model::payments::PaymentsRetrieveRequest::set_connector,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "merchant_connector_details",
                &app::model::payments::PaymentsRetrieveRequest::get_merchant_connector_details,
                &app::model::payments::PaymentsRetrieveRequest::set_merchant_connector_details,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "client_secret",
                &app::model::payments::PaymentsRetrieveRequest::get_client_secret,
                &app::model::payments::PaymentsRetrieveRequest::set_client_secret,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "expand_captures",
                &app::model::payments::PaymentsRetrieveRequest::get_expand_captures,
                &app::model::payments::PaymentsRetrieveRequest::set_expand_captures,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "expand_attempts",
                &app::model::payments::PaymentsRetrieveRequest::get_expand_attempts,
                &app::model::payments::PaymentsRetrieveRequest::set_expand_attempts,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "all_keys_required",
                &app::model::payments::PaymentsRetrieveRequest::get_all_keys_required,
                &app::model::payments::PaymentsRetrieveRequest::set_all_keys_required,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "payments_retrieve_request";
    }
};

template<>
struct serde::Serializable<app::model::payments::PaymentsStatusRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "force_sync",
                &app::model::payments::PaymentsStatusRequest::get_force_sync,
                &app::model::payments::PaymentsStatusRequest::set_force_sync,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "expand_attempts",
                &app::model::payments::PaymentsStatusRequest::get_expand_attempts,
                &app::model::payments::PaymentsStatusRequest::set_expand_attempts,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "param",
                &app::model::payments::PaymentsStatusRequest::get_param,
                &app::model::payments::PaymentsStatusRequest::set_param,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "return_raw_connector_response",
                &app::model::payments::PaymentsStatusRequest::get_return_raw_connector_response,
                &app::model::payments::PaymentsStatusRequest::set_return_raw_connector_response,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "payments_status_request";
    }
};

template<>
struct serde::Serializable<app::model::payments::PaymentErrorDetails>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "unified_details",
                &app::model::payments::PaymentErrorDetails::get_unified_details,
                &app::model::payments::PaymentErrorDetails::set_unified_details,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "issuer_details",
                &app::model::payments::PaymentErrorDetails::get_issuer_details,
                &app::model::payments::PaymentErrorDetails::set_issuer_details,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "connector_details",
                &app::model::payments::PaymentErrorDetails::get_connector_details,
                &app::model::payments::PaymentErrorDetails::set_connector_details,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "payment_error_details";
    }
};

template<>
struct serde::Serializable<app::model::payments::ApiUnifiedErrorDetails>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "category",
                &app::model::payments::ApiUnifiedErrorDetails::get_category,
                &app::model::payments::ApiUnifiedErrorDetails::set_category,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "message",
                &app::model::payments::ApiUnifiedErrorDetails::get_message,
                &app::model::payments::ApiUnifiedErrorDetails::set_message,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "standardised_code",
                &app::model::payments::ApiUnifiedErrorDetails::get_standardised_code,
                &app::model::payments::ApiUnifiedErrorDetails::set_standardised_code,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "description",
                &app::model::payments::ApiUnifiedErrorDetails::get_description,
                &app::model::payments::ApiUnifiedErrorDetails::set_description,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "user_guidance_message",
                &app::model::payments::ApiUnifiedErrorDetails::get_user_guidance_message,
                &app::model::payments::ApiUnifiedErrorDetails::set_user_guidance_message,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "recommended_action",
                &app::model::payments::ApiUnifiedErrorDetails::get_recommended_action,
                &app::model::payments::ApiUnifiedErrorDetails::set_recommended_action,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "api_unified_error_details";
    }
};

template<>
struct serde::Serializable<app::model::payments::ApiIssuerErrorDetails>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "code",
                &app::model::payments::ApiIssuerErrorDetails::get_code,
                &app::model::payments::ApiIssuerErrorDetails::set_code,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "message",
                &app::model::payments::ApiIssuerErrorDetails::get_message,
                &app::model::payments::ApiIssuerErrorDetails::set_message,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "network_details",
                &app::model::payments::ApiIssuerErrorDetails::get_network_details,
                &app::model::payments::ApiIssuerErrorDetails::set_network_details,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "api_issuer_error_details";
    }
};

template<>
struct serde::Serializable<app::model::payments::ApiNetworkErrorDetails>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "name",
                &app::model::payments::ApiNetworkErrorDetails::get_name,
                &app::model::payments::ApiNetworkErrorDetails::set_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "advice_code",
                &app::model::payments::ApiNetworkErrorDetails::get_advice_code,
                &app::model::payments::ApiNetworkErrorDetails::set_advice_code,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "advice_message",
                &app::model::payments::ApiNetworkErrorDetails::get_advice_message,
                &app::model::payments::ApiNetworkErrorDetails::set_advice_message,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "api_network_error_details";
    }
};

template<>
struct serde::Serializable<app::model::payments::ErrorDetails>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "code",
                &app::model::payments::ErrorDetails::get_code,
                &app::model::payments::ErrorDetails::set_code,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "message",
                &app::model::payments::ErrorDetails::get_message,
                &app::model::payments::ErrorDetails::set_message,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "reason",
                &app::model::payments::ErrorDetails::get_reason,
                &app::model::payments::ErrorDetails::set_reason,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "unified_code",
                &app::model::payments::ErrorDetails::get_unified_code,
                &app::model::payments::ErrorDetails::set_unified_code,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "unified_message",
                &app::model::payments::ErrorDetails::get_unified_message,
                &app::model::payments::ErrorDetails::set_unified_message,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "network_advice_code",
                &app::model::payments::ErrorDetails::get_network_advice_code,
                &app::model::payments::ErrorDetails::set_network_advice_code,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "network_decline_code",
                &app::model::payments::ErrorDetails::get_network_decline_code,
                &app::model::payments::ErrorDetails::set_network_decline_code,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "network_error_message",
                &app::model::payments::ErrorDetails::get_network_error_message,
                &app::model::payments::ErrorDetails::set_network_error_message,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "error_details";
    }
};

template<>
struct serde::Serializable<app::model::payments::ExternalAuthenticationDetailsResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "authentication_flow",
                &app::model::payments::ExternalAuthenticationDetailsResponse::
                    get_authentication_flow,
                &app::model::payments::ExternalAuthenticationDetailsResponse::
                    set_authentication_flow,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "electronic_commerce_indicator",
                &app::model::payments::ExternalAuthenticationDetailsResponse::
                    get_electronic_commerce_indicator,
                &app::model::payments::ExternalAuthenticationDetailsResponse::
                    set_electronic_commerce_indicator,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "status",
                &app::model::payments::ExternalAuthenticationDetailsResponse::get_status,
                &app::model::payments::ExternalAuthenticationDetailsResponse::set_status,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "ds_transaction_id",
                &app::model::payments::ExternalAuthenticationDetailsResponse::get_ds_transaction_id,
                &app::model::payments::ExternalAuthenticationDetailsResponse::set_ds_transaction_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "version",
                &app::model::payments::ExternalAuthenticationDetailsResponse::get_version,
                &app::model::payments::ExternalAuthenticationDetailsResponse::set_version,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "error_code",
                &app::model::payments::ExternalAuthenticationDetailsResponse::get_error_code,
                &app::model::payments::ExternalAuthenticationDetailsResponse::set_error_code,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "error_message",
                &app::model::payments::ExternalAuthenticationDetailsResponse::get_error_message,
                &app::model::payments::ExternalAuthenticationDetailsResponse::set_error_message,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "challenge_cancel_code",
                &app::model::payments::ExternalAuthenticationDetailsResponse::
                    get_challenge_cancel_code,
                &app::model::payments::ExternalAuthenticationDetailsResponse::
                    set_challenge_cancel_code,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "trans_status_reason",
                &app::model::payments::ExternalAuthenticationDetailsResponse::
                    get_trans_status_reason,
                &app::model::payments::ExternalAuthenticationDetailsResponse::
                    set_trans_status_reason,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "external_authentication_details_response";
    }
};

template<>
struct serde::Serializable<app::model::payments::PaymentListConstraints>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "payment_id",
                &app::model::payments::PaymentListConstraints::get_payment_id,
                &app::model::payments::PaymentListConstraints::set_payment_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "profile_id",
                &app::model::payments::PaymentListConstraints::get_profile_id,
                &app::model::payments::PaymentListConstraints::set_profile_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "customer_id",
                &app::model::payments::PaymentListConstraints::get_customer_id,
                &app::model::payments::PaymentListConstraints::set_customer_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "starting_after",
                &app::model::payments::PaymentListConstraints::get_starting_after,
                &app::model::payments::PaymentListConstraints::set_starting_after,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "ending_before",
                &app::model::payments::PaymentListConstraints::get_ending_before,
                &app::model::payments::PaymentListConstraints::set_ending_before,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "limit",
                &app::model::payments::PaymentListConstraints::get_limit,
                &app::model::payments::PaymentListConstraints::set_limit,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "offset",
                &app::model::payments::PaymentListConstraints::get_offset,
                &app::model::payments::PaymentListConstraints::set_offset,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "created",
                &app::model::payments::PaymentListConstraints::get_created,
                &app::model::payments::PaymentListConstraints::set_created,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "created.lt",
                &app::model::payments::PaymentListConstraints::get_created_lt,
                &app::model::payments::PaymentListConstraints::set_created_lt,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "created.gt",
                &app::model::payments::PaymentListConstraints::get_created_gt,
                &app::model::payments::PaymentListConstraints::set_created_gt,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "created.lte",
                &app::model::payments::PaymentListConstraints::get_created_lte,
                &app::model::payments::PaymentListConstraints::set_created_lte,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "created.gte",
                &app::model::payments::PaymentListConstraints::get_created_gte,
                &app::model::payments::PaymentListConstraints::set_created_gte,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "start_amount",
                &app::model::payments::PaymentListConstraints::get_start_amount,
                &app::model::payments::PaymentListConstraints::set_start_amount,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "end_amount",
                &app::model::payments::PaymentListConstraints::get_end_amount,
                &app::model::payments::PaymentListConstraints::set_end_amount,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "connector",
                &app::model::payments::PaymentListConstraints::get_connector,
                &app::model::payments::PaymentListConstraints::set_connector,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "currency",
                &app::model::payments::PaymentListConstraints::get_currency,
                &app::model::payments::PaymentListConstraints::set_currency,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "status",
                &app::model::payments::PaymentListConstraints::get_status,
                &app::model::payments::PaymentListConstraints::set_status,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_method_type",
                &app::model::payments::PaymentListConstraints::get_payment_method_type,
                &app::model::payments::PaymentListConstraints::set_payment_method_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_method_subtype",
                &app::model::payments::PaymentListConstraints::get_payment_method_subtype,
                &app::model::payments::PaymentListConstraints::set_payment_method_subtype,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "authentication_type",
                &app::model::payments::PaymentListConstraints::get_authentication_type,
                &app::model::payments::PaymentListConstraints::set_authentication_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "merchant_connector_id",
                &app::model::payments::PaymentListConstraints::get_merchant_connector_id,
                &app::model::payments::PaymentListConstraints::set_merchant_connector_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "order_on",
                &app::model::payments::PaymentListConstraints::get_order_on,
                &app::model::payments::PaymentListConstraints::set_order_on,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "order_by",
                &app::model::payments::PaymentListConstraints::get_order_by,
                &app::model::payments::PaymentListConstraints::set_order_by,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_network",
                &app::model::payments::PaymentListConstraints::get_card_network,
                &app::model::payments::PaymentListConstraints::set_card_network,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "merchant_order_reference_id",
                &app::model::payments::PaymentListConstraints::get_merchant_order_reference_id,
                &app::model::payments::PaymentListConstraints::set_merchant_order_reference_id,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "payment_list_constraints";
    }
};

template<>
struct serde::Serializable<app::model::payments::PaymentListResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "count",
                &app::model::payments::PaymentListResponse::get_count,
                &app::model::payments::PaymentListResponse::set_count,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "total_count",
                &app::model::payments::PaymentListResponse::get_total_count,
                &app::model::payments::PaymentListResponse::set_total_count,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "data",
                &app::model::payments::PaymentListResponse::get_data,
                &app::model::payments::PaymentListResponse::set_data,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "payment_list_response";
    }
};

template<>
struct serde::Serializable<app::model::payments::PlatformPaymentListConstraints>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "payment_id",
                &app::model::payments::PlatformPaymentListConstraints::get_payment_id,
                &app::model::payments::PlatformPaymentListConstraints::set_payment_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "profile_id",
                &app::model::payments::PlatformPaymentListConstraints::get_profile_id,
                &app::model::payments::PlatformPaymentListConstraints::set_profile_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "processor_merchant_id",
                &app::model::payments::PlatformPaymentListConstraints::get_processor_merchant_id,
                &app::model::payments::PlatformPaymentListConstraints::set_processor_merchant_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "customer_id",
                &app::model::payments::PlatformPaymentListConstraints::get_customer_id,
                &app::model::payments::PlatformPaymentListConstraints::set_customer_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "limit",
                &app::model::payments::PlatformPaymentListConstraints::get_limit,
                &app::model::payments::PlatformPaymentListConstraints::set_limit,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "offset",
                &app::model::payments::PlatformPaymentListConstraints::get_offset,
                &app::model::payments::PlatformPaymentListConstraints::set_offset,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "time_range",
                &app::model::payments::PlatformPaymentListConstraints::get_time_range,
                &app::model::payments::PlatformPaymentListConstraints::set_time_range,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "start_amount",
                &app::model::payments::PlatformPaymentListConstraints::get_start_amount,
                &app::model::payments::PlatformPaymentListConstraints::set_start_amount,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "end_amount",
                &app::model::payments::PlatformPaymentListConstraints::get_end_amount,
                &app::model::payments::PlatformPaymentListConstraints::set_end_amount,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "connector",
                &app::model::payments::PlatformPaymentListConstraints::get_connector,
                &app::model::payments::PlatformPaymentListConstraints::set_connector,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "currency",
                &app::model::payments::PlatformPaymentListConstraints::get_currency,
                &app::model::payments::PlatformPaymentListConstraints::set_currency,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "status",
                &app::model::payments::PlatformPaymentListConstraints::get_status,
                &app::model::payments::PlatformPaymentListConstraints::set_status,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_method",
                &app::model::payments::PlatformPaymentListConstraints::get_payment_method,
                &app::model::payments::PlatformPaymentListConstraints::set_payment_method,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_method_type",
                &app::model::payments::PlatformPaymentListConstraints::get_payment_method_type,
                &app::model::payments::PlatformPaymentListConstraints::set_payment_method_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "authentication_type",
                &app::model::payments::PlatformPaymentListConstraints::get_authentication_type,
                &app::model::payments::PlatformPaymentListConstraints::set_authentication_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "merchant_connector_id",
                &app::model::payments::PlatformPaymentListConstraints::get_merchant_connector_id,
                &app::model::payments::PlatformPaymentListConstraints::set_merchant_connector_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_network",
                &app::model::payments::PlatformPaymentListConstraints::get_card_network,
                &app::model::payments::PlatformPaymentListConstraints::set_card_network,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_discovery",
                &app::model::payments::PlatformPaymentListConstraints::get_card_discovery,
                &app::model::payments::PlatformPaymentListConstraints::set_card_discovery,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "merchant_order_reference_id",
                &app::model::payments::PlatformPaymentListConstraints::
                    get_merchant_order_reference_id,
                &app::model::payments::PlatformPaymentListConstraints::
                    set_merchant_order_reference_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "order",
                &app::model::payments::PlatformPaymentListConstraints::get_order,
                &app::model::payments::PlatformPaymentListConstraints::set_order,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "platform_payment_list_constraints";
    }
};

template<>
struct serde::Serializable<app::model::payments::PlatformPaymentListItem>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "payment_id",
                &app::model::payments::PlatformPaymentListItem::get_payment_id,
                &app::model::payments::PlatformPaymentListItem::set_payment_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "merchant_id",
                &app::model::payments::PlatformPaymentListItem::get_merchant_id,
                &app::model::payments::PlatformPaymentListItem::set_merchant_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "processor_merchant_id",
                &app::model::payments::PlatformPaymentListItem::get_processor_merchant_id,
                &app::model::payments::PlatformPaymentListItem::set_processor_merchant_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "status",
                &app::model::payments::PlatformPaymentListItem::get_status,
                &app::model::payments::PlatformPaymentListItem::set_status,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "amount",
                &app::model::payments::PlatformPaymentListItem::get_amount,
                &app::model::payments::PlatformPaymentListItem::set_amount,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "net_amount",
                &app::model::payments::PlatformPaymentListItem::get_net_amount,
                &app::model::payments::PlatformPaymentListItem::set_net_amount,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "amount_capturable",
                &app::model::payments::PlatformPaymentListItem::get_amount_capturable,
                &app::model::payments::PlatformPaymentListItem::set_amount_capturable,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "state_metadata",
                &app::model::payments::PlatformPaymentListItem::get_state_metadata,
                &app::model::payments::PlatformPaymentListItem::set_state_metadata,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "client_secret",
                &app::model::payments::PlatformPaymentListItem::get_client_secret,
                &app::model::payments::PlatformPaymentListItem::set_client_secret,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "created",
                &app::model::payments::PlatformPaymentListItem::get_created,
                &app::model::payments::PlatformPaymentListItem::set_created,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "modified_at",
                &app::model::payments::PlatformPaymentListItem::get_modified_at,
                &app::model::payments::PlatformPaymentListItem::set_modified_at,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "currency",
                &app::model::payments::PlatformPaymentListItem::get_currency,
                &app::model::payments::PlatformPaymentListItem::set_currency,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "customer_id",
                &app::model::payments::PlatformPaymentListItem::get_customer_id,
                &app::model::payments::PlatformPaymentListItem::set_customer_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "description",
                &app::model::payments::PlatformPaymentListItem::get_description,
                &app::model::payments::PlatformPaymentListItem::set_description,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "order_details",
                &app::model::payments::PlatformPaymentListItem::get_order_details,
                &app::model::payments::PlatformPaymentListItem::set_order_details,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "connector",
                &app::model::payments::PlatformPaymentListItem::get_connector,
                &app::model::payments::PlatformPaymentListItem::set_connector,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_method",
                &app::model::payments::PlatformPaymentListItem::get_payment_method,
                &app::model::payments::PlatformPaymentListItem::set_payment_method,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_method_type",
                &app::model::payments::PlatformPaymentListItem::get_payment_method_type,
                &app::model::payments::PlatformPaymentListItem::set_payment_method_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "business_label",
                &app::model::payments::PlatformPaymentListItem::get_business_label,
                &app::model::payments::PlatformPaymentListItem::set_business_label,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "business_country",
                &app::model::payments::PlatformPaymentListItem::get_business_country,
                &app::model::payments::PlatformPaymentListItem::set_business_country,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "business_sub_label",
                &app::model::payments::PlatformPaymentListItem::get_business_sub_label,
                &app::model::payments::PlatformPaymentListItem::set_business_sub_label,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "setup_future_usage",
                &app::model::payments::PlatformPaymentListItem::get_setup_future_usage,
                &app::model::payments::PlatformPaymentListItem::set_setup_future_usage,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "capture_method",
                &app::model::payments::PlatformPaymentListItem::get_capture_method,
                &app::model::payments::PlatformPaymentListItem::set_capture_method,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "authentication_type",
                &app::model::payments::PlatformPaymentListItem::get_authentication_type,
                &app::model::payments::PlatformPaymentListItem::set_authentication_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "connector_transaction_id",
                &app::model::payments::PlatformPaymentListItem::get_connector_transaction_id,
                &app::model::payments::PlatformPaymentListItem::set_connector_transaction_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "attempt_count",
                &app::model::payments::PlatformPaymentListItem::get_attempt_count,
                &app::model::payments::PlatformPaymentListItem::set_attempt_count,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "profile_id",
                &app::model::payments::PlatformPaymentListItem::get_profile_id,
                &app::model::payments::PlatformPaymentListItem::set_profile_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "merchant_connector_id",
                &app::model::payments::PlatformPaymentListItem::get_merchant_connector_id,
                &app::model::payments::PlatformPaymentListItem::set_merchant_connector_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "merchant_order_reference_id",
                &app::model::payments::PlatformPaymentListItem::get_merchant_order_reference_id,
                &app::model::payments::PlatformPaymentListItem::set_merchant_order_reference_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "metadata",
                &app::model::payments::PlatformPaymentListItem::get_metadata,
                &app::model::payments::PlatformPaymentListItem::set_metadata,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "error_message",
                &app::model::payments::PlatformPaymentListItem::get_error_message,
                &app::model::payments::PlatformPaymentListItem::set_error_message,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "updated",
                &app::model::payments::PlatformPaymentListItem::get_updated,
                &app::model::payments::PlatformPaymentListItem::set_updated,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "extended_authorization_applied",
                &app::model::payments::PlatformPaymentListItem::get_extended_authorization_applied,
                &app::model::payments::PlatformPaymentListItem::set_extended_authorization_applied,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "extended_authorization_last_applied_at",
                &app::model::payments::PlatformPaymentListItem::
                    get_extended_authorization_last_applied_at,
                &app::model::payments::PlatformPaymentListItem::
                    set_extended_authorization_last_applied_at,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "capture_before",
                &app::model::payments::PlatformPaymentListItem::get_capture_before,
                &app::model::payments::PlatformPaymentListItem::set_capture_before,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_discovery",
                &app::model::payments::PlatformPaymentListItem::get_card_discovery,
                &app::model::payments::PlatformPaymentListItem::set_card_discovery,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "mit_category",
                &app::model::payments::PlatformPaymentListItem::get_mit_category,
                &app::model::payments::PlatformPaymentListItem::set_mit_category,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "tokenization",
                &app::model::payments::PlatformPaymentListItem::get_tokenization,
                &app::model::payments::PlatformPaymentListItem::set_tokenization,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "force_3ds_challenge",
                &app::model::payments::PlatformPaymentListItem::get_force_3ds_challenge,
                &app::model::payments::PlatformPaymentListItem::set_force_3ds_challenge,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "force_3ds_challenge_trigger",
                &app::model::payments::PlatformPaymentListItem::get_force_3ds_challenge_trigger,
                &app::model::payments::PlatformPaymentListItem::set_force_3ds_challenge_trigger,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "issuer_error_code",
                &app::model::payments::PlatformPaymentListItem::get_issuer_error_code,
                &app::model::payments::PlatformPaymentListItem::set_issuer_error_code,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "issuer_error_message",
                &app::model::payments::PlatformPaymentListItem::get_issuer_error_message,
                &app::model::payments::PlatformPaymentListItem::set_issuer_error_message,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "is_iframe_redirection_enabled",
                &app::model::payments::PlatformPaymentListItem::get_is_iframe_redirection_enabled,
                &app::model::payments::PlatformPaymentListItem::set_is_iframe_redirection_enabled,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_channel",
                &app::model::payments::PlatformPaymentListItem::get_payment_channel,
                &app::model::payments::PlatformPaymentListItem::set_payment_channel,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "enable_partial_authorization",
                &app::model::payments::PlatformPaymentListItem::get_enable_partial_authorization,
                &app::model::payments::PlatformPaymentListItem::set_enable_partial_authorization,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "enable_overcapture",
                &app::model::payments::PlatformPaymentListItem::get_enable_overcapture,
                &app::model::payments::PlatformPaymentListItem::set_enable_overcapture,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "is_overcapture_enabled",
                &app::model::payments::PlatformPaymentListItem::get_is_overcapture_enabled,
                &app::model::payments::PlatformPaymentListItem::set_is_overcapture_enabled,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "network_details",
                &app::model::payments::PlatformPaymentListItem::get_network_details,
                &app::model::payments::PlatformPaymentListItem::set_network_details,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "is_stored_credential",
                &app::model::payments::PlatformPaymentListItem::get_is_stored_credential,
                &app::model::payments::PlatformPaymentListItem::set_is_stored_credential,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "request_extended_authorization",
                &app::model::payments::PlatformPaymentListItem::get_request_extended_authorization,
                &app::model::payments::PlatformPaymentListItem::set_request_extended_authorization,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "billing_descriptor",
                &app::model::payments::PlatformPaymentListItem::get_billing_descriptor,
                &app::model::payments::PlatformPaymentListItem::set_billing_descriptor,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "partner_merchant_identifier_details",
                &app::model::payments::PlatformPaymentListItem::
                    get_partner_merchant_identifier_details,
                &app::model::payments::PlatformPaymentListItem::
                    set_partner_merchant_identifier_details,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "installment_data",
                &app::model::payments::PlatformPaymentListItem::get_installment_data,
                &app::model::payments::PlatformPaymentListItem::set_installment_data,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "sender_payment_instrument_id",
                &app::model::payments::PlatformPaymentListItem::get_sender_payment_instrument_id,
                &app::model::payments::PlatformPaymentListItem::set_sender_payment_instrument_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "surcharge_details",
                &app::model::payments::PlatformPaymentListItem::get_surcharge_details,
                &app::model::payments::PlatformPaymentListItem::set_surcharge_details,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "installment_options",
                &app::model::payments::PlatformPaymentListItem::get_installment_options,
                &app::model::payments::PlatformPaymentListItem::set_installment_options,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "platform_payment_list_item";
    }
};

template<>
struct serde::Serializable<app::model::payments::PlatformPaymentListResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "count",
                &app::model::payments::PlatformPaymentListResponse::get_count,
                &app::model::payments::PlatformPaymentListResponse::set_count,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "total_count",
                &app::model::payments::PlatformPaymentListResponse::get_total_count,
                &app::model::payments::PlatformPaymentListResponse::set_total_count,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "data",
                &app::model::payments::PlatformPaymentListResponse::get_data,
                &app::model::payments::PlatformPaymentListResponse::set_data,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "platform_payment_list_response";
    }
};

template<>
struct serde::Serializable<app::model::payments::RecoveryPaymentListResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "count",
                &app::model::payments::RecoveryPaymentListResponse::get_count,
                &app::model::payments::RecoveryPaymentListResponse::set_count,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "total_count",
                &app::model::payments::RecoveryPaymentListResponse::get_total_count,
                &app::model::payments::RecoveryPaymentListResponse::set_total_count,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "data",
                &app::model::payments::RecoveryPaymentListResponse::get_data,
                &app::model::payments::RecoveryPaymentListResponse::set_data,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "recovery_payment_list_response";
    }
};

template<>
struct serde::Serializable<app::model::payments::IncrementalAuthorizationResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "authorization_id",
                &app::model::payments::IncrementalAuthorizationResponse::get_authorization_id,
                &app::model::payments::IncrementalAuthorizationResponse::set_authorization_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "amount",
                &app::model::payments::IncrementalAuthorizationResponse::get_amount,
                &app::model::payments::IncrementalAuthorizationResponse::set_amount,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "status",
                &app::model::payments::IncrementalAuthorizationResponse::get_status,
                &app::model::payments::IncrementalAuthorizationResponse::set_status,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "error_code",
                &app::model::payments::IncrementalAuthorizationResponse::get_error_code,
                &app::model::payments::IncrementalAuthorizationResponse::set_error_code,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "error_message",
                &app::model::payments::IncrementalAuthorizationResponse::get_error_message,
                &app::model::payments::IncrementalAuthorizationResponse::set_error_message,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "previously_authorized_amount",
                &app::model::payments::IncrementalAuthorizationResponse::
                    get_previously_authorized_amount,
                &app::model::payments::IncrementalAuthorizationResponse::
                    set_previously_authorized_amount,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "incremental_authorization_response";
    }
};

template<>
struct serde::Serializable<app::model::payments::PaymentListResponseV2>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "count",
                &app::model::payments::PaymentListResponseV2::get_count,
                &app::model::payments::PaymentListResponseV2::set_count,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "total_count",
                &app::model::payments::PaymentListResponseV2::get_total_count,
                &app::model::payments::PaymentListResponseV2::set_total_count,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "data",
                &app::model::payments::PaymentListResponseV2::get_data,
                &app::model::payments::PaymentListResponseV2::set_data,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "payment_list_response_v2";
    }
};

template<>
struct serde::Serializable<app::model::payments::PaymentsListRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "merchant_id",
                &app::model::payments::PaymentsListRequest::get_merchant_id,
                &app::model::payments::PaymentsListRequest::set_merchant_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "profile_id",
                &app::model::payments::PaymentsListRequest::get_profile_id,
                &app::model::payments::PaymentsListRequest::set_profile_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "status",
                &app::model::payments::PaymentsListRequest::get_status,
                &app::model::payments::PaymentsListRequest::set_status,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "connector",
                &app::model::payments::PaymentsListRequest::get_connector,
                &app::model::payments::PaymentsListRequest::set_connector,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_method",
                &app::model::payments::PaymentsListRequest::get_payment_method,
                &app::model::payments::PaymentsListRequest::set_payment_method,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_method_type",
                &app::model::payments::PaymentsListRequest::get_payment_method_type,
                &app::model::payments::PaymentsListRequest::set_payment_method_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "offset",
                &app::model::payments::PaymentsListRequest::get_offset,
                &app::model::payments::PaymentsListRequest::set_offset,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "limit",
                &app::model::payments::PaymentsListRequest::get_limit,
                &app::model::payments::PaymentsListRequest::set_limit,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "force_sync",
                &app::model::payments::PaymentsListRequest::get_force_sync,
                &app::model::payments::PaymentsListRequest::set_force_sync,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "expand_attempts",
                &app::model::payments::PaymentsListRequest::get_expand_attempts,
                &app::model::payments::PaymentsListRequest::set_expand_attempts,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "payments_list_request";
    }
};

template<>
struct serde::Serializable<app::model::payments::PaymentsFilterRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "status",
                &app::model::payments::PaymentsFilterRequest::get_status,
                &app::model::payments::PaymentsFilterRequest::set_status,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "connector",
                &app::model::payments::PaymentsFilterRequest::get_connector,
                &app::model::payments::PaymentsFilterRequest::set_connector,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_method",
                &app::model::payments::PaymentsFilterRequest::get_payment_method,
                &app::model::payments::PaymentsFilterRequest::set_payment_method,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_method_type",
                &app::model::payments::PaymentsFilterRequest::get_payment_method_type,
                &app::model::payments::PaymentsFilterRequest::set_payment_method_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "merchant_reference_id",
                &app::model::payments::PaymentsFilterRequest::get_merchant_reference_id,
                &app::model::payments::PaymentsFilterRequest::set_merchant_reference_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "profile_id",
                &app::model::payments::PaymentsFilterRequest::get_profile_id,
                &app::model::payments::PaymentsFilterRequest::set_profile_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "offset",
                &app::model::payments::PaymentsFilterRequest::get_offset,
                &app::model::payments::PaymentsFilterRequest::set_offset,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "limit",
                &app::model::payments::PaymentsFilterRequest::get_limit,
                &app::model::payments::PaymentsFilterRequest::set_limit,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "payments_filter_request";
    }
};

template<>
struct serde::Serializable<app::model::payments::PaymentsAggregateRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "start",
                &app::model::payments::PaymentsAggregateRequest::get_start,
                &app::model::payments::PaymentsAggregateRequest::set_start,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "end",
                &app::model::payments::PaymentsAggregateRequest::get_end,
                &app::model::payments::PaymentsAggregateRequest::set_end,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "status",
                &app::model::payments::PaymentsAggregateRequest::get_status,
                &app::model::payments::PaymentsAggregateRequest::set_status,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "profile_id",
                &app::model::payments::PaymentsAggregateRequest::get_profile_id,
                &app::model::payments::PaymentsAggregateRequest::set_profile_id,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "payments_aggregate_request";
    }
};

template<>
struct serde::Serializable<app::model::payments::PaymentAttemptListFilterRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "payment_id",
                &app::model::payments::PaymentAttemptListFilterRequest::get_payment_id,
                &app::model::payments::PaymentAttemptListFilterRequest::set_payment_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "status",
                &app::model::payments::PaymentAttemptListFilterRequest::get_status,
                &app::model::payments::PaymentAttemptListFilterRequest::set_status,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "connector",
                &app::model::payments::PaymentAttemptListFilterRequest::get_connector,
                &app::model::payments::PaymentAttemptListFilterRequest::set_connector,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "offset",
                &app::model::payments::PaymentAttemptListFilterRequest::get_offset,
                &app::model::payments::PaymentAttemptListFilterRequest::set_offset,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "limit",
                &app::model::payments::PaymentAttemptListFilterRequest::get_limit,
                &app::model::payments::PaymentAttemptListFilterRequest::set_limit,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "payment_attempt_list_filter_request";
    }
};

template<>
struct serde::Serializable<app::model::payments::PaymentsAdvancedListRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "query",
                &app::model::payments::PaymentsAdvancedListRequest::get_query,
                &app::model::payments::PaymentsAdvancedListRequest::set_query,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_id",
                &app::model::payments::PaymentsAdvancedListRequest::get_payment_id,
                &app::model::payments::PaymentsAdvancedListRequest::set_payment_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "profile_id",
                &app::model::payments::PaymentsAdvancedListRequest::get_profile_id,
                &app::model::payments::PaymentsAdvancedListRequest::set_profile_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "customer_id",
                &app::model::payments::PaymentsAdvancedListRequest::get_customer_id,
                &app::model::payments::PaymentsAdvancedListRequest::set_customer_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "limit",
                &app::model::payments::PaymentsAdvancedListRequest::get_limit,
                &app::model::payments::PaymentsAdvancedListRequest::set_limit,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "offset",
                &app::model::payments::PaymentsAdvancedListRequest::get_offset,
                &app::model::payments::PaymentsAdvancedListRequest::set_offset,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "connector",
                &app::model::payments::PaymentsAdvancedListRequest::get_connector,
                &app::model::payments::PaymentsAdvancedListRequest::set_connector,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "currency",
                &app::model::payments::PaymentsAdvancedListRequest::get_currency,
                &app::model::payments::PaymentsAdvancedListRequest::set_currency,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "status",
                &app::model::payments::PaymentsAdvancedListRequest::get_status,
                &app::model::payments::PaymentsAdvancedListRequest::set_status,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_method",
                &app::model::payments::PaymentsAdvancedListRequest::get_payment_method,
                &app::model::payments::PaymentsAdvancedListRequest::set_payment_method,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_method_type",
                &app::model::payments::PaymentsAdvancedListRequest::get_payment_method_type,
                &app::model::payments::PaymentsAdvancedListRequest::set_payment_method_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "order",
                &app::model::payments::PaymentsAdvancedListRequest::get_order,
                &app::model::payments::PaymentsAdvancedListRequest::set_order,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "payments_advanced_list_request";
    }
};

template<>
struct serde::Serializable<app::model::payments::PaymentsAdvancedListResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "count",
                &app::model::payments::PaymentsAdvancedListResponse::get_count,
                &app::model::payments::PaymentsAdvancedListResponse::set_count,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "total_count",
                &app::model::payments::PaymentsAdvancedListResponse::get_total_count,
                &app::model::payments::PaymentsAdvancedListResponse::set_total_count,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "data",
                &app::model::payments::PaymentsAdvancedListResponse::get_data,
                &app::model::payments::PaymentsAdvancedListResponse::set_data,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "payments_advanced_list_response";
    }
};

template<>
struct serde::Serializable<app::model::payments::PaymentsClientMethod>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "payment_method",
                &app::model::payments::PaymentsClientMethod::get_payment_method,
                &app::model::payments::PaymentsClientMethod::set_payment_method,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_method_type",
                &app::model::payments::PaymentsClientMethod::get_payment_method_type,
                &app::model::payments::PaymentsClientMethod::set_payment_method_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "connector",
                &app::model::payments::PaymentsClientMethod::get_connector,
                &app::model::payments::PaymentsClientMethod::set_connector,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "enabled",
                &app::model::payments::PaymentsClientMethod::get_enabled,
                &app::model::payments::PaymentsClientMethod::set_enabled,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "payments_client_method";
    }
};

template<>
struct serde::Serializable<app::model::payments::PaymentsClientMethodsResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "count",
                &app::model::payments::PaymentsClientMethodsResponse::get_count,
                &app::model::payments::PaymentsClientMethodsResponse::set_count,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "data",
                &app::model::payments::PaymentsClientMethodsResponse::get_data,
                &app::model::payments::PaymentsClientMethodsResponse::set_data,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "payments_client_methods_response";
    }
};

template<>
struct serde::Serializable<app::model::payments::PaymentListFilterConstraints>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "query",
                &app::model::payments::PaymentListFilterConstraints::get_query,
                &app::model::payments::PaymentListFilterConstraints::set_query,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_id",
                &app::model::payments::PaymentListFilterConstraints::get_payment_id,
                &app::model::payments::PaymentListFilterConstraints::set_payment_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "profile_id",
                &app::model::payments::PaymentListFilterConstraints::get_profile_id,
                &app::model::payments::PaymentListFilterConstraints::set_profile_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "customer_id",
                &app::model::payments::PaymentListFilterConstraints::get_customer_id,
                &app::model::payments::PaymentListFilterConstraints::set_customer_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "limit",
                &app::model::payments::PaymentListFilterConstraints::get_limit,
                &app::model::payments::PaymentListFilterConstraints::set_limit,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "offset",
                &app::model::payments::PaymentListFilterConstraints::get_offset,
                &app::model::payments::PaymentListFilterConstraints::set_offset,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "amount_filter",
                &app::model::payments::PaymentListFilterConstraints::get_amount_filter,
                &app::model::payments::PaymentListFilterConstraints::set_amount_filter,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "time_range",
                &app::model::payments::PaymentListFilterConstraints::get_time_range,
                &app::model::payments::PaymentListFilterConstraints::set_time_range,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "connector",
                &app::model::payments::PaymentListFilterConstraints::get_connector,
                &app::model::payments::PaymentListFilterConstraints::set_connector,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "currency",
                &app::model::payments::PaymentListFilterConstraints::get_currency,
                &app::model::payments::PaymentListFilterConstraints::set_currency,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "status",
                &app::model::payments::PaymentListFilterConstraints::get_status,
                &app::model::payments::PaymentListFilterConstraints::set_status,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_method",
                &app::model::payments::PaymentListFilterConstraints::get_payment_method,
                &app::model::payments::PaymentListFilterConstraints::set_payment_method,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_method_type",
                &app::model::payments::PaymentListFilterConstraints::get_payment_method_type,
                &app::model::payments::PaymentListFilterConstraints::set_payment_method_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "authentication_type",
                &app::model::payments::PaymentListFilterConstraints::get_authentication_type,
                &app::model::payments::PaymentListFilterConstraints::set_authentication_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "merchant_connector_id",
                &app::model::payments::PaymentListFilterConstraints::get_merchant_connector_id,
                &app::model::payments::PaymentListFilterConstraints::set_merchant_connector_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "order",
                &app::model::payments::PaymentListFilterConstraints::get_order,
                &app::model::payments::PaymentListFilterConstraints::set_order,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_network",
                &app::model::payments::PaymentListFilterConstraints::get_card_network,
                &app::model::payments::PaymentListFilterConstraints::set_card_network,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_last_4",
                &app::model::payments::PaymentListFilterConstraints::get_card_last_4,
                &app::model::payments::PaymentListFilterConstraints::set_card_last_4,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "active_attempt_id",
                &app::model::payments::PaymentListFilterConstraints::get_active_attempt_id,
                &app::model::payments::PaymentListFilterConstraints::set_active_attempt_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_issuer",
                &app::model::payments::PaymentListFilterConstraints::get_card_issuer,
                &app::model::payments::PaymentListFilterConstraints::set_card_issuer,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "routing_approach",
                &app::model::payments::PaymentListFilterConstraints::get_routing_approach,
                &app::model::payments::PaymentListFilterConstraints::set_routing_approach,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "refunds_status",
                &app::model::payments::PaymentListFilterConstraints::get_refunds_status,
                &app::model::payments::PaymentListFilterConstraints::set_refunds_status,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "dispute_status",
                &app::model::payments::PaymentListFilterConstraints::get_dispute_status,
                &app::model::payments::PaymentListFilterConstraints::set_dispute_status,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "client_source",
                &app::model::payments::PaymentListFilterConstraints::get_client_source,
                &app::model::payments::PaymentListFilterConstraints::set_client_source,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "client_version",
                &app::model::payments::PaymentListFilterConstraints::get_client_version,
                &app::model::payments::PaymentListFilterConstraints::set_client_version,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "first_attempt",
                &app::model::payments::PaymentListFilterConstraints::get_first_attempt,
                &app::model::payments::PaymentListFilterConstraints::set_first_attempt,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "merchant_order_reference_id",
                &app::model::payments::PaymentListFilterConstraints::
                    get_merchant_order_reference_id,
                &app::model::payments::PaymentListFilterConstraints::
                    set_merchant_order_reference_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_discovery",
                &app::model::payments::PaymentListFilterConstraints::get_card_discovery,
                &app::model::payments::PaymentListFilterConstraints::set_card_discovery,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "customer_email",
                &app::model::payments::PaymentListFilterConstraints::get_customer_email,
                &app::model::payments::PaymentListFilterConstraints::set_customer_email,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "payment_list_filter_constraints";
    }
};

template<>
struct serde::Serializable<app::model::payments::PaymentListFilters>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "connector",
                &app::model::payments::PaymentListFilters::get_connector,
                &app::model::payments::PaymentListFilters::set_connector,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "currency",
                &app::model::payments::PaymentListFilters::get_currency,
                &app::model::payments::PaymentListFilters::set_currency,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "status",
                &app::model::payments::PaymentListFilters::get_status,
                &app::model::payments::PaymentListFilters::set_status,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_method",
                &app::model::payments::PaymentListFilters::get_payment_method,
                &app::model::payments::PaymentListFilters::set_payment_method,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_method_type",
                &app::model::payments::PaymentListFilters::get_payment_method_type,
                &app::model::payments::PaymentListFilters::set_payment_method_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "authentication_type",
                &app::model::payments::PaymentListFilters::get_authentication_type,
                &app::model::payments::PaymentListFilters::set_authentication_type,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "payment_list_filters";
    }
};

template<>
struct serde::Serializable<app::model::payments::PaymentListFiltersV2>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "connector",
                &app::model::payments::PaymentListFiltersV2::get_connector,
                &app::model::payments::PaymentListFiltersV2::set_connector,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "currency",
                &app::model::payments::PaymentListFiltersV2::get_currency,
                &app::model::payments::PaymentListFiltersV2::set_currency,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "status",
                &app::model::payments::PaymentListFiltersV2::get_status,
                &app::model::payments::PaymentListFiltersV2::set_status,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_method",
                &app::model::payments::PaymentListFiltersV2::get_payment_method,
                &app::model::payments::PaymentListFiltersV2::set_payment_method,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "authentication_type",
                &app::model::payments::PaymentListFiltersV2::get_authentication_type,
                &app::model::payments::PaymentListFiltersV2::set_authentication_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_network",
                &app::model::payments::PaymentListFiltersV2::get_card_network,
                &app::model::payments::PaymentListFiltersV2::set_card_network,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_discovery",
                &app::model::payments::PaymentListFiltersV2::get_card_discovery,
                &app::model::payments::PaymentListFiltersV2::set_card_discovery,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "payment_list_filters_v2";
    }
};

template<>
struct serde::Serializable<app::model::payments::PlatformPaymentListFilters>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "connector",
                &app::model::payments::PlatformPaymentListFilters::get_connector,
                &app::model::payments::PlatformPaymentListFilters::set_connector,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "currency",
                &app::model::payments::PlatformPaymentListFilters::get_currency,
                &app::model::payments::PlatformPaymentListFilters::set_currency,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "status",
                &app::model::payments::PlatformPaymentListFilters::get_status,
                &app::model::payments::PlatformPaymentListFilters::set_status,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_method",
                &app::model::payments::PlatformPaymentListFilters::get_payment_method,
                &app::model::payments::PlatformPaymentListFilters::set_payment_method,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "authentication_type",
                &app::model::payments::PlatformPaymentListFilters::get_authentication_type,
                &app::model::payments::PlatformPaymentListFilters::set_authentication_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_network",
                &app::model::payments::PlatformPaymentListFilters::get_card_network,
                &app::model::payments::PlatformPaymentListFilters::set_card_network,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_discovery",
                &app::model::payments::PlatformPaymentListFilters::get_card_discovery,
                &app::model::payments::PlatformPaymentListFilters::set_card_discovery,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "platform_payment_list_filters";
    }
};

template<>
struct serde::Serializable<app::model::payments::PaymentsAggregateResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<
            "status_with_count",
            &app::model::payments::PaymentsAggregateResponse::get_status_with_count,
            &app::model::payments::PaymentsAggregateResponse::set_status_with_count,
            serde::FieldOptions{}>{}};
    }

    static constexpr std::string_view table_name()
    {
        return "payments_aggregate_response";
    }
};

template<>
struct serde::Serializable<app::model::payments::AmountFilter>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "start_amount",
                &app::model::payments::AmountFilter::get_start_amount,
                &app::model::payments::AmountFilter::set_start_amount,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "end_amount",
                &app::model::payments::AmountFilter::get_end_amount,
                &app::model::payments::AmountFilter::set_end_amount,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "amount_filter";
    }
};

template<>
struct serde::Serializable<app::model::payments::Order>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "on",
                &app::model::payments::Order::get_on,
                &app::model::payments::Order::set_on,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "by",
                &app::model::payments::Order::get_by,
                &app::model::payments::Order::set_by,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "order";
    }
};

template<>
struct serde::Serializable<app::model::payments::VerifyResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "verify_id",
                &app::model::payments::VerifyResponse::get_verify_id,
                &app::model::payments::VerifyResponse::set_verify_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "merchant_id",
                &app::model::payments::VerifyResponse::get_merchant_id,
                &app::model::payments::VerifyResponse::set_merchant_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "client_secret",
                &app::model::payments::VerifyResponse::get_client_secret,
                &app::model::payments::VerifyResponse::set_client_secret,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "customer_id",
                &app::model::payments::VerifyResponse::get_customer_id,
                &app::model::payments::VerifyResponse::set_customer_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "email",
                &app::model::payments::VerifyResponse::get_email,
                &app::model::payments::VerifyResponse::set_email,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "name",
                &app::model::payments::VerifyResponse::get_name,
                &app::model::payments::VerifyResponse::set_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "phone",
                &app::model::payments::VerifyResponse::get_phone,
                &app::model::payments::VerifyResponse::set_phone,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "mandate_id",
                &app::model::payments::VerifyResponse::get_mandate_id,
                &app::model::payments::VerifyResponse::set_mandate_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_method",
                &app::model::payments::VerifyResponse::get_payment_method,
                &app::model::payments::VerifyResponse::set_payment_method,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_method_data",
                &app::model::payments::VerifyResponse::get_payment_method_data,
                &app::model::payments::VerifyResponse::set_payment_method_data,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_token",
                &app::model::payments::VerifyResponse::get_payment_token,
                &app::model::payments::VerifyResponse::set_payment_token,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "error_code",
                &app::model::payments::VerifyResponse::get_error_code,
                &app::model::payments::VerifyResponse::set_error_code,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "error_message",
                &app::model::payments::VerifyResponse::get_error_message,
                &app::model::payments::VerifyResponse::set_error_message,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "verify_response";
    }
};

template<>
struct serde::Serializable<app::model::payments::MandateValidationFields>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "recurring_details",
                &app::model::payments::MandateValidationFields::get_recurring_details,
                &app::model::payments::MandateValidationFields::set_recurring_details,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "confirm",
                &app::model::payments::MandateValidationFields::get_confirm,
                &app::model::payments::MandateValidationFields::set_confirm,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "customer_id",
                &app::model::payments::MandateValidationFields::get_customer_id,
                &app::model::payments::MandateValidationFields::set_customer_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "mandate_data",
                &app::model::payments::MandateValidationFields::get_mandate_data,
                &app::model::payments::MandateValidationFields::set_mandate_data,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "setup_future_usage",
                &app::model::payments::MandateValidationFields::get_setup_future_usage,
                &app::model::payments::MandateValidationFields::set_setup_future_usage,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "off_session",
                &app::model::payments::MandateValidationFields::get_off_session,
                &app::model::payments::MandateValidationFields::set_off_session,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "mandate_validation_fields";
    }
};

template<>
struct serde::Serializable<app::model::payments::PaymentsResponseForm>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "transaction_id",
                &app::model::payments::PaymentsResponseForm::get_transaction_id,
                &app::model::payments::PaymentsResponseForm::set_transaction_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "merchant_id",
                &app::model::payments::PaymentsResponseForm::get_merchant_id,
                &app::model::payments::PaymentsResponseForm::set_merchant_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "order_id",
                &app::model::payments::PaymentsResponseForm::get_order_id,
                &app::model::payments::PaymentsResponseForm::set_order_id,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "payments_response_form";
    }
};

template<>
struct serde::Serializable<app::model::payments::OrderDetailsWithAmount>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "product_name",
                &app::model::payments::OrderDetailsWithAmount::get_product_name,
                &app::model::payments::OrderDetailsWithAmount::set_product_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "quantity",
                &app::model::payments::OrderDetailsWithAmount::get_quantity,
                &app::model::payments::OrderDetailsWithAmount::set_quantity,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "amount",
                &app::model::payments::OrderDetailsWithAmount::get_amount,
                &app::model::payments::OrderDetailsWithAmount::set_amount,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "tax_rate",
                &app::model::payments::OrderDetailsWithAmount::get_tax_rate,
                &app::model::payments::OrderDetailsWithAmount::set_tax_rate,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "total_tax_amount",
                &app::model::payments::OrderDetailsWithAmount::get_total_tax_amount,
                &app::model::payments::OrderDetailsWithAmount::set_total_tax_amount,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "requires_shipping",
                &app::model::payments::OrderDetailsWithAmount::get_requires_shipping,
                &app::model::payments::OrderDetailsWithAmount::set_requires_shipping,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "product_img_link",
                &app::model::payments::OrderDetailsWithAmount::get_product_img_link,
                &app::model::payments::OrderDetailsWithAmount::set_product_img_link,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "product_id",
                &app::model::payments::OrderDetailsWithAmount::get_product_id,
                &app::model::payments::OrderDetailsWithAmount::set_product_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "category",
                &app::model::payments::OrderDetailsWithAmount::get_category,
                &app::model::payments::OrderDetailsWithAmount::set_category,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "sub_category",
                &app::model::payments::OrderDetailsWithAmount::get_sub_category,
                &app::model::payments::OrderDetailsWithAmount::set_sub_category,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "brand",
                &app::model::payments::OrderDetailsWithAmount::get_brand,
                &app::model::payments::OrderDetailsWithAmount::set_brand,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "product_type",
                &app::model::payments::OrderDetailsWithAmount::get_product_type,
                &app::model::payments::OrderDetailsWithAmount::set_product_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "product_tax_code",
                &app::model::payments::OrderDetailsWithAmount::get_product_tax_code,
                &app::model::payments::OrderDetailsWithAmount::set_product_tax_code,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "description",
                &app::model::payments::OrderDetailsWithAmount::get_description,
                &app::model::payments::OrderDetailsWithAmount::set_description,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "sku",
                &app::model::payments::OrderDetailsWithAmount::get_sku,
                &app::model::payments::OrderDetailsWithAmount::set_sku,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "upc",
                &app::model::payments::OrderDetailsWithAmount::get_upc,
                &app::model::payments::OrderDetailsWithAmount::set_upc,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "commodity_code",
                &app::model::payments::OrderDetailsWithAmount::get_commodity_code,
                &app::model::payments::OrderDetailsWithAmount::set_commodity_code,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "unit_of_measure",
                &app::model::payments::OrderDetailsWithAmount::get_unit_of_measure,
                &app::model::payments::OrderDetailsWithAmount::set_unit_of_measure,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "total_amount",
                &app::model::payments::OrderDetailsWithAmount::get_total_amount,
                &app::model::payments::OrderDetailsWithAmount::set_total_amount,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "discount_name",
                &app::model::payments::OrderDetailsWithAmount::get_discount_name,
                &app::model::payments::OrderDetailsWithAmount::set_discount_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "unit_discount_amount",
                &app::model::payments::OrderDetailsWithAmount::get_unit_discount_amount,
                &app::model::payments::OrderDetailsWithAmount::set_unit_discount_amount,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "discount_percentage",
                &app::model::payments::OrderDetailsWithAmount::get_discount_percentage,
                &app::model::payments::OrderDetailsWithAmount::set_discount_percentage,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "discount_type",
                &app::model::payments::OrderDetailsWithAmount::get_discount_type,
                &app::model::payments::OrderDetailsWithAmount::set_discount_type,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "order_details_with_amount";
    }
};

template<>
struct serde::Serializable<app::model::payments::PaymentsSessionRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "payment_id",
                &app::model::payments::PaymentsSessionRequest::get_payment_id,
                &app::model::payments::PaymentsSessionRequest::set_payment_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "client_secret",
                &app::model::payments::PaymentsSessionRequest::get_client_secret,
                &app::model::payments::PaymentsSessionRequest::set_client_secret,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "wallets",
                &app::model::payments::PaymentsSessionRequest::get_wallets,
                &app::model::payments::PaymentsSessionRequest::set_wallets,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "merchant_connector_details",
                &app::model::payments::PaymentsSessionRequest::get_merchant_connector_details,
                &app::model::payments::PaymentsSessionRequest::set_merchant_connector_details,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "payments_session_request";
    }
};

template<>
struct serde::Serializable<app::model::payments::PaymentsPostSessionTokensRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "payment_id",
                &app::model::payments::PaymentsPostSessionTokensRequest::get_payment_id,
                &app::model::payments::PaymentsPostSessionTokensRequest::set_payment_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "client_secret",
                &app::model::payments::PaymentsPostSessionTokensRequest::get_client_secret,
                &app::model::payments::PaymentsPostSessionTokensRequest::set_client_secret,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_method_type",
                &app::model::payments::PaymentsPostSessionTokensRequest::get_payment_method_type,
                &app::model::payments::PaymentsPostSessionTokensRequest::set_payment_method_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_method",
                &app::model::payments::PaymentsPostSessionTokensRequest::get_payment_method,
                &app::model::payments::PaymentsPostSessionTokensRequest::set_payment_method,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "payments_post_session_tokens_request";
    }
};

template<>
struct serde::Serializable<app::model::payments::PaymentsPostSessionTokensResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "payment_id",
                &app::model::payments::PaymentsPostSessionTokensResponse::get_payment_id,
                &app::model::payments::PaymentsPostSessionTokensResponse::set_payment_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "next_action",
                &app::model::payments::PaymentsPostSessionTokensResponse::get_next_action,
                &app::model::payments::PaymentsPostSessionTokensResponse::set_next_action,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "status",
                &app::model::payments::PaymentsPostSessionTokensResponse::get_status,
                &app::model::payments::PaymentsPostSessionTokensResponse::set_status,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "payments_post_session_tokens_response";
    }
};

template<>
struct serde::Serializable<app::model::payments::PaymentsDynamicTaxCalculationRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "payment_id",
                &app::model::payments::PaymentsDynamicTaxCalculationRequest::get_payment_id,
                &app::model::payments::PaymentsDynamicTaxCalculationRequest::set_payment_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "shipping",
                &app::model::payments::PaymentsDynamicTaxCalculationRequest::get_shipping,
                &app::model::payments::PaymentsDynamicTaxCalculationRequest::set_shipping,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "client_secret",
                &app::model::payments::PaymentsDynamicTaxCalculationRequest::get_client_secret,
                &app::model::payments::PaymentsDynamicTaxCalculationRequest::set_client_secret,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_method_type",
                &app::model::payments::PaymentsDynamicTaxCalculationRequest::
                    get_payment_method_type,
                &app::model::payments::PaymentsDynamicTaxCalculationRequest::
                    set_payment_method_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "session_id",
                &app::model::payments::PaymentsDynamicTaxCalculationRequest::get_session_id,
                &app::model::payments::PaymentsDynamicTaxCalculationRequest::set_session_id,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "payments_dynamic_tax_calculation_request";
    }
};

template<>
struct serde::Serializable<app::model::payments::PaymentsDynamicTaxCalculationResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "payment_id",
                &app::model::payments::PaymentsDynamicTaxCalculationResponse::get_payment_id,
                &app::model::payments::PaymentsDynamicTaxCalculationResponse::set_payment_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "net_amount",
                &app::model::payments::PaymentsDynamicTaxCalculationResponse::get_net_amount,
                &app::model::payments::PaymentsDynamicTaxCalculationResponse::set_net_amount,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "order_tax_amount",
                &app::model::payments::PaymentsDynamicTaxCalculationResponse::get_order_tax_amount,
                &app::model::payments::PaymentsDynamicTaxCalculationResponse::set_order_tax_amount,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "shipping_cost",
                &app::model::payments::PaymentsDynamicTaxCalculationResponse::get_shipping_cost,
                &app::model::payments::PaymentsDynamicTaxCalculationResponse::set_shipping_cost,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "display_amount",
                &app::model::payments::PaymentsDynamicTaxCalculationResponse::get_display_amount,
                &app::model::payments::PaymentsDynamicTaxCalculationResponse::set_display_amount,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "payments_dynamic_tax_calculation_response";
    }
};

template<>
struct serde::Serializable<app::model::payments::DisplayAmountOnSdk>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "net_amount",
                &app::model::payments::DisplayAmountOnSdk::get_net_amount,
                &app::model::payments::DisplayAmountOnSdk::set_net_amount,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "order_tax_amount",
                &app::model::payments::DisplayAmountOnSdk::get_order_tax_amount,
                &app::model::payments::DisplayAmountOnSdk::set_order_tax_amount,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "shipping_cost",
                &app::model::payments::DisplayAmountOnSdk::get_shipping_cost,
                &app::model::payments::DisplayAmountOnSdk::set_shipping_cost,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "display_amount_on_sdk";
    }
};

template<>
struct serde::Serializable<app::model::payments::GpaySessionTokenData>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<
            "google_pay",
            &app::model::payments::GpaySessionTokenData::get_google_pay,
            &app::model::payments::GpaySessionTokenData::set_google_pay,
            serde::FieldOptions{}>{}};
    }

    static constexpr std::string_view table_name()
    {
        return "gpay_session_token_data";
    }
};

template<>
struct serde::Serializable<app::model::payments::Revolv3Data>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<
            "processor",
            &app::model::payments::Revolv3Data::get_processor,
            &app::model::payments::Revolv3Data::set_processor,
            serde::FieldOptions{}>{}};
    }

    static constexpr std::string_view table_name()
    {
        return "revolv3_data";
    }
};

template<>
struct serde::Serializable<app::model::payments::PaymentProcessingDetails>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "payment_processing_certificate",
                &app::model::payments::PaymentProcessingDetails::get_payment_processing_certificate,
                &app::model::payments::PaymentProcessingDetails::set_payment_processing_certificate,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_processing_certificate_key",
                &app::model::payments::PaymentProcessingDetails::
                    get_payment_processing_certificate_key,
                &app::model::payments::PaymentProcessingDetails::
                    set_payment_processing_certificate_key,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "payment_processing_details";
    }
};

template<>
struct serde::Serializable<app::model::payments::VaultDetails>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "internal_vault",
                &app::model::payments::VaultDetails::get_internal_vault,
                &app::model::payments::VaultDetails::set_internal_vault,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "external_vault_details",
                &app::model::payments::VaultDetails::get_external_vault_details,
                &app::model::payments::VaultDetails::set_external_vault_details,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "vault_details";
    }
};

template<>
struct serde::Serializable<app::model::payments::InternalVaultSessionDetails>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<
            "sdk_authorization",
            &app::model::payments::InternalVaultSessionDetails::get_sdk_authorization,
            &app::model::payments::InternalVaultSessionDetails::set_sdk_authorization,
            serde::FieldOptions{}>{}};
    }

    static constexpr std::string_view table_name()
    {
        return "internal_vault_session_details";
    }
};

template<>
struct serde::Serializable<app::model::payments::VgsSessionDetails>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "external_vault_id",
                &app::model::payments::VgsSessionDetails::get_external_vault_id,
                &app::model::payments::VgsSessionDetails::set_external_vault_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "sdk_env",
                &app::model::payments::VgsSessionDetails::get_sdk_env,
                &app::model::payments::VgsSessionDetails::set_sdk_env,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "vgs_session_details";
    }
};

template<>
struct serde::Serializable<app::model::payments::HyperswitchVaultSessionDetails>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<
            "sdk_authorization",
            &app::model::payments::HyperswitchVaultSessionDetails::get_sdk_authorization,
            &app::model::payments::HyperswitchVaultSessionDetails::set_sdk_authorization,
            serde::FieldOptions{}>{}};
    }

    static constexpr std::string_view table_name()
    {
        return "hyperswitch_vault_session_details";
    }
};

template<>
struct serde::Serializable<app::model::payments::HyperswitchVaultData>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<
            "sdk_authorization",
            &app::model::payments::HyperswitchVaultData::get_sdk_authorization,
            &app::model::payments::HyperswitchVaultData::set_sdk_authorization,
            serde::FieldOptions{}>{}};
    }

    static constexpr std::string_view table_name()
    {
        return "hyperswitch_vault_data";
    }
};

template<>
struct serde::Serializable<app::model::payments::VgsVaultData>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "vault_id",
                &app::model::payments::VgsVaultData::get_vault_id,
                &app::model::payments::VgsVaultData::set_vault_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "environment",
                &app::model::payments::VgsVaultData::get_environment,
                &app::model::payments::VgsVaultData::set_environment,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "vgs_vault_data";
    }
};

template<>
struct serde::Serializable<app::model::payments::KlarnaSessionTokenResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "session_token",
                &app::model::payments::KlarnaSessionTokenResponse::get_session_token,
                &app::model::payments::KlarnaSessionTokenResponse::set_session_token,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "session_id",
                &app::model::payments::KlarnaSessionTokenResponse::get_session_id,
                &app::model::payments::KlarnaSessionTokenResponse::set_session_id,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "klarna_session_token_response";
    }
};

template<>
struct serde::Serializable<app::model::payments::OpenBankingSessionToken>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<
            "open_banking_session_token",
            &app::model::payments::OpenBankingSessionToken::get_open_banking_session_token,
            &app::model::payments::OpenBankingSessionToken::set_open_banking_session_token,
            serde::FieldOptions{}>{}};
    }

    static constexpr std::string_view table_name()
    {
        return "open_banking_session_token";
    }
};

template<>
struct serde::Serializable<app::model::payments::ThirdPartySdkSessionResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<
            "secrets",
            &app::model::payments::ThirdPartySdkSessionResponse::get_secrets,
            &app::model::payments::ThirdPartySdkSessionResponse::set_secrets,
            serde::FieldOptions{}>{}};
    }

    static constexpr std::string_view table_name()
    {
        return "third_party_sdk_session_response";
    }
};

template<>
struct serde::Serializable<app::model::payments::PaymentsSessionResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "payment_id",
                &app::model::payments::PaymentsSessionResponse::get_payment_id,
                &app::model::payments::PaymentsSessionResponse::set_payment_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "client_secret",
                &app::model::payments::PaymentsSessionResponse::get_client_secret,
                &app::model::payments::PaymentsSessionResponse::set_client_secret,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "session_token",
                &app::model::payments::PaymentsSessionResponse::get_session_token,
                &app::model::payments::PaymentsSessionResponse::set_session_token,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "vault_details",
                &app::model::payments::PaymentsSessionResponse::get_vault_details,
                &app::model::payments::PaymentsSessionResponse::set_vault_details,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "payments_session_response";
    }
};

template<>
struct serde::Serializable<app::model::payments::PaymentRetrieveBody>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "merchant_id",
                &app::model::payments::PaymentRetrieveBody::get_merchant_id,
                &app::model::payments::PaymentRetrieveBody::set_merchant_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "force_sync",
                &app::model::payments::PaymentRetrieveBody::get_force_sync,
                &app::model::payments::PaymentRetrieveBody::set_force_sync,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "client_secret",
                &app::model::payments::PaymentRetrieveBody::get_client_secret,
                &app::model::payments::PaymentRetrieveBody::set_client_secret,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "expand_captures",
                &app::model::payments::PaymentRetrieveBody::get_expand_captures,
                &app::model::payments::PaymentRetrieveBody::set_expand_captures,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "expand_attempts",
                &app::model::payments::PaymentRetrieveBody::get_expand_attempts,
                &app::model::payments::PaymentRetrieveBody::set_expand_attempts,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "all_keys_required",
                &app::model::payments::PaymentRetrieveBody::get_all_keys_required,
                &app::model::payments::PaymentRetrieveBody::set_all_keys_required,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "payment_retrieve_body";
    }
};

template<>
struct serde::Serializable<app::model::payments::PaymentRetrieveBodyWithCredentials>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "payment_id",
                &app::model::payments::PaymentRetrieveBodyWithCredentials::get_payment_id,
                &app::model::payments::PaymentRetrieveBodyWithCredentials::set_payment_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "merchant_id",
                &app::model::payments::PaymentRetrieveBodyWithCredentials::get_merchant_id,
                &app::model::payments::PaymentRetrieveBodyWithCredentials::set_merchant_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "force_sync",
                &app::model::payments::PaymentRetrieveBodyWithCredentials::get_force_sync,
                &app::model::payments::PaymentRetrieveBodyWithCredentials::set_force_sync,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "merchant_connector_details",
                &app::model::payments::PaymentRetrieveBodyWithCredentials::
                    get_merchant_connector_details,
                &app::model::payments::PaymentRetrieveBodyWithCredentials::
                    set_merchant_connector_details,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "payment_retrieve_body_with_credentials";
    }
};

template<>
struct serde::Serializable<app::model::payments::PaymentsCompleteAuthorizeRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "payment_id",
                &app::model::payments::PaymentsCompleteAuthorizeRequest::get_payment_id,
                &app::model::payments::PaymentsCompleteAuthorizeRequest::set_payment_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "shipping",
                &app::model::payments::PaymentsCompleteAuthorizeRequest::get_shipping,
                &app::model::payments::PaymentsCompleteAuthorizeRequest::set_shipping,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "client_secret",
                &app::model::payments::PaymentsCompleteAuthorizeRequest::get_client_secret,
                &app::model::payments::PaymentsCompleteAuthorizeRequest::set_client_secret,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "threeds_method_comp_ind",
                &app::model::payments::PaymentsCompleteAuthorizeRequest::
                    get_threeds_method_comp_ind,
                &app::model::payments::PaymentsCompleteAuthorizeRequest::
                    set_threeds_method_comp_ind,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "payments_complete_authorize_request";
    }
};

template<>
struct serde::Serializable<app::model::payments::PaymentsCancelPostCaptureRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "payment_id",
                &app::model::payments::PaymentsCancelPostCaptureRequest::get_payment_id,
                &app::model::payments::PaymentsCancelPostCaptureRequest::set_payment_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "cancellation_reason",
                &app::model::payments::PaymentsCancelPostCaptureRequest::get_cancellation_reason,
                &app::model::payments::PaymentsCancelPostCaptureRequest::set_cancellation_reason,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_method_data",
                &app::model::payments::PaymentsCancelPostCaptureRequest::get_payment_method_data,
                &app::model::payments::PaymentsCancelPostCaptureRequest::set_payment_method_data,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "payments_cancel_post_capture_request";
    }
};

template<>
struct serde::Serializable<app::model::payments::PaymentsPostCaptureVoidSyncTrackingData>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "payment_id",
                &app::model::payments::PaymentsPostCaptureVoidSyncTrackingData::get_payment_id,
                &app::model::payments::PaymentsPostCaptureVoidSyncTrackingData::set_payment_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "merchant_id",
                &app::model::payments::PaymentsPostCaptureVoidSyncTrackingData::get_merchant_id,
                &app::model::payments::PaymentsPostCaptureVoidSyncTrackingData::set_merchant_id,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "payments_post_capture_void_sync_tracking_data";
    }
};

template<>
struct serde::Serializable<app::model::payments::PaymentsExtendAuthorizationRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<
            "payment_id",
            &app::model::payments::PaymentsExtendAuthorizationRequest::get_payment_id,
            &app::model::payments::PaymentsExtendAuthorizationRequest::set_payment_id,
            serde::FieldOptions{}>{}};
    }

    static constexpr std::string_view table_name()
    {
        return "payments_extend_authorization_request";
    }
};

template<>
struct serde::Serializable<app::model::payments::PaymentsIncrementalAuthorizationRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "payment_id",
                &app::model::payments::PaymentsIncrementalAuthorizationRequest::get_payment_id,
                &app::model::payments::PaymentsIncrementalAuthorizationRequest::set_payment_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "amount",
                &app::model::payments::PaymentsIncrementalAuthorizationRequest::get_amount,
                &app::model::payments::PaymentsIncrementalAuthorizationRequest::set_amount,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "reason",
                &app::model::payments::PaymentsIncrementalAuthorizationRequest::get_reason,
                &app::model::payments::PaymentsIncrementalAuthorizationRequest::set_reason,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_method_data",
                &app::model::payments::PaymentsIncrementalAuthorizationRequest::
                    get_payment_method_data,
                &app::model::payments::PaymentsIncrementalAuthorizationRequest::
                    set_payment_method_data,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "payments_incremental_authorization_request";
    }
};

template<>
struct serde::Serializable<app::model::payments::PaymentsExternalAuthenticationRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "payment_id",
                &app::model::payments::PaymentsExternalAuthenticationRequest::get_payment_id,
                &app::model::payments::PaymentsExternalAuthenticationRequest::set_payment_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "client_secret",
                &app::model::payments::PaymentsExternalAuthenticationRequest::get_client_secret,
                &app::model::payments::PaymentsExternalAuthenticationRequest::set_client_secret,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "sdk_information",
                &app::model::payments::PaymentsExternalAuthenticationRequest::get_sdk_information,
                &app::model::payments::PaymentsExternalAuthenticationRequest::set_sdk_information,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "device_channel",
                &app::model::payments::PaymentsExternalAuthenticationRequest::get_device_channel,
                &app::model::payments::PaymentsExternalAuthenticationRequest::set_device_channel,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "threeds_method_comp_ind",
                &app::model::payments::PaymentsExternalAuthenticationRequest::
                    get_threeds_method_comp_ind,
                &app::model::payments::PaymentsExternalAuthenticationRequest::
                    set_threeds_method_comp_ind,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "payments_external_authentication_request";
    }
};

template<>
struct serde::Serializable<app::model::payments::PaymentsManualUpdateRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "payment_id",
                &app::model::payments::PaymentsManualUpdateRequest::get_payment_id,
                &app::model::payments::PaymentsManualUpdateRequest::set_payment_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "attempt_id",
                &app::model::payments::PaymentsManualUpdateRequest::get_attempt_id,
                &app::model::payments::PaymentsManualUpdateRequest::set_attempt_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "merchant_id",
                &app::model::payments::PaymentsManualUpdateRequest::get_merchant_id,
                &app::model::payments::PaymentsManualUpdateRequest::set_merchant_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "attempt_status",
                &app::model::payments::PaymentsManualUpdateRequest::get_attempt_status,
                &app::model::payments::PaymentsManualUpdateRequest::set_attempt_status,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "error_code",
                &app::model::payments::PaymentsManualUpdateRequest::get_error_code,
                &app::model::payments::PaymentsManualUpdateRequest::set_error_code,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "error_message",
                &app::model::payments::PaymentsManualUpdateRequest::get_error_message,
                &app::model::payments::PaymentsManualUpdateRequest::set_error_message,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "error_reason",
                &app::model::payments::PaymentsManualUpdateRequest::get_error_reason,
                &app::model::payments::PaymentsManualUpdateRequest::set_error_reason,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "connector_transaction_id",
                &app::model::payments::PaymentsManualUpdateRequest::get_connector_transaction_id,
                &app::model::payments::PaymentsManualUpdateRequest::set_connector_transaction_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "amount_capturable",
                &app::model::payments::PaymentsManualUpdateRequest::get_amount_capturable,
                &app::model::payments::PaymentsManualUpdateRequest::set_amount_capturable,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "update_amount_captured",
                &app::model::payments::PaymentsManualUpdateRequest::get_update_amount_captured,
                &app::model::payments::PaymentsManualUpdateRequest::set_update_amount_captured,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "amount_captured",
                &app::model::payments::PaymentsManualUpdateRequest::get_amount_captured,
                &app::model::payments::PaymentsManualUpdateRequest::set_amount_captured,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "payments_manual_update_request";
    }
};

template<>
struct serde::Serializable<app::model::payments::PaymentsManualUpdateResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "payment_id",
                &app::model::payments::PaymentsManualUpdateResponse::get_payment_id,
                &app::model::payments::PaymentsManualUpdateResponse::set_payment_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "attempt_id",
                &app::model::payments::PaymentsManualUpdateResponse::get_attempt_id,
                &app::model::payments::PaymentsManualUpdateResponse::set_attempt_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "merchant_id",
                &app::model::payments::PaymentsManualUpdateResponse::get_merchant_id,
                &app::model::payments::PaymentsManualUpdateResponse::set_merchant_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "attempt_status",
                &app::model::payments::PaymentsManualUpdateResponse::get_attempt_status,
                &app::model::payments::PaymentsManualUpdateResponse::set_attempt_status,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "error_code",
                &app::model::payments::PaymentsManualUpdateResponse::get_error_code,
                &app::model::payments::PaymentsManualUpdateResponse::set_error_code,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "error_message",
                &app::model::payments::PaymentsManualUpdateResponse::get_error_message,
                &app::model::payments::PaymentsManualUpdateResponse::set_error_message,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "error_reason",
                &app::model::payments::PaymentsManualUpdateResponse::get_error_reason,
                &app::model::payments::PaymentsManualUpdateResponse::set_error_reason,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "connector_transaction_id",
                &app::model::payments::PaymentsManualUpdateResponse::get_connector_transaction_id,
                &app::model::payments::PaymentsManualUpdateResponse::set_connector_transaction_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "amount_capturable",
                &app::model::payments::PaymentsManualUpdateResponse::get_amount_capturable,
                &app::model::payments::PaymentsManualUpdateResponse::set_amount_capturable,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "amount_captured",
                &app::model::payments::PaymentsManualUpdateResponse::get_amount_captured,
                &app::model::payments::PaymentsManualUpdateResponse::set_amount_captured,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "payments_manual_update_response";
    }
};

template<>
struct serde::Serializable<app::model::payments::PaymentsManualStatusUpdateRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<
            "intent_status",
            &app::model::payments::PaymentsManualStatusUpdateRequest::get_intent_status,
            &app::model::payments::PaymentsManualStatusUpdateRequest::set_intent_status,
            serde::FieldOptions{}>{}};
    }

    static constexpr std::string_view table_name()
    {
        return "payments_manual_status_update_request";
    }
};

template<>
struct serde::Serializable<app::model::payments::PaymentsManualStatusUpdateResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "payment_id",
                &app::model::payments::PaymentsManualStatusUpdateResponse::get_payment_id,
                &app::model::payments::PaymentsManualStatusUpdateResponse::set_payment_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "attempt_id",
                &app::model::payments::PaymentsManualStatusUpdateResponse::get_attempt_id,
                &app::model::payments::PaymentsManualStatusUpdateResponse::set_attempt_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "intent_status",
                &app::model::payments::PaymentsManualStatusUpdateResponse::get_intent_status,
                &app::model::payments::PaymentsManualStatusUpdateResponse::set_intent_status,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "attempt_status",
                &app::model::payments::PaymentsManualStatusUpdateResponse::get_attempt_status,
                &app::model::payments::PaymentsManualStatusUpdateResponse::set_attempt_status,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "payments_manual_status_update_response";
    }
};

template<>
struct serde::Serializable<app::model::payments::PaymentsManualStatusUpdateEligibleStatusesResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "payment_id",
                &app::model::payments::PaymentsManualStatusUpdateEligibleStatusesResponse::
                    get_payment_id,
                &app::model::payments::PaymentsManualStatusUpdateEligibleStatusesResponse::
                    set_payment_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "eligible_statuses",
                &app::model::payments::PaymentsManualStatusUpdateEligibleStatusesResponse::
                    get_eligible_statuses,
                &app::model::payments::PaymentsManualStatusUpdateEligibleStatusesResponse::
                    set_eligible_statuses,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "payments_manual_status_update_eligible_statuses_response";
    }
};

template<>
struct serde::Serializable<app::model::payments::SdkInformation>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "sdk_app_id",
                &app::model::payments::SdkInformation::get_sdk_app_id,
                &app::model::payments::SdkInformation::set_sdk_app_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "sdk_enc_data",
                &app::model::payments::SdkInformation::get_sdk_enc_data,
                &app::model::payments::SdkInformation::set_sdk_enc_data,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "sdk_ephem_pub_key",
                &app::model::payments::SdkInformation::get_sdk_ephem_pub_key,
                &app::model::payments::SdkInformation::set_sdk_ephem_pub_key,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "sdk_trans_id",
                &app::model::payments::SdkInformation::get_sdk_trans_id,
                &app::model::payments::SdkInformation::set_sdk_trans_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "sdk_reference_number",
                &app::model::payments::SdkInformation::get_sdk_reference_number,
                &app::model::payments::SdkInformation::set_sdk_reference_number,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "sdk_max_timeout",
                &app::model::payments::SdkInformation::get_sdk_max_timeout,
                &app::model::payments::SdkInformation::set_sdk_max_timeout,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "sdk_type",
                &app::model::payments::SdkInformation::get_sdk_type,
                &app::model::payments::SdkInformation::set_sdk_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "device_details",
                &app::model::payments::SdkInformation::get_device_details,
                &app::model::payments::SdkInformation::set_device_details,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "sdk_information";
    }
};

template<>
struct serde::Serializable<app::model::payments::DeviceDetails>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "device_type",
                &app::model::payments::DeviceDetails::get_device_type,
                &app::model::payments::DeviceDetails::set_device_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "device_brand",
                &app::model::payments::DeviceDetails::get_device_brand,
                &app::model::payments::DeviceDetails::set_device_brand,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "device_os",
                &app::model::payments::DeviceDetails::get_device_os,
                &app::model::payments::DeviceDetails::set_device_os,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "device_display",
                &app::model::payments::DeviceDetails::get_device_display,
                &app::model::payments::DeviceDetails::set_device_display,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "device_details";
    }
};

template<>
struct serde::Serializable<app::model::payments::ListMethodsForPaymentsRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "client_secret",
                &app::model::payments::ListMethodsForPaymentsRequest::get_client_secret,
                &app::model::payments::ListMethodsForPaymentsRequest::set_client_secret,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "accepted_countries",
                &app::model::payments::ListMethodsForPaymentsRequest::get_accepted_countries,
                &app::model::payments::ListMethodsForPaymentsRequest::set_accepted_countries,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "amount",
                &app::model::payments::ListMethodsForPaymentsRequest::get_amount,
                &app::model::payments::ListMethodsForPaymentsRequest::set_amount,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "accepted_currencies",
                &app::model::payments::ListMethodsForPaymentsRequest::get_accepted_currencies,
                &app::model::payments::ListMethodsForPaymentsRequest::set_accepted_currencies,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "recurring_enabled",
                &app::model::payments::ListMethodsForPaymentsRequest::get_recurring_enabled,
                &app::model::payments::ListMethodsForPaymentsRequest::set_recurring_enabled,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_networks",
                &app::model::payments::ListMethodsForPaymentsRequest::get_card_networks,
                &app::model::payments::ListMethodsForPaymentsRequest::set_card_networks,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "limit",
                &app::model::payments::ListMethodsForPaymentsRequest::get_limit,
                &app::model::payments::ListMethodsForPaymentsRequest::set_limit,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "list_methods_for_payments_request";
    }
};

template<>
struct serde::Serializable<app::model::payments::PaymentMethodListResponseForPayments>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "payment_methods_enabled",
                &app::model::payments::PaymentMethodListResponseForPayments::
                    get_payment_methods_enabled,
                &app::model::payments::PaymentMethodListResponseForPayments::
                    set_payment_methods_enabled,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "customer_payment_methods",
                &app::model::payments::PaymentMethodListResponseForPayments::
                    get_customer_payment_methods,
                &app::model::payments::PaymentMethodListResponseForPayments::
                    set_customer_payment_methods,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "payment_method_list_response_for_payments";
    }
};

template<>
struct serde::Serializable<app::model::payments::ResponsePaymentMethodTypesForPayments>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "payment_method_type",
                &app::model::payments::ResponsePaymentMethodTypesForPayments::
                    get_payment_method_type,
                &app::model::payments::ResponsePaymentMethodTypesForPayments::
                    set_payment_method_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_method_subtype",
                &app::model::payments::ResponsePaymentMethodTypesForPayments::
                    get_payment_method_subtype,
                &app::model::payments::ResponsePaymentMethodTypesForPayments::
                    set_payment_method_subtype,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_experience",
                &app::model::payments::ResponsePaymentMethodTypesForPayments::
                    get_payment_experience,
                &app::model::payments::ResponsePaymentMethodTypesForPayments::
                    set_payment_experience,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "extra_information",
                &app::model::payments::ResponsePaymentMethodTypesForPayments::get_extra_information,
                &app::model::payments::ResponsePaymentMethodTypesForPayments::set_extra_information,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "required_fields",
                &app::model::payments::ResponsePaymentMethodTypesForPayments::get_required_fields,
                &app::model::payments::ResponsePaymentMethodTypesForPayments::set_required_fields,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "surcharge_details",
                &app::model::payments::ResponsePaymentMethodTypesForPayments::get_surcharge_details,
                &app::model::payments::ResponsePaymentMethodTypesForPayments::set_surcharge_details,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "response_payment_method_types_for_payments";
    }
};

template<>
struct serde::Serializable<app::model::payments::PaymentsExternalAuthenticationResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "trans_status",
                &app::model::payments::PaymentsExternalAuthenticationResponse::
                    get_transaction_status,
                &app::model::payments::PaymentsExternalAuthenticationResponse::
                    set_transaction_status,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "acs_url",
                &app::model::payments::PaymentsExternalAuthenticationResponse::get_acs_url,
                &app::model::payments::PaymentsExternalAuthenticationResponse::set_acs_url,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "challenge_request",
                &app::model::payments::PaymentsExternalAuthenticationResponse::
                    get_challenge_request,
                &app::model::payments::PaymentsExternalAuthenticationResponse::
                    set_challenge_request,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "challenge_request_key",
                &app::model::payments::PaymentsExternalAuthenticationResponse::
                    get_challenge_request_key,
                &app::model::payments::PaymentsExternalAuthenticationResponse::
                    set_challenge_request_key,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "acs_reference_number",
                &app::model::payments::PaymentsExternalAuthenticationResponse::
                    get_acs_reference_number,
                &app::model::payments::PaymentsExternalAuthenticationResponse::
                    set_acs_reference_number,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "acs_trans_id",
                &app::model::payments::PaymentsExternalAuthenticationResponse::get_acs_trans_id,
                &app::model::payments::PaymentsExternalAuthenticationResponse::set_acs_trans_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "three_dsserver_trans_id",
                &app::model::payments::PaymentsExternalAuthenticationResponse::
                    get_three_dsserver_trans_id,
                &app::model::payments::PaymentsExternalAuthenticationResponse::
                    set_three_dsserver_trans_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "acs_signed_content",
                &app::model::payments::PaymentsExternalAuthenticationResponse::
                    get_acs_signed_content,
                &app::model::payments::PaymentsExternalAuthenticationResponse::
                    set_acs_signed_content,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "three_ds_requestor_url",
                &app::model::payments::PaymentsExternalAuthenticationResponse::
                    get_three_ds_requestor_url,
                &app::model::payments::PaymentsExternalAuthenticationResponse::
                    set_three_ds_requestor_url,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "three_ds_requestor_app_url",
                &app::model::payments::PaymentsExternalAuthenticationResponse::
                    get_three_ds_requestor_app_url,
                &app::model::payments::PaymentsExternalAuthenticationResponse::
                    set_three_ds_requestor_app_url,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "error_message",
                &app::model::payments::PaymentsExternalAuthenticationResponse::get_error_message,
                &app::model::payments::PaymentsExternalAuthenticationResponse::set_error_message,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "payments_external_authentication_response";
    }
};

template<>
struct serde::Serializable<app::model::payments::PaymentsApproveRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<
            "payment_id",
            &app::model::payments::PaymentsApproveRequest::get_payment_id,
            &app::model::payments::PaymentsApproveRequest::set_payment_id,
            serde::FieldOptions{}>{}};
    }

    static constexpr std::string_view table_name()
    {
        return "payments_approve_request";
    }
};

template<>
struct serde::Serializable<app::model::payments::PaymentsRejectRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<
            "payment_id",
            &app::model::payments::PaymentsRejectRequest::get_payment_id,
            &app::model::payments::PaymentsRejectRequest::set_payment_id,
            serde::FieldOptions{}>{}};
    }

    static constexpr std::string_view table_name()
    {
        return "payments_reject_request";
    }
};

template<>
struct serde::Serializable<app::model::payments::PaymentsStartRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "payment_id",
                &app::model::payments::PaymentsStartRequest::get_payment_id,
                &app::model::payments::PaymentsStartRequest::set_payment_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "merchant_id",
                &app::model::payments::PaymentsStartRequest::get_merchant_id,
                &app::model::payments::PaymentsStartRequest::set_merchant_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "attempt_id",
                &app::model::payments::PaymentsStartRequest::get_attempt_id,
                &app::model::payments::PaymentsStartRequest::set_attempt_id,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "payments_start_request";
    }
};

template<>
struct serde::Serializable<app::model::payments::ImmediateExpirationTime>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "time",
                &app::model::payments::ImmediateExpirationTime::get_time,
                &app::model::payments::ImmediateExpirationTime::set_time,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "pix_key",
                &app::model::payments::ImmediateExpirationTime::get_pix_key,
                &app::model::payments::ImmediateExpirationTime::set_pix_key,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "immediate_expiration_time";
    }
};

template<>
struct serde::Serializable<app::model::payments::ScheduledExpirationTime>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "date",
                &app::model::payments::ScheduledExpirationTime::get_date,
                &app::model::payments::ScheduledExpirationTime::set_date,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "validity_after_expiration",
                &app::model::payments::ScheduledExpirationTime::get_validity_after_expiration,
                &app::model::payments::ScheduledExpirationTime::set_validity_after_expiration,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "pix_key",
                &app::model::payments::ScheduledExpirationTime::get_pix_key,
                &app::model::payments::ScheduledExpirationTime::set_pix_key,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "scheduled_expiration_time";
    }
};

template<>
struct serde::Serializable<app::model::payments::PixAutomaticoPushData>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "time",
                &app::model::payments::PixAutomaticoPushData::get_time,
                &app::model::payments::PixAutomaticoPushData::set_time,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "retry_policy",
                &app::model::payments::PixAutomaticoPushData::get_retry_policy,
                &app::model::payments::PixAutomaticoPushData::set_retry_policy,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "mandate_details",
                &app::model::payments::PixAutomaticoPushData::get_mandate_details,
                &app::model::payments::PixAutomaticoPushData::set_mandate_details,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "pix_automatico_push_data";
    }
};

template<>
struct serde::Serializable<app::model::payments::PixAutomaticoQrData>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "retry_policy",
                &app::model::payments::PixAutomaticoQrData::get_retry_policy,
                &app::model::payments::PixAutomaticoQrData::set_retry_policy,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "mandate_details",
                &app::model::payments::PixAutomaticoQrData::get_mandate_details,
                &app::model::payments::PixAutomaticoQrData::set_mandate_details,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "pix_automatico_qr_data";
    }
};

template<>
struct serde::Serializable<app::model::payments::PixAutomaticoMitData>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "receiver_details",
                &app::model::payments::PixAutomaticoMitData::get_receiver_details,
                &app::model::payments::PixAutomaticoMitData::set_receiver_details,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "mandate_execution_date",
                &app::model::payments::PixAutomaticoMitData::get_mandate_execution_date,
                &app::model::payments::PixAutomaticoMitData::set_mandate_execution_date,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "auto_adjust_date",
                &app::model::payments::PixAutomaticoMitData::get_auto_adjust_date,
                &app::model::payments::PixAutomaticoMitData::set_auto_adjust_date,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "pix_automatico_mit_data";
    }
};

template<>
struct serde::Serializable<app::model::payments::FrmMessage>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "frm_name",
                &app::model::payments::FrmMessage::get_frm_name,
                &app::model::payments::FrmMessage::set_frm_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "frm_transaction_id",
                &app::model::payments::FrmMessage::get_frm_transaction_id,
                &app::model::payments::FrmMessage::set_frm_transaction_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "frm_transaction_type",
                &app::model::payments::FrmMessage::get_frm_transaction_type,
                &app::model::payments::FrmMessage::set_frm_transaction_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "frm_status",
                &app::model::payments::FrmMessage::get_frm_status,
                &app::model::payments::FrmMessage::set_frm_status,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "frm_score",
                &app::model::payments::FrmMessage::get_frm_score,
                &app::model::payments::FrmMessage::set_frm_score,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "frm_reason",
                &app::model::payments::FrmMessage::get_frm_reason,
                &app::model::payments::FrmMessage::set_frm_reason,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "frm_error",
                &app::model::payments::FrmMessage::get_frm_error,
                &app::model::payments::FrmMessage::set_frm_error,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "frm_message";
    }
};

template<>
struct serde::Serializable<app::model::payments::RetrievePaymentLinkRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<
            "client_secret",
            &app::model::payments::RetrievePaymentLinkRequest::get_client_secret,
            &app::model::payments::RetrievePaymentLinkRequest::set_client_secret,
            serde::FieldOptions{}>{}};
    }

    static constexpr std::string_view table_name()
    {
        return "retrieve_payment_link_request";
    }
};

template<>
struct serde::Serializable<app::model::payments::PaymentLinkResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "link",
                &app::model::payments::PaymentLinkResponse::get_link,
                &app::model::payments::PaymentLinkResponse::set_link,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "secure_link",
                &app::model::payments::PaymentLinkResponse::get_secure_link,
                &app::model::payments::PaymentLinkResponse::set_secure_link,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_link_id",
                &app::model::payments::PaymentLinkResponse::get_payment_link_id,
                &app::model::payments::PaymentLinkResponse::set_payment_link_id,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "payment_link_response";
    }
};

template<>
struct serde::Serializable<app::model::payments::RetrievePaymentLinkResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "payment_link_id",
                &app::model::payments::RetrievePaymentLinkResponse::get_payment_link_id,
                &app::model::payments::RetrievePaymentLinkResponse::set_payment_link_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_id",
                &app::model::payments::RetrievePaymentLinkResponse::get_payment_id,
                &app::model::payments::RetrievePaymentLinkResponse::set_payment_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "merchant_id",
                &app::model::payments::RetrievePaymentLinkResponse::get_merchant_id,
                &app::model::payments::RetrievePaymentLinkResponse::set_merchant_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "processor_merchant_id",
                &app::model::payments::RetrievePaymentLinkResponse::get_processor_merchant_id,
                &app::model::payments::RetrievePaymentLinkResponse::set_processor_merchant_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "profile_id",
                &app::model::payments::RetrievePaymentLinkResponse::get_profile_id,
                &app::model::payments::RetrievePaymentLinkResponse::set_profile_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "link_to_pay",
                &app::model::payments::RetrievePaymentLinkResponse::get_link_to_pay,
                &app::model::payments::RetrievePaymentLinkResponse::set_link_to_pay,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "amount",
                &app::model::payments::RetrievePaymentLinkResponse::get_amount,
                &app::model::payments::RetrievePaymentLinkResponse::set_amount,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "created_at",
                &app::model::payments::RetrievePaymentLinkResponse::get_created_at,
                &app::model::payments::RetrievePaymentLinkResponse::set_created_at,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "expiry",
                &app::model::payments::RetrievePaymentLinkResponse::get_expiry,
                &app::model::payments::RetrievePaymentLinkResponse::set_expiry,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "description",
                &app::model::payments::RetrievePaymentLinkResponse::get_description,
                &app::model::payments::RetrievePaymentLinkResponse::set_description,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "status",
                &app::model::payments::RetrievePaymentLinkResponse::get_status,
                &app::model::payments::RetrievePaymentLinkResponse::set_status,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "currency",
                &app::model::payments::RetrievePaymentLinkResponse::get_currency,
                &app::model::payments::RetrievePaymentLinkResponse::set_currency,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "secure_link",
                &app::model::payments::RetrievePaymentLinkResponse::get_secure_link,
                &app::model::payments::RetrievePaymentLinkResponse::set_secure_link,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "retrieve_payment_link_response";
    }
};

template<>
struct serde::Serializable<app::model::payments::PaymentLinkInitiateRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "merchant_id",
                &app::model::payments::PaymentLinkInitiateRequest::get_merchant_id,
                &app::model::payments::PaymentLinkInitiateRequest::set_merchant_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_id",
                &app::model::payments::PaymentLinkInitiateRequest::get_payment_id,
                &app::model::payments::PaymentLinkInitiateRequest::set_payment_id,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "payment_link_initiate_request";
    }
};

template<>
struct serde::Serializable<app::model::payments::PaymentLinkDetails>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "amount",
                &app::model::payments::PaymentLinkDetails::get_amount,
                &app::model::payments::PaymentLinkDetails::set_amount,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "currency",
                &app::model::payments::PaymentLinkDetails::get_currency,
                &app::model::payments::PaymentLinkDetails::set_currency,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "pub_key",
                &app::model::payments::PaymentLinkDetails::get_pub_key,
                &app::model::payments::PaymentLinkDetails::set_pub_key,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "client_secret",
                &app::model::payments::PaymentLinkDetails::get_client_secret,
                &app::model::payments::PaymentLinkDetails::set_client_secret,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_id",
                &app::model::payments::PaymentLinkDetails::get_payment_id,
                &app::model::payments::PaymentLinkDetails::set_payment_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "session_expiry",
                &app::model::payments::PaymentLinkDetails::get_session_expiry,
                &app::model::payments::PaymentLinkDetails::set_session_expiry,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "merchant_logo",
                &app::model::payments::PaymentLinkDetails::get_merchant_logo,
                &app::model::payments::PaymentLinkDetails::set_merchant_logo,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "return_url",
                &app::model::payments::PaymentLinkDetails::get_return_url,
                &app::model::payments::PaymentLinkDetails::set_return_url,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "merchant_name",
                &app::model::payments::PaymentLinkDetails::get_merchant_name,
                &app::model::payments::PaymentLinkDetails::set_merchant_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "order_details",
                &app::model::payments::PaymentLinkDetails::get_order_details,
                &app::model::payments::PaymentLinkDetails::set_order_details,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "max_items_visible_after_collapse",
                &app::model::payments::PaymentLinkDetails::get_max_items_visible_after_collapse,
                &app::model::payments::PaymentLinkDetails::set_max_items_visible_after_collapse,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "theme",
                &app::model::payments::PaymentLinkDetails::get_theme,
                &app::model::payments::PaymentLinkDetails::set_theme,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "merchant_description",
                &app::model::payments::PaymentLinkDetails::get_merchant_description,
                &app::model::payments::PaymentLinkDetails::set_merchant_description,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "sdk_layout",
                &app::model::payments::PaymentLinkDetails::get_sdk_layout,
                &app::model::payments::PaymentLinkDetails::set_sdk_layout,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "display_sdk_only",
                &app::model::payments::PaymentLinkDetails::get_display_sdk_only,
                &app::model::payments::PaymentLinkDetails::set_display_sdk_only,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "hide_card_nickname_field",
                &app::model::payments::PaymentLinkDetails::get_hide_card_nickname_field,
                &app::model::payments::PaymentLinkDetails::set_hide_card_nickname_field,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "show_card_form_by_default",
                &app::model::payments::PaymentLinkDetails::get_show_card_form_by_default,
                &app::model::payments::PaymentLinkDetails::set_show_card_form_by_default,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "locale",
                &app::model::payments::PaymentLinkDetails::get_locale,
                &app::model::payments::PaymentLinkDetails::set_locale,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "transaction_details",
                &app::model::payments::PaymentLinkDetails::get_transaction_details,
                &app::model::payments::PaymentLinkDetails::set_transaction_details,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "background_image",
                &app::model::payments::PaymentLinkDetails::get_background_image,
                &app::model::payments::PaymentLinkDetails::set_background_image,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "details_layout",
                &app::model::payments::PaymentLinkDetails::get_details_layout,
                &app::model::payments::PaymentLinkDetails::set_details_layout,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "branding_visibility",
                &app::model::payments::PaymentLinkDetails::get_branding_visibility,
                &app::model::payments::PaymentLinkDetails::set_branding_visibility,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_button_text",
                &app::model::payments::PaymentLinkDetails::get_payment_button_text,
                &app::model::payments::PaymentLinkDetails::set_payment_button_text,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "skip_status_screen",
                &app::model::payments::PaymentLinkDetails::get_skip_status_screen,
                &app::model::payments::PaymentLinkDetails::set_skip_status_screen,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "custom_message_for_card_terms",
                &app::model::payments::PaymentLinkDetails::get_custom_message_for_card_terms,
                &app::model::payments::PaymentLinkDetails::set_custom_message_for_card_terms,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "custom_message_for_payment_method_types",
                &app::model::payments::PaymentLinkDetails::
                    get_custom_message_for_payment_method_types,
                &app::model::payments::PaymentLinkDetails::
                    set_custom_message_for_payment_method_types,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_button_colour",
                &app::model::payments::PaymentLinkDetails::get_payment_button_colour,
                &app::model::payments::PaymentLinkDetails::set_payment_button_colour,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_button_text_colour",
                &app::model::payments::PaymentLinkDetails::get_payment_button_text_colour,
                &app::model::payments::PaymentLinkDetails::set_payment_button_text_colour,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "background_colour",
                &app::model::payments::PaymentLinkDetails::get_background_colour,
                &app::model::payments::PaymentLinkDetails::set_background_colour,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "sdk_ui_rules",
                &app::model::payments::PaymentLinkDetails::get_sdk_ui_rules,
                &app::model::payments::PaymentLinkDetails::set_sdk_ui_rules,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "status",
                &app::model::payments::PaymentLinkDetails::get_status,
                &app::model::payments::PaymentLinkDetails::set_status,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "enable_button_only_on_form_ready",
                &app::model::payments::PaymentLinkDetails::get_enable_button_only_on_form_ready,
                &app::model::payments::PaymentLinkDetails::set_enable_button_only_on_form_ready,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_form_header_text",
                &app::model::payments::PaymentLinkDetails::get_payment_form_header_text,
                &app::model::payments::PaymentLinkDetails::set_payment_form_header_text,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_form_label_type",
                &app::model::payments::PaymentLinkDetails::get_payment_form_label_type,
                &app::model::payments::PaymentLinkDetails::set_payment_form_label_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "show_card_terms",
                &app::model::payments::PaymentLinkDetails::get_show_card_terms,
                &app::model::payments::PaymentLinkDetails::set_show_card_terms,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "is_setup_mandate_flow",
                &app::model::payments::PaymentLinkDetails::get_is_setup_mandate_flow,
                &app::model::payments::PaymentLinkDetails::set_is_setup_mandate_flow,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "capture_method",
                &app::model::payments::PaymentLinkDetails::get_capture_method,
                &app::model::payments::PaymentLinkDetails::set_capture_method,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "setup_future_usage_applied",
                &app::model::payments::PaymentLinkDetails::get_setup_future_usage_applied,
                &app::model::payments::PaymentLinkDetails::set_setup_future_usage_applied,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "color_icon_card_cvc_error",
                &app::model::payments::PaymentLinkDetails::get_color_icon_card_cvc_error,
                &app::model::payments::PaymentLinkDetails::set_color_icon_card_cvc_error,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "show_merchant_name",
                &app::model::payments::PaymentLinkDetails::get_show_merchant_name,
                &app::model::payments::PaymentLinkDetails::set_show_merchant_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_methods_separator_text",
                &app::model::payments::PaymentLinkDetails::get_payment_methods_separator_text,
                &app::model::payments::PaymentLinkDetails::set_payment_methods_separator_text,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "payment_link_details";
    }
};

template<>
struct serde::Serializable<app::model::payments::SecurePaymentLinkDetails>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "enabled_saved_payment_method",
                &app::model::payments::SecurePaymentLinkDetails::get_enabled_saved_payment_method,
                &app::model::payments::SecurePaymentLinkDetails::set_enabled_saved_payment_method,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "hide_card_nickname_field",
                &app::model::payments::SecurePaymentLinkDetails::get_hide_card_nickname_field,
                &app::model::payments::SecurePaymentLinkDetails::set_hide_card_nickname_field,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "show_card_form_by_default",
                &app::model::payments::SecurePaymentLinkDetails::get_show_card_form_by_default,
                &app::model::payments::SecurePaymentLinkDetails::set_show_card_form_by_default,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_link_details",
                &app::model::payments::SecurePaymentLinkDetails::get_payment_link_details,
                &app::model::payments::SecurePaymentLinkDetails::set_payment_link_details,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_button_text",
                &app::model::payments::SecurePaymentLinkDetails::get_payment_button_text,
                &app::model::payments::SecurePaymentLinkDetails::set_payment_button_text,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "skip_status_screen",
                &app::model::payments::SecurePaymentLinkDetails::get_skip_status_screen,
                &app::model::payments::SecurePaymentLinkDetails::set_skip_status_screen,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "custom_message_for_card_terms",
                &app::model::payments::SecurePaymentLinkDetails::get_custom_message_for_card_terms,
                &app::model::payments::SecurePaymentLinkDetails::set_custom_message_for_card_terms,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "custom_message_for_payment_method_types",
                &app::model::payments::SecurePaymentLinkDetails::
                    get_custom_message_for_payment_method_types,
                &app::model::payments::SecurePaymentLinkDetails::
                    set_custom_message_for_payment_method_types,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_button_colour",
                &app::model::payments::SecurePaymentLinkDetails::get_payment_button_colour,
                &app::model::payments::SecurePaymentLinkDetails::set_payment_button_colour,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_button_text_colour",
                &app::model::payments::SecurePaymentLinkDetails::get_payment_button_text_colour,
                &app::model::payments::SecurePaymentLinkDetails::set_payment_button_text_colour,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "background_colour",
                &app::model::payments::SecurePaymentLinkDetails::get_background_colour,
                &app::model::payments::SecurePaymentLinkDetails::set_background_colour,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "sdk_ui_rules",
                &app::model::payments::SecurePaymentLinkDetails::get_sdk_ui_rules,
                &app::model::payments::SecurePaymentLinkDetails::set_sdk_ui_rules,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "enable_button_only_on_form_ready",
                &app::model::payments::SecurePaymentLinkDetails::
                    get_enable_button_only_on_form_ready,
                &app::model::payments::SecurePaymentLinkDetails::
                    set_enable_button_only_on_form_ready,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_form_header_text",
                &app::model::payments::SecurePaymentLinkDetails::get_payment_form_header_text,
                &app::model::payments::SecurePaymentLinkDetails::set_payment_form_header_text,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_form_label_type",
                &app::model::payments::SecurePaymentLinkDetails::get_payment_form_label_type,
                &app::model::payments::SecurePaymentLinkDetails::set_payment_form_label_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "show_card_terms",
                &app::model::payments::SecurePaymentLinkDetails::get_show_card_terms,
                &app::model::payments::SecurePaymentLinkDetails::set_show_card_terms,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "color_icon_card_cvc_error",
                &app::model::payments::SecurePaymentLinkDetails::get_color_icon_card_cvc_error,
                &app::model::payments::SecurePaymentLinkDetails::set_color_icon_card_cvc_error,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_methods_separator_text",
                &app::model::payments::SecurePaymentLinkDetails::get_payment_methods_separator_text,
                &app::model::payments::SecurePaymentLinkDetails::set_payment_methods_separator_text,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "secure_payment_link_details";
    }
};

template<>
struct serde::Serializable<app::model::payments::PaymentLinkStatusDetails>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "amount",
                &app::model::payments::PaymentLinkStatusDetails::get_amount,
                &app::model::payments::PaymentLinkStatusDetails::set_amount,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "currency",
                &app::model::payments::PaymentLinkStatusDetails::get_currency,
                &app::model::payments::PaymentLinkStatusDetails::set_currency,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_id",
                &app::model::payments::PaymentLinkStatusDetails::get_payment_id,
                &app::model::payments::PaymentLinkStatusDetails::set_payment_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "merchant_logo",
                &app::model::payments::PaymentLinkStatusDetails::get_merchant_logo,
                &app::model::payments::PaymentLinkStatusDetails::set_merchant_logo,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "merchant_name",
                &app::model::payments::PaymentLinkStatusDetails::get_merchant_name,
                &app::model::payments::PaymentLinkStatusDetails::set_merchant_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "created",
                &app::model::payments::PaymentLinkStatusDetails::get_created,
                &app::model::payments::PaymentLinkStatusDetails::set_created,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "status",
                &app::model::payments::PaymentLinkStatusDetails::get_status,
                &app::model::payments::PaymentLinkStatusDetails::set_status,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "error_code",
                &app::model::payments::PaymentLinkStatusDetails::get_error_code,
                &app::model::payments::PaymentLinkStatusDetails::set_error_code,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "error_message",
                &app::model::payments::PaymentLinkStatusDetails::get_error_message,
                &app::model::payments::PaymentLinkStatusDetails::set_error_message,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "redirect",
                &app::model::payments::PaymentLinkStatusDetails::get_redirect,
                &app::model::payments::PaymentLinkStatusDetails::set_redirect,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "theme",
                &app::model::payments::PaymentLinkStatusDetails::get_theme,
                &app::model::payments::PaymentLinkStatusDetails::set_theme,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "return_url",
                &app::model::payments::PaymentLinkStatusDetails::get_return_url,
                &app::model::payments::PaymentLinkStatusDetails::set_return_url,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "locale",
                &app::model::payments::PaymentLinkStatusDetails::get_locale,
                &app::model::payments::PaymentLinkStatusDetails::set_locale,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "transaction_details",
                &app::model::payments::PaymentLinkStatusDetails::get_transaction_details,
                &app::model::payments::PaymentLinkStatusDetails::set_transaction_details,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "unified_code",
                &app::model::payments::PaymentLinkStatusDetails::get_unified_code,
                &app::model::payments::PaymentLinkStatusDetails::set_unified_code,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "unified_message",
                &app::model::payments::PaymentLinkStatusDetails::get_unified_message,
                &app::model::payments::PaymentLinkStatusDetails::set_unified_message,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "capture_method",
                &app::model::payments::PaymentLinkStatusDetails::get_capture_method,
                &app::model::payments::PaymentLinkStatusDetails::set_capture_method,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "setup_future_usage_applied",
                &app::model::payments::PaymentLinkStatusDetails::get_setup_future_usage_applied,
                &app::model::payments::PaymentLinkStatusDetails::set_setup_future_usage_applied,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "redirect_delay_seconds",
                &app::model::payments::PaymentLinkStatusDetails::get_redirect_delay_seconds,
                &app::model::payments::PaymentLinkStatusDetails::set_redirect_delay_seconds,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "payment_link_status_details";
    }
};

template<>
struct serde::Serializable<app::model::payments::PaymentLinkListConstraints>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "limit",
                &app::model::payments::PaymentLinkListConstraints::get_limit,
                &app::model::payments::PaymentLinkListConstraints::set_limit,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "offset",
                &app::model::payments::PaymentLinkListConstraints::get_offset,
                &app::model::payments::PaymentLinkListConstraints::set_offset,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "time_range",
                &app::model::payments::PaymentLinkListConstraints::get_time_range,
                &app::model::payments::PaymentLinkListConstraints::set_time_range,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "payment_link_list_constraints";
    }
};

template<>
struct serde::Serializable<app::model::payments::PaymentLinkListResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "size",
                &app::model::payments::PaymentLinkListResponse::get_size,
                &app::model::payments::PaymentLinkListResponse::set_size,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "total_count",
                &app::model::payments::PaymentLinkListResponse::get_total_count,
                &app::model::payments::PaymentLinkListResponse::set_total_count,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "data",
                &app::model::payments::PaymentLinkListResponse::get_data,
                &app::model::payments::PaymentLinkListResponse::set_data,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "payment_link_list_response";
    }
};

template<>
struct serde::Serializable<app::model::payments::PaymentCreatePaymentLinkConfig>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<
            "theme_config",
            &app::model::payments::PaymentCreatePaymentLinkConfig::get_theme_config,
            &app::model::payments::PaymentCreatePaymentLinkConfig::set_theme_config,
            serde::FieldOptions{}>{}};
    }

    static constexpr std::string_view table_name()
    {
        return "payment_create_payment_link_config";
    }
};

template<>
struct serde::Serializable<app::model::payments::OrderDetailsWithStringAmount>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "product_name",
                &app::model::payments::OrderDetailsWithStringAmount::get_product_name,
                &app::model::payments::OrderDetailsWithStringAmount::set_product_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "quantity",
                &app::model::payments::OrderDetailsWithStringAmount::get_quantity,
                &app::model::payments::OrderDetailsWithStringAmount::set_quantity,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "amount",
                &app::model::payments::OrderDetailsWithStringAmount::get_amount,
                &app::model::payments::OrderDetailsWithStringAmount::set_amount,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "product_img_link",
                &app::model::payments::OrderDetailsWithStringAmount::get_product_img_link,
                &app::model::payments::OrderDetailsWithStringAmount::set_product_img_link,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "order_details_with_string_amount";
    }
};

template<>
struct serde::Serializable<app::model::payments::ExtendedCardInfoResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<
            "payload",
            &app::model::payments::ExtendedCardInfoResponse::get_payload,
            &app::model::payments::ExtendedCardInfoResponse::set_payload,
            serde::FieldOptions{}>{}};
    }

    static constexpr std::string_view table_name()
    {
        return "extended_card_info_response";
    }
};

template<>
struct serde::Serializable<app::model::payments::ClickToPaySessionResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "dpa_id",
                &app::model::payments::ClickToPaySessionResponse::get_dpa_id,
                &app::model::payments::ClickToPaySessionResponse::set_dpa_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "dpa_name",
                &app::model::payments::ClickToPaySessionResponse::get_dpa_name,
                &app::model::payments::ClickToPaySessionResponse::set_dpa_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "locale",
                &app::model::payments::ClickToPaySessionResponse::get_locale,
                &app::model::payments::ClickToPaySessionResponse::set_locale,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_brands",
                &app::model::payments::ClickToPaySessionResponse::get_card_brands,
                &app::model::payments::ClickToPaySessionResponse::set_card_brands,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "acquirer_bin",
                &app::model::payments::ClickToPaySessionResponse::get_acquirer_bin,
                &app::model::payments::ClickToPaySessionResponse::set_acquirer_bin,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "acquirer_merchant_id",
                &app::model::payments::ClickToPaySessionResponse::get_acquirer_merchant_id,
                &app::model::payments::ClickToPaySessionResponse::set_acquirer_merchant_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "merchant_category_code",
                &app::model::payments::ClickToPaySessionResponse::get_merchant_category_code,
                &app::model::payments::ClickToPaySessionResponse::set_merchant_category_code,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "merchant_country_code",
                &app::model::payments::ClickToPaySessionResponse::get_merchant_country_code,
                &app::model::payments::ClickToPaySessionResponse::set_merchant_country_code,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "transaction_amount",
                &app::model::payments::ClickToPaySessionResponse::get_transaction_amount,
                &app::model::payments::ClickToPaySessionResponse::set_transaction_amount,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "transaction_currency_code",
                &app::model::payments::ClickToPaySessionResponse::get_transaction_currency_code,
                &app::model::payments::ClickToPaySessionResponse::set_transaction_currency_code,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "phone_number",
                &app::model::payments::ClickToPaySessionResponse::get_phone_number,
                &app::model::payments::ClickToPaySessionResponse::set_phone_number,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "email",
                &app::model::payments::ClickToPaySessionResponse::get_email,
                &app::model::payments::ClickToPaySessionResponse::set_email,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "phone_country_code",
                &app::model::payments::ClickToPaySessionResponse::get_phone_country_code,
                &app::model::payments::ClickToPaySessionResponse::set_phone_country_code,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "provider",
                &app::model::payments::ClickToPaySessionResponse::get_provider,
                &app::model::payments::ClickToPaySessionResponse::set_provider,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "dpa_client_id",
                &app::model::payments::ClickToPaySessionResponse::get_dpa_client_id,
                &app::model::payments::ClickToPaySessionResponse::set_dpa_client_id,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "click_to_pay_session_response";
    }
};

template<>
struct serde::Serializable<app::model::payments::PaymentsEligibilityCheckRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "payment_id",
                &app::model::payments::PaymentsEligibilityCheckRequest::get_payment_id,
                &app::model::payments::PaymentsEligibilityCheckRequest::set_payment_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "client_secret",
                &app::model::payments::PaymentsEligibilityCheckRequest::get_client_secret,
                &app::model::payments::PaymentsEligibilityCheckRequest::set_client_secret,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_method_type",
                &app::model::payments::PaymentsEligibilityCheckRequest::get_payment_method_type,
                &app::model::payments::PaymentsEligibilityCheckRequest::set_payment_method_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_method_subtype",
                &app::model::payments::PaymentsEligibilityCheckRequest::get_payment_method_subtype,
                &app::model::payments::PaymentsEligibilityCheckRequest::set_payment_method_subtype,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_method_data",
                &app::model::payments::PaymentsEligibilityCheckRequest::get_payment_method_data,
                &app::model::payments::PaymentsEligibilityCheckRequest::set_payment_method_data,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "browser_info",
                &app::model::payments::PaymentsEligibilityCheckRequest::get_browser_info,
                &app::model::payments::PaymentsEligibilityCheckRequest::set_browser_info,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_token",
                &app::model::payments::PaymentsEligibilityCheckRequest::get_payment_token,
                &app::model::payments::PaymentsEligibilityCheckRequest::set_payment_token,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "payments_eligibility_check_request";
    }
};

template<>
struct serde::Serializable<app::model::payments::EligibilityCard>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "card_number",
                &app::model::payments::EligibilityCard::get_card_number,
                &app::model::payments::EligibilityCard::set_card_number,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_exp_month",
                &app::model::payments::EligibilityCard::get_card_exp_month,
                &app::model::payments::EligibilityCard::set_card_exp_month,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_exp_year",
                &app::model::payments::EligibilityCard::get_card_exp_year,
                &app::model::payments::EligibilityCard::set_card_exp_year,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_cvc",
                &app::model::payments::EligibilityCard::get_card_cvc,
                &app::model::payments::EligibilityCard::set_card_cvc,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_holder_name",
                &app::model::payments::EligibilityCard::get_card_holder_name,
                &app::model::payments::EligibilityCard::set_card_holder_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_issuer",
                &app::model::payments::EligibilityCard::get_card_issuer,
                &app::model::payments::EligibilityCard::set_card_issuer,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_network",
                &app::model::payments::EligibilityCard::get_card_network,
                &app::model::payments::EligibilityCard::set_card_network,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_type",
                &app::model::payments::EligibilityCard::get_card_type,
                &app::model::payments::EligibilityCard::set_card_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_issuing_country",
                &app::model::payments::EligibilityCard::get_card_issuing_country,
                &app::model::payments::EligibilityCard::set_card_issuing_country,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_issuing_country_code",
                &app::model::payments::EligibilityCard::get_card_issuing_country_code,
                &app::model::payments::EligibilityCard::set_card_issuing_country_code,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "bank_code",
                &app::model::payments::EligibilityCard::get_bank_code,
                &app::model::payments::EligibilityCard::set_bank_code,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "nick_name",
                &app::model::payments::EligibilityCard::get_nick_name,
                &app::model::payments::EligibilityCard::set_nick_name,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "eligibility_card";
    }
};

template<>
struct serde::Serializable<app::model::payments::EligibilityCardBin>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<
            "card_bin",
            &app::model::payments::EligibilityCardBin::get_card_bin,
            &app::model::payments::EligibilityCardBin::set_card_bin,
            serde::FieldOptions{}>{}};
    }

    static constexpr std::string_view table_name()
    {
        return "eligibility_card_bin";
    }
};

template<>
struct serde::Serializable<app::model::payments::PaymentsEligibilityCheckResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "payment_id",
                &app::model::payments::PaymentsEligibilityCheckResponse::get_payment_id,
                &app::model::payments::PaymentsEligibilityCheckResponse::set_payment_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "sdk_next_action",
                &app::model::payments::PaymentsEligibilityCheckResponse::get_sdk_next_action,
                &app::model::payments::PaymentsEligibilityCheckResponse::set_sdk_next_action,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "payments_eligibility_check_response";
    }
};

template<>
struct serde::Serializable<app::model::payments::PaymentsEligibilityRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "payment_id",
                &app::model::payments::PaymentsEligibilityRequest::get_payment_id,
                &app::model::payments::PaymentsEligibilityRequest::set_payment_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "client_secret",
                &app::model::payments::PaymentsEligibilityRequest::get_client_secret,
                &app::model::payments::PaymentsEligibilityRequest::set_client_secret,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_method_type",
                &app::model::payments::PaymentsEligibilityRequest::get_payment_method_type,
                &app::model::payments::PaymentsEligibilityRequest::set_payment_method_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_method_subtype",
                &app::model::payments::PaymentsEligibilityRequest::get_payment_method_subtype,
                &app::model::payments::PaymentsEligibilityRequest::set_payment_method_subtype,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_method_data",
                &app::model::payments::PaymentsEligibilityRequest::get_payment_method_data,
                &app::model::payments::PaymentsEligibilityRequest::set_payment_method_data,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "browser_info",
                &app::model::payments::PaymentsEligibilityRequest::get_browser_info,
                &app::model::payments::PaymentsEligibilityRequest::set_browser_info,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_token",
                &app::model::payments::PaymentsEligibilityRequest::get_payment_token,
                &app::model::payments::PaymentsEligibilityRequest::set_payment_token,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "payments_eligibility_request";
    }
};

template<>
struct serde::Serializable<app::model::payments::PaymentsEligibilityResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "payment_id",
                &app::model::payments::PaymentsEligibilityResponse::get_payment_id,
                &app::model::payments::PaymentsEligibilityResponse::set_payment_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "sdk_next_action",
                &app::model::payments::PaymentsEligibilityResponse::get_sdk_next_action,
                &app::model::payments::PaymentsEligibilityResponse::set_sdk_next_action,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "surcharge_details",
                &app::model::payments::PaymentsEligibilityResponse::get_surcharge_details,
                &app::model::payments::PaymentsEligibilityResponse::set_surcharge_details,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "amount_details",
                &app::model::payments::PaymentsEligibilityResponse::get_amount_details,
                &app::model::payments::PaymentsEligibilityResponse::set_amount_details,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "offer_details",
                &app::model::payments::PaymentsEligibilityResponse::get_offer_details,
                &app::model::payments::PaymentsEligibilityResponse::set_offer_details,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "payments_eligibility_response";
    }
};

template<>
struct serde::Serializable<app::model::payments::EligibilityAmountDetails>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "total_amount",
                &app::model::payments::EligibilityAmountDetails::get_total_amount,
                &app::model::payments::EligibilityAmountDetails::set_total_amount,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "net_amount",
                &app::model::payments::EligibilityAmountDetails::get_net_amount,
                &app::model::payments::EligibilityAmountDetails::set_net_amount,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "currency",
                &app::model::payments::EligibilityAmountDetails::get_currency,
                &app::model::payments::EligibilityAmountDetails::set_currency,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "eligibility_amount_details";
    }
};

template<>
struct serde::Serializable<app::model::payments::EligibilityOfferDetails>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "uplifted_offer_quote_ids",
                &app::model::payments::EligibilityOfferDetails::get_uplifted_offer_quote_ids,
                &app::model::payments::EligibilityOfferDetails::set_uplifted_offer_quote_ids,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "eligible_offers",
                &app::model::payments::EligibilityOfferDetails::get_eligible_offers,
                &app::model::payments::EligibilityOfferDetails::set_eligible_offers,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "eligibility_offer_details";
    }
};

template<>
struct serde::Serializable<app::model::payments::EligibleOffer>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "offer_quote_id",
                &app::model::payments::EligibleOffer::get_offer_quote_id,
                &app::model::payments::EligibleOffer::set_offer_quote_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "offer_amount",
                &app::model::payments::EligibleOffer::get_offer_amount,
                &app::model::payments::EligibleOffer::set_offer_amount,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "currency",
                &app::model::payments::EligibleOffer::get_currency,
                &app::model::payments::EligibleOffer::set_currency,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "code",
                &app::model::payments::EligibleOffer::get_code,
                &app::model::payments::EligibleOffer::set_code,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "title",
                &app::model::payments::EligibleOffer::get_title,
                &app::model::payments::EligibleOffer::set_title,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "description",
                &app::model::payments::EligibleOffer::get_description,
                &app::model::payments::EligibleOffer::set_description,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "eligible_offer";
    }
};

template<>
struct serde::Serializable<app::model::payments::OfferSelection>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<
            "offer_quote_ids",
            &app::model::payments::OfferSelection::get_offer_quote_ids,
            &app::model::payments::OfferSelection::set_offer_quote_ids,
            serde::FieldOptions{}>{}};
    }

    static constexpr std::string_view table_name()
    {
        return "offer_selection";
    }
};

template<>
struct serde::Serializable<app::model::payments::AppliedOffer>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "offer_engine_merchant_id",
                &app::model::payments::AppliedOffer::get_offer_engine_merchant_id,
                &app::model::payments::AppliedOffer::set_offer_engine_merchant_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "offer_engine_txn_id",
                &app::model::payments::AppliedOffer::get_offer_engine_txn_id,
                &app::model::payments::AppliedOffer::set_offer_engine_txn_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "offer_id",
                &app::model::payments::AppliedOffer::get_offer_id,
                &app::model::payments::AppliedOffer::set_offer_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "offer_amount",
                &app::model::payments::AppliedOffer::get_offer_amount,
                &app::model::payments::AppliedOffer::set_offer_amount,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "currency",
                &app::model::payments::AppliedOffer::get_currency,
                &app::model::payments::AppliedOffer::set_currency,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "applied_offer";
    }
};

template<>
struct serde::Serializable<app::model::payments::PaymentsAttemptRecordRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "amount_details",
                &app::model::payments::PaymentsAttemptRecordRequest::get_amount_details,
                &app::model::payments::PaymentsAttemptRecordRequest::set_amount_details,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "status",
                &app::model::payments::PaymentsAttemptRecordRequest::get_status,
                &app::model::payments::PaymentsAttemptRecordRequest::set_status,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "billing",
                &app::model::payments::PaymentsAttemptRecordRequest::get_billing,
                &app::model::payments::PaymentsAttemptRecordRequest::set_billing,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "shipping",
                &app::model::payments::PaymentsAttemptRecordRequest::get_shipping,
                &app::model::payments::PaymentsAttemptRecordRequest::set_shipping,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "error",
                &app::model::payments::PaymentsAttemptRecordRequest::get_error,
                &app::model::payments::PaymentsAttemptRecordRequest::set_error,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "description",
                &app::model::payments::PaymentsAttemptRecordRequest::get_description,
                &app::model::payments::PaymentsAttemptRecordRequest::set_description,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "connector_transaction_id",
                &app::model::payments::PaymentsAttemptRecordRequest::get_connector_transaction_id,
                &app::model::payments::PaymentsAttemptRecordRequest::set_connector_transaction_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_method_type",
                &app::model::payments::PaymentsAttemptRecordRequest::get_payment_method_type,
                &app::model::payments::PaymentsAttemptRecordRequest::set_payment_method_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "connector_details",
                &app::model::payments::PaymentsAttemptRecordRequest::get_connector_details,
                &app::model::payments::PaymentsAttemptRecordRequest::set_connector_details,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_method_subtype",
                &app::model::payments::PaymentsAttemptRecordRequest::get_payment_method_subtype,
                &app::model::payments::PaymentsAttemptRecordRequest::set_payment_method_subtype,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_method_data",
                &app::model::payments::PaymentsAttemptRecordRequest::get_payment_method_data,
                &app::model::payments::PaymentsAttemptRecordRequest::set_payment_method_data,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "metadata",
                &app::model::payments::PaymentsAttemptRecordRequest::get_metadata,
                &app::model::payments::PaymentsAttemptRecordRequest::set_metadata,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "feature_metadata",
                &app::model::payments::PaymentsAttemptRecordRequest::get_feature_metadata,
                &app::model::payments::PaymentsAttemptRecordRequest::set_feature_metadata,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "transaction_created_at",
                &app::model::payments::PaymentsAttemptRecordRequest::get_transaction_created_at,
                &app::model::payments::PaymentsAttemptRecordRequest::set_transaction_created_at,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "processor_payment_method_token",
                &app::model::payments::PaymentsAttemptRecordRequest::
                    get_processor_payment_method_token,
                &app::model::payments::PaymentsAttemptRecordRequest::
                    set_processor_payment_method_token,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "connector_customer_id",
                &app::model::payments::PaymentsAttemptRecordRequest::get_connector_customer_id,
                &app::model::payments::PaymentsAttemptRecordRequest::set_connector_customer_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "retry_count",
                &app::model::payments::PaymentsAttemptRecordRequest::get_retry_count,
                &app::model::payments::PaymentsAttemptRecordRequest::set_retry_count,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "invoice_next_billing_time",
                &app::model::payments::PaymentsAttemptRecordRequest::get_invoice_next_billing_time,
                &app::model::payments::PaymentsAttemptRecordRequest::set_invoice_next_billing_time,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "invoice_billing_started_at_time",
                &app::model::payments::PaymentsAttemptRecordRequest::
                    get_invoice_billing_started_at_time,
                &app::model::payments::PaymentsAttemptRecordRequest::
                    set_invoice_billing_started_at_time,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "triggered_by",
                &app::model::payments::PaymentsAttemptRecordRequest::get_triggered_by,
                &app::model::payments::PaymentsAttemptRecordRequest::set_triggered_by,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_network",
                &app::model::payments::PaymentsAttemptRecordRequest::get_card_network,
                &app::model::payments::PaymentsAttemptRecordRequest::set_card_network,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_issuer",
                &app::model::payments::PaymentsAttemptRecordRequest::get_card_issuer,
                &app::model::payments::PaymentsAttemptRecordRequest::set_card_issuer,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "standardised_error_code",
                &app::model::payments::PaymentsAttemptRecordRequest::get_standardised_error_code,
                &app::model::payments::PaymentsAttemptRecordRequest::set_standardised_error_code,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "payments_attempt_record_request";
    }
};

template<>
struct serde::Serializable<app::model::payments::RecoveryPaymentsCreate>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "amount_details",
                &app::model::payments::RecoveryPaymentsCreate::get_amount_details,
                &app::model::payments::RecoveryPaymentsCreate::set_amount_details,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "merchant_reference_id",
                &app::model::payments::RecoveryPaymentsCreate::get_merchant_reference_id,
                &app::model::payments::RecoveryPaymentsCreate::set_merchant_reference_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "error",
                &app::model::payments::RecoveryPaymentsCreate::get_error,
                &app::model::payments::RecoveryPaymentsCreate::set_error,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "billing_merchant_connector_id",
                &app::model::payments::RecoveryPaymentsCreate::get_billing_merchant_connector_id,
                &app::model::payments::RecoveryPaymentsCreate::set_billing_merchant_connector_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_merchant_connector_id",
                &app::model::payments::RecoveryPaymentsCreate::get_payment_merchant_connector_id,
                &app::model::payments::RecoveryPaymentsCreate::set_payment_merchant_connector_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "merchant_reference_billing_connector_id",
                &app::model::payments::RecoveryPaymentsCreate::
                    get_merchant_reference_billing_connector_id,
                &app::model::payments::RecoveryPaymentsCreate::
                    set_merchant_reference_billing_connector_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "merchant_reference_payment_connector_id",
                &app::model::payments::RecoveryPaymentsCreate::
                    get_merchant_reference_payment_connector_id,
                &app::model::payments::RecoveryPaymentsCreate::
                    set_merchant_reference_payment_connector_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "connector",
                &app::model::payments::RecoveryPaymentsCreate::get_connector,
                &app::model::payments::RecoveryPaymentsCreate::set_connector,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "transaction_status",
                &app::model::payments::RecoveryPaymentsCreate::get_transaction_status,
                &app::model::payments::RecoveryPaymentsCreate::set_transaction_status,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "amount_captured",
                &app::model::payments::RecoveryPaymentsCreate::get_amount_captured,
                &app::model::payments::RecoveryPaymentsCreate::set_amount_captured,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "mit_category",
                &app::model::payments::RecoveryPaymentsCreate::get_mit_category,
                &app::model::payments::RecoveryPaymentsCreate::set_mit_category,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "customer_present",
                &app::model::payments::RecoveryPaymentsCreate::get_customer_present,
                &app::model::payments::RecoveryPaymentsCreate::set_customer_present,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "setup_future_usage",
                &app::model::payments::RecoveryPaymentsCreate::get_setup_future_usage,
                &app::model::payments::RecoveryPaymentsCreate::set_setup_future_usage,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "billing",
                &app::model::payments::RecoveryPaymentsCreate::get_billing,
                &app::model::payments::RecoveryPaymentsCreate::set_billing,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "customer_details",
                &app::model::payments::RecoveryPaymentsCreate::get_customer_details,
                &app::model::payments::RecoveryPaymentsCreate::set_customer_details,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_method_type",
                &app::model::payments::RecoveryPaymentsCreate::get_payment_method_type,
                &app::model::payments::RecoveryPaymentsCreate::set_payment_method_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_method_sub_type",
                &app::model::payments::RecoveryPaymentsCreate::get_payment_method_sub_type,
                &app::model::payments::RecoveryPaymentsCreate::set_payment_method_sub_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "transaction_created_at",
                &app::model::payments::RecoveryPaymentsCreate::get_transaction_created_at,
                &app::model::payments::RecoveryPaymentsCreate::set_transaction_created_at,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "connector_customer_id",
                &app::model::payments::RecoveryPaymentsCreate::get_connector_customer_id,
                &app::model::payments::RecoveryPaymentsCreate::set_connector_customer_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "billing_started_at",
                &app::model::payments::RecoveryPaymentsCreate::get_billing_started_at,
                &app::model::payments::RecoveryPaymentsCreate::set_billing_started_at,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "connector_transaction_id",
                &app::model::payments::RecoveryPaymentsCreate::get_connector_transaction_id,
                &app::model::payments::RecoveryPaymentsCreate::set_connector_transaction_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_method_data",
                &app::model::payments::RecoveryPaymentsCreate::get_payment_method_data,
                &app::model::payments::RecoveryPaymentsCreate::set_payment_method_data,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "enable_partial_authorization",
                &app::model::payments::RecoveryPaymentsCreate::get_enable_partial_authorization,
                &app::model::payments::RecoveryPaymentsCreate::set_enable_partial_authorization,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "metadata",
                &app::model::payments::RecoveryPaymentsCreate::get_metadata,
                &app::model::payments::RecoveryPaymentsCreate::set_metadata,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "recovery_payments_create";
    }
};

template<>
struct serde::Serializable<app::model::payments::RecordAttemptErrorDetails>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "code",
                &app::model::payments::RecordAttemptErrorDetails::get_code,
                &app::model::payments::RecordAttemptErrorDetails::set_code,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "message",
                &app::model::payments::RecordAttemptErrorDetails::get_message,
                &app::model::payments::RecordAttemptErrorDetails::set_message,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "network_advice_code",
                &app::model::payments::RecordAttemptErrorDetails::get_network_advice_code,
                &app::model::payments::RecordAttemptErrorDetails::set_network_advice_code,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "network_decline_code",
                &app::model::payments::RecordAttemptErrorDetails::get_network_decline_code,
                &app::model::payments::RecordAttemptErrorDetails::set_network_decline_code,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "network_error_message",
                &app::model::payments::RecordAttemptErrorDetails::get_network_error_message,
                &app::model::payments::RecordAttemptErrorDetails::set_network_error_message,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "record_attempt_error_details";
    }
};

template<>
struct serde::Serializable<app::model::payments::NullObject>
{
    static constexpr auto fields()
    {
        return std::tuple{};
    }

    static constexpr std::string_view table_name()
    {
        return "null_object";
    }
};

template<>
struct serde::Serializable<app::model::payments::ExternalThreeDsData>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "authentication_cryptogram",
                &app::model::payments::ExternalThreeDsData::get_authentication_cryptogram,
                &app::model::payments::ExternalThreeDsData::set_authentication_cryptogram,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "ds_trans_id",
                &app::model::payments::ExternalThreeDsData::get_ds_trans_id,
                &app::model::payments::ExternalThreeDsData::set_ds_trans_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "version",
                &app::model::payments::ExternalThreeDsData::get_version,
                &app::model::payments::ExternalThreeDsData::set_version,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "eci",
                &app::model::payments::ExternalThreeDsData::get_eci,
                &app::model::payments::ExternalThreeDsData::set_eci,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "transaction_status",
                &app::model::payments::ExternalThreeDsData::get_transaction_status,
                &app::model::payments::ExternalThreeDsData::set_transaction_status,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "exemption_indicator",
                &app::model::payments::ExternalThreeDsData::get_exemption_indicator,
                &app::model::payments::ExternalThreeDsData::set_exemption_indicator,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "network_params",
                &app::model::payments::ExternalThreeDsData::get_network_params,
                &app::model::payments::ExternalThreeDsData::set_network_params,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "external_three_ds_data";
    }
};

template<>
struct serde::Serializable<app::model::payments::NetworkParams>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<
            "cartes_bancaires",
            &app::model::payments::NetworkParams::get_cartes_bancaires,
            &app::model::payments::NetworkParams::set_cartes_bancaires,
            serde::FieldOptions{}>{}};
    }

    static constexpr std::string_view table_name()
    {
        return "network_params";
    }
};

template<>
struct serde::Serializable<app::model::payments::CartesBancairesParams>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "cavv_algorithm",
                &app::model::payments::CartesBancairesParams::get_cavv_algorithm,
                &app::model::payments::CartesBancairesParams::set_cavv_algorithm,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "cb_exemption",
                &app::model::payments::CartesBancairesParams::get_cb_exemption,
                &app::model::payments::CartesBancairesParams::set_cb_exemption,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "cb_score",
                &app::model::payments::CartesBancairesParams::get_cb_score,
                &app::model::payments::CartesBancairesParams::set_cb_score,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "cartes_bancaires_params";
    }
};

template<>
struct serde::Serializable<app::model::payments::FlexibleAmountDetails>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "min_value",
                &app::model::payments::FlexibleAmountDetails::get_min_value,
                &app::model::payments::FlexibleAmountDetails::set_min_value,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "max_value",
                &app::model::payments::FlexibleAmountDetails::get_max_value,
                &app::model::payments::FlexibleAmountDetails::set_max_value,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "value_type",
                &app::model::payments::FlexibleAmountDetails::get_value_type,
                &app::model::payments::FlexibleAmountDetails::set_value_type,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "flexible_amount_details";
    }
};

template<>
struct serde::Serializable<app::model::payments::InstallmentDetails>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "max_partial_payments",
                &app::model::payments::InstallmentDetails::get_max_partial_payments,
                &app::model::payments::InstallmentDetails::set_max_partial_payments,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "value_type",
                &app::model::payments::InstallmentDetails::get_value_type,
                &app::model::payments::InstallmentDetails::set_value_type,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "installment_details";
    }
};


#ifdef CONGELADO_TEST
namespace app::model::payments::tests {
using namespace boost::ut;
suite<"app_model_payments"> app_model_payments_suite = []
{
    "reflects converted types"_test = []
    {
        expect(
            std::tuple_size_v<
                decltype(serde::Serializable<app::model::payments::BankData>::fields())> > 0
        );
    };
};
} // namespace app::model::payments::tests
#endif
