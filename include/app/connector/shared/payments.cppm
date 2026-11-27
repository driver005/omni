module;
export module app_connector:model_shared_payments;

import std;
import core_serde;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace app::model::payments {

enum class AccountType
{
    Current,
    Savings,
    Payment,
};

class AirwallexData : public serde::Serializer<AirwallexData>
{
public:
    void set_payload(std::optional<std::string> value)
    {
        payload = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_payload() const noexcept
    {
        return payload;
    }

    std::optional<std::string> payload{};
};

class AliPayQr : public serde::Serializer<AliPayQr>
{
public:
};

class ApiConnectorErrorDetails : public serde::Serializer<ApiConnectorErrorDetails>
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

    void set_reason(std::optional<std::string> value)
    {
        reason = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_reason() const noexcept
    {
        return reason;
    }

    std::optional<std::string> code{};
    std::optional<std::string> message{};
    std::optional<std::string> reason{};
};

enum class ApplepayInitiative
{
    Web,
    Ios,
};

enum class BalanceCheckPaymentMethodData
{
    GiftCard,
};

class BankDebitAdditionalDataValue : public serde::Serializer<BankDebitAdditionalDataValue>
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

enum class BankDebitData
{
    AchBankDebit,
    SepaBankDebit,
    SepaGuarenteedBankDebit,
    BecsBankDebit,
    BacsBankDebit,
    EftDebitOrder,
};

class BankNamesValue : public serde::Serializer<BankNamesValue>
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

enum class BankRedirectData
{
    BancontactCard,
    Bizum,
    Blik,
    Eps,
    Giropay,
    Ideal,
    Interac,
    OnlineBankingCzechRepublic,
    OnlineBankingFinland,
    OnlineBankingPoland,
    OnlineBankingSlovakia,
    OpenBankingUk,
    Przelewy24,
    Sofort,
    Trustly,
    OnlineBankingFpx,
    OnlineBankingThailand,
    LocalBankRedirect,
    Eft,
    OpenBanking,
};

class BankRedirectDetailsValue : public serde::Serializer<BankRedirectDetailsValue>
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

class BankTransferAdditionalDataValue : public serde::Serializer<BankTransferAdditionalDataValue>
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

enum class BankTransferData
{
    AchBankTransfer,
    SepaBankTransfer,
    BacsBankTransfer,
    MultibancoBankTransfer,
    PermataBankTransfer,
    BcaBankTransfer,
    BniVaBankTransfer,
    BriVaBankTransfer,
    CimbVaBankTransfer,
    DanamonVaBankTransfer,
    MandiriVaBankTransfer,
    Pix,
    PixEmv,
    PixQr,
    PixAutomaticoQr,
    PixAutomaticoPush,
    Pse,
    LocalBankTransfer,
    InstantBankTransfer,
    InstantBankTransferFinland,
    InstantBankTransferPoland,
    IndonesianBankTransfer,
};

class BillingConnectorPaymentDetails : public serde::Serializer<BillingConnectorPaymentDetails>
{
public:
    void set_payment_processor_token(std::string value)
    {
        payment_processor_token = std::move(value);
    }

    [[nodiscard]] const std::string& get_payment_processor_token() const noexcept
    {
        return payment_processor_token;
    }

    void set_connector_customer_id(std::string value)
    {
        connector_customer_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_connector_customer_id() const noexcept
    {
        return connector_customer_id;
    }

    std::string payment_processor_token{};
    std::string connector_customer_id{};
};

class BoletoDocumentKindValue : public serde::Serializer<BoletoDocumentKindValue>
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

enum class BoletoPaymentTypeConstraints
{
    FixedAmount,
    FlexibleAmount,
    Installment,
};

class BoletoPaymentTypeValue : public serde::Serializer<BoletoPaymentTypeValue>
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

class CaptureFeatureMetadataValue : public serde::Serializer<CaptureFeatureMetadataValue>
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

class CardNetworkValue : public serde::Serializer<CardNetworkValue>
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

class CardNumberValue : public serde::Serializer<CardNumberValue>
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

enum class CardRedirectData
{
    Knet,
    Benefit,
    MomoAtm,
    CardRedirect,
};

class CardSegmentTypeValue : public serde::Serializer<CardSegmentTypeValue>
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

class CardTokenAdditionalDataValue : public serde::Serializer<CardTokenAdditionalDataValue>
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

class CardTypeValue : public serde::Serializer<CardTypeValue>
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

class CashappQr : public serde::Serializer<CashappQr>
{
public:
};

class CheckoutData : public serde::Serializer<CheckoutData>
{
public:
    void set_purpose_of_payment(std::optional<std::string> value)
    {
        purpose_of_payment = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_purpose_of_payment() const noexcept
    {
        return purpose_of_payment;
    }

    std::optional<std::string> purpose_of_payment{};
};

class ConnectorChargeResponseDataValue : public serde::Serializer<ConnectorChargeResponseDataValue>
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

enum class ConnectorMetadataResponse
{
    Santander,
    Revolv3,
};

class ConnectorTokenDetails : public serde::Serializer<ConnectorTokenDetails>
{
public:
    void set_token(std::string value)
    {
        token = std::move(value);
    }

    [[nodiscard]] const std::string& get_token() const noexcept
    {
        return token;
    }

    void set_connector_token_request_reference_id(std::optional<std::string> value)
    {
        connector_token_request_reference_id = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>&
    get_connector_token_request_reference_id() const noexcept
    {
        return connector_token_request_reference_id;
    }

    std::string token{};
    std::optional<std::string> connector_token_request_reference_id{};
};

class ConnectorTransactionIdValue : public serde::Serializer<ConnectorTransactionIdValue>
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

class ConnectorValue : public serde::Serializer<ConnectorValue>
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

class CountryAlpha2Value : public serde::Serializer<CountryAlpha2Value>
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

class CurrencyValue : public serde::Serializer<CurrencyValue>
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

class CustomerIdValue : public serde::Serializer<CustomerIdValue>
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

enum class DiscountType
{
    Standard,
    FixedDate,
    DailyCalendar,
    DailyBusiness,
};

class DocumentKindValue : public serde::Serializer<DocumentKindValue>
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

class EmailValue : public serde::Serializer<EmailValue>
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

class ExternalTriggerStateValue : public serde::Serializer<ExternalTriggerStateValue>
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

class FinixAdditionalDetails : public serde::Serializer<FinixAdditionalDetails>
{
public:
    void set_fraud_session_id(std::optional<std::string> value)
    {
        fraud_session_id = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_fraud_session_id() const noexcept
    {
        return fraud_session_id;
    }

    std::optional<std::string> fraud_session_id{};
};

class FundingSourceValue : public serde::Serializer<FundingSourceValue>
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

class FutureUsageValue : public serde::Serializer<FutureUsageValue>
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

class GcashRedirection : public serde::Serializer<GcashRedirection>
{
public:
};

class GiftCardAdditionalDataValue : public serde::Serializer<GiftCardAdditionalDataValue>
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

enum class GiftCardData
{
    Givex,
    PaySafeCard,
    BhnCardNetwork,
};

class GlobalPaymentIdValue : public serde::Serializer<GlobalPaymentIdValue>
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

class GoPayRedirection : public serde::Serializer<GoPayRedirection>
{
public:
};

class HashedStringValue : public serde::Serializer<HashedStringValue>
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

class IntentStatusValue : public serde::Serializer<IntentStatusValue>
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

class InteracCustomerInfoDetailsValue : public serde::Serializer<InteracCustomerInfoDetailsValue>
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

class KakaoPayRedirection : public serde::Serializer<KakaoPayRedirection>
{
public:
};

class MerchantConnectorAccountIdValue : public serde::Serializer<MerchantConnectorAccountIdValue>
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

class MerchantConnectorAuthDetailsValue :
    public serde::Serializer<MerchantConnectorAuthDetailsValue>
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

class MerchantConnectorDetailsWrapValue :
    public serde::Serializer<MerchantConnectorDetailsWrapValue>
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

class MerchantConnectorInfoValue : public serde::Serializer<MerchantConnectorInfoValue>
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

class MerchantIdValue : public serde::Serializer<MerchantIdValue>
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

class MinorUnitValue : public serde::Serializer<MinorUnitValue>
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

class MobilePayRedirection : public serde::Serializer<MobilePayRedirection>
{
public:
};

class MomoRedirection : public serde::Serializer<MomoRedirection>
{
public:
};

enum class NextActionCall
{
    PostSessionTokens,
    Confirm,
    Sync,
    CompleteAuthorize,
    AwaitMerchantCallback,
    Deny,
    EligibilityCheck,
};

class NoonData : public serde::Serializer<NoonData>
{
public:
    void set_order_category(std::optional<std::string> value)
    {
        order_category = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_order_category() const noexcept
    {
        return order_category;
    }

    std::optional<std::string> order_category{};
};

enum class PMBalanceCheckEligibilityResponse
{
    Success,
    Failure,
};

class PaymentConnectorInvokeDDCMetadata :
    public serde::Serializer<PaymentConnectorInvokeDDCMetadata>
{
public:
    void set_iframe_url(std::string value)
    {
        iframe_url = std::move(value);
    }

    [[nodiscard]] const std::string& get_iframe_url() const noexcept
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

    std::string iframe_url{};
    std::optional<std::int32_t> timeout_ms{};
};

class PaymentConnectorTransmissionValue :
    public serde::Serializer<PaymentConnectorTransmissionValue>
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

class PaymentIdValue : public serde::Serializer<PaymentIdValue>
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

class PaymentIntentStateMetadataValue : public serde::Serializer<PaymentIntentStateMetadataValue>
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

class PaymentMethodTypeValue : public serde::Serializer<PaymentMethodTypeValue>
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

class PaymentMethodValue : public serde::Serializer<PaymentMethodValue>
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

enum class PaymentProcessingDetailInputType
{
    Raw,
    LinkHierarchicalResource,
};

enum class PaymentProcessingDetailsAt
{
    Hyperswitch,
    Connector,
};

class PaymentRequestMetadata : public serde::Serializer<PaymentRequestMetadata>
{
public:
    void set_supported_networks(std::vector<std::string> value)
    {
        supported_networks = std::move(value);
    }

    [[nodiscard]] const std::vector<std::string>& get_supported_networks() const noexcept
    {
        return supported_networks;
    }

    void set_merchant_capabilities(std::vector<std::string> value)
    {
        merchant_capabilities = std::move(value);
    }

    [[nodiscard]] const std::vector<std::string>& get_merchant_capabilities() const noexcept
    {
        return merchant_capabilities;
    }

    void set_label(std::string value)
    {
        label = std::move(value);
    }

    [[nodiscard]] const std::string& get_label() const noexcept
    {
        return label;
    }

    std::vector<std::string> supported_networks{};
    std::vector<std::string> merchant_capabilities{};
    std::string label{};
};

class PaymentsConnectorThreeDsInvokeData :
    public serde::Serializer<PaymentsConnectorThreeDsInvokeData>
{
public:
    void set_directory_server_id(std::string value)
    {
        directory_server_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_directory_server_id() const noexcept
    {
        return directory_server_id;
    }

    void set_three_ds_method_url(std::string value)
    {
        three_ds_method_url = std::move(value);
    }

    [[nodiscard]] const std::string& get_three_ds_method_url() const noexcept
    {
        return three_ds_method_url;
    }

    void set_three_ds_method_data(std::string value)
    {
        three_ds_method_data = std::move(value);
    }

    [[nodiscard]] const std::string& get_three_ds_method_data() const noexcept
    {
        return three_ds_method_data;
    }

    void set_message_version(std::optional<std::string> value)
    {
        message_version = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_message_version() const noexcept
    {
        return message_version;
    }

    void set_three_ds_method_data_submission(bool value)
    {
        three_ds_method_data_submission = std::move(value);
    }

    [[nodiscard]] const bool& get_three_ds_method_data_submission() const noexcept
    {
        return three_ds_method_data_submission;
    }

    std::string directory_server_id{};
    std::string three_ds_method_url{};
    std::string three_ds_method_data{};
    std::optional<std::string> message_version{};
    bool three_ds_method_data_submission{};
};

class PaymentsRedirectionResponse : public serde::Serializer<PaymentsRedirectionResponse>
{
public:
    void set_redirect_url(std::string value)
    {
        redirect_url = std::move(value);
    }

    [[nodiscard]] const std::string& get_redirect_url() const noexcept
    {
        return redirect_url;
    }

    std::string redirect_url{};
};

enum class PeachpaymentsCardOnFileTransactionType
{
    OneOff,
    CustomerInitiatedTransaction,
    MerchantInitiatedMandate,
    MerchantInitiatedTransaction,
};

class PhoneDetailsValue : public serde::Serializer<PhoneDetailsValue>
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

enum class PixAdditionalDetails
{
    Immediate,
    Scheduled,
};

enum class PixAutomaticoAdditionalDetails
{
    PixAutomaticoPush,
    PixAutomaticoQr,
    PixAutomaticoMit,
};

class PixKeyValue : public serde::Serializer<PixKeyValue>
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

class PresenceOfCustomerDuringPaymentValue :
    public serde::Serializer<PresenceOfCustomerDuringPaymentValue>
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

class PrimitiveDateTimeValue : public serde::Serializer<PrimitiveDateTimeValue>
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

class ProfileIdValue : public serde::Serializer<ProfileIdValue>
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

enum class ProtestType
{
    Disabled,
    CalendarDays,
    BusinessDays,
    ContractDefault,
};

enum class QrCodeInformation
{
    QrCodeUrl,
    QrDataUrl,
    QrCodeImageUrl,
    QrColorDataUrl,
};

class ReceiverDetails : public serde::Serializer<ReceiverDetails>
{
public:
    void set_amount_received(std::int64_t value)
    {
        amount_received = std::move(value);
    }

    [[nodiscard]] const std::int64_t& get_amount_received() const noexcept
    {
        return amount_received;
    }

    void set_amount_charged(std::optional<std::int64_t> value)
    {
        amount_charged = std::move(value);
    }

    [[nodiscard]] const std::optional<std::int64_t>& get_amount_charged() const noexcept
    {
        return amount_charged;
    }

    void set_amount_remaining(std::optional<std::int64_t> value)
    {
        amount_remaining = std::move(value);
    }

    [[nodiscard]] const std::optional<std::int64_t>& get_amount_remaining() const noexcept
    {
        return amount_remaining;
    }

    std::int64_t amount_received{};
    std::optional<std::int64_t> amount_charged{};
    std::optional<std::int64_t> amount_remaining{};
};

enum class RecurringPaymentIntervalUnit
{
    Year,
    Month,
    Day,
    Hour,
    Minute,
};

class RevenueRecoveryABAlgorithmValue : public serde::Serializer<RevenueRecoveryABAlgorithmValue>
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

class SecretSerdeValueValue : public serde::Serializer<SecretSerdeValueValue>
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

class SecretValue : public serde::Serializer<SecretValue>
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

class StandardisedCodeValue : public serde::Serializer<StandardisedCodeValue>
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

class StringMajorUnitValue : public serde::Serializer<StringMajorUnitValue>
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

class StripeConnectorMetadata : public serde::Serializer<StripeConnectorMetadata>
{
public:
    void set_error_on_requires_action(std::optional<bool> value)
    {
        error_on_requires_action = std::move(value);
    }

    [[nodiscard]] const std::optional<bool>& get_error_on_requires_action() const noexcept
    {
        return error_on_requires_action;
    }

    std::optional<bool> error_on_requires_action{};
};

class SurchargeDetailsResponseValue : public serde::Serializer<SurchargeDetailsResponseValue>
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

class TouchNGoRedirection : public serde::Serializer<TouchNGoRedirection>
{
public:
};

class TriggeredByValue : public serde::Serializer<TriggeredByValue>
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

class UrlValue : public serde::Serializer<UrlValue>
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

class WeChatPayQr : public serde::Serializer<WeChatPayQr>
{
public:
};

class WeChatPayRedirection : public serde::Serializer<WeChatPayRedirection>
{
public:
};

enum class WorldpayxmlFundingTransactionType
{
    CreditCardBillRepayment,
    GiftCardPurchase,
    GiftCardPurchaseForAnother,
    NonReloadablePrepaidCard,
    ReloadablePrepaidCardOrAccount,
    GamingChipsPurchase,
    GamingStoredValueWallet,
    GamingStagedDigitalWallet,
    LiquidAndCryptoAssetsPurchase,
    LiquidAndCryptoStoredValueWalletLoad,
    StoredValueDigitalWalletLoad,
    StoredValueDigitalWalletLoadNonSecurities,
    SecuritiesStoredValueDigitalWalletLoad,
    SecuritiesStagedDigitalWalletLoad,
    SingleMerchantWalletLoad,
    DebitCardLoad,
    TransferToOwnDebitAccount,
    FundsTransferMeToMe,
    AccountToAccount,
    BackToBackP2pWithoutWallet,
    BackToBackP2pWithWallet,
    AgentCashOut,
    StagedDigitalWalletLoad,
    StagedDigitalWalletPurchase,
    BackToBackCardPurchase,
    PayrollDisbursementFunding,
    BusinessToConsumerDisbursement,
    BusinessToBusinessInvoicePayment,
};

enum class WorldpayxmlPaymentPurpose
{
    FamilySupport,
    RegularLabourTransfers,
    TravelAndTourism,
    Education,
    HospitalisationAndMedicalTreatment,
    EmergencyNeed,
    Savings,
    Gifts,
    Other,
    Salary,
    CrowdLending,
    CryptoCurrency,
    HighRiskSecurities,
};

class AdditionalCardInfo : public serde::Serializer<AdditionalCardInfo>
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

    void set_last4(std::optional<std::string> value)
    {
        last4 = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_last4() const noexcept
    {
        return last4;
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

    void set_is_regulated(std::optional<bool> value)
    {
        is_regulated = std::move(value);
    }

    [[nodiscard]] const std::optional<bool>& get_is_regulated() const noexcept
    {
        return is_regulated;
    }

    void set_signature_network(std::optional<CardNetworkValue> value)
    {
        signature_network = std::move(value);
    }

    [[nodiscard]] const std::optional<CardNetworkValue>& get_signature_network() const noexcept
    {
        return signature_network;
    }

    void set_auth_code(std::optional<std::string> value)
    {
        auth_code = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_auth_code() const noexcept
    {
        return auth_code;
    }

    std::optional<std::string> card_issuer{};
    std::optional<CardNetworkValue> card_network{};
    std::optional<std::string> card_type{};
    std::optional<std::string> card_subtype{};
    std::optional<CardSegmentTypeValue> card_segment_type{};
    std::optional<FundingSourceValue> funding_source{};
    std::optional<std::string> card_issuing_country{};
    std::optional<std::string> card_issuing_country_code{};
    std::optional<std::string> bank_code{};
    std::optional<std::string> last4{};
    std::optional<std::string> card_isin{};
    std::optional<std::string> card_extended_bin{};
    std::optional<SecretValue> card_exp_month{};
    std::optional<SecretValue> card_exp_year{};
    std::optional<SecretValue> card_holder_name{};
    std::optional<serde::Value> payment_checks{};
    std::optional<serde::Value> authentication_data{};
    std::optional<bool> is_regulated{};
    std::optional<CardNetworkValue> signature_network{};
    std::optional<std::string> auth_code{};
};

class AddressDetails : public serde::Serializer<AddressDetails>
{
public:
    void set_city(std::optional<std::string> value)
    {
        city = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_city() const noexcept
    {
        return city;
    }

    void set_country(std::optional<CountryAlpha2Value> value)
    {
        country = std::move(value);
    }

    [[nodiscard]] const std::optional<CountryAlpha2Value>& get_country() const noexcept
    {
        return country;
    }

    void set_line1(std::optional<SecretValue> value)
    {
        line1 = std::move(value);
    }

    [[nodiscard]] const std::optional<SecretValue>& get_line1() const noexcept
    {
        return line1;
    }

    void set_line2(std::optional<SecretValue> value)
    {
        line2 = std::move(value);
    }

    [[nodiscard]] const std::optional<SecretValue>& get_line2() const noexcept
    {
        return line2;
    }

    void set_line3(std::optional<SecretValue> value)
    {
        line3 = std::move(value);
    }

    [[nodiscard]] const std::optional<SecretValue>& get_line3() const noexcept
    {
        return line3;
    }

    void set_zip(std::optional<SecretValue> value)
    {
        zip = std::move(value);
    }

    [[nodiscard]] const std::optional<SecretValue>& get_zip() const noexcept
    {
        return zip;
    }

    void set_state(std::optional<SecretValue> value)
    {
        state = std::move(value);
    }

    [[nodiscard]] const std::optional<SecretValue>& get_state() const noexcept
    {
        return state;
    }

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

    void set_origin_zip(std::optional<SecretValue> value)
    {
        origin_zip = std::move(value);
    }

    [[nodiscard]] const std::optional<SecretValue>& get_origin_zip() const noexcept
    {
        return origin_zip;
    }

    std::optional<std::string> city{};
    std::optional<CountryAlpha2Value> country{};
    std::optional<SecretValue> line1{};
    std::optional<SecretValue> line2{};
    std::optional<SecretValue> line3{};
    std::optional<SecretValue> zip{};
    std::optional<SecretValue> state{};
    std::optional<SecretValue> first_name{};
    std::optional<SecretValue> last_name{};
    std::optional<SecretValue> origin_zip{};
};

class AdyenTestingData : public serde::Serializer<AdyenTestingData>
{
public:
    void set_holder_name(std::optional<SecretValue> value)
    {
        holder_name = std::move(value);
    }

    [[nodiscard]] const std::optional<SecretValue>& get_holder_name() const noexcept
    {
        return holder_name;
    }

    std::optional<SecretValue> holder_name{};
};

class AmountInfo : public serde::Serializer<AmountInfo>
{
public:
    void set_label(std::string value)
    {
        label = std::move(value);
    }

    [[nodiscard]] const std::string& get_label() const noexcept
    {
        return label;
    }

    void set_total_type(std::optional<std::string> value)
    {
        total_type = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_total_type() const noexcept
    {
        return total_type;
    }

    void set_amount(StringMajorUnitValue value)
    {
        amount = std::move(value);
    }

    [[nodiscard]] const StringMajorUnitValue& get_amount() const noexcept
    {
        return amount;
    }

    std::string label{};
    std::optional<std::string> total_type{};
    StringMajorUnitValue amount{};
};

class ApplePayRegularBillingDetails : public serde::Serializer<ApplePayRegularBillingDetails>
{
public:
    void set_label(std::string value)
    {
        label = std::move(value);
    }

    [[nodiscard]] const std::string& get_label() const noexcept
    {
        return label;
    }

    void set_recurring_payment_start_date(std::optional<PrimitiveDateTimeValue> value)
    {
        recurring_payment_start_date = std::move(value);
    }

    [[nodiscard]] const std::optional<PrimitiveDateTimeValue>&
    get_recurring_payment_start_date() const noexcept
    {
        return recurring_payment_start_date;
    }

    void set_recurring_payment_end_date(std::optional<PrimitiveDateTimeValue> value)
    {
        recurring_payment_end_date = std::move(value);
    }

    [[nodiscard]] const std::optional<PrimitiveDateTimeValue>&
    get_recurring_payment_end_date() const noexcept
    {
        return recurring_payment_end_date;
    }

    void set_recurring_payment_interval_unit(std::optional<RecurringPaymentIntervalUnit> value)
    {
        recurring_payment_interval_unit = std::move(value);
    }

    [[nodiscard]] const std::optional<RecurringPaymentIntervalUnit>&
    get_recurring_payment_interval_unit() const noexcept
    {
        return recurring_payment_interval_unit;
    }

    void set_recurring_payment_interval_count(std::optional<std::int32_t> value)
    {
        recurring_payment_interval_count = std::move(value);
    }

    [[nodiscard]] const std::optional<std::int32_t>&
    get_recurring_payment_interval_count() const noexcept
    {
        return recurring_payment_interval_count;
    }

    std::string label{};
    std::optional<PrimitiveDateTimeValue> recurring_payment_start_date{};
    std::optional<PrimitiveDateTimeValue> recurring_payment_end_date{};
    std::optional<RecurringPaymentIntervalUnit> recurring_payment_interval_unit{};
    std::optional<std::int32_t> recurring_payment_interval_count{};
};

class ApplyPaymentMethodDataRequest : public serde::Serializer<ApplyPaymentMethodDataRequest>
{
public:
    void set_payment_methods(std::vector<BalanceCheckPaymentMethodData> value)
    {
        payment_methods = std::move(value);
    }

    [[nodiscard]] const std::vector<BalanceCheckPaymentMethodData>&
    get_payment_methods() const noexcept
    {
        return payment_methods;
    }

    std::vector<BalanceCheckPaymentMethodData> payment_methods{};
};

class ApplyPaymentMethodDataSurchargeResponseItem :
    public serde::Serializer<ApplyPaymentMethodDataSurchargeResponseItem>
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
    std::optional<SurchargeDetailsResponseValue> surcharge_details{};
};

class BHNGiftCardDetails : public serde::Serializer<BHNGiftCardDetails>
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

    void set_pin(std::optional<SecretValue> value)
    {
        pin = std::move(value);
    }

    [[nodiscard]] const std::optional<SecretValue>& get_pin() const noexcept
    {
        return pin;
    }

    void set_cvv2(std::optional<SecretValue> value)
    {
        cvv2 = std::move(value);
    }

    [[nodiscard]] const std::optional<SecretValue>& get_cvv2() const noexcept
    {
        return cvv2;
    }

    void set_expiration_date(std::optional<std::string> value)
    {
        expiration_date = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_expiration_date() const noexcept
    {
        return expiration_date;
    }

    SecretValue account_number{};
    std::optional<SecretValue> pin{};
    std::optional<SecretValue> cvv2{};
    std::optional<std::string> expiration_date{};
};

class BankCodeResponse : public serde::Serializer<BankCodeResponse>
{
public:
    void set_bank_name(std::vector<BankNamesValue> value)
    {
        bank_name = std::move(value);
    }

    [[nodiscard]] const std::vector<BankNamesValue>& get_bank_name() const noexcept
    {
        return bank_name;
    }

    void set_eligible_connectors(std::vector<std::string> value)
    {
        eligible_connectors = std::move(value);
    }

    [[nodiscard]] const std::vector<std::string>& get_eligible_connectors() const noexcept
    {
        return eligible_connectors;
    }

    std::vector<BankNamesValue> bank_name{};
    std::vector<std::string> eligible_connectors{};
};

class BankDebitResponse : public serde::Serializer<BankDebitResponse>
{
public:
    void set_details(std::optional<BankDebitAdditionalDataValue> value)
    {
        details = std::move(value);
    }

    [[nodiscard]] const std::optional<BankDebitAdditionalDataValue>& get_details() const noexcept
    {
        return details;
    }

    std::optional<BankDebitAdditionalDataValue> details{};
};

class BankRedirectBilling : public serde::Serializer<BankRedirectBilling>
{
public:
    void set_billing_name(std::optional<SecretValue> value)
    {
        billing_name = std::move(value);
    }

    [[nodiscard]] const std::optional<SecretValue>& get_billing_name() const noexcept
    {
        return billing_name;
    }

    void set_email(std::optional<EmailValue> value)
    {
        email = std::move(value);
    }

    [[nodiscard]] const std::optional<EmailValue>& get_email() const noexcept
    {
        return email;
    }

    std::optional<SecretValue> billing_name{};
    std::optional<EmailValue> email{};
};

class BankTransferResponse : public serde::Serializer<BankTransferResponse>
{
public:
    void set_details(std::optional<BankTransferAdditionalDataValue> value)
    {
        details = std::move(value);
    }

    [[nodiscard]] const std::optional<BankTransferAdditionalDataValue>& get_details() const noexcept
    {
        return details;
    }

    std::optional<BankTransferAdditionalDataValue> details{};
};

class BeneficiaryDetails : public serde::Serializer<BeneficiaryDetails>
{
public:
    void set_name(std::optional<std::string> value)
    {
        name = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_name() const noexcept
    {
        return name;
    }

    void set_document_number(std::optional<std::string> value)
    {
        document_number = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_document_number() const noexcept
    {
        return document_number;
    }

    void set_document_type(std::optional<DocumentKindValue> value)
    {
        document_type = std::move(value);
    }

    [[nodiscard]] const std::optional<DocumentKindValue>& get_document_type() const noexcept
    {
        return document_type;
    }

    std::optional<std::string> name{};
    std::optional<std::string> document_number{};
    std::optional<DocumentKindValue> document_type{};
};

class BillingConnectorAdditionalCardInfo :
    public serde::Serializer<BillingConnectorAdditionalCardInfo>
{
public:
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

    void set_card_isin(std::optional<std::string> value)
    {
        card_isin = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_card_isin() const noexcept
    {
        return card_isin;
    }

    std::optional<CardNetworkValue> card_network{};
    std::optional<std::string> card_issuer{};
    std::optional<std::string> card_type{};
    std::optional<std::string> card_issuing_country{};
    std::optional<std::string> card_isin{};
};

class BraintreeData : public serde::Serializer<BraintreeData>
{
public:
    void set_merchant_account_id(std::optional<SecretValue> value)
    {
        merchant_account_id = std::move(value);
    }

    [[nodiscard]] const std::optional<SecretValue>& get_merchant_account_id() const noexcept
    {
        return merchant_account_id;
    }

    void set_merchant_config_currency(std::optional<CurrencyValue> value)
    {
        merchant_config_currency = std::move(value);
    }

    [[nodiscard]] const std::optional<CurrencyValue>& get_merchant_config_currency() const noexcept
    {
        return merchant_config_currency;
    }

    std::optional<SecretValue> merchant_account_id{};
    std::optional<CurrencyValue> merchant_config_currency{};
};

class Card : public serde::Serializer<Card>
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
    SecretValue card_exp_month{};
    SecretValue card_exp_year{};
    std::optional<SecretValue> card_holder_name{};
    SecretValue card_cvc{};
    std::optional<std::string> card_issuer{};
    std::optional<CardNetworkValue> card_network{};
    std::optional<std::string> card_type{};
    std::optional<std::string> card_subtype{};
    std::optional<CardSegmentTypeValue> card_segment_type{};
    std::optional<FundingSourceValue> funding_source{};
    std::optional<std::string> card_issuing_country{};
    std::optional<std::string> card_issuing_country_code{};
    std::optional<std::string> bank_code{};
    std::optional<SecretValue> nick_name{};
};

class CardRedirectResponse : public serde::Serializer<CardRedirectResponse>
{
public:
    void set_details(std::optional<CardRedirectData> value)
    {
        details = std::move(value);
    }

    [[nodiscard]] const std::optional<CardRedirectData>& get_details() const noexcept
    {
        return details;
    }

    std::optional<CardRedirectData> details{};
};

class CardToken : public serde::Serializer<CardToken>
{
public:
    void set_card_holder_name(std::optional<SecretValue> value)
    {
        card_holder_name = std::move(value);
    }

    [[nodiscard]] const std::optional<SecretValue>& get_card_holder_name() const noexcept
    {
        return card_holder_name;
    }

    void set_card_cvc(std::optional<SecretValue> value)
    {
        card_cvc = std::move(value);
    }

    [[nodiscard]] const std::optional<SecretValue>& get_card_cvc() const noexcept
    {
        return card_cvc;
    }

    void set_card_cvc_token(std::optional<SecretValue> value)
    {
        card_cvc_token = std::move(value);
    }

    [[nodiscard]] const std::optional<SecretValue>& get_card_cvc_token() const noexcept
    {
        return card_cvc_token;
    }

    std::optional<SecretValue> card_holder_name{};
    std::optional<SecretValue> card_cvc{};
    std::optional<SecretValue> card_cvc_token{};
};

class CardTokenResponse : public serde::Serializer<CardTokenResponse>
{
public:
    void set_details(std::optional<CardTokenAdditionalDataValue> value)
    {
        details = std::move(value);
    }

    [[nodiscard]] const std::optional<CardTokenAdditionalDataValue>& get_details() const noexcept
    {
        return details;
    }

    std::optional<CardTokenAdditionalDataValue> details{};
};

class CardWithNoCVC : public serde::Serializer<CardWithNoCVC>
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
    SecretValue card_exp_month{};
    SecretValue card_exp_year{};
    std::optional<SecretValue> card_holder_name{};
    std::optional<std::string> card_issuer{};
    std::optional<CardNetworkValue> card_network{};
    std::optional<std::string> card_type{};
    std::optional<std::string> card_subtype{};
    std::optional<CardSegmentTypeValue> card_segment_type{};
    std::optional<FundingSourceValue> funding_source{};
    std::optional<std::string> card_issuing_country{};
    std::optional<std::string> card_issuing_country_code{};
    std::optional<std::string> bank_code{};
    std::optional<SecretValue> nick_name{};
};

class ConnectorCode : public serde::Serializer<ConnectorCode>
{
public:
    void set_connector(ConnectorValue value)
    {
        connector = std::move(value);
    }

    [[nodiscard]] const ConnectorValue& get_connector() const noexcept
    {
        return connector;
    }

    void set_code(std::string value)
    {
        code = std::move(value);
    }

    [[nodiscard]] const std::string& get_code() const noexcept
    {
        return code;
    }

    ConnectorValue connector{};
    std::string code{};
};

class DiscountTier : public serde::Serializer<DiscountTier>
{
public:
    void set_amount(std::optional<StringMajorUnitValue> value)
    {
        amount = std::move(value);
    }

    [[nodiscard]] const std::optional<StringMajorUnitValue>& get_amount() const noexcept
    {
        return amount;
    }

    void set_end_date(std::optional<PrimitiveDateTimeValue> value)
    {
        end_date = std::move(value);
    }

    [[nodiscard]] const std::optional<PrimitiveDateTimeValue>& get_end_date() const noexcept
    {
        return end_date;
    }

    std::optional<StringMajorUnitValue> amount{};
    std::optional<PrimitiveDateTimeValue> end_date{};
};

class DokuBankTransferInstructions : public serde::Serializer<DokuBankTransferInstructions>
{
public:
    void set_expires_at(std::optional<std::int64_t> value)
    {
        expires_at = std::move(value);
    }

    [[nodiscard]] const std::optional<std::int64_t>& get_expires_at() const noexcept
    {
        return expires_at;
    }

    void set_reference(SecretValue value)
    {
        reference = std::move(value);
    }

    [[nodiscard]] const SecretValue& get_reference() const noexcept
    {
        return reference;
    }

    void set_instructions_url(std::optional<UrlValue> value)
    {
        instructions_url = std::move(value);
    }

    [[nodiscard]] const std::optional<UrlValue>& get_instructions_url() const noexcept
    {
        return instructions_url;
    }

    std::optional<std::int64_t> expires_at{};
    SecretValue reference{};
    std::optional<UrlValue> instructions_url{};
};

class EligibilityBalanceCheckResponseItem :
    public serde::Serializer<EligibilityBalanceCheckResponseItem>
{
public:
    void set_payment_method_data(BalanceCheckPaymentMethodData value)
    {
        payment_method_data = std::move(value);
    }

    [[nodiscard]] const BalanceCheckPaymentMethodData& get_payment_method_data() const noexcept
    {
        return payment_method_data;
    }

    void set_eligibility(PMBalanceCheckEligibilityResponse value)
    {
        eligibility = std::move(value);
    }

    [[nodiscard]] const PMBalanceCheckEligibilityResponse& get_eligibility() const noexcept
    {
        return eligibility;
    }

    BalanceCheckPaymentMethodData payment_method_data{};
    PMBalanceCheckEligibilityResponse eligibility{};
};

class FetchQrCodeInformation : public serde::Serializer<FetchQrCodeInformation>
{
public:
    void set_qr_code_fetch_url(UrlValue value)
    {
        qr_code_fetch_url = std::move(value);
    }

    [[nodiscard]] const UrlValue& get_qr_code_fetch_url() const noexcept
    {
        return qr_code_fetch_url;
    }

    UrlValue qr_code_fetch_url{};
};

class GiftCardDetails : public serde::Serializer<GiftCardDetails>
{
public:
    void set_number(SecretValue value)
    {
        number = std::move(value);
    }

    [[nodiscard]] const SecretValue& get_number() const noexcept
    {
        return number;
    }

    void set_cvc(SecretValue value)
    {
        cvc = std::move(value);
    }

    [[nodiscard]] const SecretValue& get_cvc() const noexcept
    {
        return cvc;
    }

    SecretValue number{};
    SecretValue cvc{};
};

class GiftCardResponse : public serde::Serializer<GiftCardResponse>
{
public:
    void set_details(std::optional<GiftCardAdditionalDataValue> value)
    {
        details = std::move(value);
    }

    [[nodiscard]] const std::optional<GiftCardAdditionalDataValue>& get_details() const noexcept
    {
        return details;
    }

    std::optional<GiftCardAdditionalDataValue> details{};
};

class InteracPaymentMethod : public serde::Serializer<InteracPaymentMethod>
{
public:
    void set_customer_info(std::optional<InteracCustomerInfoDetailsValue> value)
    {
        customer_info = std::move(value);
    }

    [[nodiscard]] const std::optional<InteracCustomerInfoDetailsValue>&
    get_customer_info() const noexcept
    {
        return customer_info;
    }

    std::optional<InteracCustomerInfoDetailsValue> customer_info{};
};

class InterestDetail : public serde::Serializer<InterestDetail>
{
public:
    void set_interest_percentage(std::optional<StringMajorUnitValue> value)
    {
        interest_percentage = std::move(value);
    }

    [[nodiscard]] const std::optional<StringMajorUnitValue>&
    get_interest_percentage() const noexcept
    {
        return interest_percentage;
    }

    void set_iof_percentage(std::optional<StringMajorUnitValue> value)
    {
        iof_percentage = std::move(value);
    }

    [[nodiscard]] const std::optional<StringMajorUnitValue>& get_iof_percentage() const noexcept
    {
        return iof_percentage;
    }

    std::optional<StringMajorUnitValue> interest_percentage{};
    std::optional<StringMajorUnitValue> iof_percentage{};
};

class MbWayRedirection : public serde::Serializer<MbWayRedirection>
{
public:
    void set_telephone_number(std::optional<SecretValue> value)
    {
        telephone_number = std::move(value);
    }

    [[nodiscard]] const std::optional<SecretValue>& get_telephone_number() const noexcept
    {
        return telephone_number;
    }

    std::optional<SecretValue> telephone_number{};
};

class MultiCurrencyProcessingData : public serde::Serializer<MultiCurrencyProcessingData>
{
public:
    void set_currency(CurrencyValue value)
    {
        currency = std::move(value);
    }

    [[nodiscard]] const CurrencyValue& get_currency() const noexcept
    {
        return currency;
    }

    void set_amount(MinorUnitValue value)
    {
        amount = std::move(value);
    }

    [[nodiscard]] const MinorUnitValue& get_amount() const noexcept
    {
        return amount;
    }

    void set_conversion_rate(std::optional<double> value)
    {
        conversion_rate = std::move(value);
    }

    [[nodiscard]] const std::optional<double>& get_conversion_rate() const noexcept
    {
        return conversion_rate;
    }

    void set_date(std::optional<PrimitiveDateTimeValue> value)
    {
        date = std::move(value);
    }

    [[nodiscard]] const std::optional<PrimitiveDateTimeValue>& get_date() const noexcept
    {
        return date;
    }

    void set_processing_reference_id(std::optional<std::string> value)
    {
        processing_reference_id = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_processing_reference_id() const noexcept
    {
        return processing_reference_id;
    }

    void set_user_id(std::optional<std::string> value)
    {
        user_id = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_user_id() const noexcept
    {
        return user_id;
    }

    void set_conversion_provider(std::optional<std::string> value)
    {
        conversion_provider = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_conversion_provider() const noexcept
    {
        return conversion_provider;
    }

    void set_conversion_reason_code(std::optional<std::string> value)
    {
        conversion_reason_code = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_conversion_reason_code() const noexcept
    {
        return conversion_reason_code;
    }

    CurrencyValue currency{};
    MinorUnitValue amount{};
    std::optional<double> conversion_rate{};
    std::optional<PrimitiveDateTimeValue> date{};
    std::optional<std::string> processing_reference_id{};
    std::optional<std::string> user_id{};
    std::optional<std::string> conversion_provider{};
    std::optional<std::string> conversion_reason_code{};
};

class PaymentAttemptRevenueRecoveryData :
    public serde::Serializer<PaymentAttemptRevenueRecoveryData>
{
public:
    void set_attempt_triggered_by(TriggeredByValue value)
    {
        attempt_triggered_by = std::move(value);
    }

    [[nodiscard]] const TriggeredByValue& get_attempt_triggered_by() const noexcept
    {
        return attempt_triggered_by;
    }

    void set_charge_id(std::optional<std::string> value)
    {
        charge_id = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_charge_id() const noexcept
    {
        return charge_id;
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

    void set_recovery_request_id(std::optional<std::string> value)
    {
        recovery_request_id = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_recovery_request_id() const noexcept
    {
        return recovery_request_id;
    }

    TriggeredByValue attempt_triggered_by{};
    std::optional<std::string> charge_id{};
    std::optional<PresenceOfCustomerDuringPaymentValue> customer_present{};
    std::optional<FutureUsageValue> setup_future_usage{};
    std::optional<std::string> recovery_request_id{};
};

class PaymentConnectorInvokeDDCData : public serde::Serializer<PaymentConnectorInvokeDDCData>
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

class PaymentStartRedirectionParams : public serde::Serializer<PaymentStartRedirectionParams>
{
public:
    void set_publishable_key(std::string value)
    {
        publishable_key = std::move(value);
    }

    [[nodiscard]] const std::string& get_publishable_key() const noexcept
    {
        return publishable_key;
    }

    void set_profile_id(ProfileIdValue value)
    {
        profile_id = std::move(value);
    }

    [[nodiscard]] const ProfileIdValue& get_profile_id() const noexcept
    {
        return profile_id;
    }

    std::string publishable_key{};
    ProfileIdValue profile_id{};
};

class PaymentStartRedirectionRequest : public serde::Serializer<PaymentStartRedirectionRequest>
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

class PaymentsRedirectRequest : public serde::Serializer<PaymentsRedirectRequest>
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

    void set_connector(std::string value)
    {
        connector = std::move(value);
    }

    [[nodiscard]] const std::string& get_connector() const noexcept
    {
        return connector;
    }

    void set_param(std::string value)
    {
        param = std::move(value);
    }

    [[nodiscard]] const std::string& get_param() const noexcept
    {
        return param;
    }

    PaymentIdValue payment_id{};
    MerchantIdValue merchant_id{};
    std::string connector{};
    std::string param{};
};

class PaypalConnectorMetadata : public serde::Serializer<PaypalConnectorMetadata>
{
public:
    void set_paypal_client_metadata_id(std::optional<SecretValue> value)
    {
        paypal_client_metadata_id = std::move(value);
    }

    [[nodiscard]] const std::optional<SecretValue>& get_paypal_client_metadata_id() const noexcept
    {
        return paypal_client_metadata_id;
    }

    std::optional<SecretValue> paypal_client_metadata_id{};
};

class PeachpaymentsData : public serde::Serializer<PeachpaymentsData>
{
public:
    void set_rrn(std::optional<std::string> value)
    {
        rrn = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_rrn() const noexcept
    {
        return rrn;
    }

    void set_card_on_file_transaction_type(
        std::optional<PeachpaymentsCardOnFileTransactionType> value
    )
    {
        card_on_file_transaction_type = std::move(value);
    }

    [[nodiscard]] const std::optional<PeachpaymentsCardOnFileTransactionType>&
    get_card_on_file_transaction_type() const noexcept
    {
        return card_on_file_transaction_type;
    }

    std::optional<std::string> rrn{};
    std::optional<PeachpaymentsCardOnFileTransactionType> card_on_file_transaction_type{};
};

class PenaltyDetail : public serde::Serializer<PenaltyDetail>
{
public:
    void set_value(std::optional<StringMajorUnitValue> value)
    {
        this->value = std::move(value);
    }

    [[nodiscard]] const std::optional<StringMajorUnitValue>& get_value() const noexcept
    {
        return value;
    }

    void set_grace_period_days(std::optional<std::uint32_t> value)
    {
        grace_period_days = std::move(value);
    }

    [[nodiscard]] const std::optional<std::uint32_t>& get_grace_period_days() const noexcept
    {
        return grace_period_days;
    }

    std::optional<StringMajorUnitValue> value{};
    std::optional<std::uint32_t> grace_period_days{};
};

class PgRedirectResponse : public serde::Serializer<PgRedirectResponse>
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

    void set_status(IntentStatusValue value)
    {
        status = std::move(value);
    }

    [[nodiscard]] const IntentStatusValue& get_status() const noexcept
    {
        return status;
    }

    void set_gateway_id(std::string value)
    {
        gateway_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_gateway_id() const noexcept
    {
        return gateway_id;
    }

    void set_customer_id(std::optional<CustomerIdValue> value)
    {
        customer_id = std::move(value);
    }

    [[nodiscard]] const std::optional<CustomerIdValue>& get_customer_id() const noexcept
    {
        return customer_id;
    }

    void set_amount(std::optional<MinorUnitValue> value)
    {
        amount = std::move(value);
    }

    [[nodiscard]] const std::optional<MinorUnitValue>& get_amount() const noexcept
    {
        return amount;
    }

    PaymentIdValue payment_id{};
    IntentStatusValue status{};
    std::string gateway_id{};
    std::optional<CustomerIdValue> customer_id{};
    std::optional<MinorUnitValue> amount{};
};

class ProtestRules : public serde::Serializer<ProtestRules>
{
public:
    void set_protest_type(std::optional<ProtestType> value)
    {
        protest_type = std::move(value);
    }

    [[nodiscard]] const std::optional<ProtestType>& get_protest_type() const noexcept
    {
        return protest_type;
    }

    void set_days_after_due_date(std::optional<std::uint32_t> value)
    {
        days_after_due_date = std::move(value);
    }

    [[nodiscard]] const std::optional<std::uint32_t>& get_days_after_due_date() const noexcept
    {
        return days_after_due_date;
    }

    std::optional<ProtestType> protest_type{};
    std::optional<std::uint32_t> days_after_due_date{};
};

class QrCodeNextStepsInstruction : public serde::Serializer<QrCodeNextStepsInstruction>
{
public:
    void set_image_data_url(UrlValue value)
    {
        image_data_url = std::move(value);
    }

    [[nodiscard]] const UrlValue& get_image_data_url() const noexcept
    {
        return image_data_url;
    }

    void set_display_to_timestamp(std::optional<std::int64_t> value)
    {
        display_to_timestamp = std::move(value);
    }

    [[nodiscard]] const std::optional<std::int64_t>& get_display_to_timestamp() const noexcept
    {
        return display_to_timestamp;
    }

    void set_qr_code_url(std::optional<UrlValue> value)
    {
        qr_code_url = std::move(value);
    }

    [[nodiscard]] const std::optional<UrlValue>& get_qr_code_url() const noexcept
    {
        return qr_code_url;
    }

    UrlValue image_data_url{};
    std::optional<std::int64_t> display_to_timestamp{};
    std::optional<UrlValue> qr_code_url{};
};

class RedirectResponse : public serde::Serializer<RedirectResponse>
{
public:
    void set_param(std::optional<SecretValue> value)
    {
        param = std::move(value);
    }

    [[nodiscard]] const std::optional<SecretValue>& get_param() const noexcept
    {
        return param;
    }

    void set_json_payload(std::optional<SecretSerdeValueValue> value)
    {
        json_payload = std::move(value);
    }

    [[nodiscard]] const std::optional<SecretSerdeValueValue>& get_json_payload() const noexcept
    {
        return json_payload;
    }

    std::optional<SecretValue> param{};
    std::optional<SecretSerdeValueValue> json_payload{};
};

class RedirectionResponse : public serde::Serializer<RedirectionResponse>
{
public:
    void set_return_url(std::string value)
    {
        return_url = std::move(value);
    }

    [[nodiscard]] const std::string& get_return_url() const noexcept
    {
        return return_url;
    }

    void set_params(std::vector<std::pair<std::string, SecretValue>> value)
    {
        params = std::move(value);
    }

    [[nodiscard]] const std::vector<std::pair<std::string, SecretValue>>&
    get_params() const noexcept
    {
        return params;
    }

    void set_return_url_with_query_params(SecretValue value)
    {
        return_url_with_query_params = std::move(value);
    }

    [[nodiscard]] const SecretValue& get_return_url_with_query_params() const noexcept
    {
        return return_url_with_query_params;
    }

    void set_http_method(std::string value)
    {
        http_method = std::move(value);
    }

    [[nodiscard]] const std::string& get_http_method() const noexcept
    {
        return http_method;
    }

    void set_headers(std::vector<std::pair<std::string, std::string>> value)
    {
        headers = std::move(value);
    }

    [[nodiscard]] const std::vector<std::pair<std::string, std::string>>&
    get_headers() const noexcept
    {
        return headers;
    }

    std::string return_url{};
    std::vector<std::pair<std::string, SecretValue>> params{};
    SecretValue return_url_with_query_params{};
    std::string http_method{};
    std::vector<std::pair<std::string, std::string>> headers{};
};

class SdkNextAction : public serde::Serializer<SdkNextAction>
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

    void set_should_block_confirm(std::optional<bool> value)
    {
        should_block_confirm = std::move(value);
    }

    [[nodiscard]] const std::optional<bool>& get_should_block_confirm() const noexcept
    {
        return should_block_confirm;
    }

    NextActionCall next_action{};
    std::optional<bool> should_block_confirm{};
};

class SecretInfoToInitiateSdk : public serde::Serializer<SecretInfoToInitiateSdk>
{
public:
    void set_display(SecretValue value)
    {
        display = std::move(value);
    }

    [[nodiscard]] const SecretValue& get_display() const noexcept
    {
        return display;
    }

    void set_payment(std::optional<SecretValue> value)
    {
        payment = std::move(value);
    }

    [[nodiscard]] const std::optional<SecretValue>& get_payment() const noexcept
    {
        return payment;
    }

    SecretValue display{};
    std::optional<SecretValue> payment{};
};

class SessionTokenForSimplifiedApplePay :
    public serde::Serializer<SessionTokenForSimplifiedApplePay>
{
public:
    void set_initiative_context(std::string value)
    {
        initiative_context = std::move(value);
    }

    [[nodiscard]] const std::string& get_initiative_context() const noexcept
    {
        return initiative_context;
    }

    void set_merchant_business_country(std::optional<CountryAlpha2Value> value)
    {
        merchant_business_country = std::move(value);
    }

    [[nodiscard]] const std::optional<CountryAlpha2Value>&
    get_merchant_business_country() const noexcept
    {
        return merchant_business_country;
    }

    std::string initiative_context{};
    std::optional<CountryAlpha2Value> merchant_business_country{};
};

class SessionTokenInfo : public serde::Serializer<SessionTokenInfo>
{
public:
    void set_certificate(SecretValue value)
    {
        certificate = std::move(value);
    }

    [[nodiscard]] const SecretValue& get_certificate() const noexcept
    {
        return certificate;
    }

    void set_certificate_keys(SecretValue value)
    {
        certificate_keys = std::move(value);
    }

    [[nodiscard]] const SecretValue& get_certificate_keys() const noexcept
    {
        return certificate_keys;
    }

    void set_merchant_identifier(std::string value)
    {
        merchant_identifier = std::move(value);
    }

    [[nodiscard]] const std::string& get_merchant_identifier() const noexcept
    {
        return merchant_identifier;
    }

    void set_display_name(std::string value)
    {
        display_name = std::move(value);
    }

    [[nodiscard]] const std::string& get_display_name() const noexcept
    {
        return display_name;
    }

    void set_initiative(ApplepayInitiative value)
    {
        initiative = std::move(value);
    }

    [[nodiscard]] const ApplepayInitiative& get_initiative() const noexcept
    {
        return initiative;
    }

    void set_initiative_context(std::optional<std::string> value)
    {
        initiative_context = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_initiative_context() const noexcept
    {
        return initiative_context;
    }

    void set_merchant_business_country(std::optional<CountryAlpha2Value> value)
    {
        merchant_business_country = std::move(value);
    }

    [[nodiscard]] const std::optional<CountryAlpha2Value>&
    get_merchant_business_country() const noexcept
    {
        return merchant_business_country;
    }

    void set_payment_processing_details_at(std::optional<PaymentProcessingDetailsAt> value)
    {
        payment_processing_details_at = std::move(value);
    }

    [[nodiscard]] const std::optional<PaymentProcessingDetailsAt>&
    get_payment_processing_details_at() const noexcept
    {
        return payment_processing_details_at;
    }

    void set_payment_processing_detail_input_type(
        std::optional<PaymentProcessingDetailInputType> value
    )
    {
        payment_processing_detail_input_type = std::move(value);
    }

    [[nodiscard]] const std::optional<PaymentProcessingDetailInputType>&
    get_payment_processing_detail_input_type() const noexcept
    {
        return payment_processing_detail_input_type;
    }

    SecretValue certificate{};
    SecretValue certificate_keys{};
    std::string merchant_identifier{};
    std::string display_name{};
    ApplepayInitiative initiative{};
    std::optional<std::string> initiative_context{};
    std::optional<CountryAlpha2Value> merchant_business_country{};
    std::optional<PaymentProcessingDetailsAt> payment_processing_details_at{};
    std::optional<PaymentProcessingDetailInputType> payment_processing_detail_input_type{};
};

class VaultCardToken : public serde::Serializer<VaultCardToken>
{
public:
    void set_card_holder_name(std::optional<SecretValue> value)
    {
        card_holder_name = std::move(value);
    }

    [[nodiscard]] const std::optional<SecretValue>& get_card_holder_name() const noexcept
    {
        return card_holder_name;
    }

    void set_card_cvc(std::optional<SecretValue> value)
    {
        card_cvc = std::move(value);
    }

    [[nodiscard]] const std::optional<SecretValue>& get_card_cvc() const noexcept
    {
        return card_cvc;
    }

    std::optional<SecretValue> card_holder_name{};
    std::optional<SecretValue> card_cvc{};
};

class VaultToken : public serde::Serializer<VaultToken>
{
public:
    void set_card_cvc(SecretValue value)
    {
        card_cvc = std::move(value);
    }

    [[nodiscard]] const SecretValue& get_card_cvc() const noexcept
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

    SecretValue card_cvc{};
    std::optional<SecretValue> card_holder_name{};
};

class WorldpayxmlData : public serde::Serializer<WorldpayxmlData>
{
public:
    void set_funding_transaction_type(std::optional<WorldpayxmlFundingTransactionType> value)
    {
        funding_transaction_type = std::move(value);
    }

    [[nodiscard]] const std::optional<WorldpayxmlFundingTransactionType>&
    get_funding_transaction_type() const noexcept
    {
        return funding_transaction_type;
    }

    void set_payment_purpose(std::optional<WorldpayxmlPaymentPurpose> value)
    {
        payment_purpose = std::move(value);
    }

    [[nodiscard]] const std::optional<WorldpayxmlPaymentPurpose>&
    get_payment_purpose() const noexcept
    {
        return payment_purpose;
    }

    std::optional<WorldpayxmlFundingTransactionType> funding_transaction_type{};
    std::optional<WorldpayxmlPaymentPurpose> payment_purpose{};
};

enum class AdditionalPaymentData
{
    Card,
    BankRedirect,
    Wallet,
    PayLater,
    BankTransfer,
    Crypto,
    BankDebit,
    MandatePayment,
    Reward,
    RealTimePayment,
    Upi,
    GiftCard,
    Voucher,
    CardRedirect,
    CardToken,
    OpenBanking,
    MobilePayment,
    NetworkToken,
};

class Address : public serde::Serializer<Address>
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

    void set_phone(std::optional<PhoneDetailsValue> value)
    {
        phone = std::move(value);
    }

    [[nodiscard]] const std::optional<PhoneDetailsValue>& get_phone() const noexcept
    {
        return phone;
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
    std::optional<PhoneDetailsValue> phone{};
    std::optional<EmailValue> email{};
};

class AdyenConnectorMetadata : public serde::Serializer<AdyenConnectorMetadata>
{
public:
    void set_testing(AdyenTestingData value)
    {
        testing = std::move(value);
    }

    [[nodiscard]] const AdyenTestingData& get_testing() const noexcept
    {
        return testing;
    }

    AdyenTestingData testing{};
};

class ApplePayRecurringDetails : public serde::Serializer<ApplePayRecurringDetails>
{
public:
    void set_payment_description(std::string value)
    {
        payment_description = std::move(value);
    }

    [[nodiscard]] const std::string& get_payment_description() const noexcept
    {
        return payment_description;
    }

    void set_regular_billing(ApplePayRegularBillingDetails value)
    {
        regular_billing = std::move(value);
    }

    [[nodiscard]] const ApplePayRegularBillingDetails& get_regular_billing() const noexcept
    {
        return regular_billing;
    }

    void set_billing_agreement(std::optional<std::string> value)
    {
        billing_agreement = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_billing_agreement() const noexcept
    {
        return billing_agreement;
    }

    void set_management_url(UrlValue value)
    {
        management_url = std::move(value);
    }

    [[nodiscard]] const UrlValue& get_management_url() const noexcept
    {
        return management_url;
    }

    std::string payment_description{};
    ApplePayRegularBillingDetails regular_billing{};
    std::optional<std::string> billing_agreement{};
    UrlValue management_url{};
};

class ApplepayConnectorMetadataRequest : public serde::Serializer<ApplepayConnectorMetadataRequest>
{
public:
    void set_session_token_data(std::optional<SessionTokenInfo> value)
    {
        session_token_data = std::move(value);
    }

    [[nodiscard]] const std::optional<SessionTokenInfo>& get_session_token_data() const noexcept
    {
        return session_token_data;
    }

    std::optional<SessionTokenInfo> session_token_data{};
};

class BankCodeInformation : public serde::Serializer<BankCodeInformation>
{
public:
    void set_bank_name(BankNamesValue value)
    {
        bank_name = std::move(value);
    }

    [[nodiscard]] const BankNamesValue& get_bank_name() const noexcept
    {
        return bank_name;
    }

    void set_connector_codes(std::vector<ConnectorCode> value)
    {
        connector_codes = std::move(value);
    }

    [[nodiscard]] const std::vector<ConnectorCode>& get_connector_codes() const noexcept
    {
        return connector_codes;
    }

    BankNamesValue bank_name{};
    std::vector<ConnectorCode> connector_codes{};
};

class BankDebitBilling : public serde::Serializer<BankDebitBilling>
{
public:
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

    void set_address(std::optional<AddressDetails> value)
    {
        address = std::move(value);
    }

    [[nodiscard]] const std::optional<AddressDetails>& get_address() const noexcept
    {
        return address;
    }

    std::optional<SecretValue> name{};
    std::optional<EmailValue> email{};
    std::optional<AddressDetails> address{};
};

class BankRedirectResponse : public serde::Serializer<BankRedirectResponse>
{
public:
    void set_bank_name(std::optional<BankNamesValue> value)
    {
        bank_name = std::move(value);
    }

    [[nodiscard]] const std::optional<BankNamesValue>& get_bank_name() const noexcept
    {
        return bank_name;
    }

    void set_details(std::optional<BankRedirectDetailsValue> value)
    {
        details = std::move(value);
    }

    [[nodiscard]] const std::optional<BankRedirectDetailsValue>& get_details() const noexcept
    {
        return details;
    }

    void set_interac(std::optional<InteracPaymentMethod> value)
    {
        interac = std::move(value);
    }

    [[nodiscard]] const std::optional<InteracPaymentMethod>& get_interac() const noexcept
    {
        return interac;
    }

    std::optional<BankNamesValue> bank_name{};
    std::optional<BankRedirectDetailsValue> details{};
    std::optional<InteracPaymentMethod> interac{};
};

enum class BankTransferInstructions
{
    DokuBankTransferInstructions,
    AchCreditTransfer,
    SepaBankInstructions,
    BacsBankInstructions,
    Multibanco,
};

enum class BillingConnectorPaymentMethodDetails
{
    Card,
};

class CheckAndApplyPaymentMethodDataResponse :
    public serde::Serializer<CheckAndApplyPaymentMethodDataResponse>
{
public:
    void set_balances(std::vector<EligibilityBalanceCheckResponseItem> value)
    {
        balances = std::move(value);
    }

    [[nodiscard]] const std::vector<EligibilityBalanceCheckResponseItem>&
    get_balances() const noexcept
    {
        return balances;
    }

    void set_remaining_amount(MinorUnitValue value)
    {
        remaining_amount = std::move(value);
    }

    [[nodiscard]] const MinorUnitValue& get_remaining_amount() const noexcept
    {
        return remaining_amount;
    }

    void set_currency(CurrencyValue value)
    {
        currency = std::move(value);
    }

    [[nodiscard]] const CurrencyValue& get_currency() const noexcept
    {
        return currency;
    }

    void set_requires_additional_pm_data(bool value)
    {
        requires_additional_pm_data = std::move(value);
    }

    [[nodiscard]] const bool& get_requires_additional_pm_data() const noexcept
    {
        return requires_additional_pm_data;
    }

    void set_surcharge_details(
        std::optional<std::vector<ApplyPaymentMethodDataSurchargeResponseItem>> value
    )
    {
        surcharge_details = std::move(value);
    }

    [[nodiscard]] const std::optional<std::vector<ApplyPaymentMethodDataSurchargeResponseItem>>&
    get_surcharge_details() const noexcept
    {
        return surcharge_details;
    }

    std::vector<EligibilityBalanceCheckResponseItem> balances{};
    MinorUnitValue remaining_amount{};
    CurrencyValue currency{};
    bool requires_additional_pm_data{};
    std::optional<std::vector<ApplyPaymentMethodDataSurchargeResponseItem>> surcharge_details{};
};

class CollectionActions : public serde::Serializer<CollectionActions>
{
public:
    void set_legal_protest(std::optional<ProtestRules> value)
    {
        legal_protest = std::move(value);
    }

    [[nodiscard]] const std::optional<ProtestRules>& get_legal_protest() const noexcept
    {
        return legal_protest;
    }

    void set_auto_write_off_days(std::optional<std::uint32_t> value)
    {
        auto_write_off_days = std::move(value);
    }

    [[nodiscard]] const std::optional<std::uint32_t>& get_auto_write_off_days() const noexcept
    {
        return auto_write_off_days;
    }

    std::optional<ProtestRules> legal_protest{};
    std::optional<std::uint32_t> auto_write_off_days{};
};

class CustomRecoveryPaymentMethodData : public serde::Serializer<CustomRecoveryPaymentMethodData>
{
public:
    void set_primary_processor_payment_method_token(SecretValue value)
    {
        primary_processor_payment_method_token = std::move(value);
    }

    [[nodiscard]] const SecretValue& get_primary_processor_payment_method_token() const noexcept
    {
        return primary_processor_payment_method_token;
    }

    void set_payment_method_metadata(AdditionalCardInfo value)
    {
        payment_method_metadata = std::move(value);
    }

    [[nodiscard]] const AdditionalCardInfo& get_payment_method_metadata() const noexcept
    {
        return payment_method_metadata;
    }

    SecretValue primary_processor_payment_method_token{};
    AdditionalCardInfo payment_method_metadata{};
};

enum class EligibilityPaymentMethodData
{
    Card,
    CardBin,
    CardRedirect,
    Wallet,
    PayLater,
    BankRedirect,
    BankDebit,
    BankTransfer,
    RealTimePayment,
    Crypto,
    MandatePayment,
    Reward,
    Upi,
    Voucher,
    GiftCard,
    CardToken,
    OpenBanking,
    MobilePayment,
    NetworkToken,
};

class GooglePayThirdPartySdk : public serde::Serializer<GooglePayThirdPartySdk>
{
public:
    void set_delayed_session_token(bool value)
    {
        delayed_session_token = std::move(value);
    }

    [[nodiscard]] const bool& get_delayed_session_token() const noexcept
    {
        return delayed_session_token;
    }

    void set_connector(std::string value)
    {
        connector = std::move(value);
    }

    [[nodiscard]] const std::string& get_connector() const noexcept
    {
        return connector;
    }

    void set_sdk_next_action(SdkNextAction value)
    {
        sdk_next_action = std::move(value);
    }

    [[nodiscard]] const SdkNextAction& get_sdk_next_action() const noexcept
    {
        return sdk_next_action;
    }

    bool delayed_session_token{};
    std::string connector{};
    SdkNextAction sdk_next_action{};
};

class PaymentAttemptFeatureMetadata : public serde::Serializer<PaymentAttemptFeatureMetadata>
{
public:
    void set_revenue_recovery(std::optional<PaymentAttemptRevenueRecoveryData> value)
    {
        revenue_recovery = std::move(value);
    }

    [[nodiscard]] const std::optional<PaymentAttemptRevenueRecoveryData>&
    get_revenue_recovery() const noexcept
    {
        return revenue_recovery;
    }

    std::optional<PaymentAttemptRevenueRecoveryData> revenue_recovery{};
};

enum class PaymentMethodData
{
    Card,
    CardWithNoCVC,
    CardRedirect,
    Wallet,
    PayLater,
    BankRedirect,
    BankDebit,
    BankTransfer,
    RealTimePayment,
    Crypto,
    MandatePayment,
    Reward,
    Upi,
    Voucher,
    GiftCard,
    CardToken,
    OpenBanking,
    MobilePayment,
    NetworkToken,
    NetworkTransactionIdAndCardDetails,
    NetworkTransactionIdAndNetworkTokenDetails,
    NetworkTransactionIdAndDecryptedWalletTokenDetails,
    ProxyCard,
    VaultCardTokenData,
};

enum class PaymentMethodDataResponse
{
    Card,
    BankTransfer,
    Wallet,
    PayLater,
    BankRedirect,
    Crypto,
    BankDebit,
    MandatePayment,
    Reward,
    RealTimePayment,
    Upi,
    Voucher,
    GiftCard,
    CardRedirect,
    CardToken,
    OpenBanking,
    MobilePayment,
    NetworkToken,
};

class PenaltyRules : public serde::Serializer<PenaltyRules>
{
public:
    void set_fixed_penalty(std::optional<PenaltyDetail> value)
    {
        fixed_penalty = std::move(value);
    }

    [[nodiscard]] const std::optional<PenaltyDetail>& get_fixed_penalty() const noexcept
    {
        return fixed_penalty;
    }

    void set_interest(std::optional<InterestDetail> value)
    {
        interest = std::move(value);
    }

    [[nodiscard]] const std::optional<InterestDetail>& get_interest() const noexcept
    {
        return interest;
    }

    std::optional<PenaltyDetail> fixed_penalty{};
    std::optional<InterestDetail> interest{};
};

enum class ProxyPaymentMethodData
{
    ProxyCard,
    VaultToken,
};

class SantanderPaymentDiscountRules : public serde::Serializer<SantanderPaymentDiscountRules>
{
public:
    void set_discount_type(std::optional<DiscountType> value)
    {
        discount_type = std::move(value);
    }

    [[nodiscard]] const std::optional<DiscountType>& get_discount_type() const noexcept
    {
        return discount_type;
    }

    void set_tiers(std::vector<DiscountTier> value)
    {
        tiers = std::move(value);
    }

    [[nodiscard]] const std::vector<DiscountTier>& get_tiers() const noexcept
    {
        return tiers;
    }

    std::optional<DiscountType> discount_type{};
    std::vector<DiscountTier> tiers{};
};

class BankData : public serde::Serializer<BankData>
{
public:
    void set_payment_method_type(PaymentMethodTypeValue value)
    {
        payment_method_type = std::move(value);
    }

    [[nodiscard]] const PaymentMethodTypeValue& get_payment_method_type() const noexcept
    {
        return payment_method_type;
    }

    void set_code_information(std::vector<BankCodeInformation> value)
    {
        code_information = std::move(value);
    }

    [[nodiscard]] const std::vector<BankCodeInformation>& get_code_information() const noexcept
    {
        return code_information;
    }

    PaymentMethodTypeValue payment_method_type{};
    std::vector<BankCodeInformation> code_information{};
};

class BankTransferNextStepsData : public serde::Serializer<BankTransferNextStepsData>
{
public:
    void set_bank_transfer_instructions(BankTransferInstructions value)
    {
        bank_transfer_instructions = std::move(value);
    }

    [[nodiscard]] const BankTransferInstructions& get_bank_transfer_instructions() const noexcept
    {
        return bank_transfer_instructions;
    }

    void set_receiver(std::optional<ReceiverDetails> value)
    {
        receiver = std::move(value);
    }

    [[nodiscard]] const std::optional<ReceiverDetails>& get_receiver() const noexcept
    {
        return receiver;
    }

    BankTransferInstructions bank_transfer_instructions{};
    std::optional<ReceiverDetails> receiver{};
};

class BoletoAdditionalDetails : public serde::Serializer<BoletoAdditionalDetails>
{
public:
    void set_due_date(std::optional<PrimitiveDateTimeValue> value)
    {
        due_date = std::move(value);
    }

    [[nodiscard]] const std::optional<PrimitiveDateTimeValue>& get_due_date() const noexcept
    {
        return due_date;
    }

    void set_document_kind(std::optional<BoletoDocumentKindValue> value)
    {
        document_kind = std::move(value);
    }

    [[nodiscard]] const std::optional<BoletoDocumentKindValue>& get_document_kind() const noexcept
    {
        return document_kind;
    }

    void set_payment_type(std::optional<BoletoPaymentTypeValue> value)
    {
        payment_type = std::move(value);
    }

    [[nodiscard]] const std::optional<BoletoPaymentTypeValue>& get_payment_type() const noexcept
    {
        return payment_type;
    }

    void set_covenant_code(std::optional<SecretValue> value)
    {
        covenant_code = std::move(value);
    }

    [[nodiscard]] const std::optional<SecretValue>& get_covenant_code() const noexcept
    {
        return covenant_code;
    }

    void set_pix_key(std::optional<PixKeyValue> value)
    {
        pix_key = std::move(value);
    }

    [[nodiscard]] const std::optional<PixKeyValue>& get_pix_key() const noexcept
    {
        return pix_key;
    }

    void set_discount_rules(std::optional<SantanderPaymentDiscountRules> value)
    {
        discount_rules = std::move(value);
    }

    [[nodiscard]] const std::optional<SantanderPaymentDiscountRules>&
    get_discount_rules() const noexcept
    {
        return discount_rules;
    }

    void set_penalties(std::optional<PenaltyRules> value)
    {
        penalties = std::move(value);
    }

    [[nodiscard]] const std::optional<PenaltyRules>& get_penalties() const noexcept
    {
        return penalties;
    }

    void set_collection_actions(std::optional<CollectionActions> value)
    {
        collection_actions = std::move(value);
    }

    [[nodiscard]] const std::optional<CollectionActions>& get_collection_actions() const noexcept
    {
        return collection_actions;
    }

    void set_payment_constraints(std::optional<BoletoPaymentTypeConstraints> value)
    {
        payment_constraints = std::move(value);
    }

    [[nodiscard]] const std::optional<BoletoPaymentTypeConstraints>&
    get_payment_constraints() const noexcept
    {
        return payment_constraints;
    }

    void set_beneficiary(std::optional<BeneficiaryDetails> value)
    {
        beneficiary = std::move(value);
    }

    [[nodiscard]] const std::optional<BeneficiaryDetails>& get_beneficiary() const noexcept
    {
        return beneficiary;
    }

    std::optional<PrimitiveDateTimeValue> due_date{};
    std::optional<BoletoDocumentKindValue> document_kind{};
    std::optional<BoletoPaymentTypeValue> payment_type{};
    std::optional<SecretValue> covenant_code{};
    std::optional<PixKeyValue> pix_key{};
    std::optional<SantanderPaymentDiscountRules> discount_rules{};
    std::optional<PenaltyRules> penalties{};
    std::optional<CollectionActions> collection_actions{};
    std::optional<BoletoPaymentTypeConstraints> payment_constraints{};
    std::optional<BeneficiaryDetails> beneficiary{};
};

class EligibilityPaymentMethodDataRequest :
    public serde::Serializer<EligibilityPaymentMethodDataRequest>
{
public:
    void set_payment_method_data(std::optional<EligibilityPaymentMethodData> value)
    {
        payment_method_data = std::move(value);
    }

    [[nodiscard]] const std::optional<EligibilityPaymentMethodData>&
    get_payment_method_data() const noexcept
    {
        return payment_method_data;
    }

    void set_billing(std::optional<Address> value)
    {
        billing = std::move(value);
    }

    [[nodiscard]] const std::optional<Address>& get_billing() const noexcept
    {
        return billing;
    }

    std::optional<EligibilityPaymentMethodData> payment_method_data{};
    std::optional<Address> billing{};
};

class PaymentMethodDataRequest : public serde::Serializer<PaymentMethodDataRequest>
{
public:
    void set_payment_method_data(std::optional<PaymentMethodData> value)
    {
        payment_method_data = std::move(value);
    }

    [[nodiscard]] const std::optional<PaymentMethodData>& get_payment_method_data() const noexcept
    {
        return payment_method_data;
    }

    void set_billing(std::optional<Address> value)
    {
        billing = std::move(value);
    }

    [[nodiscard]] const std::optional<Address>& get_billing() const noexcept
    {
        return billing;
    }

    std::optional<PaymentMethodData> payment_method_data{};
    std::optional<Address> billing{};
};

class PaymentMethodDataResponseWithBilling :
    public serde::Serializer<PaymentMethodDataResponseWithBilling>
{
public:
    void set_payment_method_data(std::optional<PaymentMethodDataResponse> value)
    {
        payment_method_data = std::move(value);
    }

    [[nodiscard]] const std::optional<PaymentMethodDataResponse>&
    get_payment_method_data() const noexcept
    {
        return payment_method_data;
    }

    void set_billing(std::optional<Address> value)
    {
        billing = std::move(value);
    }

    [[nodiscard]] const std::optional<Address>& get_billing() const noexcept
    {
        return billing;
    }

    std::optional<PaymentMethodDataResponse> payment_method_data{};
    std::optional<Address> billing{};
};

class PaymentRevenueRecoveryMetadata : public serde::Serializer<PaymentRevenueRecoveryMetadata>
{
public:
    void set_total_retry_count(std::uint16_t value)
    {
        total_retry_count = std::move(value);
    }

    [[nodiscard]] const std::uint16_t& get_total_retry_count() const noexcept
    {
        return total_retry_count;
    }

    void set_payment_connector_transmission(std::optional<PaymentConnectorTransmissionValue> value)
    {
        payment_connector_transmission = std::move(value);
    }

    [[nodiscard]] const std::optional<PaymentConnectorTransmissionValue>&
    get_payment_connector_transmission() const noexcept
    {
        return payment_connector_transmission;
    }

    void set_billing_connector_id(std::optional<MerchantConnectorAccountIdValue> value)
    {
        billing_connector_id = std::move(value);
    }

    [[nodiscard]] const std::optional<MerchantConnectorAccountIdValue>&
    get_billing_connector_id() const noexcept
    {
        return billing_connector_id;
    }

    void set_active_attempt_payment_connector_id(
        std::optional<MerchantConnectorAccountIdValue> value
    )
    {
        active_attempt_payment_connector_id = std::move(value);
    }

    [[nodiscard]] const std::optional<MerchantConnectorAccountIdValue>&
    get_active_attempt_payment_connector_id() const noexcept
    {
        return active_attempt_payment_connector_id;
    }

    void set_billing_connector_payment_details(BillingConnectorPaymentDetails value)
    {
        billing_connector_payment_details = std::move(value);
    }

    [[nodiscard]] const BillingConnectorPaymentDetails&
    get_billing_connector_payment_details() const noexcept
    {
        return billing_connector_payment_details;
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

    void set_connector(ConnectorValue value)
    {
        connector = std::move(value);
    }

    [[nodiscard]] const ConnectorValue& get_connector() const noexcept
    {
        return connector;
    }

    void set_billing_connector_payment_method_details(
        std::optional<BillingConnectorPaymentMethodDetails> value
    )
    {
        billing_connector_payment_method_details = std::move(value);
    }

    [[nodiscard]] const std::optional<BillingConnectorPaymentMethodDetails>&
    get_billing_connector_payment_method_details() const noexcept
    {
        return billing_connector_payment_method_details;
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

    void set_first_payment_attempt_pg_error_code(std::optional<std::string> value)
    {
        first_payment_attempt_pg_error_code = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>&
    get_first_payment_attempt_pg_error_code() const noexcept
    {
        return first_payment_attempt_pg_error_code;
    }

    void set_first_payment_attempt_network_decline_code(std::optional<std::string> value)
    {
        first_payment_attempt_network_decline_code = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>&
    get_first_payment_attempt_network_decline_code() const noexcept
    {
        return first_payment_attempt_network_decline_code;
    }

    void set_first_payment_attempt_network_advice_code(std::optional<std::string> value)
    {
        first_payment_attempt_network_advice_code = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>&
    get_first_payment_attempt_network_advice_code() const noexcept
    {
        return first_payment_attempt_network_advice_code;
    }

    void set_revenue_recovery_ab_routing(std::optional<RevenueRecoveryABAlgorithmValue> value)
    {
        revenue_recovery_ab_routing = std::move(value);
    }

    [[nodiscard]] const std::optional<RevenueRecoveryABAlgorithmValue>&
    get_revenue_recovery_ab_routing() const noexcept
    {
        return revenue_recovery_ab_routing;
    }

    void set_external_trigger_state(std::optional<ExternalTriggerStateValue> value)
    {
        external_trigger_state = std::move(value);
    }

    [[nodiscard]] const std::optional<ExternalTriggerStateValue>&
    get_external_trigger_state() const noexcept
    {
        return external_trigger_state;
    }

    void set_last_failure_code(std::optional<StandardisedCodeValue> value)
    {
        last_failure_code = std::move(value);
    }

    [[nodiscard]] const std::optional<StandardisedCodeValue>& get_last_failure_code() const noexcept
    {
        return last_failure_code;
    }

    std::uint16_t total_retry_count{};
    std::optional<PaymentConnectorTransmissionValue> payment_connector_transmission{};
    std::optional<MerchantConnectorAccountIdValue> billing_connector_id{};
    std::optional<MerchantConnectorAccountIdValue> active_attempt_payment_connector_id{};
    BillingConnectorPaymentDetails billing_connector_payment_details{};
    PaymentMethodValue payment_method_type{};
    PaymentMethodTypeValue payment_method_subtype{};
    ConnectorValue connector{};
    std::optional<BillingConnectorPaymentMethodDetails> billing_connector_payment_method_details{};
    std::optional<PrimitiveDateTimeValue> invoice_next_billing_time{};
    std::optional<PrimitiveDateTimeValue> invoice_billing_started_at_time{};
    std::optional<std::string> first_payment_attempt_pg_error_code{};
    std::optional<std::string> first_payment_attempt_network_decline_code{};
    std::optional<std::string> first_payment_attempt_network_advice_code{};
    std::optional<RevenueRecoveryABAlgorithmValue> revenue_recovery_ab_routing{};
    std::optional<ExternalTriggerStateValue> external_trigger_state{};
    std::optional<StandardisedCodeValue> last_failure_code{};
};

class ProxyPaymentMethodDataRequest : public serde::Serializer<ProxyPaymentMethodDataRequest>
{
public:
    void set_payment_method_data(std::optional<ProxyPaymentMethodData> value)
    {
        payment_method_data = std::move(value);
    }

    [[nodiscard]] const std::optional<ProxyPaymentMethodData>&
    get_payment_method_data() const noexcept
    {
        return payment_method_data;
    }

    void set_billing(std::optional<Address> value)
    {
        billing = std::move(value);
    }

    [[nodiscard]] const std::optional<Address>& get_billing() const noexcept
    {
        return billing;
    }

    std::optional<ProxyPaymentMethodData> payment_method_data{};
    std::optional<Address> billing{};
};

class RecordAttemptPaymentMethodDataRequest :
    public serde::Serializer<RecordAttemptPaymentMethodDataRequest>
{
public:
    void set_payment_method_data(AdditionalPaymentData value)
    {
        payment_method_data = std::move(value);
    }

    [[nodiscard]] const AdditionalPaymentData& get_payment_method_data() const noexcept
    {
        return payment_method_data;
    }

    void set_billing(std::optional<Address> value)
    {
        billing = std::move(value);
    }

    [[nodiscard]] const std::optional<Address>& get_billing() const noexcept
    {
        return billing;
    }

    AdditionalPaymentData payment_method_data{};
    std::optional<Address> billing{};
};

class SantanderBoletoData : public serde::Serializer<SantanderBoletoData>
{
public:
    void set_discount_rules(std::optional<SantanderPaymentDiscountRules> value)
    {
        discount_rules = std::move(value);
    }

    [[nodiscard]] const std::optional<SantanderPaymentDiscountRules>&
    get_discount_rules() const noexcept
    {
        return discount_rules;
    }

    void set_penalties(std::optional<PenaltyRules> value)
    {
        penalties = std::move(value);
    }

    [[nodiscard]] const std::optional<PenaltyRules>& get_penalties() const noexcept
    {
        return penalties;
    }

    void set_collection_actions(std::optional<CollectionActions> value)
    {
        collection_actions = std::move(value);
    }

    [[nodiscard]] const std::optional<CollectionActions>& get_collection_actions() const noexcept
    {
        return collection_actions;
    }

    void set_payment_constraints(std::optional<BoletoPaymentTypeConstraints> value)
    {
        payment_constraints = std::move(value);
    }

    [[nodiscard]] const std::optional<BoletoPaymentTypeConstraints>&
    get_payment_constraints() const noexcept
    {
        return payment_constraints;
    }

    void set_beneficiary(std::optional<BeneficiaryDetails> value)
    {
        beneficiary = std::move(value);
    }

    [[nodiscard]] const std::optional<BeneficiaryDetails>& get_beneficiary() const noexcept
    {
        return beneficiary;
    }

    void set_document_kind(std::optional<BoletoDocumentKindValue> value)
    {
        document_kind = std::move(value);
    }

    [[nodiscard]] const std::optional<BoletoDocumentKindValue>& get_document_kind() const noexcept
    {
        return document_kind;
    }

    std::optional<SantanderPaymentDiscountRules> discount_rules{};
    std::optional<PenaltyRules> penalties{};
    std::optional<CollectionActions> collection_actions{};
    std::optional<BoletoPaymentTypeConstraints> payment_constraints{};
    std::optional<BeneficiaryDetails> beneficiary{};
    std::optional<BoletoDocumentKindValue> document_kind{};
};

class SplitPaymentMethodDataRequest : public serde::Serializer<SplitPaymentMethodDataRequest>
{
public:
    void set_payment_method_data(PaymentMethodData value)
    {
        payment_method_data = std::move(value);
    }

    [[nodiscard]] const PaymentMethodData& get_payment_method_data() const noexcept
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

    PaymentMethodData payment_method_data{};
    PaymentMethodValue payment_method_type{};
    PaymentMethodTypeValue payment_method_subtype{};
};

enum class WalletData
{
    AliPayHkRedirect,
    AliPayQr,
    AliPayRedirect,
    AmazonPay,
    AmazonPayRedirect,
    ApplePay,
    ApplePayRedirect,
    ApplePayThirdPartySdk,
    BluecodeRedirect,
    CashappQr,
    DanaRedirect,
    GcashRedirect,
    GoPayRedirect,
    GooglePay,
    GooglePayRedirect,
    GooglePayThirdPartySdk,
    KakaoPayRedirect,
    MbWayRedirect,
    Mifinity,
    MobilePayRedirect,
    MomoRedirect,
    PaypalRedirect,
    PaypalSdk,
    Paysera,
    Paze,
    RevolutPay,
    SamsungPay,
    Skrill,
    Neteller,
    SwishQr,
    TouchNGoRedirect,
    TwintRedirect,
    VippsRedirect,
    WeroRedirect,
    WeChatPayQr,
    WeChatPayRedirect,
};

class FeatureMetadata : public serde::Serializer<FeatureMetadata>
{
public:
    void set_redirect_response(std::optional<RedirectResponse> value)
    {
        redirect_response = std::move(value);
    }

    [[nodiscard]] const std::optional<RedirectResponse>& get_redirect_response() const noexcept
    {
        return redirect_response;
    }

    void set_search_tags(std::optional<std::vector<HashedStringValue>> value)
    {
        search_tags = std::move(value);
    }

    [[nodiscard]] const std::optional<std::vector<HashedStringValue>>&
    get_search_tags() const noexcept
    {
        return search_tags;
    }

    void set_apple_pay_recurring_details(std::optional<ApplePayRecurringDetails> value)
    {
        apple_pay_recurring_details = std::move(value);
    }

    [[nodiscard]] const std::optional<ApplePayRecurringDetails>&
    get_apple_pay_recurring_details() const noexcept
    {
        return apple_pay_recurring_details;
    }

    void set_revenue_recovery(std::optional<PaymentRevenueRecoveryMetadata> value)
    {
        revenue_recovery = std::move(value);
    }

    [[nodiscard]] const std::optional<PaymentRevenueRecoveryMetadata>&
    get_revenue_recovery() const noexcept
    {
        return revenue_recovery;
    }

    void set_pix_additional_details(std::optional<PixAdditionalDetails> value)
    {
        pix_additional_details = std::move(value);
    }

    [[nodiscard]] const std::optional<PixAdditionalDetails>&
    get_pix_additional_details() const noexcept
    {
        return pix_additional_details;
    }

    void set_boleto_additional_details(std::optional<BoletoAdditionalDetails> value)
    {
        boleto_additional_details = std::move(value);
    }

    [[nodiscard]] const std::optional<BoletoAdditionalDetails>&
    get_boleto_additional_details() const noexcept
    {
        return boleto_additional_details;
    }

    void set_pix_automatico_additional_details(std::optional<PixAutomaticoAdditionalDetails> value)
    {
        pix_automatico_additional_details = std::move(value);
    }

    [[nodiscard]] const std::optional<PixAutomaticoAdditionalDetails>&
    get_pix_automatico_additional_details() const noexcept
    {
        return pix_automatico_additional_details;
    }

    void set_finix_additional_details(std::optional<FinixAdditionalDetails> value)
    {
        finix_additional_details = std::move(value);
    }

    [[nodiscard]] const std::optional<FinixAdditionalDetails>&
    get_finix_additional_details() const noexcept
    {
        return finix_additional_details;
    }

    void set_multi_currency_processing(std::optional<MultiCurrencyProcessingData> value)
    {
        multi_currency_processing = std::move(value);
    }

    [[nodiscard]] const std::optional<MultiCurrencyProcessingData>&
    get_multi_currency_processing() const noexcept
    {
        return multi_currency_processing;
    }

    std::optional<RedirectResponse> redirect_response{};
    std::optional<std::vector<HashedStringValue>> search_tags{};
    std::optional<ApplePayRecurringDetails> apple_pay_recurring_details{};
    std::optional<PaymentRevenueRecoveryMetadata> revenue_recovery{};
    std::optional<PixAdditionalDetails> pix_additional_details{};
    std::optional<BoletoAdditionalDetails> boleto_additional_details{};
    std::optional<PixAutomaticoAdditionalDetails> pix_automatico_additional_details{};
    std::optional<FinixAdditionalDetails> finix_additional_details{};
    std::optional<MultiCurrencyProcessingData> multi_currency_processing{};
};

class SantanderConnectorMetadataData : public serde::Serializer<SantanderConnectorMetadataData>
{
public:
    void set_boleto(std::optional<SantanderBoletoData> value)
    {
        boleto = std::move(value);
    }

    [[nodiscard]] const std::optional<SantanderBoletoData>& get_boleto() const noexcept
    {
        return boleto;
    }

    std::optional<SantanderBoletoData> boleto{};
};

class ConnectorMetadata : public serde::Serializer<ConnectorMetadata>
{
public:
    void set_apple_pay(std::optional<ApplepayConnectorMetadataRequest> value)
    {
        apple_pay = std::move(value);
    }

    [[nodiscard]] const std::optional<ApplepayConnectorMetadataRequest>&
    get_apple_pay() const noexcept
    {
        return apple_pay;
    }

    void set_airwallex(std::optional<AirwallexData> value)
    {
        airwallex = std::move(value);
    }

    [[nodiscard]] const std::optional<AirwallexData>& get_airwallex() const noexcept
    {
        return airwallex;
    }

    void set_noon(std::optional<NoonData> value)
    {
        noon = std::move(value);
    }

    [[nodiscard]] const std::optional<NoonData>& get_noon() const noexcept
    {
        return noon;
    }

    void set_braintree(std::optional<BraintreeData> value)
    {
        braintree = std::move(value);
    }

    [[nodiscard]] const std::optional<BraintreeData>& get_braintree() const noexcept
    {
        return braintree;
    }

    void set_adyen(std::optional<AdyenConnectorMetadata> value)
    {
        adyen = std::move(value);
    }

    [[nodiscard]] const std::optional<AdyenConnectorMetadata>& get_adyen() const noexcept
    {
        return adyen;
    }

    void set_peachpayments(std::optional<PeachpaymentsData> value)
    {
        peachpayments = std::move(value);
    }

    [[nodiscard]] const std::optional<PeachpaymentsData>& get_peachpayments() const noexcept
    {
        return peachpayments;
    }

    void set_santander(std::optional<SantanderConnectorMetadataData> value)
    {
        santander = std::move(value);
    }

    [[nodiscard]] const std::optional<SantanderConnectorMetadataData>&
    get_santander() const noexcept
    {
        return santander;
    }

    void set_paypal(std::optional<PaypalConnectorMetadata> value)
    {
        paypal = std::move(value);
    }

    [[nodiscard]] const std::optional<PaypalConnectorMetadata>& get_paypal() const noexcept
    {
        return paypal;
    }

    void set_worldpayxml(std::optional<WorldpayxmlData> value)
    {
        worldpayxml = std::move(value);
    }

    [[nodiscard]] const std::optional<WorldpayxmlData>& get_worldpayxml() const noexcept
    {
        return worldpayxml;
    }

    void set_checkout(std::optional<CheckoutData> value)
    {
        checkout = std::move(value);
    }

    [[nodiscard]] const std::optional<CheckoutData>& get_checkout() const noexcept
    {
        return checkout;
    }

    void set_stripe(std::optional<StripeConnectorMetadata> value)
    {
        stripe = std::move(value);
    }

    [[nodiscard]] const std::optional<StripeConnectorMetadata>& get_stripe() const noexcept
    {
        return stripe;
    }

    std::optional<ApplepayConnectorMetadataRequest> apple_pay{};
    std::optional<AirwallexData> airwallex{};
    std::optional<NoonData> noon{};
    std::optional<BraintreeData> braintree{};
    std::optional<AdyenConnectorMetadata> adyen{};
    std::optional<PeachpaymentsData> peachpayments{};
    std::optional<SantanderConnectorMetadataData> santander{};
    std::optional<PaypalConnectorMetadata> paypal{};
    std::optional<WorldpayxmlData> worldpayxml{};
    std::optional<CheckoutData> checkout{};
    std::optional<StripeConnectorMetadata> stripe{};
};

class PaymentsUpdateMetadataRequest : public serde::Serializer<PaymentsUpdateMetadataRequest>
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

    void set_metadata(std::optional<SecretSerdeValueValue> value)
    {
        metadata = std::move(value);
    }

    [[nodiscard]] const std::optional<SecretSerdeValueValue>& get_metadata() const noexcept
    {
        return metadata;
    }

    void set_feature_metadata(std::optional<FeatureMetadata> value)
    {
        feature_metadata = std::move(value);
    }

    [[nodiscard]] const std::optional<FeatureMetadata>& get_feature_metadata() const noexcept
    {
        return feature_metadata;
    }

    PaymentIdValue payment_id{};
    std::optional<SecretSerdeValueValue> metadata{};
    std::optional<FeatureMetadata> feature_metadata{};
};

class PaymentsUpdateMetadataResponse : public serde::Serializer<PaymentsUpdateMetadataResponse>
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

    void set_metadata(std::optional<SecretSerdeValueValue> value)
    {
        metadata = std::move(value);
    }

    [[nodiscard]] const std::optional<SecretSerdeValueValue>& get_metadata() const noexcept
    {
        return metadata;
    }

    void set_status(std::optional<IntentStatusValue> value)
    {
        status = std::move(value);
    }

    [[nodiscard]] const std::optional<IntentStatusValue>& get_status() const noexcept
    {
        return status;
    }

    void set_feature_metadata(std::optional<FeatureMetadata> value)
    {
        feature_metadata = std::move(value);
    }

    [[nodiscard]] const std::optional<FeatureMetadata>& get_feature_metadata() const noexcept
    {
        return feature_metadata;
    }

    PaymentIdValue payment_id{};
    std::optional<SecretSerdeValueValue> metadata{};
    std::optional<IntentStatusValue> status{};
    std::optional<FeatureMetadata> feature_metadata{};
};

} // namespace app::model::payments

template<>
struct serde::Serializable<app::model::payments::BankData>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "payment_method_type",
                &app::model::payments::BankData::get_payment_method_type,
                &app::model::payments::BankData::set_payment_method_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "code_information",
                &app::model::payments::BankData::get_code_information,
                &app::model::payments::BankData::set_code_information,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "bank_data";
    }
};

template<>
struct serde::Serializable<app::model::payments::BankCodeInformation>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "bank_name",
                &app::model::payments::BankCodeInformation::get_bank_name,
                &app::model::payments::BankCodeInformation::set_bank_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "connector_codes",
                &app::model::payments::BankCodeInformation::get_connector_codes,
                &app::model::payments::BankCodeInformation::set_connector_codes,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "bank_code_information";
    }
};

template<>
struct serde::Serializable<app::model::payments::ConnectorCode>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "connector",
                &app::model::payments::ConnectorCode::get_connector,
                &app::model::payments::ConnectorCode::set_connector,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "code",
                &app::model::payments::ConnectorCode::get_code,
                &app::model::payments::ConnectorCode::set_code,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "connector_code";
    }
};

template<>
struct serde::Serializable<app::model::payments::BankCodeResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "bank_name",
                &app::model::payments::BankCodeResponse::get_bank_name,
                &app::model::payments::BankCodeResponse::set_bank_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "eligible_connectors",
                &app::model::payments::BankCodeResponse::get_eligible_connectors,
                &app::model::payments::BankCodeResponse::set_eligible_connectors,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "bank_code_response";
    }
};

template<>
struct serde::Serializable<app::model::payments::PaymentAttemptFeatureMetadata>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<
            "revenue_recovery",
            &app::model::payments::PaymentAttemptFeatureMetadata::get_revenue_recovery,
            &app::model::payments::PaymentAttemptFeatureMetadata::set_revenue_recovery,
            serde::FieldOptions{}>{}};
    }

    static constexpr std::string_view table_name()
    {
        return "payment_attempt_feature_metadata";
    }
};

template<>
struct serde::Serializable<app::model::payments::PaymentAttemptRevenueRecoveryData>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "attempt_triggered_by",
                &app::model::payments::PaymentAttemptRevenueRecoveryData::get_attempt_triggered_by,
                &app::model::payments::PaymentAttemptRevenueRecoveryData::set_attempt_triggered_by,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "charge_id",
                &app::model::payments::PaymentAttemptRevenueRecoveryData::get_charge_id,
                &app::model::payments::PaymentAttemptRevenueRecoveryData::set_charge_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "customer_present",
                &app::model::payments::PaymentAttemptRevenueRecoveryData::get_customer_present,
                &app::model::payments::PaymentAttemptRevenueRecoveryData::set_customer_present,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "setup_future_usage",
                &app::model::payments::PaymentAttemptRevenueRecoveryData::get_setup_future_usage,
                &app::model::payments::PaymentAttemptRevenueRecoveryData::set_setup_future_usage,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "recovery_request_id",
                &app::model::payments::PaymentAttemptRevenueRecoveryData::get_recovery_request_id,
                &app::model::payments::PaymentAttemptRevenueRecoveryData::set_recovery_request_id,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "payment_attempt_revenue_recovery_data";
    }
};

template<>
struct serde::Serializable<app::model::payments::PaymentsRedirectRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "payment_id",
                &app::model::payments::PaymentsRedirectRequest::get_payment_id,
                &app::model::payments::PaymentsRedirectRequest::set_payment_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "merchant_id",
                &app::model::payments::PaymentsRedirectRequest::get_merchant_id,
                &app::model::payments::PaymentsRedirectRequest::set_merchant_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "connector",
                &app::model::payments::PaymentsRedirectRequest::get_connector,
                &app::model::payments::PaymentsRedirectRequest::set_connector,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "param",
                &app::model::payments::PaymentsRedirectRequest::get_param,
                &app::model::payments::PaymentsRedirectRequest::set_param,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "payments_redirect_request";
    }
};

template<>
struct serde::Serializable<app::model::payments::Card>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "card_number",
                &app::model::payments::Card::get_card_number,
                &app::model::payments::Card::set_card_number,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_exp_month",
                &app::model::payments::Card::get_card_exp_month,
                &app::model::payments::Card::set_card_exp_month,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_exp_year",
                &app::model::payments::Card::get_card_exp_year,
                &app::model::payments::Card::set_card_exp_year,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_holder_name",
                &app::model::payments::Card::get_card_holder_name,
                &app::model::payments::Card::set_card_holder_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_cvc",
                &app::model::payments::Card::get_card_cvc,
                &app::model::payments::Card::set_card_cvc,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_issuer",
                &app::model::payments::Card::get_card_issuer,
                &app::model::payments::Card::set_card_issuer,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_network",
                &app::model::payments::Card::get_card_network,
                &app::model::payments::Card::set_card_network,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_type",
                &app::model::payments::Card::get_card_type,
                &app::model::payments::Card::set_card_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_subtype",
                &app::model::payments::Card::get_card_subtype,
                &app::model::payments::Card::set_card_subtype,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_segment_type",
                &app::model::payments::Card::get_card_segment_type,
                &app::model::payments::Card::set_card_segment_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "funding_source",
                &app::model::payments::Card::get_funding_source,
                &app::model::payments::Card::set_funding_source,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_issuing_country",
                &app::model::payments::Card::get_card_issuing_country,
                &app::model::payments::Card::set_card_issuing_country,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_issuing_country_code",
                &app::model::payments::Card::get_card_issuing_country_code,
                &app::model::payments::Card::set_card_issuing_country_code,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "bank_code",
                &app::model::payments::Card::get_bank_code,
                &app::model::payments::Card::set_bank_code,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "nick_name",
                &app::model::payments::Card::get_nick_name,
                &app::model::payments::Card::set_nick_name,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "card";
    }
};

template<>
struct serde::Serializable<app::model::payments::CardWithNoCVC>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "card_number",
                &app::model::payments::CardWithNoCVC::get_card_number,
                &app::model::payments::CardWithNoCVC::set_card_number,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_exp_month",
                &app::model::payments::CardWithNoCVC::get_card_exp_month,
                &app::model::payments::CardWithNoCVC::set_card_exp_month,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_exp_year",
                &app::model::payments::CardWithNoCVC::get_card_exp_year,
                &app::model::payments::CardWithNoCVC::set_card_exp_year,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_holder_name",
                &app::model::payments::CardWithNoCVC::get_card_holder_name,
                &app::model::payments::CardWithNoCVC::set_card_holder_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_issuer",
                &app::model::payments::CardWithNoCVC::get_card_issuer,
                &app::model::payments::CardWithNoCVC::set_card_issuer,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_network",
                &app::model::payments::CardWithNoCVC::get_card_network,
                &app::model::payments::CardWithNoCVC::set_card_network,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_type",
                &app::model::payments::CardWithNoCVC::get_card_type,
                &app::model::payments::CardWithNoCVC::set_card_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_subtype",
                &app::model::payments::CardWithNoCVC::get_card_subtype,
                &app::model::payments::CardWithNoCVC::set_card_subtype,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_segment_type",
                &app::model::payments::CardWithNoCVC::get_card_segment_type,
                &app::model::payments::CardWithNoCVC::set_card_segment_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "funding_source",
                &app::model::payments::CardWithNoCVC::get_funding_source,
                &app::model::payments::CardWithNoCVC::set_funding_source,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_issuing_country",
                &app::model::payments::CardWithNoCVC::get_card_issuing_country,
                &app::model::payments::CardWithNoCVC::set_card_issuing_country,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_issuing_country_code",
                &app::model::payments::CardWithNoCVC::get_card_issuing_country_code,
                &app::model::payments::CardWithNoCVC::set_card_issuing_country_code,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "bank_code",
                &app::model::payments::CardWithNoCVC::get_bank_code,
                &app::model::payments::CardWithNoCVC::set_bank_code,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "nick_name",
                &app::model::payments::CardWithNoCVC::get_nick_name,
                &app::model::payments::CardWithNoCVC::set_nick_name,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "card_with_no_cvc";
    }
};

template<>
struct serde::Serializable<app::model::payments::CardToken>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "card_holder_name",
                &app::model::payments::CardToken::get_card_holder_name,
                &app::model::payments::CardToken::set_card_holder_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_cvc",
                &app::model::payments::CardToken::get_card_cvc,
                &app::model::payments::CardToken::set_card_cvc,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_cvc_token",
                &app::model::payments::CardToken::get_card_cvc_token,
                &app::model::payments::CardToken::set_card_cvc_token,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "card_token";
    }
};

template<>
struct serde::Serializable<app::model::payments::VaultCardToken>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "card_holder_name",
                &app::model::payments::VaultCardToken::get_card_holder_name,
                &app::model::payments::VaultCardToken::set_card_holder_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_cvc",
                &app::model::payments::VaultCardToken::get_card_cvc,
                &app::model::payments::VaultCardToken::set_card_cvc,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "vault_card_token";
    }
};

template<>
struct serde::Serializable<app::model::payments::PaymentMethodDataRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "payment_method_data",
                &app::model::payments::PaymentMethodDataRequest::get_payment_method_data,
                &app::model::payments::PaymentMethodDataRequest::set_payment_method_data,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "billing",
                &app::model::payments::PaymentMethodDataRequest::get_billing,
                &app::model::payments::PaymentMethodDataRequest::set_billing,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "payment_method_data_request";
    }
};

template<>
struct serde::Serializable<app::model::payments::SplitPaymentMethodDataRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "payment_method_data",
                &app::model::payments::SplitPaymentMethodDataRequest::get_payment_method_data,
                &app::model::payments::SplitPaymentMethodDataRequest::set_payment_method_data,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_method_type",
                &app::model::payments::SplitPaymentMethodDataRequest::get_payment_method_type,
                &app::model::payments::SplitPaymentMethodDataRequest::set_payment_method_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_method_subtype",
                &app::model::payments::SplitPaymentMethodDataRequest::get_payment_method_subtype,
                &app::model::payments::SplitPaymentMethodDataRequest::set_payment_method_subtype,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "split_payment_method_data_request";
    }
};

template<>
struct serde::Serializable<app::model::payments::RecordAttemptPaymentMethodDataRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "payment_method_data",
                &app::model::payments::RecordAttemptPaymentMethodDataRequest::
                    get_payment_method_data,
                &app::model::payments::RecordAttemptPaymentMethodDataRequest::
                    set_payment_method_data,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "billing",
                &app::model::payments::RecordAttemptPaymentMethodDataRequest::get_billing,
                &app::model::payments::RecordAttemptPaymentMethodDataRequest::set_billing,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "record_attempt_payment_method_data_request";
    }
};

template<>
struct serde::Serializable<app::model::payments::ProxyPaymentMethodDataRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "payment_method_data",
                &app::model::payments::ProxyPaymentMethodDataRequest::get_payment_method_data,
                &app::model::payments::ProxyPaymentMethodDataRequest::set_payment_method_data,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "billing",
                &app::model::payments::ProxyPaymentMethodDataRequest::get_billing,
                &app::model::payments::ProxyPaymentMethodDataRequest::set_billing,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "proxy_payment_method_data_request";
    }
};

template<>
struct serde::Serializable<app::model::payments::VaultToken>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "card_cvc",
                &app::model::payments::VaultToken::get_card_cvc,
                &app::model::payments::VaultToken::set_card_cvc,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_holder_name",
                &app::model::payments::VaultToken::get_card_holder_name,
                &app::model::payments::VaultToken::set_card_holder_name,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "vault_token";
    }
};

template<>
struct serde::Serializable<app::model::payments::ApplyPaymentMethodDataRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<
            "payment_methods",
            &app::model::payments::ApplyPaymentMethodDataRequest::get_payment_methods,
            &app::model::payments::ApplyPaymentMethodDataRequest::set_payment_methods,
            serde::FieldOptions{}>{}};
    }

    static constexpr std::string_view table_name()
    {
        return "apply_payment_method_data_request";
    }
};

template<>
struct serde::Serializable<app::model::payments::EligibilityBalanceCheckResponseItem>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "payment_method_data",
                &app::model::payments::EligibilityBalanceCheckResponseItem::get_payment_method_data,
                &app::model::payments::EligibilityBalanceCheckResponseItem::set_payment_method_data,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "eligibility",
                &app::model::payments::EligibilityBalanceCheckResponseItem::get_eligibility,
                &app::model::payments::EligibilityBalanceCheckResponseItem::set_eligibility,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "eligibility_balance_check_response_item";
    }
};

template<>
struct serde::Serializable<app::model::payments::CheckAndApplyPaymentMethodDataResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "balances",
                &app::model::payments::CheckAndApplyPaymentMethodDataResponse::get_balances,
                &app::model::payments::CheckAndApplyPaymentMethodDataResponse::set_balances,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "remaining_amount",
                &app::model::payments::CheckAndApplyPaymentMethodDataResponse::get_remaining_amount,
                &app::model::payments::CheckAndApplyPaymentMethodDataResponse::set_remaining_amount,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "currency",
                &app::model::payments::CheckAndApplyPaymentMethodDataResponse::get_currency,
                &app::model::payments::CheckAndApplyPaymentMethodDataResponse::set_currency,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "requires_additional_pm_data",
                &app::model::payments::CheckAndApplyPaymentMethodDataResponse::
                    get_requires_additional_pm_data,
                &app::model::payments::CheckAndApplyPaymentMethodDataResponse::
                    set_requires_additional_pm_data,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "surcharge_details",
                &app::model::payments::CheckAndApplyPaymentMethodDataResponse::
                    get_surcharge_details,
                &app::model::payments::CheckAndApplyPaymentMethodDataResponse::
                    set_surcharge_details,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "check_and_apply_payment_method_data_response";
    }
};

template<>
struct serde::Serializable<app::model::payments::ApplyPaymentMethodDataSurchargeResponseItem>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "payment_method_type",
                &app::model::payments::ApplyPaymentMethodDataSurchargeResponseItem::
                    get_payment_method_type,
                &app::model::payments::ApplyPaymentMethodDataSurchargeResponseItem::
                    set_payment_method_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_method_subtype",
                &app::model::payments::ApplyPaymentMethodDataSurchargeResponseItem::
                    get_payment_method_subtype,
                &app::model::payments::ApplyPaymentMethodDataSurchargeResponseItem::
                    set_payment_method_subtype,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "surcharge_details",
                &app::model::payments::ApplyPaymentMethodDataSurchargeResponseItem::
                    get_surcharge_details,
                &app::model::payments::ApplyPaymentMethodDataSurchargeResponseItem::
                    set_surcharge_details,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "apply_payment_method_data_surcharge_response_item";
    }
};

template<>
struct serde::Serializable<app::model::payments::BHNGiftCardDetails>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "account_number",
                &app::model::payments::BHNGiftCardDetails::get_account_number,
                &app::model::payments::BHNGiftCardDetails::set_account_number,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "pin",
                &app::model::payments::BHNGiftCardDetails::get_pin,
                &app::model::payments::BHNGiftCardDetails::set_pin,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "cvv2",
                &app::model::payments::BHNGiftCardDetails::get_cvv2,
                &app::model::payments::BHNGiftCardDetails::set_cvv2,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "expiration_date",
                &app::model::payments::BHNGiftCardDetails::get_expiration_date,
                &app::model::payments::BHNGiftCardDetails::set_expiration_date,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "bhn_gift_card_details";
    }
};

template<>
struct serde::Serializable<app::model::payments::GiftCardDetails>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "number",
                &app::model::payments::GiftCardDetails::get_number,
                &app::model::payments::GiftCardDetails::set_number,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "cvc",
                &app::model::payments::GiftCardDetails::get_cvc,
                &app::model::payments::GiftCardDetails::set_cvc,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "gift_card_details";
    }
};

template<>
struct serde::Serializable<app::model::payments::AdditionalCardInfo>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "card_issuer",
                &app::model::payments::AdditionalCardInfo::get_card_issuer,
                &app::model::payments::AdditionalCardInfo::set_card_issuer,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_network",
                &app::model::payments::AdditionalCardInfo::get_card_network,
                &app::model::payments::AdditionalCardInfo::set_card_network,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_type",
                &app::model::payments::AdditionalCardInfo::get_card_type,
                &app::model::payments::AdditionalCardInfo::set_card_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_subtype",
                &app::model::payments::AdditionalCardInfo::get_card_subtype,
                &app::model::payments::AdditionalCardInfo::set_card_subtype,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_segment_type",
                &app::model::payments::AdditionalCardInfo::get_card_segment_type,
                &app::model::payments::AdditionalCardInfo::set_card_segment_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "funding_source",
                &app::model::payments::AdditionalCardInfo::get_funding_source,
                &app::model::payments::AdditionalCardInfo::set_funding_source,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_issuing_country",
                &app::model::payments::AdditionalCardInfo::get_card_issuing_country,
                &app::model::payments::AdditionalCardInfo::set_card_issuing_country,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_issuing_country_code",
                &app::model::payments::AdditionalCardInfo::get_card_issuing_country_code,
                &app::model::payments::AdditionalCardInfo::set_card_issuing_country_code,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "bank_code",
                &app::model::payments::AdditionalCardInfo::get_bank_code,
                &app::model::payments::AdditionalCardInfo::set_bank_code,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "last4",
                &app::model::payments::AdditionalCardInfo::get_last4,
                &app::model::payments::AdditionalCardInfo::set_last4,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_isin",
                &app::model::payments::AdditionalCardInfo::get_card_isin,
                &app::model::payments::AdditionalCardInfo::set_card_isin,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_extended_bin",
                &app::model::payments::AdditionalCardInfo::get_card_extended_bin,
                &app::model::payments::AdditionalCardInfo::set_card_extended_bin,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_exp_month",
                &app::model::payments::AdditionalCardInfo::get_card_exp_month,
                &app::model::payments::AdditionalCardInfo::set_card_exp_month,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_exp_year",
                &app::model::payments::AdditionalCardInfo::get_card_exp_year,
                &app::model::payments::AdditionalCardInfo::set_card_exp_year,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_holder_name",
                &app::model::payments::AdditionalCardInfo::get_card_holder_name,
                &app::model::payments::AdditionalCardInfo::set_card_holder_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_checks",
                &app::model::payments::AdditionalCardInfo::get_payment_checks,
                &app::model::payments::AdditionalCardInfo::set_payment_checks,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "authentication_data",
                &app::model::payments::AdditionalCardInfo::get_authentication_data,
                &app::model::payments::AdditionalCardInfo::set_authentication_data,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "is_regulated",
                &app::model::payments::AdditionalCardInfo::get_is_regulated,
                &app::model::payments::AdditionalCardInfo::set_is_regulated,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "signature_network",
                &app::model::payments::AdditionalCardInfo::get_signature_network,
                &app::model::payments::AdditionalCardInfo::set_signature_network,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "auth_code",
                &app::model::payments::AdditionalCardInfo::get_auth_code,
                &app::model::payments::AdditionalCardInfo::set_auth_code,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "additional_card_info";
    }
};

template<>
struct serde::Serializable<app::model::payments::InteracPaymentMethod>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<
            "customer_info",
            &app::model::payments::InteracPaymentMethod::get_customer_info,
            &app::model::payments::InteracPaymentMethod::set_customer_info,
            serde::FieldOptions{}>{}};
    }

    static constexpr std::string_view table_name()
    {
        return "interac_payment_method";
    }
};

template<>
struct serde::Serializable<app::model::payments::BankRedirectBilling>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "billing_name",
                &app::model::payments::BankRedirectBilling::get_billing_name,
                &app::model::payments::BankRedirectBilling::set_billing_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "email",
                &app::model::payments::BankRedirectBilling::get_email,
                &app::model::payments::BankRedirectBilling::set_email,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "bank_redirect_billing";
    }
};

template<>
struct serde::Serializable<app::model::payments::BankDebitBilling>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "name",
                &app::model::payments::BankDebitBilling::get_name,
                &app::model::payments::BankDebitBilling::set_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "email",
                &app::model::payments::BankDebitBilling::get_email,
                &app::model::payments::BankDebitBilling::set_email,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "address",
                &app::model::payments::BankDebitBilling::get_address,
                &app::model::payments::BankDebitBilling::set_address,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "bank_debit_billing";
    }
};

template<>
struct serde::Serializable<app::model::payments::WeChatPayRedirection>
{
    static constexpr auto fields()
    {
        return std::tuple{};
    }

    static constexpr std::string_view table_name()
    {
        return "we_chat_pay_redirection";
    }
};

template<>
struct serde::Serializable<app::model::payments::WeChatPayQr>
{
    static constexpr auto fields()
    {
        return std::tuple{};
    }

    static constexpr std::string_view table_name()
    {
        return "we_chat_pay_qr";
    }
};

template<>
struct serde::Serializable<app::model::payments::CashappQr>
{
    static constexpr auto fields()
    {
        return std::tuple{};
    }

    static constexpr std::string_view table_name()
    {
        return "cashapp_qr";
    }
};

template<>
struct serde::Serializable<app::model::payments::AliPayQr>
{
    static constexpr auto fields()
    {
        return std::tuple{};
    }

    static constexpr std::string_view table_name()
    {
        return "ali_pay_qr";
    }
};

template<>
struct serde::Serializable<app::model::payments::MomoRedirection>
{
    static constexpr auto fields()
    {
        return std::tuple{};
    }

    static constexpr std::string_view table_name()
    {
        return "momo_redirection";
    }
};

template<>
struct serde::Serializable<app::model::payments::KakaoPayRedirection>
{
    static constexpr auto fields()
    {
        return std::tuple{};
    }

    static constexpr std::string_view table_name()
    {
        return "kakao_pay_redirection";
    }
};

template<>
struct serde::Serializable<app::model::payments::GoPayRedirection>
{
    static constexpr auto fields()
    {
        return std::tuple{};
    }

    static constexpr std::string_view table_name()
    {
        return "go_pay_redirection";
    }
};

template<>
struct serde::Serializable<app::model::payments::GcashRedirection>
{
    static constexpr auto fields()
    {
        return std::tuple{};
    }

    static constexpr std::string_view table_name()
    {
        return "gcash_redirection";
    }
};

template<>
struct serde::Serializable<app::model::payments::MobilePayRedirection>
{
    static constexpr auto fields()
    {
        return std::tuple{};
    }

    static constexpr std::string_view table_name()
    {
        return "mobile_pay_redirection";
    }
};

template<>
struct serde::Serializable<app::model::payments::MbWayRedirection>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<
            "telephone_number",
            &app::model::payments::MbWayRedirection::get_telephone_number,
            &app::model::payments::MbWayRedirection::set_telephone_number,
            serde::FieldOptions{}>{}};
    }

    static constexpr std::string_view table_name()
    {
        return "mb_way_redirection";
    }
};

template<>
struct serde::Serializable<app::model::payments::TouchNGoRedirection>
{
    static constexpr auto fields()
    {
        return std::tuple{};
    }

    static constexpr std::string_view table_name()
    {
        return "touch_n_go_redirection";
    }
};

template<>
struct serde::Serializable<app::model::payments::BankDebitResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<
            "details",
            &app::model::payments::BankDebitResponse::get_details,
            &app::model::payments::BankDebitResponse::set_details,
            serde::FieldOptions{}>{}};
    }

    static constexpr std::string_view table_name()
    {
        return "bank_debit_response";
    }
};

template<>
struct serde::Serializable<app::model::payments::BankRedirectResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "bank_name",
                &app::model::payments::BankRedirectResponse::get_bank_name,
                &app::model::payments::BankRedirectResponse::set_bank_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "details",
                &app::model::payments::BankRedirectResponse::get_details,
                &app::model::payments::BankRedirectResponse::set_details,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "interac",
                &app::model::payments::BankRedirectResponse::get_interac,
                &app::model::payments::BankRedirectResponse::set_interac,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "bank_redirect_response";
    }
};

template<>
struct serde::Serializable<app::model::payments::BankTransferResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<
            "details",
            &app::model::payments::BankTransferResponse::get_details,
            &app::model::payments::BankTransferResponse::set_details,
            serde::FieldOptions{}>{}};
    }

    static constexpr std::string_view table_name()
    {
        return "bank_transfer_response";
    }
};

template<>
struct serde::Serializable<app::model::payments::CardRedirectResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<
            "details",
            &app::model::payments::CardRedirectResponse::get_details,
            &app::model::payments::CardRedirectResponse::set_details,
            serde::FieldOptions{}>{}};
    }

    static constexpr std::string_view table_name()
    {
        return "card_redirect_response";
    }
};

template<>
struct serde::Serializable<app::model::payments::CardTokenResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<
            "details",
            &app::model::payments::CardTokenResponse::get_details,
            &app::model::payments::CardTokenResponse::set_details,
            serde::FieldOptions{}>{}};
    }

    static constexpr std::string_view table_name()
    {
        return "card_token_response";
    }
};

template<>
struct serde::Serializable<app::model::payments::GiftCardResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<
            "details",
            &app::model::payments::GiftCardResponse::get_details,
            &app::model::payments::GiftCardResponse::set_details,
            serde::FieldOptions{}>{}};
    }

    static constexpr std::string_view table_name()
    {
        return "gift_card_response";
    }
};

template<>
struct serde::Serializable<app::model::payments::PaymentMethodDataResponseWithBilling>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "payment_method_data",
                &app::model::payments::PaymentMethodDataResponseWithBilling::
                    get_payment_method_data,
                &app::model::payments::PaymentMethodDataResponseWithBilling::
                    set_payment_method_data,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "billing",
                &app::model::payments::PaymentMethodDataResponseWithBilling::get_billing,
                &app::model::payments::PaymentMethodDataResponseWithBilling::set_billing,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "payment_method_data_response_with_billing";
    }
};

template<>
struct serde::Serializable<app::model::payments::CustomRecoveryPaymentMethodData>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "primary_processor_payment_method_token",
                &app::model::payments::CustomRecoveryPaymentMethodData::
                    get_primary_processor_payment_method_token,
                &app::model::payments::CustomRecoveryPaymentMethodData::
                    set_primary_processor_payment_method_token,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_method_metadata",
                &app::model::payments::CustomRecoveryPaymentMethodData::get_payment_method_metadata,
                &app::model::payments::CustomRecoveryPaymentMethodData::set_payment_method_metadata,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "custom_recovery_payment_method_data";
    }
};

template<>
struct serde::Serializable<app::model::payments::Address>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "address",
                &app::model::payments::Address::get_address,
                &app::model::payments::Address::set_address,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "phone",
                &app::model::payments::Address::get_phone,
                &app::model::payments::Address::set_phone,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "email",
                &app::model::payments::Address::get_email,
                &app::model::payments::Address::set_email,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "address";
    }
};

template<>
struct serde::Serializable<app::model::payments::AddressDetails>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "city",
                &app::model::payments::AddressDetails::get_city,
                &app::model::payments::AddressDetails::set_city,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "country",
                &app::model::payments::AddressDetails::get_country,
                &app::model::payments::AddressDetails::set_country,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "line1",
                &app::model::payments::AddressDetails::get_line1,
                &app::model::payments::AddressDetails::set_line1,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "line2",
                &app::model::payments::AddressDetails::get_line2,
                &app::model::payments::AddressDetails::set_line2,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "line3",
                &app::model::payments::AddressDetails::get_line3,
                &app::model::payments::AddressDetails::set_line3,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "zip",
                &app::model::payments::AddressDetails::get_zip,
                &app::model::payments::AddressDetails::set_zip,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "state",
                &app::model::payments::AddressDetails::get_state,
                &app::model::payments::AddressDetails::set_state,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "first_name",
                &app::model::payments::AddressDetails::get_first_name,
                &app::model::payments::AddressDetails::set_first_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "last_name",
                &app::model::payments::AddressDetails::get_last_name,
                &app::model::payments::AddressDetails::set_last_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "origin_zip",
                &app::model::payments::AddressDetails::get_origin_zip,
                &app::model::payments::AddressDetails::set_origin_zip,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "address_details";
    }
};

template<>
struct serde::Serializable<app::model::payments::FetchQrCodeInformation>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<
            "qr_code_fetch_url",
            &app::model::payments::FetchQrCodeInformation::get_qr_code_fetch_url,
            &app::model::payments::FetchQrCodeInformation::set_qr_code_fetch_url,
            serde::FieldOptions{}>{}};
    }

    static constexpr std::string_view table_name()
    {
        return "fetch_qr_code_information";
    }
};

template<>
struct serde::Serializable<app::model::payments::BankTransferNextStepsData>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "bank_transfer_instructions",
                &app::model::payments::BankTransferNextStepsData::get_bank_transfer_instructions,
                &app::model::payments::BankTransferNextStepsData::set_bank_transfer_instructions,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "receiver",
                &app::model::payments::BankTransferNextStepsData::get_receiver,
                &app::model::payments::BankTransferNextStepsData::set_receiver,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "bank_transfer_next_steps_data";
    }
};

template<>
struct serde::Serializable<app::model::payments::QrCodeNextStepsInstruction>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "image_data_url",
                &app::model::payments::QrCodeNextStepsInstruction::get_image_data_url,
                &app::model::payments::QrCodeNextStepsInstruction::set_image_data_url,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "display_to_timestamp",
                &app::model::payments::QrCodeNextStepsInstruction::get_display_to_timestamp,
                &app::model::payments::QrCodeNextStepsInstruction::set_display_to_timestamp,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "qr_code_url",
                &app::model::payments::QrCodeNextStepsInstruction::get_qr_code_url,
                &app::model::payments::QrCodeNextStepsInstruction::set_qr_code_url,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "qr_code_next_steps_instruction";
    }
};

template<>
struct serde::Serializable<app::model::payments::PaymentsConnectorThreeDsInvokeData>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "directory_server_id",
                &app::model::payments::PaymentsConnectorThreeDsInvokeData::get_directory_server_id,
                &app::model::payments::PaymentsConnectorThreeDsInvokeData::set_directory_server_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "three_ds_method_url",
                &app::model::payments::PaymentsConnectorThreeDsInvokeData::get_three_ds_method_url,
                &app::model::payments::PaymentsConnectorThreeDsInvokeData::set_three_ds_method_url,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "three_ds_method_data",
                &app::model::payments::PaymentsConnectorThreeDsInvokeData::get_three_ds_method_data,
                &app::model::payments::PaymentsConnectorThreeDsInvokeData::set_three_ds_method_data,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "message_version",
                &app::model::payments::PaymentsConnectorThreeDsInvokeData::get_message_version,
                &app::model::payments::PaymentsConnectorThreeDsInvokeData::set_message_version,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "three_ds_method_data_submission",
                &app::model::payments::PaymentsConnectorThreeDsInvokeData::
                    get_three_ds_method_data_submission,
                &app::model::payments::PaymentsConnectorThreeDsInvokeData::
                    set_three_ds_method_data_submission,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "payments_connector_three_ds_invoke_data";
    }
};

template<>
struct serde::Serializable<app::model::payments::PaymentConnectorInvokeDDCMetadata>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "iframe_url",
                &app::model::payments::PaymentConnectorInvokeDDCMetadata::get_iframe_url,
                &app::model::payments::PaymentConnectorInvokeDDCMetadata::set_iframe_url,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "timeout_ms",
                &app::model::payments::PaymentConnectorInvokeDDCMetadata::get_timeout_ms,
                &app::model::payments::PaymentConnectorInvokeDDCMetadata::set_timeout_ms,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "payment_connector_invoke_ddc_metadata";
    }
};

template<>
struct serde::Serializable<app::model::payments::PaymentConnectorInvokeDDCData>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "iframe_url",
                &app::model::payments::PaymentConnectorInvokeDDCData::get_iframe_url,
                &app::model::payments::PaymentConnectorInvokeDDCData::set_iframe_url,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "timeout_ms",
                &app::model::payments::PaymentConnectorInvokeDDCData::get_timeout_ms,
                &app::model::payments::PaymentConnectorInvokeDDCData::set_timeout_ms,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "payment_connector_invoke_ddc_data";
    }
};

template<>
struct serde::Serializable<app::model::payments::DokuBankTransferInstructions>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "expires_at",
                &app::model::payments::DokuBankTransferInstructions::get_expires_at,
                &app::model::payments::DokuBankTransferInstructions::set_expires_at,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "reference",
                &app::model::payments::DokuBankTransferInstructions::get_reference,
                &app::model::payments::DokuBankTransferInstructions::set_reference,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "instructions_url",
                &app::model::payments::DokuBankTransferInstructions::get_instructions_url,
                &app::model::payments::DokuBankTransferInstructions::set_instructions_url,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "doku_bank_transfer_instructions";
    }
};

template<>
struct serde::Serializable<app::model::payments::ReceiverDetails>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "amount_received",
                &app::model::payments::ReceiverDetails::get_amount_received,
                &app::model::payments::ReceiverDetails::set_amount_received,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "amount_charged",
                &app::model::payments::ReceiverDetails::get_amount_charged,
                &app::model::payments::ReceiverDetails::set_amount_charged,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "amount_remaining",
                &app::model::payments::ReceiverDetails::get_amount_remaining,
                &app::model::payments::ReceiverDetails::set_amount_remaining,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "receiver_details";
    }
};

template<>
struct serde::Serializable<app::model::payments::ApiConnectorErrorDetails>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "code",
                &app::model::payments::ApiConnectorErrorDetails::get_code,
                &app::model::payments::ApiConnectorErrorDetails::set_code,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "message",
                &app::model::payments::ApiConnectorErrorDetails::get_message,
                &app::model::payments::ApiConnectorErrorDetails::set_message,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "reason",
                &app::model::payments::ApiConnectorErrorDetails::get_reason,
                &app::model::payments::ApiConnectorErrorDetails::set_reason,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "api_connector_error_details";
    }
};

template<>
struct serde::Serializable<app::model::payments::ConnectorTokenDetails>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "token",
                &app::model::payments::ConnectorTokenDetails::get_token,
                &app::model::payments::ConnectorTokenDetails::set_token,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "connector_token_request_reference_id",
                &app::model::payments::ConnectorTokenDetails::
                    get_connector_token_request_reference_id,
                &app::model::payments::ConnectorTokenDetails::
                    set_connector_token_request_reference_id,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "connector_token_details";
    }
};

template<>
struct serde::Serializable<app::model::payments::PaymentStartRedirectionRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<
            "id",
            &app::model::payments::PaymentStartRedirectionRequest::get_id,
            &app::model::payments::PaymentStartRedirectionRequest::set_id,
            serde::FieldOptions{}>{}};
    }

    static constexpr std::string_view table_name()
    {
        return "payment_start_redirection_request";
    }
};

template<>
struct serde::Serializable<app::model::payments::PaymentStartRedirectionParams>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "publishable_key",
                &app::model::payments::PaymentStartRedirectionParams::get_publishable_key,
                &app::model::payments::PaymentStartRedirectionParams::set_publishable_key,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "profile_id",
                &app::model::payments::PaymentStartRedirectionParams::get_profile_id,
                &app::model::payments::PaymentStartRedirectionParams::set_profile_id,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "payment_start_redirection_params";
    }
};

template<>
struct serde::Serializable<app::model::payments::PaymentsRedirectionResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<
            "redirect_url",
            &app::model::payments::PaymentsRedirectionResponse::get_redirect_url,
            &app::model::payments::PaymentsRedirectionResponse::set_redirect_url,
            serde::FieldOptions{}>{}};
    }

    static constexpr std::string_view table_name()
    {
        return "payments_redirection_response";
    }
};

template<>
struct serde::Serializable<app::model::payments::PgRedirectResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "payment_id",
                &app::model::payments::PgRedirectResponse::get_payment_id,
                &app::model::payments::PgRedirectResponse::set_payment_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "status",
                &app::model::payments::PgRedirectResponse::get_status,
                &app::model::payments::PgRedirectResponse::set_status,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "gateway_id",
                &app::model::payments::PgRedirectResponse::get_gateway_id,
                &app::model::payments::PgRedirectResponse::set_gateway_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "customer_id",
                &app::model::payments::PgRedirectResponse::get_customer_id,
                &app::model::payments::PgRedirectResponse::set_customer_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "amount",
                &app::model::payments::PgRedirectResponse::get_amount,
                &app::model::payments::PgRedirectResponse::set_amount,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "pg_redirect_response";
    }
};

template<>
struct serde::Serializable<app::model::payments::RedirectionResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "return_url",
                &app::model::payments::RedirectionResponse::get_return_url,
                &app::model::payments::RedirectionResponse::set_return_url,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "params",
                &app::model::payments::RedirectionResponse::get_params,
                &app::model::payments::RedirectionResponse::set_params,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "return_url_with_query_params",
                &app::model::payments::RedirectionResponse::get_return_url_with_query_params,
                &app::model::payments::RedirectionResponse::set_return_url_with_query_params,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "http_method",
                &app::model::payments::RedirectionResponse::get_http_method,
                &app::model::payments::RedirectionResponse::set_http_method,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "headers",
                &app::model::payments::RedirectionResponse::get_headers,
                &app::model::payments::RedirectionResponse::set_headers,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "redirection_response";
    }
};

template<>
struct serde::Serializable<app::model::payments::RedirectResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "param",
                &app::model::payments::RedirectResponse::get_param,
                &app::model::payments::RedirectResponse::set_param,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "json_payload",
                &app::model::payments::RedirectResponse::get_json_payload,
                &app::model::payments::RedirectResponse::set_json_payload,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "redirect_response";
    }
};

template<>
struct serde::Serializable<app::model::payments::PaymentsUpdateMetadataRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "payment_id",
                &app::model::payments::PaymentsUpdateMetadataRequest::get_payment_id,
                &app::model::payments::PaymentsUpdateMetadataRequest::set_payment_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "metadata",
                &app::model::payments::PaymentsUpdateMetadataRequest::get_metadata,
                &app::model::payments::PaymentsUpdateMetadataRequest::set_metadata,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "feature_metadata",
                &app::model::payments::PaymentsUpdateMetadataRequest::get_feature_metadata,
                &app::model::payments::PaymentsUpdateMetadataRequest::set_feature_metadata,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "payments_update_metadata_request";
    }
};

template<>
struct serde::Serializable<app::model::payments::PaymentsUpdateMetadataResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "payment_id",
                &app::model::payments::PaymentsUpdateMetadataResponse::get_payment_id,
                &app::model::payments::PaymentsUpdateMetadataResponse::set_payment_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "metadata",
                &app::model::payments::PaymentsUpdateMetadataResponse::get_metadata,
                &app::model::payments::PaymentsUpdateMetadataResponse::set_metadata,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "status",
                &app::model::payments::PaymentsUpdateMetadataResponse::get_status,
                &app::model::payments::PaymentsUpdateMetadataResponse::set_status,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "feature_metadata",
                &app::model::payments::PaymentsUpdateMetadataResponse::get_feature_metadata,
                &app::model::payments::PaymentsUpdateMetadataResponse::set_feature_metadata,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "payments_update_metadata_response";
    }
};

template<>
struct serde::Serializable<app::model::payments::ConnectorMetadata>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "apple_pay",
                &app::model::payments::ConnectorMetadata::get_apple_pay,
                &app::model::payments::ConnectorMetadata::set_apple_pay,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "airwallex",
                &app::model::payments::ConnectorMetadata::get_airwallex,
                &app::model::payments::ConnectorMetadata::set_airwallex,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "noon",
                &app::model::payments::ConnectorMetadata::get_noon,
                &app::model::payments::ConnectorMetadata::set_noon,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "braintree",
                &app::model::payments::ConnectorMetadata::get_braintree,
                &app::model::payments::ConnectorMetadata::set_braintree,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "adyen",
                &app::model::payments::ConnectorMetadata::get_adyen,
                &app::model::payments::ConnectorMetadata::set_adyen,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "peachpayments",
                &app::model::payments::ConnectorMetadata::get_peachpayments,
                &app::model::payments::ConnectorMetadata::set_peachpayments,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "santander",
                &app::model::payments::ConnectorMetadata::get_santander,
                &app::model::payments::ConnectorMetadata::set_santander,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "paypal",
                &app::model::payments::ConnectorMetadata::get_paypal,
                &app::model::payments::ConnectorMetadata::set_paypal,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "worldpayxml",
                &app::model::payments::ConnectorMetadata::get_worldpayxml,
                &app::model::payments::ConnectorMetadata::set_worldpayxml,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "checkout",
                &app::model::payments::ConnectorMetadata::get_checkout,
                &app::model::payments::ConnectorMetadata::set_checkout,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "stripe",
                &app::model::payments::ConnectorMetadata::get_stripe,
                &app::model::payments::ConnectorMetadata::set_stripe,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "connector_metadata";
    }
};

template<>
struct serde::Serializable<app::model::payments::PaypalConnectorMetadata>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<
            "paypal_client_metadata_id",
            &app::model::payments::PaypalConnectorMetadata::get_paypal_client_metadata_id,
            &app::model::payments::PaypalConnectorMetadata::set_paypal_client_metadata_id,
            serde::FieldOptions{}>{}};
    }

    static constexpr std::string_view table_name()
    {
        return "paypal_connector_metadata";
    }
};

template<>
struct serde::Serializable<app::model::payments::StripeConnectorMetadata>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<
            "error_on_requires_action",
            &app::model::payments::StripeConnectorMetadata::get_error_on_requires_action,
            &app::model::payments::StripeConnectorMetadata::set_error_on_requires_action,
            serde::FieldOptions{}>{}};
    }

    static constexpr std::string_view table_name()
    {
        return "stripe_connector_metadata";
    }
};

template<>
struct serde::Serializable<app::model::payments::CheckoutData>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<
            "purpose_of_payment",
            &app::model::payments::CheckoutData::get_purpose_of_payment,
            &app::model::payments::CheckoutData::set_purpose_of_payment,
            serde::FieldOptions{}>{}};
    }

    static constexpr std::string_view table_name()
    {
        return "checkout_data";
    }
};

template<>
struct serde::Serializable<app::model::payments::WorldpayxmlData>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "funding_transaction_type",
                &app::model::payments::WorldpayxmlData::get_funding_transaction_type,
                &app::model::payments::WorldpayxmlData::set_funding_transaction_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_purpose",
                &app::model::payments::WorldpayxmlData::get_payment_purpose,
                &app::model::payments::WorldpayxmlData::set_payment_purpose,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "worldpayxml_data";
    }
};

template<>
struct serde::Serializable<app::model::payments::AirwallexData>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<
            "payload",
            &app::model::payments::AirwallexData::get_payload,
            &app::model::payments::AirwallexData::set_payload,
            serde::FieldOptions{}>{}};
    }

    static constexpr std::string_view table_name()
    {
        return "airwallex_data";
    }
};

template<>
struct serde::Serializable<app::model::payments::NoonData>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<
            "order_category",
            &app::model::payments::NoonData::get_order_category,
            &app::model::payments::NoonData::set_order_category,
            serde::FieldOptions{}>{}};
    }

    static constexpr std::string_view table_name()
    {
        return "noon_data";
    }
};

template<>
struct serde::Serializable<app::model::payments::BraintreeData>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "merchant_account_id",
                &app::model::payments::BraintreeData::get_merchant_account_id,
                &app::model::payments::BraintreeData::set_merchant_account_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "merchant_config_currency",
                &app::model::payments::BraintreeData::get_merchant_config_currency,
                &app::model::payments::BraintreeData::set_merchant_config_currency,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "braintree_data";
    }
};

template<>
struct serde::Serializable<app::model::payments::MultiCurrencyProcessingData>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "currency",
                &app::model::payments::MultiCurrencyProcessingData::get_currency,
                &app::model::payments::MultiCurrencyProcessingData::set_currency,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "amount",
                &app::model::payments::MultiCurrencyProcessingData::get_amount,
                &app::model::payments::MultiCurrencyProcessingData::set_amount,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "conversion_rate",
                &app::model::payments::MultiCurrencyProcessingData::get_conversion_rate,
                &app::model::payments::MultiCurrencyProcessingData::set_conversion_rate,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "date",
                &app::model::payments::MultiCurrencyProcessingData::get_date,
                &app::model::payments::MultiCurrencyProcessingData::set_date,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "processing_reference_id",
                &app::model::payments::MultiCurrencyProcessingData::get_processing_reference_id,
                &app::model::payments::MultiCurrencyProcessingData::set_processing_reference_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "user_id",
                &app::model::payments::MultiCurrencyProcessingData::get_user_id,
                &app::model::payments::MultiCurrencyProcessingData::set_user_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "conversion_provider",
                &app::model::payments::MultiCurrencyProcessingData::get_conversion_provider,
                &app::model::payments::MultiCurrencyProcessingData::set_conversion_provider,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "conversion_reason_code",
                &app::model::payments::MultiCurrencyProcessingData::get_conversion_reason_code,
                &app::model::payments::MultiCurrencyProcessingData::set_conversion_reason_code,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "multi_currency_processing_data";
    }
};

template<>
struct serde::Serializable<app::model::payments::AdyenConnectorMetadata>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<
            "testing",
            &app::model::payments::AdyenConnectorMetadata::get_testing,
            &app::model::payments::AdyenConnectorMetadata::set_testing,
            serde::FieldOptions{}>{}};
    }

    static constexpr std::string_view table_name()
    {
        return "adyen_connector_metadata";
    }
};

template<>
struct serde::Serializable<app::model::payments::AdyenTestingData>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<
            "holder_name",
            &app::model::payments::AdyenTestingData::get_holder_name,
            &app::model::payments::AdyenTestingData::set_holder_name,
            serde::FieldOptions{}>{}};
    }

    static constexpr std::string_view table_name()
    {
        return "adyen_testing_data";
    }
};

template<>
struct serde::Serializable<app::model::payments::ApplepayConnectorMetadataRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<
            "session_token_data",
            &app::model::payments::ApplepayConnectorMetadataRequest::get_session_token_data,
            &app::model::payments::ApplepayConnectorMetadataRequest::set_session_token_data,
            serde::FieldOptions{}>{}};
    }

    static constexpr std::string_view table_name()
    {
        return "applepay_connector_metadata_request";
    }
};

template<>
struct serde::Serializable<app::model::payments::PeachpaymentsData>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "rrn",
                &app::model::payments::PeachpaymentsData::get_rrn,
                &app::model::payments::PeachpaymentsData::set_rrn,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_on_file_transaction_type",
                &app::model::payments::PeachpaymentsData::get_card_on_file_transaction_type,
                &app::model::payments::PeachpaymentsData::set_card_on_file_transaction_type,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "peachpayments_data";
    }
};

template<>
struct serde::Serializable<app::model::payments::PaymentRequestMetadata>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "supported_networks",
                &app::model::payments::PaymentRequestMetadata::get_supported_networks,
                &app::model::payments::PaymentRequestMetadata::set_supported_networks,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "merchant_capabilities",
                &app::model::payments::PaymentRequestMetadata::get_merchant_capabilities,
                &app::model::payments::PaymentRequestMetadata::set_merchant_capabilities,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "label",
                &app::model::payments::PaymentRequestMetadata::get_label,
                &app::model::payments::PaymentRequestMetadata::set_label,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "payment_request_metadata";
    }
};

template<>
struct serde::Serializable<app::model::payments::SessionTokenInfo>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "certificate",
                &app::model::payments::SessionTokenInfo::get_certificate,
                &app::model::payments::SessionTokenInfo::set_certificate,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "certificate_keys",
                &app::model::payments::SessionTokenInfo::get_certificate_keys,
                &app::model::payments::SessionTokenInfo::set_certificate_keys,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "merchant_identifier",
                &app::model::payments::SessionTokenInfo::get_merchant_identifier,
                &app::model::payments::SessionTokenInfo::set_merchant_identifier,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "display_name",
                &app::model::payments::SessionTokenInfo::get_display_name,
                &app::model::payments::SessionTokenInfo::set_display_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "initiative",
                &app::model::payments::SessionTokenInfo::get_initiative,
                &app::model::payments::SessionTokenInfo::set_initiative,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "initiative_context",
                &app::model::payments::SessionTokenInfo::get_initiative_context,
                &app::model::payments::SessionTokenInfo::set_initiative_context,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "merchant_business_country",
                &app::model::payments::SessionTokenInfo::get_merchant_business_country,
                &app::model::payments::SessionTokenInfo::set_merchant_business_country,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_processing_details_at",
                &app::model::payments::SessionTokenInfo::get_payment_processing_details_at,
                &app::model::payments::SessionTokenInfo::set_payment_processing_details_at,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_processing_detail_input_type",
                &app::model::payments::SessionTokenInfo::get_payment_processing_detail_input_type,
                &app::model::payments::SessionTokenInfo::set_payment_processing_detail_input_type,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "session_token_info";
    }
};

template<>
struct serde::Serializable<app::model::payments::SessionTokenForSimplifiedApplePay>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "initiative_context",
                &app::model::payments::SessionTokenForSimplifiedApplePay::get_initiative_context,
                &app::model::payments::SessionTokenForSimplifiedApplePay::set_initiative_context,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "merchant_business_country",
                &app::model::payments::SessionTokenForSimplifiedApplePay::
                    get_merchant_business_country,
                &app::model::payments::SessionTokenForSimplifiedApplePay::
                    set_merchant_business_country,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "session_token_for_simplified_apple_pay";
    }
};

template<>
struct serde::Serializable<app::model::payments::GooglePayThirdPartySdk>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "delayed_session_token",
                &app::model::payments::GooglePayThirdPartySdk::get_delayed_session_token,
                &app::model::payments::GooglePayThirdPartySdk::set_delayed_session_token,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "connector",
                &app::model::payments::GooglePayThirdPartySdk::get_connector,
                &app::model::payments::GooglePayThirdPartySdk::set_connector,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "sdk_next_action",
                &app::model::payments::GooglePayThirdPartySdk::get_sdk_next_action,
                &app::model::payments::GooglePayThirdPartySdk::set_sdk_next_action,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "google_pay_third_party_sdk";
    }
};

template<>
struct serde::Serializable<app::model::payments::SdkNextAction>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "next_action",
                &app::model::payments::SdkNextAction::get_next_action,
                &app::model::payments::SdkNextAction::set_next_action,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "should_block_confirm",
                &app::model::payments::SdkNextAction::get_should_block_confirm,
                &app::model::payments::SdkNextAction::set_should_block_confirm,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "sdk_next_action";
    }
};

template<>
struct serde::Serializable<app::model::payments::SecretInfoToInitiateSdk>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "display",
                &app::model::payments::SecretInfoToInitiateSdk::get_display,
                &app::model::payments::SecretInfoToInitiateSdk::set_display,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment",
                &app::model::payments::SecretInfoToInitiateSdk::get_payment,
                &app::model::payments::SecretInfoToInitiateSdk::set_payment,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "secret_info_to_initiate_sdk";
    }
};

template<>
struct serde::Serializable<app::model::payments::AmountInfo>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "label",
                &app::model::payments::AmountInfo::get_label,
                &app::model::payments::AmountInfo::set_label,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "type",
                &app::model::payments::AmountInfo::get_total_type,
                &app::model::payments::AmountInfo::set_total_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "amount",
                &app::model::payments::AmountInfo::get_amount,
                &app::model::payments::AmountInfo::set_amount,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "amount_info";
    }
};

template<>
struct serde::Serializable<app::model::payments::FeatureMetadata>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "redirect_response",
                &app::model::payments::FeatureMetadata::get_redirect_response,
                &app::model::payments::FeatureMetadata::set_redirect_response,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "search_tags",
                &app::model::payments::FeatureMetadata::get_search_tags,
                &app::model::payments::FeatureMetadata::set_search_tags,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "apple_pay_recurring_details",
                &app::model::payments::FeatureMetadata::get_apple_pay_recurring_details,
                &app::model::payments::FeatureMetadata::set_apple_pay_recurring_details,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "revenue_recovery",
                &app::model::payments::FeatureMetadata::get_revenue_recovery,
                &app::model::payments::FeatureMetadata::set_revenue_recovery,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "pix_additional_details",
                &app::model::payments::FeatureMetadata::get_pix_additional_details,
                &app::model::payments::FeatureMetadata::set_pix_additional_details,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "boleto_additional_details",
                &app::model::payments::FeatureMetadata::get_boleto_additional_details,
                &app::model::payments::FeatureMetadata::set_boleto_additional_details,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "pix_automatico_additional_details",
                &app::model::payments::FeatureMetadata::get_pix_automatico_additional_details,
                &app::model::payments::FeatureMetadata::set_pix_automatico_additional_details,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "finix_additional_details",
                &app::model::payments::FeatureMetadata::get_finix_additional_details,
                &app::model::payments::FeatureMetadata::set_finix_additional_details,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "multi_currency_processing",
                &app::model::payments::FeatureMetadata::get_multi_currency_processing,
                &app::model::payments::FeatureMetadata::set_multi_currency_processing,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "feature_metadata";
    }
};

template<>
struct serde::Serializable<app::model::payments::BoletoAdditionalDetails>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "due_date",
                &app::model::payments::BoletoAdditionalDetails::get_due_date,
                &app::model::payments::BoletoAdditionalDetails::set_due_date,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "document_kind",
                &app::model::payments::BoletoAdditionalDetails::get_document_kind,
                &app::model::payments::BoletoAdditionalDetails::set_document_kind,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_type",
                &app::model::payments::BoletoAdditionalDetails::get_payment_type,
                &app::model::payments::BoletoAdditionalDetails::set_payment_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "covenant_code",
                &app::model::payments::BoletoAdditionalDetails::get_covenant_code,
                &app::model::payments::BoletoAdditionalDetails::set_covenant_code,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "pix_key",
                &app::model::payments::BoletoAdditionalDetails::get_pix_key,
                &app::model::payments::BoletoAdditionalDetails::set_pix_key,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "discount_rules",
                &app::model::payments::BoletoAdditionalDetails::get_discount_rules,
                &app::model::payments::BoletoAdditionalDetails::set_discount_rules,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "penalties",
                &app::model::payments::BoletoAdditionalDetails::get_penalties,
                &app::model::payments::BoletoAdditionalDetails::set_penalties,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "collection_actions",
                &app::model::payments::BoletoAdditionalDetails::get_collection_actions,
                &app::model::payments::BoletoAdditionalDetails::set_collection_actions,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_constraints",
                &app::model::payments::BoletoAdditionalDetails::get_payment_constraints,
                &app::model::payments::BoletoAdditionalDetails::set_payment_constraints,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "beneficiary",
                &app::model::payments::BoletoAdditionalDetails::get_beneficiary,
                &app::model::payments::BoletoAdditionalDetails::set_beneficiary,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "boleto_additional_details";
    }
};

template<>
struct serde::Serializable<app::model::payments::FinixAdditionalDetails>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<
            "fraud_session_id",
            &app::model::payments::FinixAdditionalDetails::get_fraud_session_id,
            &app::model::payments::FinixAdditionalDetails::set_fraud_session_id,
            serde::FieldOptions{}>{}};
    }

    static constexpr std::string_view table_name()
    {
        return "finix_additional_details";
    }
};

template<>
struct serde::Serializable<app::model::payments::ApplePayRecurringDetails>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "payment_description",
                &app::model::payments::ApplePayRecurringDetails::get_payment_description,
                &app::model::payments::ApplePayRecurringDetails::set_payment_description,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "regular_billing",
                &app::model::payments::ApplePayRecurringDetails::get_regular_billing,
                &app::model::payments::ApplePayRecurringDetails::set_regular_billing,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "billing_agreement",
                &app::model::payments::ApplePayRecurringDetails::get_billing_agreement,
                &app::model::payments::ApplePayRecurringDetails::set_billing_agreement,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "management_url",
                &app::model::payments::ApplePayRecurringDetails::get_management_url,
                &app::model::payments::ApplePayRecurringDetails::set_management_url,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "apple_pay_recurring_details";
    }
};

template<>
struct serde::Serializable<app::model::payments::ApplePayRegularBillingDetails>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "label",
                &app::model::payments::ApplePayRegularBillingDetails::get_label,
                &app::model::payments::ApplePayRegularBillingDetails::set_label,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "recurring_payment_start_date",
                &app::model::payments::ApplePayRegularBillingDetails::
                    get_recurring_payment_start_date,
                &app::model::payments::ApplePayRegularBillingDetails::
                    set_recurring_payment_start_date,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "recurring_payment_end_date",
                &app::model::payments::ApplePayRegularBillingDetails::
                    get_recurring_payment_end_date,
                &app::model::payments::ApplePayRegularBillingDetails::
                    set_recurring_payment_end_date,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "recurring_payment_interval_unit",
                &app::model::payments::ApplePayRegularBillingDetails::
                    get_recurring_payment_interval_unit,
                &app::model::payments::ApplePayRegularBillingDetails::
                    set_recurring_payment_interval_unit,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "recurring_payment_interval_count",
                &app::model::payments::ApplePayRegularBillingDetails::
                    get_recurring_payment_interval_count,
                &app::model::payments::ApplePayRegularBillingDetails::
                    set_recurring_payment_interval_count,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "apple_pay_regular_billing_details";
    }
};

template<>
struct serde::Serializable<app::model::payments::EligibilityPaymentMethodDataRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "payment_method_data",
                &app::model::payments::EligibilityPaymentMethodDataRequest::get_payment_method_data,
                &app::model::payments::EligibilityPaymentMethodDataRequest::set_payment_method_data,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "billing",
                &app::model::payments::EligibilityPaymentMethodDataRequest::get_billing,
                &app::model::payments::EligibilityPaymentMethodDataRequest::set_billing,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "eligibility_payment_method_data_request";
    }
};

template<>
struct serde::Serializable<app::model::payments::PaymentRevenueRecoveryMetadata>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "total_retry_count",
                &app::model::payments::PaymentRevenueRecoveryMetadata::get_total_retry_count,
                &app::model::payments::PaymentRevenueRecoveryMetadata::set_total_retry_count,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_connector_transmission",
                &app::model::payments::PaymentRevenueRecoveryMetadata::
                    get_payment_connector_transmission,
                &app::model::payments::PaymentRevenueRecoveryMetadata::
                    set_payment_connector_transmission,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "billing_connector_id",
                &app::model::payments::PaymentRevenueRecoveryMetadata::get_billing_connector_id,
                &app::model::payments::PaymentRevenueRecoveryMetadata::set_billing_connector_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "active_attempt_payment_connector_id",
                &app::model::payments::PaymentRevenueRecoveryMetadata::
                    get_active_attempt_payment_connector_id,
                &app::model::payments::PaymentRevenueRecoveryMetadata::
                    set_active_attempt_payment_connector_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "billing_connector_payment_details",
                &app::model::payments::PaymentRevenueRecoveryMetadata::
                    get_billing_connector_payment_details,
                &app::model::payments::PaymentRevenueRecoveryMetadata::
                    set_billing_connector_payment_details,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_method_type",
                &app::model::payments::PaymentRevenueRecoveryMetadata::get_payment_method_type,
                &app::model::payments::PaymentRevenueRecoveryMetadata::set_payment_method_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_method_subtype",
                &app::model::payments::PaymentRevenueRecoveryMetadata::get_payment_method_subtype,
                &app::model::payments::PaymentRevenueRecoveryMetadata::set_payment_method_subtype,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "connector",
                &app::model::payments::PaymentRevenueRecoveryMetadata::get_connector,
                &app::model::payments::PaymentRevenueRecoveryMetadata::set_connector,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "billing_connector_payment_method_details",
                &app::model::payments::PaymentRevenueRecoveryMetadata::
                    get_billing_connector_payment_method_details,
                &app::model::payments::PaymentRevenueRecoveryMetadata::
                    set_billing_connector_payment_method_details,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "invoice_next_billing_time",
                &app::model::payments::PaymentRevenueRecoveryMetadata::
                    get_invoice_next_billing_time,
                &app::model::payments::PaymentRevenueRecoveryMetadata::
                    set_invoice_next_billing_time,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "invoice_billing_started_at_time",
                &app::model::payments::PaymentRevenueRecoveryMetadata::
                    get_invoice_billing_started_at_time,
                &app::model::payments::PaymentRevenueRecoveryMetadata::
                    set_invoice_billing_started_at_time,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "first_payment_attempt_pg_error_code",
                &app::model::payments::PaymentRevenueRecoveryMetadata::
                    get_first_payment_attempt_pg_error_code,
                &app::model::payments::PaymentRevenueRecoveryMetadata::
                    set_first_payment_attempt_pg_error_code,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "first_payment_attempt_network_decline_code",
                &app::model::payments::PaymentRevenueRecoveryMetadata::
                    get_first_payment_attempt_network_decline_code,
                &app::model::payments::PaymentRevenueRecoveryMetadata::
                    set_first_payment_attempt_network_decline_code,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "first_payment_attempt_network_advice_code",
                &app::model::payments::PaymentRevenueRecoveryMetadata::
                    get_first_payment_attempt_network_advice_code,
                &app::model::payments::PaymentRevenueRecoveryMetadata::
                    set_first_payment_attempt_network_advice_code,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "revenue_recovery_ab_routing",
                &app::model::payments::PaymentRevenueRecoveryMetadata::
                    get_revenue_recovery_ab_routing,
                &app::model::payments::PaymentRevenueRecoveryMetadata::
                    set_revenue_recovery_ab_routing,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "external_trigger_state",
                &app::model::payments::PaymentRevenueRecoveryMetadata::get_external_trigger_state,
                &app::model::payments::PaymentRevenueRecoveryMetadata::set_external_trigger_state,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "last_failure_code",
                &app::model::payments::PaymentRevenueRecoveryMetadata::get_last_failure_code,
                &app::model::payments::PaymentRevenueRecoveryMetadata::set_last_failure_code,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "payment_revenue_recovery_metadata";
    }
};

template<>
struct serde::Serializable<app::model::payments::BillingConnectorAdditionalCardInfo>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "card_network",
                &app::model::payments::BillingConnectorAdditionalCardInfo::get_card_network,
                &app::model::payments::BillingConnectorAdditionalCardInfo::set_card_network,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_issuer",
                &app::model::payments::BillingConnectorAdditionalCardInfo::get_card_issuer,
                &app::model::payments::BillingConnectorAdditionalCardInfo::set_card_issuer,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_type",
                &app::model::payments::BillingConnectorAdditionalCardInfo::get_card_type,
                &app::model::payments::BillingConnectorAdditionalCardInfo::set_card_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_issuing_country",
                &app::model::payments::BillingConnectorAdditionalCardInfo::get_card_issuing_country,
                &app::model::payments::BillingConnectorAdditionalCardInfo::set_card_issuing_country,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_isin",
                &app::model::payments::BillingConnectorAdditionalCardInfo::get_card_isin,
                &app::model::payments::BillingConnectorAdditionalCardInfo::set_card_isin,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "billing_connector_additional_card_info";
    }
};

template<>
struct serde::Serializable<app::model::payments::BillingConnectorPaymentDetails>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "payment_processor_token",
                &app::model::payments::BillingConnectorPaymentDetails::get_payment_processor_token,
                &app::model::payments::BillingConnectorPaymentDetails::set_payment_processor_token,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "connector_customer_id",
                &app::model::payments::BillingConnectorPaymentDetails::get_connector_customer_id,
                &app::model::payments::BillingConnectorPaymentDetails::set_connector_customer_id,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "billing_connector_payment_details";
    }
};

template<>
struct serde::Serializable<app::model::payments::SantanderConnectorMetadataData>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<
            "boleto",
            &app::model::payments::SantanderConnectorMetadataData::get_boleto,
            &app::model::payments::SantanderConnectorMetadataData::set_boleto,
            serde::FieldOptions{}>{}};
    }

    static constexpr std::string_view table_name()
    {
        return "santander_connector_metadata_data";
    }
};

template<>
struct serde::Serializable<app::model::payments::SantanderBoletoData>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "discount_rules",
                &app::model::payments::SantanderBoletoData::get_discount_rules,
                &app::model::payments::SantanderBoletoData::set_discount_rules,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "penalties",
                &app::model::payments::SantanderBoletoData::get_penalties,
                &app::model::payments::SantanderBoletoData::set_penalties,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "collection_actions",
                &app::model::payments::SantanderBoletoData::get_collection_actions,
                &app::model::payments::SantanderBoletoData::set_collection_actions,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_constraints",
                &app::model::payments::SantanderBoletoData::get_payment_constraints,
                &app::model::payments::SantanderBoletoData::set_payment_constraints,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "beneficiary",
                &app::model::payments::SantanderBoletoData::get_beneficiary,
                &app::model::payments::SantanderBoletoData::set_beneficiary,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "document_kind",
                &app::model::payments::SantanderBoletoData::get_document_kind,
                &app::model::payments::SantanderBoletoData::set_document_kind,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "santander_boleto_data";
    }
};

template<>
struct serde::Serializable<app::model::payments::BeneficiaryDetails>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "name",
                &app::model::payments::BeneficiaryDetails::get_name,
                &app::model::payments::BeneficiaryDetails::set_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "document_number",
                &app::model::payments::BeneficiaryDetails::get_document_number,
                &app::model::payments::BeneficiaryDetails::set_document_number,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "document_type",
                &app::model::payments::BeneficiaryDetails::get_document_type,
                &app::model::payments::BeneficiaryDetails::set_document_type,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "beneficiary_details";
    }
};

template<>
struct serde::Serializable<app::model::payments::PenaltyRules>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "fixed_penalty",
                &app::model::payments::PenaltyRules::get_fixed_penalty,
                &app::model::payments::PenaltyRules::set_fixed_penalty,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "interest",
                &app::model::payments::PenaltyRules::get_interest,
                &app::model::payments::PenaltyRules::set_interest,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "penalty_rules";
    }
};

template<>
struct serde::Serializable<app::model::payments::InterestDetail>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "interest_percentage",
                &app::model::payments::InterestDetail::get_interest_percentage,
                &app::model::payments::InterestDetail::set_interest_percentage,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "iof_percentage",
                &app::model::payments::InterestDetail::get_iof_percentage,
                &app::model::payments::InterestDetail::set_iof_percentage,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "interest_detail";
    }
};

template<>
struct serde::Serializable<app::model::payments::PenaltyDetail>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "value",
                &app::model::payments::PenaltyDetail::get_value,
                &app::model::payments::PenaltyDetail::set_value,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "grace_period_days",
                &app::model::payments::PenaltyDetail::get_grace_period_days,
                &app::model::payments::PenaltyDetail::set_grace_period_days,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "penalty_detail";
    }
};

template<>
struct serde::Serializable<app::model::payments::CollectionActions>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "legal_protest",
                &app::model::payments::CollectionActions::get_legal_protest,
                &app::model::payments::CollectionActions::set_legal_protest,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "auto_write_off_days",
                &app::model::payments::CollectionActions::get_auto_write_off_days,
                &app::model::payments::CollectionActions::set_auto_write_off_days,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "collection_actions";
    }
};

template<>
struct serde::Serializable<app::model::payments::SantanderPaymentDiscountRules>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "discount_type",
                &app::model::payments::SantanderPaymentDiscountRules::get_discount_type,
                &app::model::payments::SantanderPaymentDiscountRules::set_discount_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "tiers",
                &app::model::payments::SantanderPaymentDiscountRules::get_tiers,
                &app::model::payments::SantanderPaymentDiscountRules::set_tiers,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "santander_payment_discount_rules";
    }
};

template<>
struct serde::Serializable<app::model::payments::DiscountTier>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "amount",
                &app::model::payments::DiscountTier::get_amount,
                &app::model::payments::DiscountTier::set_amount,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "end_date",
                &app::model::payments::DiscountTier::get_end_date,
                &app::model::payments::DiscountTier::set_end_date,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "discount_tier";
    }
};

template<>
struct serde::Serializable<app::model::payments::ProtestRules>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "protest_type",
                &app::model::payments::ProtestRules::get_protest_type,
                &app::model::payments::ProtestRules::set_protest_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "days_after_due_date",
                &app::model::payments::ProtestRules::get_days_after_due_date,
                &app::model::payments::ProtestRules::set_days_after_due_date,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "protest_rules";
    }
};
