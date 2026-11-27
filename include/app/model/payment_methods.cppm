module;
export module app_model:payment_methods;

import core_serde;
import core_behavior;

import std;
import core_serde;
import core_behavior;
import app_connector;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace app::model::payment_methods {

class AcceptedCountriesValue : public core::behavior::Serializer<AcceptedCountriesValue>
{
public:
    void set_value(std::string val)
    {
        value = std::move(val);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

class AcceptedCurrenciesValue : public core::behavior::Serializer<AcceptedCurrenciesValue>
{
public:
    void set_value(std::string val)
    {
        value = std::move(val);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

class AcknowledgementStatusValue : public core::behavior::Serializer<AcknowledgementStatusValue>
{
public:
    void set_value(std::string val)
    {
        value = std::move(val);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

class AddressValue : public core::behavior::Serializer<AddressValue>
{
public:
    void set_value(std::string val)
    {
        value = std::move(val);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

class AssociatedPaymentMethodsValue :
    public core::behavior::Serializer<AssociatedPaymentMethodsValue>
{
public:
    void set_value(std::string val)
    {
        value = std::move(val);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

class BankHolderTypeValue : public core::behavior::Serializer<BankHolderTypeValue>
{
public:
    void set_value(std::string val)
    {
        value = std::move(val);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

class BankTypeValue : public core::behavior::Serializer<BankTypeValue>
{
public:
    void set_value(std::string val)
    {
        value = std::move(val);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

class BankValue : public core::behavior::Serializer<BankValue>
{
public:
    void set_value(std::string val)
    {
        value = std::move(val);
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
    void set_value(std::string val)
    {
        value = std::move(val);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

class CaptureMethodValue : public core::behavior::Serializer<CaptureMethodValue>
{
public:
    void set_value(std::string val)
    {
        value = std::move(val);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};


enum class CardType
{
    Credit,
    Debit
};

class CoBadgedCardNetworksValue : public core::behavior::Serializer<CoBadgedCardNetworksValue>
{
public:
    void set_value(std::string val)
    {
        value = std::move(val);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

class CountryValue : public core::behavior::Serializer<CountryValue>
{
public:
    void set_value(std::string val)
    {
        value = std::move(val);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

class CustomerAcceptanceSupportValue :
    public core::behavior::Serializer<CustomerAcceptanceSupportValue>
{
public:
    void set_value(std::string val)
    {
        value = std::move(val);
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
    void set_value(std::string val)
    {
        value = std::move(val);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

class CustomerDetailsValue : public core::behavior::Serializer<CustomerDetailsValue>
{
public:
    void set_value(std::string val)
    {
        value = std::move(val);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

class DeleteTokenizeByDateRequest : public core::behavior::Serializer<DeleteTokenizeByDateRequest>
{
public:
    void set_buffer_minutes(std::int32_t value)
    {
        buffer_minutes = std::move(value);
    }

    [[nodiscard]] const std::int32_t& get_buffer_minutes() const noexcept
    {
        return buffer_minutes;
    }

    void set_service_name(std::string value)
    {
        service_name = std::move(value);
    }

    [[nodiscard]] const std::string& get_service_name() const noexcept
    {
        return service_name;
    }

    void set_max_rows(std::int32_t value)
    {
        max_rows = std::move(value);
    }

    [[nodiscard]] const std::int32_t& get_max_rows() const noexcept
    {
        return max_rows;
    }

    std::int32_t buffer_minutes{};
    std::string service_name{};
    std::int32_t max_rows{};
};

class DeleteTokenizeByTokenRequest : public core::behavior::Serializer<DeleteTokenizeByTokenRequest>
{
public:
    void set_lookup_key(std::string value)
    {
        lookup_key = std::move(value);
    }

    [[nodiscard]] const std::string& get_lookup_key() const noexcept
    {
        return lookup_key;
    }

    void set_service_name(std::string value)
    {
        service_name = std::move(value);
    }

    [[nodiscard]] const std::string& get_service_name() const noexcept
    {
        return service_name;
    }

    std::string lookup_key{};
    std::string service_name{};
};

class EnabledPaymentMethodValue : public core::behavior::Serializer<EnabledPaymentMethodValue>
{
public:
    void set_value(std::string val)
    {
        value = std::move(val);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

class ErrorDetailsValue : public core::behavior::Serializer<ErrorDetailsValue>
{
public:
    void set_value(std::string val)
    {
        value = std::move(val);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

class ExternalVaultTokenData : public core::behavior::Serializer<ExternalVaultTokenData>
{
public:
    void set_tokenized_card_number(std::string value)
    {
        tokenized_card_number = std::move(value);
    }

    [[nodiscard]] const std::string& get_tokenized_card_number() const noexcept
    {
        return tokenized_card_number;
    }

    std::string tokenized_card_number{};
};

class FieldTypeValue : public core::behavior::Serializer<FieldTypeValue>
{
public:
    void set_value(std::string val)
    {
        value = std::move(val);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

class FloatMajorUnitValue : public core::behavior::Serializer<FloatMajorUnitValue>
{
public:
    void set_value(std::string val)
    {
        value = std::move(val);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

class FutureUsageValue : public core::behavior::Serializer<FutureUsageValue>
{
public:
    void set_value(std::string val)
    {
        value = std::move(val);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

class GenericLinkUiConfigFormDataValue :
    public core::behavior::Serializer<GenericLinkUiConfigFormDataValue>
{
public:
    void set_value(std::string val)
    {
        value = std::move(val);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

class GenericLinkUiConfigValue : public core::behavior::Serializer<GenericLinkUiConfigValue>
{
public:
    void set_value(std::string val)
    {
        value = std::move(val);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

class GetNetworkTokenEiligibilityResponse :
    public core::behavior::Serializer<GetNetworkTokenEiligibilityResponse>
{
public:
    void set_eligible_for_network_tokenization(bool value)
    {
        eligible_for_network_tokenization = std::move(value);
    }

    [[nodiscard]] const bool& get_eligible_for_network_tokenization() const noexcept
    {
        return eligible_for_network_tokenization;
    }

    bool eligible_for_network_tokenization{};
};

class GetTokenizePayloadRequest : public core::behavior::Serializer<GetTokenizePayloadRequest>
{
public:
    void set_lookup_key(std::string value)
    {
        lookup_key = std::move(value);
    }

    [[nodiscard]] const std::string& get_lookup_key() const noexcept
    {
        return lookup_key;
    }

    void set_service_name(std::string value)
    {
        service_name = std::move(value);
    }

    [[nodiscard]] const std::string& get_service_name() const noexcept
    {
        return service_name;
    }

    void set_get_value2(bool value)
    {
        get_value2 = std::move(value);
    }

    [[nodiscard]] const bool& get_get_value2() const noexcept
    {
        return get_value2;
    }

    std::string lookup_key{};
    std::string service_name{};
    bool get_value2{};
};

class GetTokenizePayloadResponse : public core::behavior::Serializer<GetTokenizePayloadResponse>
{
public:
    void set_lookup_key(std::string value)
    {
        lookup_key = std::move(value);
    }

    [[nodiscard]] const std::string& get_lookup_key() const noexcept
    {
        return lookup_key;
    }

    void set_get_value2(std::optional<bool> value)
    {
        get_value2 = std::move(value);
    }

    [[nodiscard]] const std::optional<bool>& get_get_value2() const noexcept
    {
        return get_value2;
    }

    std::string lookup_key{};
    std::optional<bool> get_value2{};
};

class GlobalCustomerIdValue : public core::behavior::Serializer<GlobalCustomerIdValue>
{
public:
    void set_value(std::string val)
    {
        value = std::move(val);
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
    void set_value(std::string val)
    {
        value = std::move(val);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

class GlobalPaymentMethodSessionIdValue :
    public core::behavior::Serializer<GlobalPaymentMethodSessionIdValue>
{
public:
    void set_value(std::string val)
    {
        value = std::move(val);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

class GlobalTokenIdValue : public core::behavior::Serializer<GlobalTokenIdValue>
{
public:
    void set_value(std::string val)
    {
        value = std::move(val);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

class InstallmentInterestRateValue : public core::behavior::Serializer<InstallmentInterestRateValue>
{
public:
    void set_value(std::string val)
    {
        value = std::move(val);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

class IntentStatusValue : public core::behavior::Serializer<IntentStatusValue>
{
public:
    void set_value(std::string val)
    {
        value = std::move(val);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

class MandateDataValue : public core::behavior::Serializer<MandateDataValue>
{
public:
    void set_value(std::string val)
    {
        value = std::move(val);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

class MandateTypeValue : public core::behavior::Serializer<MandateTypeValue>
{
public:
    void set_value(std::string val)
    {
        value = std::move(val);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

class MaskedBankDetails : public core::behavior::Serializer<MaskedBankDetails>
{
public:
    void set_mask(std::string value)
    {
        mask = std::move(value);
    }

    [[nodiscard]] const std::string& get_mask() const noexcept
    {
        return mask;
    }

    void set_account_holder_name(std::optional<std::string> value)
    {
        account_holder_name = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_account_holder_name() const noexcept
    {
        return account_holder_name;
    }

    void set_bank_name(std::optional<std::string> value)
    {
        bank_name = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_bank_name() const noexcept
    {
        return bank_name;
    }

    std::string mask{};
    std::optional<std::string> account_holder_name{};
    std::optional<std::string> bank_name{};
};

class MerchantIdValue : public core::behavior::Serializer<MerchantIdValue>
{
public:
    void set_value(std::string val)
    {
        value = std::move(val);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

enum class MigrationStatus
{
    Success,
    Failed
};

class NetworkTokenEligibilityRequest :
    public core::behavior::Serializer<NetworkTokenEligibilityRequest>
{
public:
    void set_card_bin(std::string value)
    {
        card_bin = std::move(value);
    }

    [[nodiscard]] const std::string& get_card_bin() const noexcept
    {
        return card_bin;
    }

    std::string card_bin{};
};

class NetworkTokenStatusCheckFailureResponse :
    public core::behavior::Serializer<NetworkTokenStatusCheckFailureResponse>
{
public:
    void set_error_message(std::string value)
    {
        error_message = std::move(value);
    }

    [[nodiscard]] const std::string& get_error_message() const noexcept
    {
        return error_message;
    }

    std::string error_message{};
};

enum class NetworkTokenStatusCheckResponse
{
    SuccessResponse,
    FailureResponse
};

class NetworkTokenValue : public core::behavior::Serializer<NetworkTokenValue>
{
public:
    void set_value(std::string val)
    {
        value = std::move(val);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

class NetworkTokenizationValue : public core::behavior::Serializer<NetworkTokenizationValue>
{
public:
    void set_value(std::string val)
    {
        value = std::move(val);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

class NextActionDataValue : public core::behavior::Serializer<NextActionDataValue>
{
public:
    void set_value(std::string val)
    {
        value = std::move(val);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

class NonZeroU8Value : public core::behavior::Serializer<NonZeroU8Value>
{
public:
    void set_value(std::string val)
    {
        value = std::move(val);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

class OptionalEncryptableNameValue : public core::behavior::Serializer<OptionalEncryptableNameValue>
{
public:
    void set_value(std::string val)
    {
        value = std::move(val);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

class PayViaSingleUseTokenValue : public core::behavior::Serializer<PayViaSingleUseTokenValue>
{
public:
    void set_value(std::string val)
    {
        value = std::move(val);
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
    void set_value(std::string val)
    {
        value = std::move(val);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

class PaymentIdValue : public core::behavior::Serializer<PaymentIdValue>
{
public:
    void set_value(std::string val)
    {
        value = std::move(val);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

class PaymentMethodCollectStatusValue :
    public core::behavior::Serializer<PaymentMethodCollectStatusValue>
{
public:
    void set_value(std::string val)
    {
        value = std::move(val);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

class PaymentMethodDeleteResponse : public core::behavior::Serializer<PaymentMethodDeleteResponse>
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

    void set_deleted(bool value)
    {
        deleted = std::move(value);
    }

    [[nodiscard]] const bool& get_deleted() const noexcept
    {
        return deleted;
    }

    std::string payment_method_id{};
    bool deleted{};
};

class PaymentMethodDeleteSessionResponse :
    public core::behavior::Serializer<PaymentMethodDeleteSessionResponse>
{
public:
    void set_payment_method_token(std::string value)
    {
        payment_method_token = std::move(value);
    }

    [[nodiscard]] const std::string& get_payment_method_token() const noexcept
    {
        return payment_method_token;
    }

    std::string payment_method_token{};
};

class PaymentMethodId : public core::behavior::Serializer<PaymentMethodId>
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

    std::string payment_method_id{};
};

class PaymentMethodIssuerCodeValue : public core::behavior::Serializer<PaymentMethodIssuerCodeValue>
{
public:
    void set_value(std::string val)
    {
        value = std::move(val);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

enum class PaymentMethodListResult
{
    Success,
    Failed
};

class PaymentMethodRetrieveRequest : public core::behavior::Serializer<PaymentMethodRetrieveRequest>
{
public:
    void set_fetch_raw_detail(bool value)
    {
        fetch_raw_detail = std::move(value);
    }

    [[nodiscard]] const bool& get_fetch_raw_detail() const noexcept
    {
        return fetch_raw_detail;
    }

    void set_force_sync(bool value)
    {
        force_sync = std::move(value);
    }

    [[nodiscard]] const bool& get_force_sync() const noexcept
    {
        return force_sync;
    }

    bool fetch_raw_detail{};
    bool force_sync{};
};

class PaymentMethodSessionDeleteSavedPaymentMethod :
    public core::behavior::Serializer<PaymentMethodSessionDeleteSavedPaymentMethod>
{
public:
    void set_payment_method_token(std::string value)
    {
        payment_method_token = std::move(value);
    }

    [[nodiscard]] const std::string& get_payment_method_token() const noexcept
    {
        return payment_method_token;
    }

    std::string payment_method_token{};
};

class PaymentMethodStatusValue : public core::behavior::Serializer<PaymentMethodStatusValue>
{
public:
    void set_value(std::string val)
    {
        value = std::move(val);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

class PaymentMethodsBatchRecord : public core::behavior::Serializer<PaymentMethodsBatchRecord>
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

    void set_line_number(std::optional<std::int64_t> value)
    {
        line_number = std::move(value);
    }

    [[nodiscard]] const std::optional<std::int64_t>& get_line_number() const noexcept
    {
        return line_number;
    }

    std::string payment_method_id{};
    std::optional<std::int64_t> line_number{};
};

class PaymentTypeValue : public core::behavior::Serializer<PaymentTypeValue>
{
public:
    void set_value(std::string val)
    {
        value = std::move(val);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

class PayoutsMandateReferenceRecord :
    public core::behavior::Serializer<PayoutsMandateReferenceRecord>
{
public:
    void set_transfer_method_id(std::optional<std::string> value)
    {
        transfer_method_id = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_transfer_method_id() const noexcept
    {
        return transfer_method_id;
    }

    std::optional<std::string> transfer_method_id{};
};

class PrimitiveDateTimeValue : public core::behavior::Serializer<PrimitiveDateTimeValue>
{
public:
    void set_value(std::string val)
    {
        value = std::move(val);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

class ProfileIdValue : public core::behavior::Serializer<ProfileIdValue>
{
public:
    void set_value(std::string val)
    {
        value = std::move(val);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

class RawProxyCardDataResponse : public core::behavior::Serializer<RawProxyCardDataResponse>
{
public:
    void set_card_number(std::string value)
    {
        card_number = std::move(value);
    }

    [[nodiscard]] const std::string& get_card_number() const noexcept
    {
        return card_number;
    }

    void set_card_exp_year(std::optional<std::string> value)
    {
        card_exp_year = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_card_exp_year() const noexcept
    {
        return card_exp_year;
    }

    void set_card_exp_month(std::optional<std::string> value)
    {
        card_exp_month = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_card_exp_month() const noexcept
    {
        return card_exp_month;
    }

    std::string card_number{};
    std::optional<std::string> card_exp_year{};
    std::optional<std::string> card_exp_month{};
};

class SdkNextActionValue : public core::behavior::Serializer<SdkNextActionValue>
{
public:
    void set_value(std::string val)
    {
        value = std::move(val);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

class StorageTypeValue : public core::behavior::Serializer<StorageTypeValue>
{
public:
    void set_value(std::string val)
    {
        value = std::move(val);
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
    void set_value(std::string val)
    {
        value = std::move(val);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

class SurchargePercentage : public core::behavior::Serializer<SurchargePercentage>
{
public:
    void set_percentage(double value)
    {
        percentage = std::move(value);
    }

    [[nodiscard]] const double& get_percentage() const noexcept
    {
        return percentage;
    }

    double percentage{};
};

enum class SurchargeResponse
{
    Fixed,
    Rate
};

class TokenDataTypeValue : public core::behavior::Serializer<TokenDataTypeValue>
{
public:
    void set_value(std::string val)
    {
        value = std::move(val);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

enum class TokenDetailsResponse
{
    NetworkTokenDetails
};

class TokenStatusValue : public core::behavior::Serializer<TokenStatusValue>
{
public:
    void set_value(std::string val)
    {
        value = std::move(val);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

class TokenizePayloadEncrypted : public core::behavior::Serializer<TokenizePayloadEncrypted>
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

    void set_key_id(std::string value)
    {
        key_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_key_id() const noexcept
    {
        return key_id;
    }

    void set_version(std::optional<std::string> value)
    {
        version = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_version() const noexcept
    {
        return version;
    }

    std::string payload{};
    std::string key_id{};
    std::optional<std::string> version{};
};

class TokenizePayloadRequest : public core::behavior::Serializer<TokenizePayloadRequest>
{
public:
    void set_value1(std::string value)
    {
        value1 = std::move(value);
    }

    [[nodiscard]] const std::string& get_value1() const noexcept
    {
        return value1;
    }

    void set_value2(std::string value)
    {
        value2 = std::move(value);
    }

    [[nodiscard]] const std::string& get_value2() const noexcept
    {
        return value2;
    }

    void set_lookup_key(std::string value)
    {
        lookup_key = std::move(value);
    }

    [[nodiscard]] const std::string& get_lookup_key() const noexcept
    {
        return lookup_key;
    }

    void set_service_name(std::string value)
    {
        service_name = std::move(value);
    }

    [[nodiscard]] const std::string& get_service_name() const noexcept
    {
        return service_name;
    }

    std::string value1{};
    std::string value2{};
    std::string lookup_key{};
    std::string service_name{};
};

class TokenizePaymentMethodRequest : public core::behavior::Serializer<TokenizePaymentMethodRequest>
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

    void set_card_cvc(std::optional<std::string> value)
    {
        card_cvc = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_card_cvc() const noexcept
    {
        return card_cvc;
    }

    std::string payment_method_id{};
    std::optional<std::string> card_cvc{};
};

class TotalPaymentMethodCountResponse :
    public core::behavior::Serializer<TotalPaymentMethodCountResponse>
{
public:
    void set_total_count(std::int64_t value)
    {
        total_count = std::move(value);
    }

    [[nodiscard]] const std::int64_t& get_total_count() const noexcept
    {
        return total_count;
    }

    std::int64_t total_count{};
};

enum class UpdateStatus
{
    Success,
    Failed
};

class UrlValue : public core::behavior::Serializer<UrlValue>
{
public:
    void set_value(std::string val)
    {
        value = std::move(val);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

class VaultSessionDetailsValue : public core::behavior::Serializer<VaultSessionDetailsValue>
{
public:
    void set_value(std::string val)
    {
        value = std::move(val);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};


enum class WalletDetail
{
    ApplePayDecryptedData,
    GooglePayDecryptedData
};

enum class WalletDetailUpdate
{
    PayPal
};

class WalletValue : public core::behavior::Serializer<WalletValue>
{
public:
    void set_value(std::string val)
    {
        value = std::move(val);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

class AuthenticationDetails : public core::behavior::Serializer<AuthenticationDetails>
{
public:
    void set_status(IntentStatusValue value)
    {
        status = std::move(value);
    }

    [[nodiscard]] const IntentStatusValue& get_status() const noexcept
    {
        return status;
    }

    void set_error(std::optional<ErrorDetailsValue> value)
    {
        error = std::move(value);
    }

    [[nodiscard]] const std::optional<ErrorDetailsValue>& get_error() const noexcept
    {
        return error;
    }

    IntentStatusValue status{};
    std::optional<ErrorDetailsValue> error{};
};

class CardCVCTokenStorageDetails : public core::behavior::Serializer<CardCVCTokenStorageDetails>
{
public:
    void set_is_stored(bool value)
    {
        is_stored = std::move(value);
    }

    [[nodiscard]] const bool& get_is_stored() const noexcept
    {
        return is_stored;
    }

    void set_expires_at(std::optional<PrimitiveDateTimeValue> value)
    {
        expires_at = std::move(value);
    }

    [[nodiscard]] const std::optional<PrimitiveDateTimeValue>& get_expires_at() const noexcept
    {
        return expires_at;
    }

    bool is_stored{};
    std::optional<PrimitiveDateTimeValue> expires_at{};
};

class CardDetail : public core::behavior::Serializer<CardDetail>
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

    void set_card_exp_month(std::string value)
    {
        card_exp_month = std::move(value);
    }

    [[nodiscard]] const std::string& get_card_exp_month() const noexcept
    {
        return card_exp_month;
    }

    void set_card_exp_year(std::string value)
    {
        card_exp_year = std::move(value);
    }

    [[nodiscard]] const std::string& get_card_exp_year() const noexcept
    {
        return card_exp_year;
    }

    void set_card_cvc(std::optional<std::string> value)
    {
        card_cvc = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_card_cvc() const noexcept
    {
        return card_cvc;
    }

    void set_card_holder_name(std::optional<std::string> value)
    {
        card_holder_name = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_card_holder_name() const noexcept
    {
        return card_holder_name;
    }

    void set_nick_name(std::optional<std::string> value)
    {
        nick_name = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_nick_name() const noexcept
    {
        return nick_name;
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

    CardNumberValue card_number{};
    std::string card_exp_month{};
    std::string card_exp_year{};
    std::optional<std::string> card_cvc{};
    std::optional<std::string> card_holder_name{};
    std::optional<std::string> nick_name{};
    std::optional<std::string> card_issuing_country{};
    std::optional<std::string> card_issuing_country_code{};
    std::optional<CardNetworkValue> card_network{};
    std::optional<std::string> card_issuer{};
    std::optional<std::string> card_type{};
    std::optional<std::string> card_subtype{};
    std::optional<CardSegmentTypeValue> card_segment_type{};
    std::optional<FundingSourceValue> funding_source{};
};

class CardDetailFromLocker : public core::behavior::Serializer<CardDetailFromLocker>
{
public:
    void set_scheme(std::optional<std::string> value)
    {
        scheme = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_scheme() const noexcept
    {
        return scheme;
    }

    void set_issuer_country(std::optional<std::string> value)
    {
        issuer_country = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_issuer_country() const noexcept
    {
        return issuer_country;
    }

    void set_issuer_country_code(std::optional<std::string> value)
    {
        issuer_country_code = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_issuer_country_code() const noexcept
    {
        return issuer_country_code;
    }

    void set_last4_digits(std::optional<std::string> value)
    {
        last4_digits = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_last4_digits() const noexcept
    {
        return last4_digits;
    }

    void set_card_number(std::optional<CardNumberValue> value)
    {
        card_number = std::move(value);
    }

    [[nodiscard]] const std::optional<CardNumberValue>& get_card_number() const noexcept
    {
        return card_number;
    }

    void set_expiry_month(std::optional<std::string> value)
    {
        expiry_month = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_expiry_month() const noexcept
    {
        return expiry_month;
    }

    void set_expiry_year(std::optional<std::string> value)
    {
        expiry_year = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_expiry_year() const noexcept
    {
        return expiry_year;
    }

    void set_card_token(std::optional<std::string> value)
    {
        card_token = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_card_token() const noexcept
    {
        return card_token;
    }

    void set_card_holder_name(std::optional<std::string> value)
    {
        card_holder_name = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_card_holder_name() const noexcept
    {
        return card_holder_name;
    }

    void set_card_fingerprint(std::optional<std::string> value)
    {
        card_fingerprint = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_card_fingerprint() const noexcept
    {
        return card_fingerprint;
    }

    void set_nick_name(std::optional<std::string> value)
    {
        nick_name = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_nick_name() const noexcept
    {
        return nick_name;
    }

    void set_card_network(std::optional<CardNetworkValue> value)
    {
        card_network = std::move(value);
    }

    [[nodiscard]] const std::optional<CardNetworkValue>& get_card_network() const noexcept
    {
        return card_network;
    }

    void set_card_isin(std::optional<std::string> value)
    {
        card_isin = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_card_isin() const noexcept
    {
        return card_isin;
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

    void set_saved_to_locker(bool value)
    {
        saved_to_locker = std::move(value);
    }

    [[nodiscard]] const bool& get_saved_to_locker() const noexcept
    {
        return saved_to_locker;
    }

    std::optional<std::string> scheme{};
    std::optional<std::string> issuer_country{};
    std::optional<std::string> issuer_country_code{};
    std::optional<std::string> last4_digits{};
    std::optional<CardNumberValue> card_number{};
    std::optional<std::string> expiry_month{};
    std::optional<std::string> expiry_year{};
    std::optional<std::string> card_token{};
    std::optional<std::string> card_holder_name{};
    std::optional<std::string> card_fingerprint{};
    std::optional<std::string> nick_name{};
    std::optional<CardNetworkValue> card_network{};
    std::optional<std::string> card_isin{};
    std::optional<std::string> card_issuer{};
    std::optional<std::string> card_type{};
    std::optional<std::string> card_subtype{};
    std::optional<CardSegmentTypeValue> card_segment_type{};
    std::optional<FundingSourceValue> funding_source{};
    bool saved_to_locker{};
};

class CardDetailUpdate : public core::behavior::Serializer<CardDetailUpdate>
{
public:
    void set_card_exp_month(std::optional<std::string> value)
    {
        card_exp_month = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_card_exp_month() const noexcept
    {
        return card_exp_month;
    }

    void set_card_exp_year(std::optional<std::string> value)
    {
        card_exp_year = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_card_exp_year() const noexcept
    {
        return card_exp_year;
    }

    void set_card_holder_name(std::optional<std::string> value)
    {
        card_holder_name = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_card_holder_name() const noexcept
    {
        return card_holder_name;
    }

    void set_nick_name(std::optional<std::string> value)
    {
        nick_name = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_nick_name() const noexcept
    {
        return nick_name;
    }

    void set_last4_digits(std::optional<std::string> value)
    {
        last4_digits = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_last4_digits() const noexcept
    {
        return last4_digits;
    }

    void set_card_issuer(std::optional<std::string> value)
    {
        card_issuer = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_card_issuer() const noexcept
    {
        return card_issuer;
    }

    void set_issuer_country(std::optional<std::string> value)
    {
        issuer_country = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_issuer_country() const noexcept
    {
        return issuer_country;
    }

    void set_issuer_country_code(std::optional<std::string> value)
    {
        issuer_country_code = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_issuer_country_code() const noexcept
    {
        return issuer_country_code;
    }

    void set_card_network(std::optional<CardNetworkValue> value)
    {
        card_network = std::move(value);
    }

    [[nodiscard]] const std::optional<CardNetworkValue>& get_card_network() const noexcept
    {
        return card_network;
    }

    std::optional<std::string> card_exp_month{};
    std::optional<std::string> card_exp_year{};
    std::optional<std::string> card_holder_name{};
    std::optional<std::string> nick_name{};
    std::optional<std::string> last4_digits{};
    std::optional<std::string> card_issuer{};
    std::optional<std::string> issuer_country{};
    std::optional<std::string> issuer_country_code{};
    std::optional<CardNetworkValue> card_network{};
};

class CoBadgedCardData : public core::behavior::Serializer<CoBadgedCardData>
{
public:
    void set_co_badged_card_networks_info(CoBadgedCardNetworksValue value)
    {
        co_badged_card_networks_info = std::move(value);
    }

    [[nodiscard]] const CoBadgedCardNetworksValue& get_co_badged_card_networks_info() const noexcept
    {
        return co_badged_card_networks_info;
    }

    void set_issuer_country_code(CountryAlpha2Value value)
    {
        issuer_country_code = std::move(value);
    }

    [[nodiscard]] const CountryAlpha2Value& get_issuer_country_code() const noexcept
    {
        return issuer_country_code;
    }

    void set_is_regulated(bool value)
    {
        is_regulated = std::move(value);
    }

    [[nodiscard]] const bool& get_is_regulated() const noexcept
    {
        return is_regulated;
    }

    void set_regulated_name(std::optional<RegulatedNameValue> value)
    {
        regulated_name = std::move(value);
    }

    [[nodiscard]] const std::optional<RegulatedNameValue>& get_regulated_name() const noexcept
    {
        return regulated_name;
    }

    CoBadgedCardNetworksValue co_badged_card_networks_info{};
    CountryAlpha2Value issuer_country_code{};
    bool is_regulated{};
    std::optional<RegulatedNameValue> regulated_name{};
};

class CountryCodeWithName : public core::behavior::Serializer<CountryCodeWithName>
{
public:
    void set_code(CountryAlpha2Value value)
    {
        code = std::move(value);
    }

    [[nodiscard]] const CountryAlpha2Value& get_code() const noexcept
    {
        return code;
    }

    void set_name(CountryValue value)
    {
        name = std::move(value);
    }

    [[nodiscard]] const CountryValue& get_name() const noexcept
    {
        return name;
    }

    CountryAlpha2Value code{};
    CountryValue name{};
};

class CustomerDefaultPaymentMethodResponse :
    public core::behavior::Serializer<CustomerDefaultPaymentMethodResponse>
{
public:
    void set_default_payment_method_id(std::optional<std::string> value)
    {
        default_payment_method_id = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_default_payment_method_id() const noexcept
    {
        return default_payment_method_id;
    }

    void set_customer_id(CustomerIdValue value)
    {
        customer_id = std::move(value);
    }

    [[nodiscard]] const CustomerIdValue& get_customer_id() const noexcept
    {
        return customer_id;
    }

    void set_payment_method(PaymentMethodValue value)
    {
        payment_method = std::move(value);
    }

    [[nodiscard]] const PaymentMethodValue& get_payment_method() const noexcept
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

    std::optional<std::string> default_payment_method_id{};
    CustomerIdValue customer_id{};
    PaymentMethodValue payment_method{};
    std::optional<PaymentMethodTypeValue> payment_method_type{};
};

class DefaultPaymentMethod : public core::behavior::Serializer<DefaultPaymentMethod>
{
public:
    void set_customer_id(CustomerIdValue value)
    {
        customer_id = std::move(value);
    }

    [[nodiscard]] const CustomerIdValue& get_customer_id() const noexcept
    {
        return customer_id;
    }

    void set_payment_method_id(std::string value)
    {
        payment_method_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_payment_method_id() const noexcept
    {
        return payment_method_id;
    }

    CustomerIdValue customer_id{};
    std::string payment_method_id{};
};

class GetTokenDataRequest : public core::behavior::Serializer<GetTokenDataRequest>
{
public:
    void set_token_type(TokenDataTypeValue value)
    {
        token_type = std::move(value);
    }

    [[nodiscard]] const TokenDataTypeValue& get_token_type() const noexcept
    {
        return token_type;
    }

    TokenDataTypeValue token_type{};
};

class ListCountriesCurrenciesRequest :
    public core::behavior::Serializer<ListCountriesCurrenciesRequest>
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

    void set_payment_method_type(PaymentMethodTypeValue value)
    {
        payment_method_type = std::move(value);
    }

    [[nodiscard]] const PaymentMethodTypeValue& get_payment_method_type() const noexcept
    {
        return payment_method_type;
    }

    ConnectorValue connector{};
    PaymentMethodTypeValue payment_method_type{};
};

class ListMethodsForPaymentMethodsRequest :
    public core::behavior::Serializer<ListMethodsForPaymentMethodsRequest>
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

    void set_include_new(std::optional<bool> value)
    {
        include_new = std::move(value);
    }

    [[nodiscard]] const std::optional<bool>& get_include_new() const noexcept
    {
        return include_new;
    }

    std::optional<std::string> client_secret{};
    std::optional<std::vector<CountryAlpha2Value>> accepted_countries{};
    std::optional<MinorUnitValue> amount{};
    std::optional<std::vector<CurrencyValue>> accepted_currencies{};
    std::optional<bool> recurring_enabled{};
    std::optional<std::vector<CardNetworkValue>> card_networks{};
    std::optional<std::int64_t> limit{};
    std::optional<bool> include_new{};
};

class MigrateCardDetail : public core::behavior::Serializer<MigrateCardDetail>
{
public:
    void set_card_number(std::string value)
    {
        card_number = std::move(value);
    }

    [[nodiscard]] const std::string& get_card_number() const noexcept
    {
        return card_number;
    }

    void set_card_exp_month(std::string value)
    {
        card_exp_month = std::move(value);
    }

    [[nodiscard]] const std::string& get_card_exp_month() const noexcept
    {
        return card_exp_month;
    }

    void set_card_exp_year(std::string value)
    {
        card_exp_year = std::move(value);
    }

    [[nodiscard]] const std::string& get_card_exp_year() const noexcept
    {
        return card_exp_year;
    }

    void set_card_holder_name(std::optional<std::string> value)
    {
        card_holder_name = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_card_holder_name() const noexcept
    {
        return card_holder_name;
    }

    void set_nick_name(std::optional<std::string> value)
    {
        nick_name = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_nick_name() const noexcept
    {
        return nick_name;
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

    std::string card_number{};
    std::string card_exp_month{};
    std::string card_exp_year{};
    std::optional<std::string> card_holder_name{};
    std::optional<std::string> nick_name{};
    std::optional<std::string> card_issuing_country{};
    std::optional<std::string> card_issuing_country_code{};
    std::optional<CardNetworkValue> card_network{};
    std::optional<std::string> card_issuer{};
    std::optional<std::string> card_type{};
    std::optional<std::string> card_subtype{};
    std::optional<CardSegmentTypeValue> card_segment_type{};
    std::optional<FundingSourceValue> funding_source{};
};

class MigrateNetworkTokenData : public core::behavior::Serializer<MigrateNetworkTokenData>
{
public:
    void set_network_token_number(CardNumberValue value)
    {
        network_token_number = std::move(value);
    }

    [[nodiscard]] const CardNumberValue& get_network_token_number() const noexcept
    {
        return network_token_number;
    }

    void set_network_token_exp_month(std::string value)
    {
        network_token_exp_month = std::move(value);
    }

    [[nodiscard]] const std::string& get_network_token_exp_month() const noexcept
    {
        return network_token_exp_month;
    }

    void set_network_token_exp_year(std::string value)
    {
        network_token_exp_year = std::move(value);
    }

    [[nodiscard]] const std::string& get_network_token_exp_year() const noexcept
    {
        return network_token_exp_year;
    }

    void set_card_holder_name(std::optional<std::string> value)
    {
        card_holder_name = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_card_holder_name() const noexcept
    {
        return card_holder_name;
    }

    void set_nick_name(std::optional<std::string> value)
    {
        nick_name = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_nick_name() const noexcept
    {
        return nick_name;
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

    CardNumberValue network_token_number{};
    std::string network_token_exp_month{};
    std::string network_token_exp_year{};
    std::optional<std::string> card_holder_name{};
    std::optional<std::string> nick_name{};
    std::optional<std::string> card_issuing_country{};
    std::optional<std::string> card_issuing_country_code{};
    std::optional<CardNetworkValue> card_network{};
    std::optional<std::string> card_issuer{};
    std::optional<std::string> card_type{};
    std::optional<std::string> card_subtype{};
    std::optional<CardSegmentTypeValue> card_segment_type{};
    std::optional<FundingSourceValue> funding_source{};
};

class ModularPaymentMethodMigrationRecord :
    public core::behavior::Serializer<ModularPaymentMethodMigrationRecord>
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

    void set_payment_method_id(PaymentMethodId value)
    {
        payment_method_id = std::move(value);
    }

    [[nodiscard]] const PaymentMethodId& get_payment_method_id() const noexcept
    {
        return payment_method_id;
    }

    void set_line_number(std::optional<std::int64_t> value)
    {
        line_number = std::move(value);
    }

    [[nodiscard]] const std::optional<std::int64_t>& get_line_number() const noexcept
    {
        return line_number;
    }

    MerchantIdValue merchant_id{};
    PaymentMethodId payment_method_id{};
    std::optional<std::int64_t> line_number{};
};

class ModularPaymentMethodMigrationRowResult :
    public core::behavior::Serializer<ModularPaymentMethodMigrationRowResult>
{
public:
    void set_row_number(std::size_t value)
    {
        row_number = std::move(value);
    }

    [[nodiscard]] const std::size_t& get_row_number() const noexcept
    {
        return row_number;
    }

    void set_merchant_id(std::optional<MerchantIdValue> value)
    {
        merchant_id = std::move(value);
    }

    [[nodiscard]] const std::optional<MerchantIdValue>& get_merchant_id() const noexcept
    {
        return merchant_id;
    }

    void set_payment_method_id(std::optional<std::string> value)
    {
        payment_method_id = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_payment_method_id() const noexcept
    {
        return payment_method_id;
    }

    void set_old_fingerprint_id(std::optional<std::string> value)
    {
        old_fingerprint_id = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_old_fingerprint_id() const noexcept
    {
        return old_fingerprint_id;
    }

    void set_new_fingerprint_id(std::optional<std::string> value)
    {
        new_fingerprint_id = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_new_fingerprint_id() const noexcept
    {
        return new_fingerprint_id;
    }

    void set_migration_status(MigrationStatus value)
    {
        migration_status = std::move(value);
    }

    [[nodiscard]] const MigrationStatus& get_migration_status() const noexcept
    {
        return migration_status;
    }

    void set_error_message(std::optional<std::string> value)
    {
        error_message = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_error_message() const noexcept
    {
        return error_message;
    }

    std::size_t row_number{};
    std::optional<MerchantIdValue> merchant_id{};
    std::optional<std::string> payment_method_id{};
    std::optional<std::string> old_fingerprint_id{};
    std::optional<std::string> new_fingerprint_id{};
    MigrationStatus migration_status{};
    std::optional<std::string> error_message{};
};

class NetworkTokenDetailsPaymentMethod :
    public core::behavior::Serializer<NetworkTokenDetailsPaymentMethod>
{
public:
    void set_last4_digits(std::optional<std::string> value)
    {
        last4_digits = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_last4_digits() const noexcept
    {
        return last4_digits;
    }

    void set_issuer_country(std::optional<CountryAlpha2Value> value)
    {
        issuer_country = std::move(value);
    }

    [[nodiscard]] const std::optional<CountryAlpha2Value>& get_issuer_country() const noexcept
    {
        return issuer_country;
    }

    void set_network_token_expiry_month(std::optional<std::string> value)
    {
        network_token_expiry_month = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_network_token_expiry_month() const noexcept
    {
        return network_token_expiry_month;
    }

    void set_network_token_expiry_year(std::optional<std::string> value)
    {
        network_token_expiry_year = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_network_token_expiry_year() const noexcept
    {
        return network_token_expiry_year;
    }

    void set_nick_name(std::optional<std::string> value)
    {
        nick_name = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_nick_name() const noexcept
    {
        return nick_name;
    }

    void set_card_holder_name(std::optional<std::string> value)
    {
        card_holder_name = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_card_holder_name() const noexcept
    {
        return card_holder_name;
    }

    void set_card_isin(std::optional<std::string> value)
    {
        card_isin = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_card_isin() const noexcept
    {
        return card_isin;
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

    void set_saved_to_locker(bool value)
    {
        saved_to_locker = std::move(value);
    }

    [[nodiscard]] const bool& get_saved_to_locker() const noexcept
    {
        return saved_to_locker;
    }

    void set_par(std::optional<std::string> value)
    {
        par = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_par() const noexcept
    {
        return par;
    }

    std::optional<std::string> last4_digits{};
    std::optional<CountryAlpha2Value> issuer_country{};
    std::optional<std::string> network_token_expiry_month{};
    std::optional<std::string> network_token_expiry_year{};
    std::optional<std::string> nick_name{};
    std::optional<std::string> card_holder_name{};
    std::optional<std::string> card_isin{};
    std::optional<std::string> card_issuer{};
    std::optional<CardNetworkValue> card_network{};
    std::optional<std::string> card_type{};
    bool saved_to_locker{};
    std::optional<std::string> par{};
};

class NetworkTokenDetailsResponse : public core::behavior::Serializer<NetworkTokenDetailsResponse>
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

    void set_network_token_exp_month(std::string value)
    {
        network_token_exp_month = std::move(value);
    }

    [[nodiscard]] const std::string& get_network_token_exp_month() const noexcept
    {
        return network_token_exp_month;
    }

    void set_network_token_exp_year(std::string value)
    {
        network_token_exp_year = std::move(value);
    }

    [[nodiscard]] const std::string& get_network_token_exp_year() const noexcept
    {
        return network_token_exp_year;
    }

    void set_cryptogram(std::optional<std::string> value)
    {
        cryptogram = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_cryptogram() const noexcept
    {
        return cryptogram;
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

    void set_card_type(std::optional<CardType> value)
    {
        card_type = std::move(value);
    }

    [[nodiscard]] const std::optional<CardType>& get_card_type() const noexcept
    {
        return card_type;
    }

    void set_card_issuing_country(std::optional<CountryAlpha2Value> value)
    {
        card_issuing_country = std::move(value);
    }

    [[nodiscard]] const std::optional<CountryAlpha2Value>& get_card_issuing_country() const noexcept
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

    void set_card_holder_name(std::optional<std::string> value)
    {
        card_holder_name = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_card_holder_name() const noexcept
    {
        return card_holder_name;
    }

    void set_nick_name(std::optional<std::string> value)
    {
        nick_name = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_nick_name() const noexcept
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

    NetworkTokenValue network_token{};
    std::string network_token_exp_month{};
    std::string network_token_exp_year{};
    std::optional<std::string> cryptogram{};
    std::optional<std::string> card_issuer{};
    std::optional<CardNetworkValue> card_network{};
    std::optional<CardType> card_type{};
    std::optional<CountryAlpha2Value> card_issuing_country{};
    std::optional<std::string> bank_code{};
    std::optional<std::string> card_holder_name{};
    std::optional<std::string> nick_name{};
    std::optional<std::string> eci{};
};

class NetworkTokenStatusCheckSuccessResponse :
    public core::behavior::Serializer<NetworkTokenStatusCheckSuccessResponse>
{
public:
    void set_status(TokenStatusValue value)
    {
        status = std::move(value);
    }

    [[nodiscard]] const TokenStatusValue& get_status() const noexcept
    {
        return status;
    }

    void set_token_expiry_month(std::optional<std::string> value)
    {
        token_expiry_month = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_token_expiry_month() const noexcept
    {
        return token_expiry_month;
    }

    void set_token_expiry_year(std::optional<std::string> value)
    {
        token_expiry_year = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_token_expiry_year() const noexcept
    {
        return token_expiry_year;
    }

    void set_card_last_four(std::optional<std::string> value)
    {
        card_last_four = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_card_last_four() const noexcept
    {
        return card_last_four;
    }

    void set_token_last_four(std::optional<std::string> value)
    {
        token_last_four = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_token_last_four() const noexcept
    {
        return token_last_four;
    }

    void set_card_expiry_month(std::optional<std::string> value)
    {
        card_expiry_month = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_card_expiry_month() const noexcept
    {
        return card_expiry_month;
    }

    void set_card_expiry_year(std::optional<std::string> value)
    {
        card_expiry_year = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_card_expiry_year() const noexcept
    {
        return card_expiry_year;
    }

    void set_payment_method_id(GlobalPaymentMethodIdValue value)
    {
        payment_method_id = std::move(value);
    }

    [[nodiscard]] const GlobalPaymentMethodIdValue& get_payment_method_id() const noexcept
    {
        return payment_method_id;
    }

    void set_customer_id(GlobalCustomerIdValue value)
    {
        customer_id = std::move(value);
    }

    [[nodiscard]] const GlobalCustomerIdValue& get_customer_id() const noexcept
    {
        return customer_id;
    }

    TokenStatusValue status{};
    std::optional<std::string> token_expiry_month{};
    std::optional<std::string> token_expiry_year{};
    std::optional<std::string> card_last_four{};
    std::optional<std::string> token_last_four{};
    std::optional<std::string> card_expiry_month{};
    std::optional<std::string> card_expiry_year{};
    GlobalPaymentMethodIdValue payment_method_id{};
    GlobalCustomerIdValue customer_id{};
};

class PaymentExperienceTypes : public core::behavior::Serializer<PaymentExperienceTypes>
{
public:
    void set_payment_experience_type(PaymentExperienceValue value)
    {
        payment_experience_type = std::move(value);
    }

    [[nodiscard]] const PaymentExperienceValue& get_payment_experience_type() const noexcept
    {
        return payment_experience_type;
    }

    void set_eligible_connectors(std::vector<std::string> value)
    {
        eligible_connectors = std::move(value);
    }

    [[nodiscard]] const std::vector<std::string>& get_eligible_connectors() const noexcept
    {
        return eligible_connectors;
    }

    PaymentExperienceValue payment_experience_type{};
    std::vector<std::string> eligible_connectors{};
};

class PaymentMethodCollectLinkDetails :
    public core::behavior::Serializer<PaymentMethodCollectLinkDetails>
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

    void set_client_secret(std::string value)
    {
        client_secret = std::move(value);
    }

    [[nodiscard]] const std::string& get_client_secret() const noexcept
    {
        return client_secret;
    }

    void set_pm_collect_link_id(std::string value)
    {
        pm_collect_link_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_pm_collect_link_id() const noexcept
    {
        return pm_collect_link_id;
    }

    void set_customer_id(CustomerIdValue value)
    {
        customer_id = std::move(value);
    }

    [[nodiscard]] const CustomerIdValue& get_customer_id() const noexcept
    {
        return customer_id;
    }

    void set_session_expiry(PrimitiveDateTimeValue value)
    {
        session_expiry = std::move(value);
    }

    [[nodiscard]] const PrimitiveDateTimeValue& get_session_expiry() const noexcept
    {
        return session_expiry;
    }

    void set_return_url(std::optional<std::string> value)
    {
        return_url = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_return_url() const noexcept
    {
        return return_url;
    }

    void set_ui_config(GenericLinkUiConfigFormDataValue value)
    {
        ui_config = std::move(value);
    }

    [[nodiscard]] const GenericLinkUiConfigFormDataValue& get_ui_config() const noexcept
    {
        return ui_config;
    }

    void set_enabled_payment_methods(std::optional<std::vector<EnabledPaymentMethodValue>> value)
    {
        enabled_payment_methods = std::move(value);
    }

    [[nodiscard]] const std::optional<std::vector<EnabledPaymentMethodValue>>&
    get_enabled_payment_methods() const noexcept
    {
        return enabled_payment_methods;
    }

    std::string publishable_key{};
    std::string client_secret{};
    std::string pm_collect_link_id{};
    CustomerIdValue customer_id{};
    PrimitiveDateTimeValue session_expiry{};
    std::optional<std::string> return_url{};
    GenericLinkUiConfigFormDataValue ui_config{};
    std::optional<std::vector<EnabledPaymentMethodValue>> enabled_payment_methods{};
};

class PaymentMethodCollectLinkRenderRequest :
    public core::behavior::Serializer<PaymentMethodCollectLinkRenderRequest>
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

    void set_pm_collect_link_id(std::string value)
    {
        pm_collect_link_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_pm_collect_link_id() const noexcept
    {
        return pm_collect_link_id;
    }

    MerchantIdValue merchant_id{};
    std::string pm_collect_link_id{};
};

class PaymentMethodCollectLinkRequest :
    public core::behavior::Serializer<PaymentMethodCollectLinkRequest>
{
public:
    void set_pm_collect_link_id(std::optional<std::string> value)
    {
        pm_collect_link_id = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_pm_collect_link_id() const noexcept
    {
        return pm_collect_link_id;
    }

    void set_customer_id(CustomerIdValue value)
    {
        customer_id = std::move(value);
    }

    [[nodiscard]] const CustomerIdValue& get_customer_id() const noexcept
    {
        return customer_id;
    }

    void set_ui_config(std::optional<GenericLinkUiConfigValue> value)
    {
        ui_config = std::move(value);
    }

    [[nodiscard]] const std::optional<GenericLinkUiConfigValue>& get_ui_config() const noexcept
    {
        return ui_config;
    }

    void set_session_expiry(std::optional<std::uint32_t> value)
    {
        session_expiry = std::move(value);
    }

    [[nodiscard]] const std::optional<std::uint32_t>& get_session_expiry() const noexcept
    {
        return session_expiry;
    }

    void set_return_url(std::optional<std::string> value)
    {
        return_url = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_return_url() const noexcept
    {
        return return_url;
    }

    void set_enabled_payment_methods(std::optional<std::vector<EnabledPaymentMethodValue>> value)
    {
        enabled_payment_methods = std::move(value);
    }

    [[nodiscard]] const std::optional<std::vector<EnabledPaymentMethodValue>>&
    get_enabled_payment_methods() const noexcept
    {
        return enabled_payment_methods;
    }

    std::optional<std::string> pm_collect_link_id{};
    CustomerIdValue customer_id{};
    std::optional<GenericLinkUiConfigValue> ui_config{};
    std::optional<std::uint32_t> session_expiry{};
    std::optional<std::string> return_url{};
    std::optional<std::vector<EnabledPaymentMethodValue>> enabled_payment_methods{};
};

class PaymentMethodCollectLinkResponse :
    public core::behavior::Serializer<PaymentMethodCollectLinkResponse>
{
public:
    void set_pm_collect_link_id(std::string value)
    {
        pm_collect_link_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_pm_collect_link_id() const noexcept
    {
        return pm_collect_link_id;
    }

    void set_customer_id(CustomerIdValue value)
    {
        customer_id = std::move(value);
    }

    [[nodiscard]] const CustomerIdValue& get_customer_id() const noexcept
    {
        return customer_id;
    }

    void set_expiry(PrimitiveDateTimeValue value)
    {
        expiry = std::move(value);
    }

    [[nodiscard]] const PrimitiveDateTimeValue& get_expiry() const noexcept
    {
        return expiry;
    }

    void set_link(std::string value)
    {
        link = std::move(value);
    }

    [[nodiscard]] const std::string& get_link() const noexcept
    {
        return link;
    }

    void set_return_url(std::optional<std::string> value)
    {
        return_url = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_return_url() const noexcept
    {
        return return_url;
    }

    void set_ui_config(GenericLinkUiConfigValue value)
    {
        ui_config = std::move(value);
    }

    [[nodiscard]] const GenericLinkUiConfigValue& get_ui_config() const noexcept
    {
        return ui_config;
    }

    void set_enabled_payment_methods(std::optional<std::vector<EnabledPaymentMethodValue>> value)
    {
        enabled_payment_methods = std::move(value);
    }

    [[nodiscard]] const std::optional<std::vector<EnabledPaymentMethodValue>>&
    get_enabled_payment_methods() const noexcept
    {
        return enabled_payment_methods;
    }

    std::string pm_collect_link_id{};
    CustomerIdValue customer_id{};
    PrimitiveDateTimeValue expiry{};
    std::string link{};
    std::optional<std::string> return_url{};
    GenericLinkUiConfigValue ui_config{};
    std::optional<std::vector<EnabledPaymentMethodValue>> enabled_payment_methods{};
};

class PaymentMethodCollectLinkStatusDetails :
    public core::behavior::Serializer<PaymentMethodCollectLinkStatusDetails>
{
public:
    void set_pm_collect_link_id(std::string value)
    {
        pm_collect_link_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_pm_collect_link_id() const noexcept
    {
        return pm_collect_link_id;
    }

    void set_customer_id(CustomerIdValue value)
    {
        customer_id = std::move(value);
    }

    [[nodiscard]] const CustomerIdValue& get_customer_id() const noexcept
    {
        return customer_id;
    }

    void set_session_expiry(PrimitiveDateTimeValue value)
    {
        session_expiry = std::move(value);
    }

    [[nodiscard]] const PrimitiveDateTimeValue& get_session_expiry() const noexcept
    {
        return session_expiry;
    }

    void set_return_url(std::optional<UrlValue> value)
    {
        return_url = std::move(value);
    }

    [[nodiscard]] const std::optional<UrlValue>& get_return_url() const noexcept
    {
        return return_url;
    }

    void set_status(PaymentMethodCollectStatusValue value)
    {
        status = std::move(value);
    }

    [[nodiscard]] const PaymentMethodCollectStatusValue& get_status() const noexcept
    {
        return status;
    }

    void set_ui_config(GenericLinkUiConfigFormDataValue value)
    {
        ui_config = std::move(value);
    }

    [[nodiscard]] const GenericLinkUiConfigFormDataValue& get_ui_config() const noexcept
    {
        return ui_config;
    }

    std::string pm_collect_link_id{};
    CustomerIdValue customer_id{};
    PrimitiveDateTimeValue session_expiry{};
    std::optional<UrlValue> return_url{};
    PaymentMethodCollectStatusValue status{};
    GenericLinkUiConfigFormDataValue ui_config{};
};

class PaymentMethodGetTokenDetailsResponse :
    public core::behavior::Serializer<PaymentMethodGetTokenDetailsResponse>
{
public:
    void set_id(GlobalPaymentMethodIdValue value)
    {
        id = std::move(value);
    }

    [[nodiscard]] const GlobalPaymentMethodIdValue& get_id() const noexcept
    {
        return id;
    }

    void set_payment_method_token(std::string value)
    {
        payment_method_token = std::move(value);
    }

    [[nodiscard]] const std::string& get_payment_method_token() const noexcept
    {
        return payment_method_token;
    }

    GlobalPaymentMethodIdValue id{};
    std::string payment_method_token{};
};

class PaymentMethodIntentCreate : public core::behavior::Serializer<PaymentMethodIntentCreate>
{
public:
    void set_metadata(std::optional<SecretSerdeValueValue> value)
    {
        metadata = std::move(value);
    }

    [[nodiscard]] const std::optional<SecretSerdeValueValue>& get_metadata() const noexcept
    {
        return metadata;
    }

    void set_billing(std::optional<AddressValue> value)
    {
        billing = std::move(value);
    }

    [[nodiscard]] const std::optional<AddressValue>& get_billing() const noexcept
    {
        return billing;
    }

    void set_customer_id(GlobalCustomerIdValue value)
    {
        customer_id = std::move(value);
    }

    [[nodiscard]] const GlobalCustomerIdValue& get_customer_id() const noexcept
    {
        return customer_id;
    }

    std::optional<SecretSerdeValueValue> metadata{};
    std::optional<AddressValue> billing{};
    GlobalCustomerIdValue customer_id{};
};

class PaymentMethodListInstallmentAmountDetails :
    public core::behavior::Serializer<PaymentMethodListInstallmentAmountDetails>
{
public:
    void set_amount_per_installment(FloatMajorUnitValue value)
    {
        amount_per_installment = std::move(value);
    }

    [[nodiscard]] const FloatMajorUnitValue& get_amount_per_installment() const noexcept
    {
        return amount_per_installment;
    }

    void set_total_amount(FloatMajorUnitValue value)
    {
        total_amount = std::move(value);
    }

    [[nodiscard]] const FloatMajorUnitValue& get_total_amount() const noexcept
    {
        return total_amount;
    }

    FloatMajorUnitValue amount_per_installment{};
    FloatMajorUnitValue total_amount{};
};

class PaymentMethodListIntentDataInput :
    public core::behavior::Serializer<PaymentMethodListIntentDataInput>
{
public:
    void set_merchant_name(std::optional<std::string> value)
    {
        merchant_name = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_merchant_name() const noexcept
    {
        return merchant_name;
    }

    void set_mandate_payment(std::optional<MandateDataValue> value)
    {
        mandate_payment = std::move(value);
    }

    [[nodiscard]] const std::optional<MandateDataValue>& get_mandate_payment() const noexcept
    {
        return mandate_payment;
    }

    void set_payment_type(std::optional<PaymentTypeValue> value)
    {
        payment_type = std::move(value);
    }

    [[nodiscard]] const std::optional<PaymentTypeValue>& get_payment_type() const noexcept
    {
        return payment_type;
    }

    void set_capture_method(std::optional<CaptureMethodValue> value)
    {
        capture_method = std::move(value);
    }

    [[nodiscard]] const std::optional<CaptureMethodValue>& get_capture_method() const noexcept
    {
        return capture_method;
    }

    std::optional<std::string> merchant_name{};
    std::optional<MandateDataValue> mandate_payment{};
    std::optional<PaymentTypeValue> payment_type{};
    std::optional<CaptureMethodValue> capture_method{};
};

class PaymentMethodListRequest : public core::behavior::Serializer<PaymentMethodListRequest>
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

    void set_accepted_currencies(std::optional<std::vector<CurrencyValue>> value)
    {
        accepted_currencies = std::move(value);
    }

    [[nodiscard]] const std::optional<std::vector<CurrencyValue>>&
    get_accepted_currencies() const noexcept
    {
        return accepted_currencies;
    }

    void set_amount(std::optional<MinorUnitValue> value)
    {
        amount = std::move(value);
    }

    [[nodiscard]] const std::optional<MinorUnitValue>& get_amount() const noexcept
    {
        return amount;
    }

    void set_recurring_enabled(std::optional<bool> value)
    {
        recurring_enabled = std::move(value);
    }

    [[nodiscard]] const std::optional<bool>& get_recurring_enabled() const noexcept
    {
        return recurring_enabled;
    }

    void set_installment_payment_enabled(std::optional<bool> value)
    {
        installment_payment_enabled = std::move(value);
    }

    [[nodiscard]] const std::optional<bool>& get_installment_payment_enabled() const noexcept
    {
        return installment_payment_enabled;
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
    std::optional<std::vector<CurrencyValue>> accepted_currencies{};
    std::optional<MinorUnitValue> amount{};
    std::optional<bool> recurring_enabled{};
    std::optional<bool> installment_payment_enabled{};
    std::optional<std::vector<CardNetworkValue>> card_networks{};
    std::optional<std::int64_t> limit{};
};

class PaymentMethodMigrationResponse :
    public core::behavior::Serializer<PaymentMethodMigrationResponse>
{
public:
    void set_line_number(std::optional<std::int64_t> value)
    {
        line_number = std::move(value);
    }

    [[nodiscard]] const std::optional<std::int64_t>& get_line_number() const noexcept
    {
        return line_number;
    }

    void set_payment_method_id(std::optional<std::string> value)
    {
        payment_method_id = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_payment_method_id() const noexcept
    {
        return payment_method_id;
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

    void set_customer_id(std::optional<CustomerIdValue> value)
    {
        customer_id = std::move(value);
    }

    [[nodiscard]] const std::optional<CustomerIdValue>& get_customer_id() const noexcept
    {
        return customer_id;
    }

    void set_migration_status(MigrationStatus value)
    {
        migration_status = std::move(value);
    }

    [[nodiscard]] const MigrationStatus& get_migration_status() const noexcept
    {
        return migration_status;
    }

    void set_migration_error(std::optional<std::string> value)
    {
        migration_error = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_migration_error() const noexcept
    {
        return migration_error;
    }

    void set_card_number_masked(std::optional<std::string> value)
    {
        card_number_masked = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_card_number_masked() const noexcept
    {
        return card_number_masked;
    }

    void set_card_migrated(std::optional<bool> value)
    {
        card_migrated = std::move(value);
    }

    [[nodiscard]] const std::optional<bool>& get_card_migrated() const noexcept
    {
        return card_migrated;
    }

    void set_payment_method_migrated(std::optional<bool> value)
    {
        payment_method_migrated = std::move(value);
    }

    [[nodiscard]] const std::optional<bool>& get_payment_method_migrated() const noexcept
    {
        return payment_method_migrated;
    }

    void set_network_token_migrated(std::optional<bool> value)
    {
        network_token_migrated = std::move(value);
    }

    [[nodiscard]] const std::optional<bool>& get_network_token_migrated() const noexcept
    {
        return network_token_migrated;
    }

    void set_connector_mandate_details_migrated(std::optional<bool> value)
    {
        connector_mandate_details_migrated = std::move(value);
    }

    [[nodiscard]] const std::optional<bool>& get_connector_mandate_details_migrated() const noexcept
    {
        return connector_mandate_details_migrated;
    }

    void set_network_transaction_id_migrated(std::optional<bool> value)
    {
        network_transaction_id_migrated = std::move(value);
    }

    [[nodiscard]] const std::optional<bool>& get_network_transaction_id_migrated() const noexcept
    {
        return network_transaction_id_migrated;
    }

    std::optional<std::int64_t> line_number{};
    std::optional<std::string> payment_method_id{};
    std::optional<PaymentMethodValue> payment_method{};
    std::optional<PaymentMethodTypeValue> payment_method_type{};
    std::optional<CustomerIdValue> customer_id{};
    MigrationStatus migration_status{};
    std::optional<std::string> migration_error{};
    std::optional<std::string> card_number_masked{};
    std::optional<bool> card_migrated{};
    std::optional<bool> payment_method_migrated{};
    std::optional<bool> network_token_migrated{};
    std::optional<bool> connector_mandate_details_migrated{};
    std::optional<bool> network_transaction_id_migrated{};
};

class PaymentMethodRecord : public core::behavior::Serializer<PaymentMethodRecord>
{
public:
    void set_customer_id(CustomerIdValue value)
    {
        customer_id = std::move(value);
    }

    [[nodiscard]] const CustomerIdValue& get_customer_id() const noexcept
    {
        return customer_id;
    }

    void set_name(std::optional<std::string> value)
    {
        name = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_name() const noexcept
    {
        return name;
    }

    void set_card_holder_name(std::optional<std::string> value)
    {
        card_holder_name = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_card_holder_name() const noexcept
    {
        return card_holder_name;
    }

    void set_email(std::optional<EmailValue> value)
    {
        email = std::move(value);
    }

    [[nodiscard]] const std::optional<EmailValue>& get_email() const noexcept
    {
        return email;
    }

    void set_phone(std::optional<std::string> value)
    {
        phone = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_phone() const noexcept
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

    void set_merchant_id(std::optional<MerchantIdValue> value)
    {
        merchant_id = std::move(value);
    }

    [[nodiscard]] const std::optional<MerchantIdValue>& get_merchant_id() const noexcept
    {
        return merchant_id;
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

    void set_nick_name(std::optional<std::string> value)
    {
        nick_name = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_nick_name() const noexcept
    {
        return nick_name;
    }

    void set_payment_instrument_id(std::optional<std::string> value)
    {
        payment_instrument_id = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_payment_instrument_id() const noexcept
    {
        return payment_instrument_id;
    }

    void set_connector_customer_id(std::optional<std::string> value)
    {
        connector_customer_id = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_connector_customer_id() const noexcept
    {
        return connector_customer_id;
    }

    void set_connector_mandate_request_reference_id(std::optional<std::string> value)
    {
        connector_mandate_request_reference_id = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>&
    get_connector_mandate_request_reference_id() const noexcept
    {
        return connector_mandate_request_reference_id;
    }

    void set_card_number_masked(std::optional<std::string> value)
    {
        card_number_masked = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_card_number_masked() const noexcept
    {
        return card_number_masked;
    }

    void set_card_expiry_month(std::optional<std::string> value)
    {
        card_expiry_month = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_card_expiry_month() const noexcept
    {
        return card_expiry_month;
    }

    void set_card_expiry_year(std::optional<std::string> value)
    {
        card_expiry_year = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_card_expiry_year() const noexcept
    {
        return card_expiry_year;
    }

    void set_card_scheme(std::optional<std::string> value)
    {
        card_scheme = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_card_scheme() const noexcept
    {
        return card_scheme;
    }

    void set_original_transaction_id(std::optional<std::string> value)
    {
        original_transaction_id = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_original_transaction_id() const noexcept
    {
        return original_transaction_id;
    }

    void set_billing_address_zip(std::optional<std::string> value)
    {
        billing_address_zip = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_billing_address_zip() const noexcept
    {
        return billing_address_zip;
    }

    void set_billing_address_state(std::optional<std::string> value)
    {
        billing_address_state = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_billing_address_state() const noexcept
    {
        return billing_address_state;
    }

    void set_billing_address_first_name(std::optional<std::string> value)
    {
        billing_address_first_name = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_billing_address_first_name() const noexcept
    {
        return billing_address_first_name;
    }

    void set_billing_address_last_name(std::optional<std::string> value)
    {
        billing_address_last_name = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_billing_address_last_name() const noexcept
    {
        return billing_address_last_name;
    }

    void set_billing_address_city(std::optional<std::string> value)
    {
        billing_address_city = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_billing_address_city() const noexcept
    {
        return billing_address_city;
    }

    void set_billing_address_country(std::optional<CountryAlpha2Value> value)
    {
        billing_address_country = std::move(value);
    }

    [[nodiscard]] const std::optional<CountryAlpha2Value>&
    get_billing_address_country() const noexcept
    {
        return billing_address_country;
    }

    void set_billing_address_line1(std::optional<std::string> value)
    {
        billing_address_line1 = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_billing_address_line1() const noexcept
    {
        return billing_address_line1;
    }

    void set_billing_address_line2(std::optional<std::string> value)
    {
        billing_address_line2 = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_billing_address_line2() const noexcept
    {
        return billing_address_line2;
    }

    void set_billing_address_line3(std::optional<std::string> value)
    {
        billing_address_line3 = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_billing_address_line3() const noexcept
    {
        return billing_address_line3;
    }

    void set_raw_card_number(std::optional<std::string> value)
    {
        raw_card_number = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_raw_card_number() const noexcept
    {
        return raw_card_number;
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

    void set_merchant_connector_ids(std::optional<std::string> value)
    {
        merchant_connector_ids = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_merchant_connector_ids() const noexcept
    {
        return merchant_connector_ids;
    }

    void set_original_transaction_amount(std::optional<std::int64_t> value)
    {
        original_transaction_amount = std::move(value);
    }

    [[nodiscard]] const std::optional<std::int64_t>&
    get_original_transaction_amount() const noexcept
    {
        return original_transaction_amount;
    }

    void set_original_transaction_currency(std::optional<CurrencyValue> value)
    {
        original_transaction_currency = std::move(value);
    }

    [[nodiscard]] const std::optional<CurrencyValue>&
    get_original_transaction_currency() const noexcept
    {
        return original_transaction_currency;
    }

    void set_line_number(std::optional<std::int64_t> value)
    {
        line_number = std::move(value);
    }

    [[nodiscard]] const std::optional<std::int64_t>& get_line_number() const noexcept
    {
        return line_number;
    }

    void set_network_token_number(std::optional<CardNumberValue> value)
    {
        network_token_number = std::move(value);
    }

    [[nodiscard]] const std::optional<CardNumberValue>& get_network_token_number() const noexcept
    {
        return network_token_number;
    }

    void set_network_token_expiry_month(std::optional<std::string> value)
    {
        network_token_expiry_month = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_network_token_expiry_month() const noexcept
    {
        return network_token_expiry_month;
    }

    void set_network_token_expiry_year(std::optional<std::string> value)
    {
        network_token_expiry_year = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_network_token_expiry_year() const noexcept
    {
        return network_token_expiry_year;
    }

    void set_network_token_requestor_ref_id(std::optional<std::string> value)
    {
        network_token_requestor_ref_id = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>&
    get_network_token_requestor_ref_id() const noexcept
    {
        return network_token_requestor_ref_id;
    }

    void set_account_number(std::optional<std::string> value)
    {
        account_number = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_account_number() const noexcept
    {
        return account_number;
    }

    void set_routing_number(std::optional<std::string> value)
    {
        routing_number = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_routing_number() const noexcept
    {
        return routing_number;
    }

    void set_bank_account_holder_name(std::optional<std::string> value)
    {
        bank_account_holder_name = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_bank_account_holder_name() const noexcept
    {
        return bank_account_holder_name;
    }

    void set_bank_type(std::optional<BankTypeValue> value)
    {
        bank_type = std::move(value);
    }

    [[nodiscard]] const std::optional<BankTypeValue>& get_bank_type() const noexcept
    {
        return bank_type;
    }

    void set_bank_holder_type(std::optional<BankHolderTypeValue> value)
    {
        bank_holder_type = std::move(value);
    }

    [[nodiscard]] const std::optional<BankHolderTypeValue>& get_bank_holder_type() const noexcept
    {
        return bank_holder_type;
    }

    void set_bank_name(std::optional<BankNamesValue> value)
    {
        bank_name = std::move(value);
    }

    [[nodiscard]] const std::optional<BankNamesValue>& get_bank_name() const noexcept
    {
        return bank_name;
    }

    CustomerIdValue customer_id{};
    std::optional<std::string> name{};
    std::optional<std::string> card_holder_name{};
    std::optional<EmailValue> email{};
    std::optional<std::string> phone{};
    std::optional<std::string> phone_country_code{};
    std::optional<MerchantIdValue> merchant_id{};
    std::optional<PaymentMethodValue> payment_method{};
    std::optional<PaymentMethodTypeValue> payment_method_type{};
    std::optional<std::string> nick_name{};
    std::optional<std::string> payment_instrument_id{};
    std::optional<std::string> connector_customer_id{};
    std::optional<std::string> connector_mandate_request_reference_id{};
    std::optional<std::string> card_number_masked{};
    std::optional<std::string> card_expiry_month{};
    std::optional<std::string> card_expiry_year{};
    std::optional<std::string> card_scheme{};
    std::optional<std::string> original_transaction_id{};
    std::optional<std::string> billing_address_zip{};
    std::optional<std::string> billing_address_state{};
    std::optional<std::string> billing_address_first_name{};
    std::optional<std::string> billing_address_last_name{};
    std::optional<std::string> billing_address_city{};
    std::optional<CountryAlpha2Value> billing_address_country{};
    std::optional<std::string> billing_address_line1{};
    std::optional<std::string> billing_address_line2{};
    std::optional<std::string> billing_address_line3{};
    std::optional<std::string> raw_card_number{};
    std::optional<MerchantConnectorAccountIdValue> merchant_connector_id{};
    std::optional<std::string> merchant_connector_ids{};
    std::optional<std::int64_t> original_transaction_amount{};
    std::optional<CurrencyValue> original_transaction_currency{};
    std::optional<std::int64_t> line_number{};
    std::optional<CardNumberValue> network_token_number{};
    std::optional<std::string> network_token_expiry_month{};
    std::optional<std::string> network_token_expiry_year{};
    std::optional<std::string> network_token_requestor_ref_id{};
    std::optional<std::string> account_number{};
    std::optional<std::string> routing_number{};
    std::optional<std::string> bank_account_holder_name{};
    std::optional<BankTypeValue> bank_type{};
    std::optional<BankHolderTypeValue> bank_holder_type{};
    std::optional<BankNamesValue> bank_name{};
};

class PaymentMethodRecordUpdateResponse :
    public core::behavior::Serializer<PaymentMethodRecordUpdateResponse>
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

    void set_status(PaymentMethodStatusValue value)
    {
        status = std::move(value);
    }

    [[nodiscard]] const PaymentMethodStatusValue& get_status() const noexcept
    {
        return status;
    }

    void set_network_transaction_id(std::optional<std::string> value)
    {
        network_transaction_id = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_network_transaction_id() const noexcept
    {
        return network_transaction_id;
    }

    void set_connector_mandate_details(std::optional<SecretSerdeValueValue> value)
    {
        connector_mandate_details = std::move(value);
    }

    [[nodiscard]] const std::optional<SecretSerdeValueValue>&
    get_connector_mandate_details() const noexcept
    {
        return connector_mandate_details;
    }

    void set_updated_payment_method_data(std::optional<bool> value)
    {
        updated_payment_method_data = std::move(value);
    }

    [[nodiscard]] const std::optional<bool>& get_updated_payment_method_data() const noexcept
    {
        return updated_payment_method_data;
    }

    std::string payment_method_id{};
    PaymentMethodStatusValue status{};
    std::optional<std::string> network_transaction_id{};
    std::optional<SecretSerdeValueValue> connector_mandate_details{};
    std::optional<bool> updated_payment_method_data{};
};

class PaymentMethodSessionConfirmRequest :
    public core::behavior::Serializer<PaymentMethodSessionConfirmRequest>
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

    void set_payment_method_subtype(std::optional<PaymentMethodTypeValue> value)
    {
        payment_method_subtype = std::move(value);
    }

    [[nodiscard]] const std::optional<PaymentMethodTypeValue>&
    get_payment_method_subtype() const noexcept
    {
        return payment_method_subtype;
    }

    void set_payment_method_data(PaymentMethodDataRequestValue value)
    {
        payment_method_data = std::move(value);
    }

    [[nodiscard]] const PaymentMethodDataRequestValue& get_payment_method_data() const noexcept
    {
        return payment_method_data;
    }

    void set_return_url(std::optional<UrlValue> value)
    {
        return_url = std::move(value);
    }

    [[nodiscard]] const std::optional<UrlValue>& get_return_url() const noexcept
    {
        return return_url;
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

    PaymentMethodValue payment_method_type{};
    std::optional<PaymentMethodTypeValue> payment_method_subtype{};
    PaymentMethodDataRequestValue payment_method_data{};
    std::optional<UrlValue> return_url{};
    std::optional<CustomerAcceptanceValue> customer_acceptance{};
};

class PaymentMethodSessionRequest : public core::behavior::Serializer<PaymentMethodSessionRequest>
{
public:
    void set_customer_id(std::optional<GlobalCustomerIdValue> value)
    {
        customer_id = std::move(value);
    }

    [[nodiscard]] const std::optional<GlobalCustomerIdValue>& get_customer_id() const noexcept
    {
        return customer_id;
    }

    void set_billing(std::optional<AddressValue> value)
    {
        billing = std::move(value);
    }

    [[nodiscard]] const std::optional<AddressValue>& get_billing() const noexcept
    {
        return billing;
    }

    void set_return_url(std::optional<UrlValue> value)
    {
        return_url = std::move(value);
    }

    [[nodiscard]] const std::optional<UrlValue>& get_return_url() const noexcept
    {
        return return_url;
    }

    void set_network_tokenization(std::optional<NetworkTokenizationValue> value)
    {
        network_tokenization = std::move(value);
    }

    [[nodiscard]] const std::optional<NetworkTokenizationValue>&
    get_network_tokenization() const noexcept
    {
        return network_tokenization;
    }

    void set_expires_in(std::optional<std::uint32_t> value)
    {
        expires_in = std::move(value);
    }

    [[nodiscard]] const std::optional<std::uint32_t>& get_expires_in() const noexcept
    {
        return expires_in;
    }

    void set_tokenization_data(std::optional<SecretSerdeValueValue> value)
    {
        tokenization_data = std::move(value);
    }

    [[nodiscard]] const std::optional<SecretSerdeValueValue>& get_tokenization_data() const noexcept
    {
        return tokenization_data;
    }

    void set_storage_type(StorageTypeValue value)
    {
        storage_type = std::move(value);
    }

    [[nodiscard]] const StorageTypeValue& get_storage_type() const noexcept
    {
        return storage_type;
    }

    void set_keep_alive(std::optional<bool> value)
    {
        keep_alive = std::move(value);
    }

    [[nodiscard]] const std::optional<bool>& get_keep_alive() const noexcept
    {
        return keep_alive;
    }

    std::optional<GlobalCustomerIdValue> customer_id{};
    std::optional<AddressValue> billing{};
    std::optional<UrlValue> return_url{};
    std::optional<NetworkTokenizationValue> network_tokenization{};
    std::optional<std::uint32_t> expires_in{};
    std::optional<SecretSerdeValueValue> tokenization_data{};
    StorageTypeValue storage_type{};
    std::optional<bool> keep_alive{};
};

class PaymentMethodUpdateResponse : public core::behavior::Serializer<PaymentMethodUpdateResponse>
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

    void set_status(std::optional<PaymentMethodStatusValue> value)
    {
        status = std::move(value);
    }

    [[nodiscard]] const std::optional<PaymentMethodStatusValue>& get_status() const noexcept
    {
        return status;
    }

    void set_network_transaction_id(std::optional<std::string> value)
    {
        network_transaction_id = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_network_transaction_id() const noexcept
    {
        return network_transaction_id;
    }

    void set_connector_mandate_details(std::optional<SecretSerdeValueValue> value)
    {
        connector_mandate_details = std::move(value);
    }

    [[nodiscard]] const std::optional<SecretSerdeValueValue>&
    get_connector_mandate_details() const noexcept
    {
        return connector_mandate_details;
    }

    void set_update_status(UpdateStatus value)
    {
        update_status = std::move(value);
    }

    [[nodiscard]] const UpdateStatus& get_update_status() const noexcept
    {
        return update_status;
    }

    void set_update_error(std::optional<std::string> value)
    {
        update_error = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_update_error() const noexcept
    {
        return update_error;
    }

    void set_updated_payment_method_data(std::optional<bool> value)
    {
        updated_payment_method_data = std::move(value);
    }

    [[nodiscard]] const std::optional<bool>& get_updated_payment_method_data() const noexcept
    {
        return updated_payment_method_data;
    }

    void set_line_number(std::optional<std::int64_t> value)
    {
        line_number = std::move(value);
    }

    [[nodiscard]] const std::optional<std::int64_t>& get_line_number() const noexcept
    {
        return line_number;
    }

    std::string payment_method_id{};
    std::optional<PaymentMethodStatusValue> status{};
    std::optional<std::string> network_transaction_id{};
    std::optional<SecretSerdeValueValue> connector_mandate_details{};
    UpdateStatus update_status{};
    std::optional<std::string> update_error{};
    std::optional<bool> updated_payment_method_data{};
    std::optional<std::int64_t> line_number{};
};

class PaymentMethodsSessionUpdateRequest :
    public core::behavior::Serializer<PaymentMethodsSessionUpdateRequest>
{
public:
    void set_billing(std::optional<AddressValue> value)
    {
        billing = std::move(value);
    }

    [[nodiscard]] const std::optional<AddressValue>& get_billing() const noexcept
    {
        return billing;
    }

    void set_network_tokenization(std::optional<NetworkTokenizationValue> value)
    {
        network_tokenization = std::move(value);
    }

    [[nodiscard]] const std::optional<NetworkTokenizationValue>&
    get_network_tokenization() const noexcept
    {
        return network_tokenization;
    }

    void set_tokenization_data(std::optional<SecretSerdeValueValue> value)
    {
        tokenization_data = std::move(value);
    }

    [[nodiscard]] const std::optional<SecretSerdeValueValue>& get_tokenization_data() const noexcept
    {
        return tokenization_data;
    }

    std::optional<AddressValue> billing{};
    std::optional<NetworkTokenizationValue> network_tokenization{};
    std::optional<SecretSerdeValueValue> tokenization_data{};
};

class PaymentsMandateReferenceRecord :
    public core::behavior::Serializer<PaymentsMandateReferenceRecord>
{
public:
    void set_connector_mandate_id(std::string value)
    {
        connector_mandate_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_connector_mandate_id() const noexcept
    {
        return connector_mandate_id;
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

    void set_original_payment_authorized_amount(std::optional<std::int64_t> value)
    {
        original_payment_authorized_amount = std::move(value);
    }

    [[nodiscard]] const std::optional<std::int64_t>&
    get_original_payment_authorized_amount() const noexcept
    {
        return original_payment_authorized_amount;
    }

    void set_original_payment_authorized_currency(std::optional<CurrencyValue> value)
    {
        original_payment_authorized_currency = std::move(value);
    }

    [[nodiscard]] const std::optional<CurrencyValue>&
    get_original_payment_authorized_currency() const noexcept
    {
        return original_payment_authorized_currency;
    }

    void set_connector_customer_id(std::optional<std::string> value)
    {
        connector_customer_id = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_connector_customer_id() const noexcept
    {
        return connector_customer_id;
    }

    void set_connector_mandate_request_reference_id(std::optional<std::string> value)
    {
        connector_mandate_request_reference_id = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>&
    get_connector_mandate_request_reference_id() const noexcept
    {
        return connector_mandate_request_reference_id;
    }

    std::string connector_mandate_id{};
    std::optional<PaymentMethodTypeValue> payment_method_type{};
    std::optional<std::int64_t> original_payment_authorized_amount{};
    std::optional<CurrencyValue> original_payment_authorized_currency{};
    std::optional<std::string> connector_customer_id{};
    std::optional<std::string> connector_mandate_request_reference_id{};
};

class PayoutsMandateReference : public core::behavior::Serializer<PayoutsMandateReference>
{
public:
    void set_field0(std::unordered_map<std::string, PayoutsMandateReferenceRecord> value)
    {
        field0 = std::move(value);
    }

    [[nodiscard]] const std::unordered_map<std::string, PayoutsMandateReferenceRecord>&
    get_field0() const noexcept
    {
        return field0;
    }

    std::unordered_map<std::string, PayoutsMandateReferenceRecord> field0{};
};

class RequestPaymentMethodTypes : public core::behavior::Serializer<RequestPaymentMethodTypes>
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

    void set_payment_experience(std::optional<PaymentExperienceValue> value)
    {
        payment_experience = std::move(value);
    }

    [[nodiscard]] const std::optional<PaymentExperienceValue>&
    get_payment_experience() const noexcept
    {
        return payment_experience;
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

    void set_accepted_currencies(std::optional<AcceptedCurrenciesValue> value)
    {
        accepted_currencies = std::move(value);
    }

    [[nodiscard]] const std::optional<AcceptedCurrenciesValue>&
    get_accepted_currencies() const noexcept
    {
        return accepted_currencies;
    }

    void set_accepted_countries(std::optional<AcceptedCountriesValue> value)
    {
        accepted_countries = std::move(value);
    }

    [[nodiscard]] const std::optional<AcceptedCountriesValue>&
    get_accepted_countries() const noexcept
    {
        return accepted_countries;
    }

    void set_minimum_amount(std::optional<MinorUnitValue> value)
    {
        minimum_amount = std::move(value);
    }

    [[nodiscard]] const std::optional<MinorUnitValue>& get_minimum_amount() const noexcept
    {
        return minimum_amount;
    }

    void set_maximum_amount(std::optional<MinorUnitValue> value)
    {
        maximum_amount = std::move(value);
    }

    [[nodiscard]] const std::optional<MinorUnitValue>& get_maximum_amount() const noexcept
    {
        return maximum_amount;
    }

    void set_recurring_enabled(std::optional<bool> value)
    {
        recurring_enabled = std::move(value);
    }

    [[nodiscard]] const std::optional<bool>& get_recurring_enabled() const noexcept
    {
        return recurring_enabled;
    }

    void set_installment_payment_enabled(std::optional<bool> value)
    {
        installment_payment_enabled = std::move(value);
    }

    [[nodiscard]] const std::optional<bool>& get_installment_payment_enabled() const noexcept
    {
        return installment_payment_enabled;
    }

    void set_pay_via_single_use_token(std::optional<PayViaSingleUseTokenValue> value)
    {
        pay_via_single_use_token = std::move(value);
    }

    [[nodiscard]] const std::optional<PayViaSingleUseTokenValue>&
    get_pay_via_single_use_token() const noexcept
    {
        return pay_via_single_use_token;
    }

    PaymentMethodTypeValue payment_method_type{};
    std::optional<PaymentExperienceValue> payment_experience{};
    std::optional<std::vector<CardNetworkValue>> card_networks{};
    std::optional<AcceptedCurrenciesValue> accepted_currencies{};
    std::optional<AcceptedCountriesValue> accepted_countries{};
    std::optional<MinorUnitValue> minimum_amount{};
    std::optional<MinorUnitValue> maximum_amount{};
    std::optional<bool> recurring_enabled{};
    std::optional<bool> installment_payment_enabled{};
    std::optional<PayViaSingleUseTokenValue> pay_via_single_use_token{};
};

class RequiredFieldInfo : public core::behavior::Serializer<RequiredFieldInfo>
{
public:
    void set_required_field(std::string value)
    {
        required_field = std::move(value);
    }

    [[nodiscard]] const std::string& get_required_field() const noexcept
    {
        return required_field;
    }

    void set_display_name(std::string value)
    {
        display_name = std::move(value);
    }

    [[nodiscard]] const std::string& get_display_name() const noexcept
    {
        return display_name;
    }

    void set_field_type(FieldTypeValue value)
    {
        field_type = std::move(value);
    }

    [[nodiscard]] const FieldTypeValue& get_field_type() const noexcept
    {
        return field_type;
    }

    void set_value(std::optional<std::string> value)
    {
        value = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_value() const noexcept
    {
        return value;
    }

    std::string required_field{};
    std::string display_name{};
    FieldTypeValue field_type{};
    std::optional<std::string> value{};
};

class ResponsePaymentMethodIntermediate :
    public core::behavior::Serializer<ResponsePaymentMethodIntermediate>
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

    void set_payment_experience(std::optional<PaymentExperienceValue> value)
    {
        payment_experience = std::move(value);
    }

    [[nodiscard]] const std::optional<PaymentExperienceValue>&
    get_payment_experience() const noexcept
    {
        return payment_experience;
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

    void set_payment_method(PaymentMethodValue value)
    {
        payment_method = std::move(value);
    }

    [[nodiscard]] const PaymentMethodValue& get_payment_method() const noexcept
    {
        return payment_method;
    }

    void set_connector(std::string value)
    {
        connector = std::move(value);
    }

    [[nodiscard]] const std::string& get_connector() const noexcept
    {
        return connector;
    }

    void set_merchant_connector_id(std::string value)
    {
        merchant_connector_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_merchant_connector_id() const noexcept
    {
        return merchant_connector_id;
    }

    PaymentMethodTypeValue payment_method_type{};
    std::optional<PaymentExperienceValue> payment_experience{};
    std::optional<std::vector<CardNetworkValue>> card_networks{};
    PaymentMethodValue payment_method{};
    std::string connector{};
    std::string merchant_connector_id{};
};

class SurchargeDetailsResponse : public core::behavior::Serializer<SurchargeDetailsResponse>
{
public:
    void set_surcharge(SurchargeResponse value)
    {
        surcharge = std::move(value);
    }

    [[nodiscard]] const SurchargeResponse& get_surcharge() const noexcept
    {
        return surcharge;
    }

    void set_tax_on_surcharge(std::optional<SurchargePercentage> value)
    {
        tax_on_surcharge = std::move(value);
    }

    [[nodiscard]] const std::optional<SurchargePercentage>& get_tax_on_surcharge() const noexcept
    {
        return tax_on_surcharge;
    }

    void set_display_surcharge_amount(double value)
    {
        display_surcharge_amount = std::move(value);
    }

    [[nodiscard]] const double& get_display_surcharge_amount() const noexcept
    {
        return display_surcharge_amount;
    }

    void set_display_tax_on_surcharge_amount(double value)
    {
        display_tax_on_surcharge_amount = std::move(value);
    }

    [[nodiscard]] const double& get_display_tax_on_surcharge_amount() const noexcept
    {
        return display_tax_on_surcharge_amount;
    }

    void set_display_total_surcharge_amount(double value)
    {
        display_total_surcharge_amount = std::move(value);
    }

    [[nodiscard]] const double& get_display_total_surcharge_amount() const noexcept
    {
        return display_total_surcharge_amount;
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

    SurchargeResponse surcharge{};
    std::optional<SurchargePercentage> tax_on_surcharge{};
    double display_surcharge_amount{};
    double display_tax_on_surcharge_amount{};
    double display_total_surcharge_amount{};
    std::optional<double> surcharge_percentage{};
    std::optional<SurchargeErrorDetailsValue> error_details{};
};

class TokenDataResponse : public core::behavior::Serializer<TokenDataResponse>
{
public:
    void set_payment_method_id(GlobalPaymentMethodIdValue value)
    {
        payment_method_id = std::move(value);
    }

    [[nodiscard]] const GlobalPaymentMethodIdValue& get_payment_method_id() const noexcept
    {
        return payment_method_id;
    }

    void set_token_type(TokenDataTypeValue value)
    {
        token_type = std::move(value);
    }

    [[nodiscard]] const TokenDataTypeValue& get_token_type() const noexcept
    {
        return token_type;
    }

    void set_token_details(TokenDetailsResponse value)
    {
        token_details = std::move(value);
    }

    [[nodiscard]] const TokenDetailsResponse& get_token_details() const noexcept
    {
        return token_details;
    }

    GlobalPaymentMethodIdValue payment_method_id{};
    TokenDataTypeValue token_type{};
    TokenDetailsResponse token_details{};
};

class TokenizeCardRequest : public core::behavior::Serializer<TokenizeCardRequest>
{
public:
    void set_raw_card_number(CardNumberValue value)
    {
        raw_card_number = std::move(value);
    }

    [[nodiscard]] const CardNumberValue& get_raw_card_number() const noexcept
    {
        return raw_card_number;
    }

    void set_card_expiry_month(std::string value)
    {
        card_expiry_month = std::move(value);
    }

    [[nodiscard]] const std::string& get_card_expiry_month() const noexcept
    {
        return card_expiry_month;
    }

    void set_card_expiry_year(std::string value)
    {
        card_expiry_year = std::move(value);
    }

    [[nodiscard]] const std::string& get_card_expiry_year() const noexcept
    {
        return card_expiry_year;
    }

    void set_card_cvc(std::optional<std::string> value)
    {
        card_cvc = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_card_cvc() const noexcept
    {
        return card_cvc;
    }

    void set_card_holder_name(std::optional<std::string> value)
    {
        card_holder_name = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_card_holder_name() const noexcept
    {
        return card_holder_name;
    }

    void set_nick_name(std::optional<std::string> value)
    {
        nick_name = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_nick_name() const noexcept
    {
        return nick_name;
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

    void set_card_type(std::optional<CardType> value)
    {
        card_type = std::move(value);
    }

    [[nodiscard]] const std::optional<CardType>& get_card_type() const noexcept
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

    CardNumberValue raw_card_number{};
    std::string card_expiry_month{};
    std::string card_expiry_year{};
    std::optional<std::string> card_cvc{};
    std::optional<std::string> card_holder_name{};
    std::optional<std::string> nick_name{};
    std::optional<std::string> card_issuing_country{};
    std::optional<std::string> card_issuing_country_code{};
    std::optional<CardNetworkValue> card_network{};
    std::optional<std::string> card_issuer{};
    std::optional<CardType> card_type{};
    std::optional<std::string> card_subtype{};
    std::optional<CardSegmentTypeValue> card_segment_type{};
    std::optional<FundingSourceValue> funding_source{};
};

class UpdatePaymentMethodRecord : public core::behavior::Serializer<UpdatePaymentMethodRecord>
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

    void set_status(std::optional<PaymentMethodStatusValue> value)
    {
        status = std::move(value);
    }

    [[nodiscard]] const std::optional<PaymentMethodStatusValue>& get_status() const noexcept
    {
        return status;
    }

    void set_network_transaction_id(std::optional<std::string> value)
    {
        network_transaction_id = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_network_transaction_id() const noexcept
    {
        return network_transaction_id;
    }

    void set_line_number(std::optional<std::int64_t> value)
    {
        line_number = std::move(value);
    }

    [[nodiscard]] const std::optional<std::int64_t>& get_line_number() const noexcept
    {
        return line_number;
    }

    void set_payment_instrument_id(std::optional<std::string> value)
    {
        payment_instrument_id = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_payment_instrument_id() const noexcept
    {
        return payment_instrument_id;
    }

    void set_connector_customer_id(std::optional<std::string> value)
    {
        connector_customer_id = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_connector_customer_id() const noexcept
    {
        return connector_customer_id;
    }

    void set_merchant_connector_ids(std::optional<std::string> value)
    {
        merchant_connector_ids = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_merchant_connector_ids() const noexcept
    {
        return merchant_connector_ids;
    }

    void set_card_expiry_month(std::optional<std::string> value)
    {
        card_expiry_month = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_card_expiry_month() const noexcept
    {
        return card_expiry_month;
    }

    void set_card_expiry_year(std::optional<std::string> value)
    {
        card_expiry_year = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_card_expiry_year() const noexcept
    {
        return card_expiry_year;
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

    void set_card_issuer(std::optional<std::string> value)
    {
        card_issuer = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_card_issuer() const noexcept
    {
        return card_issuer;
    }

    std::string payment_method_id{};
    std::optional<PaymentMethodStatusValue> status{};
    std::optional<std::string> network_transaction_id{};
    std::optional<std::int64_t> line_number{};
    std::optional<std::string> payment_instrument_id{};
    std::optional<std::string> connector_customer_id{};
    std::optional<std::string> merchant_connector_ids{};
    std::optional<std::string> card_expiry_month{};
    std::optional<std::string> card_expiry_year{};
    std::optional<PaymentMethodTypeValue> payment_method_type{};
    std::optional<CardNetworkValue> card_network{};
    std::optional<std::string> card_type{};
    std::optional<std::string> card_issuer{};
};

class CardNetworkTypes : public core::behavior::Serializer<CardNetworkTypes>
{
public:
    void set_card_network(CardNetworkValue value)
    {
        card_network = std::move(value);
    }

    [[nodiscard]] const CardNetworkValue& get_card_network() const noexcept
    {
        return card_network;
    }

    void set_surcharge_details(std::optional<SurchargeDetailsResponse> value)
    {
        surcharge_details = std::move(value);
    }

    [[nodiscard]] const std::optional<SurchargeDetailsResponse>&
    get_surcharge_details() const noexcept
    {
        return surcharge_details;
    }

    void set_eligible_connectors(std::vector<std::string> value)
    {
        eligible_connectors = std::move(value);
    }

    [[nodiscard]] const std::vector<std::string>& get_eligible_connectors() const noexcept
    {
        return eligible_connectors;
    }

    CardNetworkValue card_network{};
    std::optional<SurchargeDetailsResponse> surcharge_details{};
    std::vector<std::string> eligible_connectors{};
};

class CustomerPaymentMethod : public core::behavior::Serializer<CustomerPaymentMethod>
{
public:
    void set_payment_token(std::string value)
    {
        payment_token = std::move(value);
    }

    [[nodiscard]] const std::string& get_payment_token() const noexcept
    {
        return payment_token;
    }

    void set_payment_method_id(std::string value)
    {
        payment_method_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_payment_method_id() const noexcept
    {
        return payment_method_id;
    }

    void set_customer_id(CustomerIdValue value)
    {
        customer_id = std::move(value);
    }

    [[nodiscard]] const CustomerIdValue& get_customer_id() const noexcept
    {
        return customer_id;
    }

    void set_payment_method(PaymentMethodValue value)
    {
        payment_method = std::move(value);
    }

    [[nodiscard]] const PaymentMethodValue& get_payment_method() const noexcept
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

    void set_payment_method_issuer(std::optional<std::string> value)
    {
        payment_method_issuer = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_payment_method_issuer() const noexcept
    {
        return payment_method_issuer;
    }

    void set_payment_method_issuer_code(std::optional<PaymentMethodIssuerCodeValue> value)
    {
        payment_method_issuer_code = std::move(value);
    }

    [[nodiscard]] const std::optional<PaymentMethodIssuerCodeValue>&
    get_payment_method_issuer_code() const noexcept
    {
        return payment_method_issuer_code;
    }

    void set_recurring_enabled(std::optional<bool> value)
    {
        recurring_enabled = std::move(value);
    }

    [[nodiscard]] const std::optional<bool>& get_recurring_enabled() const noexcept
    {
        return recurring_enabled;
    }

    void set_installment_payment_enabled(std::optional<bool> value)
    {
        installment_payment_enabled = std::move(value);
    }

    [[nodiscard]] const std::optional<bool>& get_installment_payment_enabled() const noexcept
    {
        return installment_payment_enabled;
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

    void set_card(std::optional<CardDetailFromLocker> value)
    {
        card = std::move(value);
    }

    [[nodiscard]] const std::optional<CardDetailFromLocker>& get_card() const noexcept
    {
        return card;
    }

    void set_wallets(std::optional<WalletPaymentMethodDataForClient> value)
    {
        wallets = std::move(value);
    }

    [[nodiscard]] const std::optional<WalletPaymentMethodDataForClient>&
    get_wallets() const noexcept
    {
        return wallets;
    }

    void set_metadata(std::optional<SecretSerdeValueValue> value)
    {
        metadata = std::move(value);
    }

    [[nodiscard]] const std::optional<SecretSerdeValueValue>& get_metadata() const noexcept
    {
        return metadata;
    }

    void set_created(std::optional<PrimitiveDateTimeValue> value)
    {
        created = std::move(value);
    }

    [[nodiscard]] const std::optional<PrimitiveDateTimeValue>& get_created() const noexcept
    {
        return created;
    }

    void set_bank_transfer(std::optional<BankValue> value)
    {
        bank_transfer = std::move(value);
    }

    [[nodiscard]] const std::optional<BankValue>& get_bank_transfer() const noexcept
    {
        return bank_transfer;
    }

    void set_bank(std::optional<MaskedBankDetails> value)
    {
        bank = std::move(value);
    }

    [[nodiscard]] const std::optional<MaskedBankDetails>& get_bank() const noexcept
    {
        return bank;
    }

    void set_surcharge_details(std::optional<SurchargeDetailsResponse> value)
    {
        surcharge_details = std::move(value);
    }

    [[nodiscard]] const std::optional<SurchargeDetailsResponse>&
    get_surcharge_details() const noexcept
    {
        return surcharge_details;
    }

    void set_requires_cvv(bool value)
    {
        requires_cvv = std::move(value);
    }

    [[nodiscard]] const bool& get_requires_cvv() const noexcept
    {
        return requires_cvv;
    }

    void set_last_used_at(std::optional<PrimitiveDateTimeValue> value)
    {
        last_used_at = std::move(value);
    }

    [[nodiscard]] const std::optional<PrimitiveDateTimeValue>& get_last_used_at() const noexcept
    {
        return last_used_at;
    }

    void set_default_payment_method_set(bool value)
    {
        default_payment_method_set = std::move(value);
    }

    [[nodiscard]] const bool& get_default_payment_method_set() const noexcept
    {
        return default_payment_method_set;
    }

    void set_billing(std::optional<AddressValue> value)
    {
        billing = std::move(value);
    }

    [[nodiscard]] const std::optional<AddressValue>& get_billing() const noexcept
    {
        return billing;
    }

    std::string payment_token{};
    std::string payment_method_id{};
    CustomerIdValue customer_id{};
    PaymentMethodValue payment_method{};
    std::optional<PaymentMethodTypeValue> payment_method_type{};
    std::optional<std::string> payment_method_issuer{};
    std::optional<PaymentMethodIssuerCodeValue> payment_method_issuer_code{};
    std::optional<bool> recurring_enabled{};
    std::optional<bool> installment_payment_enabled{};
    std::optional<std::vector<PaymentExperienceValue>> payment_experience{};
    std::optional<CardDetailFromLocker> card{};
    std::optional<WalletPaymentMethodDataForClient> wallets{};
    std::optional<SecretSerdeValueValue> metadata{};
    std::optional<PrimitiveDateTimeValue> created{};
    std::optional<BankValue> bank_transfer{};
    std::optional<MaskedBankDetails> bank{};
    std::optional<SurchargeDetailsResponse> surcharge_details{};
    bool requires_cvv{};
    std::optional<PrimitiveDateTimeValue> last_used_at{};
    bool default_payment_method_set{};
    std::optional<AddressValue> billing{};
};

class CustomerPaymentMethodUpdateResponse :
    public core::behavior::Serializer<CustomerPaymentMethodUpdateResponse>
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

    void set_customer_id(std::optional<CustomerIdValue> value)
    {
        customer_id = std::move(value);
    }

    [[nodiscard]] const std::optional<CustomerIdValue>& get_customer_id() const noexcept
    {
        return customer_id;
    }

    void set_payment_method_id(std::string value)
    {
        payment_method_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_payment_method_id() const noexcept
    {
        return payment_method_id;
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

    void set_card(std::optional<CardDetailFromLocker> value)
    {
        card = std::move(value);
    }

    [[nodiscard]] const std::optional<CardDetailFromLocker>& get_card() const noexcept
    {
        return card;
    }

    void set_wallet(std::optional<PaymentMethodDataWalletInfo> value)
    {
        wallet = std::move(value);
    }

    [[nodiscard]] const std::optional<PaymentMethodDataWalletInfo>& get_wallet() const noexcept
    {
        return wallet;
    }

    void set_recurring_enabled(std::optional<bool> value)
    {
        recurring_enabled = std::move(value);
    }

    [[nodiscard]] const std::optional<bool>& get_recurring_enabled() const noexcept
    {
        return recurring_enabled;
    }

    void set_installment_payment_enabled(std::optional<bool> value)
    {
        installment_payment_enabled = std::move(value);
    }

    [[nodiscard]] const std::optional<bool>& get_installment_payment_enabled() const noexcept
    {
        return installment_payment_enabled;
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

    void set_metadata(std::optional<SecretSerdeValueValue> value)
    {
        metadata = std::move(value);
    }

    [[nodiscard]] const std::optional<SecretSerdeValueValue>& get_metadata() const noexcept
    {
        return metadata;
    }

    void set_created(std::optional<PrimitiveDateTimeValue> value)
    {
        created = std::move(value);
    }

    [[nodiscard]] const std::optional<PrimitiveDateTimeValue>& get_created() const noexcept
    {
        return created;
    }

    void set_bank_transfer(std::optional<BankValue> value)
    {
        bank_transfer = std::move(value);
    }

    [[nodiscard]] const std::optional<BankValue>& get_bank_transfer() const noexcept
    {
        return bank_transfer;
    }

    void set_last_used_at(std::optional<PrimitiveDateTimeValue> value)
    {
        last_used_at = std::move(value);
    }

    [[nodiscard]] const std::optional<PrimitiveDateTimeValue>& get_last_used_at() const noexcept
    {
        return last_used_at;
    }

    void set_client_secret(std::optional<std::string> value)
    {
        client_secret = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_client_secret() const noexcept
    {
        return client_secret;
    }

    MerchantIdValue merchant_id{};
    std::optional<CustomerIdValue> customer_id{};
    std::string payment_method_id{};
    std::optional<PaymentMethodValue> payment_method{};
    std::optional<PaymentMethodTypeValue> payment_method_type{};
    std::optional<CardDetailFromLocker> card{};
    std::optional<PaymentMethodDataWalletInfo> wallet{};
    std::optional<bool> recurring_enabled{};
    std::optional<bool> installment_payment_enabled{};
    std::optional<std::vector<PaymentExperienceValue>> payment_experience{};
    std::optional<SecretSerdeValueValue> metadata{};
    std::optional<PrimitiveDateTimeValue> created{};
    std::optional<BankValue> bank_transfer{};
    std::optional<PrimitiveDateTimeValue> last_used_at{};
    std::optional<std::string> client_secret{};
};

class ListCountriesCurrenciesResponse :
    public core::behavior::Serializer<ListCountriesCurrenciesResponse>
{
public:
    void set_currencies(std::vector<CurrencyValue> value)
    {
        currencies = std::move(value);
    }

    [[nodiscard]] const std::vector<CurrencyValue>& get_currencies() const noexcept
    {
        return currencies;
    }

    void set_countries(std::vector<CountryCodeWithName> value)
    {
        countries = std::move(value);
    }

    [[nodiscard]] const std::vector<CountryCodeWithName>& get_countries() const noexcept
    {
        return countries;
    }

    std::vector<CurrencyValue> currencies{};
    std::vector<CountryCodeWithName> countries{};
};

/** @brief Full ISO-4217 currency set — local substitute for api_enums::Currency::iter()
 *  (upstream core/payment_methods/cards.rs:7841-7855). */
class AllCurrencyCodes : public core::behavior::Serializer<AllCurrencyCodes>
{
public:
    std::vector<CurrencyValue> operator()() const
    {
        const std::vector<std::string> codes{
            "AED", "AFN", "ALL", "AMD", "ANG", "AOA", "ARS", "AUD", "AWG", "AZN", "BAM", "BBD",
            "BDT", "BGN", "BHD", "BIF", "BMD", "BND", "BOB", "BOV", "BRL", "BSD", "BTN", "BWP",
            "BYN", "BZD", "CAD", "CDF", "CHE", "CHF", "CHW", "CLF", "CLP", "CNY", "COP", "COU",
            "CRC", "CUC", "CUP", "CVE", "CZK", "DJF", "DKK", "DOP", "DZD", "EGP", "ERN", "ETB",
            "EUR", "FJD", "FKP", "GBP", "GEL", "GHS", "GIP", "GMD", "GNF", "GTQ", "GYD", "HKD",
            "HNL", "HRK", "HTG", "HUF", "IDR", "ILS", "INR", "IQD", "IRR", "ISK", "JMD", "JOD",
            "JPY", "KES", "KGS", "KHR", "KMF", "KPW", "KRW", "KWD", "KYD", "KZT", "LAK", "LBP",
            "LKR", "LRD", "LSL", "LYD", "MAD", "MDL", "MGA", "MKD", "MMK", "MNT", "MOP", "MRU",
            "MUR", "MVR", "MWK", "MXN", "MXV", "MYR", "MZN", "NAD", "NGN", "NIO", "NOK", "NPR",
            "NZD", "OMR", "PAB", "PEN", "PGK", "PHP", "PKR", "PLN", "PYG", "QAR", "RON", "RSD",
            "RUB", "RWF", "SAR", "SBD", "SCR", "SDG", "SEK", "SGD", "SHP", "SLL", "SOS", "SRD",
            "SSP", "STN", "SVC", "SYP", "SZL", "THB", "TJS", "TMT", "TND", "TOP", "TRY", "TTD",
            "TWD", "TZS", "UAH", "UGX", "USD", "USN", "UYI", "UYU", "UYW", "UZS", "VED", "VES",
            "VND", "VUV", "WST", "XAF", "XAG", "XAU", "XBA", "XBB", "XBC", "XBD", "XCD", "XDR",
            "XOF", "XPD", "XPF", "XPT", "XSU", "XTS", "XUA", "XXX", "YER", "ZAR", "ZMW", "ZWL",
        };
        std::vector<CurrencyValue> result{};
        result.reserve(codes.size());
        for (const auto& code: codes) {
            CurrencyValue value{};
            value.set_value(code);
            result.push_back(std::move(value));
        }
        return result;
    }
};

/** @brief Full ISO-3166-1 alpha-2 set — local substitute for api_enums::CountryAlpha2::iter()
 *  and common_enums::Country::from_alpha2 (upstream cards.rs:7843-7854). */
class AllCountryCodes : public core::behavior::Serializer<AllCountryCodes>
{
public:
    std::vector<CountryCodeWithName> operator()() const
    {
        const std::vector<std::pair<std::string, std::string>> entries{
            {"AD", "Andorra"},
            {"AE", "United Arab Emirates"},
            {"AF", "Afghanistan"},
            {"AG", "Antigua and Barbuda"},
            {"AI", "Anguilla"},
            {"AL", "Albania"},
            {"AM", "Armenia"},
            {"AO", "Angola"},
            {"AQ", "Antarctica"},
            {"AR", "Argentina"},
            {"AS", "American Samoa"},
            {"AT", "Austria"},
            {"AU", "Australia"},
            {"AW", "Aruba"},
            {"AX", "Aland Islands"},
            {"AZ", "Azerbaijan"},
            {"BA", "Bosnia and Herzegovina"},
            {"BB", "Barbados"},
            {"BD", "Bangladesh"},
            {"BE", "Belgium"},
            {"BF", "Burkina Faso"},
            {"BG", "Bulgaria"},
            {"BH", "Bahrain"},
            {"BI", "Burundi"},
            {"BJ", "Benin"},
            {"BL", "Saint Barthelemy"},
            {"BM", "Bermuda"},
            {"BN", "Brunei Darussalam"},
            {"BO", "Bolivia"},
            {"BQ", "Bonaire Sint Eustatius and Saba"},
            {"BR", "Brazil"},
            {"BS", "Bahamas"},
            {"BT", "Bhutan"},
            {"BV", "Bouvet Island"},
            {"BW", "Botswana"},
            {"BY", "Belarus"},
            {"BZ", "Belize"},
            {"CA", "Canada"},
            {"CC", "Cocos Islands"},
            {"CD", "Congo Democratic Republic of the"},
            {"CF", "Central African Republic"},
            {"CG", "Congo"},
            {"CH", "Switzerland"},
            {"CI", "Cote d'Ivoire"},
            {"CK", "Cook Islands"},
            {"CL", "Chile"},
            {"CM", "Cameroon"},
            {"CN", "China"},
            {"CO", "Colombia"},
            {"CR", "Costa Rica"},
            {"CU", "Cuba"},
            {"CV", "Cabo Verde"},
            {"CW", "Curacao"},
            {"CX", "Christmas Island"},
            {"CY", "Cyprus"},
            {"CZ", "Czechia"},
            {"DE", "Germany"},
            {"DJ", "Djibouti"},
            {"DK", "Denmark"},
            {"DM", "Dominica"},
            {"DO", "Dominican Republic"},
            {"DZ", "Algeria"},
            {"EC", "Ecuador"},
            {"EE", "Estonia"},
            {"EG", "Egypt"},
            {"EH", "Western Sahara"},
            {"ER", "Eritrea"},
            {"ES", "Spain"},
            {"ET", "Ethiopia"},
            {"FI", "Finland"},
            {"FJ", "Fiji"},
            {"FK", "Falkland Islands"},
            {"FM", "Micronesia"},
            {"FO", "Faroe Islands"},
            {"FR", "France"},
            {"GA", "Gabon"},
            {"GB", "United Kingdom"},
            {"GD", "Grenada"},
            {"GE", "Georgia"},
            {"GF", "French Guiana"},
            {"GG", "Guernsey"},
            {"GH", "Ghana"},
            {"GI", "Gibraltar"},
            {"GL", "Greenland"},
            {"GM", "Gambia"},
            {"GN", "Guinea"},
            {"GP", "Guadeloupe"},
            {"GQ", "Equatorial Guinea"},
            {"GR", "Greece"},
            {"GS", "South Georgia and the South Sandwich Islands"},
            {"GT", "Guatemala"},
            {"GU", "Guam"},
            {"GW", "Guinea-Bissau"},
            {"GY", "Guyana"},
            {"HK", "Hong Kong"},
            {"HM", "Heard Island and McDonald Islands"},
            {"HN", "Honduras"},
            {"HR", "Croatia"},
            {"HT", "Haiti"},
            {"HU", "Hungary"},
            {"ID", "Indonesia"},
            {"IE", "Ireland"},
            {"IL", "Israel"},
            {"IM", "Isle of Man"},
            {"IN", "India"},
            {"IO", "British Indian Ocean Territory"},
            {"IQ", "Iraq"},
            {"IR", "Iran"},
            {"IS", "Iceland"},
            {"IT", "Italy"},
            {"JE", "Jersey"},
            {"JM", "Jamaica"},
            {"JO", "Jordan"},
            {"JP", "Japan"},
            {"KE", "Kenya"},
            {"KG", "Kyrgyzstan"},
            {"KH", "Cambodia"},
            {"KI", "Kiribati"},
            {"KM", "Comoros"},
            {"KN", "Saint Kitts and Nevis"},
            {"KP", "Korea Democratic People's Republic of"},
            {"KR", "Korea Republic of"},
            {"KW", "Kuwait"},
            {"KY", "Cayman Islands"},
            {"KZ", "Kazakhstan"},
            {"LA", "Lao People's Democratic Republic"},
            {"LB", "Lebanon"},
            {"LC", "Saint Lucia"},
            {"LI", "Liechtenstein"},
            {"LK", "Sri Lanka"},
            {"LR", "Liberia"},
            {"LS", "Lesotho"},
            {"LT", "Lithuania"},
            {"LU", "Luxembourg"},
            {"LV", "Latvia"},
            {"LY", "Libya"},
            {"MA", "Morocco"},
            {"MC", "Monaco"},
            {"MD", "Moldova Republic of"},
            {"ME", "Montenegro"},
            {"MF", "Saint Martin"},
            {"MG", "Madagascar"},
            {"MH", "Marshall Islands"},
            {"MK", "North Macedonia"},
            {"ML", "Mali"},
            {"MM", "Myanmar"},
            {"MN", "Mongolia"},
            {"MO", "Macao"},
            {"MP", "Northern Mariana Islands"},
            {"MQ", "Martinique"},
            {"MR", "Mauritania"},
            {"MS", "Montserrat"},
            {"MT", "Malta"},
            {"MU", "Mauritius"},
            {"MV", "Maldives"},
            {"MW", "Malawi"},
            {"MX", "Mexico"},
            {"MY", "Malaysia"},
            {"MZ", "Mozambique"},
            {"NA", "Namibia"},
            {"NC", "New Caledonia"},
            {"NE", "Niger"},
            {"NF", "Norfolk Island"},
            {"NG", "Nigeria"},
            {"NI", "Nicaragua"},
            {"NL", "Netherlands"},
            {"NO", "Norway"},
            {"NP", "Nepal"},
            {"NR", "Nauru"},
            {"NU", "Niue"},
            {"NZ", "New Zealand"},
            {"OM", "Oman"},
            {"PA", "Panama"},
            {"PE", "Peru"},
            {"PF", "French Polynesia"},
            {"PG", "Papua New Guinea"},
            {"PH", "Philippines"},
            {"PK", "Pakistan"},
            {"PL", "Poland"},
            {"PM", "Saint Pierre and Miquelon"},
            {"PN", "Pitcairn"},
            {"PR", "Puerto Rico"},
            {"PS", "Palestine State of"},
            {"PT", "Portugal"},
            {"PW", "Palau"},
            {"PY", "Paraguay"},
            {"QA", "Qatar"},
            {"RE", "Reunion"},
            {"RO", "Romania"},
            {"RS", "Serbia"},
            {"RU", "Russian Federation"},
            {"RW", "Rwanda"},
            {"SA", "Saudi Arabia"},
            {"SB", "Solomon Islands"},
            {"SC", "Seychelles"},
            {"SD", "Sudan"},
            {"SE", "Sweden"},
            {"SG", "Singapore"},
            {"SH", "Saint Helena Ascension and Tristan da Cunha"},
            {"SI", "Slovenia"},
            {"SJ", "Svalbard and Jan Mayen"},
            {"SK", "Slovakia"},
            {"SL", "Sierra Leone"},
            {"SM", "San Marino"},
            {"SN", "Senegal"},
            {"SO", "Somalia"},
            {"SR", "Suriname"},
            {"SS", "South Sudan"},
            {"ST", "Sao Tome and Principe"},
            {"SV", "El Salvador"},
            {"SX", "Sint Maarten"},
            {"SY", "Syrian Arab Republic"},
            {"SZ", "Eswatini"},
            {"TC", "Turks and Caicos Islands"},
            {"TD", "Chad"},
            {"TF", "French Southern Territories"},
            {"TG", "Togo"},
            {"TH", "Thailand"},
            {"TJ", "Tajikistan"},
            {"TK", "Tokelau"},
            {"TL", "Timor-Leste"},
            {"TM", "Turkmenistan"},
            {"TN", "Tunisia"},
            {"TO", "Tonga"},
            {"TR", "Turkey"},
            {"TT", "Trinidad and Tobago"},
            {"TV", "Tuvalu"},
            {"TW", "Taiwan Province of China"},
            {"TZ", "Tanzania United Republic of"},
            {"UA", "Ukraine"},
            {"UG", "Uganda"},
            {"UM", "United States Minor Outlying Islands"},
            {"US", "United States of America"},
            {"UY", "Uruguay"},
            {"UZ", "Uzbekistan"},
            {"VA", "Holy See"},
            {"VC", "Saint Vincent and the Grenadines"},
            {"VE", "Venezuela Bolivarian Republic of"},
            {"VG", "Virgin Islands British"},
            {"VI", "Virgin Islands U.S."},
            {"VN", "Viet Nam"},
            {"VU", "Vanuatu"},
            {"WF", "Wallis and Futuna"},
            {"WS", "Samoa"},
            {"YE", "Yemen"},
            {"YT", "Mayotte"},
            {"ZA", "South Africa"},
            {"ZM", "Zambia"},
            {"ZW", "Zimbabwe"},
        };
        std::vector<CountryCodeWithName> result{};
        result.reserve(entries.size());
        for (const auto& [code, name]: entries) {
            CountryAlpha2Value alpha{};
            alpha.set_value(code);
            CountryValue country{};
            country.set_value(name);
            CountryCodeWithName item{};
            item.set_code(std::move(alpha));
            item.set_name(std::move(country));
            result.push_back(std::move(item));
        }
        return result;
    }
};

class LockerCardResponse : public core::behavior::Serializer<LockerCardResponse>
{
public:
    void set_card(Card value)
    {
        card = std::move(value);
    }

    [[nodiscard]] const Card& get_card() const noexcept
    {
        return card;
    }

    void set_metadata(std::optional<SecretSerdeValueValue> value)
    {
        metadata = std::move(value);
    }

    [[nodiscard]] const std::optional<SecretSerdeValueValue>& get_metadata() const noexcept
    {
        return metadata;
    }

    Card card{};
    std::optional<SecretSerdeValueValue> metadata{};
};

class MigrateNetworkTokenDetail : public core::behavior::Serializer<MigrateNetworkTokenDetail>
{
public:
    void set_network_token_data(MigrateNetworkTokenData value)
    {
        network_token_data = std::move(value);
    }

    [[nodiscard]] const MigrateNetworkTokenData& get_network_token_data() const noexcept
    {
        return network_token_data;
    }

    void set_network_token_requestor_ref_id(std::string value)
    {
        network_token_requestor_ref_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_network_token_requestor_ref_id() const noexcept
    {
        return network_token_requestor_ref_id;
    }

    MigrateNetworkTokenData network_token_data{};
    std::string network_token_requestor_ref_id{};
};

class ModularPaymentMethodMigrationResponse :
    public core::behavior::Serializer<ModularPaymentMethodMigrationResponse>
{
public:
    void set_total_rows(std::size_t value)
    {
        total_rows = std::move(value);
    }

    [[nodiscard]] const std::size_t& get_total_rows() const noexcept
    {
        return total_rows;
    }

    void set_successful_count(std::size_t value)
    {
        successful_count = std::move(value);
    }

    [[nodiscard]] const std::size_t& get_successful_count() const noexcept
    {
        return successful_count;
    }

    void set_failed_count(std::size_t value)
    {
        failed_count = std::move(value);
    }

    [[nodiscard]] const std::size_t& get_failed_count() const noexcept
    {
        return failed_count;
    }

    void set_results(std::vector<ModularPaymentMethodMigrationRowResult> value)
    {
        results = std::move(value);
    }

    [[nodiscard]] const std::vector<ModularPaymentMethodMigrationRowResult>&
    get_results() const noexcept
    {
        return results;
    }

    std::size_t total_rows{};
    std::size_t successful_count{};
    std::size_t failed_count{};
    std::vector<ModularPaymentMethodMigrationRowResult> results{};
};

class NetworkTokenResponse : public core::behavior::Serializer<NetworkTokenResponse>
{
public:
    void set_payment_method_data(NetworkTokenDetailsPaymentMethod value)
    {
        payment_method_data = std::move(value);
    }

    [[nodiscard]] const NetworkTokenDetailsPaymentMethod& get_payment_method_data() const noexcept
    {
        return payment_method_data;
    }

    NetworkTokenDetailsPaymentMethod payment_method_data{};
};

enum class PaymentMethodCreateData
{
    Card,
    ProxyCard,
    BankDebit,
    Wallet,
    BankRedirect
};


enum class PaymentMethodListData
{
    Card,
    BankDebit,
    Bank,
    Wallet
};

class PaymentMethodListInstallmentPlan :
    public core::behavior::Serializer<PaymentMethodListInstallmentPlan>
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

    void set_interest_rate(InstallmentInterestRateValue value)
    {
        interest_rate = std::move(value);
    }

    [[nodiscard]] const InstallmentInterestRateValue& get_interest_rate() const noexcept
    {
        return interest_rate;
    }

    void set_amount_details(PaymentMethodListInstallmentAmountDetails value)
    {
        amount_details = std::move(value);
    }

    [[nodiscard]] const PaymentMethodListInstallmentAmountDetails&
    get_amount_details() const noexcept
    {
        return amount_details;
    }

    NonZeroU8Value number_of_installments{};
    BillingFrequencyValue billing_frequency{};
    InstallmentInterestRateValue interest_rate{};
    PaymentMethodListInstallmentAmountDetails amount_details{};
};

class PaymentMethodResponse : public core::behavior::Serializer<PaymentMethodResponse>
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

    void set_customer_id(std::optional<CustomerIdValue> value)
    {
        customer_id = std::move(value);
    }

    [[nodiscard]] const std::optional<CustomerIdValue>& get_customer_id() const noexcept
    {
        return customer_id;
    }

    void set_payment_method_id(std::string value)
    {
        payment_method_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_payment_method_id() const noexcept
    {
        return payment_method_id;
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

    void set_card(std::optional<CardDetailFromLocker> value)
    {
        card = std::move(value);
    }

    [[nodiscard]] const std::optional<CardDetailFromLocker>& get_card() const noexcept
    {
        return card;
    }

    void set_recurring_enabled(std::optional<bool> value)
    {
        recurring_enabled = std::move(value);
    }

    [[nodiscard]] const std::optional<bool>& get_recurring_enabled() const noexcept
    {
        return recurring_enabled;
    }

    void set_installment_payment_enabled(std::optional<bool> value)
    {
        installment_payment_enabled = std::move(value);
    }

    [[nodiscard]] const std::optional<bool>& get_installment_payment_enabled() const noexcept
    {
        return installment_payment_enabled;
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

    void set_metadata(std::optional<SecretSerdeValueValue> value)
    {
        metadata = std::move(value);
    }

    [[nodiscard]] const std::optional<SecretSerdeValueValue>& get_metadata() const noexcept
    {
        return metadata;
    }

    void set_created(std::optional<PrimitiveDateTimeValue> value)
    {
        created = std::move(value);
    }

    [[nodiscard]] const std::optional<PrimitiveDateTimeValue>& get_created() const noexcept
    {
        return created;
    }

    void set_bank_transfer(std::optional<BankValue> value)
    {
        bank_transfer = std::move(value);
    }

    [[nodiscard]] const std::optional<BankValue>& get_bank_transfer() const noexcept
    {
        return bank_transfer;
    }

    void set_last_used_at(std::optional<PrimitiveDateTimeValue> value)
    {
        last_used_at = std::move(value);
    }

    [[nodiscard]] const std::optional<PrimitiveDateTimeValue>& get_last_used_at() const noexcept
    {
        return last_used_at;
    }

    void set_client_secret(std::optional<std::string> value)
    {
        client_secret = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_client_secret() const noexcept
    {
        return client_secret;
    }

    MerchantIdValue merchant_id{};
    std::optional<CustomerIdValue> customer_id{};
    std::string payment_method_id{};
    std::optional<PaymentMethodValue> payment_method{};
    std::optional<PaymentMethodTypeValue> payment_method_type{};
    std::optional<CardDetailFromLocker> card{};
    std::optional<bool> recurring_enabled{};
    std::optional<bool> installment_payment_enabled{};
    std::optional<std::vector<PaymentExperienceValue>> payment_experience{};
    std::optional<SecretSerdeValueValue> metadata{};
    std::optional<PrimitiveDateTimeValue> created{};
    std::optional<BankValue> bank_transfer{};
    std::optional<PrimitiveDateTimeValue> last_used_at{};
    std::optional<std::string> client_secret{};
};

enum class PaymentMethodResponseData
{
    Card,
    BankDebit,
    Wallet
};

enum class PaymentMethodSubtypeSpecificData
{
    Card,
    Bank
};

enum class PaymentMethodSubtypeSpecificDataForClient
{
    Card,
    Bank
};

class PaymentMethodUpdate : public core::behavior::Serializer<PaymentMethodUpdate>
{
public:
    void set_update_last_used_at(std::optional<bool> value)
    {
        update_last_used_at = std::move(value);
    }

    [[nodiscard]] const std::optional<bool>& get_update_last_used_at() const noexcept
    {
        return update_last_used_at;
    }

    void set_card(std::optional<CardDetailUpdate> value)
    {
        card = std::move(value);
    }

    [[nodiscard]] const std::optional<CardDetailUpdate>& get_card() const noexcept
    {
        return card;
    }

    void set_wallet(std::optional<PaymentMethodDataWalletInfo> value)
    {
        wallet = std::move(value);
    }

    [[nodiscard]] const std::optional<PaymentMethodDataWalletInfo>& get_wallet() const noexcept
    {
        return wallet;
    }

    void set_client_secret(std::optional<std::string> value)
    {
        client_secret = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_client_secret() const noexcept
    {
        return client_secret;
    }

    std::optional<bool> update_last_used_at{};
    std::optional<CardDetailUpdate> card{};
    std::optional<PaymentMethodDataWalletInfo> wallet{};
    std::optional<std::string> client_secret{};
};

enum class PaymentMethodUpdateData
{
    Card,
    BankDebit,
    Wallet
};

enum class PaymentMethodsData
{
    Card,
    BankDetails,
    WalletDetails,
    BankDebit
};

class PaymentsMandateReference : public core::behavior::Serializer<PaymentsMandateReference>
{
public:
    void set_field0(std::unordered_map<std::string, PaymentsMandateReferenceRecord> value)
    {
        field0 = std::move(value);
    }

    [[nodiscard]] const std::unordered_map<std::string, PaymentsMandateReferenceRecord>&
    get_field0() const noexcept
    {
        return field0;
    }

    std::unordered_map<std::string, PaymentsMandateReferenceRecord> field0{};
};

class RawCardWithNTDetails : public core::behavior::Serializer<RawCardWithNTDetails>
{
public:
    void set_card_details(CardDetail value)
    {
        card_details = std::move(value);
    }

    [[nodiscard]] const CardDetail& get_card_details() const noexcept
    {
        return card_details;
    }

    void set_network_token_details(CardDetail value)
    {
        network_token_details = std::move(value);
    }

    [[nodiscard]] const CardDetail& get_network_token_details() const noexcept
    {
        return network_token_details;
    }

    CardDetail card_details{};
    CardDetail network_token_details{};
};

class CardNetworkTokenizeRequest : public core::behavior::Serializer<CardNetworkTokenizeRequest>
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

    void set_data(TokenizeDataRequest value)
    {
        data = std::move(value);
    }

    [[nodiscard]] const TokenizeDataRequest& get_data() const noexcept
    {
        return data;
    }

    void set_customer(CustomerDetailsValue value)
    {
        customer = std::move(value);
    }

    [[nodiscard]] const CustomerDetailsValue& get_customer() const noexcept
    {
        return customer;
    }

    void set_billing(std::optional<AddressValue> value)
    {
        billing = std::move(value);
    }

    [[nodiscard]] const std::optional<AddressValue>& get_billing() const noexcept
    {
        return billing;
    }

    void set_metadata(std::optional<SecretSerdeValueValue> value)
    {
        metadata = std::move(value);
    }

    [[nodiscard]] const std::optional<SecretSerdeValueValue>& get_metadata() const noexcept
    {
        return metadata;
    }

    void set_payment_method_issuer(std::optional<std::string> value)
    {
        payment_method_issuer = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_payment_method_issuer() const noexcept
    {
        return payment_method_issuer;
    }

    MerchantIdValue merchant_id{};
    TokenizeDataRequest data{};
    CustomerDetailsValue customer{};
    std::optional<AddressValue> billing{};
    std::optional<SecretSerdeValueValue> metadata{};
    std::optional<std::string> payment_method_issuer{};
};

class CardNetworkTokenizeResponse : public core::behavior::Serializer<CardNetworkTokenizeResponse>
{
public:
    void set_payment_method_response(std::optional<PaymentMethodResponse> value)
    {
        payment_method_response = std::move(value);
    }

    [[nodiscard]] const std::optional<PaymentMethodResponse>&
    get_payment_method_response() const noexcept
    {
        return payment_method_response;
    }

    void set_customer(std::optional<CustomerDetailsValue> value)
    {
        customer = std::move(value);
    }

    [[nodiscard]] const std::optional<CustomerDetailsValue>& get_customer() const noexcept
    {
        return customer;
    }

    void set_card_tokenized(bool value)
    {
        card_tokenized = std::move(value);
    }

    [[nodiscard]] const bool& get_card_tokenized() const noexcept
    {
        return card_tokenized;
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

    void set_tokenization_data(std::optional<TokenizeDataRequest> value)
    {
        tokenization_data = std::move(value);
    }

    [[nodiscard]] const std::optional<TokenizeDataRequest>& get_tokenization_data() const noexcept
    {
        return tokenization_data;
    }

    std::optional<PaymentMethodResponse> payment_method_response{};
    std::optional<CustomerDetailsValue> customer{};
    bool card_tokenized{};
    std::optional<std::string> error_code{};
    std::optional<std::string> error_message{};
    std::optional<TokenizeDataRequest> tokenization_data{};
};

class CommonMandateReference : public core::behavior::Serializer<CommonMandateReference>
{
public:
    void set_payments(std::optional<PaymentsMandateReference> value)
    {
        payments = std::move(value);
    }

    [[nodiscard]] const std::optional<PaymentsMandateReference>& get_payments() const noexcept
    {
        return payments;
    }

    void set_payouts(std::optional<PayoutsMandateReference> value)
    {
        payouts = std::move(value);
    }

    [[nodiscard]] const std::optional<PayoutsMandateReference>& get_payouts() const noexcept
    {
        return payouts;
    }

    std::optional<PaymentsMandateReference> payments{};
    std::optional<PayoutsMandateReference> payouts{};
};

class CustomerPaymentMethodForClient :
    public core::behavior::Serializer<CustomerPaymentMethodForClient>
{
public:
    void set_payment_token(std::string value)
    {
        payment_token = std::move(value);
    }

    [[nodiscard]] const std::string& get_payment_token() const noexcept
    {
        return payment_token;
    }

    void set_payment_method(PaymentMethodValue value)
    {
        payment_method = std::move(value);
    }

    [[nodiscard]] const PaymentMethodValue& get_payment_method() const noexcept
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

    void set_default_payment_method_set(bool value)
    {
        default_payment_method_set = std::move(value);
    }

    [[nodiscard]] const bool& get_default_payment_method_set() const noexcept
    {
        return default_payment_method_set;
    }

    void set_requires_cvv(bool value)
    {
        requires_cvv = std::move(value);
    }

    [[nodiscard]] const bool& get_requires_cvv() const noexcept
    {
        return requires_cvv;
    }

    void set_created(std::optional<PrimitiveDateTimeValue> value)
    {
        created = std::move(value);
    }

    [[nodiscard]] const std::optional<PrimitiveDateTimeValue>& get_created() const noexcept
    {
        return created;
    }

    void set_last_used_at(std::optional<PrimitiveDateTimeValue> value)
    {
        last_used_at = std::move(value);
    }

    [[nodiscard]] const std::optional<PrimitiveDateTimeValue>& get_last_used_at() const noexcept
    {
        return last_used_at;
    }

    void set_recurring_enabled(std::optional<bool> value)
    {
        recurring_enabled = std::move(value);
    }

    [[nodiscard]] const std::optional<bool>& get_recurring_enabled() const noexcept
    {
        return recurring_enabled;
    }

    void set_payment_method_data(std::optional<CustomerPaymentMethodDataForClient> value)
    {
        payment_method_data = std::move(value);
    }

    [[nodiscard]] const std::optional<CustomerPaymentMethodDataForClient>&
    get_payment_method_data() const noexcept
    {
        return payment_method_data;
    }

    std::string payment_token{};
    PaymentMethodValue payment_method{};
    std::optional<PaymentMethodTypeValue> payment_method_type{};
    bool default_payment_method_set{};
    bool requires_cvv{};
    std::optional<PrimitiveDateTimeValue> created{};
    std::optional<PrimitiveDateTimeValue> last_used_at{};
    std::optional<bool> recurring_enabled{};
    std::optional<CustomerPaymentMethodDataForClient> payment_method_data{};
};

class CustomerPaymentMethodResponseItem :
    public core::behavior::Serializer<CustomerPaymentMethodResponseItem>
{
public:
    void set_payment_method_token(std::string value)
    {
        payment_method_token = std::move(value);
    }

    [[nodiscard]] const std::string& get_payment_method_token() const noexcept
    {
        return payment_method_token;
    }

    void set_customer_id(GlobalCustomerIdValue value)
    {
        customer_id = std::move(value);
    }

    [[nodiscard]] const GlobalCustomerIdValue& get_customer_id() const noexcept
    {
        return customer_id;
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

    void set_recurring_enabled(bool value)
    {
        recurring_enabled = std::move(value);
    }

    [[nodiscard]] const bool& get_recurring_enabled() const noexcept
    {
        return recurring_enabled;
    }

    void set_payment_method_data(std::optional<PaymentMethodListData> value)
    {
        payment_method_data = std::move(value);
    }

    [[nodiscard]] const std::optional<PaymentMethodListData>&
    get_payment_method_data() const noexcept
    {
        return payment_method_data;
    }

    void set_bank(std::optional<MaskedBankDetails> value)
    {
        bank = std::move(value);
    }

    [[nodiscard]] const std::optional<MaskedBankDetails>& get_bank() const noexcept
    {
        return bank;
    }

    void set_created(PrimitiveDateTimeValue value)
    {
        created = std::move(value);
    }

    [[nodiscard]] const PrimitiveDateTimeValue& get_created() const noexcept
    {
        return created;
    }

    void set_requires_cvv(bool value)
    {
        requires_cvv = std::move(value);
    }

    [[nodiscard]] const bool& get_requires_cvv() const noexcept
    {
        return requires_cvv;
    }

    void set_last_used_at(PrimitiveDateTimeValue value)
    {
        last_used_at = std::move(value);
    }

    [[nodiscard]] const PrimitiveDateTimeValue& get_last_used_at() const noexcept
    {
        return last_used_at;
    }

    void set_is_default(bool value)
    {
        is_default = std::move(value);
    }

    [[nodiscard]] const bool& get_is_default() const noexcept
    {
        return is_default;
    }

    void set_billing(std::optional<AddressValue> value)
    {
        billing = std::move(value);
    }

    [[nodiscard]] const std::optional<AddressValue>& get_billing() const noexcept
    {
        return billing;
    }

    std::string payment_method_token{};
    GlobalCustomerIdValue customer_id{};
    PaymentMethodValue payment_method_type{};
    std::optional<PaymentMethodTypeValue> payment_method_subtype{};
    bool recurring_enabled{};
    std::optional<PaymentMethodListData> payment_method_data{};
    std::optional<MaskedBankDetails> bank{};
    PrimitiveDateTimeValue created{};
    bool requires_cvv{};
    PrimitiveDateTimeValue last_used_at{};
    bool is_default{};
    std::optional<AddressValue> billing{};
};

class CustomerPaymentMethodsListResponse :
    public core::behavior::Serializer<CustomerPaymentMethodsListResponse>
{
public:
    void set_customer_payment_methods(std::vector<CustomerPaymentMethod> value)
    {
        customer_payment_methods = std::move(value);
    }

    [[nodiscard]] const std::vector<CustomerPaymentMethod>&
    get_customer_payment_methods() const noexcept
    {
        return customer_payment_methods;
    }

    void set_is_guest_customer(std::optional<bool> value)
    {
        is_guest_customer = std::move(value);
    }

    [[nodiscard]] const std::optional<bool>& get_is_guest_customer() const noexcept
    {
        return is_guest_customer;
    }

    std::vector<CustomerPaymentMethod> customer_payment_methods{};
    std::optional<bool> is_guest_customer{};
};

class PaymentMethodCreate : public core::behavior::Serializer<PaymentMethodCreate>
{
public:
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

    void set_payment_method_issuer(std::optional<std::string> value)
    {
        payment_method_issuer = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_payment_method_issuer() const noexcept
    {
        return payment_method_issuer;
    }

    void set_payment_method_issuer_code(std::optional<PaymentMethodIssuerCodeValue> value)
    {
        payment_method_issuer_code = std::move(value);
    }

    [[nodiscard]] const std::optional<PaymentMethodIssuerCodeValue>&
    get_payment_method_issuer_code() const noexcept
    {
        return payment_method_issuer_code;
    }

    void set_card(std::optional<CardDetail> value)
    {
        card = std::move(value);
    }

    [[nodiscard]] const std::optional<CardDetail>& get_card() const noexcept
    {
        return card;
    }

    void set_metadata(std::optional<SecretSerdeValueValue> value)
    {
        metadata = std::move(value);
    }

    [[nodiscard]] const std::optional<SecretSerdeValueValue>& get_metadata() const noexcept
    {
        return metadata;
    }

    void set_customer_id(std::optional<CustomerIdValue> value)
    {
        customer_id = std::move(value);
    }

    [[nodiscard]] const std::optional<CustomerIdValue>& get_customer_id() const noexcept
    {
        return customer_id;
    }

    void set_card_network(std::optional<std::string> value)
    {
        card_network = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_card_network() const noexcept
    {
        return card_network;
    }

    void set_bank_transfer(std::optional<BankValue> value)
    {
        bank_transfer = std::move(value);
    }

    [[nodiscard]] const std::optional<BankValue>& get_bank_transfer() const noexcept
    {
        return bank_transfer;
    }

    void set_bank_transfer_data(std::optional<BankTransferValue> value)
    {
        bank_transfer_data = std::move(value);
    }

    [[nodiscard]] const std::optional<BankTransferValue>& get_bank_transfer_data() const noexcept
    {
        return bank_transfer_data;
    }

    void set_wallet(std::optional<WalletValue> value)
    {
        wallet = std::move(value);
    }

    [[nodiscard]] const std::optional<WalletValue>& get_wallet() const noexcept
    {
        return wallet;
    }

    void set_client_secret(std::optional<std::string> value)
    {
        client_secret = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_client_secret() const noexcept
    {
        return client_secret;
    }

    void set_payment_method_data(std::optional<PaymentMethodCreateData> value)
    {
        payment_method_data = std::move(value);
    }

    [[nodiscard]] const std::optional<PaymentMethodCreateData>&
    get_payment_method_data() const noexcept
    {
        return payment_method_data;
    }

    void set_billing(std::optional<AddressValue> value)
    {
        billing = std::move(value);
    }

    [[nodiscard]] const std::optional<AddressValue>& get_billing() const noexcept
    {
        return billing;
    }

    void set_connector_mandate_details(std::optional<PaymentsMandateReference> value)
    {
        connector_mandate_details = std::move(value);
    }

    [[nodiscard]] const std::optional<PaymentsMandateReference>&
    get_connector_mandate_details() const noexcept
    {
        return connector_mandate_details;
    }

    void set_network_transaction_id(std::optional<std::string> value)
    {
        network_transaction_id = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_network_transaction_id() const noexcept
    {
        return network_transaction_id;
    }

    std::optional<PaymentMethodValue> payment_method{};
    std::optional<PaymentMethodTypeValue> payment_method_type{};
    std::optional<std::string> payment_method_issuer{};
    std::optional<PaymentMethodIssuerCodeValue> payment_method_issuer_code{};
    std::optional<CardDetail> card{};
    std::optional<SecretSerdeValueValue> metadata{};
    std::optional<CustomerIdValue> customer_id{};
    std::optional<std::string> card_network{};
    std::optional<BankValue> bank_transfer{};
    std::optional<BankTransferValue> bank_transfer_data{};
    std::optional<WalletValue> wallet{};
    std::optional<std::string> client_secret{};
    std::optional<PaymentMethodCreateData> payment_method_data{};
    std::optional<AddressValue> billing{};
    std::optional<PaymentsMandateReference> connector_mandate_details{};
    std::optional<std::string> network_transaction_id{};
};

class PaymentMethodDetailsResponse : public core::behavior::Serializer<PaymentMethodDetailsResponse>
{
public:
    void set_id(GlobalPaymentMethodIdValue value)
    {
        id = std::move(value);
    }

    [[nodiscard]] const GlobalPaymentMethodIdValue& get_id() const noexcept
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

    void set_customer_id(std::optional<GlobalCustomerIdValue> value)
    {
        customer_id = std::move(value);
    }

    [[nodiscard]] const std::optional<GlobalCustomerIdValue>& get_customer_id() const noexcept
    {
        return customer_id;
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

    void set_recurring_enabled(std::optional<bool> value)
    {
        recurring_enabled = std::move(value);
    }

    [[nodiscard]] const std::optional<bool>& get_recurring_enabled() const noexcept
    {
        return recurring_enabled;
    }

    void set_created(std::optional<PrimitiveDateTimeValue> value)
    {
        created = std::move(value);
    }

    [[nodiscard]] const std::optional<PrimitiveDateTimeValue>& get_created() const noexcept
    {
        return created;
    }

    void set_last_used_at(std::optional<PrimitiveDateTimeValue> value)
    {
        last_used_at = std::move(value);
    }

    [[nodiscard]] const std::optional<PrimitiveDateTimeValue>& get_last_used_at() const noexcept
    {
        return last_used_at;
    }

    void set_payment_method_data(std::optional<PaymentMethodResponseData> value)
    {
        payment_method_data = std::move(value);
    }

    [[nodiscard]] const std::optional<PaymentMethodResponseData>&
    get_payment_method_data() const noexcept
    {
        return payment_method_data;
    }

    void set_connector_tokens(std::optional<std::vector<ConnectorTokenDetails>> value)
    {
        connector_tokens = std::move(value);
    }

    [[nodiscard]] const std::optional<std::vector<ConnectorTokenDetails>>&
    get_connector_tokens() const noexcept
    {
        return connector_tokens;
    }

    void set_network_token(std::optional<NetworkTokenResponse> value)
    {
        network_token = std::move(value);
    }

    [[nodiscard]] const std::optional<NetworkTokenResponse>& get_network_token() const noexcept
    {
        return network_token;
    }

    void set_storage_type(StorageTypeValue value)
    {
        storage_type = std::move(value);
    }

    [[nodiscard]] const StorageTypeValue& get_storage_type() const noexcept
    {
        return storage_type;
    }

    void set_network_transaction_id(std::optional<std::string> value)
    {
        network_transaction_id = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_network_transaction_id() const noexcept
    {
        return network_transaction_id;
    }

    void set_billing(std::optional<AddressValue> value)
    {
        billing = std::move(value);
    }

    [[nodiscard]] const std::optional<AddressValue>& get_billing() const noexcept
    {
        return billing;
    }

    void set_acknowledgement_status(std::optional<AcknowledgementStatusValue> value)
    {
        acknowledgement_status = std::move(value);
    }

    [[nodiscard]] const std::optional<AcknowledgementStatusValue>&
    get_acknowledgement_status() const noexcept
    {
        return acknowledgement_status;
    }

    GlobalPaymentMethodIdValue id{};
    MerchantIdValue merchant_id{};
    std::optional<GlobalCustomerIdValue> customer_id{};
    std::optional<PaymentMethodValue> payment_method_type{};
    std::optional<PaymentMethodTypeValue> payment_method_subtype{};
    std::optional<bool> recurring_enabled{};
    std::optional<PrimitiveDateTimeValue> created{};
    std::optional<PrimitiveDateTimeValue> last_used_at{};
    std::optional<PaymentMethodResponseData> payment_method_data{};
    std::optional<std::vector<ConnectorTokenDetails>> connector_tokens{};
    std::optional<NetworkTokenResponse> network_token{};
    StorageTypeValue storage_type{};
    std::optional<std::string> network_transaction_id{};
    std::optional<AddressValue> billing{};
    std::optional<AcknowledgementStatusValue> acknowledgement_status{};
};

class PaymentMethodIntentConfirm : public core::behavior::Serializer<PaymentMethodIntentConfirm>
{
public:
    void set_customer_id(std::optional<CustomerIdValue> value)
    {
        customer_id = std::move(value);
    }

    [[nodiscard]] const std::optional<CustomerIdValue>& get_customer_id() const noexcept
    {
        return customer_id;
    }

    void set_payment_method_data(PaymentMethodCreateData value)
    {
        payment_method_data = std::move(value);
    }

    [[nodiscard]] const PaymentMethodCreateData& get_payment_method_data() const noexcept
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

    std::optional<CustomerIdValue> customer_id{};
    PaymentMethodCreateData payment_method_data{};
    PaymentMethodValue payment_method_type{};
    PaymentMethodTypeValue payment_method_subtype{};
};

class PaymentMethodListInstallmentOption :
    public core::behavior::Serializer<PaymentMethodListInstallmentOption>
{
public:
    void set_payment_method(PaymentMethodValue value)
    {
        payment_method = std::move(value);
    }

    [[nodiscard]] const PaymentMethodValue& get_payment_method() const noexcept
    {
        return payment_method;
    }

    void set_available_plans(std::vector<PaymentMethodListInstallmentPlan> value)
    {
        available_plans = std::move(value);
    }

    [[nodiscard]] const std::vector<PaymentMethodListInstallmentPlan>&
    get_available_plans() const noexcept
    {
        return available_plans;
    }

    PaymentMethodValue payment_method{};
    std::vector<PaymentMethodListInstallmentPlan> available_plans{};
};

class PaymentMethodMigrateResponse : public core::behavior::Serializer<PaymentMethodMigrateResponse>
{
public:
    void set_payment_method_response(PaymentMethodResponse value)
    {
        payment_method_response = std::move(value);
    }

    [[nodiscard]] const PaymentMethodResponse& get_payment_method_response() const noexcept
    {
        return payment_method_response;
    }

    void set_card_migrated(std::optional<bool> value)
    {
        card_migrated = std::move(value);
    }

    [[nodiscard]] const std::optional<bool>& get_card_migrated() const noexcept
    {
        return card_migrated;
    }

    void set_payment_method_migrated(std::optional<bool> value)
    {
        payment_method_migrated = std::move(value);
    }

    [[nodiscard]] const std::optional<bool>& get_payment_method_migrated() const noexcept
    {
        return payment_method_migrated;
    }

    void set_network_token_migrated(std::optional<bool> value)
    {
        network_token_migrated = std::move(value);
    }

    [[nodiscard]] const std::optional<bool>& get_network_token_migrated() const noexcept
    {
        return network_token_migrated;
    }

    void set_connector_mandate_details_migrated(std::optional<bool> value)
    {
        connector_mandate_details_migrated = std::move(value);
    }

    [[nodiscard]] const std::optional<bool>& get_connector_mandate_details_migrated() const noexcept
    {
        return connector_mandate_details_migrated;
    }

    void set_network_transaction_id_migrated(std::optional<bool> value)
    {
        network_transaction_id_migrated = std::move(value);
    }

    [[nodiscard]] const std::optional<bool>& get_network_transaction_id_migrated() const noexcept
    {
        return network_transaction_id_migrated;
    }

    PaymentMethodResponse payment_method_response{};
    std::optional<bool> card_migrated{};
    std::optional<bool> payment_method_migrated{};
    std::optional<bool> network_token_migrated{};
    std::optional<bool> connector_mandate_details_migrated{};
    std::optional<bool> network_transaction_id_migrated{};
};

class PaymentMethodResponseItem : public core::behavior::Serializer<PaymentMethodResponseItem>
{
public:
    void set_id(GlobalPaymentMethodIdValue value)
    {
        id = std::move(value);
    }

    [[nodiscard]] const GlobalPaymentMethodIdValue& get_id() const noexcept
    {
        return id;
    }

    void set_customer_id(GlobalCustomerIdValue value)
    {
        customer_id = std::move(value);
    }

    [[nodiscard]] const GlobalCustomerIdValue& get_customer_id() const noexcept
    {
        return customer_id;
    }

    void set_merchant_customer_reference_id(std::optional<CustomerIdValue> value)
    {
        merchant_customer_reference_id = std::move(value);
    }

    [[nodiscard]] const std::optional<CustomerIdValue>&
    get_merchant_customer_reference_id() const noexcept
    {
        return merchant_customer_reference_id;
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

    void set_recurring_enabled(std::optional<bool> value)
    {
        recurring_enabled = std::move(value);
    }

    [[nodiscard]] const std::optional<bool>& get_recurring_enabled() const noexcept
    {
        return recurring_enabled;
    }

    void set_payment_method_data(std::optional<PaymentMethodListData> value)
    {
        payment_method_data = std::move(value);
    }

    [[nodiscard]] const std::optional<PaymentMethodListData>&
    get_payment_method_data() const noexcept
    {
        return payment_method_data;
    }

    void set_created(PrimitiveDateTimeValue value)
    {
        created = std::move(value);
    }

    [[nodiscard]] const PrimitiveDateTimeValue& get_created() const noexcept
    {
        return created;
    }

    void set_requires_cvv(bool value)
    {
        requires_cvv = std::move(value);
    }

    [[nodiscard]] const bool& get_requires_cvv() const noexcept
    {
        return requires_cvv;
    }

    void set_last_used_at(PrimitiveDateTimeValue value)
    {
        last_used_at = std::move(value);
    }

    [[nodiscard]] const PrimitiveDateTimeValue& get_last_used_at() const noexcept
    {
        return last_used_at;
    }

    void set_is_default(bool value)
    {
        is_default = std::move(value);
    }

    [[nodiscard]] const bool& get_is_default() const noexcept
    {
        return is_default;
    }

    void set_billing(std::optional<AddressValue> value)
    {
        billing = std::move(value);
    }

    [[nodiscard]] const std::optional<AddressValue>& get_billing() const noexcept
    {
        return billing;
    }

    void set_network_tokenization(std::optional<NetworkTokenResponse> value)
    {
        network_tokenization = std::move(value);
    }

    [[nodiscard]] const std::optional<NetworkTokenResponse>&
    get_network_tokenization() const noexcept
    {
        return network_tokenization;
    }

    void set_connector_tokens(std::optional<std::vector<ConnectorTokenDetails>> value)
    {
        connector_tokens = std::move(value);
    }

    [[nodiscard]] const std::optional<std::vector<ConnectorTokenDetails>>&
    get_connector_tokens() const noexcept
    {
        return connector_tokens;
    }

    void set_network_transaction_id(std::optional<std::string> value)
    {
        network_transaction_id = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_network_transaction_id() const noexcept
    {
        return network_transaction_id;
    }

    GlobalPaymentMethodIdValue id{};
    GlobalCustomerIdValue customer_id{};
    std::optional<CustomerIdValue> merchant_customer_reference_id{};
    PaymentMethodValue payment_method_type{};
    std::optional<PaymentMethodTypeValue> payment_method_subtype{};
    std::optional<bool> recurring_enabled{};
    std::optional<PaymentMethodListData> payment_method_data{};
    PrimitiveDateTimeValue created{};
    bool requires_cvv{};
    PrimitiveDateTimeValue last_used_at{};
    bool is_default{};
    std::optional<AddressValue> billing{};
    std::optional<NetworkTokenResponse> network_tokenization{};
    std::optional<std::vector<ConnectorTokenDetails>> connector_tokens{};
    std::optional<std::string> network_transaction_id{};
};

class PaymentMethodSessionResponse : public core::behavior::Serializer<PaymentMethodSessionResponse>
{
public:
    void set_id(GlobalPaymentMethodSessionIdValue value)
    {
        id = std::move(value);
    }

    [[nodiscard]] const GlobalPaymentMethodSessionIdValue& get_id() const noexcept
    {
        return id;
    }

    void set_customer_id(std::optional<GlobalCustomerIdValue> value)
    {
        customer_id = std::move(value);
    }

    [[nodiscard]] const std::optional<GlobalCustomerIdValue>& get_customer_id() const noexcept
    {
        return customer_id;
    }

    void set_billing(std::optional<AddressValue> value)
    {
        billing = std::move(value);
    }

    [[nodiscard]] const std::optional<AddressValue>& get_billing() const noexcept
    {
        return billing;
    }

    void set_network_tokenization(std::optional<NetworkTokenizationValue> value)
    {
        network_tokenization = std::move(value);
    }

    [[nodiscard]] const std::optional<NetworkTokenizationValue>&
    get_network_tokenization() const noexcept
    {
        return network_tokenization;
    }

    void set_tokenization_data(std::optional<SecretSerdeValueValue> value)
    {
        tokenization_data = std::move(value);
    }

    [[nodiscard]] const std::optional<SecretSerdeValueValue>& get_tokenization_data() const noexcept
    {
        return tokenization_data;
    }

    void set_expires_at(PrimitiveDateTimeValue value)
    {
        expires_at = std::move(value);
    }

    [[nodiscard]] const PrimitiveDateTimeValue& get_expires_at() const noexcept
    {
        return expires_at;
    }

    void set_client_secret(std::string value)
    {
        client_secret = std::move(value);
    }

    [[nodiscard]] const std::string& get_client_secret() const noexcept
    {
        return client_secret;
    }

    void set_return_url(std::optional<UrlValue> value)
    {
        return_url = std::move(value);
    }

    [[nodiscard]] const std::optional<UrlValue>& get_return_url() const noexcept
    {
        return return_url;
    }

    void set_next_action(std::optional<NextActionDataValue> value)
    {
        next_action = std::move(value);
    }

    [[nodiscard]] const std::optional<NextActionDataValue>& get_next_action() const noexcept
    {
        return next_action;
    }

    void set_authentication_details(std::optional<AuthenticationDetails> value)
    {
        authentication_details = std::move(value);
    }

    [[nodiscard]] const std::optional<AuthenticationDetails>&
    get_authentication_details() const noexcept
    {
        return authentication_details;
    }

    void set_associated_payment_methods(
        std::optional<std::vector<AssociatedPaymentMethodsValue>> value
    )
    {
        associated_payment_methods = std::move(value);
    }

    [[nodiscard]] const std::optional<std::vector<AssociatedPaymentMethodsValue>>&
    get_associated_payment_methods() const noexcept
    {
        return associated_payment_methods;
    }

    void set_associated_token_id(std::optional<GlobalTokenIdValue> value)
    {
        associated_token_id = std::move(value);
    }

    [[nodiscard]] const std::optional<GlobalTokenIdValue>& get_associated_token_id() const noexcept
    {
        return associated_token_id;
    }

    void set_storage_type(StorageTypeValue value)
    {
        storage_type = std::move(value);
    }

    [[nodiscard]] const StorageTypeValue& get_storage_type() const noexcept
    {
        return storage_type;
    }

    void set_card_cvc_token_storage(std::optional<CardCVCTokenStorageDetails> value)
    {
        card_cvc_token_storage = std::move(value);
    }

    [[nodiscard]] const std::optional<CardCVCTokenStorageDetails>&
    get_card_cvc_token_storage() const noexcept
    {
        return card_cvc_token_storage;
    }

    void set_payment_method_data(std::optional<PaymentMethodResponseData> value)
    {
        payment_method_data = std::move(value);
    }

    [[nodiscard]] const std::optional<PaymentMethodResponseData>&
    get_payment_method_data() const noexcept
    {
        return payment_method_data;
    }

    void set_sdk_authorization(std::optional<std::string> value)
    {
        sdk_authorization = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_sdk_authorization() const noexcept
    {
        return sdk_authorization;
    }

    void set_keep_alive(bool value)
    {
        keep_alive = std::move(value);
    }

    [[nodiscard]] const bool& get_keep_alive() const noexcept
    {
        return keep_alive;
    }

    void set_network_tokenization_data(std::optional<NetworkTokenResponse> value)
    {
        network_tokenization_data = std::move(value);
    }

    [[nodiscard]] const std::optional<NetworkTokenResponse>&
    get_network_tokenization_data() const noexcept
    {
        return network_tokenization_data;
    }

    void set_external_vault_details(std::optional<VaultSessionDetailsValue> value)
    {
        external_vault_details = std::move(value);
    }

    [[nodiscard]] const std::optional<VaultSessionDetailsValue>&
    get_external_vault_details() const noexcept
    {
        return external_vault_details;
    }

    GlobalPaymentMethodSessionIdValue id{};
    std::optional<GlobalCustomerIdValue> customer_id{};
    std::optional<AddressValue> billing{};
    std::optional<NetworkTokenizationValue> network_tokenization{};
    std::optional<SecretSerdeValueValue> tokenization_data{};
    PrimitiveDateTimeValue expires_at{};
    std::string client_secret{};
    std::optional<UrlValue> return_url{};
    std::optional<NextActionDataValue> next_action{};
    std::optional<AuthenticationDetails> authentication_details{};
    std::optional<std::vector<AssociatedPaymentMethodsValue>> associated_payment_methods{};
    std::optional<GlobalTokenIdValue> associated_token_id{};
    StorageTypeValue storage_type{};
    std::optional<CardCVCTokenStorageDetails> card_cvc_token_storage{};
    std::optional<PaymentMethodResponseData> payment_method_data{};
    std::optional<std::string> sdk_authorization{};
    bool keep_alive{};
    std::optional<NetworkTokenResponse> network_tokenization_data{};
    std::optional<VaultSessionDetailsValue> external_vault_details{};
};

class PaymentMethodSessionUpdateSavedPaymentMethod :
    public core::behavior::Serializer<PaymentMethodSessionUpdateSavedPaymentMethod>
{
public:
    void set_payment_method_token(std::optional<std::string> value)
    {
        payment_method_token = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_payment_method_token() const noexcept
    {
        return payment_method_token;
    }

    void set_payment_method_update_request(PaymentMethodUpdate value)
    {
        payment_method_update_request = std::move(value);
    }

    [[nodiscard]] const PaymentMethodUpdate& get_payment_method_update_request() const noexcept
    {
        return payment_method_update_request;
    }

    std::optional<std::string> payment_method_token{};
    PaymentMethodUpdate payment_method_update_request{};
};

class PaymentMethodsBatchRetrieveResponse :
    public core::behavior::Serializer<PaymentMethodsBatchRetrieveResponse>
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

    void set_payment_method_data(std::optional<PaymentMethodsData> value)
    {
        payment_method_data = std::move(value);
    }

    [[nodiscard]] const std::optional<PaymentMethodsData>& get_payment_method_data() const noexcept
    {
        return payment_method_data;
    }

    void set_error_message(std::optional<std::string> value)
    {
        error_message = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_error_message() const noexcept
    {
        return error_message;
    }

    void set_line_number(std::optional<std::int64_t> value)
    {
        line_number = std::move(value);
    }

    [[nodiscard]] const std::optional<std::int64_t>& get_line_number() const noexcept
    {
        return line_number;
    }

    std::string payment_method_id{};
    std::optional<PaymentMethodValue> payment_method_type{};
    std::optional<PaymentMethodTypeValue> payment_method_subtype{};
    std::optional<PaymentMethodsData> payment_method_data{};
    std::optional<std::string> error_message{};
    std::optional<std::int64_t> line_number{};
};

class ResponsePaymentMethodTypes : public core::behavior::Serializer<ResponsePaymentMethodTypes>
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

    void set_payment_experience(std::optional<std::vector<PaymentExperienceTypes>> value)
    {
        payment_experience = std::move(value);
    }

    [[nodiscard]] const std::optional<std::vector<PaymentExperienceTypes>>&
    get_payment_experience() const noexcept
    {
        return payment_experience;
    }

    void set_card_networks(std::optional<std::vector<CardNetworkTypes>> value)
    {
        card_networks = std::move(value);
    }

    [[nodiscard]] const std::optional<std::vector<CardNetworkTypes>>&
    get_card_networks() const noexcept
    {
        return card_networks;
    }

    void set_bank_names(std::optional<std::vector<BankCodeResponseValue>> value)
    {
        bank_names = std::move(value);
    }

    [[nodiscard]] const std::optional<std::vector<BankCodeResponseValue>>&
    get_bank_names() const noexcept
    {
        return bank_names;
    }

    void set_bank_debits(std::optional<BankDebitTypes> value)
    {
        bank_debits = std::move(value);
    }

    [[nodiscard]] const std::optional<BankDebitTypes>& get_bank_debits() const noexcept
    {
        return bank_debits;
    }

    void set_bank_transfers(std::optional<BankTransferTypes> value)
    {
        bank_transfers = std::move(value);
    }

    [[nodiscard]] const std::optional<BankTransferTypes>& get_bank_transfers() const noexcept
    {
        return bank_transfers;
    }

    void set_required_fields(
        std::optional<std::unordered_map<std::string, RequiredFieldInfo>> value
    )
    {
        required_fields = std::move(value);
    }

    [[nodiscard]] const std::optional<std::unordered_map<std::string, RequiredFieldInfo>>&
    get_required_fields() const noexcept
    {
        return required_fields;
    }

    void set_surcharge_details(std::optional<SurchargeDetailsResponse> value)
    {
        surcharge_details = std::move(value);
    }

    [[nodiscard]] const std::optional<SurchargeDetailsResponse>&
    get_surcharge_details() const noexcept
    {
        return surcharge_details;
    }

    void set_pm_auth_connector(std::optional<std::string> value)
    {
        pm_auth_connector = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_pm_auth_connector() const noexcept
    {
        return pm_auth_connector;
    }

    void set_customer_acceptance_support(CustomerAcceptanceSupportValue value)
    {
        customer_acceptance_support = std::move(value);
    }

    [[nodiscard]] const CustomerAcceptanceSupportValue&
    get_customer_acceptance_support() const noexcept
    {
        return customer_acceptance_support;
    }

    PaymentMethodTypeValue payment_method_type{};
    std::optional<std::vector<PaymentExperienceTypes>> payment_experience{};
    std::optional<std::vector<CardNetworkTypes>> card_networks{};
    std::optional<std::vector<BankCodeResponseValue>> bank_names{};
    std::optional<BankDebitTypes> bank_debits{};
    std::optional<BankTransferTypes> bank_transfers{};
    std::optional<std::unordered_map<std::string, RequiredFieldInfo>> required_fields{};
    std::optional<SurchargeDetailsResponse> surcharge_details{};
    std::optional<std::string> pm_auth_connector{};
    CustomerAcceptanceSupportValue customer_acceptance_support{};
};

class ResponsePaymentMethodsEnabledForClient :
    public core::behavior::Serializer<ResponsePaymentMethodsEnabledForClient>
{
public:
    void set_payment_method(PaymentMethodValue value)
    {
        payment_method = std::move(value);
    }

    [[nodiscard]] const PaymentMethodValue& get_payment_method() const noexcept
    {
        return payment_method;
    }

    void set_payment_method_type(PaymentMethodTypeValue value)
    {
        payment_method_type = std::move(value);
    }

    [[nodiscard]] const PaymentMethodTypeValue& get_payment_method_type() const noexcept
    {
        return payment_method_type;
    }

    void set_data(std::optional<PaymentMethodSubtypeSpecificDataForClient> value)
    {
        data = std::move(value);
    }

    [[nodiscard]] const std::optional<PaymentMethodSubtypeSpecificDataForClient>&
    get_data() const noexcept
    {
        return data;
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

    void set_collect_shipping_details_from_wallets(std::optional<bool> value)
    {
        collect_shipping_details_from_wallets = std::move(value);
    }

    [[nodiscard]] const std::optional<bool>&
    get_collect_shipping_details_from_wallets() const noexcept
    {
        return collect_shipping_details_from_wallets;
    }

    void set_collect_billing_details_from_wallets(std::optional<bool> value)
    {
        collect_billing_details_from_wallets = std::move(value);
    }

    [[nodiscard]] const std::optional<bool>&
    get_collect_billing_details_from_wallets() const noexcept
    {
        return collect_billing_details_from_wallets;
    }

    void set_customer_acceptance_support(CustomerAcceptanceSupportValue value)
    {
        customer_acceptance_support = std::move(value);
    }

    [[nodiscard]] const CustomerAcceptanceSupportValue&
    get_customer_acceptance_support() const noexcept
    {
        return customer_acceptance_support;
    }

    PaymentMethodValue payment_method{};
    PaymentMethodTypeValue payment_method_type{};
    std::optional<PaymentMethodSubtypeSpecificDataForClient> data{};
    std::optional<std::vector<PaymentExperienceValue>> payment_experience{};
    std::optional<bool> collect_shipping_details_from_wallets{};
    std::optional<bool> collect_billing_details_from_wallets{};
    CustomerAcceptanceSupportValue customer_acceptance_support{};
};

class PaymentMethodIntentConfirmInternal :
    public core::behavior::Serializer<PaymentMethodIntentConfirmInternal>
{
public:
    void set_id(GlobalPaymentMethodIdValue value)
    {
        id = std::move(value);
    }

    [[nodiscard]] const GlobalPaymentMethodIdValue& get_id() const noexcept
    {
        return id;
    }

    void set_request(PaymentMethodIntentConfirm value)
    {
        request = std::move(value);
    }

    [[nodiscard]] const PaymentMethodIntentConfirm& get_request() const noexcept
    {
        return request;
    }

    GlobalPaymentMethodIdValue id{};
    PaymentMethodIntentConfirm request{};
};

class PaymentMethodListIntentData : public core::behavior::Serializer<PaymentMethodListIntentData>
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

    void set_client_secret(std::optional<std::string> value)
    {
        client_secret = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_client_secret() const noexcept
    {
        return client_secret;
    }

    void set_description(std::optional<std::string> value)
    {
        description = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_description() const noexcept
    {
        return description;
    }

    void set_customer_id(std::optional<CustomerIdValue> value)
    {
        customer_id = std::move(value);
    }

    [[nodiscard]] const std::optional<CustomerIdValue>& get_customer_id() const noexcept
    {
        return customer_id;
    }

    void set_return_url(std::optional<std::string> value)
    {
        return_url = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_return_url() const noexcept
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

    void set_billing(std::optional<AddressValue> value)
    {
        billing = std::move(value);
    }

    [[nodiscard]] const std::optional<AddressValue>& get_billing() const noexcept
    {
        return billing;
    }

    void set_shipping(std::optional<AddressValue> value)
    {
        shipping = std::move(value);
    }

    [[nodiscard]] const std::optional<AddressValue>& get_shipping() const noexcept
    {
        return shipping;
    }

    void set_email(std::optional<EmailValue> value)
    {
        email = std::move(value);
    }

    [[nodiscard]] const std::optional<EmailValue>& get_email() const noexcept
    {
        return email;
    }

    void set_metadata(std::optional<SecretSerdeValueValue> value)
    {
        metadata = std::move(value);
    }

    [[nodiscard]] const std::optional<SecretSerdeValueValue>& get_metadata() const noexcept
    {
        return metadata;
    }

    void set_order_details(std::optional<std::vector<std::string>> value)
    {
        order_details = std::move(value);
    }

    [[nodiscard]] const std::optional<std::vector<std::string>>& get_order_details() const noexcept
    {
        return order_details;
    }

    void set_created(std::optional<PrimitiveDateTimeValue> value)
    {
        created = std::move(value);
    }

    [[nodiscard]] const std::optional<PrimitiveDateTimeValue>& get_created() const noexcept
    {
        return created;
    }

    void set_expires_on(std::optional<PrimitiveDateTimeValue> value)
    {
        expires_on = std::move(value);
    }

    [[nodiscard]] const std::optional<PrimitiveDateTimeValue>& get_expires_on() const noexcept
    {
        return expires_on;
    }

    void set_profile_id(std::optional<ProfileIdValue> value)
    {
        profile_id = std::move(value);
    }

    [[nodiscard]] const std::optional<ProfileIdValue>& get_profile_id() const noexcept
    {
        return profile_id;
    }

    void set_merchant_order_reference_id(std::optional<std::string> value)
    {
        merchant_order_reference_id = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_merchant_order_reference_id() const noexcept
    {
        return merchant_order_reference_id;
    }

    void set_attempt_count(std::int16_t value)
    {
        attempt_count = std::move(value);
    }

    [[nodiscard]] const std::int16_t& get_attempt_count() const noexcept
    {
        return attempt_count;
    }

    void set_installment_options(
        std::optional<std::vector<PaymentMethodListInstallmentOption>> value
    )
    {
        installment_options = std::move(value);
    }

    [[nodiscard]] const std::optional<std::vector<PaymentMethodListInstallmentOption>>&
    get_installment_options() const noexcept
    {
        return installment_options;
    }

    void set_capture_method(std::optional<CaptureMethodValue> value)
    {
        capture_method = std::move(value);
    }

    [[nodiscard]] const std::optional<CaptureMethodValue>& get_capture_method() const noexcept
    {
        return capture_method;
    }

    void set_merchant_name(std::optional<std::string> value)
    {
        merchant_name = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_merchant_name() const noexcept
    {
        return merchant_name;
    }

    void set_mandate_payment(std::optional<MandateDataValue> value)
    {
        mandate_payment = std::move(value);
    }

    [[nodiscard]] const std::optional<MandateDataValue>& get_mandate_payment() const noexcept
    {
        return mandate_payment;
    }

    void set_payment_type(std::optional<PaymentTypeValue> value)
    {
        payment_type = std::move(value);
    }

    [[nodiscard]] const std::optional<PaymentTypeValue>& get_payment_type() const noexcept
    {
        return payment_type;
    }

    void set_request_external_three_ds_authentication(std::optional<bool> value)
    {
        request_external_three_ds_authentication = std::move(value);
    }

    [[nodiscard]] const std::optional<bool>&
    get_request_external_three_ds_authentication() const noexcept
    {
        return request_external_three_ds_authentication;
    }

    void set_is_tax_calculation_enabled(std::optional<bool> value)
    {
        is_tax_calculation_enabled = std::move(value);
    }

    [[nodiscard]] const std::optional<bool>& get_is_tax_calculation_enabled() const noexcept
    {
        return is_tax_calculation_enabled;
    }

    void set_is_guest_customer(std::optional<bool> value)
    {
        is_guest_customer = std::move(value);
    }

    [[nodiscard]] const std::optional<bool>& get_is_guest_customer() const noexcept
    {
        return is_guest_customer;
    }

    PaymentIdValue payment_id{};
    IntentStatusValue status{};
    MinorUnitValue amount{};
    std::optional<CurrencyValue> currency{};
    std::optional<std::string> client_secret{};
    std::optional<std::string> description{};
    std::optional<CustomerIdValue> customer_id{};
    std::optional<std::string> return_url{};
    std::optional<FutureUsageValue> setup_future_usage{};
    std::optional<AddressValue> billing{};
    std::optional<AddressValue> shipping{};
    std::optional<EmailValue> email{};
    std::optional<SecretSerdeValueValue> metadata{};
    std::optional<std::vector<std::string>> order_details{};
    std::optional<PrimitiveDateTimeValue> created{};
    std::optional<PrimitiveDateTimeValue> expires_on{};
    std::optional<ProfileIdValue> profile_id{};
    std::optional<std::string> merchant_order_reference_id{};
    std::int16_t attempt_count{};
    std::optional<std::vector<PaymentMethodListInstallmentOption>> installment_options{};
    std::optional<CaptureMethodValue> capture_method{};
    std::optional<std::string> merchant_name{};
    std::optional<MandateDataValue> mandate_payment{};
    std::optional<PaymentTypeValue> payment_type{};
    std::optional<bool> request_external_three_ds_authentication{};
    std::optional<bool> is_tax_calculation_enabled{};
    std::optional<bool> is_guest_customer{};
};

class PaymentMethodListResponseForSession :
    public core::behavior::Serializer<PaymentMethodListResponseForSession>
{
public:
    void set_payment_methods_enabled(std::vector<ResponsePaymentMethodTypes> value)
    {
        payment_methods_enabled = std::move(value);
    }

    [[nodiscard]] const std::vector<ResponsePaymentMethodTypes>&
    get_payment_methods_enabled() const noexcept
    {
        return payment_methods_enabled;
    }

    void set_customer_payment_methods(std::vector<CustomerPaymentMethodResponseItem> value)
    {
        customer_payment_methods = std::move(value);
    }

    [[nodiscard]] const std::vector<CustomerPaymentMethodResponseItem>&
    get_customer_payment_methods() const noexcept
    {
        return customer_payment_methods;
    }

    std::vector<ResponsePaymentMethodTypes> payment_methods_enabled{};
    std::vector<CustomerPaymentMethodResponseItem> customer_payment_methods{};
};

class PaymentMethodMigrate : public core::behavior::Serializer<PaymentMethodMigrate>
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

    void set_payment_method_issuer(std::optional<std::string> value)
    {
        payment_method_issuer = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_payment_method_issuer() const noexcept
    {
        return payment_method_issuer;
    }

    void set_payment_method_issuer_code(std::optional<PaymentMethodIssuerCodeValue> value)
    {
        payment_method_issuer_code = std::move(value);
    }

    [[nodiscard]] const std::optional<PaymentMethodIssuerCodeValue>&
    get_payment_method_issuer_code() const noexcept
    {
        return payment_method_issuer_code;
    }

    void set_card(std::optional<MigrateCardDetail> value)
    {
        card = std::move(value);
    }

    [[nodiscard]] const std::optional<MigrateCardDetail>& get_card() const noexcept
    {
        return card;
    }

    void set_network_token(std::optional<MigrateNetworkTokenDetail> value)
    {
        network_token = std::move(value);
    }

    [[nodiscard]] const std::optional<MigrateNetworkTokenDetail>& get_network_token() const noexcept
    {
        return network_token;
    }

    void set_metadata(std::optional<SecretSerdeValueValue> value)
    {
        metadata = std::move(value);
    }

    [[nodiscard]] const std::optional<SecretSerdeValueValue>& get_metadata() const noexcept
    {
        return metadata;
    }

    void set_customer_id(std::optional<CustomerIdValue> value)
    {
        customer_id = std::move(value);
    }

    [[nodiscard]] const std::optional<CustomerIdValue>& get_customer_id() const noexcept
    {
        return customer_id;
    }

    void set_card_network(std::optional<std::string> value)
    {
        card_network = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_card_network() const noexcept
    {
        return card_network;
    }

    void set_bank_transfer(std::optional<BankValue> value)
    {
        bank_transfer = std::move(value);
    }

    [[nodiscard]] const std::optional<BankValue>& get_bank_transfer() const noexcept
    {
        return bank_transfer;
    }

    void set_bank_transfer_data(std::optional<BankTransferValue> value)
    {
        bank_transfer_data = std::move(value);
    }

    [[nodiscard]] const std::optional<BankTransferValue>& get_bank_transfer_data() const noexcept
    {
        return bank_transfer_data;
    }

    void set_wallet(std::optional<WalletValue> value)
    {
        wallet = std::move(value);
    }

    [[nodiscard]] const std::optional<WalletValue>& get_wallet() const noexcept
    {
        return wallet;
    }

    void set_payment_method_data(std::optional<PaymentMethodCreateData> value)
    {
        payment_method_data = std::move(value);
    }

    [[nodiscard]] const std::optional<PaymentMethodCreateData>&
    get_payment_method_data() const noexcept
    {
        return payment_method_data;
    }

    void set_billing(std::optional<AddressValue> value)
    {
        billing = std::move(value);
    }

    [[nodiscard]] const std::optional<AddressValue>& get_billing() const noexcept
    {
        return billing;
    }

    void set_connector_mandate_details(std::optional<CommonMandateReference> value)
    {
        connector_mandate_details = std::move(value);
    }

    [[nodiscard]] const std::optional<CommonMandateReference>&
    get_connector_mandate_details() const noexcept
    {
        return connector_mandate_details;
    }

    void set_network_transaction_id(std::optional<std::string> value)
    {
        network_transaction_id = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_network_transaction_id() const noexcept
    {
        return network_transaction_id;
    }

    MerchantIdValue merchant_id{};
    std::optional<PaymentMethodValue> payment_method{};
    std::optional<PaymentMethodTypeValue> payment_method_type{};
    std::optional<std::string> payment_method_issuer{};
    std::optional<PaymentMethodIssuerCodeValue> payment_method_issuer_code{};
    std::optional<MigrateCardDetail> card{};
    std::optional<MigrateNetworkTokenDetail> network_token{};
    std::optional<SecretSerdeValueValue> metadata{};
    std::optional<CustomerIdValue> customer_id{};
    std::optional<std::string> card_network{};
    std::optional<BankValue> bank_transfer{};
    std::optional<BankTransferValue> bank_transfer_data{};
    std::optional<WalletValue> wallet{};
    std::optional<PaymentMethodCreateData> payment_method_data{};
    std::optional<AddressValue> billing{};
    std::optional<CommonMandateReference> connector_mandate_details{};
    std::optional<std::string> network_transaction_id{};
};

class ResponsePaymentMethodsEnabled :
    public core::behavior::Serializer<ResponsePaymentMethodsEnabled>
{
public:
    void set_payment_method(PaymentMethodValue value)
    {
        payment_method = std::move(value);
    }

    [[nodiscard]] const PaymentMethodValue& get_payment_method() const noexcept
    {
        return payment_method;
    }

    void set_payment_method_types(std::vector<ResponsePaymentMethodTypes> value)
    {
        payment_method_types = std::move(value);
    }

    [[nodiscard]] const std::vector<ResponsePaymentMethodTypes>&
    get_payment_method_types() const noexcept
    {
        return payment_method_types;
    }

    PaymentMethodValue payment_method{};
    std::vector<ResponsePaymentMethodTypes> payment_method_types{};
};

class ClientPaymentMethodsListResponse :
    public core::behavior::Serializer<ClientPaymentMethodsListResponse>
{
public:
    void set_payment_methods_enabled(std::vector<ResponsePaymentMethodsEnabledForClient> value)
    {
        payment_methods_enabled = std::move(value);
    }

    [[nodiscard]] const std::vector<ResponsePaymentMethodsEnabledForClient>&
    get_payment_methods_enabled() const noexcept
    {
        return payment_methods_enabled;
    }

    void set_customer_payment_methods(std::vector<CustomerPaymentMethodForClient> value)
    {
        customer_payment_methods = std::move(value);
    }

    [[nodiscard]] const std::vector<CustomerPaymentMethodForClient>&
    get_customer_payment_methods() const noexcept
    {
        return customer_payment_methods;
    }

    void set_sdk_next_action(SdkNextActionValue value)
    {
        sdk_next_action = std::move(value);
    }

    [[nodiscard]] const SdkNextActionValue& get_sdk_next_action() const noexcept
    {
        return sdk_next_action;
    }

    void set_intent_data(PaymentMethodListIntentData value)
    {
        intent_data = std::move(value);
    }

    [[nodiscard]] const PaymentMethodListIntentData& get_intent_data() const noexcept
    {
        return intent_data;
    }

    std::vector<ResponsePaymentMethodsEnabledForClient> payment_methods_enabled{};
    std::vector<CustomerPaymentMethodForClient> customer_payment_methods{};
    SdkNextActionValue sdk_next_action{};
    PaymentMethodListIntentData intent_data{};
};

class PaymentMethodListResponse : public core::behavior::Serializer<PaymentMethodListResponse>
{
public:
    void set_redirect_url(std::optional<std::string> value)
    {
        redirect_url = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_redirect_url() const noexcept
    {
        return redirect_url;
    }

    void set_currency(std::optional<CurrencyValue> value)
    {
        currency = std::move(value);
    }

    [[nodiscard]] const std::optional<CurrencyValue>& get_currency() const noexcept
    {
        return currency;
    }

    void set_payment_methods(std::vector<ResponsePaymentMethodsEnabled> value)
    {
        payment_methods = std::move(value);
    }

    [[nodiscard]] const std::vector<ResponsePaymentMethodsEnabled>&
    get_payment_methods() const noexcept
    {
        return payment_methods;
    }

    void set_mandate_payment(std::optional<MandateTypeValue> value)
    {
        mandate_payment = std::move(value);
    }

    [[nodiscard]] const std::optional<MandateTypeValue>& get_mandate_payment() const noexcept
    {
        return mandate_payment;
    }

    void set_merchant_name(OptionalEncryptableNameValue value)
    {
        merchant_name = std::move(value);
    }

    [[nodiscard]] const OptionalEncryptableNameValue& get_merchant_name() const noexcept
    {
        return merchant_name;
    }

    void set_show_surcharge_breakup_screen(bool value)
    {
        show_surcharge_breakup_screen = std::move(value);
    }

    [[nodiscard]] const bool& get_show_surcharge_breakup_screen() const noexcept
    {
        return show_surcharge_breakup_screen;
    }

    void set_payment_type(std::optional<PaymentTypeValue> value)
    {
        payment_type = std::move(value);
    }

    [[nodiscard]] const std::optional<PaymentTypeValue>& get_payment_type() const noexcept
    {
        return payment_type;
    }

    void set_request_external_three_ds_authentication(bool value)
    {
        request_external_three_ds_authentication = std::move(value);
    }

    [[nodiscard]] const bool& get_request_external_three_ds_authentication() const noexcept
    {
        return request_external_three_ds_authentication;
    }

    void set_collect_shipping_details_from_wallets(std::optional<bool> value)
    {
        collect_shipping_details_from_wallets = std::move(value);
    }

    [[nodiscard]] const std::optional<bool>&
    get_collect_shipping_details_from_wallets() const noexcept
    {
        return collect_shipping_details_from_wallets;
    }

    void set_collect_billing_details_from_wallets(std::optional<bool> value)
    {
        collect_billing_details_from_wallets = std::move(value);
    }

    [[nodiscard]] const std::optional<bool>&
    get_collect_billing_details_from_wallets() const noexcept
    {
        return collect_billing_details_from_wallets;
    }

    void set_is_tax_calculation_enabled(bool value)
    {
        is_tax_calculation_enabled = std::move(value);
    }

    [[nodiscard]] const bool& get_is_tax_calculation_enabled() const noexcept
    {
        return is_tax_calculation_enabled;
    }

    void set_sdk_next_action(SdkNextActionValue value)
    {
        sdk_next_action = std::move(value);
    }

    [[nodiscard]] const SdkNextActionValue& get_sdk_next_action() const noexcept
    {
        return sdk_next_action;
    }

    void set_is_guest_customer(bool value)
    {
        is_guest_customer = std::move(value);
    }

    [[nodiscard]] const bool& get_is_guest_customer() const noexcept
    {
        return is_guest_customer;
    }

    void set_intent_data(std::optional<PaymentMethodListIntentData> value)
    {
        intent_data = std::move(value);
    }

    [[nodiscard]] const std::optional<PaymentMethodListIntentData>& get_intent_data() const noexcept
    {
        return intent_data;
    }

    std::optional<std::string> redirect_url{};
    std::optional<CurrencyValue> currency{};
    std::vector<ResponsePaymentMethodsEnabled> payment_methods{};
    std::optional<MandateTypeValue> mandate_payment{};
    OptionalEncryptableNameValue merchant_name{};
    bool show_surcharge_breakup_screen{};
    std::optional<PaymentTypeValue> payment_type{};
    bool request_external_three_ds_authentication{};
    std::optional<bool> collect_shipping_details_from_wallets{};
    std::optional<bool> collect_billing_details_from_wallets{};
    bool is_tax_calculation_enabled{};
    SdkNextActionValue sdk_next_action{};
    bool is_guest_customer{};
    std::optional<PaymentMethodListIntentData> intent_data{};
};
} // namespace app::model::payment_methods

template<>
struct serde::Serializable<app::model::payment_methods::PaymentsMandateReferenceRecord>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "connector_mandate_id",
                &app::model::payment_methods::PaymentsMandateReferenceRecord::
                    get_connector_mandate_id,
                &app::model::payment_methods::PaymentsMandateReferenceRecord::
                    set_connector_mandate_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_method_type",
                &app::model::payment_methods::PaymentsMandateReferenceRecord::
                    get_payment_method_type,
                &app::model::payment_methods::PaymentsMandateReferenceRecord::
                    set_payment_method_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "original_payment_authorized_amount",
                &app::model::payment_methods::PaymentsMandateReferenceRecord::
                    get_original_payment_authorized_amount,
                &app::model::payment_methods::PaymentsMandateReferenceRecord::
                    set_original_payment_authorized_amount,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "original_payment_authorized_currency",
                &app::model::payment_methods::PaymentsMandateReferenceRecord::
                    get_original_payment_authorized_currency,
                &app::model::payment_methods::PaymentsMandateReferenceRecord::
                    set_original_payment_authorized_currency,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "connector_customer_id",
                &app::model::payment_methods::PaymentsMandateReferenceRecord::
                    get_connector_customer_id,
                &app::model::payment_methods::PaymentsMandateReferenceRecord::
                    set_connector_customer_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "connector_mandate_request_reference_id",
                &app::model::payment_methods::PaymentsMandateReferenceRecord::
                    get_connector_mandate_request_reference_id,
                &app::model::payment_methods::PaymentsMandateReferenceRecord::
                    set_connector_mandate_request_reference_id,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "payments_mandate_reference_record";
    }
};

template<>
struct serde::Serializable<app::model::payment_methods::PaymentsMandateReference>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<
            "field0",
            &app::model::payment_methods::PaymentsMandateReference::get_field0,
            &app::model::payment_methods::PaymentsMandateReference::set_field0,
            serde::FieldOptions{}>{}};
    }

    static constexpr std::string_view table_name()
    {
        return "payments_mandate_reference";
    }
};

template<>
struct serde::Serializable<app::model::payment_methods::CardDetail>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "card_number",
                &app::model::payment_methods::CardDetail::get_card_number,
                &app::model::payment_methods::CardDetail::set_card_number,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_exp_month",
                &app::model::payment_methods::CardDetail::get_card_exp_month,
                &app::model::payment_methods::CardDetail::set_card_exp_month,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_exp_year",
                &app::model::payment_methods::CardDetail::get_card_exp_year,
                &app::model::payment_methods::CardDetail::set_card_exp_year,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_cvc",
                &app::model::payment_methods::CardDetail::get_card_cvc,
                &app::model::payment_methods::CardDetail::set_card_cvc,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_holder_name",
                &app::model::payment_methods::CardDetail::get_card_holder_name,
                &app::model::payment_methods::CardDetail::set_card_holder_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "nick_name",
                &app::model::payment_methods::CardDetail::get_nick_name,
                &app::model::payment_methods::CardDetail::set_nick_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_issuing_country",
                &app::model::payment_methods::CardDetail::get_card_issuing_country,
                &app::model::payment_methods::CardDetail::set_card_issuing_country,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_issuing_country_code",
                &app::model::payment_methods::CardDetail::get_card_issuing_country_code,
                &app::model::payment_methods::CardDetail::set_card_issuing_country_code,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_network",
                &app::model::payment_methods::CardDetail::get_card_network,
                &app::model::payment_methods::CardDetail::set_card_network,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_issuer",
                &app::model::payment_methods::CardDetail::get_card_issuer,
                &app::model::payment_methods::CardDetail::set_card_issuer,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_type",
                &app::model::payment_methods::CardDetail::get_card_type,
                &app::model::payment_methods::CardDetail::set_card_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_subtype",
                &app::model::payment_methods::CardDetail::get_card_subtype,
                &app::model::payment_methods::CardDetail::set_card_subtype,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_segment_type",
                &app::model::payment_methods::CardDetail::get_card_segment_type,
                &app::model::payment_methods::CardDetail::set_card_segment_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "funding_source",
                &app::model::payment_methods::CardDetail::get_funding_source,
                &app::model::payment_methods::CardDetail::set_funding_source,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "card_detail";
    }
};

template<>
struct serde::Serializable<app::model::payment_methods::PaymentMethodCreate>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "payment_method",
                &app::model::payment_methods::PaymentMethodCreate::get_payment_method,
                &app::model::payment_methods::PaymentMethodCreate::set_payment_method,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_method_type",
                &app::model::payment_methods::PaymentMethodCreate::get_payment_method_type,
                &app::model::payment_methods::PaymentMethodCreate::set_payment_method_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_method_issuer",
                &app::model::payment_methods::PaymentMethodCreate::get_payment_method_issuer,
                &app::model::payment_methods::PaymentMethodCreate::set_payment_method_issuer,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_method_issuer_code",
                &app::model::payment_methods::PaymentMethodCreate::get_payment_method_issuer_code,
                &app::model::payment_methods::PaymentMethodCreate::set_payment_method_issuer_code,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card",
                &app::model::payment_methods::PaymentMethodCreate::get_card,
                &app::model::payment_methods::PaymentMethodCreate::set_card,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "metadata",
                &app::model::payment_methods::PaymentMethodCreate::get_metadata,
                &app::model::payment_methods::PaymentMethodCreate::set_metadata,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "customer_id",
                &app::model::payment_methods::PaymentMethodCreate::get_customer_id,
                &app::model::payment_methods::PaymentMethodCreate::set_customer_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_network",
                &app::model::payment_methods::PaymentMethodCreate::get_card_network,
                &app::model::payment_methods::PaymentMethodCreate::set_card_network,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "bank_transfer",
                &app::model::payment_methods::PaymentMethodCreate::get_bank_transfer,
                &app::model::payment_methods::PaymentMethodCreate::set_bank_transfer,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "bank_transfer_data",
                &app::model::payment_methods::PaymentMethodCreate::get_bank_transfer_data,
                &app::model::payment_methods::PaymentMethodCreate::set_bank_transfer_data,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "wallet",
                &app::model::payment_methods::PaymentMethodCreate::get_wallet,
                &app::model::payment_methods::PaymentMethodCreate::set_wallet,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "client_secret",
                &app::model::payment_methods::PaymentMethodCreate::get_client_secret,
                &app::model::payment_methods::PaymentMethodCreate::set_client_secret,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_method_data",
                &app::model::payment_methods::PaymentMethodCreate::get_payment_method_data,
                &app::model::payment_methods::PaymentMethodCreate::set_payment_method_data,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "billing",
                &app::model::payment_methods::PaymentMethodCreate::get_billing,
                &app::model::payment_methods::PaymentMethodCreate::set_billing,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "connector_mandate_details",
                &app::model::payment_methods::PaymentMethodCreate::get_connector_mandate_details,
                &app::model::payment_methods::PaymentMethodCreate::set_connector_mandate_details,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "network_transaction_id",
                &app::model::payment_methods::PaymentMethodCreate::get_network_transaction_id,
                &app::model::payment_methods::PaymentMethodCreate::set_network_transaction_id,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "payment_method_create";
    }
};

template<>
struct serde::Serializable<app::model::payment_methods::PaymentMethodRetrieveRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "fetch_raw_detail",
                &app::model::payment_methods::PaymentMethodRetrieveRequest::get_fetch_raw_detail,
                &app::model::payment_methods::PaymentMethodRetrieveRequest::set_fetch_raw_detail,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "force_sync",
                &app::model::payment_methods::PaymentMethodRetrieveRequest::get_force_sync,
                &app::model::payment_methods::PaymentMethodRetrieveRequest::set_force_sync,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "payment_method_retrieve_request";
    }
};

template<>
struct serde::Serializable<app::model::payment_methods::PaymentMethodIntentCreate>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "metadata",
                &app::model::payment_methods::PaymentMethodIntentCreate::get_metadata,
                &app::model::payment_methods::PaymentMethodIntentCreate::set_metadata,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "billing",
                &app::model::payment_methods::PaymentMethodIntentCreate::get_billing,
                &app::model::payment_methods::PaymentMethodIntentCreate::set_billing,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "customer_id",
                &app::model::payment_methods::PaymentMethodIntentCreate::get_customer_id,
                &app::model::payment_methods::PaymentMethodIntentCreate::set_customer_id,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "payment_method_intent_create";
    }
};

template<>
struct serde::Serializable<app::model::payment_methods::PaymentMethodIntentConfirm>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "customer_id",
                &app::model::payment_methods::PaymentMethodIntentConfirm::get_customer_id,
                &app::model::payment_methods::PaymentMethodIntentConfirm::set_customer_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_method_data",
                &app::model::payment_methods::PaymentMethodIntentConfirm::get_payment_method_data,
                &app::model::payment_methods::PaymentMethodIntentConfirm::set_payment_method_data,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_method_type",
                &app::model::payment_methods::PaymentMethodIntentConfirm::get_payment_method_type,
                &app::model::payment_methods::PaymentMethodIntentConfirm::set_payment_method_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_method_subtype",
                &app::model::payment_methods::PaymentMethodIntentConfirm::
                    get_payment_method_subtype,
                &app::model::payment_methods::PaymentMethodIntentConfirm::
                    set_payment_method_subtype,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "payment_method_intent_confirm";
    }
};

template<>
struct serde::Serializable<app::model::payment_methods::PaymentMethodIntentConfirmInternal>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "id",
                &app::model::payment_methods::PaymentMethodIntentConfirmInternal::get_id,
                &app::model::payment_methods::PaymentMethodIntentConfirmInternal::set_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "request",
                &app::model::payment_methods::PaymentMethodIntentConfirmInternal::get_request,
                &app::model::payment_methods::PaymentMethodIntentConfirmInternal::set_request,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "payment_method_intent_confirm_internal";
    }
};

template<>
struct serde::Serializable<app::model::payment_methods::MigrateNetworkTokenData>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "network_token_number",
                &app::model::payment_methods::MigrateNetworkTokenData::get_network_token_number,
                &app::model::payment_methods::MigrateNetworkTokenData::set_network_token_number,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "network_token_exp_month",
                &app::model::payment_methods::MigrateNetworkTokenData::get_network_token_exp_month,
                &app::model::payment_methods::MigrateNetworkTokenData::set_network_token_exp_month,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "network_token_exp_year",
                &app::model::payment_methods::MigrateNetworkTokenData::get_network_token_exp_year,
                &app::model::payment_methods::MigrateNetworkTokenData::set_network_token_exp_year,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_holder_name",
                &app::model::payment_methods::MigrateNetworkTokenData::get_card_holder_name,
                &app::model::payment_methods::MigrateNetworkTokenData::set_card_holder_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "nick_name",
                &app::model::payment_methods::MigrateNetworkTokenData::get_nick_name,
                &app::model::payment_methods::MigrateNetworkTokenData::set_nick_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_issuing_country",
                &app::model::payment_methods::MigrateNetworkTokenData::get_card_issuing_country,
                &app::model::payment_methods::MigrateNetworkTokenData::set_card_issuing_country,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_issuing_country_code",
                &app::model::payment_methods::MigrateNetworkTokenData::
                    get_card_issuing_country_code,
                &app::model::payment_methods::MigrateNetworkTokenData::
                    set_card_issuing_country_code,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_network",
                &app::model::payment_methods::MigrateNetworkTokenData::get_card_network,
                &app::model::payment_methods::MigrateNetworkTokenData::set_card_network,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_issuer",
                &app::model::payment_methods::MigrateNetworkTokenData::get_card_issuer,
                &app::model::payment_methods::MigrateNetworkTokenData::set_card_issuer,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_type",
                &app::model::payment_methods::MigrateNetworkTokenData::get_card_type,
                &app::model::payment_methods::MigrateNetworkTokenData::set_card_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_subtype",
                &app::model::payment_methods::MigrateNetworkTokenData::get_card_subtype,
                &app::model::payment_methods::MigrateNetworkTokenData::set_card_subtype,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_segment_type",
                &app::model::payment_methods::MigrateNetworkTokenData::get_card_segment_type,
                &app::model::payment_methods::MigrateNetworkTokenData::set_card_segment_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "funding_source",
                &app::model::payment_methods::MigrateNetworkTokenData::get_funding_source,
                &app::model::payment_methods::MigrateNetworkTokenData::set_funding_source,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "migrate_network_token_data";
    }
};

template<>
struct serde::Serializable<app::model::payment_methods::MigrateNetworkTokenDetail>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "network_token_data",
                &app::model::payment_methods::MigrateNetworkTokenDetail::get_network_token_data,
                &app::model::payment_methods::MigrateNetworkTokenDetail::set_network_token_data,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "network_token_requestor_ref_id",
                &app::model::payment_methods::MigrateNetworkTokenDetail::
                    get_network_token_requestor_ref_id,
                &app::model::payment_methods::MigrateNetworkTokenDetail::
                    set_network_token_requestor_ref_id,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "migrate_network_token_detail";
    }
};

template<>
struct serde::Serializable<app::model::payment_methods::MigrateCardDetail>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "card_number",
                &app::model::payment_methods::MigrateCardDetail::get_card_number,
                &app::model::payment_methods::MigrateCardDetail::set_card_number,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_exp_month",
                &app::model::payment_methods::MigrateCardDetail::get_card_exp_month,
                &app::model::payment_methods::MigrateCardDetail::set_card_exp_month,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_exp_year",
                &app::model::payment_methods::MigrateCardDetail::get_card_exp_year,
                &app::model::payment_methods::MigrateCardDetail::set_card_exp_year,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_holder_name",
                &app::model::payment_methods::MigrateCardDetail::get_card_holder_name,
                &app::model::payment_methods::MigrateCardDetail::set_card_holder_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "nick_name",
                &app::model::payment_methods::MigrateCardDetail::get_nick_name,
                &app::model::payment_methods::MigrateCardDetail::set_nick_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_issuing_country",
                &app::model::payment_methods::MigrateCardDetail::get_card_issuing_country,
                &app::model::payment_methods::MigrateCardDetail::set_card_issuing_country,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_issuing_country_code",
                &app::model::payment_methods::MigrateCardDetail::get_card_issuing_country_code,
                &app::model::payment_methods::MigrateCardDetail::set_card_issuing_country_code,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_network",
                &app::model::payment_methods::MigrateCardDetail::get_card_network,
                &app::model::payment_methods::MigrateCardDetail::set_card_network,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_issuer",
                &app::model::payment_methods::MigrateCardDetail::get_card_issuer,
                &app::model::payment_methods::MigrateCardDetail::set_card_issuer,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_type",
                &app::model::payment_methods::MigrateCardDetail::get_card_type,
                &app::model::payment_methods::MigrateCardDetail::set_card_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_subtype",
                &app::model::payment_methods::MigrateCardDetail::get_card_subtype,
                &app::model::payment_methods::MigrateCardDetail::set_card_subtype,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_segment_type",
                &app::model::payment_methods::MigrateCardDetail::get_card_segment_type,
                &app::model::payment_methods::MigrateCardDetail::set_card_segment_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "funding_source",
                &app::model::payment_methods::MigrateCardDetail::get_funding_source,
                &app::model::payment_methods::MigrateCardDetail::set_funding_source,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "migrate_card_detail";
    }
};

template<>
struct serde::Serializable<app::model::payment_methods::PayoutsMandateReferenceRecord>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<
            "transfer_method_id",
            &app::model::payment_methods::PayoutsMandateReferenceRecord::get_transfer_method_id,
            &app::model::payment_methods::PayoutsMandateReferenceRecord::set_transfer_method_id,
            serde::FieldOptions{}>{}};
    }

    static constexpr std::string_view table_name()
    {
        return "payouts_mandate_reference_record";
    }
};

template<>
struct serde::Serializable<app::model::payment_methods::PayoutsMandateReference>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<
            "field0",
            &app::model::payment_methods::PayoutsMandateReference::get_field0,
            &app::model::payment_methods::PayoutsMandateReference::set_field0,
            serde::FieldOptions{}>{}};
    }

    static constexpr std::string_view table_name()
    {
        return "payouts_mandate_reference";
    }
};

template<>
struct serde::Serializable<app::model::payment_methods::CommonMandateReference>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "payments",
                &app::model::payment_methods::CommonMandateReference::get_payments,
                &app::model::payment_methods::CommonMandateReference::set_payments,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payouts",
                &app::model::payment_methods::CommonMandateReference::get_payouts,
                &app::model::payment_methods::CommonMandateReference::set_payouts,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "common_mandate_reference";
    }
};

template<>
struct serde::Serializable<app::model::payment_methods::PaymentMethodMigrate>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "merchant_id",
                &app::model::payment_methods::PaymentMethodMigrate::get_merchant_id,
                &app::model::payment_methods::PaymentMethodMigrate::set_merchant_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_method",
                &app::model::payment_methods::PaymentMethodMigrate::get_payment_method,
                &app::model::payment_methods::PaymentMethodMigrate::set_payment_method,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_method_type",
                &app::model::payment_methods::PaymentMethodMigrate::get_payment_method_type,
                &app::model::payment_methods::PaymentMethodMigrate::set_payment_method_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_method_issuer",
                &app::model::payment_methods::PaymentMethodMigrate::get_payment_method_issuer,
                &app::model::payment_methods::PaymentMethodMigrate::set_payment_method_issuer,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_method_issuer_code",
                &app::model::payment_methods::PaymentMethodMigrate::get_payment_method_issuer_code,
                &app::model::payment_methods::PaymentMethodMigrate::set_payment_method_issuer_code,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card",
                &app::model::payment_methods::PaymentMethodMigrate::get_card,
                &app::model::payment_methods::PaymentMethodMigrate::set_card,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "network_token",
                &app::model::payment_methods::PaymentMethodMigrate::get_network_token,
                &app::model::payment_methods::PaymentMethodMigrate::set_network_token,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "metadata",
                &app::model::payment_methods::PaymentMethodMigrate::get_metadata,
                &app::model::payment_methods::PaymentMethodMigrate::set_metadata,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "customer_id",
                &app::model::payment_methods::PaymentMethodMigrate::get_customer_id,
                &app::model::payment_methods::PaymentMethodMigrate::set_customer_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_network",
                &app::model::payment_methods::PaymentMethodMigrate::get_card_network,
                &app::model::payment_methods::PaymentMethodMigrate::set_card_network,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "bank_transfer",
                &app::model::payment_methods::PaymentMethodMigrate::get_bank_transfer,
                &app::model::payment_methods::PaymentMethodMigrate::set_bank_transfer,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "bank_transfer_data",
                &app::model::payment_methods::PaymentMethodMigrate::get_bank_transfer_data,
                &app::model::payment_methods::PaymentMethodMigrate::set_bank_transfer_data,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "wallet",
                &app::model::payment_methods::PaymentMethodMigrate::get_wallet,
                &app::model::payment_methods::PaymentMethodMigrate::set_wallet,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_method_data",
                &app::model::payment_methods::PaymentMethodMigrate::get_payment_method_data,
                &app::model::payment_methods::PaymentMethodMigrate::set_payment_method_data,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "billing",
                &app::model::payment_methods::PaymentMethodMigrate::get_billing,
                &app::model::payment_methods::PaymentMethodMigrate::set_billing,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "connector_mandate_details",
                &app::model::payment_methods::PaymentMethodMigrate::get_connector_mandate_details,
                &app::model::payment_methods::PaymentMethodMigrate::set_connector_mandate_details,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "network_transaction_id",
                &app::model::payment_methods::PaymentMethodMigrate::get_network_transaction_id,
                &app::model::payment_methods::PaymentMethodMigrate::set_network_transaction_id,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "payment_method_migrate";
    }
};

template<>
struct serde::Serializable<app::model::payment_methods::CardDetailFromLocker>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "scheme",
                &app::model::payment_methods::CardDetailFromLocker::get_scheme,
                &app::model::payment_methods::CardDetailFromLocker::set_scheme,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "issuer_country",
                &app::model::payment_methods::CardDetailFromLocker::get_issuer_country,
                &app::model::payment_methods::CardDetailFromLocker::set_issuer_country,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "issuer_country_code",
                &app::model::payment_methods::CardDetailFromLocker::get_issuer_country_code,
                &app::model::payment_methods::CardDetailFromLocker::set_issuer_country_code,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "last4_digits",
                &app::model::payment_methods::CardDetailFromLocker::get_last4_digits,
                &app::model::payment_methods::CardDetailFromLocker::set_last4_digits,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_number",
                &app::model::payment_methods::CardDetailFromLocker::get_card_number,
                &app::model::payment_methods::CardDetailFromLocker::set_card_number,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "expiry_month",
                &app::model::payment_methods::CardDetailFromLocker::get_expiry_month,
                &app::model::payment_methods::CardDetailFromLocker::set_expiry_month,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "expiry_year",
                &app::model::payment_methods::CardDetailFromLocker::get_expiry_year,
                &app::model::payment_methods::CardDetailFromLocker::set_expiry_year,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_token",
                &app::model::payment_methods::CardDetailFromLocker::get_card_token,
                &app::model::payment_methods::CardDetailFromLocker::set_card_token,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_holder_name",
                &app::model::payment_methods::CardDetailFromLocker::get_card_holder_name,
                &app::model::payment_methods::CardDetailFromLocker::set_card_holder_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_fingerprint",
                &app::model::payment_methods::CardDetailFromLocker::get_card_fingerprint,
                &app::model::payment_methods::CardDetailFromLocker::set_card_fingerprint,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "nick_name",
                &app::model::payment_methods::CardDetailFromLocker::get_nick_name,
                &app::model::payment_methods::CardDetailFromLocker::set_nick_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_network",
                &app::model::payment_methods::CardDetailFromLocker::get_card_network,
                &app::model::payment_methods::CardDetailFromLocker::set_card_network,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_isin",
                &app::model::payment_methods::CardDetailFromLocker::get_card_isin,
                &app::model::payment_methods::CardDetailFromLocker::set_card_isin,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_issuer",
                &app::model::payment_methods::CardDetailFromLocker::get_card_issuer,
                &app::model::payment_methods::CardDetailFromLocker::set_card_issuer,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_type",
                &app::model::payment_methods::CardDetailFromLocker::get_card_type,
                &app::model::payment_methods::CardDetailFromLocker::set_card_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_subtype",
                &app::model::payment_methods::CardDetailFromLocker::get_card_subtype,
                &app::model::payment_methods::CardDetailFromLocker::set_card_subtype,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_segment_type",
                &app::model::payment_methods::CardDetailFromLocker::get_card_segment_type,
                &app::model::payment_methods::CardDetailFromLocker::set_card_segment_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "funding_source",
                &app::model::payment_methods::CardDetailFromLocker::get_funding_source,
                &app::model::payment_methods::CardDetailFromLocker::set_funding_source,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "saved_to_locker",
                &app::model::payment_methods::CardDetailFromLocker::get_saved_to_locker,
                &app::model::payment_methods::CardDetailFromLocker::set_saved_to_locker,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "card_detail_from_locker";
    }
};

template<>
struct serde::Serializable<app::model::payment_methods::PaymentMethodResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "merchant_id",
                &app::model::payment_methods::PaymentMethodResponse::get_merchant_id,
                &app::model::payment_methods::PaymentMethodResponse::set_merchant_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "customer_id",
                &app::model::payment_methods::PaymentMethodResponse::get_customer_id,
                &app::model::payment_methods::PaymentMethodResponse::set_customer_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_method_id",
                &app::model::payment_methods::PaymentMethodResponse::get_payment_method_id,
                &app::model::payment_methods::PaymentMethodResponse::set_payment_method_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_method",
                &app::model::payment_methods::PaymentMethodResponse::get_payment_method,
                &app::model::payment_methods::PaymentMethodResponse::set_payment_method,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_method_type",
                &app::model::payment_methods::PaymentMethodResponse::get_payment_method_type,
                &app::model::payment_methods::PaymentMethodResponse::set_payment_method_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card",
                &app::model::payment_methods::PaymentMethodResponse::get_card,
                &app::model::payment_methods::PaymentMethodResponse::set_card,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "recurring_enabled",
                &app::model::payment_methods::PaymentMethodResponse::get_recurring_enabled,
                &app::model::payment_methods::PaymentMethodResponse::set_recurring_enabled,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "installment_payment_enabled",
                &app::model::payment_methods::PaymentMethodResponse::
                    get_installment_payment_enabled,
                &app::model::payment_methods::PaymentMethodResponse::
                    set_installment_payment_enabled,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_experience",
                &app::model::payment_methods::PaymentMethodResponse::get_payment_experience,
                &app::model::payment_methods::PaymentMethodResponse::set_payment_experience,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "metadata",
                &app::model::payment_methods::PaymentMethodResponse::get_metadata,
                &app::model::payment_methods::PaymentMethodResponse::set_metadata,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "created",
                &app::model::payment_methods::PaymentMethodResponse::get_created,
                &app::model::payment_methods::PaymentMethodResponse::set_created,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "bank_transfer",
                &app::model::payment_methods::PaymentMethodResponse::get_bank_transfer,
                &app::model::payment_methods::PaymentMethodResponse::set_bank_transfer,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "last_used_at",
                &app::model::payment_methods::PaymentMethodResponse::get_last_used_at,
                &app::model::payment_methods::PaymentMethodResponse::set_last_used_at,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "client_secret",
                &app::model::payment_methods::PaymentMethodResponse::get_client_secret,
                &app::model::payment_methods::PaymentMethodResponse::set_client_secret,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "payment_method_response";
    }
};

template<>
struct serde::Serializable<app::model::payment_methods::PaymentMethodMigrateResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "payment_method_response",
                &app::model::payment_methods::PaymentMethodMigrateResponse::
                    get_payment_method_response,
                &app::model::payment_methods::PaymentMethodMigrateResponse::
                    set_payment_method_response,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_migrated",
                &app::model::payment_methods::PaymentMethodMigrateResponse::get_card_migrated,
                &app::model::payment_methods::PaymentMethodMigrateResponse::set_card_migrated,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_method_migrated",
                &app::model::payment_methods::PaymentMethodMigrateResponse::
                    get_payment_method_migrated,
                &app::model::payment_methods::PaymentMethodMigrateResponse::
                    set_payment_method_migrated,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "network_token_migrated",
                &app::model::payment_methods::PaymentMethodMigrateResponse::
                    get_network_token_migrated,
                &app::model::payment_methods::PaymentMethodMigrateResponse::
                    set_network_token_migrated,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "connector_mandate_details_migrated",
                &app::model::payment_methods::PaymentMethodMigrateResponse::
                    get_connector_mandate_details_migrated,
                &app::model::payment_methods::PaymentMethodMigrateResponse::
                    set_connector_mandate_details_migrated,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "network_transaction_id_migrated",
                &app::model::payment_methods::PaymentMethodMigrateResponse::
                    get_network_transaction_id_migrated,
                &app::model::payment_methods::PaymentMethodMigrateResponse::
                    set_network_transaction_id_migrated,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "payment_method_migrate_response";
    }
};

template<>
struct serde::Serializable<app::model::payment_methods::PaymentMethodRecordUpdateResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "payment_method_id",
                &app::model::payment_methods::PaymentMethodRecordUpdateResponse::
                    get_payment_method_id,
                &app::model::payment_methods::PaymentMethodRecordUpdateResponse::
                    set_payment_method_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "status",
                &app::model::payment_methods::PaymentMethodRecordUpdateResponse::get_status,
                &app::model::payment_methods::PaymentMethodRecordUpdateResponse::set_status,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "network_transaction_id",
                &app::model::payment_methods::PaymentMethodRecordUpdateResponse::
                    get_network_transaction_id,
                &app::model::payment_methods::PaymentMethodRecordUpdateResponse::
                    set_network_transaction_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "connector_mandate_details",
                &app::model::payment_methods::PaymentMethodRecordUpdateResponse::
                    get_connector_mandate_details,
                &app::model::payment_methods::PaymentMethodRecordUpdateResponse::
                    set_connector_mandate_details,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "updated_payment_method_data",
                &app::model::payment_methods::PaymentMethodRecordUpdateResponse::
                    get_updated_payment_method_data,
                &app::model::payment_methods::PaymentMethodRecordUpdateResponse::
                    set_updated_payment_method_data,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "payment_method_record_update_response";
    }
};

template<>
struct serde::Serializable<app::model::payment_methods::CardDetailUpdate>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "card_exp_month",
                &app::model::payment_methods::CardDetailUpdate::get_card_exp_month,
                &app::model::payment_methods::CardDetailUpdate::set_card_exp_month,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_exp_year",
                &app::model::payment_methods::CardDetailUpdate::get_card_exp_year,
                &app::model::payment_methods::CardDetailUpdate::set_card_exp_year,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_holder_name",
                &app::model::payment_methods::CardDetailUpdate::get_card_holder_name,
                &app::model::payment_methods::CardDetailUpdate::set_card_holder_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "nick_name",
                &app::model::payment_methods::CardDetailUpdate::get_nick_name,
                &app::model::payment_methods::CardDetailUpdate::set_nick_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "last4_digits",
                &app::model::payment_methods::CardDetailUpdate::get_last4_digits,
                &app::model::payment_methods::CardDetailUpdate::set_last4_digits,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_issuer",
                &app::model::payment_methods::CardDetailUpdate::get_card_issuer,
                &app::model::payment_methods::CardDetailUpdate::set_card_issuer,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "issuer_country",
                &app::model::payment_methods::CardDetailUpdate::get_issuer_country,
                &app::model::payment_methods::CardDetailUpdate::set_issuer_country,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "issuer_country_code",
                &app::model::payment_methods::CardDetailUpdate::get_issuer_country_code,
                &app::model::payment_methods::CardDetailUpdate::set_issuer_country_code,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_network",
                &app::model::payment_methods::CardDetailUpdate::get_card_network,
                &app::model::payment_methods::CardDetailUpdate::set_card_network,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "card_detail_update";
    }
};

template<>
struct serde::Serializable<app::model::payment_methods::PaymentMethodUpdate>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "update_last_used_at",
                &app::model::payment_methods::PaymentMethodUpdate::get_update_last_used_at,
                &app::model::payment_methods::PaymentMethodUpdate::set_update_last_used_at,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card",
                &app::model::payment_methods::PaymentMethodUpdate::get_card,
                &app::model::payment_methods::PaymentMethodUpdate::set_card,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "wallet",
                &app::model::payment_methods::PaymentMethodUpdate::get_wallet,
                &app::model::payment_methods::PaymentMethodUpdate::set_wallet,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "client_secret",
                &app::model::payment_methods::PaymentMethodUpdate::get_client_secret,
                &app::model::payment_methods::PaymentMethodUpdate::set_client_secret,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "payment_method_update";
    }
};

template<>
struct serde::Serializable<app::model::payment_methods::RawCardWithNTDetails>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "card_details",
                &app::model::payment_methods::RawCardWithNTDetails::get_card_details,
                &app::model::payment_methods::RawCardWithNTDetails::set_card_details,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "network_token_details",
                &app::model::payment_methods::RawCardWithNTDetails::get_network_token_details,
                &app::model::payment_methods::RawCardWithNTDetails::set_network_token_details,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "raw_card_with_nt_details";
    }
};

template<>
struct serde::Serializable<app::model::payment_methods::RawProxyCardDataResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "card_number",
                &app::model::payment_methods::RawProxyCardDataResponse::get_card_number,
                &app::model::payment_methods::RawProxyCardDataResponse::set_card_number,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_exp_year",
                &app::model::payment_methods::RawProxyCardDataResponse::get_card_exp_year,
                &app::model::payment_methods::RawProxyCardDataResponse::set_card_exp_year,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_exp_month",
                &app::model::payment_methods::RawProxyCardDataResponse::get_card_exp_month,
                &app::model::payment_methods::RawProxyCardDataResponse::set_card_exp_month,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "raw_proxy_card_data_response";
    }
};

template<>
struct serde::Serializable<app::model::payment_methods::CustomerPaymentMethodUpdateResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "merchant_id",
                &app::model::payment_methods::CustomerPaymentMethodUpdateResponse::get_merchant_id,
                &app::model::payment_methods::CustomerPaymentMethodUpdateResponse::set_merchant_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "customer_id",
                &app::model::payment_methods::CustomerPaymentMethodUpdateResponse::get_customer_id,
                &app::model::payment_methods::CustomerPaymentMethodUpdateResponse::set_customer_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_method_id",
                &app::model::payment_methods::CustomerPaymentMethodUpdateResponse::
                    get_payment_method_id,
                &app::model::payment_methods::CustomerPaymentMethodUpdateResponse::
                    set_payment_method_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_method",
                &app::model::payment_methods::CustomerPaymentMethodUpdateResponse::
                    get_payment_method,
                &app::model::payment_methods::CustomerPaymentMethodUpdateResponse::
                    set_payment_method,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_method_type",
                &app::model::payment_methods::CustomerPaymentMethodUpdateResponse::
                    get_payment_method_type,
                &app::model::payment_methods::CustomerPaymentMethodUpdateResponse::
                    set_payment_method_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card",
                &app::model::payment_methods::CustomerPaymentMethodUpdateResponse::get_card,
                &app::model::payment_methods::CustomerPaymentMethodUpdateResponse::set_card,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "wallet",
                &app::model::payment_methods::CustomerPaymentMethodUpdateResponse::get_wallet,
                &app::model::payment_methods::CustomerPaymentMethodUpdateResponse::set_wallet,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "recurring_enabled",
                &app::model::payment_methods::CustomerPaymentMethodUpdateResponse::
                    get_recurring_enabled,
                &app::model::payment_methods::CustomerPaymentMethodUpdateResponse::
                    set_recurring_enabled,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "installment_payment_enabled",
                &app::model::payment_methods::CustomerPaymentMethodUpdateResponse::
                    get_installment_payment_enabled,
                &app::model::payment_methods::CustomerPaymentMethodUpdateResponse::
                    set_installment_payment_enabled,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_experience",
                &app::model::payment_methods::CustomerPaymentMethodUpdateResponse::
                    get_payment_experience,
                &app::model::payment_methods::CustomerPaymentMethodUpdateResponse::
                    set_payment_experience,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "metadata",
                &app::model::payment_methods::CustomerPaymentMethodUpdateResponse::get_metadata,
                &app::model::payment_methods::CustomerPaymentMethodUpdateResponse::set_metadata,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "created",
                &app::model::payment_methods::CustomerPaymentMethodUpdateResponse::get_created,
                &app::model::payment_methods::CustomerPaymentMethodUpdateResponse::set_created,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "bank_transfer",
                &app::model::payment_methods::CustomerPaymentMethodUpdateResponse::
                    get_bank_transfer,
                &app::model::payment_methods::CustomerPaymentMethodUpdateResponse::
                    set_bank_transfer,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "last_used_at",
                &app::model::payment_methods::CustomerPaymentMethodUpdateResponse::get_last_used_at,
                &app::model::payment_methods::CustomerPaymentMethodUpdateResponse::set_last_used_at,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "client_secret",
                &app::model::payment_methods::CustomerPaymentMethodUpdateResponse::
                    get_client_secret,
                &app::model::payment_methods::CustomerPaymentMethodUpdateResponse::
                    set_client_secret,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "customer_payment_method_update_response";
    }
};

template<>
struct serde::Serializable<app::model::payment_methods::NetworkTokenDetailsPaymentMethod>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "last4_digits",
                &app::model::payment_methods::NetworkTokenDetailsPaymentMethod::get_last4_digits,
                &app::model::payment_methods::NetworkTokenDetailsPaymentMethod::set_last4_digits,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "issuer_country",
                &app::model::payment_methods::NetworkTokenDetailsPaymentMethod::get_issuer_country,
                &app::model::payment_methods::NetworkTokenDetailsPaymentMethod::set_issuer_country,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "network_token_expiry_month",
                &app::model::payment_methods::NetworkTokenDetailsPaymentMethod::
                    get_network_token_expiry_month,
                &app::model::payment_methods::NetworkTokenDetailsPaymentMethod::
                    set_network_token_expiry_month,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "network_token_expiry_year",
                &app::model::payment_methods::NetworkTokenDetailsPaymentMethod::
                    get_network_token_expiry_year,
                &app::model::payment_methods::NetworkTokenDetailsPaymentMethod::
                    set_network_token_expiry_year,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "nick_name",
                &app::model::payment_methods::NetworkTokenDetailsPaymentMethod::get_nick_name,
                &app::model::payment_methods::NetworkTokenDetailsPaymentMethod::set_nick_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_holder_name",
                &app::model::payment_methods::NetworkTokenDetailsPaymentMethod::
                    get_card_holder_name,
                &app::model::payment_methods::NetworkTokenDetailsPaymentMethod::
                    set_card_holder_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_isin",
                &app::model::payment_methods::NetworkTokenDetailsPaymentMethod::get_card_isin,
                &app::model::payment_methods::NetworkTokenDetailsPaymentMethod::set_card_isin,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_issuer",
                &app::model::payment_methods::NetworkTokenDetailsPaymentMethod::get_card_issuer,
                &app::model::payment_methods::NetworkTokenDetailsPaymentMethod::set_card_issuer,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_network",
                &app::model::payment_methods::NetworkTokenDetailsPaymentMethod::get_card_network,
                &app::model::payment_methods::NetworkTokenDetailsPaymentMethod::set_card_network,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_type",
                &app::model::payment_methods::NetworkTokenDetailsPaymentMethod::get_card_type,
                &app::model::payment_methods::NetworkTokenDetailsPaymentMethod::set_card_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "saved_to_locker",
                &app::model::payment_methods::NetworkTokenDetailsPaymentMethod::get_saved_to_locker,
                &app::model::payment_methods::NetworkTokenDetailsPaymentMethod::set_saved_to_locker,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "par",
                &app::model::payment_methods::NetworkTokenDetailsPaymentMethod::get_par,
                &app::model::payment_methods::NetworkTokenDetailsPaymentMethod::set_par,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "network_token_details_payment_method";
    }
};

template<>
struct serde::Serializable<app::model::payment_methods::NetworkTokenResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<
            "payment_method_data",
            &app::model::payment_methods::NetworkTokenResponse::get_payment_method_data,
            &app::model::payment_methods::NetworkTokenResponse::set_payment_method_data,
            serde::FieldOptions{}>{}};
    }

    static constexpr std::string_view table_name()
    {
        return "network_token_response";
    }
};

template<>
struct serde::Serializable<app::model::payment_methods::PaymentMethodDetailsResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "id",
                &app::model::payment_methods::PaymentMethodDetailsResponse::get_id,
                &app::model::payment_methods::PaymentMethodDetailsResponse::set_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "merchant_id",
                &app::model::payment_methods::PaymentMethodDetailsResponse::get_merchant_id,
                &app::model::payment_methods::PaymentMethodDetailsResponse::set_merchant_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "customer_id",
                &app::model::payment_methods::PaymentMethodDetailsResponse::get_customer_id,
                &app::model::payment_methods::PaymentMethodDetailsResponse::set_customer_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_method_type",
                &app::model::payment_methods::PaymentMethodDetailsResponse::get_payment_method_type,
                &app::model::payment_methods::PaymentMethodDetailsResponse::set_payment_method_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_method_subtype",
                &app::model::payment_methods::PaymentMethodDetailsResponse::
                    get_payment_method_subtype,
                &app::model::payment_methods::PaymentMethodDetailsResponse::
                    set_payment_method_subtype,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "recurring_enabled",
                &app::model::payment_methods::PaymentMethodDetailsResponse::get_recurring_enabled,
                &app::model::payment_methods::PaymentMethodDetailsResponse::set_recurring_enabled,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "created",
                &app::model::payment_methods::PaymentMethodDetailsResponse::get_created,
                &app::model::payment_methods::PaymentMethodDetailsResponse::set_created,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "last_used_at",
                &app::model::payment_methods::PaymentMethodDetailsResponse::get_last_used_at,
                &app::model::payment_methods::PaymentMethodDetailsResponse::set_last_used_at,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_method_data",
                &app::model::payment_methods::PaymentMethodDetailsResponse::get_payment_method_data,
                &app::model::payment_methods::PaymentMethodDetailsResponse::set_payment_method_data,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "connector_tokens",
                &app::model::payment_methods::PaymentMethodDetailsResponse::get_connector_tokens,
                &app::model::payment_methods::PaymentMethodDetailsResponse::set_connector_tokens,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "network_token",
                &app::model::payment_methods::PaymentMethodDetailsResponse::get_network_token,
                &app::model::payment_methods::PaymentMethodDetailsResponse::set_network_token,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "storage_type",
                &app::model::payment_methods::PaymentMethodDetailsResponse::get_storage_type,
                &app::model::payment_methods::PaymentMethodDetailsResponse::set_storage_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "network_transaction_id",
                &app::model::payment_methods::PaymentMethodDetailsResponse::
                    get_network_transaction_id,
                &app::model::payment_methods::PaymentMethodDetailsResponse::
                    set_network_transaction_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "billing",
                &app::model::payment_methods::PaymentMethodDetailsResponse::get_billing,
                &app::model::payment_methods::PaymentMethodDetailsResponse::set_billing,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "acknowledgement_status",
                &app::model::payment_methods::PaymentMethodDetailsResponse::
                    get_acknowledgement_status,
                &app::model::payment_methods::PaymentMethodDetailsResponse::
                    set_acknowledgement_status,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "payment_method_details_response";
    }
};

template<>
struct serde::Serializable<app::model::payment_methods::CardCVCTokenStorageDetails>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "is_stored",
                &app::model::payment_methods::CardCVCTokenStorageDetails::get_is_stored,
                &app::model::payment_methods::CardCVCTokenStorageDetails::set_is_stored,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "expires_at",
                &app::model::payment_methods::CardCVCTokenStorageDetails::get_expires_at,
                &app::model::payment_methods::CardCVCTokenStorageDetails::set_expires_at,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "card_cvc_token_storage_details";
    }
};

template<>
struct serde::Serializable<app::model::payment_methods::ExternalVaultTokenData>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<
            "tokenized_card_number",
            &app::model::payment_methods::ExternalVaultTokenData::get_tokenized_card_number,
            &app::model::payment_methods::ExternalVaultTokenData::set_tokenized_card_number,
            serde::FieldOptions{}>{}};
    }

    static constexpr std::string_view table_name()
    {
        return "external_vault_token_data";
    }
};

template<>
struct serde::Serializable<app::model::payment_methods::CoBadgedCardData>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "co_badged_card_networks_info",
                &app::model::payment_methods::CoBadgedCardData::get_co_badged_card_networks_info,
                &app::model::payment_methods::CoBadgedCardData::set_co_badged_card_networks_info,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "issuer_country_code",
                &app::model::payment_methods::CoBadgedCardData::get_issuer_country_code,
                &app::model::payment_methods::CoBadgedCardData::set_issuer_country_code,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "is_regulated",
                &app::model::payment_methods::CoBadgedCardData::get_is_regulated,
                &app::model::payment_methods::CoBadgedCardData::set_is_regulated,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "regulated_name",
                &app::model::payment_methods::CoBadgedCardData::get_regulated_name,
                &app::model::payment_methods::CoBadgedCardData::set_regulated_name,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "co_badged_card_data";
    }
};

template<>
struct serde::Serializable<app::model::payment_methods::LockerCardResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "card",
                &app::model::payment_methods::LockerCardResponse::get_card,
                &app::model::payment_methods::LockerCardResponse::set_card,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "metadata",
                &app::model::payment_methods::LockerCardResponse::get_metadata,
                &app::model::payment_methods::LockerCardResponse::set_metadata,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "locker_card_response";
    }
};

template<>
struct serde::Serializable<app::model::payment_methods::MaskedBankDetails>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "mask",
                &app::model::payment_methods::MaskedBankDetails::get_mask,
                &app::model::payment_methods::MaskedBankDetails::set_mask,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "account_holder_name",
                &app::model::payment_methods::MaskedBankDetails::get_account_holder_name,
                &app::model::payment_methods::MaskedBankDetails::set_account_holder_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "bank_name",
                &app::model::payment_methods::MaskedBankDetails::get_bank_name,
                &app::model::payment_methods::MaskedBankDetails::set_bank_name,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "masked_bank_details";
    }
};

template<>
struct serde::Serializable<app::model::payment_methods::CustomerPaymentMethodResponseItem>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "payment_method_token",
                &app::model::payment_methods::CustomerPaymentMethodResponseItem::
                    get_payment_method_token,
                &app::model::payment_methods::CustomerPaymentMethodResponseItem::
                    set_payment_method_token,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "customer_id",
                &app::model::payment_methods::CustomerPaymentMethodResponseItem::get_customer_id,
                &app::model::payment_methods::CustomerPaymentMethodResponseItem::set_customer_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_method_type",
                &app::model::payment_methods::CustomerPaymentMethodResponseItem::
                    get_payment_method_type,
                &app::model::payment_methods::CustomerPaymentMethodResponseItem::
                    set_payment_method_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_method_subtype",
                &app::model::payment_methods::CustomerPaymentMethodResponseItem::
                    get_payment_method_subtype,
                &app::model::payment_methods::CustomerPaymentMethodResponseItem::
                    set_payment_method_subtype,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "recurring_enabled",
                &app::model::payment_methods::CustomerPaymentMethodResponseItem::
                    get_recurring_enabled,
                &app::model::payment_methods::CustomerPaymentMethodResponseItem::
                    set_recurring_enabled,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_method_data",
                &app::model::payment_methods::CustomerPaymentMethodResponseItem::
                    get_payment_method_data,
                &app::model::payment_methods::CustomerPaymentMethodResponseItem::
                    set_payment_method_data,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "bank",
                &app::model::payment_methods::CustomerPaymentMethodResponseItem::get_bank,
                &app::model::payment_methods::CustomerPaymentMethodResponseItem::set_bank,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "created",
                &app::model::payment_methods::CustomerPaymentMethodResponseItem::get_created,
                &app::model::payment_methods::CustomerPaymentMethodResponseItem::set_created,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "requires_cvv",
                &app::model::payment_methods::CustomerPaymentMethodResponseItem::get_requires_cvv,
                &app::model::payment_methods::CustomerPaymentMethodResponseItem::set_requires_cvv,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "last_used_at",
                &app::model::payment_methods::CustomerPaymentMethodResponseItem::get_last_used_at,
                &app::model::payment_methods::CustomerPaymentMethodResponseItem::set_last_used_at,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "is_default",
                &app::model::payment_methods::CustomerPaymentMethodResponseItem::get_is_default,
                &app::model::payment_methods::CustomerPaymentMethodResponseItem::set_is_default,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "billing",
                &app::model::payment_methods::CustomerPaymentMethodResponseItem::get_billing,
                &app::model::payment_methods::CustomerPaymentMethodResponseItem::set_billing,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "customer_payment_method_response_item";
    }
};

template<>
struct serde::Serializable<app::model::payment_methods::RequiredFieldInfo>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "required_field",
                &app::model::payment_methods::RequiredFieldInfo::get_required_field,
                &app::model::payment_methods::RequiredFieldInfo::set_required_field,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "display_name",
                &app::model::payment_methods::RequiredFieldInfo::get_display_name,
                &app::model::payment_methods::RequiredFieldInfo::set_display_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "field_type",
                &app::model::payment_methods::RequiredFieldInfo::get_field_type,
                &app::model::payment_methods::RequiredFieldInfo::set_field_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "value",
                &app::model::payment_methods::RequiredFieldInfo::get_value,
                &app::model::payment_methods::RequiredFieldInfo::set_value,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "required_field_info";
    }
};

template<>
struct serde::Serializable<app::model::payment_methods::PaymentExperienceTypes>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "payment_experience_type",
                &app::model::payment_methods::PaymentExperienceTypes::get_payment_experience_type,
                &app::model::payment_methods::PaymentExperienceTypes::set_payment_experience_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "eligible_connectors",
                &app::model::payment_methods::PaymentExperienceTypes::get_eligible_connectors,
                &app::model::payment_methods::PaymentExperienceTypes::set_eligible_connectors,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "payment_experience_types";
    }
};

template<>
struct serde::Serializable<app::model::payment_methods::SurchargePercentage>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<
            "percentage",
            &app::model::payment_methods::SurchargePercentage::get_percentage,
            &app::model::payment_methods::SurchargePercentage::set_percentage,
            serde::FieldOptions{}>{}};
    }

    static constexpr std::string_view table_name()
    {
        return "surcharge_percentage";
    }
};

template<>
struct serde::Serializable<app::model::payment_methods::SurchargeDetailsResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "surcharge",
                &app::model::payment_methods::SurchargeDetailsResponse::get_surcharge,
                &app::model::payment_methods::SurchargeDetailsResponse::set_surcharge,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "tax_on_surcharge",
                &app::model::payment_methods::SurchargeDetailsResponse::get_tax_on_surcharge,
                &app::model::payment_methods::SurchargeDetailsResponse::set_tax_on_surcharge,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "display_surcharge_amount",
                &app::model::payment_methods::SurchargeDetailsResponse::
                    get_display_surcharge_amount,
                &app::model::payment_methods::SurchargeDetailsResponse::
                    set_display_surcharge_amount,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "display_tax_on_surcharge_amount",
                &app::model::payment_methods::SurchargeDetailsResponse::
                    get_display_tax_on_surcharge_amount,
                &app::model::payment_methods::SurchargeDetailsResponse::
                    set_display_tax_on_surcharge_amount,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "display_total_surcharge_amount",
                &app::model::payment_methods::SurchargeDetailsResponse::
                    get_display_total_surcharge_amount,
                &app::model::payment_methods::SurchargeDetailsResponse::
                    set_display_total_surcharge_amount,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "surcharge_percentage",
                &app::model::payment_methods::SurchargeDetailsResponse::get_surcharge_percentage,
                &app::model::payment_methods::SurchargeDetailsResponse::set_surcharge_percentage,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "error_details",
                &app::model::payment_methods::SurchargeDetailsResponse::get_error_details,
                &app::model::payment_methods::SurchargeDetailsResponse::set_error_details,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "surcharge_details_response";
    }
};

template<>
struct serde::Serializable<app::model::payment_methods::CardNetworkTypes>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "card_network",
                &app::model::payment_methods::CardNetworkTypes::get_card_network,
                &app::model::payment_methods::CardNetworkTypes::set_card_network,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "surcharge_details",
                &app::model::payment_methods::CardNetworkTypes::get_surcharge_details,
                &app::model::payment_methods::CardNetworkTypes::set_surcharge_details,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "eligible_connectors",
                &app::model::payment_methods::CardNetworkTypes::get_eligible_connectors,
                &app::model::payment_methods::CardNetworkTypes::set_eligible_connectors,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "card_network_types";
    }
};

template<>
struct serde::Serializable<app::model::payment_methods::ResponsePaymentMethodTypes>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "payment_method_type",
                &app::model::payment_methods::ResponsePaymentMethodTypes::get_payment_method_type,
                &app::model::payment_methods::ResponsePaymentMethodTypes::set_payment_method_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_experience",
                &app::model::payment_methods::ResponsePaymentMethodTypes::get_payment_experience,
                &app::model::payment_methods::ResponsePaymentMethodTypes::set_payment_experience,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_networks",
                &app::model::payment_methods::ResponsePaymentMethodTypes::get_card_networks,
                &app::model::payment_methods::ResponsePaymentMethodTypes::set_card_networks,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "bank_names",
                &app::model::payment_methods::ResponsePaymentMethodTypes::get_bank_names,
                &app::model::payment_methods::ResponsePaymentMethodTypes::set_bank_names,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "bank_debits",
                &app::model::payment_methods::ResponsePaymentMethodTypes::get_bank_debits,
                &app::model::payment_methods::ResponsePaymentMethodTypes::set_bank_debits,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "bank_transfers",
                &app::model::payment_methods::ResponsePaymentMethodTypes::get_bank_transfers,
                &app::model::payment_methods::ResponsePaymentMethodTypes::set_bank_transfers,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "required_fields",
                &app::model::payment_methods::ResponsePaymentMethodTypes::get_required_fields,
                &app::model::payment_methods::ResponsePaymentMethodTypes::set_required_fields,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "surcharge_details",
                &app::model::payment_methods::ResponsePaymentMethodTypes::get_surcharge_details,
                &app::model::payment_methods::ResponsePaymentMethodTypes::set_surcharge_details,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "pm_auth_connector",
                &app::model::payment_methods::ResponsePaymentMethodTypes::get_pm_auth_connector,
                &app::model::payment_methods::ResponsePaymentMethodTypes::set_pm_auth_connector,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "customer_acceptance_support",
                &app::model::payment_methods::ResponsePaymentMethodTypes::
                    get_customer_acceptance_support,
                &app::model::payment_methods::ResponsePaymentMethodTypes::
                    set_customer_acceptance_support,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "response_payment_method_types";
    }
};

template<>
struct serde::Serializable<app::model::payment_methods::PaymentMethodListResponseForSession>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "payment_methods_enabled",
                &app::model::payment_methods::PaymentMethodListResponseForSession::
                    get_payment_methods_enabled,
                &app::model::payment_methods::PaymentMethodListResponseForSession::
                    set_payment_methods_enabled,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "customer_payment_methods",
                &app::model::payment_methods::PaymentMethodListResponseForSession::
                    get_customer_payment_methods,
                &app::model::payment_methods::PaymentMethodListResponseForSession::
                    set_customer_payment_methods,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "payment_method_list_response_for_session";
    }
};

template<>
struct serde::Serializable<app::model::payment_methods::ResponsePaymentMethodsEnabled>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "payment_method",
                &app::model::payment_methods::ResponsePaymentMethodsEnabled::get_payment_method,
                &app::model::payment_methods::ResponsePaymentMethodsEnabled::set_payment_method,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_method_types",
                &app::model::payment_methods::ResponsePaymentMethodsEnabled::
                    get_payment_method_types,
                &app::model::payment_methods::ResponsePaymentMethodsEnabled::
                    set_payment_method_types,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "response_payment_methods_enabled";
    }
};

template<>
struct serde::Serializable<app::model::payment_methods::ResponsePaymentMethodIntermediate>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "payment_method_type",
                &app::model::payment_methods::ResponsePaymentMethodIntermediate::
                    get_payment_method_type,
                &app::model::payment_methods::ResponsePaymentMethodIntermediate::
                    set_payment_method_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_experience",
                &app::model::payment_methods::ResponsePaymentMethodIntermediate::
                    get_payment_experience,
                &app::model::payment_methods::ResponsePaymentMethodIntermediate::
                    set_payment_experience,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_networks",
                &app::model::payment_methods::ResponsePaymentMethodIntermediate::get_card_networks,
                &app::model::payment_methods::ResponsePaymentMethodIntermediate::set_card_networks,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_method",
                &app::model::payment_methods::ResponsePaymentMethodIntermediate::get_payment_method,
                &app::model::payment_methods::ResponsePaymentMethodIntermediate::set_payment_method,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "connector",
                &app::model::payment_methods::ResponsePaymentMethodIntermediate::get_connector,
                &app::model::payment_methods::ResponsePaymentMethodIntermediate::set_connector,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "merchant_connector_id",
                &app::model::payment_methods::ResponsePaymentMethodIntermediate::
                    get_merchant_connector_id,
                &app::model::payment_methods::ResponsePaymentMethodIntermediate::
                    set_merchant_connector_id,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "response_payment_method_intermediate";
    }
};

template<>
struct serde::Serializable<app::model::payment_methods::RequestPaymentMethodTypes>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "payment_method_type",
                &app::model::payment_methods::RequestPaymentMethodTypes::get_payment_method_type,
                &app::model::payment_methods::RequestPaymentMethodTypes::set_payment_method_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_experience",
                &app::model::payment_methods::RequestPaymentMethodTypes::get_payment_experience,
                &app::model::payment_methods::RequestPaymentMethodTypes::set_payment_experience,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_networks",
                &app::model::payment_methods::RequestPaymentMethodTypes::get_card_networks,
                &app::model::payment_methods::RequestPaymentMethodTypes::set_card_networks,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "accepted_currencies",
                &app::model::payment_methods::RequestPaymentMethodTypes::get_accepted_currencies,
                &app::model::payment_methods::RequestPaymentMethodTypes::set_accepted_currencies,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "accepted_countries",
                &app::model::payment_methods::RequestPaymentMethodTypes::get_accepted_countries,
                &app::model::payment_methods::RequestPaymentMethodTypes::set_accepted_countries,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "minimum_amount",
                &app::model::payment_methods::RequestPaymentMethodTypes::get_minimum_amount,
                &app::model::payment_methods::RequestPaymentMethodTypes::set_minimum_amount,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "maximum_amount",
                &app::model::payment_methods::RequestPaymentMethodTypes::get_maximum_amount,
                &app::model::payment_methods::RequestPaymentMethodTypes::set_maximum_amount,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "recurring_enabled",
                &app::model::payment_methods::RequestPaymentMethodTypes::get_recurring_enabled,
                &app::model::payment_methods::RequestPaymentMethodTypes::set_recurring_enabled,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "installment_payment_enabled",
                &app::model::payment_methods::RequestPaymentMethodTypes::
                    get_installment_payment_enabled,
                &app::model::payment_methods::RequestPaymentMethodTypes::
                    set_installment_payment_enabled,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "pay_via_single_use_token",
                &app::model::payment_methods::RequestPaymentMethodTypes::
                    get_pay_via_single_use_token,
                &app::model::payment_methods::RequestPaymentMethodTypes::
                    set_pay_via_single_use_token,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "request_payment_method_types";
    }
};

template<>
struct serde::Serializable<app::model::payment_methods::PaymentMethodListRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "client_secret",
                &app::model::payment_methods::PaymentMethodListRequest::get_client_secret,
                &app::model::payment_methods::PaymentMethodListRequest::set_client_secret,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "accepted_countries",
                &app::model::payment_methods::PaymentMethodListRequest::get_accepted_countries,
                &app::model::payment_methods::PaymentMethodListRequest::set_accepted_countries,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "accepted_currencies",
                &app::model::payment_methods::PaymentMethodListRequest::get_accepted_currencies,
                &app::model::payment_methods::PaymentMethodListRequest::set_accepted_currencies,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "amount",
                &app::model::payment_methods::PaymentMethodListRequest::get_amount,
                &app::model::payment_methods::PaymentMethodListRequest::set_amount,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "recurring_enabled",
                &app::model::payment_methods::PaymentMethodListRequest::get_recurring_enabled,
                &app::model::payment_methods::PaymentMethodListRequest::set_recurring_enabled,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "installment_payment_enabled",
                &app::model::payment_methods::PaymentMethodListRequest::
                    get_installment_payment_enabled,
                &app::model::payment_methods::PaymentMethodListRequest::
                    set_installment_payment_enabled,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_networks",
                &app::model::payment_methods::PaymentMethodListRequest::get_card_networks,
                &app::model::payment_methods::PaymentMethodListRequest::set_card_networks,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "limit",
                &app::model::payment_methods::PaymentMethodListRequest::get_limit,
                &app::model::payment_methods::PaymentMethodListRequest::set_limit,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "payment_method_list_request";
    }
};

template<>
struct serde::Serializable<app::model::payment_methods::ListMethodsForPaymentMethodsRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "client_secret",
                &app::model::payment_methods::ListMethodsForPaymentMethodsRequest::
                    get_client_secret,
                &app::model::payment_methods::ListMethodsForPaymentMethodsRequest::
                    set_client_secret,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "accepted_countries",
                &app::model::payment_methods::ListMethodsForPaymentMethodsRequest::
                    get_accepted_countries,
                &app::model::payment_methods::ListMethodsForPaymentMethodsRequest::
                    set_accepted_countries,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "amount",
                &app::model::payment_methods::ListMethodsForPaymentMethodsRequest::get_amount,
                &app::model::payment_methods::ListMethodsForPaymentMethodsRequest::set_amount,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "accepted_currencies",
                &app::model::payment_methods::ListMethodsForPaymentMethodsRequest::
                    get_accepted_currencies,
                &app::model::payment_methods::ListMethodsForPaymentMethodsRequest::
                    set_accepted_currencies,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "recurring_enabled",
                &app::model::payment_methods::ListMethodsForPaymentMethodsRequest::
                    get_recurring_enabled,
                &app::model::payment_methods::ListMethodsForPaymentMethodsRequest::
                    set_recurring_enabled,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_networks",
                &app::model::payment_methods::ListMethodsForPaymentMethodsRequest::
                    get_card_networks,
                &app::model::payment_methods::ListMethodsForPaymentMethodsRequest::
                    set_card_networks,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "limit",
                &app::model::payment_methods::ListMethodsForPaymentMethodsRequest::get_limit,
                &app::model::payment_methods::ListMethodsForPaymentMethodsRequest::set_limit,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "include_new",
                &app::model::payment_methods::ListMethodsForPaymentMethodsRequest::get_include_new,
                &app::model::payment_methods::ListMethodsForPaymentMethodsRequest::set_include_new,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "list_methods_for_payment_methods_request";
    }
};

template<>
struct serde::Serializable<app::model::payment_methods::PaymentMethodListInstallmentAmountDetails>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "amount_per_installment",
                &app::model::payment_methods::PaymentMethodListInstallmentAmountDetails::
                    get_amount_per_installment,
                &app::model::payment_methods::PaymentMethodListInstallmentAmountDetails::
                    set_amount_per_installment,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "total_amount",
                &app::model::payment_methods::PaymentMethodListInstallmentAmountDetails::
                    get_total_amount,
                &app::model::payment_methods::PaymentMethodListInstallmentAmountDetails::
                    set_total_amount,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "payment_method_list_installment_amount_details";
    }
};

template<>
struct serde::Serializable<app::model::payment_methods::PaymentMethodListInstallmentPlan>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "number_of_installments",
                &app::model::payment_methods::PaymentMethodListInstallmentPlan::
                    get_number_of_installments,
                &app::model::payment_methods::PaymentMethodListInstallmentPlan::
                    set_number_of_installments,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "billing_frequency",
                &app::model::payment_methods::PaymentMethodListInstallmentPlan::
                    get_billing_frequency,
                &app::model::payment_methods::PaymentMethodListInstallmentPlan::
                    set_billing_frequency,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "interest_rate",
                &app::model::payment_methods::PaymentMethodListInstallmentPlan::get_interest_rate,
                &app::model::payment_methods::PaymentMethodListInstallmentPlan::set_interest_rate,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "amount_details",
                &app::model::payment_methods::PaymentMethodListInstallmentPlan::get_amount_details,
                &app::model::payment_methods::PaymentMethodListInstallmentPlan::set_amount_details,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "payment_method_list_installment_plan";
    }
};

template<>
struct serde::Serializable<app::model::payment_methods::PaymentMethodListInstallmentOption>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "payment_method",
                &app::model::payment_methods::PaymentMethodListInstallmentOption::
                    get_payment_method,
                &app::model::payment_methods::PaymentMethodListInstallmentOption::
                    set_payment_method,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "available_plans",
                &app::model::payment_methods::PaymentMethodListInstallmentOption::
                    get_available_plans,
                &app::model::payment_methods::PaymentMethodListInstallmentOption::
                    set_available_plans,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "payment_method_list_installment_option";
    }
};

template<>
struct serde::Serializable<app::model::payment_methods::PaymentMethodListIntentData>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "payment_id",
                &app::model::payment_methods::PaymentMethodListIntentData::get_payment_id,
                &app::model::payment_methods::PaymentMethodListIntentData::set_payment_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "status",
                &app::model::payment_methods::PaymentMethodListIntentData::get_status,
                &app::model::payment_methods::PaymentMethodListIntentData::set_status,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "amount",
                &app::model::payment_methods::PaymentMethodListIntentData::get_amount,
                &app::model::payment_methods::PaymentMethodListIntentData::set_amount,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "currency",
                &app::model::payment_methods::PaymentMethodListIntentData::get_currency,
                &app::model::payment_methods::PaymentMethodListIntentData::set_currency,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "client_secret",
                &app::model::payment_methods::PaymentMethodListIntentData::get_client_secret,
                &app::model::payment_methods::PaymentMethodListIntentData::set_client_secret,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "description",
                &app::model::payment_methods::PaymentMethodListIntentData::get_description,
                &app::model::payment_methods::PaymentMethodListIntentData::set_description,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "customer_id",
                &app::model::payment_methods::PaymentMethodListIntentData::get_customer_id,
                &app::model::payment_methods::PaymentMethodListIntentData::set_customer_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "return_url",
                &app::model::payment_methods::PaymentMethodListIntentData::get_return_url,
                &app::model::payment_methods::PaymentMethodListIntentData::set_return_url,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "setup_future_usage",
                &app::model::payment_methods::PaymentMethodListIntentData::get_setup_future_usage,
                &app::model::payment_methods::PaymentMethodListIntentData::set_setup_future_usage,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "billing",
                &app::model::payment_methods::PaymentMethodListIntentData::get_billing,
                &app::model::payment_methods::PaymentMethodListIntentData::set_billing,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "shipping",
                &app::model::payment_methods::PaymentMethodListIntentData::get_shipping,
                &app::model::payment_methods::PaymentMethodListIntentData::set_shipping,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "email",
                &app::model::payment_methods::PaymentMethodListIntentData::get_email,
                &app::model::payment_methods::PaymentMethodListIntentData::set_email,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "metadata",
                &app::model::payment_methods::PaymentMethodListIntentData::get_metadata,
                &app::model::payment_methods::PaymentMethodListIntentData::set_metadata,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "order_details",
                &app::model::payment_methods::PaymentMethodListIntentData::get_order_details,
                &app::model::payment_methods::PaymentMethodListIntentData::set_order_details,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "created",
                &app::model::payment_methods::PaymentMethodListIntentData::get_created,
                &app::model::payment_methods::PaymentMethodListIntentData::set_created,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "expires_on",
                &app::model::payment_methods::PaymentMethodListIntentData::get_expires_on,
                &app::model::payment_methods::PaymentMethodListIntentData::set_expires_on,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "profile_id",
                &app::model::payment_methods::PaymentMethodListIntentData::get_profile_id,
                &app::model::payment_methods::PaymentMethodListIntentData::set_profile_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "merchant_order_reference_id",
                &app::model::payment_methods::PaymentMethodListIntentData::
                    get_merchant_order_reference_id,
                &app::model::payment_methods::PaymentMethodListIntentData::
                    set_merchant_order_reference_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "attempt_count",
                &app::model::payment_methods::PaymentMethodListIntentData::get_attempt_count,
                &app::model::payment_methods::PaymentMethodListIntentData::set_attempt_count,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "installment_options",
                &app::model::payment_methods::PaymentMethodListIntentData::get_installment_options,
                &app::model::payment_methods::PaymentMethodListIntentData::set_installment_options,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "capture_method",
                &app::model::payment_methods::PaymentMethodListIntentData::get_capture_method,
                &app::model::payment_methods::PaymentMethodListIntentData::set_capture_method,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "merchant_name",
                &app::model::payment_methods::PaymentMethodListIntentData::get_merchant_name,
                &app::model::payment_methods::PaymentMethodListIntentData::set_merchant_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "mandate_payment",
                &app::model::payment_methods::PaymentMethodListIntentData::get_mandate_payment,
                &app::model::payment_methods::PaymentMethodListIntentData::set_mandate_payment,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_type",
                &app::model::payment_methods::PaymentMethodListIntentData::get_payment_type,
                &app::model::payment_methods::PaymentMethodListIntentData::set_payment_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "request_external_three_ds_authentication",
                &app::model::payment_methods::PaymentMethodListIntentData::
                    get_request_external_three_ds_authentication,
                &app::model::payment_methods::PaymentMethodListIntentData::
                    set_request_external_three_ds_authentication,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "is_tax_calculation_enabled",
                &app::model::payment_methods::PaymentMethodListIntentData::
                    get_is_tax_calculation_enabled,
                &app::model::payment_methods::PaymentMethodListIntentData::
                    set_is_tax_calculation_enabled,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "is_guest_customer",
                &app::model::payment_methods::PaymentMethodListIntentData::get_is_guest_customer,
                &app::model::payment_methods::PaymentMethodListIntentData::set_is_guest_customer,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "payment_method_list_intent_data";
    }
};

template<>
struct serde::Serializable<app::model::payment_methods::PaymentMethodListResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "redirect_url",
                &app::model::payment_methods::PaymentMethodListResponse::get_redirect_url,
                &app::model::payment_methods::PaymentMethodListResponse::set_redirect_url,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "currency",
                &app::model::payment_methods::PaymentMethodListResponse::get_currency,
                &app::model::payment_methods::PaymentMethodListResponse::set_currency,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_methods",
                &app::model::payment_methods::PaymentMethodListResponse::get_payment_methods,
                &app::model::payment_methods::PaymentMethodListResponse::set_payment_methods,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "mandate_payment",
                &app::model::payment_methods::PaymentMethodListResponse::get_mandate_payment,
                &app::model::payment_methods::PaymentMethodListResponse::set_mandate_payment,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "merchant_name",
                &app::model::payment_methods::PaymentMethodListResponse::get_merchant_name,
                &app::model::payment_methods::PaymentMethodListResponse::set_merchant_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "show_surcharge_breakup_screen",
                &app::model::payment_methods::PaymentMethodListResponse::
                    get_show_surcharge_breakup_screen,
                &app::model::payment_methods::PaymentMethodListResponse::
                    set_show_surcharge_breakup_screen,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_type",
                &app::model::payment_methods::PaymentMethodListResponse::get_payment_type,
                &app::model::payment_methods::PaymentMethodListResponse::set_payment_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "request_external_three_ds_authentication",
                &app::model::payment_methods::PaymentMethodListResponse::
                    get_request_external_three_ds_authentication,
                &app::model::payment_methods::PaymentMethodListResponse::
                    set_request_external_three_ds_authentication,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "collect_shipping_details_from_wallets",
                &app::model::payment_methods::PaymentMethodListResponse::
                    get_collect_shipping_details_from_wallets,
                &app::model::payment_methods::PaymentMethodListResponse::
                    set_collect_shipping_details_from_wallets,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "collect_billing_details_from_wallets",
                &app::model::payment_methods::PaymentMethodListResponse::
                    get_collect_billing_details_from_wallets,
                &app::model::payment_methods::PaymentMethodListResponse::
                    set_collect_billing_details_from_wallets,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "is_tax_calculation_enabled",
                &app::model::payment_methods::PaymentMethodListResponse::
                    get_is_tax_calculation_enabled,
                &app::model::payment_methods::PaymentMethodListResponse::
                    set_is_tax_calculation_enabled,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "sdk_next_action",
                &app::model::payment_methods::PaymentMethodListResponse::get_sdk_next_action,
                &app::model::payment_methods::PaymentMethodListResponse::set_sdk_next_action,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "is_guest_customer",
                &app::model::payment_methods::PaymentMethodListResponse::get_is_guest_customer,
                &app::model::payment_methods::PaymentMethodListResponse::set_is_guest_customer,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "intent_data",
                &app::model::payment_methods::PaymentMethodListResponse::get_intent_data,
                &app::model::payment_methods::PaymentMethodListResponse::set_intent_data,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "payment_method_list_response";
    }
};

template<>
struct serde::Serializable<app::model::payment_methods::PaymentMethodListIntentDataInput>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "merchant_name",
                &app::model::payment_methods::PaymentMethodListIntentDataInput::get_merchant_name,
                &app::model::payment_methods::PaymentMethodListIntentDataInput::set_merchant_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "mandate_payment",
                &app::model::payment_methods::PaymentMethodListIntentDataInput::get_mandate_payment,
                &app::model::payment_methods::PaymentMethodListIntentDataInput::set_mandate_payment,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_type",
                &app::model::payment_methods::PaymentMethodListIntentDataInput::get_payment_type,
                &app::model::payment_methods::PaymentMethodListIntentDataInput::set_payment_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "capture_method",
                &app::model::payment_methods::PaymentMethodListIntentDataInput::get_capture_method,
                &app::model::payment_methods::PaymentMethodListIntentDataInput::set_capture_method,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "payment_method_list_intent_data_input";
    }
};

template<>
struct serde::Serializable<app::model::payment_methods::ResponsePaymentMethodsEnabledForClient>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "payment_method",
                &app::model::payment_methods::ResponsePaymentMethodsEnabledForClient::
                    get_payment_method,
                &app::model::payment_methods::ResponsePaymentMethodsEnabledForClient::
                    set_payment_method,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_method_type",
                &app::model::payment_methods::ResponsePaymentMethodsEnabledForClient::
                    get_payment_method_type,
                &app::model::payment_methods::ResponsePaymentMethodsEnabledForClient::
                    set_payment_method_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "data",
                &app::model::payment_methods::ResponsePaymentMethodsEnabledForClient::get_data,
                &app::model::payment_methods::ResponsePaymentMethodsEnabledForClient::set_data,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_experience",
                &app::model::payment_methods::ResponsePaymentMethodsEnabledForClient::
                    get_payment_experience,
                &app::model::payment_methods::ResponsePaymentMethodsEnabledForClient::
                    set_payment_experience,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "collect_shipping_details_from_wallets",
                &app::model::payment_methods::ResponsePaymentMethodsEnabledForClient::
                    get_collect_shipping_details_from_wallets,
                &app::model::payment_methods::ResponsePaymentMethodsEnabledForClient::
                    set_collect_shipping_details_from_wallets,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "collect_billing_details_from_wallets",
                &app::model::payment_methods::ResponsePaymentMethodsEnabledForClient::
                    get_collect_billing_details_from_wallets,
                &app::model::payment_methods::ResponsePaymentMethodsEnabledForClient::
                    set_collect_billing_details_from_wallets,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "customer_acceptance_support",
                &app::model::payment_methods::ResponsePaymentMethodsEnabledForClient::
                    get_customer_acceptance_support,
                &app::model::payment_methods::ResponsePaymentMethodsEnabledForClient::
                    set_customer_acceptance_support,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "response_payment_methods_enabled_for_client";
    }
};

template<>
struct serde::Serializable<app::model::payment_methods::CustomerPaymentMethodForClient>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "payment_token",
                &app::model::payment_methods::CustomerPaymentMethodForClient::get_payment_token,
                &app::model::payment_methods::CustomerPaymentMethodForClient::set_payment_token,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_method",
                &app::model::payment_methods::CustomerPaymentMethodForClient::get_payment_method,
                &app::model::payment_methods::CustomerPaymentMethodForClient::set_payment_method,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_method_type",
                &app::model::payment_methods::CustomerPaymentMethodForClient::
                    get_payment_method_type,
                &app::model::payment_methods::CustomerPaymentMethodForClient::
                    set_payment_method_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "default_payment_method_set",
                &app::model::payment_methods::CustomerPaymentMethodForClient::
                    get_default_payment_method_set,
                &app::model::payment_methods::CustomerPaymentMethodForClient::
                    set_default_payment_method_set,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "requires_cvv",
                &app::model::payment_methods::CustomerPaymentMethodForClient::get_requires_cvv,
                &app::model::payment_methods::CustomerPaymentMethodForClient::set_requires_cvv,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "created",
                &app::model::payment_methods::CustomerPaymentMethodForClient::get_created,
                &app::model::payment_methods::CustomerPaymentMethodForClient::set_created,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "last_used_at",
                &app::model::payment_methods::CustomerPaymentMethodForClient::get_last_used_at,
                &app::model::payment_methods::CustomerPaymentMethodForClient::set_last_used_at,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "recurring_enabled",
                &app::model::payment_methods::CustomerPaymentMethodForClient::get_recurring_enabled,
                &app::model::payment_methods::CustomerPaymentMethodForClient::set_recurring_enabled,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_method_data",
                &app::model::payment_methods::CustomerPaymentMethodForClient::
                    get_payment_method_data,
                &app::model::payment_methods::CustomerPaymentMethodForClient::
                    set_payment_method_data,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "customer_payment_method_for_client";
    }
};

template<>
struct serde::Serializable<app::model::payment_methods::ClientPaymentMethodsListResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "payment_methods_enabled",
                &app::model::payment_methods::ClientPaymentMethodsListResponse::
                    get_payment_methods_enabled,
                &app::model::payment_methods::ClientPaymentMethodsListResponse::
                    set_payment_methods_enabled,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "customer_payment_methods",
                &app::model::payment_methods::ClientPaymentMethodsListResponse::
                    get_customer_payment_methods,
                &app::model::payment_methods::ClientPaymentMethodsListResponse::
                    set_customer_payment_methods,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "sdk_next_action",
                &app::model::payment_methods::ClientPaymentMethodsListResponse::get_sdk_next_action,
                &app::model::payment_methods::ClientPaymentMethodsListResponse::set_sdk_next_action,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "intent_data",
                &app::model::payment_methods::ClientPaymentMethodsListResponse::get_intent_data,
                &app::model::payment_methods::ClientPaymentMethodsListResponse::set_intent_data,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "client_payment_methods_list_response";
    }
};

template<>
struct serde::Serializable<app::model::payment_methods::CustomerPaymentMethod>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "payment_token",
                &app::model::payment_methods::CustomerPaymentMethod::get_payment_token,
                &app::model::payment_methods::CustomerPaymentMethod::set_payment_token,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_method_id",
                &app::model::payment_methods::CustomerPaymentMethod::get_payment_method_id,
                &app::model::payment_methods::CustomerPaymentMethod::set_payment_method_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "customer_id",
                &app::model::payment_methods::CustomerPaymentMethod::get_customer_id,
                &app::model::payment_methods::CustomerPaymentMethod::set_customer_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_method",
                &app::model::payment_methods::CustomerPaymentMethod::get_payment_method,
                &app::model::payment_methods::CustomerPaymentMethod::set_payment_method,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_method_type",
                &app::model::payment_methods::CustomerPaymentMethod::get_payment_method_type,
                &app::model::payment_methods::CustomerPaymentMethod::set_payment_method_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_method_issuer",
                &app::model::payment_methods::CustomerPaymentMethod::get_payment_method_issuer,
                &app::model::payment_methods::CustomerPaymentMethod::set_payment_method_issuer,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_method_issuer_code",
                &app::model::payment_methods::CustomerPaymentMethod::get_payment_method_issuer_code,
                &app::model::payment_methods::CustomerPaymentMethod::set_payment_method_issuer_code,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "recurring_enabled",
                &app::model::payment_methods::CustomerPaymentMethod::get_recurring_enabled,
                &app::model::payment_methods::CustomerPaymentMethod::set_recurring_enabled,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "installment_payment_enabled",
                &app::model::payment_methods::CustomerPaymentMethod::
                    get_installment_payment_enabled,
                &app::model::payment_methods::CustomerPaymentMethod::
                    set_installment_payment_enabled,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_experience",
                &app::model::payment_methods::CustomerPaymentMethod::get_payment_experience,
                &app::model::payment_methods::CustomerPaymentMethod::set_payment_experience,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card",
                &app::model::payment_methods::CustomerPaymentMethod::get_card,
                &app::model::payment_methods::CustomerPaymentMethod::set_card,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "wallets",
                &app::model::payment_methods::CustomerPaymentMethod::get_wallets,
                &app::model::payment_methods::CustomerPaymentMethod::set_wallets,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "metadata",
                &app::model::payment_methods::CustomerPaymentMethod::get_metadata,
                &app::model::payment_methods::CustomerPaymentMethod::set_metadata,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "created",
                &app::model::payment_methods::CustomerPaymentMethod::get_created,
                &app::model::payment_methods::CustomerPaymentMethod::set_created,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "bank_transfer",
                &app::model::payment_methods::CustomerPaymentMethod::get_bank_transfer,
                &app::model::payment_methods::CustomerPaymentMethod::set_bank_transfer,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "bank",
                &app::model::payment_methods::CustomerPaymentMethod::get_bank,
                &app::model::payment_methods::CustomerPaymentMethod::set_bank,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "surcharge_details",
                &app::model::payment_methods::CustomerPaymentMethod::get_surcharge_details,
                &app::model::payment_methods::CustomerPaymentMethod::set_surcharge_details,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "requires_cvv",
                &app::model::payment_methods::CustomerPaymentMethod::get_requires_cvv,
                &app::model::payment_methods::CustomerPaymentMethod::set_requires_cvv,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "last_used_at",
                &app::model::payment_methods::CustomerPaymentMethod::get_last_used_at,
                &app::model::payment_methods::CustomerPaymentMethod::set_last_used_at,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "default_payment_method_set",
                &app::model::payment_methods::CustomerPaymentMethod::get_default_payment_method_set,
                &app::model::payment_methods::CustomerPaymentMethod::set_default_payment_method_set,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "billing",
                &app::model::payment_methods::CustomerPaymentMethod::get_billing,
                &app::model::payment_methods::CustomerPaymentMethod::set_billing,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "customer_payment_method";
    }
};

template<>
struct serde::Serializable<app::model::payment_methods::CustomerPaymentMethodsListResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "customer_payment_methods",
                &app::model::payment_methods::CustomerPaymentMethodsListResponse::
                    get_customer_payment_methods,
                &app::model::payment_methods::CustomerPaymentMethodsListResponse::
                    set_customer_payment_methods,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "is_guest_customer",
                &app::model::payment_methods::CustomerPaymentMethodsListResponse::
                    get_is_guest_customer,
                &app::model::payment_methods::CustomerPaymentMethodsListResponse::
                    set_is_guest_customer,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "customer_payment_methods_list_response";
    }
};

template<>
struct serde::Serializable<app::model::payment_methods::GetTokenDataRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<
            "token_type",
            &app::model::payment_methods::GetTokenDataRequest::get_token_type,
            &app::model::payment_methods::GetTokenDataRequest::set_token_type,
            serde::FieldOptions{}>{}};
    }

    static constexpr std::string_view table_name()
    {
        return "get_token_data_request";
    }
};

template<>
struct serde::Serializable<app::model::payment_methods::TokenDataResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "payment_method_id",
                &app::model::payment_methods::TokenDataResponse::get_payment_method_id,
                &app::model::payment_methods::TokenDataResponse::set_payment_method_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "token_type",
                &app::model::payment_methods::TokenDataResponse::get_token_type,
                &app::model::payment_methods::TokenDataResponse::set_token_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "token_details",
                &app::model::payment_methods::TokenDataResponse::get_token_details,
                &app::model::payment_methods::TokenDataResponse::set_token_details,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "token_data_response";
    }
};

template<>
struct serde::Serializable<app::model::payment_methods::NetworkTokenDetailsResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "network_token",
                &app::model::payment_methods::NetworkTokenDetailsResponse::get_network_token,
                &app::model::payment_methods::NetworkTokenDetailsResponse::set_network_token,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "network_token_exp_month",
                &app::model::payment_methods::NetworkTokenDetailsResponse::
                    get_network_token_exp_month,
                &app::model::payment_methods::NetworkTokenDetailsResponse::
                    set_network_token_exp_month,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "network_token_exp_year",
                &app::model::payment_methods::NetworkTokenDetailsResponse::
                    get_network_token_exp_year,
                &app::model::payment_methods::NetworkTokenDetailsResponse::
                    set_network_token_exp_year,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "cryptogram",
                &app::model::payment_methods::NetworkTokenDetailsResponse::get_cryptogram,
                &app::model::payment_methods::NetworkTokenDetailsResponse::set_cryptogram,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_issuer",
                &app::model::payment_methods::NetworkTokenDetailsResponse::get_card_issuer,
                &app::model::payment_methods::NetworkTokenDetailsResponse::set_card_issuer,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_network",
                &app::model::payment_methods::NetworkTokenDetailsResponse::get_card_network,
                &app::model::payment_methods::NetworkTokenDetailsResponse::set_card_network,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_type",
                &app::model::payment_methods::NetworkTokenDetailsResponse::get_card_type,
                &app::model::payment_methods::NetworkTokenDetailsResponse::set_card_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_issuing_country",
                &app::model::payment_methods::NetworkTokenDetailsResponse::get_card_issuing_country,
                &app::model::payment_methods::NetworkTokenDetailsResponse::set_card_issuing_country,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "bank_code",
                &app::model::payment_methods::NetworkTokenDetailsResponse::get_bank_code,
                &app::model::payment_methods::NetworkTokenDetailsResponse::set_bank_code,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_holder_name",
                &app::model::payment_methods::NetworkTokenDetailsResponse::get_card_holder_name,
                &app::model::payment_methods::NetworkTokenDetailsResponse::set_card_holder_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "nick_name",
                &app::model::payment_methods::NetworkTokenDetailsResponse::get_nick_name,
                &app::model::payment_methods::NetworkTokenDetailsResponse::set_nick_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "eci",
                &app::model::payment_methods::NetworkTokenDetailsResponse::get_eci,
                &app::model::payment_methods::NetworkTokenDetailsResponse::set_eci,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "network_token_details_response";
    }
};

template<>
struct serde::Serializable<app::model::payment_methods::TotalPaymentMethodCountResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<
            "total_count",
            &app::model::payment_methods::TotalPaymentMethodCountResponse::get_total_count,
            &app::model::payment_methods::TotalPaymentMethodCountResponse::set_total_count,
            serde::FieldOptions{}>{}};
    }

    static constexpr std::string_view table_name()
    {
        return "total_payment_method_count_response";
    }
};

template<>
struct serde::Serializable<app::model::payment_methods::PaymentMethodDeleteResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "payment_method_id",
                &app::model::payment_methods::PaymentMethodDeleteResponse::get_payment_method_id,
                &app::model::payment_methods::PaymentMethodDeleteResponse::set_payment_method_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "deleted",
                &app::model::payment_methods::PaymentMethodDeleteResponse::get_deleted,
                &app::model::payment_methods::PaymentMethodDeleteResponse::set_deleted,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "payment_method_delete_response";
    }
};

template<>
struct serde::Serializable<app::model::payment_methods::PaymentMethodDeleteSessionResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<
            "payment_method_token",
            &app::model::payment_methods::PaymentMethodDeleteSessionResponse::
                get_payment_method_token,
            &app::model::payment_methods::PaymentMethodDeleteSessionResponse::
                set_payment_method_token,
            serde::FieldOptions{}>{}};
    }

    static constexpr std::string_view table_name()
    {
        return "payment_method_delete_session_response";
    }
};

template<>
struct serde::Serializable<app::model::payment_methods::CustomerDefaultPaymentMethodResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "default_payment_method_id",
                &app::model::payment_methods::CustomerDefaultPaymentMethodResponse::
                    get_default_payment_method_id,
                &app::model::payment_methods::CustomerDefaultPaymentMethodResponse::
                    set_default_payment_method_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "customer_id",
                &app::model::payment_methods::CustomerDefaultPaymentMethodResponse::get_customer_id,
                &app::model::payment_methods::CustomerDefaultPaymentMethodResponse::set_customer_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_method",
                &app::model::payment_methods::CustomerDefaultPaymentMethodResponse::
                    get_payment_method,
                &app::model::payment_methods::CustomerDefaultPaymentMethodResponse::
                    set_payment_method,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_method_type",
                &app::model::payment_methods::CustomerDefaultPaymentMethodResponse::
                    get_payment_method_type,
                &app::model::payment_methods::CustomerDefaultPaymentMethodResponse::
                    set_payment_method_type,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "customer_default_payment_method_response";
    }
};

template<>
struct serde::Serializable<app::model::payment_methods::PaymentMethodResponseItem>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "id",
                &app::model::payment_methods::PaymentMethodResponseItem::get_id,
                &app::model::payment_methods::PaymentMethodResponseItem::set_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "customer_id",
                &app::model::payment_methods::PaymentMethodResponseItem::get_customer_id,
                &app::model::payment_methods::PaymentMethodResponseItem::set_customer_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "merchant_customer_reference_id",
                &app::model::payment_methods::PaymentMethodResponseItem::
                    get_merchant_customer_reference_id,
                &app::model::payment_methods::PaymentMethodResponseItem::
                    set_merchant_customer_reference_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_method_type",
                &app::model::payment_methods::PaymentMethodResponseItem::get_payment_method_type,
                &app::model::payment_methods::PaymentMethodResponseItem::set_payment_method_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_method_subtype",
                &app::model::payment_methods::PaymentMethodResponseItem::get_payment_method_subtype,
                &app::model::payment_methods::PaymentMethodResponseItem::set_payment_method_subtype,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "recurring_enabled",
                &app::model::payment_methods::PaymentMethodResponseItem::get_recurring_enabled,
                &app::model::payment_methods::PaymentMethodResponseItem::set_recurring_enabled,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_method_data",
                &app::model::payment_methods::PaymentMethodResponseItem::get_payment_method_data,
                &app::model::payment_methods::PaymentMethodResponseItem::set_payment_method_data,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "created",
                &app::model::payment_methods::PaymentMethodResponseItem::get_created,
                &app::model::payment_methods::PaymentMethodResponseItem::set_created,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "requires_cvv",
                &app::model::payment_methods::PaymentMethodResponseItem::get_requires_cvv,
                &app::model::payment_methods::PaymentMethodResponseItem::set_requires_cvv,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "last_used_at",
                &app::model::payment_methods::PaymentMethodResponseItem::get_last_used_at,
                &app::model::payment_methods::PaymentMethodResponseItem::set_last_used_at,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "is_default",
                &app::model::payment_methods::PaymentMethodResponseItem::get_is_default,
                &app::model::payment_methods::PaymentMethodResponseItem::set_is_default,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "billing",
                &app::model::payment_methods::PaymentMethodResponseItem::get_billing,
                &app::model::payment_methods::PaymentMethodResponseItem::set_billing,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "network_tokenization",
                &app::model::payment_methods::PaymentMethodResponseItem::get_network_tokenization,
                &app::model::payment_methods::PaymentMethodResponseItem::set_network_tokenization,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "connector_tokens",
                &app::model::payment_methods::PaymentMethodResponseItem::get_connector_tokens,
                &app::model::payment_methods::PaymentMethodResponseItem::set_connector_tokens,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "network_transaction_id",
                &app::model::payment_methods::PaymentMethodResponseItem::get_network_transaction_id,
                &app::model::payment_methods::PaymentMethodResponseItem::set_network_transaction_id,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "payment_method_response_item";
    }
};

template<>
struct serde::Serializable<app::model::payment_methods::PaymentMethodCollectLinkRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "pm_collect_link_id",
                &app::model::payment_methods::PaymentMethodCollectLinkRequest::
                    get_pm_collect_link_id,
                &app::model::payment_methods::PaymentMethodCollectLinkRequest::
                    set_pm_collect_link_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "customer_id",
                &app::model::payment_methods::PaymentMethodCollectLinkRequest::get_customer_id,
                &app::model::payment_methods::PaymentMethodCollectLinkRequest::set_customer_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "ui_config",
                &app::model::payment_methods::PaymentMethodCollectLinkRequest::get_ui_config,
                &app::model::payment_methods::PaymentMethodCollectLinkRequest::set_ui_config,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "session_expiry",
                &app::model::payment_methods::PaymentMethodCollectLinkRequest::get_session_expiry,
                &app::model::payment_methods::PaymentMethodCollectLinkRequest::set_session_expiry,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "return_url",
                &app::model::payment_methods::PaymentMethodCollectLinkRequest::get_return_url,
                &app::model::payment_methods::PaymentMethodCollectLinkRequest::set_return_url,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "enabled_payment_methods",
                &app::model::payment_methods::PaymentMethodCollectLinkRequest::
                    get_enabled_payment_methods,
                &app::model::payment_methods::PaymentMethodCollectLinkRequest::
                    set_enabled_payment_methods,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "payment_method_collect_link_request";
    }
};

template<>
struct serde::Serializable<app::model::payment_methods::PaymentMethodCollectLinkResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "pm_collect_link_id",
                &app::model::payment_methods::PaymentMethodCollectLinkResponse::
                    get_pm_collect_link_id,
                &app::model::payment_methods::PaymentMethodCollectLinkResponse::
                    set_pm_collect_link_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "customer_id",
                &app::model::payment_methods::PaymentMethodCollectLinkResponse::get_customer_id,
                &app::model::payment_methods::PaymentMethodCollectLinkResponse::set_customer_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "expiry",
                &app::model::payment_methods::PaymentMethodCollectLinkResponse::get_expiry,
                &app::model::payment_methods::PaymentMethodCollectLinkResponse::set_expiry,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "link",
                &app::model::payment_methods::PaymentMethodCollectLinkResponse::get_link,
                &app::model::payment_methods::PaymentMethodCollectLinkResponse::set_link,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "return_url",
                &app::model::payment_methods::PaymentMethodCollectLinkResponse::get_return_url,
                &app::model::payment_methods::PaymentMethodCollectLinkResponse::set_return_url,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "ui_config",
                &app::model::payment_methods::PaymentMethodCollectLinkResponse::get_ui_config,
                &app::model::payment_methods::PaymentMethodCollectLinkResponse::set_ui_config,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "enabled_payment_methods",
                &app::model::payment_methods::PaymentMethodCollectLinkResponse::
                    get_enabled_payment_methods,
                &app::model::payment_methods::PaymentMethodCollectLinkResponse::
                    set_enabled_payment_methods,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "payment_method_collect_link_response";
    }
};

template<>
struct serde::Serializable<app::model::payment_methods::PaymentMethodCollectLinkRenderRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "merchant_id",
                &app::model::payment_methods::PaymentMethodCollectLinkRenderRequest::
                    get_merchant_id,
                &app::model::payment_methods::PaymentMethodCollectLinkRenderRequest::
                    set_merchant_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "pm_collect_link_id",
                &app::model::payment_methods::PaymentMethodCollectLinkRenderRequest::
                    get_pm_collect_link_id,
                &app::model::payment_methods::PaymentMethodCollectLinkRenderRequest::
                    set_pm_collect_link_id,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "payment_method_collect_link_render_request";
    }
};

template<>
struct serde::Serializable<app::model::payment_methods::PaymentMethodCollectLinkDetails>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "publishable_key",
                &app::model::payment_methods::PaymentMethodCollectLinkDetails::get_publishable_key,
                &app::model::payment_methods::PaymentMethodCollectLinkDetails::set_publishable_key,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "client_secret",
                &app::model::payment_methods::PaymentMethodCollectLinkDetails::get_client_secret,
                &app::model::payment_methods::PaymentMethodCollectLinkDetails::set_client_secret,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "pm_collect_link_id",
                &app::model::payment_methods::PaymentMethodCollectLinkDetails::
                    get_pm_collect_link_id,
                &app::model::payment_methods::PaymentMethodCollectLinkDetails::
                    set_pm_collect_link_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "customer_id",
                &app::model::payment_methods::PaymentMethodCollectLinkDetails::get_customer_id,
                &app::model::payment_methods::PaymentMethodCollectLinkDetails::set_customer_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "session_expiry",
                &app::model::payment_methods::PaymentMethodCollectLinkDetails::get_session_expiry,
                &app::model::payment_methods::PaymentMethodCollectLinkDetails::set_session_expiry,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "return_url",
                &app::model::payment_methods::PaymentMethodCollectLinkDetails::get_return_url,
                &app::model::payment_methods::PaymentMethodCollectLinkDetails::set_return_url,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "ui_config",
                &app::model::payment_methods::PaymentMethodCollectLinkDetails::get_ui_config,
                &app::model::payment_methods::PaymentMethodCollectLinkDetails::set_ui_config,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "enabled_payment_methods",
                &app::model::payment_methods::PaymentMethodCollectLinkDetails::
                    get_enabled_payment_methods,
                &app::model::payment_methods::PaymentMethodCollectLinkDetails::
                    set_enabled_payment_methods,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "payment_method_collect_link_details";
    }
};

template<>
struct serde::Serializable<app::model::payment_methods::PaymentMethodCollectLinkStatusDetails>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "pm_collect_link_id",
                &app::model::payment_methods::PaymentMethodCollectLinkStatusDetails::
                    get_pm_collect_link_id,
                &app::model::payment_methods::PaymentMethodCollectLinkStatusDetails::
                    set_pm_collect_link_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "customer_id",
                &app::model::payment_methods::PaymentMethodCollectLinkStatusDetails::
                    get_customer_id,
                &app::model::payment_methods::PaymentMethodCollectLinkStatusDetails::
                    set_customer_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "session_expiry",
                &app::model::payment_methods::PaymentMethodCollectLinkStatusDetails::
                    get_session_expiry,
                &app::model::payment_methods::PaymentMethodCollectLinkStatusDetails::
                    set_session_expiry,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "return_url",
                &app::model::payment_methods::PaymentMethodCollectLinkStatusDetails::get_return_url,
                &app::model::payment_methods::PaymentMethodCollectLinkStatusDetails::set_return_url,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "status",
                &app::model::payment_methods::PaymentMethodCollectLinkStatusDetails::get_status,
                &app::model::payment_methods::PaymentMethodCollectLinkStatusDetails::set_status,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "ui_config",
                &app::model::payment_methods::PaymentMethodCollectLinkStatusDetails::get_ui_config,
                &app::model::payment_methods::PaymentMethodCollectLinkStatusDetails::set_ui_config,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "payment_method_collect_link_status_details";
    }
};

template<>
struct serde::Serializable<app::model::payment_methods::PaymentMethodId>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<
            "payment_method_id",
            &app::model::payment_methods::PaymentMethodId::get_payment_method_id,
            &app::model::payment_methods::PaymentMethodId::set_payment_method_id,
            serde::FieldOptions{}>{}};
    }

    static constexpr std::string_view table_name()
    {
        return "payment_method_id";
    }
};

template<>
struct serde::Serializable<app::model::payment_methods::ModularPaymentMethodMigrationRecord>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "merchant_id",
                &app::model::payment_methods::ModularPaymentMethodMigrationRecord::get_merchant_id,
                &app::model::payment_methods::ModularPaymentMethodMigrationRecord::set_merchant_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_method_id",
                &app::model::payment_methods::ModularPaymentMethodMigrationRecord::
                    get_payment_method_id,
                &app::model::payment_methods::ModularPaymentMethodMigrationRecord::
                    set_payment_method_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "line_number",
                &app::model::payment_methods::ModularPaymentMethodMigrationRecord::get_line_number,
                &app::model::payment_methods::ModularPaymentMethodMigrationRecord::set_line_number,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "modular_payment_method_migration_record";
    }
};

template<>
struct serde::Serializable<app::model::payment_methods::DefaultPaymentMethod>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "customer_id",
                &app::model::payment_methods::DefaultPaymentMethod::get_customer_id,
                &app::model::payment_methods::DefaultPaymentMethod::set_customer_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_method_id",
                &app::model::payment_methods::DefaultPaymentMethod::get_payment_method_id,
                &app::model::payment_methods::DefaultPaymentMethod::set_payment_method_id,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "default_payment_method";
    }
};

template<>
struct serde::Serializable<app::model::payment_methods::TokenizePayloadEncrypted>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "payload",
                &app::model::payment_methods::TokenizePayloadEncrypted::get_payload,
                &app::model::payment_methods::TokenizePayloadEncrypted::set_payload,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "key_id",
                &app::model::payment_methods::TokenizePayloadEncrypted::get_key_id,
                &app::model::payment_methods::TokenizePayloadEncrypted::set_key_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "version",
                &app::model::payment_methods::TokenizePayloadEncrypted::get_version,
                &app::model::payment_methods::TokenizePayloadEncrypted::set_version,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "tokenize_payload_encrypted";
    }
};

template<>
struct serde::Serializable<app::model::payment_methods::TokenizePayloadRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "value1",
                &app::model::payment_methods::TokenizePayloadRequest::get_value1,
                &app::model::payment_methods::TokenizePayloadRequest::set_value1,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "value2",
                &app::model::payment_methods::TokenizePayloadRequest::get_value2,
                &app::model::payment_methods::TokenizePayloadRequest::set_value2,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "lookup_key",
                &app::model::payment_methods::TokenizePayloadRequest::get_lookup_key,
                &app::model::payment_methods::TokenizePayloadRequest::set_lookup_key,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "service_name",
                &app::model::payment_methods::TokenizePayloadRequest::get_service_name,
                &app::model::payment_methods::TokenizePayloadRequest::set_service_name,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "tokenize_payload_request";
    }
};

template<>
struct serde::Serializable<app::model::payment_methods::GetTokenizePayloadRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "lookup_key",
                &app::model::payment_methods::GetTokenizePayloadRequest::get_lookup_key,
                &app::model::payment_methods::GetTokenizePayloadRequest::set_lookup_key,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "service_name",
                &app::model::payment_methods::GetTokenizePayloadRequest::get_service_name,
                &app::model::payment_methods::GetTokenizePayloadRequest::set_service_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "get_value2",
                &app::model::payment_methods::GetTokenizePayloadRequest::get_get_value2,
                &app::model::payment_methods::GetTokenizePayloadRequest::set_get_value2,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "get_tokenize_payload_request";
    }
};

template<>
struct serde::Serializable<app::model::payment_methods::DeleteTokenizeByTokenRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "lookup_key",
                &app::model::payment_methods::DeleteTokenizeByTokenRequest::get_lookup_key,
                &app::model::payment_methods::DeleteTokenizeByTokenRequest::set_lookup_key,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "service_name",
                &app::model::payment_methods::DeleteTokenizeByTokenRequest::get_service_name,
                &app::model::payment_methods::DeleteTokenizeByTokenRequest::set_service_name,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "delete_tokenize_by_token_request";
    }
};

template<>
struct serde::Serializable<app::model::payment_methods::DeleteTokenizeByDateRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "buffer_minutes",
                &app::model::payment_methods::DeleteTokenizeByDateRequest::get_buffer_minutes,
                &app::model::payment_methods::DeleteTokenizeByDateRequest::set_buffer_minutes,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "service_name",
                &app::model::payment_methods::DeleteTokenizeByDateRequest::get_service_name,
                &app::model::payment_methods::DeleteTokenizeByDateRequest::set_service_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "max_rows",
                &app::model::payment_methods::DeleteTokenizeByDateRequest::get_max_rows,
                &app::model::payment_methods::DeleteTokenizeByDateRequest::set_max_rows,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "delete_tokenize_by_date_request";
    }
};

template<>
struct serde::Serializable<app::model::payment_methods::GetTokenizePayloadResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "lookup_key",
                &app::model::payment_methods::GetTokenizePayloadResponse::get_lookup_key,
                &app::model::payment_methods::GetTokenizePayloadResponse::set_lookup_key,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "get_value2",
                &app::model::payment_methods::GetTokenizePayloadResponse::get_get_value2,
                &app::model::payment_methods::GetTokenizePayloadResponse::set_get_value2,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "get_tokenize_payload_response";
    }
};

template<>
struct serde::Serializable<app::model::payment_methods::ListCountriesCurrenciesRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "connector",
                &app::model::payment_methods::ListCountriesCurrenciesRequest::get_connector,
                &app::model::payment_methods::ListCountriesCurrenciesRequest::set_connector,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "paymentMethodType",
                &app::model::payment_methods::ListCountriesCurrenciesRequest::
                    get_payment_method_type,
                &app::model::payment_methods::ListCountriesCurrenciesRequest::
                    set_payment_method_type,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "list_countries_currencies_request";
    }
};

template<>
struct serde::Serializable<app::model::payment_methods::CountryCodeWithName>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "code",
                &app::model::payment_methods::CountryCodeWithName::get_code,
                &app::model::payment_methods::CountryCodeWithName::set_code,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "name",
                &app::model::payment_methods::CountryCodeWithName::get_name,
                &app::model::payment_methods::CountryCodeWithName::set_name,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "country_code_with_name";
    }
};

template<>
struct serde::Serializable<app::model::payment_methods::ListCountriesCurrenciesResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "currencies",
                &app::model::payment_methods::ListCountriesCurrenciesResponse::get_currencies,
                &app::model::payment_methods::ListCountriesCurrenciesResponse::set_currencies,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "countries",
                &app::model::payment_methods::ListCountriesCurrenciesResponse::get_countries,
                &app::model::payment_methods::ListCountriesCurrenciesResponse::set_countries,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "list_countries_currencies_response";
    }
};

template<>
struct serde::Serializable<app::model::payment_methods::PaymentMethodRecord>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "customer_id",
                &app::model::payment_methods::PaymentMethodRecord::get_customer_id,
                &app::model::payment_methods::PaymentMethodRecord::set_customer_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "name",
                &app::model::payment_methods::PaymentMethodRecord::get_name,
                &app::model::payment_methods::PaymentMethodRecord::set_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_holder_name",
                &app::model::payment_methods::PaymentMethodRecord::get_card_holder_name,
                &app::model::payment_methods::PaymentMethodRecord::set_card_holder_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "email",
                &app::model::payment_methods::PaymentMethodRecord::get_email,
                &app::model::payment_methods::PaymentMethodRecord::set_email,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "phone",
                &app::model::payment_methods::PaymentMethodRecord::get_phone,
                &app::model::payment_methods::PaymentMethodRecord::set_phone,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "phone_country_code",
                &app::model::payment_methods::PaymentMethodRecord::get_phone_country_code,
                &app::model::payment_methods::PaymentMethodRecord::set_phone_country_code,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "merchant_id",
                &app::model::payment_methods::PaymentMethodRecord::get_merchant_id,
                &app::model::payment_methods::PaymentMethodRecord::set_merchant_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_method",
                &app::model::payment_methods::PaymentMethodRecord::get_payment_method,
                &app::model::payment_methods::PaymentMethodRecord::set_payment_method,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_method_type",
                &app::model::payment_methods::PaymentMethodRecord::get_payment_method_type,
                &app::model::payment_methods::PaymentMethodRecord::set_payment_method_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "nick_name",
                &app::model::payment_methods::PaymentMethodRecord::get_nick_name,
                &app::model::payment_methods::PaymentMethodRecord::set_nick_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_instrument_id",
                &app::model::payment_methods::PaymentMethodRecord::get_payment_instrument_id,
                &app::model::payment_methods::PaymentMethodRecord::set_payment_instrument_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "connector_customer_id",
                &app::model::payment_methods::PaymentMethodRecord::get_connector_customer_id,
                &app::model::payment_methods::PaymentMethodRecord::set_connector_customer_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "connector_mandate_request_reference_id",
                &app::model::payment_methods::PaymentMethodRecord::
                    get_connector_mandate_request_reference_id,
                &app::model::payment_methods::PaymentMethodRecord::
                    set_connector_mandate_request_reference_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_number_masked",
                &app::model::payment_methods::PaymentMethodRecord::get_card_number_masked,
                &app::model::payment_methods::PaymentMethodRecord::set_card_number_masked,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_expiry_month",
                &app::model::payment_methods::PaymentMethodRecord::get_card_expiry_month,
                &app::model::payment_methods::PaymentMethodRecord::set_card_expiry_month,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_expiry_year",
                &app::model::payment_methods::PaymentMethodRecord::get_card_expiry_year,
                &app::model::payment_methods::PaymentMethodRecord::set_card_expiry_year,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_scheme",
                &app::model::payment_methods::PaymentMethodRecord::get_card_scheme,
                &app::model::payment_methods::PaymentMethodRecord::set_card_scheme,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "original_transaction_id",
                &app::model::payment_methods::PaymentMethodRecord::get_original_transaction_id,
                &app::model::payment_methods::PaymentMethodRecord::set_original_transaction_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "billing_address_zip",
                &app::model::payment_methods::PaymentMethodRecord::get_billing_address_zip,
                &app::model::payment_methods::PaymentMethodRecord::set_billing_address_zip,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "billing_address_state",
                &app::model::payment_methods::PaymentMethodRecord::get_billing_address_state,
                &app::model::payment_methods::PaymentMethodRecord::set_billing_address_state,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "billing_address_first_name",
                &app::model::payment_methods::PaymentMethodRecord::get_billing_address_first_name,
                &app::model::payment_methods::PaymentMethodRecord::set_billing_address_first_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "billing_address_last_name",
                &app::model::payment_methods::PaymentMethodRecord::get_billing_address_last_name,
                &app::model::payment_methods::PaymentMethodRecord::set_billing_address_last_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "billing_address_city",
                &app::model::payment_methods::PaymentMethodRecord::get_billing_address_city,
                &app::model::payment_methods::PaymentMethodRecord::set_billing_address_city,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "billing_address_country",
                &app::model::payment_methods::PaymentMethodRecord::get_billing_address_country,
                &app::model::payment_methods::PaymentMethodRecord::set_billing_address_country,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "billing_address_line1",
                &app::model::payment_methods::PaymentMethodRecord::get_billing_address_line1,
                &app::model::payment_methods::PaymentMethodRecord::set_billing_address_line1,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "billing_address_line2",
                &app::model::payment_methods::PaymentMethodRecord::get_billing_address_line2,
                &app::model::payment_methods::PaymentMethodRecord::set_billing_address_line2,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "billing_address_line3",
                &app::model::payment_methods::PaymentMethodRecord::get_billing_address_line3,
                &app::model::payment_methods::PaymentMethodRecord::set_billing_address_line3,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "raw_card_number",
                &app::model::payment_methods::PaymentMethodRecord::get_raw_card_number,
                &app::model::payment_methods::PaymentMethodRecord::set_raw_card_number,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "merchant_connector_id",
                &app::model::payment_methods::PaymentMethodRecord::get_merchant_connector_id,
                &app::model::payment_methods::PaymentMethodRecord::set_merchant_connector_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "merchant_connector_ids",
                &app::model::payment_methods::PaymentMethodRecord::get_merchant_connector_ids,
                &app::model::payment_methods::PaymentMethodRecord::set_merchant_connector_ids,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "original_transaction_amount",
                &app::model::payment_methods::PaymentMethodRecord::get_original_transaction_amount,
                &app::model::payment_methods::PaymentMethodRecord::set_original_transaction_amount,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "original_transaction_currency",
                &app::model::payment_methods::PaymentMethodRecord::
                    get_original_transaction_currency,
                &app::model::payment_methods::PaymentMethodRecord::
                    set_original_transaction_currency,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "line_number",
                &app::model::payment_methods::PaymentMethodRecord::get_line_number,
                &app::model::payment_methods::PaymentMethodRecord::set_line_number,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "network_token_number",
                &app::model::payment_methods::PaymentMethodRecord::get_network_token_number,
                &app::model::payment_methods::PaymentMethodRecord::set_network_token_number,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "network_token_expiry_month",
                &app::model::payment_methods::PaymentMethodRecord::get_network_token_expiry_month,
                &app::model::payment_methods::PaymentMethodRecord::set_network_token_expiry_month,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "network_token_expiry_year",
                &app::model::payment_methods::PaymentMethodRecord::get_network_token_expiry_year,
                &app::model::payment_methods::PaymentMethodRecord::set_network_token_expiry_year,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "network_token_requestor_ref_id",
                &app::model::payment_methods::PaymentMethodRecord::
                    get_network_token_requestor_ref_id,
                &app::model::payment_methods::PaymentMethodRecord::
                    set_network_token_requestor_ref_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "account_number",
                &app::model::payment_methods::PaymentMethodRecord::get_account_number,
                &app::model::payment_methods::PaymentMethodRecord::set_account_number,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "routing_number",
                &app::model::payment_methods::PaymentMethodRecord::get_routing_number,
                &app::model::payment_methods::PaymentMethodRecord::set_routing_number,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "bank_account_holder_name",
                &app::model::payment_methods::PaymentMethodRecord::get_bank_account_holder_name,
                &app::model::payment_methods::PaymentMethodRecord::set_bank_account_holder_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "bank_type",
                &app::model::payment_methods::PaymentMethodRecord::get_bank_type,
                &app::model::payment_methods::PaymentMethodRecord::set_bank_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "bank_holder_type",
                &app::model::payment_methods::PaymentMethodRecord::get_bank_holder_type,
                &app::model::payment_methods::PaymentMethodRecord::set_bank_holder_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "bank_name",
                &app::model::payment_methods::PaymentMethodRecord::get_bank_name,
                &app::model::payment_methods::PaymentMethodRecord::set_bank_name,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "payment_method_record";
    }
};

template<>
struct serde::Serializable<app::model::payment_methods::UpdatePaymentMethodRecord>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "payment_method_id",
                &app::model::payment_methods::UpdatePaymentMethodRecord::get_payment_method_id,
                &app::model::payment_methods::UpdatePaymentMethodRecord::set_payment_method_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "status",
                &app::model::payment_methods::UpdatePaymentMethodRecord::get_status,
                &app::model::payment_methods::UpdatePaymentMethodRecord::set_status,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "network_transaction_id",
                &app::model::payment_methods::UpdatePaymentMethodRecord::get_network_transaction_id,
                &app::model::payment_methods::UpdatePaymentMethodRecord::set_network_transaction_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "line_number",
                &app::model::payment_methods::UpdatePaymentMethodRecord::get_line_number,
                &app::model::payment_methods::UpdatePaymentMethodRecord::set_line_number,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_instrument_id",
                &app::model::payment_methods::UpdatePaymentMethodRecord::get_payment_instrument_id,
                &app::model::payment_methods::UpdatePaymentMethodRecord::set_payment_instrument_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "connector_customer_id",
                &app::model::payment_methods::UpdatePaymentMethodRecord::get_connector_customer_id,
                &app::model::payment_methods::UpdatePaymentMethodRecord::set_connector_customer_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "merchant_connector_ids",
                &app::model::payment_methods::UpdatePaymentMethodRecord::get_merchant_connector_ids,
                &app::model::payment_methods::UpdatePaymentMethodRecord::set_merchant_connector_ids,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_expiry_month",
                &app::model::payment_methods::UpdatePaymentMethodRecord::get_card_expiry_month,
                &app::model::payment_methods::UpdatePaymentMethodRecord::set_card_expiry_month,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_expiry_year",
                &app::model::payment_methods::UpdatePaymentMethodRecord::get_card_expiry_year,
                &app::model::payment_methods::UpdatePaymentMethodRecord::set_card_expiry_year,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_method_type",
                &app::model::payment_methods::UpdatePaymentMethodRecord::get_payment_method_type,
                &app::model::payment_methods::UpdatePaymentMethodRecord::set_payment_method_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_network",
                &app::model::payment_methods::UpdatePaymentMethodRecord::get_card_network,
                &app::model::payment_methods::UpdatePaymentMethodRecord::set_card_network,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_type",
                &app::model::payment_methods::UpdatePaymentMethodRecord::get_card_type,
                &app::model::payment_methods::UpdatePaymentMethodRecord::set_card_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_issuer",
                &app::model::payment_methods::UpdatePaymentMethodRecord::get_card_issuer,
                &app::model::payment_methods::UpdatePaymentMethodRecord::set_card_issuer,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "update_payment_method_record";
    }
};

template<>
struct serde::Serializable<app::model::payment_methods::PaymentMethodsBatchRecord>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "payment_method_id",
                &app::model::payment_methods::PaymentMethodsBatchRecord::get_payment_method_id,
                &app::model::payment_methods::PaymentMethodsBatchRecord::set_payment_method_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "line_number",
                &app::model::payment_methods::PaymentMethodsBatchRecord::get_line_number,
                &app::model::payment_methods::PaymentMethodsBatchRecord::set_line_number,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "payment_methods_batch_record";
    }
};

template<>
struct serde::Serializable<app::model::payment_methods::PaymentMethodsBatchRetrieveResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "payment_method_id",
                &app::model::payment_methods::PaymentMethodsBatchRetrieveResponse::
                    get_payment_method_id,
                &app::model::payment_methods::PaymentMethodsBatchRetrieveResponse::
                    set_payment_method_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_method_type",
                &app::model::payment_methods::PaymentMethodsBatchRetrieveResponse::
                    get_payment_method_type,
                &app::model::payment_methods::PaymentMethodsBatchRetrieveResponse::
                    set_payment_method_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_method_subtype",
                &app::model::payment_methods::PaymentMethodsBatchRetrieveResponse::
                    get_payment_method_subtype,
                &app::model::payment_methods::PaymentMethodsBatchRetrieveResponse::
                    set_payment_method_subtype,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_method_data",
                &app::model::payment_methods::PaymentMethodsBatchRetrieveResponse::
                    get_payment_method_data,
                &app::model::payment_methods::PaymentMethodsBatchRetrieveResponse::
                    set_payment_method_data,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "error_message",
                &app::model::payment_methods::PaymentMethodsBatchRetrieveResponse::
                    get_error_message,
                &app::model::payment_methods::PaymentMethodsBatchRetrieveResponse::
                    set_error_message,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "line_number",
                &app::model::payment_methods::PaymentMethodsBatchRetrieveResponse::get_line_number,
                &app::model::payment_methods::PaymentMethodsBatchRetrieveResponse::set_line_number,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "payment_methods_batch_retrieve_response";
    }
};

template<>
struct serde::Serializable<app::model::payment_methods::PaymentMethodUpdateResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "payment_method_id",
                &app::model::payment_methods::PaymentMethodUpdateResponse::get_payment_method_id,
                &app::model::payment_methods::PaymentMethodUpdateResponse::set_payment_method_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "status",
                &app::model::payment_methods::PaymentMethodUpdateResponse::get_status,
                &app::model::payment_methods::PaymentMethodUpdateResponse::set_status,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "network_transaction_id",
                &app::model::payment_methods::PaymentMethodUpdateResponse::
                    get_network_transaction_id,
                &app::model::payment_methods::PaymentMethodUpdateResponse::
                    set_network_transaction_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "connector_mandate_details",
                &app::model::payment_methods::PaymentMethodUpdateResponse::
                    get_connector_mandate_details,
                &app::model::payment_methods::PaymentMethodUpdateResponse::
                    set_connector_mandate_details,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "update_status",
                &app::model::payment_methods::PaymentMethodUpdateResponse::get_update_status,
                &app::model::payment_methods::PaymentMethodUpdateResponse::set_update_status,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "update_error",
                &app::model::payment_methods::PaymentMethodUpdateResponse::get_update_error,
                &app::model::payment_methods::PaymentMethodUpdateResponse::set_update_error,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "updated_payment_method_data",
                &app::model::payment_methods::PaymentMethodUpdateResponse::
                    get_updated_payment_method_data,
                &app::model::payment_methods::PaymentMethodUpdateResponse::
                    set_updated_payment_method_data,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "line_number",
                &app::model::payment_methods::PaymentMethodUpdateResponse::get_line_number,
                &app::model::payment_methods::PaymentMethodUpdateResponse::set_line_number,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "payment_method_update_response";
    }
};

template<>
struct serde::Serializable<app::model::payment_methods::PaymentMethodMigrationResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "line_number",
                &app::model::payment_methods::PaymentMethodMigrationResponse::get_line_number,
                &app::model::payment_methods::PaymentMethodMigrationResponse::set_line_number,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_method_id",
                &app::model::payment_methods::PaymentMethodMigrationResponse::get_payment_method_id,
                &app::model::payment_methods::PaymentMethodMigrationResponse::set_payment_method_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_method",
                &app::model::payment_methods::PaymentMethodMigrationResponse::get_payment_method,
                &app::model::payment_methods::PaymentMethodMigrationResponse::set_payment_method,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_method_type",
                &app::model::payment_methods::PaymentMethodMigrationResponse::
                    get_payment_method_type,
                &app::model::payment_methods::PaymentMethodMigrationResponse::
                    set_payment_method_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "customer_id",
                &app::model::payment_methods::PaymentMethodMigrationResponse::get_customer_id,
                &app::model::payment_methods::PaymentMethodMigrationResponse::set_customer_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "migration_status",
                &app::model::payment_methods::PaymentMethodMigrationResponse::get_migration_status,
                &app::model::payment_methods::PaymentMethodMigrationResponse::set_migration_status,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "migration_error",
                &app::model::payment_methods::PaymentMethodMigrationResponse::get_migration_error,
                &app::model::payment_methods::PaymentMethodMigrationResponse::set_migration_error,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_number_masked",
                &app::model::payment_methods::PaymentMethodMigrationResponse::
                    get_card_number_masked,
                &app::model::payment_methods::PaymentMethodMigrationResponse::
                    set_card_number_masked,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_migrated",
                &app::model::payment_methods::PaymentMethodMigrationResponse::get_card_migrated,
                &app::model::payment_methods::PaymentMethodMigrationResponse::set_card_migrated,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_method_migrated",
                &app::model::payment_methods::PaymentMethodMigrationResponse::
                    get_payment_method_migrated,
                &app::model::payment_methods::PaymentMethodMigrationResponse::
                    set_payment_method_migrated,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "network_token_migrated",
                &app::model::payment_methods::PaymentMethodMigrationResponse::
                    get_network_token_migrated,
                &app::model::payment_methods::PaymentMethodMigrationResponse::
                    set_network_token_migrated,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "connector_mandate_details_migrated",
                &app::model::payment_methods::PaymentMethodMigrationResponse::
                    get_connector_mandate_details_migrated,
                &app::model::payment_methods::PaymentMethodMigrationResponse::
                    set_connector_mandate_details_migrated,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "network_transaction_id_migrated",
                &app::model::payment_methods::PaymentMethodMigrationResponse::
                    get_network_transaction_id_migrated,
                &app::model::payment_methods::PaymentMethodMigrationResponse::
                    set_network_transaction_id_migrated,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "payment_method_migration_response";
    }
};

template<>
struct serde::Serializable<app::model::payment_methods::ModularPaymentMethodMigrationRowResult>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "row_number",
                &app::model::payment_methods::ModularPaymentMethodMigrationRowResult::
                    get_row_number,
                &app::model::payment_methods::ModularPaymentMethodMigrationRowResult::
                    set_row_number,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "merchant_id",
                &app::model::payment_methods::ModularPaymentMethodMigrationRowResult::
                    get_merchant_id,
                &app::model::payment_methods::ModularPaymentMethodMigrationRowResult::
                    set_merchant_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_method_id",
                &app::model::payment_methods::ModularPaymentMethodMigrationRowResult::
                    get_payment_method_id,
                &app::model::payment_methods::ModularPaymentMethodMigrationRowResult::
                    set_payment_method_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "old_fingerprint_id",
                &app::model::payment_methods::ModularPaymentMethodMigrationRowResult::
                    get_old_fingerprint_id,
                &app::model::payment_methods::ModularPaymentMethodMigrationRowResult::
                    set_old_fingerprint_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "new_fingerprint_id",
                &app::model::payment_methods::ModularPaymentMethodMigrationRowResult::
                    get_new_fingerprint_id,
                &app::model::payment_methods::ModularPaymentMethodMigrationRowResult::
                    set_new_fingerprint_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "migration_status",
                &app::model::payment_methods::ModularPaymentMethodMigrationRowResult::
                    get_migration_status,
                &app::model::payment_methods::ModularPaymentMethodMigrationRowResult::
                    set_migration_status,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "error_message",
                &app::model::payment_methods::ModularPaymentMethodMigrationRowResult::
                    get_error_message,
                &app::model::payment_methods::ModularPaymentMethodMigrationRowResult::
                    set_error_message,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "modular_payment_method_migration_row_result";
    }
};

template<>
struct serde::Serializable<app::model::payment_methods::ModularPaymentMethodMigrationResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "total_rows",
                &app::model::payment_methods::ModularPaymentMethodMigrationResponse::get_total_rows,
                &app::model::payment_methods::ModularPaymentMethodMigrationResponse::set_total_rows,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "successful_count",
                &app::model::payment_methods::ModularPaymentMethodMigrationResponse::
                    get_successful_count,
                &app::model::payment_methods::ModularPaymentMethodMigrationResponse::
                    set_successful_count,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "failed_count",
                &app::model::payment_methods::ModularPaymentMethodMigrationResponse::
                    get_failed_count,
                &app::model::payment_methods::ModularPaymentMethodMigrationResponse::
                    set_failed_count,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "results",
                &app::model::payment_methods::ModularPaymentMethodMigrationResponse::get_results,
                &app::model::payment_methods::ModularPaymentMethodMigrationResponse::set_results,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "modular_payment_method_migration_response";
    }
};

template<>
struct serde::Serializable<app::model::payment_methods::CardNetworkTokenizeRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "merchant_id",
                &app::model::payment_methods::CardNetworkTokenizeRequest::get_merchant_id,
                &app::model::payment_methods::CardNetworkTokenizeRequest::set_merchant_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "data",
                &app::model::payment_methods::CardNetworkTokenizeRequest::get_data,
                &app::model::payment_methods::CardNetworkTokenizeRequest::set_data,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "customer",
                &app::model::payment_methods::CardNetworkTokenizeRequest::get_customer,
                &app::model::payment_methods::CardNetworkTokenizeRequest::set_customer,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "billing",
                &app::model::payment_methods::CardNetworkTokenizeRequest::get_billing,
                &app::model::payment_methods::CardNetworkTokenizeRequest::set_billing,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "metadata",
                &app::model::payment_methods::CardNetworkTokenizeRequest::get_metadata,
                &app::model::payment_methods::CardNetworkTokenizeRequest::set_metadata,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_method_issuer",
                &app::model::payment_methods::CardNetworkTokenizeRequest::get_payment_method_issuer,
                &app::model::payment_methods::CardNetworkTokenizeRequest::set_payment_method_issuer,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "card_network_tokenize_request";
    }
};

template<>
struct serde::Serializable<app::model::payment_methods::TokenizeCardRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "raw_card_number",
                &app::model::payment_methods::TokenizeCardRequest::get_raw_card_number,
                &app::model::payment_methods::TokenizeCardRequest::set_raw_card_number,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_expiry_month",
                &app::model::payment_methods::TokenizeCardRequest::get_card_expiry_month,
                &app::model::payment_methods::TokenizeCardRequest::set_card_expiry_month,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_expiry_year",
                &app::model::payment_methods::TokenizeCardRequest::get_card_expiry_year,
                &app::model::payment_methods::TokenizeCardRequest::set_card_expiry_year,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_cvc",
                &app::model::payment_methods::TokenizeCardRequest::get_card_cvc,
                &app::model::payment_methods::TokenizeCardRequest::set_card_cvc,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_holder_name",
                &app::model::payment_methods::TokenizeCardRequest::get_card_holder_name,
                &app::model::payment_methods::TokenizeCardRequest::set_card_holder_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "nick_name",
                &app::model::payment_methods::TokenizeCardRequest::get_nick_name,
                &app::model::payment_methods::TokenizeCardRequest::set_nick_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_issuing_country",
                &app::model::payment_methods::TokenizeCardRequest::get_card_issuing_country,
                &app::model::payment_methods::TokenizeCardRequest::set_card_issuing_country,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_issuing_country_code",
                &app::model::payment_methods::TokenizeCardRequest::get_card_issuing_country_code,
                &app::model::payment_methods::TokenizeCardRequest::set_card_issuing_country_code,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_network",
                &app::model::payment_methods::TokenizeCardRequest::get_card_network,
                &app::model::payment_methods::TokenizeCardRequest::set_card_network,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_issuer",
                &app::model::payment_methods::TokenizeCardRequest::get_card_issuer,
                &app::model::payment_methods::TokenizeCardRequest::set_card_issuer,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_type",
                &app::model::payment_methods::TokenizeCardRequest::get_card_type,
                &app::model::payment_methods::TokenizeCardRequest::set_card_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_subtype",
                &app::model::payment_methods::TokenizeCardRequest::get_card_subtype,
                &app::model::payment_methods::TokenizeCardRequest::set_card_subtype,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_segment_type",
                &app::model::payment_methods::TokenizeCardRequest::get_card_segment_type,
                &app::model::payment_methods::TokenizeCardRequest::set_card_segment_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "funding_source",
                &app::model::payment_methods::TokenizeCardRequest::get_funding_source,
                &app::model::payment_methods::TokenizeCardRequest::set_funding_source,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "tokenize_card_request";
    }
};

template<>
struct serde::Serializable<app::model::payment_methods::TokenizePaymentMethodRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "payment_method_id",
                &app::model::payment_methods::TokenizePaymentMethodRequest::get_payment_method_id,
                &app::model::payment_methods::TokenizePaymentMethodRequest::set_payment_method_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_cvc",
                &app::model::payment_methods::TokenizePaymentMethodRequest::get_card_cvc,
                &app::model::payment_methods::TokenizePaymentMethodRequest::set_card_cvc,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "tokenize_payment_method_request";
    }
};

template<>
struct serde::Serializable<app::model::payment_methods::CardNetworkTokenizeResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "payment_method_response",
                &app::model::payment_methods::CardNetworkTokenizeResponse::
                    get_payment_method_response,
                &app::model::payment_methods::CardNetworkTokenizeResponse::
                    set_payment_method_response,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "customer",
                &app::model::payment_methods::CardNetworkTokenizeResponse::get_customer,
                &app::model::payment_methods::CardNetworkTokenizeResponse::set_customer,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_tokenized",
                &app::model::payment_methods::CardNetworkTokenizeResponse::get_card_tokenized,
                &app::model::payment_methods::CardNetworkTokenizeResponse::set_card_tokenized,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "error_code",
                &app::model::payment_methods::CardNetworkTokenizeResponse::get_error_code,
                &app::model::payment_methods::CardNetworkTokenizeResponse::set_error_code,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "error_message",
                &app::model::payment_methods::CardNetworkTokenizeResponse::get_error_message,
                &app::model::payment_methods::CardNetworkTokenizeResponse::set_error_message,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "tokenization_data",
                &app::model::payment_methods::CardNetworkTokenizeResponse::get_tokenization_data,
                &app::model::payment_methods::CardNetworkTokenizeResponse::set_tokenization_data,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "card_network_tokenize_response";
    }
};

template<>
struct serde::Serializable<app::model::payment_methods::PaymentMethodSessionRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "customer_id",
                &app::model::payment_methods::PaymentMethodSessionRequest::get_customer_id,
                &app::model::payment_methods::PaymentMethodSessionRequest::set_customer_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "billing",
                &app::model::payment_methods::PaymentMethodSessionRequest::get_billing,
                &app::model::payment_methods::PaymentMethodSessionRequest::set_billing,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "return_url",
                &app::model::payment_methods::PaymentMethodSessionRequest::get_return_url,
                &app::model::payment_methods::PaymentMethodSessionRequest::set_return_url,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "network_tokenization",
                &app::model::payment_methods::PaymentMethodSessionRequest::get_network_tokenization,
                &app::model::payment_methods::PaymentMethodSessionRequest::set_network_tokenization,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "expires_in",
                &app::model::payment_methods::PaymentMethodSessionRequest::get_expires_in,
                &app::model::payment_methods::PaymentMethodSessionRequest::set_expires_in,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "tokenization_data",
                &app::model::payment_methods::PaymentMethodSessionRequest::get_tokenization_data,
                &app::model::payment_methods::PaymentMethodSessionRequest::set_tokenization_data,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "storage_type",
                &app::model::payment_methods::PaymentMethodSessionRequest::get_storage_type,
                &app::model::payment_methods::PaymentMethodSessionRequest::set_storage_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "keep_alive",
                &app::model::payment_methods::PaymentMethodSessionRequest::get_keep_alive,
                &app::model::payment_methods::PaymentMethodSessionRequest::set_keep_alive,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "payment_method_session_request";
    }
};

template<>
struct serde::Serializable<app::model::payment_methods::PaymentMethodsSessionUpdateRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "billing",
                &app::model::payment_methods::PaymentMethodsSessionUpdateRequest::get_billing,
                &app::model::payment_methods::PaymentMethodsSessionUpdateRequest::set_billing,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "network_tokenization",
                &app::model::payment_methods::PaymentMethodsSessionUpdateRequest::
                    get_network_tokenization,
                &app::model::payment_methods::PaymentMethodsSessionUpdateRequest::
                    set_network_tokenization,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "tokenization_data",
                &app::model::payment_methods::PaymentMethodsSessionUpdateRequest::
                    get_tokenization_data,
                &app::model::payment_methods::PaymentMethodsSessionUpdateRequest::
                    set_tokenization_data,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "payment_methods_session_update_request";
    }
};

template<>
struct serde::Serializable<
    app::model::payment_methods::PaymentMethodSessionUpdateSavedPaymentMethod>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "payment_method_token",
                &app::model::payment_methods::PaymentMethodSessionUpdateSavedPaymentMethod::
                    get_payment_method_token,
                &app::model::payment_methods::PaymentMethodSessionUpdateSavedPaymentMethod::
                    set_payment_method_token,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_method_update_request",
                &app::model::payment_methods::PaymentMethodSessionUpdateSavedPaymentMethod::
                    get_payment_method_update_request,
                &app::model::payment_methods::PaymentMethodSessionUpdateSavedPaymentMethod::
                    set_payment_method_update_request,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "payment_method_session_update_saved_payment_method";
    }
};

template<>
struct serde::Serializable<
    app::model::payment_methods::PaymentMethodSessionDeleteSavedPaymentMethod>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<
            "payment_method_token",
            &app::model::payment_methods::PaymentMethodSessionDeleteSavedPaymentMethod::
                get_payment_method_token,
            &app::model::payment_methods::PaymentMethodSessionDeleteSavedPaymentMethod::
                set_payment_method_token,
            serde::FieldOptions{}>{}};
    }

    static constexpr std::string_view table_name()
    {
        return "payment_method_session_delete_saved_payment_method";
    }
};

template<>
struct serde::Serializable<app::model::payment_methods::PaymentMethodSessionConfirmRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "payment_method_type",
                &app::model::payment_methods::PaymentMethodSessionConfirmRequest::
                    get_payment_method_type,
                &app::model::payment_methods::PaymentMethodSessionConfirmRequest::
                    set_payment_method_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_method_subtype",
                &app::model::payment_methods::PaymentMethodSessionConfirmRequest::
                    get_payment_method_subtype,
                &app::model::payment_methods::PaymentMethodSessionConfirmRequest::
                    set_payment_method_subtype,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_method_data",
                &app::model::payment_methods::PaymentMethodSessionConfirmRequest::
                    get_payment_method_data,
                &app::model::payment_methods::PaymentMethodSessionConfirmRequest::
                    set_payment_method_data,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "return_url",
                &app::model::payment_methods::PaymentMethodSessionConfirmRequest::get_return_url,
                &app::model::payment_methods::PaymentMethodSessionConfirmRequest::set_return_url,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "customer_acceptance",
                &app::model::payment_methods::PaymentMethodSessionConfirmRequest::
                    get_customer_acceptance,
                &app::model::payment_methods::PaymentMethodSessionConfirmRequest::
                    set_customer_acceptance,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "payment_method_session_confirm_request";
    }
};

template<>
struct serde::Serializable<app::model::payment_methods::AuthenticationDetails>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "status",
                &app::model::payment_methods::AuthenticationDetails::get_status,
                &app::model::payment_methods::AuthenticationDetails::set_status,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "error",
                &app::model::payment_methods::AuthenticationDetails::get_error,
                &app::model::payment_methods::AuthenticationDetails::set_error,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "authentication_details";
    }
};

template<>
struct serde::Serializable<app::model::payment_methods::PaymentMethodSessionResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "id",
                &app::model::payment_methods::PaymentMethodSessionResponse::get_id,
                &app::model::payment_methods::PaymentMethodSessionResponse::set_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "customer_id",
                &app::model::payment_methods::PaymentMethodSessionResponse::get_customer_id,
                &app::model::payment_methods::PaymentMethodSessionResponse::set_customer_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "billing",
                &app::model::payment_methods::PaymentMethodSessionResponse::get_billing,
                &app::model::payment_methods::PaymentMethodSessionResponse::set_billing,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "network_tokenization",
                &app::model::payment_methods::PaymentMethodSessionResponse::
                    get_network_tokenization,
                &app::model::payment_methods::PaymentMethodSessionResponse::
                    set_network_tokenization,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "tokenization_data",
                &app::model::payment_methods::PaymentMethodSessionResponse::get_tokenization_data,
                &app::model::payment_methods::PaymentMethodSessionResponse::set_tokenization_data,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "expires_at",
                &app::model::payment_methods::PaymentMethodSessionResponse::get_expires_at,
                &app::model::payment_methods::PaymentMethodSessionResponse::set_expires_at,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "client_secret",
                &app::model::payment_methods::PaymentMethodSessionResponse::get_client_secret,
                &app::model::payment_methods::PaymentMethodSessionResponse::set_client_secret,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "return_url",
                &app::model::payment_methods::PaymentMethodSessionResponse::get_return_url,
                &app::model::payment_methods::PaymentMethodSessionResponse::set_return_url,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "next_action",
                &app::model::payment_methods::PaymentMethodSessionResponse::get_next_action,
                &app::model::payment_methods::PaymentMethodSessionResponse::set_next_action,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "authentication_details",
                &app::model::payment_methods::PaymentMethodSessionResponse::
                    get_authentication_details,
                &app::model::payment_methods::PaymentMethodSessionResponse::
                    set_authentication_details,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "associated_payment_methods",
                &app::model::payment_methods::PaymentMethodSessionResponse::
                    get_associated_payment_methods,
                &app::model::payment_methods::PaymentMethodSessionResponse::
                    set_associated_payment_methods,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "associated_token_id",
                &app::model::payment_methods::PaymentMethodSessionResponse::get_associated_token_id,
                &app::model::payment_methods::PaymentMethodSessionResponse::set_associated_token_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "storage_type",
                &app::model::payment_methods::PaymentMethodSessionResponse::get_storage_type,
                &app::model::payment_methods::PaymentMethodSessionResponse::set_storage_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_cvc_token_storage",
                &app::model::payment_methods::PaymentMethodSessionResponse::
                    get_card_cvc_token_storage,
                &app::model::payment_methods::PaymentMethodSessionResponse::
                    set_card_cvc_token_storage,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_method_data",
                &app::model::payment_methods::PaymentMethodSessionResponse::get_payment_method_data,
                &app::model::payment_methods::PaymentMethodSessionResponse::set_payment_method_data,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "sdk_authorization",
                &app::model::payment_methods::PaymentMethodSessionResponse::get_sdk_authorization,
                &app::model::payment_methods::PaymentMethodSessionResponse::set_sdk_authorization,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "keep_alive",
                &app::model::payment_methods::PaymentMethodSessionResponse::get_keep_alive,
                &app::model::payment_methods::PaymentMethodSessionResponse::set_keep_alive,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "network_tokenization_data",
                &app::model::payment_methods::PaymentMethodSessionResponse::
                    get_network_tokenization_data,
                &app::model::payment_methods::PaymentMethodSessionResponse::
                    set_network_tokenization_data,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "external_vault_details",
                &app::model::payment_methods::PaymentMethodSessionResponse::
                    get_external_vault_details,
                &app::model::payment_methods::PaymentMethodSessionResponse::
                    set_external_vault_details,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "payment_method_session_response";
    }
};

template<>
struct serde::Serializable<app::model::payment_methods::NetworkTokenStatusCheckSuccessResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "status",
                &app::model::payment_methods::NetworkTokenStatusCheckSuccessResponse::get_status,
                &app::model::payment_methods::NetworkTokenStatusCheckSuccessResponse::set_status,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "token_expiry_month",
                &app::model::payment_methods::NetworkTokenStatusCheckSuccessResponse::
                    get_token_expiry_month,
                &app::model::payment_methods::NetworkTokenStatusCheckSuccessResponse::
                    set_token_expiry_month,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "token_expiry_year",
                &app::model::payment_methods::NetworkTokenStatusCheckSuccessResponse::
                    get_token_expiry_year,
                &app::model::payment_methods::NetworkTokenStatusCheckSuccessResponse::
                    set_token_expiry_year,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_last_four",
                &app::model::payment_methods::NetworkTokenStatusCheckSuccessResponse::
                    get_card_last_four,
                &app::model::payment_methods::NetworkTokenStatusCheckSuccessResponse::
                    set_card_last_four,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "token_last_four",
                &app::model::payment_methods::NetworkTokenStatusCheckSuccessResponse::
                    get_token_last_four,
                &app::model::payment_methods::NetworkTokenStatusCheckSuccessResponse::
                    set_token_last_four,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_expiry_month",
                &app::model::payment_methods::NetworkTokenStatusCheckSuccessResponse::
                    get_card_expiry_month,
                &app::model::payment_methods::NetworkTokenStatusCheckSuccessResponse::
                    set_card_expiry_month,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_expiry_year",
                &app::model::payment_methods::NetworkTokenStatusCheckSuccessResponse::
                    get_card_expiry_year,
                &app::model::payment_methods::NetworkTokenStatusCheckSuccessResponse::
                    set_card_expiry_year,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_method_id",
                &app::model::payment_methods::NetworkTokenStatusCheckSuccessResponse::
                    get_payment_method_id,
                &app::model::payment_methods::NetworkTokenStatusCheckSuccessResponse::
                    set_payment_method_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "customer_id",
                &app::model::payment_methods::NetworkTokenStatusCheckSuccessResponse::
                    get_customer_id,
                &app::model::payment_methods::NetworkTokenStatusCheckSuccessResponse::
                    set_customer_id,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "network_token_status_check_success_response";
    }
};

template<>
struct serde::Serializable<app::model::payment_methods::NetworkTokenStatusCheckFailureResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<
            "error_message",
            &app::model::payment_methods::NetworkTokenStatusCheckFailureResponse::get_error_message,
            &app::model::payment_methods::NetworkTokenStatusCheckFailureResponse::set_error_message,
            serde::FieldOptions{}>{}};
    }

    static constexpr std::string_view table_name()
    {
        return "network_token_status_check_failure_response";
    }
};

template<>
struct serde::Serializable<app::model::payment_methods::NetworkTokenEligibilityRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<
            "card_bin",
            &app::model::payment_methods::NetworkTokenEligibilityRequest::get_card_bin,
            &app::model::payment_methods::NetworkTokenEligibilityRequest::set_card_bin,
            serde::FieldOptions{}>{}};
    }

    static constexpr std::string_view table_name()
    {
        return "network_token_eligibility_request";
    }
};

template<>
struct serde::Serializable<app::model::payment_methods::GetNetworkTokenEiligibilityResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<
            "eligible_for_network_tokenization",
            &app::model::payment_methods::GetNetworkTokenEiligibilityResponse::
                get_eligible_for_network_tokenization,
            &app::model::payment_methods::GetNetworkTokenEiligibilityResponse::
                set_eligible_for_network_tokenization,
            serde::FieldOptions{}>{}};
    }

    static constexpr std::string_view table_name()
    {
        return "get_network_token_eiligibility_response";
    }
};

template<>
struct serde::Serializable<app::model::payment_methods::PaymentMethodGetTokenDetailsResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "id",
                &app::model::payment_methods::PaymentMethodGetTokenDetailsResponse::get_id,
                &app::model::payment_methods::PaymentMethodGetTokenDetailsResponse::set_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_method_token",
                &app::model::payment_methods::PaymentMethodGetTokenDetailsResponse::
                    get_payment_method_token,
                &app::model::payment_methods::PaymentMethodGetTokenDetailsResponse::
                    set_payment_method_token,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "payment_method_get_token_details_response";
    }
};

#ifdef CONGELADO_TEST
namespace app::model::payment_methods::tests {
using namespace boost::ut;
suite << app_model_payment_methods >> app_model_payment_methods_suite = []
{
    "reflects converted types"_test = []
    {
        expect(
            std::tuple_size_v<
                decltype(serde::Serializable<
                         app::model::payment_methods::PaymentMethodCreate>::fields())> > 0
        );
    };
};
} // namespace app::model::payment_methods::tests
#endif
