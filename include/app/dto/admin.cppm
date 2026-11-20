module;
export module app_dto:admin;

import std;
import core_serde;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace app::dto::admin {

enum class AcceptedCountries
{
    EnableOnly,
    DisableOnly,
    AllAccepted,
};

enum class AcceptedCurrencies
{
    EnableOnly,
    DisableOnly,
    AllAccepted,
};

enum class AdditionalMerchantData
{
    OpenBankingRecipientData,
};

class AddressDetailsValue
{
public:
    void set_value(std::string v) { value = std::move(v); }
    [[nodiscard]] const std::string& get_value() const noexcept { return value; }
    std::string value{};
};

class AlwaysEnableOvercaptureBoolValue
{
public:
    void set_value(std::string v) { value = std::move(v); }
    [[nodiscard]] const std::string& get_value() const noexcept { return value; }
    std::string value{};
};

class AlwaysRequestExtendedAuthorizationValue
{
public:
    void set_value(std::string v) { value = std::move(v); }
    [[nodiscard]] const std::string& get_value() const noexcept { return value; }
    std::string value{};
};

class AuthenticationConnectorAccountMapValue
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

class CardNetworkValue
{
public:
    void set_value(std::string v) { value = std::move(v); }
    [[nodiscard]] const std::string& get_value() const noexcept { return value; }
    std::string value{};
};

class CardSegmentTypeValue
{
public:
    void set_value(std::string v) { value = std::move(v); }
    [[nodiscard]] const std::string& get_value() const noexcept { return value; }
    std::string value{};
};

enum class CardTestingGuardStatus
{
    Enabled,
    Disabled,
};

class CardTypeValue
{
public:
    void set_value(std::string v) { value = std::move(v); }
    [[nodiscard]] const std::string& get_value() const noexcept { return value; }
    std::string value{};
};

class ConnectorAgnosticMitChoice
{
public:
    void set_enabled(bool value) { enabled = std::move(value); }
    [[nodiscard]] const bool& get_enabled() const noexcept { return enabled; }
    bool enabled{};
};

enum class ConnectorAuthType
{
    TemporaryAuth,
    HeaderKey,
    BodyKey,
    SignatureKey,
    MultiAuthKey,
    CurrencyAuthKey,
    CertificateAuth,
    NoKey,
};

class ConnectorStatusValue
{
public:
    void set_value(std::string v) { value = std::move(v); }
    [[nodiscard]] const std::string& get_value() const noexcept { return value; }
    std::string value{};
};

class ConnectorTypeValue
{
public:
    void set_value(std::string v) { value = std::move(v); }
    [[nodiscard]] const std::string& get_value() const noexcept { return value; }
    std::string value{};
};

class ConnectorValue
{
public:
    void set_value(std::string v) { value = std::move(v); }
    [[nodiscard]] const std::string& get_value() const noexcept { return value; }
    std::string value{};
};

class CountryAlpha2Value
{
public:
    void set_value(std::string v) { value = std::move(v); }
    [[nodiscard]] const std::string& get_value() const noexcept { return value; }
    std::string value{};
};

class DisputePollingIntervalInHoursValue
{
public:
    void set_value(std::string v) { value = std::move(v); }
    [[nodiscard]] const std::string& get_value() const noexcept { return value; }
    std::string value{};
};

class DisputeStatusValue
{
public:
    void set_value(std::string v) { value = std::move(v); }
    [[nodiscard]] const std::string& get_value() const noexcept { return value; }
    std::string value{};
};

class ElementPositionValue
{
public:
    void set_value(std::string v) { value = std::move(v); }
    [[nodiscard]] const std::string& get_value() const noexcept { return value; }
    std::string value{};
};

class ElementSizeValue
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

class EnabledPaymentMethodValue
{
public:
    void set_value(std::string v) { value = std::move(v); }
    [[nodiscard]] const std::string& get_value() const noexcept { return value; }
    std::string value{};
};

class ExtendedCardInfoChoice
{
public:
    void set_enabled(bool value) { enabled = std::move(value); }
    [[nodiscard]] const bool& get_enabled() const noexcept { return enabled; }
    bool enabled{};
};

class ExternalVaultEnabledValue
{
public:
    void set_value(std::string v) { value = std::move(v); }
    [[nodiscard]] const std::string& get_value() const noexcept { return value; }
    std::string value{};
};

class FrmActionValue
{
public:
    void set_value(std::string v) { value = std::move(v); }
    [[nodiscard]] const std::string& get_value() const noexcept { return value; }
    std::string value{};
};

class FrmPreferredFlowTypesValue
{
public:
    void set_value(std::string v) { value = std::move(v); }
    [[nodiscard]] const std::string& get_value() const noexcept { return value; }
    std::string value{};
};

class FundingSourceValue
{
public:
    void set_value(std::string v) { value = std::move(v); }
    [[nodiscard]] const std::string& get_value() const noexcept { return value; }
    std::string value{};
};

class GenericLinkUiConfigValue
{
public:
    void set_value(std::string v) { value = std::move(v); }
    [[nodiscard]] const std::string& get_value() const noexcept { return value; }
    std::string value{};
};

class IntentStatusValue
{
public:
    void set_value(std::string v) { value = std::move(v); }
    [[nodiscard]] const std::string& get_value() const noexcept { return value; }
    std::string value{};
};

class InternalNetworkTokenizationCredentials
{
public:
    void set_token_service_api_key(std::string value) { token_service_api_key = std::move(value); }
    [[nodiscard]] const std::string& get_token_service_api_key() const noexcept { return token_service_api_key; }
    void set_public_key(std::string value) { public_key = std::move(value); }
    [[nodiscard]] const std::string& get_public_key() const noexcept { return public_key; }
    void set_private_key(std::string value) { private_key = std::move(value); }
    [[nodiscard]] const std::string& get_private_key() const noexcept { return private_key; }
    void set_key_id(std::string value) { key_id = std::move(value); }
    [[nodiscard]] const std::string& get_key_id() const noexcept { return key_id; }
    std::string token_service_api_key{};
    std::string public_key{};
    std::string private_key{};
    std::string key_id{};
};

class InvoiceStatusValue
{
public:
    void set_value(std::string v) { value = std::move(v); }
    [[nodiscard]] const std::string& get_value() const noexcept { return value; }
    std::string value{};
};

class MandateStatusValue
{
public:
    void set_value(std::string v) { value = std::move(v); }
    [[nodiscard]] const std::string& get_value() const noexcept { return value; }
    std::string value{};
};

class MaskedHeaders
{
public:
    void set_value(std::unordered_map<std::string, std::string> v) { value = std::move(v); }
    [[nodiscard]] const std::unordered_map<std::string, std::string>& get_value() const noexcept { return value; }
    std::unordered_map<std::string, std::string> value{};
};

enum class MerchantAccountData
{
    Iban,
    Bacs,
    FasterPayments,
    Sepa,
    SepaInstant,
    Elixir,
    Bankgiro,
    Plusgiro,
};

class MerchantAccountTypeValue
{
public:
    void set_value(std::string v) { value = std::move(v); }
    [[nodiscard]] const std::string& get_value() const noexcept { return value; }
    std::string value{};
};

class MerchantCategoryCodeValue
{
public:
    void set_value(std::string v) { value = std::move(v); }
    [[nodiscard]] const std::string& get_value() const noexcept { return value; }
    std::string value{};
};

class MerchantConnectorAccountIdValue
{
public:
    void set_value(std::string v) { value = std::move(v); }
    [[nodiscard]] const std::string& get_value() const noexcept { return value; }
    std::string value{};
};

class MerchantConnectorDetailsWrap
{
public:
    void set_creds_identifier(std::string value) { creds_identifier = std::move(value); }
    [[nodiscard]] const std::string& get_creds_identifier() const noexcept { return creds_identifier; }
    void set_encoded_data(std::optional<std::string> value) { encoded_data = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_encoded_data() const noexcept { return encoded_data; }
    std::string creds_identifier{};
    std::optional<std::string> encoded_data{};
};

class MerchantConnectorWebhookDetails
{
public:
    void set_merchant_secret(std::string value) { merchant_secret = std::move(value); }
    [[nodiscard]] const std::string& get_merchant_secret() const noexcept { return merchant_secret; }
    void set_additional_secret(std::optional<std::string> value) { additional_secret = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_additional_secret() const noexcept { return additional_secret; }
    std::string merchant_secret{};
    std::optional<std::string> additional_secret{};
};

class MerchantCountryCodeValue
{
public:
    void set_value(std::string v) { value = std::move(v); }
    [[nodiscard]] const std::string& get_value() const noexcept { return value; }
    std::string value{};
};

class MerchantIdValue
{
public:
    void set_value(std::string v) { value = std::move(v); }
    [[nodiscard]] const std::string& get_value() const noexcept { return value; }
    std::string value{};
};

class MerchantKeyTransferRequest
{
public:
    void set_from(std::uint32_t value) { from = std::move(value); }
    [[nodiscard]] const std::uint32_t& get_from() const noexcept { return from; }
    void set_limit(std::uint32_t value) { limit = std::move(value); }
    [[nodiscard]] const std::uint32_t& get_limit() const noexcept { return limit; }
    std::uint32_t from{};
    std::uint32_t limit{};
};

class MerchantProductTypeValue
{
public:
    void set_value(std::string v) { value = std::move(v); }
    [[nodiscard]] const std::string& get_value() const noexcept { return value; }
    std::string value{};
};

enum class MerchantRecipientData
{
    ConnectorRecipientId,
    WalletId,
    AccountData,
};

enum class NetworkTokeizationProviderCredentials
{
    InternalNetworkTokenService,
};

class OfferEngineMerchantConfig
{
public:
    void set_api_key(std::string value) { api_key = std::move(value); }
    [[nodiscard]] const std::string& get_api_key() const noexcept { return api_key; }
    void set_merchant_id(std::string value) { merchant_id = std::move(value); }
    [[nodiscard]] const std::string& get_merchant_id() const noexcept { return merchant_id; }
    std::string api_key{};
    std::string merchant_id{};
};

class OptionalEncryptableNameValue
{
public:
    void set_value(std::string v) { value = std::move(v); }
    [[nodiscard]] const std::string& get_value() const noexcept { return value; }
    std::string value{};
};

class OrderFulfillmentTime
{
public:
    void set_value(std::int64_t v) { value = std::move(v); }
    [[nodiscard]] const std::int64_t& get_value() const noexcept { return value; }
    std::int64_t value{};
};

class OrganizationIdValue
{
public:
    void set_value(std::string v) { value = std::move(v); }
    [[nodiscard]] const std::string& get_value() const noexcept { return value; }
    std::string value{};
};

class PaymentLinkDetailsLayoutValue
{
public:
    void set_value(std::string v) { value = std::move(v); }
    [[nodiscard]] const std::string& get_value() const noexcept { return value; }
    std::string value{};
};

class PaymentLinkSdkLabelTypeValue
{
public:
    void set_value(std::string v) { value = std::move(v); }
    [[nodiscard]] const std::string& get_value() const noexcept { return value; }
    std::string value{};
};

class PaymentLinkShowSdkTermsValue
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

class PaymentMethodsConfigValue
{
public:
    void set_value(std::string v) { value = std::move(v); }
    [[nodiscard]] const std::string& get_value() const noexcept { return value; }
    std::string value{};
};

class PaymentMethodsEnabledValue
{
public:
    void set_value(std::string v) { value = std::move(v); }
    [[nodiscard]] const std::string& get_value() const noexcept { return value; }
    std::string value{};
};

class PayoutStatusValue
{
public:
    void set_value(std::string v) { value = std::move(v); }
    [[nodiscard]] const std::string& get_value() const noexcept { return value; }
    std::string value{};
};

class ProfileAcquirerConfigsResponseValue
{
public:
    void set_value(std::string v) { value = std::move(v); }
    [[nodiscard]] const std::string& get_value() const noexcept { return value; }
    std::string value{};
};

class ProfileAcquirerResponseValue
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

class ReconStatusValue
{
public:
    void set_value(std::string v) { value = std::move(v); }
    [[nodiscard]] const std::string& get_value() const noexcept { return value; }
    std::string value{};
};

class RefundStatusValue
{
public:
    void set_value(std::string v) { value = std::move(v); }
    [[nodiscard]] const std::string& get_value() const noexcept { return value; }
    std::string value{};
};

class RequestPaymentMethodTypesValue
{
public:
    void set_value(std::string v) { value = std::move(v); }
    [[nodiscard]] const std::string& get_value() const noexcept { return value; }
    std::string value{};
};

class RevenueRecoveryConfig
{
public:
    void set_max_retry_count(std::uint16_t value) { max_retry_count = std::move(value); }
    [[nodiscard]] const std::uint16_t& get_max_retry_count() const noexcept { return max_retry_count; }
    void set_billing_connector_retry_threshold(std::uint16_t value) { billing_connector_retry_threshold = std::move(value); }
    [[nodiscard]] const std::uint16_t& get_billing_connector_retry_threshold() const noexcept { return billing_connector_retry_threshold; }
    void set_max_hybrid_cascading_retry_count(std::uint16_t value) { max_hybrid_cascading_retry_count = std::move(value); }
    [[nodiscard]] const std::uint16_t& get_max_hybrid_cascading_retry_count() const noexcept { return max_hybrid_cascading_retry_count; }
    void set_grace_period_days(std::uint16_t value) { grace_period_days = std::move(value); }
    [[nodiscard]] const std::uint16_t& get_grace_period_days() const noexcept { return grace_period_days; }
    void set_enable_partial_authorization(bool value) { enable_partial_authorization = std::move(value); }
    [[nodiscard]] const bool& get_enable_partial_authorization() const noexcept { return enable_partial_authorization; }
    void set_max_hard_decline_retries(std::uint16_t value) { max_hard_decline_retries = std::move(value); }
    [[nodiscard]] const std::uint16_t& get_max_hard_decline_retries() const noexcept { return max_hard_decline_retries; }
    std::uint16_t max_retry_count{};
    std::uint16_t billing_connector_retry_threshold{};
    std::uint16_t max_hybrid_cascading_retry_count{};
    std::uint16_t grace_period_days{};
    bool enable_partial_authorization{};
    std::uint16_t max_hard_decline_retries{};
};

class RevenueRecoveryMetadata
{
public:
    void set_max_retry_count(std::uint16_t value) { max_retry_count = std::move(value); }
    [[nodiscard]] const std::uint16_t& get_max_retry_count() const noexcept { return max_retry_count; }
    void set_billing_connector_retry_threshold(std::uint16_t value) { billing_connector_retry_threshold = std::move(value); }
    [[nodiscard]] const std::uint16_t& get_billing_connector_retry_threshold() const noexcept { return billing_connector_retry_threshold; }
    void set_max_hybrid_cascading_retry_count(std::uint16_t value) { max_hybrid_cascading_retry_count = std::move(value); }
    [[nodiscard]] const std::uint16_t& get_max_hybrid_cascading_retry_count() const noexcept { return max_hybrid_cascading_retry_count; }
    void set_grace_period_days(std::uint16_t value) { grace_period_days = std::move(value); }
    [[nodiscard]] const std::uint16_t& get_grace_period_days() const noexcept { return grace_period_days; }
    void set_enable_partial_authorization(bool value) { enable_partial_authorization = std::move(value); }
    [[nodiscard]] const bool& get_enable_partial_authorization() const noexcept { return enable_partial_authorization; }
    void set_max_hard_decline_retries(std::uint16_t value) { max_hard_decline_retries = std::move(value); }
    [[nodiscard]] const std::uint16_t& get_max_hard_decline_retries() const noexcept { return max_hard_decline_retries; }
    void set_billing_account_reference(std::unordered_map<std::string, std::string> value) { billing_account_reference = std::move(value); }
    [[nodiscard]] const std::unordered_map<std::string, std::string>& get_billing_account_reference() const noexcept { return billing_account_reference; }
    std::uint16_t max_retry_count{};
    std::uint16_t billing_connector_retry_threshold{};
    std::uint16_t max_hybrid_cascading_retry_count{};
    std::uint16_t grace_period_days{};
    bool enable_partial_authorization{};
    std::uint16_t max_hard_decline_retries{};
    std::unordered_map<std::string, std::string> billing_account_reference{};
};

class SecretSerdeValueValue
{
public:
    void set_value(std::string v) { value = std::move(v); }
    [[nodiscard]] const std::string& get_value() const noexcept { return value; }
    std::string value{};
};

class ToggleAllKVRequest
{
public:
    void set_kv_enabled(bool value) { kv_enabled = std::move(value); }
    [[nodiscard]] const bool& get_kv_enabled() const noexcept { return kv_enabled; }
    bool kv_enabled{};
};

class ToggleAllKVResponse
{
public:
    void set_total_updated(std::size_t value) { total_updated = std::move(value); }
    [[nodiscard]] const std::size_t& get_total_updated() const noexcept { return total_updated; }
    void set_kv_enabled(bool value) { kv_enabled = std::move(value); }
    [[nodiscard]] const bool& get_kv_enabled() const noexcept { return kv_enabled; }
    std::size_t total_updated{};
    bool kv_enabled{};
};

class TransactionDetailsUiConfiguration
{
public:
    void set_position(std::optional<std::int8_t> value) { position = std::move(value); }
    [[nodiscard]] const std::optional<std::int8_t>& get_position() const noexcept { return position; }
    void set_is_key_bold(std::optional<bool> value) { is_key_bold = std::move(value); }
    [[nodiscard]] const std::optional<bool>& get_is_key_bold() const noexcept { return is_key_bold; }
    void set_is_value_bold(std::optional<bool> value) { is_value_bold = std::move(value); }
    [[nodiscard]] const std::optional<bool>& get_is_value_bold() const noexcept { return is_value_bold; }
    std::optional<std::int8_t> position{};
    std::optional<bool> is_key_bold{};
    std::optional<bool> is_value_bold{};
};

class TransferKeyResponse
{
public:
    void set_total_transferred(std::size_t value) { total_transferred = std::move(value); }
    [[nodiscard]] const std::size_t& get_total_transferred() const noexcept { return total_transferred; }
    std::size_t total_transferred{};
};

class TtlForExtendedCardInfo
{
public:
    void set_value(std::uint16_t v) { value = std::move(v); }
    [[nodiscard]] const std::uint16_t& get_value() const noexcept { return value; }
    std::uint16_t value{};
};

class UIWidgetFormLayoutValue
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

class VaultSdkValue
{
public:
    void set_value(std::string v) { value = std::move(v); }
    [[nodiscard]] const std::string& get_value() const noexcept { return value; }
    std::string value{};
};

class VaultTokenTypeValue
{
public:
    void set_value(std::string v) { value = std::move(v); }
    [[nodiscard]] const std::string& get_value() const noexcept { return value; }
    std::string value{};
};

class AuthenticationConnectorDetails
{
public:
    void set_authentication_connectors(std::vector<AuthenticationConnectorsValue> value) { authentication_connectors = std::move(value); }
    [[nodiscard]] const std::vector<AuthenticationConnectorsValue>& get_authentication_connectors() const noexcept { return authentication_connectors; }
    void set_three_ds_requestor_url(std::string value) { three_ds_requestor_url = std::move(value); }
    [[nodiscard]] const std::string& get_three_ds_requestor_url() const noexcept { return three_ds_requestor_url; }
    void set_three_ds_requestor_app_url(std::optional<std::string> value) { three_ds_requestor_app_url = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_three_ds_requestor_app_url() const noexcept { return three_ds_requestor_app_url; }
    std::vector<AuthenticationConnectorsValue> authentication_connectors{};
    std::string three_ds_requestor_url{};
    std::optional<std::string> three_ds_requestor_app_url{};
};

class BusinessGenericLinkConfig
{
public:
    void set_domain_name(std::optional<std::string> value) { domain_name = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_domain_name() const noexcept { return domain_name; }
    void set_allowed_domains(std::vector<std::string> value) { allowed_domains = std::move(value); }
    [[nodiscard]] const std::vector<std::string>& get_allowed_domains() const noexcept { return allowed_domains; }
    void set_ui_config(GenericLinkUiConfigValue value) { ui_config = std::move(value); }
    [[nodiscard]] const GenericLinkUiConfigValue& get_ui_config() const noexcept { return ui_config; }
    std::optional<std::string> domain_name{};
    std::vector<std::string> allowed_domains{};
    GenericLinkUiConfigValue ui_config{};
};

class CardBlockingConfig
{
public:
    void set_issuing_country(std::optional<std::vector<CountryAlpha2Value>> value) { issuing_country = std::move(value); }
    [[nodiscard]] const std::optional<std::vector<CountryAlpha2Value>>& get_issuing_country() const noexcept { return issuing_country; }
    void set_card_types(std::optional<std::vector<CardTypeValue>> value) { card_types = std::move(value); }
    [[nodiscard]] const std::optional<std::vector<CardTypeValue>>& get_card_types() const noexcept { return card_types; }
    void set_card_subtypes(std::optional<std::vector<std::string>> value) { card_subtypes = std::move(value); }
    [[nodiscard]] const std::optional<std::vector<std::string>>& get_card_subtypes() const noexcept { return card_subtypes; }
    void set_issuers(std::optional<std::vector<std::string>> value) { issuers = std::move(value); }
    [[nodiscard]] const std::optional<std::vector<std::string>>& get_issuers() const noexcept { return issuers; }
    void set_block_if_bin_info_unavailable(std::optional<bool> value) { block_if_bin_info_unavailable = std::move(value); }
    [[nodiscard]] const std::optional<bool>& get_block_if_bin_info_unavailable() const noexcept { return block_if_bin_info_unavailable; }
    void set_card_networks(std::optional<std::vector<CardNetworkValue>> value) { card_networks = std::move(value); }
    [[nodiscard]] const std::optional<std::vector<CardNetworkValue>>& get_card_networks() const noexcept { return card_networks; }
    void set_funding_sources(std::optional<std::vector<FundingSourceValue>> value) { funding_sources = std::move(value); }
    [[nodiscard]] const std::optional<std::vector<FundingSourceValue>>& get_funding_sources() const noexcept { return funding_sources; }
    void set_card_segment_types(std::optional<std::vector<CardSegmentTypeValue>> value) { card_segment_types = std::move(value); }
    [[nodiscard]] const std::optional<std::vector<CardSegmentTypeValue>>& get_card_segment_types() const noexcept { return card_segment_types; }
    void set_block_virtual_cards(std::optional<bool> value) { block_virtual_cards = std::move(value); }
    [[nodiscard]] const std::optional<bool>& get_block_virtual_cards() const noexcept { return block_virtual_cards; }
    void set_block_non_reloadable_prepaid_cards(std::optional<bool> value) { block_non_reloadable_prepaid_cards = std::move(value); }
    [[nodiscard]] const std::optional<bool>& get_block_non_reloadable_prepaid_cards() const noexcept { return block_non_reloadable_prepaid_cards; }
    void set_gambling_blocked(std::optional<bool> value) { gambling_blocked = std::move(value); }
    [[nodiscard]] const std::optional<bool>& get_gambling_blocked() const noexcept { return gambling_blocked; }
    std::optional<std::vector<CountryAlpha2Value>> issuing_country{};
    std::optional<std::vector<CardTypeValue>> card_types{};
    std::optional<std::vector<std::string>> card_subtypes{};
    std::optional<std::vector<std::string>> issuers{};
    std::optional<bool> block_if_bin_info_unavailable{};
    std::optional<std::vector<CardNetworkValue>> card_networks{};
    std::optional<std::vector<FundingSourceValue>> funding_sources{};
    std::optional<std::vector<CardSegmentTypeValue>> card_segment_types{};
    std::optional<bool> block_virtual_cards{};
    std::optional<bool> block_non_reloadable_prepaid_cards{};
    std::optional<bool> gambling_blocked{};
};

class CardTestingGuardConfig
{
public:
    void set_card_ip_blocking_status(CardTestingGuardStatus value) { card_ip_blocking_status = std::move(value); }
    [[nodiscard]] const CardTestingGuardStatus& get_card_ip_blocking_status() const noexcept { return card_ip_blocking_status; }
    void set_card_ip_blocking_threshold(std::int32_t value) { card_ip_blocking_threshold = std::move(value); }
    [[nodiscard]] const std::int32_t& get_card_ip_blocking_threshold() const noexcept { return card_ip_blocking_threshold; }
    void set_guest_user_card_blocking_status(CardTestingGuardStatus value) { guest_user_card_blocking_status = std::move(value); }
    [[nodiscard]] const CardTestingGuardStatus& get_guest_user_card_blocking_status() const noexcept { return guest_user_card_blocking_status; }
    void set_guest_user_card_blocking_threshold(std::int32_t value) { guest_user_card_blocking_threshold = std::move(value); }
    [[nodiscard]] const std::int32_t& get_guest_user_card_blocking_threshold() const noexcept { return guest_user_card_blocking_threshold; }
    void set_customer_id_blocking_status(CardTestingGuardStatus value) { customer_id_blocking_status = std::move(value); }
    [[nodiscard]] const CardTestingGuardStatus& get_customer_id_blocking_status() const noexcept { return customer_id_blocking_status; }
    void set_customer_id_blocking_threshold(std::int32_t value) { customer_id_blocking_threshold = std::move(value); }
    [[nodiscard]] const std::int32_t& get_customer_id_blocking_threshold() const noexcept { return customer_id_blocking_threshold; }
    void set_card_testing_guard_expiry(std::int32_t value) { card_testing_guard_expiry = std::move(value); }
    [[nodiscard]] const std::int32_t& get_card_testing_guard_expiry() const noexcept { return card_testing_guard_expiry; }
    void set_guest_ip_blocking_status(std::optional<CardTestingGuardStatus> value) { guest_ip_blocking_status = std::move(value); }
    [[nodiscard]] const std::optional<CardTestingGuardStatus>& get_guest_ip_blocking_status() const noexcept { return guest_ip_blocking_status; }
    void set_guest_ip_blocking_threshold(std::optional<std::int32_t> value) { guest_ip_blocking_threshold = std::move(value); }
    [[nodiscard]] const std::optional<std::int32_t>& get_guest_ip_blocking_threshold() const noexcept { return guest_ip_blocking_threshold; }
    CardTestingGuardStatus card_ip_blocking_status{};
    std::int32_t card_ip_blocking_threshold{};
    CardTestingGuardStatus guest_user_card_blocking_status{};
    std::int32_t guest_user_card_blocking_threshold{};
    CardTestingGuardStatus customer_id_blocking_status{};
    std::int32_t customer_id_blocking_threshold{};
    std::int32_t card_testing_guard_expiry{};
    std::optional<CardTestingGuardStatus> guest_ip_blocking_status{};
    std::optional<std::int32_t> guest_ip_blocking_threshold{};
};

class ConnectorWalletDetails
{
public:
    void set_apple_pay_combined(std::optional<SecretSerdeValueValue> value) { apple_pay_combined = std::move(value); }
    [[nodiscard]] const std::optional<SecretSerdeValueValue>& get_apple_pay_combined() const noexcept { return apple_pay_combined; }
    void set_apple_pay(std::optional<SecretSerdeValueValue> value) { apple_pay = std::move(value); }
    [[nodiscard]] const std::optional<SecretSerdeValueValue>& get_apple_pay() const noexcept { return apple_pay; }
    void set_amazon_pay(std::optional<SecretSerdeValueValue> value) { amazon_pay = std::move(value); }
    [[nodiscard]] const std::optional<SecretSerdeValueValue>& get_amazon_pay() const noexcept { return amazon_pay; }
    void set_samsung_pay(std::optional<SecretSerdeValueValue> value) { samsung_pay = std::move(value); }
    [[nodiscard]] const std::optional<SecretSerdeValueValue>& get_samsung_pay() const noexcept { return samsung_pay; }
    void set_paze(std::optional<SecretSerdeValueValue> value) { paze = std::move(value); }
    [[nodiscard]] const std::optional<SecretSerdeValueValue>& get_paze() const noexcept { return paze; }
    void set_google_pay(std::optional<SecretSerdeValueValue> value) { google_pay = std::move(value); }
    [[nodiscard]] const std::optional<SecretSerdeValueValue>& get_google_pay() const noexcept { return google_pay; }
    std::optional<SecretSerdeValueValue> apple_pay_combined{};
    std::optional<SecretSerdeValueValue> apple_pay{};
    std::optional<SecretSerdeValueValue> amazon_pay{};
    std::optional<SecretSerdeValueValue> samsung_pay{};
    std::optional<SecretSerdeValueValue> paze{};
    std::optional<SecretSerdeValueValue> google_pay{};
};

class ExtendedCardInfoConfig
{
public:
    void set_public_key(std::string value) { public_key = std::move(value); }
    [[nodiscard]] const std::string& get_public_key() const noexcept { return public_key; }
    void set_ttl_in_secs(TtlForExtendedCardInfo value) { ttl_in_secs = std::move(value); }
    [[nodiscard]] const TtlForExtendedCardInfo& get_ttl_in_secs() const noexcept { return ttl_in_secs; }
    std::string public_key{};
    TtlForExtendedCardInfo ttl_in_secs{};
};

class FrmPaymentMethodType
{
public:
    void set_payment_method_type(std::optional<PaymentMethodTypeValue> value) { payment_method_type = std::move(value); }
    [[nodiscard]] const std::optional<PaymentMethodTypeValue>& get_payment_method_type() const noexcept { return payment_method_type; }
    void set_card_networks(std::optional<std::vector<CardNetworkValue>> value) { card_networks = std::move(value); }
    [[nodiscard]] const std::optional<std::vector<CardNetworkValue>>& get_card_networks() const noexcept { return card_networks; }
    void set_flow(FrmPreferredFlowTypesValue value) { flow = std::move(value); }
    [[nodiscard]] const FrmPreferredFlowTypesValue& get_flow() const noexcept { return flow; }
    void set_action(FrmActionValue value) { action = std::move(value); }
    [[nodiscard]] const FrmActionValue& get_action() const noexcept { return action; }
    std::optional<PaymentMethodTypeValue> payment_method_type{};
    std::optional<std::vector<CardNetworkValue>> card_networks{};
    FrmPreferredFlowTypesValue flow{};
    FrmActionValue action{};
};

class MCACGraphData
{
public:
    void set_connector_name(ConnectorValue value) { connector_name = std::move(value); }
    [[nodiscard]] const ConnectorValue& get_connector_name() const noexcept { return connector_name; }
    void set_payment_methods_enabled(std::optional<std::vector<PaymentMethodsEnabledValue>> value) { payment_methods_enabled = std::move(value); }
    [[nodiscard]] const std::optional<std::vector<PaymentMethodsEnabledValue>>& get_payment_methods_enabled() const noexcept { return payment_methods_enabled; }
    ConnectorValue connector_name{};
    std::optional<std::vector<PaymentMethodsEnabledValue>> payment_methods_enabled{};
};

class MerchantAccountDeleteResponse
{
public:
    void set_merchant_id(MerchantIdValue value) { merchant_id = std::move(value); }
    [[nodiscard]] const MerchantIdValue& get_merchant_id() const noexcept { return merchant_id; }
    void set_deleted(bool value) { deleted = std::move(value); }
    [[nodiscard]] const bool& get_deleted() const noexcept { return deleted; }
    MerchantIdValue merchant_id{};
    bool deleted{};
};

class MerchantAccountListRequest
{
public:
    void set_organization_id(OrganizationIdValue value) { organization_id = std::move(value); }
    [[nodiscard]] const OrganizationIdValue& get_organization_id() const noexcept { return organization_id; }
    OrganizationIdValue organization_id{};
};

class MerchantAccountMetadata
{
public:
    void set_compatible_connector(std::optional<ConnectorValue> value) { compatible_connector = std::move(value); }
    [[nodiscard]] const std::optional<ConnectorValue>& get_compatible_connector() const noexcept { return compatible_connector; }
    void set_data(std::optional<SecretSerdeValueValue> value) { data = std::move(value); }
    [[nodiscard]] const std::optional<SecretSerdeValueValue>& get_data() const noexcept { return data; }
    std::optional<ConnectorValue> compatible_connector{};
    std::optional<SecretSerdeValueValue> data{};
};

class MerchantConnectorAccountFeatureMetadata
{
public:
    void set_revenue_recovery(std::optional<RevenueRecoveryMetadata> value) { revenue_recovery = std::move(value); }
    [[nodiscard]] const std::optional<RevenueRecoveryMetadata>& get_revenue_recovery() const noexcept { return revenue_recovery; }
    std::optional<RevenueRecoveryMetadata> revenue_recovery{};
};

class MerchantConnectorDeleteResponse
{
public:
    void set_merchant_id(MerchantIdValue value) { merchant_id = std::move(value); }
    [[nodiscard]] const MerchantIdValue& get_merchant_id() const noexcept { return merchant_id; }
    void set_merchant_connector_id(MerchantConnectorAccountIdValue value) { merchant_connector_id = std::move(value); }
    [[nodiscard]] const MerchantConnectorAccountIdValue& get_merchant_connector_id() const noexcept { return merchant_connector_id; }
    void set_deleted(bool value) { deleted = std::move(value); }
    [[nodiscard]] const bool& get_deleted() const noexcept { return deleted; }
    MerchantIdValue merchant_id{};
    MerchantConnectorAccountIdValue merchant_connector_id{};
    bool deleted{};
};

class MerchantConnectorDetails
{
public:
    void set_connector_account_details(SecretSerdeValueValue value) { connector_account_details = std::move(value); }
    [[nodiscard]] const SecretSerdeValueValue& get_connector_account_details() const noexcept { return connector_account_details; }
    void set_metadata(std::optional<SecretSerdeValueValue> value) { metadata = std::move(value); }
    [[nodiscard]] const std::optional<SecretSerdeValueValue>& get_metadata() const noexcept { return metadata; }
    SecretSerdeValueValue connector_account_details{};
    std::optional<SecretSerdeValueValue> metadata{};
};

class MerchantConnectorId
{
public:
    void set_merchant_id(MerchantIdValue value) { merchant_id = std::move(value); }
    [[nodiscard]] const MerchantIdValue& get_merchant_id() const noexcept { return merchant_id; }
    void set_merchant_connector_id(MerchantConnectorAccountIdValue value) { merchant_connector_id = std::move(value); }
    [[nodiscard]] const MerchantConnectorAccountIdValue& get_merchant_connector_id() const noexcept { return merchant_connector_id; }
    MerchantIdValue merchant_id{};
    MerchantConnectorAccountIdValue merchant_connector_id{};
};

class MerchantConnectorInfo
{
public:
    void set_connector_label(std::string value) { connector_label = std::move(value); }
    [[nodiscard]] const std::string& get_connector_label() const noexcept { return connector_label; }
    void set_merchant_connector_id(MerchantConnectorAccountIdValue value) { merchant_connector_id = std::move(value); }
    [[nodiscard]] const MerchantConnectorAccountIdValue& get_merchant_connector_id() const noexcept { return merchant_connector_id; }
    void set_connector_type(ConnectorTypeValue value) { connector_type = std::move(value); }
    [[nodiscard]] const ConnectorTypeValue& get_connector_type() const noexcept { return connector_type; }
    std::string connector_label{};
    MerchantConnectorAccountIdValue merchant_connector_id{};
    ConnectorTypeValue connector_type{};
};

class MerchantDetails
{
public:
    void set_primary_contact_person(std::optional<std::string> value) { primary_contact_person = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_primary_contact_person() const noexcept { return primary_contact_person; }
    void set_primary_phone(std::optional<std::string> value) { primary_phone = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_primary_phone() const noexcept { return primary_phone; }
    void set_primary_email(std::optional<EmailValue> value) { primary_email = std::move(value); }
    [[nodiscard]] const std::optional<EmailValue>& get_primary_email() const noexcept { return primary_email; }
    void set_secondary_contact_person(std::optional<std::string> value) { secondary_contact_person = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_secondary_contact_person() const noexcept { return secondary_contact_person; }
    void set_secondary_phone(std::optional<std::string> value) { secondary_phone = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_secondary_phone() const noexcept { return secondary_phone; }
    void set_secondary_email(std::optional<EmailValue> value) { secondary_email = std::move(value); }
    [[nodiscard]] const std::optional<EmailValue>& get_secondary_email() const noexcept { return secondary_email; }
    void set_website(std::optional<std::string> value) { website = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_website() const noexcept { return website; }
    void set_about_business(std::optional<std::string> value) { about_business = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_about_business() const noexcept { return about_business; }
    void set_address(std::optional<AddressDetailsValue> value) { address = std::move(value); }
    [[nodiscard]] const std::optional<AddressDetailsValue>& get_address() const noexcept { return address; }
    void set_merchant_tax_registration_id(std::optional<std::string> value) { merchant_tax_registration_id = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_merchant_tax_registration_id() const noexcept { return merchant_tax_registration_id; }
    void set_merchant_vat_registration_id(std::optional<std::string> value) { merchant_vat_registration_id = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_merchant_vat_registration_id() const noexcept { return merchant_vat_registration_id; }
    std::optional<std::string> primary_contact_person{};
    std::optional<std::string> primary_phone{};
    std::optional<EmailValue> primary_email{};
    std::optional<std::string> secondary_contact_person{};
    std::optional<std::string> secondary_phone{};
    std::optional<EmailValue> secondary_email{};
    std::optional<std::string> website{};
    std::optional<std::string> about_business{};
    std::optional<AddressDetailsValue> address{};
    std::optional<std::string> merchant_tax_registration_id{};
    std::optional<std::string> merchant_vat_registration_id{};
};

class MerchantId
{
public:
    void set_merchant_id(MerchantIdValue value) { merchant_id = std::move(value); }
    [[nodiscard]] const MerchantIdValue& get_merchant_id() const noexcept { return merchant_id; }
    MerchantIdValue merchant_id{};
};

class PaymentLinkBackgroundImageConfig
{
public:
    void set_url(UrlValue value) { url = std::move(value); }
    [[nodiscard]] const UrlValue& get_url() const noexcept { return url; }
    void set_position(std::optional<ElementPositionValue> value) { position = std::move(value); }
    [[nodiscard]] const std::optional<ElementPositionValue>& get_position() const noexcept { return position; }
    void set_size(std::optional<ElementSizeValue> value) { size = std::move(value); }
    [[nodiscard]] const std::optional<ElementSizeValue>& get_size() const noexcept { return size; }
    UrlValue url{};
    std::optional<ElementPositionValue> position{};
    std::optional<ElementSizeValue> size{};
};

class PaymentLinkTransactionDetails
{
public:
    void set_key(std::string value) { key = std::move(value); }
    [[nodiscard]] const std::string& get_key() const noexcept { return key; }
    void set_value(std::string v) { value = std::move(v); }
    [[nodiscard]] const std::string& get_value() const noexcept { return value; }
    void set_ui_configuration(std::optional<TransactionDetailsUiConfiguration> value) { ui_configuration = std::move(value); }
    [[nodiscard]] const std::optional<TransactionDetailsUiConfiguration>& get_ui_configuration() const noexcept { return ui_configuration; }
    std::string key{};
    std::string value{};
    std::optional<TransactionDetailsUiConfiguration> ui_configuration{};
};

class PaymentMethodsEnabled
{
public:
    void set_payment_method(PaymentMethodValue value) { payment_method = std::move(value); }
    [[nodiscard]] const PaymentMethodValue& get_payment_method() const noexcept { return payment_method; }
    void set_payment_method_types(std::optional<std::vector<RequestPaymentMethodTypesValue>> value) { payment_method_types = std::move(value); }
    [[nodiscard]] const std::optional<std::vector<RequestPaymentMethodTypesValue>>& get_payment_method_types() const noexcept { return payment_method_types; }
    PaymentMethodValue payment_method{};
    std::optional<std::vector<RequestPaymentMethodTypesValue>> payment_method_types{};
};

class PrimaryBusinessDetails
{
public:
    void set_country(CountryAlpha2Value value) { country = std::move(value); }
    [[nodiscard]] const CountryAlpha2Value& get_country() const noexcept { return country; }
    void set_business(std::string value) { business = std::move(value); }
    [[nodiscard]] const std::string& get_business() const noexcept { return business; }
    CountryAlpha2Value country{};
    std::string business{};
};

class SurchargeConnectorDetails
{
public:
    void set_surcharge_connector_id(std::optional<MerchantConnectorAccountIdValue> value) { surcharge_connector_id = std::move(value); }
    [[nodiscard]] const std::optional<MerchantConnectorAccountIdValue>& get_surcharge_connector_id() const noexcept { return surcharge_connector_id; }
    std::optional<MerchantConnectorAccountIdValue> surcharge_connector_id{};
};

class ToggleKVRequest
{
public:
    void set_merchant_id(MerchantIdValue value) { merchant_id = std::move(value); }
    [[nodiscard]] const MerchantIdValue& get_merchant_id() const noexcept { return merchant_id; }
    void set_kv_enabled(bool value) { kv_enabled = std::move(value); }
    [[nodiscard]] const bool& get_kv_enabled() const noexcept { return kv_enabled; }
    MerchantIdValue merchant_id{};
    bool kv_enabled{};
};

class ToggleKVResponse
{
public:
    void set_merchant_id(MerchantIdValue value) { merchant_id = std::move(value); }
    [[nodiscard]] const MerchantIdValue& get_merchant_id() const noexcept { return merchant_id; }
    void set_kv_enabled(bool value) { kv_enabled = std::move(value); }
    [[nodiscard]] const bool& get_kv_enabled() const noexcept { return kv_enabled; }
    MerchantIdValue merchant_id{};
    bool kv_enabled{};
};

class VaultTokenField
{
public:
    void set_token_type(VaultTokenTypeValue value) { token_type = std::move(value); }
    [[nodiscard]] const VaultTokenTypeValue& get_token_type() const noexcept { return token_type; }
    VaultTokenTypeValue token_type{};
};

class WebhookDetailsRequest
{
public:
    void set_webhook_version(std::optional<std::string> value) { webhook_version = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_webhook_version() const noexcept { return webhook_version; }
    void set_webhook_username(std::optional<std::string> value) { webhook_username = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_webhook_username() const noexcept { return webhook_username; }
    void set_webhook_password(std::optional<std::string> value) { webhook_password = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_webhook_password() const noexcept { return webhook_password; }
    void set_webhook_url(std::optional<std::string> value) { webhook_url = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_webhook_url() const noexcept { return webhook_url; }
    void set_payment_created_enabled(std::optional<bool> value) { payment_created_enabled = std::move(value); }
    [[nodiscard]] const std::optional<bool>& get_payment_created_enabled() const noexcept { return payment_created_enabled; }
    void set_payment_succeeded_enabled(std::optional<bool> value) { payment_succeeded_enabled = std::move(value); }
    [[nodiscard]] const std::optional<bool>& get_payment_succeeded_enabled() const noexcept { return payment_succeeded_enabled; }
    void set_payment_failed_enabled(std::optional<bool> value) { payment_failed_enabled = std::move(value); }
    [[nodiscard]] const std::optional<bool>& get_payment_failed_enabled() const noexcept { return payment_failed_enabled; }
    void set_payment_statuses_enabled(std::optional<std::vector<IntentStatusValue>> value) { payment_statuses_enabled = std::move(value); }
    [[nodiscard]] const std::optional<std::vector<IntentStatusValue>>& get_payment_statuses_enabled() const noexcept { return payment_statuses_enabled; }
    void set_refund_statuses_enabled(std::optional<std::vector<RefundStatusValue>> value) { refund_statuses_enabled = std::move(value); }
    [[nodiscard]] const std::optional<std::vector<RefundStatusValue>>& get_refund_statuses_enabled() const noexcept { return refund_statuses_enabled; }
    void set_payout_statuses_enabled(std::optional<std::vector<PayoutStatusValue>> value) { payout_statuses_enabled = std::move(value); }
    [[nodiscard]] const std::optional<std::vector<PayoutStatusValue>>& get_payout_statuses_enabled() const noexcept { return payout_statuses_enabled; }
    void set_dispute_statuses_enabled(std::optional<std::vector<DisputeStatusValue>> value) { dispute_statuses_enabled = std::move(value); }
    [[nodiscard]] const std::optional<std::vector<DisputeStatusValue>>& get_dispute_statuses_enabled() const noexcept { return dispute_statuses_enabled; }
    void set_mandate_statuses_enabled(std::optional<std::vector<MandateStatusValue>> value) { mandate_statuses_enabled = std::move(value); }
    [[nodiscard]] const std::optional<std::vector<MandateStatusValue>>& get_mandate_statuses_enabled() const noexcept { return mandate_statuses_enabled; }
    void set_invoice_statuses_enabled(std::optional<std::vector<InvoiceStatusValue>> value) { invoice_statuses_enabled = std::move(value); }
    [[nodiscard]] const std::optional<std::vector<InvoiceStatusValue>>& get_invoice_statuses_enabled() const noexcept { return invoice_statuses_enabled; }
    std::optional<std::string> webhook_version{};
    std::optional<std::string> webhook_username{};
    std::optional<std::string> webhook_password{};
    std::optional<std::string> webhook_url{};
    std::optional<bool> payment_created_enabled{};
    std::optional<bool> payment_succeeded_enabled{};
    std::optional<bool> payment_failed_enabled{};
    std::optional<std::vector<IntentStatusValue>> payment_statuses_enabled{};
    std::optional<std::vector<RefundStatusValue>> refund_statuses_enabled{};
    std::optional<std::vector<PayoutStatusValue>> payout_statuses_enabled{};
    std::optional<std::vector<DisputeStatusValue>> dispute_statuses_enabled{};
    std::optional<std::vector<MandateStatusValue>> mandate_statuses_enabled{};
    std::optional<std::vector<InvoiceStatusValue>> invoice_statuses_enabled{};
};

class WebhookDetailsResponse
{
public:
    void set_webhook_version(std::optional<std::string> value) { webhook_version = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_webhook_version() const noexcept { return webhook_version; }
    void set_webhook_username(std::optional<std::string> value) { webhook_username = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_webhook_username() const noexcept { return webhook_username; }
    void set_webhook_password(std::optional<std::string> value) { webhook_password = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_webhook_password() const noexcept { return webhook_password; }
    void set_webhook_url(std::optional<std::string> value) { webhook_url = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_webhook_url() const noexcept { return webhook_url; }
    void set_payment_created_enabled(std::optional<bool> value) { payment_created_enabled = std::move(value); }
    [[nodiscard]] const std::optional<bool>& get_payment_created_enabled() const noexcept { return payment_created_enabled; }
    void set_payment_succeeded_enabled(std::optional<bool> value) { payment_succeeded_enabled = std::move(value); }
    [[nodiscard]] const std::optional<bool>& get_payment_succeeded_enabled() const noexcept { return payment_succeeded_enabled; }
    void set_payment_failed_enabled(std::optional<bool> value) { payment_failed_enabled = std::move(value); }
    [[nodiscard]] const std::optional<bool>& get_payment_failed_enabled() const noexcept { return payment_failed_enabled; }
    void set_payment_statuses_enabled(std::optional<std::vector<IntentStatusValue>> value) { payment_statuses_enabled = std::move(value); }
    [[nodiscard]] const std::optional<std::vector<IntentStatusValue>>& get_payment_statuses_enabled() const noexcept { return payment_statuses_enabled; }
    void set_refund_statuses_enabled(std::optional<std::vector<RefundStatusValue>> value) { refund_statuses_enabled = std::move(value); }
    [[nodiscard]] const std::optional<std::vector<RefundStatusValue>>& get_refund_statuses_enabled() const noexcept { return refund_statuses_enabled; }
    void set_payout_statuses_enabled(std::optional<std::vector<PayoutStatusValue>> value) { payout_statuses_enabled = std::move(value); }
    [[nodiscard]] const std::optional<std::vector<PayoutStatusValue>>& get_payout_statuses_enabled() const noexcept { return payout_statuses_enabled; }
    void set_dispute_statuses_enabled(std::optional<std::vector<DisputeStatusValue>> value) { dispute_statuses_enabled = std::move(value); }
    [[nodiscard]] const std::optional<std::vector<DisputeStatusValue>>& get_dispute_statuses_enabled() const noexcept { return dispute_statuses_enabled; }
    void set_mandate_statuses_enabled(std::optional<std::vector<MandateStatusValue>> value) { mandate_statuses_enabled = std::move(value); }
    [[nodiscard]] const std::optional<std::vector<MandateStatusValue>>& get_mandate_statuses_enabled() const noexcept { return mandate_statuses_enabled; }
    void set_invoice_statuses_enabled(std::optional<std::vector<InvoiceStatusValue>> value) { invoice_statuses_enabled = std::move(value); }
    [[nodiscard]] const std::optional<std::vector<InvoiceStatusValue>>& get_invoice_statuses_enabled() const noexcept { return invoice_statuses_enabled; }
    std::optional<std::string> webhook_version{};
    std::optional<std::string> webhook_username{};
    std::optional<std::string> webhook_password{};
    std::optional<std::string> webhook_url{};
    std::optional<bool> payment_created_enabled{};
    std::optional<bool> payment_succeeded_enabled{};
    std::optional<bool> payment_failed_enabled{};
    std::optional<std::vector<IntentStatusValue>> payment_statuses_enabled{};
    std::optional<std::vector<RefundStatusValue>> refund_statuses_enabled{};
    std::optional<std::vector<PayoutStatusValue>> payout_statuses_enabled{};
    std::optional<std::vector<DisputeStatusValue>> dispute_statuses_enabled{};
    std::optional<std::vector<MandateStatusValue>> mandate_statuses_enabled{};
    std::optional<std::vector<InvoiceStatusValue>> invoice_statuses_enabled{};
};

class BusinessCollectLinkConfig
{
public:
    void set_config(BusinessGenericLinkConfig value) { config = std::move(value); }
    [[nodiscard]] const BusinessGenericLinkConfig& get_config() const noexcept { return config; }
    void set_enabled_payment_methods(std::vector<EnabledPaymentMethodValue> value) { enabled_payment_methods = std::move(value); }
    [[nodiscard]] const std::vector<EnabledPaymentMethodValue>& get_enabled_payment_methods() const noexcept { return enabled_payment_methods; }
    BusinessGenericLinkConfig config{};
    std::vector<EnabledPaymentMethodValue> enabled_payment_methods{};
};

class BusinessPayoutLinkConfig
{
public:
    void set_config(BusinessGenericLinkConfig value) { config = std::move(value); }
    [[nodiscard]] const BusinessGenericLinkConfig& get_config() const noexcept { return config; }
    void set_form_layout(std::optional<UIWidgetFormLayoutValue> value) { form_layout = std::move(value); }
    [[nodiscard]] const std::optional<UIWidgetFormLayoutValue>& get_form_layout() const noexcept { return form_layout; }
    void set_payout_test_mode(std::optional<bool> value) { payout_test_mode = std::move(value); }
    [[nodiscard]] const std::optional<bool>& get_payout_test_mode() const noexcept { return payout_test_mode; }
    BusinessGenericLinkConfig config{};
    std::optional<UIWidgetFormLayoutValue> form_layout{};
    std::optional<bool> payout_test_mode{};
};

class ExternalVaultConnectorDetails
{
public:
    void set_vault_connector_id(MerchantConnectorAccountIdValue value) { vault_connector_id = std::move(value); }
    [[nodiscard]] const MerchantConnectorAccountIdValue& get_vault_connector_id() const noexcept { return vault_connector_id; }
    void set_vault_sdk(std::optional<VaultSdkValue> value) { vault_sdk = std::move(value); }
    [[nodiscard]] const std::optional<VaultSdkValue>& get_vault_sdk() const noexcept { return vault_sdk; }
    void set_vault_token_selector(std::optional<std::vector<VaultTokenField>> value) { vault_token_selector = std::move(value); }
    [[nodiscard]] const std::optional<std::vector<VaultTokenField>>& get_vault_token_selector() const noexcept { return vault_token_selector; }
    MerchantConnectorAccountIdValue vault_connector_id{};
    std::optional<VaultSdkValue> vault_sdk{};
    std::optional<std::vector<VaultTokenField>> vault_token_selector{};
};

class FrmPaymentMethod
{
public:
    void set_payment_method(std::optional<PaymentMethodValue> value) { payment_method = std::move(value); }
    [[nodiscard]] const std::optional<PaymentMethodValue>& get_payment_method() const noexcept { return payment_method; }
    void set_payment_method_types(std::optional<std::vector<FrmPaymentMethodType>> value) { payment_method_types = std::move(value); }
    [[nodiscard]] const std::optional<std::vector<FrmPaymentMethodType>>& get_payment_method_types() const noexcept { return payment_method_types; }
    void set_flow(std::optional<FrmPreferredFlowTypesValue> value) { flow = std::move(value); }
    [[nodiscard]] const std::optional<FrmPreferredFlowTypesValue>& get_flow() const noexcept { return flow; }
    std::optional<PaymentMethodValue> payment_method{};
    std::optional<std::vector<FrmPaymentMethodType>> payment_method_types{};
    std::optional<FrmPreferredFlowTypesValue> flow{};
};

class MerchantAccountCreateWithoutOrgId
{
public:
    void set_merchant_name(std::string value) { merchant_name = std::move(value); }
    [[nodiscard]] const std::string& get_merchant_name() const noexcept { return merchant_name; }
    void set_merchant_details(std::optional<MerchantDetails> value) { merchant_details = std::move(value); }
    [[nodiscard]] const std::optional<MerchantDetails>& get_merchant_details() const noexcept { return merchant_details; }
    void set_metadata(std::optional<SecretSerdeValueValue> value) { metadata = std::move(value); }
    [[nodiscard]] const std::optional<SecretSerdeValueValue>& get_metadata() const noexcept { return metadata; }
    void set_product_type(std::optional<MerchantProductTypeValue> value) { product_type = std::move(value); }
    [[nodiscard]] const std::optional<MerchantProductTypeValue>& get_product_type() const noexcept { return product_type; }
    std::string merchant_name{};
    std::optional<MerchantDetails> merchant_details{};
    std::optional<SecretSerdeValueValue> metadata{};
    std::optional<MerchantProductTypeValue> product_type{};
};

class PaymentLinkConfig
{
public:
    void set_theme(std::string value) { theme = std::move(value); }
    [[nodiscard]] const std::string& get_theme() const noexcept { return theme; }
    void set_logo(std::string value) { logo = std::move(value); }
    [[nodiscard]] const std::string& get_logo() const noexcept { return logo; }
    void set_seller_name(std::string value) { seller_name = std::move(value); }
    [[nodiscard]] const std::string& get_seller_name() const noexcept { return seller_name; }
    void set_sdk_layout(std::string value) { sdk_layout = std::move(value); }
    [[nodiscard]] const std::string& get_sdk_layout() const noexcept { return sdk_layout; }
    void set_display_sdk_only(bool value) { display_sdk_only = std::move(value); }
    [[nodiscard]] const bool& get_display_sdk_only() const noexcept { return display_sdk_only; }
    void set_enabled_saved_payment_method(bool value) { enabled_saved_payment_method = std::move(value); }
    [[nodiscard]] const bool& get_enabled_saved_payment_method() const noexcept { return enabled_saved_payment_method; }
    void set_hide_card_nickname_field(bool value) { hide_card_nickname_field = std::move(value); }
    [[nodiscard]] const bool& get_hide_card_nickname_field() const noexcept { return hide_card_nickname_field; }
    void set_show_card_form_by_default(bool value) { show_card_form_by_default = std::move(value); }
    [[nodiscard]] const bool& get_show_card_form_by_default() const noexcept { return show_card_form_by_default; }
    void set_allowed_domains(std::optional<std::vector<std::string>> value) { allowed_domains = std::move(value); }
    [[nodiscard]] const std::optional<std::vector<std::string>>& get_allowed_domains() const noexcept { return allowed_domains; }
    void set_transaction_details(std::optional<std::vector<PaymentLinkTransactionDetails>> value) { transaction_details = std::move(value); }
    [[nodiscard]] const std::optional<std::vector<PaymentLinkTransactionDetails>>& get_transaction_details() const noexcept { return transaction_details; }
    void set_background_image(std::optional<PaymentLinkBackgroundImageConfig> value) { background_image = std::move(value); }
    [[nodiscard]] const std::optional<PaymentLinkBackgroundImageConfig>& get_background_image() const noexcept { return background_image; }
    void set_details_layout(std::optional<PaymentLinkDetailsLayoutValue> value) { details_layout = std::move(value); }
    [[nodiscard]] const std::optional<PaymentLinkDetailsLayoutValue>& get_details_layout() const noexcept { return details_layout; }
    void set_branding_visibility(std::optional<bool> value) { branding_visibility = std::move(value); }
    [[nodiscard]] const std::optional<bool>& get_branding_visibility() const noexcept { return branding_visibility; }
    void set_payment_button_text(std::optional<std::string> value) { payment_button_text = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_payment_button_text() const noexcept { return payment_button_text; }
    void set_custom_message_for_card_terms(std::optional<std::string> value) { custom_message_for_card_terms = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_custom_message_for_card_terms() const noexcept { return custom_message_for_card_terms; }
    void set_custom_message_for_payment_method_types(std::optional<PaymentMethodsConfigValue> value) { custom_message_for_payment_method_types = std::move(value); }
    [[nodiscard]] const std::optional<PaymentMethodsConfigValue>& get_custom_message_for_payment_method_types() const noexcept { return custom_message_for_payment_method_types; }
    void set_payment_button_colour(std::optional<std::string> value) { payment_button_colour = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_payment_button_colour() const noexcept { return payment_button_colour; }
    void set_skip_status_screen(std::optional<bool> value) { skip_status_screen = std::move(value); }
    [[nodiscard]] const std::optional<bool>& get_skip_status_screen() const noexcept { return skip_status_screen; }
    void set_payment_button_text_colour(std::optional<std::string> value) { payment_button_text_colour = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_payment_button_text_colour() const noexcept { return payment_button_text_colour; }
    void set_background_colour(std::optional<std::string> value) { background_colour = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_background_colour() const noexcept { return background_colour; }
    void set_sdk_ui_rules(std::optional<std::unordered_map<std::string, std::unordered_map<std::string, std::string>>> value) { sdk_ui_rules = std::move(value); }
    [[nodiscard]] const std::optional<std::unordered_map<std::string, std::unordered_map<std::string, std::string>>>& get_sdk_ui_rules() const noexcept { return sdk_ui_rules; }
    void set_payment_link_ui_rules(std::optional<std::unordered_map<std::string, std::unordered_map<std::string, std::string>>> value) { payment_link_ui_rules = std::move(value); }
    [[nodiscard]] const std::optional<std::unordered_map<std::string, std::unordered_map<std::string, std::string>>>& get_payment_link_ui_rules() const noexcept { return payment_link_ui_rules; }
    void set_enable_button_only_on_form_ready(bool value) { enable_button_only_on_form_ready = std::move(value); }
    [[nodiscard]] const bool& get_enable_button_only_on_form_ready() const noexcept { return enable_button_only_on_form_ready; }
    void set_payment_form_header_text(std::optional<std::string> value) { payment_form_header_text = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_payment_form_header_text() const noexcept { return payment_form_header_text; }
    void set_payment_form_label_type(std::optional<PaymentLinkSdkLabelTypeValue> value) { payment_form_label_type = std::move(value); }
    [[nodiscard]] const std::optional<PaymentLinkSdkLabelTypeValue>& get_payment_form_label_type() const noexcept { return payment_form_label_type; }
    void set_show_card_terms(std::optional<PaymentLinkShowSdkTermsValue> value) { show_card_terms = std::move(value); }
    [[nodiscard]] const std::optional<PaymentLinkShowSdkTermsValue>& get_show_card_terms() const noexcept { return show_card_terms; }
    void set_is_setup_mandate_flow(std::optional<bool> value) { is_setup_mandate_flow = std::move(value); }
    [[nodiscard]] const std::optional<bool>& get_is_setup_mandate_flow() const noexcept { return is_setup_mandate_flow; }
    void set_color_icon_card_cvc_error(std::optional<std::string> value) { color_icon_card_cvc_error = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_color_icon_card_cvc_error() const noexcept { return color_icon_card_cvc_error; }
    void set_show_merchant_name(std::optional<bool> value) { show_merchant_name = std::move(value); }
    [[nodiscard]] const std::optional<bool>& get_show_merchant_name() const noexcept { return show_merchant_name; }
    void set_payment_methods_separator_text(std::optional<std::string> value) { payment_methods_separator_text = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_payment_methods_separator_text() const noexcept { return payment_methods_separator_text; }
    void set_redirect_delay_seconds(std::optional<std::uint32_t> value) { redirect_delay_seconds = std::move(value); }
    [[nodiscard]] const std::optional<std::uint32_t>& get_redirect_delay_seconds() const noexcept { return redirect_delay_seconds; }
    std::string theme{};
    std::string logo{};
    std::string seller_name{};
    std::string sdk_layout{};
    bool display_sdk_only{};
    bool enabled_saved_payment_method{};
    bool hide_card_nickname_field{};
    bool show_card_form_by_default{};
    std::optional<std::vector<std::string>> allowed_domains{};
    std::optional<std::vector<PaymentLinkTransactionDetails>> transaction_details{};
    std::optional<PaymentLinkBackgroundImageConfig> background_image{};
    std::optional<PaymentLinkDetailsLayoutValue> details_layout{};
    std::optional<bool> branding_visibility{};
    std::optional<std::string> payment_button_text{};
    std::optional<std::string> custom_message_for_card_terms{};
    std::optional<PaymentMethodsConfigValue> custom_message_for_payment_method_types{};
    std::optional<std::string> payment_button_colour{};
    std::optional<bool> skip_status_screen{};
    std::optional<std::string> payment_button_text_colour{};
    std::optional<std::string> background_colour{};
    std::optional<std::unordered_map<std::string, std::unordered_map<std::string, std::string>>> sdk_ui_rules{};
    std::optional<std::unordered_map<std::string, std::unordered_map<std::string, std::string>>> payment_link_ui_rules{};
    bool enable_button_only_on_form_ready{};
    std::optional<std::string> payment_form_header_text{};
    std::optional<PaymentLinkSdkLabelTypeValue> payment_form_label_type{};
    std::optional<PaymentLinkShowSdkTermsValue> show_card_terms{};
    std::optional<bool> is_setup_mandate_flow{};
    std::optional<std::string> color_icon_card_cvc_error{};
    std::optional<bool> show_merchant_name{};
    std::optional<std::string> payment_methods_separator_text{};
    std::optional<std::uint32_t> redirect_delay_seconds{};
};

class PaymentLinkConfigRequest
{
public:
    void set_theme(std::optional<std::string> value) { theme = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_theme() const noexcept { return theme; }
    void set_logo(std::optional<std::string> value) { logo = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_logo() const noexcept { return logo; }
    void set_seller_name(std::optional<std::string> value) { seller_name = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_seller_name() const noexcept { return seller_name; }
    void set_sdk_layout(std::optional<std::string> value) { sdk_layout = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_sdk_layout() const noexcept { return sdk_layout; }
    void set_display_sdk_only(std::optional<bool> value) { display_sdk_only = std::move(value); }
    [[nodiscard]] const std::optional<bool>& get_display_sdk_only() const noexcept { return display_sdk_only; }
    void set_enabled_saved_payment_method(std::optional<bool> value) { enabled_saved_payment_method = std::move(value); }
    [[nodiscard]] const std::optional<bool>& get_enabled_saved_payment_method() const noexcept { return enabled_saved_payment_method; }
    void set_hide_card_nickname_field(std::optional<bool> value) { hide_card_nickname_field = std::move(value); }
    [[nodiscard]] const std::optional<bool>& get_hide_card_nickname_field() const noexcept { return hide_card_nickname_field; }
    void set_show_card_form_by_default(std::optional<bool> value) { show_card_form_by_default = std::move(value); }
    [[nodiscard]] const std::optional<bool>& get_show_card_form_by_default() const noexcept { return show_card_form_by_default; }
    void set_transaction_details(std::optional<std::vector<PaymentLinkTransactionDetails>> value) { transaction_details = std::move(value); }
    [[nodiscard]] const std::optional<std::vector<PaymentLinkTransactionDetails>>& get_transaction_details() const noexcept { return transaction_details; }
    void set_background_image(std::optional<PaymentLinkBackgroundImageConfig> value) { background_image = std::move(value); }
    [[nodiscard]] const std::optional<PaymentLinkBackgroundImageConfig>& get_background_image() const noexcept { return background_image; }
    void set_details_layout(std::optional<PaymentLinkDetailsLayoutValue> value) { details_layout = std::move(value); }
    [[nodiscard]] const std::optional<PaymentLinkDetailsLayoutValue>& get_details_layout() const noexcept { return details_layout; }
    void set_payment_button_text(std::optional<std::string> value) { payment_button_text = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_payment_button_text() const noexcept { return payment_button_text; }
    void set_custom_message_for_card_terms(std::optional<std::string> value) { custom_message_for_card_terms = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_custom_message_for_card_terms() const noexcept { return custom_message_for_card_terms; }
    void set_custom_message_for_payment_method_types(std::optional<PaymentMethodsConfigValue> value) { custom_message_for_payment_method_types = std::move(value); }
    [[nodiscard]] const std::optional<PaymentMethodsConfigValue>& get_custom_message_for_payment_method_types() const noexcept { return custom_message_for_payment_method_types; }
    void set_payment_button_colour(std::optional<std::string> value) { payment_button_colour = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_payment_button_colour() const noexcept { return payment_button_colour; }
    void set_skip_status_screen(std::optional<bool> value) { skip_status_screen = std::move(value); }
    [[nodiscard]] const std::optional<bool>& get_skip_status_screen() const noexcept { return skip_status_screen; }
    void set_payment_button_text_colour(std::optional<std::string> value) { payment_button_text_colour = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_payment_button_text_colour() const noexcept { return payment_button_text_colour; }
    void set_background_colour(std::optional<std::string> value) { background_colour = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_background_colour() const noexcept { return background_colour; }
    void set_sdk_ui_rules(std::optional<std::unordered_map<std::string, std::unordered_map<std::string, std::string>>> value) { sdk_ui_rules = std::move(value); }
    [[nodiscard]] const std::optional<std::unordered_map<std::string, std::unordered_map<std::string, std::string>>>& get_sdk_ui_rules() const noexcept { return sdk_ui_rules; }
    void set_payment_link_ui_rules(std::optional<std::unordered_map<std::string, std::unordered_map<std::string, std::string>>> value) { payment_link_ui_rules = std::move(value); }
    [[nodiscard]] const std::optional<std::unordered_map<std::string, std::unordered_map<std::string, std::string>>>& get_payment_link_ui_rules() const noexcept { return payment_link_ui_rules; }
    void set_enable_button_only_on_form_ready(std::optional<bool> value) { enable_button_only_on_form_ready = std::move(value); }
    [[nodiscard]] const std::optional<bool>& get_enable_button_only_on_form_ready() const noexcept { return enable_button_only_on_form_ready; }
    void set_payment_form_header_text(std::optional<std::string> value) { payment_form_header_text = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_payment_form_header_text() const noexcept { return payment_form_header_text; }
    void set_payment_form_label_type(std::optional<PaymentLinkSdkLabelTypeValue> value) { payment_form_label_type = std::move(value); }
    [[nodiscard]] const std::optional<PaymentLinkSdkLabelTypeValue>& get_payment_form_label_type() const noexcept { return payment_form_label_type; }
    void set_show_card_terms(std::optional<PaymentLinkShowSdkTermsValue> value) { show_card_terms = std::move(value); }
    [[nodiscard]] const std::optional<PaymentLinkShowSdkTermsValue>& get_show_card_terms() const noexcept { return show_card_terms; }
    void set_is_setup_mandate_flow(std::optional<bool> value) { is_setup_mandate_flow = std::move(value); }
    [[nodiscard]] const std::optional<bool>& get_is_setup_mandate_flow() const noexcept { return is_setup_mandate_flow; }
    void set_color_icon_card_cvc_error(std::optional<std::string> value) { color_icon_card_cvc_error = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_color_icon_card_cvc_error() const noexcept { return color_icon_card_cvc_error; }
    void set_show_merchant_name(std::optional<bool> value) { show_merchant_name = std::move(value); }
    [[nodiscard]] const std::optional<bool>& get_show_merchant_name() const noexcept { return show_merchant_name; }
    void set_payment_methods_separator_text(std::optional<std::string> value) { payment_methods_separator_text = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_payment_methods_separator_text() const noexcept { return payment_methods_separator_text; }
    void set_redirect_delay_seconds(std::optional<std::uint32_t> value) { redirect_delay_seconds = std::move(value); }
    [[nodiscard]] const std::optional<std::uint32_t>& get_redirect_delay_seconds() const noexcept { return redirect_delay_seconds; }
    std::optional<std::string> theme{};
    std::optional<std::string> logo{};
    std::optional<std::string> seller_name{};
    std::optional<std::string> sdk_layout{};
    std::optional<bool> display_sdk_only{};
    std::optional<bool> enabled_saved_payment_method{};
    std::optional<bool> hide_card_nickname_field{};
    std::optional<bool> show_card_form_by_default{};
    std::optional<std::vector<PaymentLinkTransactionDetails>> transaction_details{};
    std::optional<PaymentLinkBackgroundImageConfig> background_image{};
    std::optional<PaymentLinkDetailsLayoutValue> details_layout{};
    std::optional<std::string> payment_button_text{};
    std::optional<std::string> custom_message_for_card_terms{};
    std::optional<PaymentMethodsConfigValue> custom_message_for_payment_method_types{};
    std::optional<std::string> payment_button_colour{};
    std::optional<bool> skip_status_screen{};
    std::optional<std::string> payment_button_text_colour{};
    std::optional<std::string> background_colour{};
    std::optional<std::unordered_map<std::string, std::unordered_map<std::string, std::string>>> sdk_ui_rules{};
    std::optional<std::unordered_map<std::string, std::unordered_map<std::string, std::string>>> payment_link_ui_rules{};
    std::optional<bool> enable_button_only_on_form_ready{};
    std::optional<std::string> payment_form_header_text{};
    std::optional<PaymentLinkSdkLabelTypeValue> payment_form_label_type{};
    std::optional<PaymentLinkShowSdkTermsValue> show_card_terms{};
    std::optional<bool> is_setup_mandate_flow{};
    std::optional<std::string> color_icon_card_cvc_error{};
    std::optional<bool> show_merchant_name{};
    std::optional<std::string> payment_methods_separator_text{};
    std::optional<std::uint32_t> redirect_delay_seconds{};
};

class WalletBlockingConfig
{
public:
    void set_card_types(std::optional<std::vector<CardTypeValue>> value) { card_types = std::move(value); }
    [[nodiscard]] const std::optional<std::vector<CardTypeValue>>& get_card_types() const noexcept { return card_types; }
    void set_apple_pay(std::optional<CardBlockingConfig> value) { apple_pay = std::move(value); }
    [[nodiscard]] const std::optional<CardBlockingConfig>& get_apple_pay() const noexcept { return apple_pay; }
    void set_google_pay(std::optional<CardBlockingConfig> value) { google_pay = std::move(value); }
    [[nodiscard]] const std::optional<CardBlockingConfig>& get_google_pay() const noexcept { return google_pay; }
    std::optional<std::vector<CardTypeValue>> card_types{};
    std::optional<CardBlockingConfig> apple_pay{};
    std::optional<CardBlockingConfig> google_pay{};
};

class BusinessPaymentLinkConfig
{
public:
    void set_domain_name(std::optional<std::string> value) { domain_name = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_domain_name() const noexcept { return domain_name; }
    void set_default_config(std::optional<PaymentLinkConfigRequest> value) { default_config = std::move(value); }
    [[nodiscard]] const std::optional<PaymentLinkConfigRequest>& get_default_config() const noexcept { return default_config; }
    void set_business_specific_configs(std::optional<std::unordered_map<std::string, PaymentLinkConfigRequest>> value) { business_specific_configs = std::move(value); }
    [[nodiscard]] const std::optional<std::unordered_map<std::string, PaymentLinkConfigRequest>>& get_business_specific_configs() const noexcept { return business_specific_configs; }
    void set_allowed_domains(std::optional<std::vector<std::string>> value) { allowed_domains = std::move(value); }
    [[nodiscard]] const std::optional<std::vector<std::string>>& get_allowed_domains() const noexcept { return allowed_domains; }
    void set_branding_visibility(std::optional<bool> value) { branding_visibility = std::move(value); }
    [[nodiscard]] const std::optional<bool>& get_branding_visibility() const noexcept { return branding_visibility; }
    std::optional<std::string> domain_name{};
    std::optional<PaymentLinkConfigRequest> default_config{};
    std::optional<std::unordered_map<std::string, PaymentLinkConfigRequest>> business_specific_configs{};
    std::optional<std::vector<std::string>> allowed_domains{};
    std::optional<bool> branding_visibility{};
};

class FrmConfigs
{
public:
    void set_gateway(std::optional<ConnectorValue> value) { gateway = std::move(value); }
    [[nodiscard]] const std::optional<ConnectorValue>& get_gateway() const noexcept { return gateway; }
    void set_payment_methods(std::vector<FrmPaymentMethod> value) { payment_methods = std::move(value); }
    [[nodiscard]] const std::vector<FrmPaymentMethod>& get_payment_methods() const noexcept { return payment_methods; }
    std::optional<ConnectorValue> gateway{};
    std::vector<FrmPaymentMethod> payment_methods{};
};

class MerchantAccountCreate
{
public:
    void set_merchant_id(MerchantIdValue value) { merchant_id = std::move(value); }
    [[nodiscard]] const MerchantIdValue& get_merchant_id() const noexcept { return merchant_id; }
    void set_merchant_name(std::optional<std::string> value) { merchant_name = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_merchant_name() const noexcept { return merchant_name; }
    void set_merchant_details(std::optional<MerchantDetails> value) { merchant_details = std::move(value); }
    [[nodiscard]] const std::optional<MerchantDetails>& get_merchant_details() const noexcept { return merchant_details; }
    void set_return_url(std::optional<UrlValue> value) { return_url = std::move(value); }
    [[nodiscard]] const std::optional<UrlValue>& get_return_url() const noexcept { return return_url; }
    void set_webhook_details(std::optional<WebhookDetailsRequest> value) { webhook_details = std::move(value); }
    [[nodiscard]] const std::optional<WebhookDetailsRequest>& get_webhook_details() const noexcept { return webhook_details; }
    void set_routing_algorithm(std::optional<serde::Value> value) { routing_algorithm = std::move(value); }
    [[nodiscard]] const std::optional<serde::Value>& get_routing_algorithm() const noexcept { return routing_algorithm; }
    void set_payout_routing_algorithm(std::optional<serde::Value> value) { payout_routing_algorithm = std::move(value); }
    [[nodiscard]] const std::optional<serde::Value>& get_payout_routing_algorithm() const noexcept { return payout_routing_algorithm; }
    void set_sub_merchants_enabled(std::optional<bool> value) { sub_merchants_enabled = std::move(value); }
    [[nodiscard]] const std::optional<bool>& get_sub_merchants_enabled() const noexcept { return sub_merchants_enabled; }
    void set_parent_merchant_id(std::optional<MerchantIdValue> value) { parent_merchant_id = std::move(value); }
    [[nodiscard]] const std::optional<MerchantIdValue>& get_parent_merchant_id() const noexcept { return parent_merchant_id; }
    void set_enable_payment_response_hash(std::optional<bool> value) { enable_payment_response_hash = std::move(value); }
    [[nodiscard]] const std::optional<bool>& get_enable_payment_response_hash() const noexcept { return enable_payment_response_hash; }
    void set_payment_response_hash_key(std::optional<std::string> value) { payment_response_hash_key = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_payment_response_hash_key() const noexcept { return payment_response_hash_key; }
    void set_redirect_to_merchant_with_http_post(std::optional<bool> value) { redirect_to_merchant_with_http_post = std::move(value); }
    [[nodiscard]] const std::optional<bool>& get_redirect_to_merchant_with_http_post() const noexcept { return redirect_to_merchant_with_http_post; }
    void set_metadata(std::optional<MerchantAccountMetadata> value) { metadata = std::move(value); }
    [[nodiscard]] const std::optional<MerchantAccountMetadata>& get_metadata() const noexcept { return metadata; }
    void set_publishable_key(std::optional<std::string> value) { publishable_key = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_publishable_key() const noexcept { return publishable_key; }
    void set_locker_id(std::optional<std::string> value) { locker_id = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_locker_id() const noexcept { return locker_id; }
    void set_primary_business_details(std::optional<std::vector<PrimaryBusinessDetails>> value) { primary_business_details = std::move(value); }
    [[nodiscard]] const std::optional<std::vector<PrimaryBusinessDetails>>& get_primary_business_details() const noexcept { return primary_business_details; }
    void set_frm_routing_algorithm(std::optional<serde::Value> value) { frm_routing_algorithm = std::move(value); }
    [[nodiscard]] const std::optional<serde::Value>& get_frm_routing_algorithm() const noexcept { return frm_routing_algorithm; }
    void set_organization_id(std::optional<OrganizationIdValue> value) { organization_id = std::move(value); }
    [[nodiscard]] const std::optional<OrganizationIdValue>& get_organization_id() const noexcept { return organization_id; }
    void set_pm_collect_link_config(std::optional<BusinessCollectLinkConfig> value) { pm_collect_link_config = std::move(value); }
    [[nodiscard]] const std::optional<BusinessCollectLinkConfig>& get_pm_collect_link_config() const noexcept { return pm_collect_link_config; }
    void set_product_type(std::optional<MerchantProductTypeValue> value) { product_type = std::move(value); }
    [[nodiscard]] const std::optional<MerchantProductTypeValue>& get_product_type() const noexcept { return product_type; }
    void set_merchant_account_type(std::optional<MerchantAccountTypeValue> value) { merchant_account_type = std::move(value); }
    [[nodiscard]] const std::optional<MerchantAccountTypeValue>& get_merchant_account_type() const noexcept { return merchant_account_type; }
    void set_network_tokenization_credentials(std::optional<NetworkTokeizationProviderCredentials> value) { network_tokenization_credentials = std::move(value); }
    [[nodiscard]] const std::optional<NetworkTokeizationProviderCredentials>& get_network_tokenization_credentials() const noexcept { return network_tokenization_credentials; }
    MerchantIdValue merchant_id{};
    std::optional<std::string> merchant_name{};
    std::optional<MerchantDetails> merchant_details{};
    std::optional<UrlValue> return_url{};
    std::optional<WebhookDetailsRequest> webhook_details{};
    std::optional<serde::Value> routing_algorithm{};
    std::optional<serde::Value> payout_routing_algorithm{};
    std::optional<bool> sub_merchants_enabled{};
    std::optional<MerchantIdValue> parent_merchant_id{};
    std::optional<bool> enable_payment_response_hash{};
    std::optional<std::string> payment_response_hash_key{};
    std::optional<bool> redirect_to_merchant_with_http_post{};
    std::optional<MerchantAccountMetadata> metadata{};
    std::optional<std::string> publishable_key{};
    std::optional<std::string> locker_id{};
    std::optional<std::vector<PrimaryBusinessDetails>> primary_business_details{};
    std::optional<serde::Value> frm_routing_algorithm{};
    std::optional<OrganizationIdValue> organization_id{};
    std::optional<BusinessCollectLinkConfig> pm_collect_link_config{};
    std::optional<MerchantProductTypeValue> product_type{};
    std::optional<MerchantAccountTypeValue> merchant_account_type{};
    std::optional<NetworkTokeizationProviderCredentials> network_tokenization_credentials{};
};

class MerchantAccountResponse
{
public:
    void set_merchant_id(MerchantIdValue value) { merchant_id = std::move(value); }
    [[nodiscard]] const MerchantIdValue& get_merchant_id() const noexcept { return merchant_id; }
    void set_merchant_name(OptionalEncryptableNameValue value) { merchant_name = std::move(value); }
    [[nodiscard]] const OptionalEncryptableNameValue& get_merchant_name() const noexcept { return merchant_name; }
    void set_return_url(std::optional<std::string> value) { return_url = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_return_url() const noexcept { return return_url; }
    void set_enable_payment_response_hash(bool value) { enable_payment_response_hash = std::move(value); }
    [[nodiscard]] const bool& get_enable_payment_response_hash() const noexcept { return enable_payment_response_hash; }
    void set_payment_response_hash_key(std::optional<std::string> value) { payment_response_hash_key = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_payment_response_hash_key() const noexcept { return payment_response_hash_key; }
    void set_redirect_to_merchant_with_http_post(bool value) { redirect_to_merchant_with_http_post = std::move(value); }
    [[nodiscard]] const bool& get_redirect_to_merchant_with_http_post() const noexcept { return redirect_to_merchant_with_http_post; }
    void set_merchant_details(std::optional<std::string> value) { merchant_details = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_merchant_details() const noexcept { return merchant_details; }
    void set_webhook_details(std::optional<WebhookDetailsResponse> value) { webhook_details = std::move(value); }
    [[nodiscard]] const std::optional<WebhookDetailsResponse>& get_webhook_details() const noexcept { return webhook_details; }
    void set_routing_algorithm(std::optional<serde::Value> value) { routing_algorithm = std::move(value); }
    [[nodiscard]] const std::optional<serde::Value>& get_routing_algorithm() const noexcept { return routing_algorithm; }
    void set_payout_routing_algorithm(std::optional<serde::Value> value) { payout_routing_algorithm = std::move(value); }
    [[nodiscard]] const std::optional<serde::Value>& get_payout_routing_algorithm() const noexcept { return payout_routing_algorithm; }
    void set_sub_merchants_enabled(std::optional<bool> value) { sub_merchants_enabled = std::move(value); }
    [[nodiscard]] const std::optional<bool>& get_sub_merchants_enabled() const noexcept { return sub_merchants_enabled; }
    void set_parent_merchant_id(std::optional<MerchantIdValue> value) { parent_merchant_id = std::move(value); }
    [[nodiscard]] const std::optional<MerchantIdValue>& get_parent_merchant_id() const noexcept { return parent_merchant_id; }
    void set_publishable_key(std::optional<std::string> value) { publishable_key = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_publishable_key() const noexcept { return publishable_key; }
    void set_metadata(std::optional<SecretSerdeValueValue> value) { metadata = std::move(value); }
    [[nodiscard]] const std::optional<SecretSerdeValueValue>& get_metadata() const noexcept { return metadata; }
    void set_locker_id(std::optional<std::string> value) { locker_id = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_locker_id() const noexcept { return locker_id; }
    void set_primary_business_details(std::vector<PrimaryBusinessDetails> value) { primary_business_details = std::move(value); }
    [[nodiscard]] const std::vector<PrimaryBusinessDetails>& get_primary_business_details() const noexcept { return primary_business_details; }
    void set_frm_routing_algorithm(std::optional<serde::Value> value) { frm_routing_algorithm = std::move(value); }
    [[nodiscard]] const std::optional<serde::Value>& get_frm_routing_algorithm() const noexcept { return frm_routing_algorithm; }
    void set_organization_id(OrganizationIdValue value) { organization_id = std::move(value); }
    [[nodiscard]] const OrganizationIdValue& get_organization_id() const noexcept { return organization_id; }
    void set_is_recon_enabled(bool value) { is_recon_enabled = std::move(value); }
    [[nodiscard]] const bool& get_is_recon_enabled() const noexcept { return is_recon_enabled; }
    void set_default_profile(std::optional<ProfileIdValue> value) { default_profile = std::move(value); }
    [[nodiscard]] const std::optional<ProfileIdValue>& get_default_profile() const noexcept { return default_profile; }
    void set_recon_status(ReconStatusValue value) { recon_status = std::move(value); }
    [[nodiscard]] const ReconStatusValue& get_recon_status() const noexcept { return recon_status; }
    void set_pm_collect_link_config(std::optional<BusinessCollectLinkConfig> value) { pm_collect_link_config = std::move(value); }
    [[nodiscard]] const std::optional<BusinessCollectLinkConfig>& get_pm_collect_link_config() const noexcept { return pm_collect_link_config; }
    void set_product_type(std::optional<MerchantProductTypeValue> value) { product_type = std::move(value); }
    [[nodiscard]] const std::optional<MerchantProductTypeValue>& get_product_type() const noexcept { return product_type; }
    void set_merchant_account_type(MerchantAccountTypeValue value) { merchant_account_type = std::move(value); }
    [[nodiscard]] const MerchantAccountTypeValue& get_merchant_account_type() const noexcept { return merchant_account_type; }
    void set_network_tokenization_credentials(std::optional<NetworkTokeizationProviderCredentials> value) { network_tokenization_credentials = std::move(value); }
    [[nodiscard]] const std::optional<NetworkTokeizationProviderCredentials>& get_network_tokenization_credentials() const noexcept { return network_tokenization_credentials; }
    MerchantIdValue merchant_id{};
    OptionalEncryptableNameValue merchant_name{};
    std::optional<std::string> return_url{};
    bool enable_payment_response_hash{};
    std::optional<std::string> payment_response_hash_key{};
    bool redirect_to_merchant_with_http_post{};
    std::optional<std::string> merchant_details{};
    std::optional<WebhookDetailsResponse> webhook_details{};
    std::optional<serde::Value> routing_algorithm{};
    std::optional<serde::Value> payout_routing_algorithm{};
    std::optional<bool> sub_merchants_enabled{};
    std::optional<MerchantIdValue> parent_merchant_id{};
    std::optional<std::string> publishable_key{};
    std::optional<SecretSerdeValueValue> metadata{};
    std::optional<std::string> locker_id{};
    std::vector<PrimaryBusinessDetails> primary_business_details{};
    std::optional<serde::Value> frm_routing_algorithm{};
    OrganizationIdValue organization_id{};
    bool is_recon_enabled{};
    std::optional<ProfileIdValue> default_profile{};
    ReconStatusValue recon_status{};
    std::optional<BusinessCollectLinkConfig> pm_collect_link_config{};
    std::optional<MerchantProductTypeValue> product_type{};
    MerchantAccountTypeValue merchant_account_type{};
    std::optional<NetworkTokeizationProviderCredentials> network_tokenization_credentials{};
};

class MerchantAccountUpdate
{
public:
    void set_merchant_id(MerchantIdValue value) { merchant_id = std::move(value); }
    [[nodiscard]] const MerchantIdValue& get_merchant_id() const noexcept { return merchant_id; }
    void set_merchant_name(std::optional<std::string> value) { merchant_name = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_merchant_name() const noexcept { return merchant_name; }
    void set_merchant_details(std::optional<MerchantDetails> value) { merchant_details = std::move(value); }
    [[nodiscard]] const std::optional<MerchantDetails>& get_merchant_details() const noexcept { return merchant_details; }
    void set_return_url(std::optional<UrlValue> value) { return_url = std::move(value); }
    [[nodiscard]] const std::optional<UrlValue>& get_return_url() const noexcept { return return_url; }
    void set_webhook_details(std::optional<WebhookDetailsRequest> value) { webhook_details = std::move(value); }
    [[nodiscard]] const std::optional<WebhookDetailsRequest>& get_webhook_details() const noexcept { return webhook_details; }
    void set_routing_algorithm(std::optional<serde::Value> value) { routing_algorithm = std::move(value); }
    [[nodiscard]] const std::optional<serde::Value>& get_routing_algorithm() const noexcept { return routing_algorithm; }
    void set_payout_routing_algorithm(std::optional<serde::Value> value) { payout_routing_algorithm = std::move(value); }
    [[nodiscard]] const std::optional<serde::Value>& get_payout_routing_algorithm() const noexcept { return payout_routing_algorithm; }
    void set_sub_merchants_enabled(std::optional<bool> value) { sub_merchants_enabled = std::move(value); }
    [[nodiscard]] const std::optional<bool>& get_sub_merchants_enabled() const noexcept { return sub_merchants_enabled; }
    void set_parent_merchant_id(std::optional<MerchantIdValue> value) { parent_merchant_id = std::move(value); }
    [[nodiscard]] const std::optional<MerchantIdValue>& get_parent_merchant_id() const noexcept { return parent_merchant_id; }
    void set_enable_payment_response_hash(std::optional<bool> value) { enable_payment_response_hash = std::move(value); }
    [[nodiscard]] const std::optional<bool>& get_enable_payment_response_hash() const noexcept { return enable_payment_response_hash; }
    void set_payment_response_hash_key(std::optional<std::string> value) { payment_response_hash_key = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_payment_response_hash_key() const noexcept { return payment_response_hash_key; }
    void set_redirect_to_merchant_with_http_post(std::optional<bool> value) { redirect_to_merchant_with_http_post = std::move(value); }
    [[nodiscard]] const std::optional<bool>& get_redirect_to_merchant_with_http_post() const noexcept { return redirect_to_merchant_with_http_post; }
    void set_metadata(std::optional<SecretSerdeValueValue> value) { metadata = std::move(value); }
    [[nodiscard]] const std::optional<SecretSerdeValueValue>& get_metadata() const noexcept { return metadata; }
    void set_publishable_key(std::optional<std::string> value) { publishable_key = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_publishable_key() const noexcept { return publishable_key; }
    void set_locker_id(std::optional<std::string> value) { locker_id = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_locker_id() const noexcept { return locker_id; }
    void set_primary_business_details(std::optional<std::vector<PrimaryBusinessDetails>> value) { primary_business_details = std::move(value); }
    [[nodiscard]] const std::optional<std::vector<PrimaryBusinessDetails>>& get_primary_business_details() const noexcept { return primary_business_details; }
    void set_frm_routing_algorithm(std::optional<serde::Value> value) { frm_routing_algorithm = std::move(value); }
    [[nodiscard]] const std::optional<serde::Value>& get_frm_routing_algorithm() const noexcept { return frm_routing_algorithm; }
    void set_default_profile(std::optional<ProfileIdValue> value) { default_profile = std::move(value); }
    [[nodiscard]] const std::optional<ProfileIdValue>& get_default_profile() const noexcept { return default_profile; }
    void set_pm_collect_link_config(std::optional<BusinessCollectLinkConfig> value) { pm_collect_link_config = std::move(value); }
    [[nodiscard]] const std::optional<BusinessCollectLinkConfig>& get_pm_collect_link_config() const noexcept { return pm_collect_link_config; }
    void set_network_tokenization_credentials(std::optional<NetworkTokeizationProviderCredentials> value) { network_tokenization_credentials = std::move(value); }
    [[nodiscard]] const std::optional<NetworkTokeizationProviderCredentials>& get_network_tokenization_credentials() const noexcept { return network_tokenization_credentials; }
    void set_offer_engine_config(std::optional<OfferEngineMerchantConfig> value) { offer_engine_config = std::move(value); }
    [[nodiscard]] const std::optional<OfferEngineMerchantConfig>& get_offer_engine_config() const noexcept { return offer_engine_config; }
    MerchantIdValue merchant_id{};
    std::optional<std::string> merchant_name{};
    std::optional<MerchantDetails> merchant_details{};
    std::optional<UrlValue> return_url{};
    std::optional<WebhookDetailsRequest> webhook_details{};
    std::optional<serde::Value> routing_algorithm{};
    std::optional<serde::Value> payout_routing_algorithm{};
    std::optional<bool> sub_merchants_enabled{};
    std::optional<MerchantIdValue> parent_merchant_id{};
    std::optional<bool> enable_payment_response_hash{};
    std::optional<std::string> payment_response_hash_key{};
    std::optional<bool> redirect_to_merchant_with_http_post{};
    std::optional<SecretSerdeValueValue> metadata{};
    std::optional<std::string> publishable_key{};
    std::optional<std::string> locker_id{};
    std::optional<std::vector<PrimaryBusinessDetails>> primary_business_details{};
    std::optional<serde::Value> frm_routing_algorithm{};
    std::optional<ProfileIdValue> default_profile{};
    std::optional<BusinessCollectLinkConfig> pm_collect_link_config{};
    std::optional<NetworkTokeizationProviderCredentials> network_tokenization_credentials{};
    std::optional<OfferEngineMerchantConfig> offer_engine_config{};
};

class PaymentMethodBlockingConfig
{
public:
    void set_card(std::optional<CardBlockingConfig> value) { card = std::move(value); }
    [[nodiscard]] const std::optional<CardBlockingConfig>& get_card() const noexcept { return card; }
    void set_wallet(std::optional<WalletBlockingConfig> value) { wallet = std::move(value); }
    [[nodiscard]] const std::optional<WalletBlockingConfig>& get_wallet() const noexcept { return wallet; }
    std::optional<CardBlockingConfig> card{};
    std::optional<WalletBlockingConfig> wallet{};
};

class MerchantConnectorCreate
{
public:
    void set_connector_type(ConnectorTypeValue value) { connector_type = std::move(value); }
    [[nodiscard]] const ConnectorTypeValue& get_connector_type() const noexcept { return connector_type; }
    void set_connector_name(ConnectorValue value) { connector_name = std::move(value); }
    [[nodiscard]] const ConnectorValue& get_connector_name() const noexcept { return connector_name; }
    void set_connector_label(std::optional<std::string> value) { connector_label = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_connector_label() const noexcept { return connector_label; }
    void set_profile_id(ProfileIdValue value) { profile_id = std::move(value); }
    [[nodiscard]] const ProfileIdValue& get_profile_id() const noexcept { return profile_id; }
    void set_connector_account_details(std::optional<SecretSerdeValueValue> value) { connector_account_details = std::move(value); }
    [[nodiscard]] const std::optional<SecretSerdeValueValue>& get_connector_account_details() const noexcept { return connector_account_details; }
    void set_payment_methods_enabled(std::optional<std::vector<PaymentMethodsEnabledValue>> value) { payment_methods_enabled = std::move(value); }
    [[nodiscard]] const std::optional<std::vector<PaymentMethodsEnabledValue>>& get_payment_methods_enabled() const noexcept { return payment_methods_enabled; }
    void set_connector_webhook_details(std::optional<MerchantConnectorWebhookDetails> value) { connector_webhook_details = std::move(value); }
    [[nodiscard]] const std::optional<MerchantConnectorWebhookDetails>& get_connector_webhook_details() const noexcept { return connector_webhook_details; }
    void set_metadata(std::optional<SecretSerdeValueValue> value) { metadata = std::move(value); }
    [[nodiscard]] const std::optional<SecretSerdeValueValue>& get_metadata() const noexcept { return metadata; }
    void set_disabled(std::optional<bool> value) { disabled = std::move(value); }
    [[nodiscard]] const std::optional<bool>& get_disabled() const noexcept { return disabled; }
    void set_frm_configs(std::optional<std::vector<FrmConfigs>> value) { frm_configs = std::move(value); }
    [[nodiscard]] const std::optional<std::vector<FrmConfigs>>& get_frm_configs() const noexcept { return frm_configs; }
    void set_pm_auth_config(std::optional<SecretSerdeValueValue> value) { pm_auth_config = std::move(value); }
    [[nodiscard]] const std::optional<SecretSerdeValueValue>& get_pm_auth_config() const noexcept { return pm_auth_config; }
    void set_status(std::optional<ConnectorStatusValue> value) { status = std::move(value); }
    [[nodiscard]] const std::optional<ConnectorStatusValue>& get_status() const noexcept { return status; }
    void set_additional_merchant_data(std::optional<AdditionalMerchantData> value) { additional_merchant_data = std::move(value); }
    [[nodiscard]] const std::optional<AdditionalMerchantData>& get_additional_merchant_data() const noexcept { return additional_merchant_data; }
    void set_connector_wallets_details(std::optional<ConnectorWalletDetails> value) { connector_wallets_details = std::move(value); }
    [[nodiscard]] const std::optional<ConnectorWalletDetails>& get_connector_wallets_details() const noexcept { return connector_wallets_details; }
    void set_feature_metadata(std::optional<MerchantConnectorAccountFeatureMetadata> value) { feature_metadata = std::move(value); }
    [[nodiscard]] const std::optional<MerchantConnectorAccountFeatureMetadata>& get_feature_metadata() const noexcept { return feature_metadata; }
    ConnectorTypeValue connector_type{};
    ConnectorValue connector_name{};
    std::optional<std::string> connector_label{};
    ProfileIdValue profile_id{};
    std::optional<SecretSerdeValueValue> connector_account_details{};
    std::optional<std::vector<PaymentMethodsEnabledValue>> payment_methods_enabled{};
    std::optional<MerchantConnectorWebhookDetails> connector_webhook_details{};
    std::optional<SecretSerdeValueValue> metadata{};
    std::optional<bool> disabled{};
    std::optional<std::vector<FrmConfigs>> frm_configs{};
    std::optional<SecretSerdeValueValue> pm_auth_config{};
    std::optional<ConnectorStatusValue> status{};
    std::optional<AdditionalMerchantData> additional_merchant_data{};
    std::optional<ConnectorWalletDetails> connector_wallets_details{};
    std::optional<MerchantConnectorAccountFeatureMetadata> feature_metadata{};
};

class MerchantConnectorListResponse
{
public:
    void set_connector_type(ConnectorTypeValue value) { connector_type = std::move(value); }
    [[nodiscard]] const ConnectorTypeValue& get_connector_type() const noexcept { return connector_type; }
    void set_connector_name(std::string value) { connector_name = std::move(value); }
    [[nodiscard]] const std::string& get_connector_name() const noexcept { return connector_name; }
    void set_connector_label(std::optional<std::string> value) { connector_label = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_connector_label() const noexcept { return connector_label; }
    void set_merchant_connector_id(MerchantConnectorAccountIdValue value) { merchant_connector_id = std::move(value); }
    [[nodiscard]] const MerchantConnectorAccountIdValue& get_merchant_connector_id() const noexcept { return merchant_connector_id; }
    void set_profile_id(ProfileIdValue value) { profile_id = std::move(value); }
    [[nodiscard]] const ProfileIdValue& get_profile_id() const noexcept { return profile_id; }
    void set_payment_methods_enabled(std::optional<std::vector<PaymentMethodsEnabled>> value) { payment_methods_enabled = std::move(value); }
    [[nodiscard]] const std::optional<std::vector<PaymentMethodsEnabled>>& get_payment_methods_enabled() const noexcept { return payment_methods_enabled; }
    void set_test_mode(std::optional<bool> value) { test_mode = std::move(value); }
    [[nodiscard]] const std::optional<bool>& get_test_mode() const noexcept { return test_mode; }
    void set_disabled(std::optional<bool> value) { disabled = std::move(value); }
    [[nodiscard]] const std::optional<bool>& get_disabled() const noexcept { return disabled; }
    void set_frm_configs(std::optional<std::vector<FrmConfigs>> value) { frm_configs = std::move(value); }
    [[nodiscard]] const std::optional<std::vector<FrmConfigs>>& get_frm_configs() const noexcept { return frm_configs; }
    void set_business_country(std::optional<CountryAlpha2Value> value) { business_country = std::move(value); }
    [[nodiscard]] const std::optional<CountryAlpha2Value>& get_business_country() const noexcept { return business_country; }
    void set_business_label(std::optional<std::string> value) { business_label = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_business_label() const noexcept { return business_label; }
    void set_business_sub_label(std::optional<std::string> value) { business_sub_label = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_business_sub_label() const noexcept { return business_sub_label; }
    void set_applepay_verified_domains(std::optional<std::vector<std::string>> value) { applepay_verified_domains = std::move(value); }
    [[nodiscard]] const std::optional<std::vector<std::string>>& get_applepay_verified_domains() const noexcept { return applepay_verified_domains; }
    void set_pm_auth_config(std::optional<SecretSerdeValueValue> value) { pm_auth_config = std::move(value); }
    [[nodiscard]] const std::optional<SecretSerdeValueValue>& get_pm_auth_config() const noexcept { return pm_auth_config; }
    void set_status(ConnectorStatusValue value) { status = std::move(value); }
    [[nodiscard]] const ConnectorStatusValue& get_status() const noexcept { return status; }
    ConnectorTypeValue connector_type{};
    std::string connector_name{};
    std::optional<std::string> connector_label{};
    MerchantConnectorAccountIdValue merchant_connector_id{};
    ProfileIdValue profile_id{};
    std::optional<std::vector<PaymentMethodsEnabled>> payment_methods_enabled{};
    std::optional<bool> test_mode{};
    std::optional<bool> disabled{};
    std::optional<std::vector<FrmConfigs>> frm_configs{};
    std::optional<CountryAlpha2Value> business_country{};
    std::optional<std::string> business_label{};
    std::optional<std::string> business_sub_label{};
    std::optional<std::vector<std::string>> applepay_verified_domains{};
    std::optional<SecretSerdeValueValue> pm_auth_config{};
    ConnectorStatusValue status{};
};

class MerchantConnectorResponse
{
public:
    void set_connector_type(ConnectorTypeValue value) { connector_type = std::move(value); }
    [[nodiscard]] const ConnectorTypeValue& get_connector_type() const noexcept { return connector_type; }
    void set_connector_name(ConnectorValue value) { connector_name = std::move(value); }
    [[nodiscard]] const ConnectorValue& get_connector_name() const noexcept { return connector_name; }
    void set_connector_label(std::optional<std::string> value) { connector_label = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_connector_label() const noexcept { return connector_label; }
    void set_id(MerchantConnectorAccountIdValue value) { id = std::move(value); }
    [[nodiscard]] const MerchantConnectorAccountIdValue& get_id() const noexcept { return id; }
    void set_profile_id(ProfileIdValue value) { profile_id = std::move(value); }
    [[nodiscard]] const ProfileIdValue& get_profile_id() const noexcept { return profile_id; }
    void set_connector_account_details(SecretSerdeValueValue value) { connector_account_details = std::move(value); }
    [[nodiscard]] const SecretSerdeValueValue& get_connector_account_details() const noexcept { return connector_account_details; }
    void set_payment_methods_enabled(std::optional<std::vector<PaymentMethodsEnabledValue>> value) { payment_methods_enabled = std::move(value); }
    [[nodiscard]] const std::optional<std::vector<PaymentMethodsEnabledValue>>& get_payment_methods_enabled() const noexcept { return payment_methods_enabled; }
    void set_connector_webhook_details(std::optional<MerchantConnectorWebhookDetails> value) { connector_webhook_details = std::move(value); }
    [[nodiscard]] const std::optional<MerchantConnectorWebhookDetails>& get_connector_webhook_details() const noexcept { return connector_webhook_details; }
    void set_metadata(std::optional<SecretSerdeValueValue> value) { metadata = std::move(value); }
    [[nodiscard]] const std::optional<SecretSerdeValueValue>& get_metadata() const noexcept { return metadata; }
    void set_disabled(std::optional<bool> value) { disabled = std::move(value); }
    [[nodiscard]] const std::optional<bool>& get_disabled() const noexcept { return disabled; }
    void set_frm_configs(std::optional<std::vector<FrmConfigs>> value) { frm_configs = std::move(value); }
    [[nodiscard]] const std::optional<std::vector<FrmConfigs>>& get_frm_configs() const noexcept { return frm_configs; }
    void set_applepay_verified_domains(std::optional<std::vector<std::string>> value) { applepay_verified_domains = std::move(value); }
    [[nodiscard]] const std::optional<std::vector<std::string>>& get_applepay_verified_domains() const noexcept { return applepay_verified_domains; }
    void set_pm_auth_config(std::optional<SecretSerdeValueValue> value) { pm_auth_config = std::move(value); }
    [[nodiscard]] const std::optional<SecretSerdeValueValue>& get_pm_auth_config() const noexcept { return pm_auth_config; }
    void set_status(ConnectorStatusValue value) { status = std::move(value); }
    [[nodiscard]] const ConnectorStatusValue& get_status() const noexcept { return status; }
    void set_additional_merchant_data(std::optional<AdditionalMerchantData> value) { additional_merchant_data = std::move(value); }
    [[nodiscard]] const std::optional<AdditionalMerchantData>& get_additional_merchant_data() const noexcept { return additional_merchant_data; }
    void set_connector_wallets_details(std::optional<ConnectorWalletDetails> value) { connector_wallets_details = std::move(value); }
    [[nodiscard]] const std::optional<ConnectorWalletDetails>& get_connector_wallets_details() const noexcept { return connector_wallets_details; }
    void set_feature_metadata(std::optional<MerchantConnectorAccountFeatureMetadata> value) { feature_metadata = std::move(value); }
    [[nodiscard]] const std::optional<MerchantConnectorAccountFeatureMetadata>& get_feature_metadata() const noexcept { return feature_metadata; }
    ConnectorTypeValue connector_type{};
    ConnectorValue connector_name{};
    std::optional<std::string> connector_label{};
    MerchantConnectorAccountIdValue id{};
    ProfileIdValue profile_id{};
    SecretSerdeValueValue connector_account_details{};
    std::optional<std::vector<PaymentMethodsEnabledValue>> payment_methods_enabled{};
    std::optional<MerchantConnectorWebhookDetails> connector_webhook_details{};
    std::optional<SecretSerdeValueValue> metadata{};
    std::optional<bool> disabled{};
    std::optional<std::vector<FrmConfigs>> frm_configs{};
    std::optional<std::vector<std::string>> applepay_verified_domains{};
    std::optional<SecretSerdeValueValue> pm_auth_config{};
    ConnectorStatusValue status{};
    std::optional<AdditionalMerchantData> additional_merchant_data{};
    std::optional<ConnectorWalletDetails> connector_wallets_details{};
    std::optional<MerchantConnectorAccountFeatureMetadata> feature_metadata{};
};

class MerchantConnectorUpdate
{
public:
    void set_connector_type(ConnectorTypeValue value) { connector_type = std::move(value); }
    [[nodiscard]] const ConnectorTypeValue& get_connector_type() const noexcept { return connector_type; }
    void set_connector_label(std::optional<std::string> value) { connector_label = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_connector_label() const noexcept { return connector_label; }
    void set_connector_account_details(std::optional<SecretSerdeValueValue> value) { connector_account_details = std::move(value); }
    [[nodiscard]] const std::optional<SecretSerdeValueValue>& get_connector_account_details() const noexcept { return connector_account_details; }
    void set_payment_methods_enabled(std::optional<std::vector<PaymentMethodsEnabled>> value) { payment_methods_enabled = std::move(value); }
    [[nodiscard]] const std::optional<std::vector<PaymentMethodsEnabled>>& get_payment_methods_enabled() const noexcept { return payment_methods_enabled; }
    void set_connector_webhook_details(std::optional<MerchantConnectorWebhookDetails> value) { connector_webhook_details = std::move(value); }
    [[nodiscard]] const std::optional<MerchantConnectorWebhookDetails>& get_connector_webhook_details() const noexcept { return connector_webhook_details; }
    void set_metadata(std::optional<SecretSerdeValueValue> value) { metadata = std::move(value); }
    [[nodiscard]] const std::optional<SecretSerdeValueValue>& get_metadata() const noexcept { return metadata; }
    void set_test_mode(std::optional<bool> value) { test_mode = std::move(value); }
    [[nodiscard]] const std::optional<bool>& get_test_mode() const noexcept { return test_mode; }
    void set_disabled(std::optional<bool> value) { disabled = std::move(value); }
    [[nodiscard]] const std::optional<bool>& get_disabled() const noexcept { return disabled; }
    void set_frm_configs(std::optional<std::vector<FrmConfigs>> value) { frm_configs = std::move(value); }
    [[nodiscard]] const std::optional<std::vector<FrmConfigs>>& get_frm_configs() const noexcept { return frm_configs; }
    void set_pm_auth_config(std::optional<SecretSerdeValueValue> value) { pm_auth_config = std::move(value); }
    [[nodiscard]] const std::optional<SecretSerdeValueValue>& get_pm_auth_config() const noexcept { return pm_auth_config; }
    void set_status(std::optional<ConnectorStatusValue> value) { status = std::move(value); }
    [[nodiscard]] const std::optional<ConnectorStatusValue>& get_status() const noexcept { return status; }
    void set_additional_merchant_data(std::optional<AdditionalMerchantData> value) { additional_merchant_data = std::move(value); }
    [[nodiscard]] const std::optional<AdditionalMerchantData>& get_additional_merchant_data() const noexcept { return additional_merchant_data; }
    void set_connector_wallets_details(std::optional<ConnectorWalletDetails> value) { connector_wallets_details = std::move(value); }
    [[nodiscard]] const std::optional<ConnectorWalletDetails>& get_connector_wallets_details() const noexcept { return connector_wallets_details; }
    ConnectorTypeValue connector_type{};
    std::optional<std::string> connector_label{};
    std::optional<SecretSerdeValueValue> connector_account_details{};
    std::optional<std::vector<PaymentMethodsEnabled>> payment_methods_enabled{};
    std::optional<MerchantConnectorWebhookDetails> connector_webhook_details{};
    std::optional<SecretSerdeValueValue> metadata{};
    std::optional<bool> test_mode{};
    std::optional<bool> disabled{};
    std::optional<std::vector<FrmConfigs>> frm_configs{};
    std::optional<SecretSerdeValueValue> pm_auth_config{};
    std::optional<ConnectorStatusValue> status{};
    std::optional<AdditionalMerchantData> additional_merchant_data{};
    std::optional<ConnectorWalletDetails> connector_wallets_details{};
};

class ProfileCreate
{
public:
    void set_profile_name(std::optional<std::string> value) { profile_name = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_profile_name() const noexcept { return profile_name; }
    void set_return_url(std::optional<UrlValue> value) { return_url = std::move(value); }
    [[nodiscard]] const std::optional<UrlValue>& get_return_url() const noexcept { return return_url; }
    void set_enable_payment_response_hash(std::optional<bool> value) { enable_payment_response_hash = std::move(value); }
    [[nodiscard]] const std::optional<bool>& get_enable_payment_response_hash() const noexcept { return enable_payment_response_hash; }
    void set_payment_response_hash_key(std::optional<std::string> value) { payment_response_hash_key = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_payment_response_hash_key() const noexcept { return payment_response_hash_key; }
    void set_redirect_to_merchant_with_http_post(std::optional<bool> value) { redirect_to_merchant_with_http_post = std::move(value); }
    [[nodiscard]] const std::optional<bool>& get_redirect_to_merchant_with_http_post() const noexcept { return redirect_to_merchant_with_http_post; }
    void set_webhook_details(std::optional<WebhookDetailsRequest> value) { webhook_details = std::move(value); }
    [[nodiscard]] const std::optional<WebhookDetailsRequest>& get_webhook_details() const noexcept { return webhook_details; }
    void set_metadata(std::optional<SecretSerdeValueValue> value) { metadata = std::move(value); }
    [[nodiscard]] const std::optional<SecretSerdeValueValue>& get_metadata() const noexcept { return metadata; }
    void set_routing_algorithm(std::optional<serde::Value> value) { routing_algorithm = std::move(value); }
    [[nodiscard]] const std::optional<serde::Value>& get_routing_algorithm() const noexcept { return routing_algorithm; }
    void set_intent_fulfillment_time(std::optional<std::uint32_t> value) { intent_fulfillment_time = std::move(value); }
    [[nodiscard]] const std::optional<std::uint32_t>& get_intent_fulfillment_time() const noexcept { return intent_fulfillment_time; }
    void set_frm_routing_algorithm(std::optional<serde::Value> value) { frm_routing_algorithm = std::move(value); }
    [[nodiscard]] const std::optional<serde::Value>& get_frm_routing_algorithm() const noexcept { return frm_routing_algorithm; }
    void set_payout_routing_algorithm(std::optional<serde::Value> value) { payout_routing_algorithm = std::move(value); }
    [[nodiscard]] const std::optional<serde::Value>& get_payout_routing_algorithm() const noexcept { return payout_routing_algorithm; }
    void set_applepay_verified_domains(std::optional<std::vector<std::string>> value) { applepay_verified_domains = std::move(value); }
    [[nodiscard]] const std::optional<std::vector<std::string>>& get_applepay_verified_domains() const noexcept { return applepay_verified_domains; }
    void set_session_expiry(std::optional<std::uint32_t> value) { session_expiry = std::move(value); }
    [[nodiscard]] const std::optional<std::uint32_t>& get_session_expiry() const noexcept { return session_expiry; }
    void set_payment_link_config(std::optional<BusinessPaymentLinkConfig> value) { payment_link_config = std::move(value); }
    [[nodiscard]] const std::optional<BusinessPaymentLinkConfig>& get_payment_link_config() const noexcept { return payment_link_config; }
    void set_authentication_connector_details(std::optional<AuthenticationConnectorDetails> value) { authentication_connector_details = std::move(value); }
    [[nodiscard]] const std::optional<AuthenticationConnectorDetails>& get_authentication_connector_details() const noexcept { return authentication_connector_details; }
    void set_use_billing_as_payment_method_billing(std::optional<bool> value) { use_billing_as_payment_method_billing = std::move(value); }
    [[nodiscard]] const std::optional<bool>& get_use_billing_as_payment_method_billing() const noexcept { return use_billing_as_payment_method_billing; }
    void set_collect_shipping_details_from_wallet_connector(std::optional<bool> value) { collect_shipping_details_from_wallet_connector = std::move(value); }
    [[nodiscard]] const std::optional<bool>& get_collect_shipping_details_from_wallet_connector() const noexcept { return collect_shipping_details_from_wallet_connector; }
    void set_collect_billing_details_from_wallet_connector(std::optional<bool> value) { collect_billing_details_from_wallet_connector = std::move(value); }
    [[nodiscard]] const std::optional<bool>& get_collect_billing_details_from_wallet_connector() const noexcept { return collect_billing_details_from_wallet_connector; }
    void set_always_collect_shipping_details_from_wallet_connector(std::optional<bool> value) { always_collect_shipping_details_from_wallet_connector = std::move(value); }
    [[nodiscard]] const std::optional<bool>& get_always_collect_shipping_details_from_wallet_connector() const noexcept { return always_collect_shipping_details_from_wallet_connector; }
    void set_always_collect_billing_details_from_wallet_connector(std::optional<bool> value) { always_collect_billing_details_from_wallet_connector = std::move(value); }
    [[nodiscard]] const std::optional<bool>& get_always_collect_billing_details_from_wallet_connector() const noexcept { return always_collect_billing_details_from_wallet_connector; }
    void set_is_connector_agnostic_mit_enabled(std::optional<bool> value) { is_connector_agnostic_mit_enabled = std::move(value); }
    [[nodiscard]] const std::optional<bool>& get_is_connector_agnostic_mit_enabled() const noexcept { return is_connector_agnostic_mit_enabled; }
    void set_payout_link_config(std::optional<BusinessPayoutLinkConfig> value) { payout_link_config = std::move(value); }
    [[nodiscard]] const std::optional<BusinessPayoutLinkConfig>& get_payout_link_config() const noexcept { return payout_link_config; }
    void set_outgoing_webhook_custom_http_headers(std::optional<std::unordered_map<std::string, std::string>> value) { outgoing_webhook_custom_http_headers = std::move(value); }
    [[nodiscard]] const std::optional<std::unordered_map<std::string, std::string>>& get_outgoing_webhook_custom_http_headers() const noexcept { return outgoing_webhook_custom_http_headers; }
    void set_tax_connector_id(std::optional<MerchantConnectorAccountIdValue> value) { tax_connector_id = std::move(value); }
    [[nodiscard]] const std::optional<MerchantConnectorAccountIdValue>& get_tax_connector_id() const noexcept { return tax_connector_id; }
    void set_is_tax_connector_enabled(bool value) { is_tax_connector_enabled = std::move(value); }
    [[nodiscard]] const bool& get_is_tax_connector_enabled() const noexcept { return is_tax_connector_enabled; }
    void set_is_network_tokenization_enabled(bool value) { is_network_tokenization_enabled = std::move(value); }
    [[nodiscard]] const bool& get_is_network_tokenization_enabled() const noexcept { return is_network_tokenization_enabled; }
    void set_is_auto_retries_enabled(std::optional<bool> value) { is_auto_retries_enabled = std::move(value); }
    [[nodiscard]] const std::optional<bool>& get_is_auto_retries_enabled() const noexcept { return is_auto_retries_enabled; }
    void set_max_auto_retries_enabled(std::optional<std::uint8_t> value) { max_auto_retries_enabled = std::move(value); }
    [[nodiscard]] const std::optional<std::uint8_t>& get_max_auto_retries_enabled() const noexcept { return max_auto_retries_enabled; }
    void set_always_request_extended_authorization(std::optional<AlwaysRequestExtendedAuthorizationValue> value) { always_request_extended_authorization = std::move(value); }
    [[nodiscard]] const std::optional<AlwaysRequestExtendedAuthorizationValue>& get_always_request_extended_authorization() const noexcept { return always_request_extended_authorization; }
    void set_is_click_to_pay_enabled(bool value) { is_click_to_pay_enabled = std::move(value); }
    [[nodiscard]] const bool& get_is_click_to_pay_enabled() const noexcept { return is_click_to_pay_enabled; }
    void set_authentication_product_ids(std::optional<AuthenticationConnectorAccountMapValue> value) { authentication_product_ids = std::move(value); }
    [[nodiscard]] const std::optional<AuthenticationConnectorAccountMapValue>& get_authentication_product_ids() const noexcept { return authentication_product_ids; }
    void set_card_testing_guard_config(std::optional<CardTestingGuardConfig> value) { card_testing_guard_config = std::move(value); }
    [[nodiscard]] const std::optional<CardTestingGuardConfig>& get_card_testing_guard_config() const noexcept { return card_testing_guard_config; }
    void set_is_clear_pan_retries_enabled(std::optional<bool> value) { is_clear_pan_retries_enabled = std::move(value); }
    [[nodiscard]] const std::optional<bool>& get_is_clear_pan_retries_enabled() const noexcept { return is_clear_pan_retries_enabled; }
    void set_force_3ds_challenge(std::optional<bool> value) { force_3ds_challenge = std::move(value); }
    [[nodiscard]] const std::optional<bool>& get_force_3ds_challenge() const noexcept { return force_3ds_challenge; }
    void set_is_debit_routing_enabled(std::optional<bool> value) { is_debit_routing_enabled = std::move(value); }
    [[nodiscard]] const std::optional<bool>& get_is_debit_routing_enabled() const noexcept { return is_debit_routing_enabled; }
    void set_merchant_business_country(std::optional<CountryAlpha2Value> value) { merchant_business_country = std::move(value); }
    [[nodiscard]] const std::optional<CountryAlpha2Value>& get_merchant_business_country() const noexcept { return merchant_business_country; }
    void set_is_iframe_redirection_enabled(std::optional<bool> value) { is_iframe_redirection_enabled = std::move(value); }
    [[nodiscard]] const std::optional<bool>& get_is_iframe_redirection_enabled() const noexcept { return is_iframe_redirection_enabled; }
    void set_is_pre_network_tokenization_enabled(std::optional<bool> value) { is_pre_network_tokenization_enabled = std::move(value); }
    [[nodiscard]] const std::optional<bool>& get_is_pre_network_tokenization_enabled() const noexcept { return is_pre_network_tokenization_enabled; }
    void set_merchant_category_code(std::optional<MerchantCategoryCodeValue> value) { merchant_category_code = std::move(value); }
    [[nodiscard]] const std::optional<MerchantCategoryCodeValue>& get_merchant_category_code() const noexcept { return merchant_category_code; }
    void set_merchant_country_code(std::optional<MerchantCountryCodeValue> value) { merchant_country_code = std::move(value); }
    [[nodiscard]] const std::optional<MerchantCountryCodeValue>& get_merchant_country_code() const noexcept { return merchant_country_code; }
    void set_dispute_polling_interval(std::optional<DisputePollingIntervalInHoursValue> value) { dispute_polling_interval = std::move(value); }
    [[nodiscard]] const std::optional<DisputePollingIntervalInHoursValue>& get_dispute_polling_interval() const noexcept { return dispute_polling_interval; }
    void set_is_manual_retry_enabled(std::optional<bool> value) { is_manual_retry_enabled = std::move(value); }
    [[nodiscard]] const std::optional<bool>& get_is_manual_retry_enabled() const noexcept { return is_manual_retry_enabled; }
    void set_auto_fallback_capture_method(std::optional<bool> value) { auto_fallback_capture_method = std::move(value); }
    [[nodiscard]] const std::optional<bool>& get_auto_fallback_capture_method() const noexcept { return auto_fallback_capture_method; }
    void set_always_enable_overcapture(std::optional<AlwaysEnableOvercaptureBoolValue> value) { always_enable_overcapture = std::move(value); }
    [[nodiscard]] const std::optional<AlwaysEnableOvercaptureBoolValue>& get_always_enable_overcapture() const noexcept { return always_enable_overcapture; }
    void set_is_external_vault_enabled(std::optional<ExternalVaultEnabledValue> value) { is_external_vault_enabled = std::move(value); }
    [[nodiscard]] const std::optional<ExternalVaultEnabledValue>& get_is_external_vault_enabled() const noexcept { return is_external_vault_enabled; }
    void set_external_vault_connector_details(std::optional<ExternalVaultConnectorDetails> value) { external_vault_connector_details = std::move(value); }
    [[nodiscard]] const std::optional<ExternalVaultConnectorDetails>& get_external_vault_connector_details() const noexcept { return external_vault_connector_details; }
    void set_billing_processor_id(std::optional<MerchantConnectorAccountIdValue> value) { billing_processor_id = std::move(value); }
    [[nodiscard]] const std::optional<MerchantConnectorAccountIdValue>& get_billing_processor_id() const noexcept { return billing_processor_id; }
    void set_surcharge_connector_details(std::optional<SurchargeConnectorDetails> value) { surcharge_connector_details = std::move(value); }
    [[nodiscard]] const std::optional<SurchargeConnectorDetails>& get_surcharge_connector_details() const noexcept { return surcharge_connector_details; }
    void set_is_l2_l3_enabled(std::optional<bool> value) { is_l2_l3_enabled = std::move(value); }
    [[nodiscard]] const std::optional<bool>& get_is_l2_l3_enabled() const noexcept { return is_l2_l3_enabled; }
    void set_network_tokenization_credentials(std::optional<NetworkTokeizationProviderCredentials> value) { network_tokenization_credentials = std::move(value); }
    [[nodiscard]] const std::optional<NetworkTokeizationProviderCredentials>& get_network_tokenization_credentials() const noexcept { return network_tokenization_credentials; }
    void set_payment_method_blocking(std::optional<PaymentMethodBlockingConfig> value) { payment_method_blocking = std::move(value); }
    [[nodiscard]] const std::optional<PaymentMethodBlockingConfig>& get_payment_method_blocking() const noexcept { return payment_method_blocking; }
    std::optional<std::string> profile_name{};
    std::optional<UrlValue> return_url{};
    std::optional<bool> enable_payment_response_hash{};
    std::optional<std::string> payment_response_hash_key{};
    std::optional<bool> redirect_to_merchant_with_http_post{};
    std::optional<WebhookDetailsRequest> webhook_details{};
    std::optional<SecretSerdeValueValue> metadata{};
    std::optional<serde::Value> routing_algorithm{};
    std::optional<std::uint32_t> intent_fulfillment_time{};
    std::optional<serde::Value> frm_routing_algorithm{};
    std::optional<serde::Value> payout_routing_algorithm{};
    std::optional<std::vector<std::string>> applepay_verified_domains{};
    std::optional<std::uint32_t> session_expiry{};
    std::optional<BusinessPaymentLinkConfig> payment_link_config{};
    std::optional<AuthenticationConnectorDetails> authentication_connector_details{};
    std::optional<bool> use_billing_as_payment_method_billing{};
    std::optional<bool> collect_shipping_details_from_wallet_connector{};
    std::optional<bool> collect_billing_details_from_wallet_connector{};
    std::optional<bool> always_collect_shipping_details_from_wallet_connector{};
    std::optional<bool> always_collect_billing_details_from_wallet_connector{};
    std::optional<bool> is_connector_agnostic_mit_enabled{};
    std::optional<BusinessPayoutLinkConfig> payout_link_config{};
    std::optional<std::unordered_map<std::string, std::string>> outgoing_webhook_custom_http_headers{};
    std::optional<MerchantConnectorAccountIdValue> tax_connector_id{};
    bool is_tax_connector_enabled{};
    bool is_network_tokenization_enabled{};
    std::optional<bool> is_auto_retries_enabled{};
    std::optional<std::uint8_t> max_auto_retries_enabled{};
    std::optional<AlwaysRequestExtendedAuthorizationValue> always_request_extended_authorization{};
    bool is_click_to_pay_enabled{};
    std::optional<AuthenticationConnectorAccountMapValue> authentication_product_ids{};
    std::optional<CardTestingGuardConfig> card_testing_guard_config{};
    std::optional<bool> is_clear_pan_retries_enabled{};
    std::optional<bool> force_3ds_challenge{};
    std::optional<bool> is_debit_routing_enabled{};
    std::optional<CountryAlpha2Value> merchant_business_country{};
    std::optional<bool> is_iframe_redirection_enabled{};
    std::optional<bool> is_pre_network_tokenization_enabled{};
    std::optional<MerchantCategoryCodeValue> merchant_category_code{};
    std::optional<MerchantCountryCodeValue> merchant_country_code{};
    std::optional<DisputePollingIntervalInHoursValue> dispute_polling_interval{};
    std::optional<bool> is_manual_retry_enabled{};
    std::optional<bool> auto_fallback_capture_method{};
    std::optional<AlwaysEnableOvercaptureBoolValue> always_enable_overcapture{};
    std::optional<ExternalVaultEnabledValue> is_external_vault_enabled{};
    std::optional<ExternalVaultConnectorDetails> external_vault_connector_details{};
    std::optional<MerchantConnectorAccountIdValue> billing_processor_id{};
    std::optional<SurchargeConnectorDetails> surcharge_connector_details{};
    std::optional<bool> is_l2_l3_enabled{};
    std::optional<NetworkTokeizationProviderCredentials> network_tokenization_credentials{};
    std::optional<PaymentMethodBlockingConfig> payment_method_blocking{};
};

class ProfileResponse
{
public:
    void set_merchant_id(MerchantIdValue value) { merchant_id = std::move(value); }
    [[nodiscard]] const MerchantIdValue& get_merchant_id() const noexcept { return merchant_id; }
    void set_profile_id(ProfileIdValue value) { profile_id = std::move(value); }
    [[nodiscard]] const ProfileIdValue& get_profile_id() const noexcept { return profile_id; }
    void set_profile_name(std::string value) { profile_name = std::move(value); }
    [[nodiscard]] const std::string& get_profile_name() const noexcept { return profile_name; }
    void set_return_url(std::optional<std::string> value) { return_url = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_return_url() const noexcept { return return_url; }
    void set_enable_payment_response_hash(bool value) { enable_payment_response_hash = std::move(value); }
    [[nodiscard]] const bool& get_enable_payment_response_hash() const noexcept { return enable_payment_response_hash; }
    void set_payment_response_hash_key(std::optional<std::string> value) { payment_response_hash_key = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_payment_response_hash_key() const noexcept { return payment_response_hash_key; }
    void set_redirect_to_merchant_with_http_post(bool value) { redirect_to_merchant_with_http_post = std::move(value); }
    [[nodiscard]] const bool& get_redirect_to_merchant_with_http_post() const noexcept { return redirect_to_merchant_with_http_post; }
    void set_webhook_details(std::optional<WebhookDetailsResponse> value) { webhook_details = std::move(value); }
    [[nodiscard]] const std::optional<WebhookDetailsResponse>& get_webhook_details() const noexcept { return webhook_details; }
    void set_metadata(std::optional<SecretSerdeValueValue> value) { metadata = std::move(value); }
    [[nodiscard]] const std::optional<SecretSerdeValueValue>& get_metadata() const noexcept { return metadata; }
    void set_routing_algorithm(std::optional<serde::Value> value) { routing_algorithm = std::move(value); }
    [[nodiscard]] const std::optional<serde::Value>& get_routing_algorithm() const noexcept { return routing_algorithm; }
    void set_intent_fulfillment_time(std::optional<std::int64_t> value) { intent_fulfillment_time = std::move(value); }
    [[nodiscard]] const std::optional<std::int64_t>& get_intent_fulfillment_time() const noexcept { return intent_fulfillment_time; }
    void set_frm_routing_algorithm(std::optional<serde::Value> value) { frm_routing_algorithm = std::move(value); }
    [[nodiscard]] const std::optional<serde::Value>& get_frm_routing_algorithm() const noexcept { return frm_routing_algorithm; }
    void set_payout_routing_algorithm(std::optional<serde::Value> value) { payout_routing_algorithm = std::move(value); }
    [[nodiscard]] const std::optional<serde::Value>& get_payout_routing_algorithm() const noexcept { return payout_routing_algorithm; }
    void set_applepay_verified_domains(std::optional<std::vector<std::string>> value) { applepay_verified_domains = std::move(value); }
    [[nodiscard]] const std::optional<std::vector<std::string>>& get_applepay_verified_domains() const noexcept { return applepay_verified_domains; }
    void set_session_expiry(std::optional<std::int64_t> value) { session_expiry = std::move(value); }
    [[nodiscard]] const std::optional<std::int64_t>& get_session_expiry() const noexcept { return session_expiry; }
    void set_payment_link_config(std::optional<BusinessPaymentLinkConfig> value) { payment_link_config = std::move(value); }
    [[nodiscard]] const std::optional<BusinessPaymentLinkConfig>& get_payment_link_config() const noexcept { return payment_link_config; }
    void set_authentication_connector_details(std::optional<AuthenticationConnectorDetails> value) { authentication_connector_details = std::move(value); }
    [[nodiscard]] const std::optional<AuthenticationConnectorDetails>& get_authentication_connector_details() const noexcept { return authentication_connector_details; }
    void set_use_billing_as_payment_method_billing(std::optional<bool> value) { use_billing_as_payment_method_billing = std::move(value); }
    [[nodiscard]] const std::optional<bool>& get_use_billing_as_payment_method_billing() const noexcept { return use_billing_as_payment_method_billing; }
    void set_extended_card_info_config(std::optional<ExtendedCardInfoConfig> value) { extended_card_info_config = std::move(value); }
    [[nodiscard]] const std::optional<ExtendedCardInfoConfig>& get_extended_card_info_config() const noexcept { return extended_card_info_config; }
    void set_collect_shipping_details_from_wallet_connector(std::optional<bool> value) { collect_shipping_details_from_wallet_connector = std::move(value); }
    [[nodiscard]] const std::optional<bool>& get_collect_shipping_details_from_wallet_connector() const noexcept { return collect_shipping_details_from_wallet_connector; }
    void set_collect_billing_details_from_wallet_connector(std::optional<bool> value) { collect_billing_details_from_wallet_connector = std::move(value); }
    [[nodiscard]] const std::optional<bool>& get_collect_billing_details_from_wallet_connector() const noexcept { return collect_billing_details_from_wallet_connector; }
    void set_always_collect_shipping_details_from_wallet_connector(std::optional<bool> value) { always_collect_shipping_details_from_wallet_connector = std::move(value); }
    [[nodiscard]] const std::optional<bool>& get_always_collect_shipping_details_from_wallet_connector() const noexcept { return always_collect_shipping_details_from_wallet_connector; }
    void set_always_collect_billing_details_from_wallet_connector(std::optional<bool> value) { always_collect_billing_details_from_wallet_connector = std::move(value); }
    [[nodiscard]] const std::optional<bool>& get_always_collect_billing_details_from_wallet_connector() const noexcept { return always_collect_billing_details_from_wallet_connector; }
    void set_is_connector_agnostic_mit_enabled(std::optional<bool> value) { is_connector_agnostic_mit_enabled = std::move(value); }
    [[nodiscard]] const std::optional<bool>& get_is_connector_agnostic_mit_enabled() const noexcept { return is_connector_agnostic_mit_enabled; }
    void set_payout_link_config(std::optional<BusinessPayoutLinkConfig> value) { payout_link_config = std::move(value); }
    [[nodiscard]] const std::optional<BusinessPayoutLinkConfig>& get_payout_link_config() const noexcept { return payout_link_config; }
    void set_outgoing_webhook_custom_http_headers(std::optional<MaskedHeaders> value) { outgoing_webhook_custom_http_headers = std::move(value); }
    [[nodiscard]] const std::optional<MaskedHeaders>& get_outgoing_webhook_custom_http_headers() const noexcept { return outgoing_webhook_custom_http_headers; }
    void set_tax_connector_id(std::optional<MerchantConnectorAccountIdValue> value) { tax_connector_id = std::move(value); }
    [[nodiscard]] const std::optional<MerchantConnectorAccountIdValue>& get_tax_connector_id() const noexcept { return tax_connector_id; }
    void set_is_tax_connector_enabled(bool value) { is_tax_connector_enabled = std::move(value); }
    [[nodiscard]] const bool& get_is_tax_connector_enabled() const noexcept { return is_tax_connector_enabled; }
    void set_is_network_tokenization_enabled(bool value) { is_network_tokenization_enabled = std::move(value); }
    [[nodiscard]] const bool& get_is_network_tokenization_enabled() const noexcept { return is_network_tokenization_enabled; }
    void set_is_auto_retries_enabled(bool value) { is_auto_retries_enabled = std::move(value); }
    [[nodiscard]] const bool& get_is_auto_retries_enabled() const noexcept { return is_auto_retries_enabled; }
    void set_max_auto_retries_enabled(std::optional<std::int16_t> value) { max_auto_retries_enabled = std::move(value); }
    [[nodiscard]] const std::optional<std::int16_t>& get_max_auto_retries_enabled() const noexcept { return max_auto_retries_enabled; }
    void set_always_request_extended_authorization(std::optional<AlwaysRequestExtendedAuthorizationValue> value) { always_request_extended_authorization = std::move(value); }
    [[nodiscard]] const std::optional<AlwaysRequestExtendedAuthorizationValue>& get_always_request_extended_authorization() const noexcept { return always_request_extended_authorization; }
    void set_is_click_to_pay_enabled(bool value) { is_click_to_pay_enabled = std::move(value); }
    [[nodiscard]] const bool& get_is_click_to_pay_enabled() const noexcept { return is_click_to_pay_enabled; }
    void set_authentication_product_ids(std::optional<AuthenticationConnectorAccountMapValue> value) { authentication_product_ids = std::move(value); }
    [[nodiscard]] const std::optional<AuthenticationConnectorAccountMapValue>& get_authentication_product_ids() const noexcept { return authentication_product_ids; }
    void set_card_testing_guard_config(std::optional<CardTestingGuardConfig> value) { card_testing_guard_config = std::move(value); }
    [[nodiscard]] const std::optional<CardTestingGuardConfig>& get_card_testing_guard_config() const noexcept { return card_testing_guard_config; }
    void set_is_clear_pan_retries_enabled(bool value) { is_clear_pan_retries_enabled = std::move(value); }
    [[nodiscard]] const bool& get_is_clear_pan_retries_enabled() const noexcept { return is_clear_pan_retries_enabled; }
    void set_force_3ds_challenge(bool value) { force_3ds_challenge = std::move(value); }
    [[nodiscard]] const bool& get_force_3ds_challenge() const noexcept { return force_3ds_challenge; }
    void set_is_debit_routing_enabled(std::optional<bool> value) { is_debit_routing_enabled = std::move(value); }
    [[nodiscard]] const std::optional<bool>& get_is_debit_routing_enabled() const noexcept { return is_debit_routing_enabled; }
    void set_merchant_business_country(std::optional<CountryAlpha2Value> value) { merchant_business_country = std::move(value); }
    [[nodiscard]] const std::optional<CountryAlpha2Value>& get_merchant_business_country() const noexcept { return merchant_business_country; }
    void set_is_pre_network_tokenization_enabled(bool value) { is_pre_network_tokenization_enabled = std::move(value); }
    [[nodiscard]] const bool& get_is_pre_network_tokenization_enabled() const noexcept { return is_pre_network_tokenization_enabled; }
    void set_acquirer_configs(std::optional<std::vector<ProfileAcquirerResponseValue>> value) { acquirer_configs = std::move(value); }
    [[nodiscard]] const std::optional<std::vector<ProfileAcquirerResponseValue>>& get_acquirer_configs() const noexcept { return acquirer_configs; }
    void set_acquirer_config_bucket(std::optional<ProfileAcquirerConfigsResponseValue> value) { acquirer_config_bucket = std::move(value); }
    [[nodiscard]] const std::optional<ProfileAcquirerConfigsResponseValue>& get_acquirer_config_bucket() const noexcept { return acquirer_config_bucket; }
    void set_is_iframe_redirection_enabled(std::optional<bool> value) { is_iframe_redirection_enabled = std::move(value); }
    [[nodiscard]] const std::optional<bool>& get_is_iframe_redirection_enabled() const noexcept { return is_iframe_redirection_enabled; }
    void set_merchant_category_code(std::optional<MerchantCategoryCodeValue> value) { merchant_category_code = std::move(value); }
    [[nodiscard]] const std::optional<MerchantCategoryCodeValue>& get_merchant_category_code() const noexcept { return merchant_category_code; }
    void set_merchant_country_code(std::optional<MerchantCountryCodeValue> value) { merchant_country_code = std::move(value); }
    [[nodiscard]] const std::optional<MerchantCountryCodeValue>& get_merchant_country_code() const noexcept { return merchant_country_code; }
    void set_dispute_polling_interval(std::optional<DisputePollingIntervalInHoursValue> value) { dispute_polling_interval = std::move(value); }
    [[nodiscard]] const std::optional<DisputePollingIntervalInHoursValue>& get_dispute_polling_interval() const noexcept { return dispute_polling_interval; }
    void set_is_manual_retry_enabled(std::optional<bool> value) { is_manual_retry_enabled = std::move(value); }
    [[nodiscard]] const std::optional<bool>& get_is_manual_retry_enabled() const noexcept { return is_manual_retry_enabled; }
    void set_auto_fallback_capture_method(std::optional<bool> value) { auto_fallback_capture_method = std::move(value); }
    [[nodiscard]] const std::optional<bool>& get_auto_fallback_capture_method() const noexcept { return auto_fallback_capture_method; }
    void set_always_enable_overcapture(std::optional<AlwaysEnableOvercaptureBoolValue> value) { always_enable_overcapture = std::move(value); }
    [[nodiscard]] const std::optional<AlwaysEnableOvercaptureBoolValue>& get_always_enable_overcapture() const noexcept { return always_enable_overcapture; }
    void set_is_external_vault_enabled(std::optional<ExternalVaultEnabledValue> value) { is_external_vault_enabled = std::move(value); }
    [[nodiscard]] const std::optional<ExternalVaultEnabledValue>& get_is_external_vault_enabled() const noexcept { return is_external_vault_enabled; }
    void set_external_vault_connector_details(std::optional<ExternalVaultConnectorDetails> value) { external_vault_connector_details = std::move(value); }
    [[nodiscard]] const std::optional<ExternalVaultConnectorDetails>& get_external_vault_connector_details() const noexcept { return external_vault_connector_details; }
    void set_billing_processor_id(std::optional<MerchantConnectorAccountIdValue> value) { billing_processor_id = std::move(value); }
    [[nodiscard]] const std::optional<MerchantConnectorAccountIdValue>& get_billing_processor_id() const noexcept { return billing_processor_id; }
    void set_surcharge_connector_details(std::optional<SurchargeConnectorDetails> value) { surcharge_connector_details = std::move(value); }
    [[nodiscard]] const std::optional<SurchargeConnectorDetails>& get_surcharge_connector_details() const noexcept { return surcharge_connector_details; }
    void set_is_l2_l3_enabled(std::optional<bool> value) { is_l2_l3_enabled = std::move(value); }
    [[nodiscard]] const std::optional<bool>& get_is_l2_l3_enabled() const noexcept { return is_l2_l3_enabled; }
    void set_network_tokenization_credentials(std::optional<NetworkTokeizationProviderCredentials> value) { network_tokenization_credentials = std::move(value); }
    [[nodiscard]] const std::optional<NetworkTokeizationProviderCredentials>& get_network_tokenization_credentials() const noexcept { return network_tokenization_credentials; }
    void set_payment_method_blocking(std::optional<PaymentMethodBlockingConfig> value) { payment_method_blocking = std::move(value); }
    [[nodiscard]] const std::optional<PaymentMethodBlockingConfig>& get_payment_method_blocking() const noexcept { return payment_method_blocking; }
    MerchantIdValue merchant_id{};
    ProfileIdValue profile_id{};
    std::string profile_name{};
    std::optional<std::string> return_url{};
    bool enable_payment_response_hash{};
    std::optional<std::string> payment_response_hash_key{};
    bool redirect_to_merchant_with_http_post{};
    std::optional<WebhookDetailsResponse> webhook_details{};
    std::optional<SecretSerdeValueValue> metadata{};
    std::optional<serde::Value> routing_algorithm{};
    std::optional<std::int64_t> intent_fulfillment_time{};
    std::optional<serde::Value> frm_routing_algorithm{};
    std::optional<serde::Value> payout_routing_algorithm{};
    std::optional<std::vector<std::string>> applepay_verified_domains{};
    std::optional<std::int64_t> session_expiry{};
    std::optional<BusinessPaymentLinkConfig> payment_link_config{};
    std::optional<AuthenticationConnectorDetails> authentication_connector_details{};
    std::optional<bool> use_billing_as_payment_method_billing{};
    std::optional<ExtendedCardInfoConfig> extended_card_info_config{};
    std::optional<bool> collect_shipping_details_from_wallet_connector{};
    std::optional<bool> collect_billing_details_from_wallet_connector{};
    std::optional<bool> always_collect_shipping_details_from_wallet_connector{};
    std::optional<bool> always_collect_billing_details_from_wallet_connector{};
    std::optional<bool> is_connector_agnostic_mit_enabled{};
    std::optional<BusinessPayoutLinkConfig> payout_link_config{};
    std::optional<MaskedHeaders> outgoing_webhook_custom_http_headers{};
    std::optional<MerchantConnectorAccountIdValue> tax_connector_id{};
    bool is_tax_connector_enabled{};
    bool is_network_tokenization_enabled{};
    bool is_auto_retries_enabled{};
    std::optional<std::int16_t> max_auto_retries_enabled{};
    std::optional<AlwaysRequestExtendedAuthorizationValue> always_request_extended_authorization{};
    bool is_click_to_pay_enabled{};
    std::optional<AuthenticationConnectorAccountMapValue> authentication_product_ids{};
    std::optional<CardTestingGuardConfig> card_testing_guard_config{};
    bool is_clear_pan_retries_enabled{};
    bool force_3ds_challenge{};
    std::optional<bool> is_debit_routing_enabled{};
    std::optional<CountryAlpha2Value> merchant_business_country{};
    bool is_pre_network_tokenization_enabled{};
    std::optional<std::vector<ProfileAcquirerResponseValue>> acquirer_configs{};
    std::optional<ProfileAcquirerConfigsResponseValue> acquirer_config_bucket{};
    std::optional<bool> is_iframe_redirection_enabled{};
    std::optional<MerchantCategoryCodeValue> merchant_category_code{};
    std::optional<MerchantCountryCodeValue> merchant_country_code{};
    std::optional<DisputePollingIntervalInHoursValue> dispute_polling_interval{};
    std::optional<bool> is_manual_retry_enabled{};
    std::optional<bool> auto_fallback_capture_method{};
    std::optional<AlwaysEnableOvercaptureBoolValue> always_enable_overcapture{};
    std::optional<ExternalVaultEnabledValue> is_external_vault_enabled{};
    std::optional<ExternalVaultConnectorDetails> external_vault_connector_details{};
    std::optional<MerchantConnectorAccountIdValue> billing_processor_id{};
    std::optional<SurchargeConnectorDetails> surcharge_connector_details{};
    std::optional<bool> is_l2_l3_enabled{};
    std::optional<NetworkTokeizationProviderCredentials> network_tokenization_credentials{};
    std::optional<PaymentMethodBlockingConfig> payment_method_blocking{};
};

class ProfileUpdate
{
public:
    void set_profile_name(std::optional<std::string> value) { profile_name = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_profile_name() const noexcept { return profile_name; }
    void set_return_url(std::optional<UrlValue> value) { return_url = std::move(value); }
    [[nodiscard]] const std::optional<UrlValue>& get_return_url() const noexcept { return return_url; }
    void set_enable_payment_response_hash(std::optional<bool> value) { enable_payment_response_hash = std::move(value); }
    [[nodiscard]] const std::optional<bool>& get_enable_payment_response_hash() const noexcept { return enable_payment_response_hash; }
    void set_payment_response_hash_key(std::optional<std::string> value) { payment_response_hash_key = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_payment_response_hash_key() const noexcept { return payment_response_hash_key; }
    void set_redirect_to_merchant_with_http_post(std::optional<bool> value) { redirect_to_merchant_with_http_post = std::move(value); }
    [[nodiscard]] const std::optional<bool>& get_redirect_to_merchant_with_http_post() const noexcept { return redirect_to_merchant_with_http_post; }
    void set_webhook_details(std::optional<WebhookDetailsRequest> value) { webhook_details = std::move(value); }
    [[nodiscard]] const std::optional<WebhookDetailsRequest>& get_webhook_details() const noexcept { return webhook_details; }
    void set_metadata(std::optional<SecretSerdeValueValue> value) { metadata = std::move(value); }
    [[nodiscard]] const std::optional<SecretSerdeValueValue>& get_metadata() const noexcept { return metadata; }
    void set_routing_algorithm(std::optional<serde::Value> value) { routing_algorithm = std::move(value); }
    [[nodiscard]] const std::optional<serde::Value>& get_routing_algorithm() const noexcept { return routing_algorithm; }
    void set_intent_fulfillment_time(std::optional<std::uint32_t> value) { intent_fulfillment_time = std::move(value); }
    [[nodiscard]] const std::optional<std::uint32_t>& get_intent_fulfillment_time() const noexcept { return intent_fulfillment_time; }
    void set_frm_routing_algorithm(std::optional<serde::Value> value) { frm_routing_algorithm = std::move(value); }
    [[nodiscard]] const std::optional<serde::Value>& get_frm_routing_algorithm() const noexcept { return frm_routing_algorithm; }
    void set_payout_routing_algorithm(std::optional<serde::Value> value) { payout_routing_algorithm = std::move(value); }
    [[nodiscard]] const std::optional<serde::Value>& get_payout_routing_algorithm() const noexcept { return payout_routing_algorithm; }
    void set_applepay_verified_domains(std::optional<std::vector<std::string>> value) { applepay_verified_domains = std::move(value); }
    [[nodiscard]] const std::optional<std::vector<std::string>>& get_applepay_verified_domains() const noexcept { return applepay_verified_domains; }
    void set_session_expiry(std::optional<std::uint32_t> value) { session_expiry = std::move(value); }
    [[nodiscard]] const std::optional<std::uint32_t>& get_session_expiry() const noexcept { return session_expiry; }
    void set_payment_link_config(std::optional<BusinessPaymentLinkConfig> value) { payment_link_config = std::move(value); }
    [[nodiscard]] const std::optional<BusinessPaymentLinkConfig>& get_payment_link_config() const noexcept { return payment_link_config; }
    void set_authentication_connector_details(std::optional<AuthenticationConnectorDetails> value) { authentication_connector_details = std::move(value); }
    [[nodiscard]] const std::optional<AuthenticationConnectorDetails>& get_authentication_connector_details() const noexcept { return authentication_connector_details; }
    void set_extended_card_info_config(std::optional<ExtendedCardInfoConfig> value) { extended_card_info_config = std::move(value); }
    [[nodiscard]] const std::optional<ExtendedCardInfoConfig>& get_extended_card_info_config() const noexcept { return extended_card_info_config; }
    void set_use_billing_as_payment_method_billing(std::optional<bool> value) { use_billing_as_payment_method_billing = std::move(value); }
    [[nodiscard]] const std::optional<bool>& get_use_billing_as_payment_method_billing() const noexcept { return use_billing_as_payment_method_billing; }
    void set_collect_shipping_details_from_wallet_connector(std::optional<bool> value) { collect_shipping_details_from_wallet_connector = std::move(value); }
    [[nodiscard]] const std::optional<bool>& get_collect_shipping_details_from_wallet_connector() const noexcept { return collect_shipping_details_from_wallet_connector; }
    void set_collect_billing_details_from_wallet_connector(std::optional<bool> value) { collect_billing_details_from_wallet_connector = std::move(value); }
    [[nodiscard]] const std::optional<bool>& get_collect_billing_details_from_wallet_connector() const noexcept { return collect_billing_details_from_wallet_connector; }
    void set_always_collect_shipping_details_from_wallet_connector(std::optional<bool> value) { always_collect_shipping_details_from_wallet_connector = std::move(value); }
    [[nodiscard]] const std::optional<bool>& get_always_collect_shipping_details_from_wallet_connector() const noexcept { return always_collect_shipping_details_from_wallet_connector; }
    void set_always_collect_billing_details_from_wallet_connector(std::optional<bool> value) { always_collect_billing_details_from_wallet_connector = std::move(value); }
    [[nodiscard]] const std::optional<bool>& get_always_collect_billing_details_from_wallet_connector() const noexcept { return always_collect_billing_details_from_wallet_connector; }
    void set_always_request_extended_authorization(std::optional<AlwaysRequestExtendedAuthorizationValue> value) { always_request_extended_authorization = std::move(value); }
    [[nodiscard]] const std::optional<AlwaysRequestExtendedAuthorizationValue>& get_always_request_extended_authorization() const noexcept { return always_request_extended_authorization; }
    void set_is_connector_agnostic_mit_enabled(std::optional<bool> value) { is_connector_agnostic_mit_enabled = std::move(value); }
    [[nodiscard]] const std::optional<bool>& get_is_connector_agnostic_mit_enabled() const noexcept { return is_connector_agnostic_mit_enabled; }
    void set_payout_link_config(std::optional<BusinessPayoutLinkConfig> value) { payout_link_config = std::move(value); }
    [[nodiscard]] const std::optional<BusinessPayoutLinkConfig>& get_payout_link_config() const noexcept { return payout_link_config; }
    void set_outgoing_webhook_custom_http_headers(std::optional<std::unordered_map<std::string, std::string>> value) { outgoing_webhook_custom_http_headers = std::move(value); }
    [[nodiscard]] const std::optional<std::unordered_map<std::string, std::string>>& get_outgoing_webhook_custom_http_headers() const noexcept { return outgoing_webhook_custom_http_headers; }
    void set_tax_connector_id(std::optional<MerchantConnectorAccountIdValue> value) { tax_connector_id = std::move(value); }
    [[nodiscard]] const std::optional<MerchantConnectorAccountIdValue>& get_tax_connector_id() const noexcept { return tax_connector_id; }
    void set_is_tax_connector_enabled(std::optional<bool> value) { is_tax_connector_enabled = std::move(value); }
    [[nodiscard]] const std::optional<bool>& get_is_tax_connector_enabled() const noexcept { return is_tax_connector_enabled; }
    void set_dynamic_routing_algorithm(std::optional<serde::Value> value) { dynamic_routing_algorithm = std::move(value); }
    [[nodiscard]] const std::optional<serde::Value>& get_dynamic_routing_algorithm() const noexcept { return dynamic_routing_algorithm; }
    void set_is_network_tokenization_enabled(std::optional<bool> value) { is_network_tokenization_enabled = std::move(value); }
    [[nodiscard]] const std::optional<bool>& get_is_network_tokenization_enabled() const noexcept { return is_network_tokenization_enabled; }
    void set_is_auto_retries_enabled(std::optional<bool> value) { is_auto_retries_enabled = std::move(value); }
    [[nodiscard]] const std::optional<bool>& get_is_auto_retries_enabled() const noexcept { return is_auto_retries_enabled; }
    void set_max_auto_retries_enabled(std::optional<std::uint8_t> value) { max_auto_retries_enabled = std::move(value); }
    [[nodiscard]] const std::optional<std::uint8_t>& get_max_auto_retries_enabled() const noexcept { return max_auto_retries_enabled; }
    void set_is_click_to_pay_enabled(std::optional<bool> value) { is_click_to_pay_enabled = std::move(value); }
    [[nodiscard]] const std::optional<bool>& get_is_click_to_pay_enabled() const noexcept { return is_click_to_pay_enabled; }
    void set_authentication_product_ids(std::optional<AuthenticationConnectorAccountMapValue> value) { authentication_product_ids = std::move(value); }
    [[nodiscard]] const std::optional<AuthenticationConnectorAccountMapValue>& get_authentication_product_ids() const noexcept { return authentication_product_ids; }
    void set_card_testing_guard_config(std::optional<CardTestingGuardConfig> value) { card_testing_guard_config = std::move(value); }
    [[nodiscard]] const std::optional<CardTestingGuardConfig>& get_card_testing_guard_config() const noexcept { return card_testing_guard_config; }
    void set_is_clear_pan_retries_enabled(std::optional<bool> value) { is_clear_pan_retries_enabled = std::move(value); }
    [[nodiscard]] const std::optional<bool>& get_is_clear_pan_retries_enabled() const noexcept { return is_clear_pan_retries_enabled; }
    void set_force_3ds_challenge(std::optional<bool> value) { force_3ds_challenge = std::move(value); }
    [[nodiscard]] const std::optional<bool>& get_force_3ds_challenge() const noexcept { return force_3ds_challenge; }
    void set_is_debit_routing_enabled(std::optional<bool> value) { is_debit_routing_enabled = std::move(value); }
    [[nodiscard]] const std::optional<bool>& get_is_debit_routing_enabled() const noexcept { return is_debit_routing_enabled; }
    void set_merchant_business_country(std::optional<CountryAlpha2Value> value) { merchant_business_country = std::move(value); }
    [[nodiscard]] const std::optional<CountryAlpha2Value>& get_merchant_business_country() const noexcept { return merchant_business_country; }
    void set_is_iframe_redirection_enabled(std::optional<bool> value) { is_iframe_redirection_enabled = std::move(value); }
    [[nodiscard]] const std::optional<bool>& get_is_iframe_redirection_enabled() const noexcept { return is_iframe_redirection_enabled; }
    void set_is_pre_network_tokenization_enabled(std::optional<bool> value) { is_pre_network_tokenization_enabled = std::move(value); }
    [[nodiscard]] const std::optional<bool>& get_is_pre_network_tokenization_enabled() const noexcept { return is_pre_network_tokenization_enabled; }
    void set_merchant_category_code(std::optional<MerchantCategoryCodeValue> value) { merchant_category_code = std::move(value); }
    [[nodiscard]] const std::optional<MerchantCategoryCodeValue>& get_merchant_category_code() const noexcept { return merchant_category_code; }
    void set_merchant_country_code(std::optional<MerchantCountryCodeValue> value) { merchant_country_code = std::move(value); }
    [[nodiscard]] const std::optional<MerchantCountryCodeValue>& get_merchant_country_code() const noexcept { return merchant_country_code; }
    void set_dispute_polling_interval(std::optional<DisputePollingIntervalInHoursValue> value) { dispute_polling_interval = std::move(value); }
    [[nodiscard]] const std::optional<DisputePollingIntervalInHoursValue>& get_dispute_polling_interval() const noexcept { return dispute_polling_interval; }
    void set_is_manual_retry_enabled(std::optional<bool> value) { is_manual_retry_enabled = std::move(value); }
    [[nodiscard]] const std::optional<bool>& get_is_manual_retry_enabled() const noexcept { return is_manual_retry_enabled; }
    void set_auto_fallback_capture_method(std::optional<bool> value) { auto_fallback_capture_method = std::move(value); }
    [[nodiscard]] const std::optional<bool>& get_auto_fallback_capture_method() const noexcept { return auto_fallback_capture_method; }
    void set_always_enable_overcapture(std::optional<AlwaysEnableOvercaptureBoolValue> value) { always_enable_overcapture = std::move(value); }
    [[nodiscard]] const std::optional<AlwaysEnableOvercaptureBoolValue>& get_always_enable_overcapture() const noexcept { return always_enable_overcapture; }
    void set_is_external_vault_enabled(std::optional<ExternalVaultEnabledValue> value) { is_external_vault_enabled = std::move(value); }
    [[nodiscard]] const std::optional<ExternalVaultEnabledValue>& get_is_external_vault_enabled() const noexcept { return is_external_vault_enabled; }
    void set_external_vault_connector_details(std::optional<ExternalVaultConnectorDetails> value) { external_vault_connector_details = std::move(value); }
    [[nodiscard]] const std::optional<ExternalVaultConnectorDetails>& get_external_vault_connector_details() const noexcept { return external_vault_connector_details; }
    void set_billing_processor_id(std::optional<MerchantConnectorAccountIdValue> value) { billing_processor_id = std::move(value); }
    [[nodiscard]] const std::optional<MerchantConnectorAccountIdValue>& get_billing_processor_id() const noexcept { return billing_processor_id; }
    void set_surcharge_connector_details(std::optional<SurchargeConnectorDetails> value) { surcharge_connector_details = std::move(value); }
    [[nodiscard]] const std::optional<SurchargeConnectorDetails>& get_surcharge_connector_details() const noexcept { return surcharge_connector_details; }
    void set_is_l2_l3_enabled(std::optional<bool> value) { is_l2_l3_enabled = std::move(value); }
    [[nodiscard]] const std::optional<bool>& get_is_l2_l3_enabled() const noexcept { return is_l2_l3_enabled; }
    void set_network_tokenization_credentials(std::optional<NetworkTokeizationProviderCredentials> value) { network_tokenization_credentials = std::move(value); }
    [[nodiscard]] const std::optional<NetworkTokeizationProviderCredentials>& get_network_tokenization_credentials() const noexcept { return network_tokenization_credentials; }
    void set_payment_method_blocking(std::optional<PaymentMethodBlockingConfig> value) { payment_method_blocking = std::move(value); }
    [[nodiscard]] const std::optional<PaymentMethodBlockingConfig>& get_payment_method_blocking() const noexcept { return payment_method_blocking; }
    std::optional<std::string> profile_name{};
    std::optional<UrlValue> return_url{};
    std::optional<bool> enable_payment_response_hash{};
    std::optional<std::string> payment_response_hash_key{};
    std::optional<bool> redirect_to_merchant_with_http_post{};
    std::optional<WebhookDetailsRequest> webhook_details{};
    std::optional<SecretSerdeValueValue> metadata{};
    std::optional<serde::Value> routing_algorithm{};
    std::optional<std::uint32_t> intent_fulfillment_time{};
    std::optional<serde::Value> frm_routing_algorithm{};
    std::optional<serde::Value> payout_routing_algorithm{};
    std::optional<std::vector<std::string>> applepay_verified_domains{};
    std::optional<std::uint32_t> session_expiry{};
    std::optional<BusinessPaymentLinkConfig> payment_link_config{};
    std::optional<AuthenticationConnectorDetails> authentication_connector_details{};
    std::optional<ExtendedCardInfoConfig> extended_card_info_config{};
    std::optional<bool> use_billing_as_payment_method_billing{};
    std::optional<bool> collect_shipping_details_from_wallet_connector{};
    std::optional<bool> collect_billing_details_from_wallet_connector{};
    std::optional<bool> always_collect_shipping_details_from_wallet_connector{};
    std::optional<bool> always_collect_billing_details_from_wallet_connector{};
    std::optional<AlwaysRequestExtendedAuthorizationValue> always_request_extended_authorization{};
    std::optional<bool> is_connector_agnostic_mit_enabled{};
    std::optional<BusinessPayoutLinkConfig> payout_link_config{};
    std::optional<std::unordered_map<std::string, std::string>> outgoing_webhook_custom_http_headers{};
    std::optional<MerchantConnectorAccountIdValue> tax_connector_id{};
    std::optional<bool> is_tax_connector_enabled{};
    std::optional<serde::Value> dynamic_routing_algorithm{};
    std::optional<bool> is_network_tokenization_enabled{};
    std::optional<bool> is_auto_retries_enabled{};
    std::optional<std::uint8_t> max_auto_retries_enabled{};
    std::optional<bool> is_click_to_pay_enabled{};
    std::optional<AuthenticationConnectorAccountMapValue> authentication_product_ids{};
    std::optional<CardTestingGuardConfig> card_testing_guard_config{};
    std::optional<bool> is_clear_pan_retries_enabled{};
    std::optional<bool> force_3ds_challenge{};
    std::optional<bool> is_debit_routing_enabled{};
    std::optional<CountryAlpha2Value> merchant_business_country{};
    std::optional<bool> is_iframe_redirection_enabled{};
    std::optional<bool> is_pre_network_tokenization_enabled{};
    std::optional<MerchantCategoryCodeValue> merchant_category_code{};
    std::optional<MerchantCountryCodeValue> merchant_country_code{};
    std::optional<DisputePollingIntervalInHoursValue> dispute_polling_interval{};
    std::optional<bool> is_manual_retry_enabled{};
    std::optional<bool> auto_fallback_capture_method{};
    std::optional<AlwaysEnableOvercaptureBoolValue> always_enable_overcapture{};
    std::optional<ExternalVaultEnabledValue> is_external_vault_enabled{};
    std::optional<ExternalVaultConnectorDetails> external_vault_connector_details{};
    std::optional<MerchantConnectorAccountIdValue> billing_processor_id{};
    std::optional<SurchargeConnectorDetails> surcharge_connector_details{};
    std::optional<bool> is_l2_l3_enabled{};
    std::optional<NetworkTokeizationProviderCredentials> network_tokenization_credentials{};
    std::optional<PaymentMethodBlockingConfig> payment_method_blocking{};
};
} // namespace app::dto::admin

template<>
struct serde::Serializable<app::dto::admin::AddressDetailsValue>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"value", &app::dto::admin::AddressDetailsValue::get_value, &app::dto::admin::AddressDetailsValue::set_value, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "address_details_value"; }
};

template<>
struct serde::Serializable<app::dto::admin::AlwaysEnableOvercaptureBoolValue>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"value", &app::dto::admin::AlwaysEnableOvercaptureBoolValue::get_value, &app::dto::admin::AlwaysEnableOvercaptureBoolValue::set_value, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "always_enable_overcapture_bool_value"; }
};

template<>
struct serde::Serializable<app::dto::admin::AlwaysRequestExtendedAuthorizationValue>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"value", &app::dto::admin::AlwaysRequestExtendedAuthorizationValue::get_value, &app::dto::admin::AlwaysRequestExtendedAuthorizationValue::set_value, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "always_request_extended_authorization_value"; }
};

template<>
struct serde::Serializable<app::dto::admin::AuthenticationConnectorAccountMapValue>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"value", &app::dto::admin::AuthenticationConnectorAccountMapValue::get_value, &app::dto::admin::AuthenticationConnectorAccountMapValue::set_value, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "authentication_connector_account_map_value"; }
};

template<>
struct serde::Serializable<app::dto::admin::AuthenticationConnectorsValue>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"value", &app::dto::admin::AuthenticationConnectorsValue::get_value, &app::dto::admin::AuthenticationConnectorsValue::set_value, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "authentication_connectors_value"; }
};

template<>
struct serde::Serializable<app::dto::admin::CardNetworkValue>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"value", &app::dto::admin::CardNetworkValue::get_value, &app::dto::admin::CardNetworkValue::set_value, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "card_network_value"; }
};

template<>
struct serde::Serializable<app::dto::admin::CardSegmentTypeValue>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"value", &app::dto::admin::CardSegmentTypeValue::get_value, &app::dto::admin::CardSegmentTypeValue::set_value, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "card_segment_type_value"; }
};

template<>
struct serde::Serializable<app::dto::admin::CardTypeValue>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"value", &app::dto::admin::CardTypeValue::get_value, &app::dto::admin::CardTypeValue::set_value, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "card_type_value"; }
};

template<>
struct serde::Serializable<app::dto::admin::ConnectorStatusValue>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"value", &app::dto::admin::ConnectorStatusValue::get_value, &app::dto::admin::ConnectorStatusValue::set_value, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "connector_status_value"; }
};

template<>
struct serde::Serializable<app::dto::admin::ConnectorTypeValue>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"value", &app::dto::admin::ConnectorTypeValue::get_value, &app::dto::admin::ConnectorTypeValue::set_value, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "connector_type_value"; }
};

template<>
struct serde::Serializable<app::dto::admin::ConnectorValue>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"value", &app::dto::admin::ConnectorValue::get_value, &app::dto::admin::ConnectorValue::set_value, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "connector_value"; }
};

template<>
struct serde::Serializable<app::dto::admin::CountryAlpha2Value>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"value", &app::dto::admin::CountryAlpha2Value::get_value, &app::dto::admin::CountryAlpha2Value::set_value, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "country_alpha2_value"; }
};

template<>
struct serde::Serializable<app::dto::admin::DisputePollingIntervalInHoursValue>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"value", &app::dto::admin::DisputePollingIntervalInHoursValue::get_value, &app::dto::admin::DisputePollingIntervalInHoursValue::set_value, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "dispute_polling_interval_in_hours_value"; }
};

template<>
struct serde::Serializable<app::dto::admin::DisputeStatusValue>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"value", &app::dto::admin::DisputeStatusValue::get_value, &app::dto::admin::DisputeStatusValue::set_value, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "dispute_status_value"; }
};

template<>
struct serde::Serializable<app::dto::admin::ElementPositionValue>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"value", &app::dto::admin::ElementPositionValue::get_value, &app::dto::admin::ElementPositionValue::set_value, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "element_position_value"; }
};

template<>
struct serde::Serializable<app::dto::admin::ElementSizeValue>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"value", &app::dto::admin::ElementSizeValue::get_value, &app::dto::admin::ElementSizeValue::set_value, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "element_size_value"; }
};

template<>
struct serde::Serializable<app::dto::admin::EmailValue>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"value", &app::dto::admin::EmailValue::get_value, &app::dto::admin::EmailValue::set_value, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "email_value"; }
};

template<>
struct serde::Serializable<app::dto::admin::EnabledPaymentMethodValue>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"value", &app::dto::admin::EnabledPaymentMethodValue::get_value, &app::dto::admin::EnabledPaymentMethodValue::set_value, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "enabled_payment_method_value"; }
};

template<>
struct serde::Serializable<app::dto::admin::ExternalVaultEnabledValue>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"value", &app::dto::admin::ExternalVaultEnabledValue::get_value, &app::dto::admin::ExternalVaultEnabledValue::set_value, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "external_vault_enabled_value"; }
};

template<>
struct serde::Serializable<app::dto::admin::FrmActionValue>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"value", &app::dto::admin::FrmActionValue::get_value, &app::dto::admin::FrmActionValue::set_value, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "frm_action_value"; }
};

template<>
struct serde::Serializable<app::dto::admin::FrmPreferredFlowTypesValue>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"value", &app::dto::admin::FrmPreferredFlowTypesValue::get_value, &app::dto::admin::FrmPreferredFlowTypesValue::set_value, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "frm_preferred_flow_types_value"; }
};

template<>
struct serde::Serializable<app::dto::admin::FundingSourceValue>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"value", &app::dto::admin::FundingSourceValue::get_value, &app::dto::admin::FundingSourceValue::set_value, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "funding_source_value"; }
};

template<>
struct serde::Serializable<app::dto::admin::GenericLinkUiConfigValue>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"value", &app::dto::admin::GenericLinkUiConfigValue::get_value, &app::dto::admin::GenericLinkUiConfigValue::set_value, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "generic_link_ui_config_value"; }
};

template<>
struct serde::Serializable<app::dto::admin::IntentStatusValue>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"value", &app::dto::admin::IntentStatusValue::get_value, &app::dto::admin::IntentStatusValue::set_value, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "intent_status_value"; }
};

template<>
struct serde::Serializable<app::dto::admin::InvoiceStatusValue>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"value", &app::dto::admin::InvoiceStatusValue::get_value, &app::dto::admin::InvoiceStatusValue::set_value, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "invoice_status_value"; }
};

template<>
struct serde::Serializable<app::dto::admin::MandateStatusValue>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"value", &app::dto::admin::MandateStatusValue::get_value, &app::dto::admin::MandateStatusValue::set_value, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "mandate_status_value"; }
};

template<>
struct serde::Serializable<app::dto::admin::MerchantAccountTypeValue>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"value", &app::dto::admin::MerchantAccountTypeValue::get_value, &app::dto::admin::MerchantAccountTypeValue::set_value, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "merchant_account_type_value"; }
};

template<>
struct serde::Serializable<app::dto::admin::MerchantCategoryCodeValue>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"value", &app::dto::admin::MerchantCategoryCodeValue::get_value, &app::dto::admin::MerchantCategoryCodeValue::set_value, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "merchant_category_code_value"; }
};

template<>
struct serde::Serializable<app::dto::admin::MerchantConnectorAccountIdValue>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"value", &app::dto::admin::MerchantConnectorAccountIdValue::get_value, &app::dto::admin::MerchantConnectorAccountIdValue::set_value, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "merchant_connector_account_id_value"; }
};

template<>
struct serde::Serializable<app::dto::admin::MerchantCountryCodeValue>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"value", &app::dto::admin::MerchantCountryCodeValue::get_value, &app::dto::admin::MerchantCountryCodeValue::set_value, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "merchant_country_code_value"; }
};

template<>
struct serde::Serializable<app::dto::admin::MerchantIdValue>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"value", &app::dto::admin::MerchantIdValue::get_value, &app::dto::admin::MerchantIdValue::set_value, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "merchant_id_value"; }
};

template<>
struct serde::Serializable<app::dto::admin::MerchantProductTypeValue>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"value", &app::dto::admin::MerchantProductTypeValue::get_value, &app::dto::admin::MerchantProductTypeValue::set_value, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "merchant_product_type_value"; }
};

template<>
struct serde::Serializable<app::dto::admin::OptionalEncryptableNameValue>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"value", &app::dto::admin::OptionalEncryptableNameValue::get_value, &app::dto::admin::OptionalEncryptableNameValue::set_value, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "optional_encryptable_name_value"; }
};

template<>
struct serde::Serializable<app::dto::admin::OrganizationIdValue>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"value", &app::dto::admin::OrganizationIdValue::get_value, &app::dto::admin::OrganizationIdValue::set_value, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "organization_id_value"; }
};

template<>
struct serde::Serializable<app::dto::admin::PaymentLinkDetailsLayoutValue>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"value", &app::dto::admin::PaymentLinkDetailsLayoutValue::get_value, &app::dto::admin::PaymentLinkDetailsLayoutValue::set_value, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "payment_link_details_layout_value"; }
};

template<>
struct serde::Serializable<app::dto::admin::PaymentLinkSdkLabelTypeValue>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"value", &app::dto::admin::PaymentLinkSdkLabelTypeValue::get_value, &app::dto::admin::PaymentLinkSdkLabelTypeValue::set_value, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "payment_link_sdk_label_type_value"; }
};

template<>
struct serde::Serializable<app::dto::admin::PaymentLinkShowSdkTermsValue>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"value", &app::dto::admin::PaymentLinkShowSdkTermsValue::get_value, &app::dto::admin::PaymentLinkShowSdkTermsValue::set_value, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "payment_link_show_sdk_terms_value"; }
};

template<>
struct serde::Serializable<app::dto::admin::PaymentMethodTypeValue>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"value", &app::dto::admin::PaymentMethodTypeValue::get_value, &app::dto::admin::PaymentMethodTypeValue::set_value, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "payment_method_type_value"; }
};

template<>
struct serde::Serializable<app::dto::admin::PaymentMethodValue>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"value", &app::dto::admin::PaymentMethodValue::get_value, &app::dto::admin::PaymentMethodValue::set_value, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "payment_method_value"; }
};

template<>
struct serde::Serializable<app::dto::admin::PaymentMethodsConfigValue>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"value", &app::dto::admin::PaymentMethodsConfigValue::get_value, &app::dto::admin::PaymentMethodsConfigValue::set_value, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "payment_methods_config_value"; }
};

template<>
struct serde::Serializable<app::dto::admin::PaymentMethodsEnabledValue>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"value", &app::dto::admin::PaymentMethodsEnabledValue::get_value, &app::dto::admin::PaymentMethodsEnabledValue::set_value, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "payment_methods_enabled_value"; }
};

template<>
struct serde::Serializable<app::dto::admin::PayoutStatusValue>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"value", &app::dto::admin::PayoutStatusValue::get_value, &app::dto::admin::PayoutStatusValue::set_value, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "payout_status_value"; }
};

template<>
struct serde::Serializable<app::dto::admin::ProfileAcquirerConfigsResponseValue>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"value", &app::dto::admin::ProfileAcquirerConfigsResponseValue::get_value, &app::dto::admin::ProfileAcquirerConfigsResponseValue::set_value, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "profile_acquirer_configs_response_value"; }
};

template<>
struct serde::Serializable<app::dto::admin::ProfileAcquirerResponseValue>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"value", &app::dto::admin::ProfileAcquirerResponseValue::get_value, &app::dto::admin::ProfileAcquirerResponseValue::set_value, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "profile_acquirer_response_value"; }
};

template<>
struct serde::Serializable<app::dto::admin::ProfileIdValue>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"value", &app::dto::admin::ProfileIdValue::get_value, &app::dto::admin::ProfileIdValue::set_value, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "profile_id_value"; }
};

template<>
struct serde::Serializable<app::dto::admin::ReconStatusValue>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"value", &app::dto::admin::ReconStatusValue::get_value, &app::dto::admin::ReconStatusValue::set_value, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "recon_status_value"; }
};

template<>
struct serde::Serializable<app::dto::admin::RefundStatusValue>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"value", &app::dto::admin::RefundStatusValue::get_value, &app::dto::admin::RefundStatusValue::set_value, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "refund_status_value"; }
};

template<>
struct serde::Serializable<app::dto::admin::RequestPaymentMethodTypesValue>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"value", &app::dto::admin::RequestPaymentMethodTypesValue::get_value, &app::dto::admin::RequestPaymentMethodTypesValue::set_value, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "request_payment_method_types_value"; }
};

template<>
struct serde::Serializable<app::dto::admin::SecretSerdeValueValue>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"value", &app::dto::admin::SecretSerdeValueValue::get_value, &app::dto::admin::SecretSerdeValueValue::set_value, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "secret_serde_value_value"; }
};

template<>
struct serde::Serializable<app::dto::admin::UIWidgetFormLayoutValue>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"value", &app::dto::admin::UIWidgetFormLayoutValue::get_value, &app::dto::admin::UIWidgetFormLayoutValue::set_value, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "ui_widget_form_layout_value"; }
};

template<>
struct serde::Serializable<app::dto::admin::UrlValue>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"value", &app::dto::admin::UrlValue::get_value, &app::dto::admin::UrlValue::set_value, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "url_value"; }
};

template<>
struct serde::Serializable<app::dto::admin::VaultSdkValue>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"value", &app::dto::admin::VaultSdkValue::get_value, &app::dto::admin::VaultSdkValue::set_value, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "vault_sdk_value"; }
};

template<>
struct serde::Serializable<app::dto::admin::VaultTokenTypeValue>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"value", &app::dto::admin::VaultTokenTypeValue::get_value, &app::dto::admin::VaultTokenTypeValue::set_value, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "vault_token_type_value"; }
};

template<>
struct serde::Serializable<app::dto::admin::MerchantAccountListRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"organization_id", &app::dto::admin::MerchantAccountListRequest::get_organization_id, &app::dto::admin::MerchantAccountListRequest::set_organization_id, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "merchant_account_list_request"; }
};

template<>
struct serde::Serializable<app::dto::admin::MerchantAccountMetadata>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"compatible_connector", &app::dto::admin::MerchantAccountMetadata::get_compatible_connector, &app::dto::admin::MerchantAccountMetadata::set_compatible_connector, serde::FieldOptions{}>{},
            serde::FieldDesc<"data", &app::dto::admin::MerchantAccountMetadata::get_data, &app::dto::admin::MerchantAccountMetadata::set_data, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "merchant_account_metadata"; }
};

template<>
struct serde::Serializable<app::dto::admin::MerchantDetails>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"primary_contact_person", &app::dto::admin::MerchantDetails::get_primary_contact_person, &app::dto::admin::MerchantDetails::set_primary_contact_person, serde::FieldOptions{}>{},
            serde::FieldDesc<"primary_phone", &app::dto::admin::MerchantDetails::get_primary_phone, &app::dto::admin::MerchantDetails::set_primary_phone, serde::FieldOptions{}>{},
            serde::FieldDesc<"primary_email", &app::dto::admin::MerchantDetails::get_primary_email, &app::dto::admin::MerchantDetails::set_primary_email, serde::FieldOptions{}>{},
            serde::FieldDesc<"secondary_contact_person", &app::dto::admin::MerchantDetails::get_secondary_contact_person, &app::dto::admin::MerchantDetails::set_secondary_contact_person, serde::FieldOptions{}>{},
            serde::FieldDesc<"secondary_phone", &app::dto::admin::MerchantDetails::get_secondary_phone, &app::dto::admin::MerchantDetails::set_secondary_phone, serde::FieldOptions{}>{},
            serde::FieldDesc<"secondary_email", &app::dto::admin::MerchantDetails::get_secondary_email, &app::dto::admin::MerchantDetails::set_secondary_email, serde::FieldOptions{}>{},
            serde::FieldDesc<"website", &app::dto::admin::MerchantDetails::get_website, &app::dto::admin::MerchantDetails::set_website, serde::FieldOptions{}>{},
            serde::FieldDesc<"about_business", &app::dto::admin::MerchantDetails::get_about_business, &app::dto::admin::MerchantDetails::set_about_business, serde::FieldOptions{}>{},
            serde::FieldDesc<"address", &app::dto::admin::MerchantDetails::get_address, &app::dto::admin::MerchantDetails::set_address, serde::FieldOptions{}>{},
            serde::FieldDesc<"merchant_tax_registration_id", &app::dto::admin::MerchantDetails::get_merchant_tax_registration_id, &app::dto::admin::MerchantDetails::set_merchant_tax_registration_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"merchant_vat_registration_id", &app::dto::admin::MerchantDetails::get_merchant_vat_registration_id, &app::dto::admin::MerchantDetails::set_merchant_vat_registration_id, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "merchant_details"; }
};

template<>
struct serde::Serializable<app::dto::admin::PrimaryBusinessDetails>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"country", &app::dto::admin::PrimaryBusinessDetails::get_country, &app::dto::admin::PrimaryBusinessDetails::set_country, serde::FieldOptions{}>{},
            serde::FieldDesc<"business", &app::dto::admin::PrimaryBusinessDetails::get_business, &app::dto::admin::PrimaryBusinessDetails::set_business, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "primary_business_details"; }
};

template<>
struct serde::Serializable<app::dto::admin::WebhookDetailsRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"webhook_version", &app::dto::admin::WebhookDetailsRequest::get_webhook_version, &app::dto::admin::WebhookDetailsRequest::set_webhook_version, serde::FieldOptions{}>{},
            serde::FieldDesc<"webhook_username", &app::dto::admin::WebhookDetailsRequest::get_webhook_username, &app::dto::admin::WebhookDetailsRequest::set_webhook_username, serde::FieldOptions{}>{},
            serde::FieldDesc<"webhook_password", &app::dto::admin::WebhookDetailsRequest::get_webhook_password, &app::dto::admin::WebhookDetailsRequest::set_webhook_password, serde::FieldOptions{}>{},
            serde::FieldDesc<"webhook_url", &app::dto::admin::WebhookDetailsRequest::get_webhook_url, &app::dto::admin::WebhookDetailsRequest::set_webhook_url, serde::FieldOptions{}>{},
            serde::FieldDesc<"payment_created_enabled", &app::dto::admin::WebhookDetailsRequest::get_payment_created_enabled, &app::dto::admin::WebhookDetailsRequest::set_payment_created_enabled, serde::FieldOptions{}>{},
            serde::FieldDesc<"payment_succeeded_enabled", &app::dto::admin::WebhookDetailsRequest::get_payment_succeeded_enabled, &app::dto::admin::WebhookDetailsRequest::set_payment_succeeded_enabled, serde::FieldOptions{}>{},
            serde::FieldDesc<"payment_failed_enabled", &app::dto::admin::WebhookDetailsRequest::get_payment_failed_enabled, &app::dto::admin::WebhookDetailsRequest::set_payment_failed_enabled, serde::FieldOptions{}>{},
            serde::FieldDesc<"payment_statuses_enabled", &app::dto::admin::WebhookDetailsRequest::get_payment_statuses_enabled, &app::dto::admin::WebhookDetailsRequest::set_payment_statuses_enabled, serde::FieldOptions{}>{},
            serde::FieldDesc<"refund_statuses_enabled", &app::dto::admin::WebhookDetailsRequest::get_refund_statuses_enabled, &app::dto::admin::WebhookDetailsRequest::set_refund_statuses_enabled, serde::FieldOptions{}>{},
            serde::FieldDesc<"payout_statuses_enabled", &app::dto::admin::WebhookDetailsRequest::get_payout_statuses_enabled, &app::dto::admin::WebhookDetailsRequest::set_payout_statuses_enabled, serde::FieldOptions{}>{},
            serde::FieldDesc<"dispute_statuses_enabled", &app::dto::admin::WebhookDetailsRequest::get_dispute_statuses_enabled, &app::dto::admin::WebhookDetailsRequest::set_dispute_statuses_enabled, serde::FieldOptions{}>{},
            serde::FieldDesc<"mandate_statuses_enabled", &app::dto::admin::WebhookDetailsRequest::get_mandate_statuses_enabled, &app::dto::admin::WebhookDetailsRequest::set_mandate_statuses_enabled, serde::FieldOptions{}>{},
            serde::FieldDesc<"invoice_statuses_enabled", &app::dto::admin::WebhookDetailsRequest::get_invoice_statuses_enabled, &app::dto::admin::WebhookDetailsRequest::set_invoice_statuses_enabled, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "webhook_details_request"; }
};

template<>
struct serde::Serializable<app::dto::admin::BusinessGenericLinkConfig>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"domain_name", &app::dto::admin::BusinessGenericLinkConfig::get_domain_name, &app::dto::admin::BusinessGenericLinkConfig::set_domain_name, serde::FieldOptions{}>{},
            serde::FieldDesc<"allowed_domains", &app::dto::admin::BusinessGenericLinkConfig::get_allowed_domains, &app::dto::admin::BusinessGenericLinkConfig::set_allowed_domains, serde::FieldOptions{}>{},
            serde::FieldDesc<"ui_config", &app::dto::admin::BusinessGenericLinkConfig::get_ui_config, &app::dto::admin::BusinessGenericLinkConfig::set_ui_config, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "business_generic_link_config"; }
};

template<>
struct serde::Serializable<app::dto::admin::BusinessCollectLinkConfig>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"config", &app::dto::admin::BusinessCollectLinkConfig::get_config, &app::dto::admin::BusinessCollectLinkConfig::set_config, serde::FieldOptions{}>{},
            serde::FieldDesc<"enabled_payment_methods", &app::dto::admin::BusinessCollectLinkConfig::get_enabled_payment_methods, &app::dto::admin::BusinessCollectLinkConfig::set_enabled_payment_methods, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "business_collect_link_config"; }
};

template<>
struct serde::Serializable<app::dto::admin::MerchantAccountCreate>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"merchant_id", &app::dto::admin::MerchantAccountCreate::get_merchant_id, &app::dto::admin::MerchantAccountCreate::set_merchant_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"merchant_name", &app::dto::admin::MerchantAccountCreate::get_merchant_name, &app::dto::admin::MerchantAccountCreate::set_merchant_name, serde::FieldOptions{}>{},
            serde::FieldDesc<"merchant_details", &app::dto::admin::MerchantAccountCreate::get_merchant_details, &app::dto::admin::MerchantAccountCreate::set_merchant_details, serde::FieldOptions{}>{},
            serde::FieldDesc<"return_url", &app::dto::admin::MerchantAccountCreate::get_return_url, &app::dto::admin::MerchantAccountCreate::set_return_url, serde::FieldOptions{}>{},
            serde::FieldDesc<"webhook_details", &app::dto::admin::MerchantAccountCreate::get_webhook_details, &app::dto::admin::MerchantAccountCreate::set_webhook_details, serde::FieldOptions{}>{},
            serde::FieldDesc<"routing_algorithm", &app::dto::admin::MerchantAccountCreate::get_routing_algorithm, &app::dto::admin::MerchantAccountCreate::set_routing_algorithm, serde::FieldOptions{}>{},
            serde::FieldDesc<"payout_routing_algorithm", &app::dto::admin::MerchantAccountCreate::get_payout_routing_algorithm, &app::dto::admin::MerchantAccountCreate::set_payout_routing_algorithm, serde::FieldOptions{}>{},
            serde::FieldDesc<"sub_merchants_enabled", &app::dto::admin::MerchantAccountCreate::get_sub_merchants_enabled, &app::dto::admin::MerchantAccountCreate::set_sub_merchants_enabled, serde::FieldOptions{}>{},
            serde::FieldDesc<"parent_merchant_id", &app::dto::admin::MerchantAccountCreate::get_parent_merchant_id, &app::dto::admin::MerchantAccountCreate::set_parent_merchant_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"enable_payment_response_hash", &app::dto::admin::MerchantAccountCreate::get_enable_payment_response_hash, &app::dto::admin::MerchantAccountCreate::set_enable_payment_response_hash, serde::FieldOptions{}>{},
            serde::FieldDesc<"payment_response_hash_key", &app::dto::admin::MerchantAccountCreate::get_payment_response_hash_key, &app::dto::admin::MerchantAccountCreate::set_payment_response_hash_key, serde::FieldOptions{}>{},
            serde::FieldDesc<"redirect_to_merchant_with_http_post", &app::dto::admin::MerchantAccountCreate::get_redirect_to_merchant_with_http_post, &app::dto::admin::MerchantAccountCreate::set_redirect_to_merchant_with_http_post, serde::FieldOptions{}>{},
            serde::FieldDesc<"metadata", &app::dto::admin::MerchantAccountCreate::get_metadata, &app::dto::admin::MerchantAccountCreate::set_metadata, serde::FieldOptions{}>{},
            serde::FieldDesc<"publishable_key", &app::dto::admin::MerchantAccountCreate::get_publishable_key, &app::dto::admin::MerchantAccountCreate::set_publishable_key, serde::FieldOptions{}>{},
            serde::FieldDesc<"locker_id", &app::dto::admin::MerchantAccountCreate::get_locker_id, &app::dto::admin::MerchantAccountCreate::set_locker_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"primary_business_details", &app::dto::admin::MerchantAccountCreate::get_primary_business_details, &app::dto::admin::MerchantAccountCreate::set_primary_business_details, serde::FieldOptions{}>{},
            serde::FieldDesc<"frm_routing_algorithm", &app::dto::admin::MerchantAccountCreate::get_frm_routing_algorithm, &app::dto::admin::MerchantAccountCreate::set_frm_routing_algorithm, serde::FieldOptions{}>{},
            serde::FieldDesc<"organization_id", &app::dto::admin::MerchantAccountCreate::get_organization_id, &app::dto::admin::MerchantAccountCreate::set_organization_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"pm_collect_link_config", &app::dto::admin::MerchantAccountCreate::get_pm_collect_link_config, &app::dto::admin::MerchantAccountCreate::set_pm_collect_link_config, serde::FieldOptions{}>{},
            serde::FieldDesc<"product_type", &app::dto::admin::MerchantAccountCreate::get_product_type, &app::dto::admin::MerchantAccountCreate::set_product_type, serde::FieldOptions{}>{},
            serde::FieldDesc<"merchant_account_type", &app::dto::admin::MerchantAccountCreate::get_merchant_account_type, &app::dto::admin::MerchantAccountCreate::set_merchant_account_type, serde::FieldOptions{}>{},
            serde::FieldDesc<"network_tokenization_credentials", &app::dto::admin::MerchantAccountCreate::get_network_tokenization_credentials, &app::dto::admin::MerchantAccountCreate::set_network_tokenization_credentials, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "merchant_account_create"; }
};

template<>
struct serde::Serializable<app::dto::admin::MerchantAccountCreateWithoutOrgId>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"merchant_name", &app::dto::admin::MerchantAccountCreateWithoutOrgId::get_merchant_name, &app::dto::admin::MerchantAccountCreateWithoutOrgId::set_merchant_name, serde::FieldOptions{}>{},
            serde::FieldDesc<"merchant_details", &app::dto::admin::MerchantAccountCreateWithoutOrgId::get_merchant_details, &app::dto::admin::MerchantAccountCreateWithoutOrgId::set_merchant_details, serde::FieldOptions{}>{},
            serde::FieldDesc<"metadata", &app::dto::admin::MerchantAccountCreateWithoutOrgId::get_metadata, &app::dto::admin::MerchantAccountCreateWithoutOrgId::set_metadata, serde::FieldOptions{}>{},
            serde::FieldDesc<"product_type", &app::dto::admin::MerchantAccountCreateWithoutOrgId::get_product_type, &app::dto::admin::MerchantAccountCreateWithoutOrgId::set_product_type, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "merchant_account_create_without_org_id"; }
};

template<>
struct serde::Serializable<app::dto::admin::CardTestingGuardConfig>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"card_ip_blocking_status", &app::dto::admin::CardTestingGuardConfig::get_card_ip_blocking_status, &app::dto::admin::CardTestingGuardConfig::set_card_ip_blocking_status, serde::FieldOptions{}>{},
            serde::FieldDesc<"card_ip_blocking_threshold", &app::dto::admin::CardTestingGuardConfig::get_card_ip_blocking_threshold, &app::dto::admin::CardTestingGuardConfig::set_card_ip_blocking_threshold, serde::FieldOptions{}>{},
            serde::FieldDesc<"guest_user_card_blocking_status", &app::dto::admin::CardTestingGuardConfig::get_guest_user_card_blocking_status, &app::dto::admin::CardTestingGuardConfig::set_guest_user_card_blocking_status, serde::FieldOptions{}>{},
            serde::FieldDesc<"guest_user_card_blocking_threshold", &app::dto::admin::CardTestingGuardConfig::get_guest_user_card_blocking_threshold, &app::dto::admin::CardTestingGuardConfig::set_guest_user_card_blocking_threshold, serde::FieldOptions{}>{},
            serde::FieldDesc<"customer_id_blocking_status", &app::dto::admin::CardTestingGuardConfig::get_customer_id_blocking_status, &app::dto::admin::CardTestingGuardConfig::set_customer_id_blocking_status, serde::FieldOptions{}>{},
            serde::FieldDesc<"customer_id_blocking_threshold", &app::dto::admin::CardTestingGuardConfig::get_customer_id_blocking_threshold, &app::dto::admin::CardTestingGuardConfig::set_customer_id_blocking_threshold, serde::FieldOptions{}>{},
            serde::FieldDesc<"card_testing_guard_expiry", &app::dto::admin::CardTestingGuardConfig::get_card_testing_guard_expiry, &app::dto::admin::CardTestingGuardConfig::set_card_testing_guard_expiry, serde::FieldOptions{}>{},
            serde::FieldDesc<"guest_ip_blocking_status", &app::dto::admin::CardTestingGuardConfig::get_guest_ip_blocking_status, &app::dto::admin::CardTestingGuardConfig::set_guest_ip_blocking_status, serde::FieldOptions{}>{},
            serde::FieldDesc<"guest_ip_blocking_threshold", &app::dto::admin::CardTestingGuardConfig::get_guest_ip_blocking_threshold, &app::dto::admin::CardTestingGuardConfig::set_guest_ip_blocking_threshold, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "card_testing_guard_config"; }
};

template<>
struct serde::Serializable<app::dto::admin::CardBlockingConfig>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"issuing_country", &app::dto::admin::CardBlockingConfig::get_issuing_country, &app::dto::admin::CardBlockingConfig::set_issuing_country, serde::FieldOptions{}>{},
            serde::FieldDesc<"card_types", &app::dto::admin::CardBlockingConfig::get_card_types, &app::dto::admin::CardBlockingConfig::set_card_types, serde::FieldOptions{}>{},
            serde::FieldDesc<"card_subtypes", &app::dto::admin::CardBlockingConfig::get_card_subtypes, &app::dto::admin::CardBlockingConfig::set_card_subtypes, serde::FieldOptions{}>{},
            serde::FieldDesc<"issuers", &app::dto::admin::CardBlockingConfig::get_issuers, &app::dto::admin::CardBlockingConfig::set_issuers, serde::FieldOptions{}>{},
            serde::FieldDesc<"block_if_bin_info_unavailable", &app::dto::admin::CardBlockingConfig::get_block_if_bin_info_unavailable, &app::dto::admin::CardBlockingConfig::set_block_if_bin_info_unavailable, serde::FieldOptions{}>{},
            serde::FieldDesc<"card_networks", &app::dto::admin::CardBlockingConfig::get_card_networks, &app::dto::admin::CardBlockingConfig::set_card_networks, serde::FieldOptions{}>{},
            serde::FieldDesc<"funding_sources", &app::dto::admin::CardBlockingConfig::get_funding_sources, &app::dto::admin::CardBlockingConfig::set_funding_sources, serde::FieldOptions{}>{},
            serde::FieldDesc<"card_segment_types", &app::dto::admin::CardBlockingConfig::get_card_segment_types, &app::dto::admin::CardBlockingConfig::set_card_segment_types, serde::FieldOptions{}>{},
            serde::FieldDesc<"block_virtual_cards", &app::dto::admin::CardBlockingConfig::get_block_virtual_cards, &app::dto::admin::CardBlockingConfig::set_block_virtual_cards, serde::FieldOptions{}>{},
            serde::FieldDesc<"block_non_reloadable_prepaid_cards", &app::dto::admin::CardBlockingConfig::get_block_non_reloadable_prepaid_cards, &app::dto::admin::CardBlockingConfig::set_block_non_reloadable_prepaid_cards, serde::FieldOptions{}>{},
            serde::FieldDesc<"gambling_blocked", &app::dto::admin::CardBlockingConfig::get_gambling_blocked, &app::dto::admin::CardBlockingConfig::set_gambling_blocked, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "card_blocking_config"; }
};

template<>
struct serde::Serializable<app::dto::admin::WalletBlockingConfig>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"card_types", &app::dto::admin::WalletBlockingConfig::get_card_types, &app::dto::admin::WalletBlockingConfig::set_card_types, serde::FieldOptions{}>{},
            serde::FieldDesc<"apple_pay", &app::dto::admin::WalletBlockingConfig::get_apple_pay, &app::dto::admin::WalletBlockingConfig::set_apple_pay, serde::FieldOptions{}>{},
            serde::FieldDesc<"google_pay", &app::dto::admin::WalletBlockingConfig::get_google_pay, &app::dto::admin::WalletBlockingConfig::set_google_pay, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "wallet_blocking_config"; }
};

template<>
struct serde::Serializable<app::dto::admin::PaymentMethodBlockingConfig>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"card", &app::dto::admin::PaymentMethodBlockingConfig::get_card, &app::dto::admin::PaymentMethodBlockingConfig::set_card, serde::FieldOptions{}>{},
            serde::FieldDesc<"wallet", &app::dto::admin::PaymentMethodBlockingConfig::get_wallet, &app::dto::admin::PaymentMethodBlockingConfig::set_wallet, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "payment_method_blocking_config"; }
};

template<>
struct serde::Serializable<app::dto::admin::AuthenticationConnectorDetails>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"authentication_connectors", &app::dto::admin::AuthenticationConnectorDetails::get_authentication_connectors, &app::dto::admin::AuthenticationConnectorDetails::set_authentication_connectors, serde::FieldOptions{}>{},
            serde::FieldDesc<"three_ds_requestor_url", &app::dto::admin::AuthenticationConnectorDetails::get_three_ds_requestor_url, &app::dto::admin::AuthenticationConnectorDetails::set_three_ds_requestor_url, serde::FieldOptions{}>{},
            serde::FieldDesc<"three_ds_requestor_app_url", &app::dto::admin::AuthenticationConnectorDetails::get_three_ds_requestor_app_url, &app::dto::admin::AuthenticationConnectorDetails::set_three_ds_requestor_app_url, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "authentication_connector_details"; }
};

template<>
struct serde::Serializable<app::dto::admin::VaultTokenField>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"token_type", &app::dto::admin::VaultTokenField::get_token_type, &app::dto::admin::VaultTokenField::set_token_type, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "vault_token_field"; }
};

template<>
struct serde::Serializable<app::dto::admin::ExternalVaultConnectorDetails>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"vault_connector_id", &app::dto::admin::ExternalVaultConnectorDetails::get_vault_connector_id, &app::dto::admin::ExternalVaultConnectorDetails::set_vault_connector_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"vault_sdk", &app::dto::admin::ExternalVaultConnectorDetails::get_vault_sdk, &app::dto::admin::ExternalVaultConnectorDetails::set_vault_sdk, serde::FieldOptions{}>{},
            serde::FieldDesc<"vault_token_selector", &app::dto::admin::ExternalVaultConnectorDetails::get_vault_token_selector, &app::dto::admin::ExternalVaultConnectorDetails::set_vault_token_selector, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "external_vault_connector_details"; }
};

template<>
struct serde::Serializable<app::dto::admin::SurchargeConnectorDetails>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"surcharge_connector_id", &app::dto::admin::SurchargeConnectorDetails::get_surcharge_connector_id, &app::dto::admin::SurchargeConnectorDetails::set_surcharge_connector_id, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "surcharge_connector_details"; }
};

template<>
struct serde::Serializable<app::dto::admin::OfferEngineMerchantConfig>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"api_key", &app::dto::admin::OfferEngineMerchantConfig::get_api_key, &app::dto::admin::OfferEngineMerchantConfig::set_api_key, serde::FieldOptions{}>{},
            serde::FieldDesc<"merchant_id", &app::dto::admin::OfferEngineMerchantConfig::get_merchant_id, &app::dto::admin::OfferEngineMerchantConfig::set_merchant_id, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "offer_engine_merchant_config"; }
};

template<>
struct serde::Serializable<app::dto::admin::MerchantAccountUpdate>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"merchant_id", &app::dto::admin::MerchantAccountUpdate::get_merchant_id, &app::dto::admin::MerchantAccountUpdate::set_merchant_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"merchant_name", &app::dto::admin::MerchantAccountUpdate::get_merchant_name, &app::dto::admin::MerchantAccountUpdate::set_merchant_name, serde::FieldOptions{}>{},
            serde::FieldDesc<"merchant_details", &app::dto::admin::MerchantAccountUpdate::get_merchant_details, &app::dto::admin::MerchantAccountUpdate::set_merchant_details, serde::FieldOptions{}>{},
            serde::FieldDesc<"return_url", &app::dto::admin::MerchantAccountUpdate::get_return_url, &app::dto::admin::MerchantAccountUpdate::set_return_url, serde::FieldOptions{}>{},
            serde::FieldDesc<"webhook_details", &app::dto::admin::MerchantAccountUpdate::get_webhook_details, &app::dto::admin::MerchantAccountUpdate::set_webhook_details, serde::FieldOptions{}>{},
            serde::FieldDesc<"routing_algorithm", &app::dto::admin::MerchantAccountUpdate::get_routing_algorithm, &app::dto::admin::MerchantAccountUpdate::set_routing_algorithm, serde::FieldOptions{}>{},
            serde::FieldDesc<"payout_routing_algorithm", &app::dto::admin::MerchantAccountUpdate::get_payout_routing_algorithm, &app::dto::admin::MerchantAccountUpdate::set_payout_routing_algorithm, serde::FieldOptions{}>{},
            serde::FieldDesc<"sub_merchants_enabled", &app::dto::admin::MerchantAccountUpdate::get_sub_merchants_enabled, &app::dto::admin::MerchantAccountUpdate::set_sub_merchants_enabled, serde::FieldOptions{}>{},
            serde::FieldDesc<"parent_merchant_id", &app::dto::admin::MerchantAccountUpdate::get_parent_merchant_id, &app::dto::admin::MerchantAccountUpdate::set_parent_merchant_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"enable_payment_response_hash", &app::dto::admin::MerchantAccountUpdate::get_enable_payment_response_hash, &app::dto::admin::MerchantAccountUpdate::set_enable_payment_response_hash, serde::FieldOptions{}>{},
            serde::FieldDesc<"payment_response_hash_key", &app::dto::admin::MerchantAccountUpdate::get_payment_response_hash_key, &app::dto::admin::MerchantAccountUpdate::set_payment_response_hash_key, serde::FieldOptions{}>{},
            serde::FieldDesc<"redirect_to_merchant_with_http_post", &app::dto::admin::MerchantAccountUpdate::get_redirect_to_merchant_with_http_post, &app::dto::admin::MerchantAccountUpdate::set_redirect_to_merchant_with_http_post, serde::FieldOptions{}>{},
            serde::FieldDesc<"metadata", &app::dto::admin::MerchantAccountUpdate::get_metadata, &app::dto::admin::MerchantAccountUpdate::set_metadata, serde::FieldOptions{}>{},
            serde::FieldDesc<"publishable_key", &app::dto::admin::MerchantAccountUpdate::get_publishable_key, &app::dto::admin::MerchantAccountUpdate::set_publishable_key, serde::FieldOptions{}>{},
            serde::FieldDesc<"locker_id", &app::dto::admin::MerchantAccountUpdate::get_locker_id, &app::dto::admin::MerchantAccountUpdate::set_locker_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"primary_business_details", &app::dto::admin::MerchantAccountUpdate::get_primary_business_details, &app::dto::admin::MerchantAccountUpdate::set_primary_business_details, serde::FieldOptions{}>{},
            serde::FieldDesc<"frm_routing_algorithm", &app::dto::admin::MerchantAccountUpdate::get_frm_routing_algorithm, &app::dto::admin::MerchantAccountUpdate::set_frm_routing_algorithm, serde::FieldOptions{}>{},
            serde::FieldDesc<"default_profile", &app::dto::admin::MerchantAccountUpdate::get_default_profile, &app::dto::admin::MerchantAccountUpdate::set_default_profile, serde::FieldOptions{}>{},
            serde::FieldDesc<"pm_collect_link_config", &app::dto::admin::MerchantAccountUpdate::get_pm_collect_link_config, &app::dto::admin::MerchantAccountUpdate::set_pm_collect_link_config, serde::FieldOptions{}>{},
            serde::FieldDesc<"network_tokenization_credentials", &app::dto::admin::MerchantAccountUpdate::get_network_tokenization_credentials, &app::dto::admin::MerchantAccountUpdate::set_network_tokenization_credentials, serde::FieldOptions{}>{},
            serde::FieldDesc<"offer_engine_config", &app::dto::admin::MerchantAccountUpdate::get_offer_engine_config, &app::dto::admin::MerchantAccountUpdate::set_offer_engine_config, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "merchant_account_update"; }
};

template<>
struct serde::Serializable<app::dto::admin::WebhookDetailsResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"webhook_version", &app::dto::admin::WebhookDetailsResponse::get_webhook_version, &app::dto::admin::WebhookDetailsResponse::set_webhook_version, serde::FieldOptions{}>{},
            serde::FieldDesc<"webhook_username", &app::dto::admin::WebhookDetailsResponse::get_webhook_username, &app::dto::admin::WebhookDetailsResponse::set_webhook_username, serde::FieldOptions{}>{},
            serde::FieldDesc<"webhook_password", &app::dto::admin::WebhookDetailsResponse::get_webhook_password, &app::dto::admin::WebhookDetailsResponse::set_webhook_password, serde::FieldOptions{}>{},
            serde::FieldDesc<"webhook_url", &app::dto::admin::WebhookDetailsResponse::get_webhook_url, &app::dto::admin::WebhookDetailsResponse::set_webhook_url, serde::FieldOptions{}>{},
            serde::FieldDesc<"payment_created_enabled", &app::dto::admin::WebhookDetailsResponse::get_payment_created_enabled, &app::dto::admin::WebhookDetailsResponse::set_payment_created_enabled, serde::FieldOptions{}>{},
            serde::FieldDesc<"payment_succeeded_enabled", &app::dto::admin::WebhookDetailsResponse::get_payment_succeeded_enabled, &app::dto::admin::WebhookDetailsResponse::set_payment_succeeded_enabled, serde::FieldOptions{}>{},
            serde::FieldDesc<"payment_failed_enabled", &app::dto::admin::WebhookDetailsResponse::get_payment_failed_enabled, &app::dto::admin::WebhookDetailsResponse::set_payment_failed_enabled, serde::FieldOptions{}>{},
            serde::FieldDesc<"payment_statuses_enabled", &app::dto::admin::WebhookDetailsResponse::get_payment_statuses_enabled, &app::dto::admin::WebhookDetailsResponse::set_payment_statuses_enabled, serde::FieldOptions{}>{},
            serde::FieldDesc<"refund_statuses_enabled", &app::dto::admin::WebhookDetailsResponse::get_refund_statuses_enabled, &app::dto::admin::WebhookDetailsResponse::set_refund_statuses_enabled, serde::FieldOptions{}>{},
            serde::FieldDesc<"payout_statuses_enabled", &app::dto::admin::WebhookDetailsResponse::get_payout_statuses_enabled, &app::dto::admin::WebhookDetailsResponse::set_payout_statuses_enabled, serde::FieldOptions{}>{},
            serde::FieldDesc<"dispute_statuses_enabled", &app::dto::admin::WebhookDetailsResponse::get_dispute_statuses_enabled, &app::dto::admin::WebhookDetailsResponse::set_dispute_statuses_enabled, serde::FieldOptions{}>{},
            serde::FieldDesc<"mandate_statuses_enabled", &app::dto::admin::WebhookDetailsResponse::get_mandate_statuses_enabled, &app::dto::admin::WebhookDetailsResponse::set_mandate_statuses_enabled, serde::FieldOptions{}>{},
            serde::FieldDesc<"invoice_statuses_enabled", &app::dto::admin::WebhookDetailsResponse::get_invoice_statuses_enabled, &app::dto::admin::WebhookDetailsResponse::set_invoice_statuses_enabled, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "webhook_details_response"; }
};

template<>
struct serde::Serializable<app::dto::admin::MerchantAccountResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"merchant_id", &app::dto::admin::MerchantAccountResponse::get_merchant_id, &app::dto::admin::MerchantAccountResponse::set_merchant_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"merchant_name", &app::dto::admin::MerchantAccountResponse::get_merchant_name, &app::dto::admin::MerchantAccountResponse::set_merchant_name, serde::FieldOptions{}>{},
            serde::FieldDesc<"return_url", &app::dto::admin::MerchantAccountResponse::get_return_url, &app::dto::admin::MerchantAccountResponse::set_return_url, serde::FieldOptions{}>{},
            serde::FieldDesc<"enable_payment_response_hash", &app::dto::admin::MerchantAccountResponse::get_enable_payment_response_hash, &app::dto::admin::MerchantAccountResponse::set_enable_payment_response_hash, serde::FieldOptions{}>{},
            serde::FieldDesc<"payment_response_hash_key", &app::dto::admin::MerchantAccountResponse::get_payment_response_hash_key, &app::dto::admin::MerchantAccountResponse::set_payment_response_hash_key, serde::FieldOptions{}>{},
            serde::FieldDesc<"redirect_to_merchant_with_http_post", &app::dto::admin::MerchantAccountResponse::get_redirect_to_merchant_with_http_post, &app::dto::admin::MerchantAccountResponse::set_redirect_to_merchant_with_http_post, serde::FieldOptions{}>{},
            serde::FieldDesc<"merchant_details", &app::dto::admin::MerchantAccountResponse::get_merchant_details, &app::dto::admin::MerchantAccountResponse::set_merchant_details, serde::FieldOptions{}>{},
            serde::FieldDesc<"webhook_details", &app::dto::admin::MerchantAccountResponse::get_webhook_details, &app::dto::admin::MerchantAccountResponse::set_webhook_details, serde::FieldOptions{}>{},
            serde::FieldDesc<"routing_algorithm", &app::dto::admin::MerchantAccountResponse::get_routing_algorithm, &app::dto::admin::MerchantAccountResponse::set_routing_algorithm, serde::FieldOptions{}>{},
            serde::FieldDesc<"payout_routing_algorithm", &app::dto::admin::MerchantAccountResponse::get_payout_routing_algorithm, &app::dto::admin::MerchantAccountResponse::set_payout_routing_algorithm, serde::FieldOptions{}>{},
            serde::FieldDesc<"sub_merchants_enabled", &app::dto::admin::MerchantAccountResponse::get_sub_merchants_enabled, &app::dto::admin::MerchantAccountResponse::set_sub_merchants_enabled, serde::FieldOptions{}>{},
            serde::FieldDesc<"parent_merchant_id", &app::dto::admin::MerchantAccountResponse::get_parent_merchant_id, &app::dto::admin::MerchantAccountResponse::set_parent_merchant_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"publishable_key", &app::dto::admin::MerchantAccountResponse::get_publishable_key, &app::dto::admin::MerchantAccountResponse::set_publishable_key, serde::FieldOptions{}>{},
            serde::FieldDesc<"metadata", &app::dto::admin::MerchantAccountResponse::get_metadata, &app::dto::admin::MerchantAccountResponse::set_metadata, serde::FieldOptions{}>{},
            serde::FieldDesc<"locker_id", &app::dto::admin::MerchantAccountResponse::get_locker_id, &app::dto::admin::MerchantAccountResponse::set_locker_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"primary_business_details", &app::dto::admin::MerchantAccountResponse::get_primary_business_details, &app::dto::admin::MerchantAccountResponse::set_primary_business_details, serde::FieldOptions{}>{},
            serde::FieldDesc<"frm_routing_algorithm", &app::dto::admin::MerchantAccountResponse::get_frm_routing_algorithm, &app::dto::admin::MerchantAccountResponse::set_frm_routing_algorithm, serde::FieldOptions{}>{},
            serde::FieldDesc<"organization_id", &app::dto::admin::MerchantAccountResponse::get_organization_id, &app::dto::admin::MerchantAccountResponse::set_organization_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"is_recon_enabled", &app::dto::admin::MerchantAccountResponse::get_is_recon_enabled, &app::dto::admin::MerchantAccountResponse::set_is_recon_enabled, serde::FieldOptions{}>{},
            serde::FieldDesc<"default_profile", &app::dto::admin::MerchantAccountResponse::get_default_profile, &app::dto::admin::MerchantAccountResponse::set_default_profile, serde::FieldOptions{}>{},
            serde::FieldDesc<"recon_status", &app::dto::admin::MerchantAccountResponse::get_recon_status, &app::dto::admin::MerchantAccountResponse::set_recon_status, serde::FieldOptions{}>{},
            serde::FieldDesc<"pm_collect_link_config", &app::dto::admin::MerchantAccountResponse::get_pm_collect_link_config, &app::dto::admin::MerchantAccountResponse::set_pm_collect_link_config, serde::FieldOptions{}>{},
            serde::FieldDesc<"product_type", &app::dto::admin::MerchantAccountResponse::get_product_type, &app::dto::admin::MerchantAccountResponse::set_product_type, serde::FieldOptions{}>{},
            serde::FieldDesc<"merchant_account_type", &app::dto::admin::MerchantAccountResponse::get_merchant_account_type, &app::dto::admin::MerchantAccountResponse::set_merchant_account_type, serde::FieldOptions{}>{},
            serde::FieldDesc<"network_tokenization_credentials", &app::dto::admin::MerchantAccountResponse::get_network_tokenization_credentials, &app::dto::admin::MerchantAccountResponse::set_network_tokenization_credentials, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "merchant_account_response"; }
};

template<>
struct serde::Serializable<app::dto::admin::InternalNetworkTokenizationCredentials>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"token_service_api_key", &app::dto::admin::InternalNetworkTokenizationCredentials::get_token_service_api_key, &app::dto::admin::InternalNetworkTokenizationCredentials::set_token_service_api_key, serde::FieldOptions{}>{},
            serde::FieldDesc<"public_key", &app::dto::admin::InternalNetworkTokenizationCredentials::get_public_key, &app::dto::admin::InternalNetworkTokenizationCredentials::set_public_key, serde::FieldOptions{}>{},
            serde::FieldDesc<"private_key", &app::dto::admin::InternalNetworkTokenizationCredentials::get_private_key, &app::dto::admin::InternalNetworkTokenizationCredentials::set_private_key, serde::FieldOptions{}>{},
            serde::FieldDesc<"key_id", &app::dto::admin::InternalNetworkTokenizationCredentials::get_key_id, &app::dto::admin::InternalNetworkTokenizationCredentials::set_key_id, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "internal_network_tokenization_credentials"; }
};

template<>
struct serde::Serializable<app::dto::admin::MerchantAccountDeleteResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"merchant_id", &app::dto::admin::MerchantAccountDeleteResponse::get_merchant_id, &app::dto::admin::MerchantAccountDeleteResponse::set_merchant_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"deleted", &app::dto::admin::MerchantAccountDeleteResponse::get_deleted, &app::dto::admin::MerchantAccountDeleteResponse::set_deleted, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "merchant_account_delete_response"; }
};

template<>
struct serde::Serializable<app::dto::admin::MerchantId>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"merchant_id", &app::dto::admin::MerchantId::get_merchant_id, &app::dto::admin::MerchantId::set_merchant_id, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "merchant_id"; }
};

template<>
struct serde::Serializable<app::dto::admin::MerchantConnectorId>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"merchant_id", &app::dto::admin::MerchantConnectorId::get_merchant_id, &app::dto::admin::MerchantConnectorId::set_merchant_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"merchant_connector_id", &app::dto::admin::MerchantConnectorId::get_merchant_connector_id, &app::dto::admin::MerchantConnectorId::set_merchant_connector_id, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "merchant_connector_id"; }
};

template<>
struct serde::Serializable<app::dto::admin::RevenueRecoveryMetadata>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"max_retry_count", &app::dto::admin::RevenueRecoveryMetadata::get_max_retry_count, &app::dto::admin::RevenueRecoveryMetadata::set_max_retry_count, serde::FieldOptions{}>{},
            serde::FieldDesc<"billing_connector_retry_threshold", &app::dto::admin::RevenueRecoveryMetadata::get_billing_connector_retry_threshold, &app::dto::admin::RevenueRecoveryMetadata::set_billing_connector_retry_threshold, serde::FieldOptions{}>{},
            serde::FieldDesc<"max_hybrid_cascading_retry_count", &app::dto::admin::RevenueRecoveryMetadata::get_max_hybrid_cascading_retry_count, &app::dto::admin::RevenueRecoveryMetadata::set_max_hybrid_cascading_retry_count, serde::FieldOptions{}>{},
            serde::FieldDesc<"grace_period_days", &app::dto::admin::RevenueRecoveryMetadata::get_grace_period_days, &app::dto::admin::RevenueRecoveryMetadata::set_grace_period_days, serde::FieldOptions{}>{},
            serde::FieldDesc<"enable_partial_authorization", &app::dto::admin::RevenueRecoveryMetadata::get_enable_partial_authorization, &app::dto::admin::RevenueRecoveryMetadata::set_enable_partial_authorization, serde::FieldOptions{}>{},
            serde::FieldDesc<"max_hard_decline_retries", &app::dto::admin::RevenueRecoveryMetadata::get_max_hard_decline_retries, &app::dto::admin::RevenueRecoveryMetadata::set_max_hard_decline_retries, serde::FieldOptions{}>{},
            serde::FieldDesc<"billing_account_reference", &app::dto::admin::RevenueRecoveryMetadata::get_billing_account_reference, &app::dto::admin::RevenueRecoveryMetadata::set_billing_account_reference, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "revenue_recovery_metadata"; }
};

template<>
struct serde::Serializable<app::dto::admin::MerchantConnectorAccountFeatureMetadata>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"revenue_recovery", &app::dto::admin::MerchantConnectorAccountFeatureMetadata::get_revenue_recovery, &app::dto::admin::MerchantConnectorAccountFeatureMetadata::set_revenue_recovery, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "merchant_connector_account_feature_metadata"; }
};

template<>
struct serde::Serializable<app::dto::admin::MerchantConnectorWebhookDetails>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"merchant_secret", &app::dto::admin::MerchantConnectorWebhookDetails::get_merchant_secret, &app::dto::admin::MerchantConnectorWebhookDetails::set_merchant_secret, serde::FieldOptions{}>{},
            serde::FieldDesc<"additional_secret", &app::dto::admin::MerchantConnectorWebhookDetails::get_additional_secret, &app::dto::admin::MerchantConnectorWebhookDetails::set_additional_secret, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "merchant_connector_webhook_details"; }
};

template<>
struct serde::Serializable<app::dto::admin::ConnectorWalletDetails>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"apple_pay_combined", &app::dto::admin::ConnectorWalletDetails::get_apple_pay_combined, &app::dto::admin::ConnectorWalletDetails::set_apple_pay_combined, serde::FieldOptions{}>{},
            serde::FieldDesc<"apple_pay", &app::dto::admin::ConnectorWalletDetails::get_apple_pay, &app::dto::admin::ConnectorWalletDetails::set_apple_pay, serde::FieldOptions{}>{},
            serde::FieldDesc<"amazon_pay", &app::dto::admin::ConnectorWalletDetails::get_amazon_pay, &app::dto::admin::ConnectorWalletDetails::set_amazon_pay, serde::FieldOptions{}>{},
            serde::FieldDesc<"samsung_pay", &app::dto::admin::ConnectorWalletDetails::get_samsung_pay, &app::dto::admin::ConnectorWalletDetails::set_samsung_pay, serde::FieldOptions{}>{},
            serde::FieldDesc<"paze", &app::dto::admin::ConnectorWalletDetails::get_paze, &app::dto::admin::ConnectorWalletDetails::set_paze, serde::FieldOptions{}>{},
            serde::FieldDesc<"google_pay", &app::dto::admin::ConnectorWalletDetails::get_google_pay, &app::dto::admin::ConnectorWalletDetails::set_google_pay, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "connector_wallet_details"; }
};

template<>
struct serde::Serializable<app::dto::admin::FrmPaymentMethodType>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"payment_method_type", &app::dto::admin::FrmPaymentMethodType::get_payment_method_type, &app::dto::admin::FrmPaymentMethodType::set_payment_method_type, serde::FieldOptions{}>{},
            serde::FieldDesc<"card_networks", &app::dto::admin::FrmPaymentMethodType::get_card_networks, &app::dto::admin::FrmPaymentMethodType::set_card_networks, serde::FieldOptions{}>{},
            serde::FieldDesc<"flow", &app::dto::admin::FrmPaymentMethodType::get_flow, &app::dto::admin::FrmPaymentMethodType::set_flow, serde::FieldOptions{}>{},
            serde::FieldDesc<"action", &app::dto::admin::FrmPaymentMethodType::get_action, &app::dto::admin::FrmPaymentMethodType::set_action, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "frm_payment_method_type"; }
};

template<>
struct serde::Serializable<app::dto::admin::FrmPaymentMethod>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"payment_method", &app::dto::admin::FrmPaymentMethod::get_payment_method, &app::dto::admin::FrmPaymentMethod::set_payment_method, serde::FieldOptions{}>{},
            serde::FieldDesc<"payment_method_types", &app::dto::admin::FrmPaymentMethod::get_payment_method_types, &app::dto::admin::FrmPaymentMethod::set_payment_method_types, serde::FieldOptions{}>{},
            serde::FieldDesc<"flow", &app::dto::admin::FrmPaymentMethod::get_flow, &app::dto::admin::FrmPaymentMethod::set_flow, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "frm_payment_method"; }
};

template<>
struct serde::Serializable<app::dto::admin::FrmConfigs>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"gateway", &app::dto::admin::FrmConfigs::get_gateway, &app::dto::admin::FrmConfigs::set_gateway, serde::FieldOptions{}>{},
            serde::FieldDesc<"payment_methods", &app::dto::admin::FrmConfigs::get_payment_methods, &app::dto::admin::FrmConfigs::set_payment_methods, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "frm_configs"; }
};

template<>
struct serde::Serializable<app::dto::admin::MerchantConnectorCreate>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"connector_type", &app::dto::admin::MerchantConnectorCreate::get_connector_type, &app::dto::admin::MerchantConnectorCreate::set_connector_type, serde::FieldOptions{}>{},
            serde::FieldDesc<"connector_name", &app::dto::admin::MerchantConnectorCreate::get_connector_name, &app::dto::admin::MerchantConnectorCreate::set_connector_name, serde::FieldOptions{}>{},
            serde::FieldDesc<"connector_label", &app::dto::admin::MerchantConnectorCreate::get_connector_label, &app::dto::admin::MerchantConnectorCreate::set_connector_label, serde::FieldOptions{}>{},
            serde::FieldDesc<"profile_id", &app::dto::admin::MerchantConnectorCreate::get_profile_id, &app::dto::admin::MerchantConnectorCreate::set_profile_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"connector_account_details", &app::dto::admin::MerchantConnectorCreate::get_connector_account_details, &app::dto::admin::MerchantConnectorCreate::set_connector_account_details, serde::FieldOptions{}>{},
            serde::FieldDesc<"payment_methods_enabled", &app::dto::admin::MerchantConnectorCreate::get_payment_methods_enabled, &app::dto::admin::MerchantConnectorCreate::set_payment_methods_enabled, serde::FieldOptions{}>{},
            serde::FieldDesc<"connector_webhook_details", &app::dto::admin::MerchantConnectorCreate::get_connector_webhook_details, &app::dto::admin::MerchantConnectorCreate::set_connector_webhook_details, serde::FieldOptions{}>{},
            serde::FieldDesc<"metadata", &app::dto::admin::MerchantConnectorCreate::get_metadata, &app::dto::admin::MerchantConnectorCreate::set_metadata, serde::FieldOptions{}>{},
            serde::FieldDesc<"disabled", &app::dto::admin::MerchantConnectorCreate::get_disabled, &app::dto::admin::MerchantConnectorCreate::set_disabled, serde::FieldOptions{}>{},
            serde::FieldDesc<"frm_configs", &app::dto::admin::MerchantConnectorCreate::get_frm_configs, &app::dto::admin::MerchantConnectorCreate::set_frm_configs, serde::FieldOptions{}>{},
            serde::FieldDesc<"pm_auth_config", &app::dto::admin::MerchantConnectorCreate::get_pm_auth_config, &app::dto::admin::MerchantConnectorCreate::set_pm_auth_config, serde::FieldOptions{}>{},
            serde::FieldDesc<"status", &app::dto::admin::MerchantConnectorCreate::get_status, &app::dto::admin::MerchantConnectorCreate::set_status, serde::FieldOptions{}>{},
            serde::FieldDesc<"additional_merchant_data", &app::dto::admin::MerchantConnectorCreate::get_additional_merchant_data, &app::dto::admin::MerchantConnectorCreate::set_additional_merchant_data, serde::FieldOptions{}>{},
            serde::FieldDesc<"connector_wallets_details", &app::dto::admin::MerchantConnectorCreate::get_connector_wallets_details, &app::dto::admin::MerchantConnectorCreate::set_connector_wallets_details, serde::FieldOptions{}>{},
            serde::FieldDesc<"feature_metadata", &app::dto::admin::MerchantConnectorCreate::get_feature_metadata, &app::dto::admin::MerchantConnectorCreate::set_feature_metadata, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "merchant_connector_create"; }
};

template<>
struct serde::Serializable<app::dto::admin::RevenueRecoveryConfig>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"max_retry_count", &app::dto::admin::RevenueRecoveryConfig::get_max_retry_count, &app::dto::admin::RevenueRecoveryConfig::set_max_retry_count, serde::FieldOptions{}>{},
            serde::FieldDesc<"billing_connector_retry_threshold", &app::dto::admin::RevenueRecoveryConfig::get_billing_connector_retry_threshold, &app::dto::admin::RevenueRecoveryConfig::set_billing_connector_retry_threshold, serde::FieldOptions{}>{},
            serde::FieldDesc<"max_hybrid_cascading_retry_count", &app::dto::admin::RevenueRecoveryConfig::get_max_hybrid_cascading_retry_count, &app::dto::admin::RevenueRecoveryConfig::set_max_hybrid_cascading_retry_count, serde::FieldOptions{}>{},
            serde::FieldDesc<"grace_period_days", &app::dto::admin::RevenueRecoveryConfig::get_grace_period_days, &app::dto::admin::RevenueRecoveryConfig::set_grace_period_days, serde::FieldOptions{}>{},
            serde::FieldDesc<"enable_partial_authorization", &app::dto::admin::RevenueRecoveryConfig::get_enable_partial_authorization, &app::dto::admin::RevenueRecoveryConfig::set_enable_partial_authorization, serde::FieldOptions{}>{},
            serde::FieldDesc<"max_hard_decline_retries", &app::dto::admin::RevenueRecoveryConfig::get_max_hard_decline_retries, &app::dto::admin::RevenueRecoveryConfig::set_max_hard_decline_retries, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "revenue_recovery_config"; }
};

template<>
struct serde::Serializable<app::dto::admin::MerchantConnectorInfo>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"connector_label", &app::dto::admin::MerchantConnectorInfo::get_connector_label, &app::dto::admin::MerchantConnectorInfo::set_connector_label, serde::FieldOptions{}>{},
            serde::FieldDesc<"merchant_connector_id", &app::dto::admin::MerchantConnectorInfo::get_merchant_connector_id, &app::dto::admin::MerchantConnectorInfo::set_merchant_connector_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"connector_type", &app::dto::admin::MerchantConnectorInfo::get_connector_type, &app::dto::admin::MerchantConnectorInfo::set_connector_type, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "merchant_connector_info"; }
};

template<>
struct serde::Serializable<app::dto::admin::MerchantConnectorResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"connector_type", &app::dto::admin::MerchantConnectorResponse::get_connector_type, &app::dto::admin::MerchantConnectorResponse::set_connector_type, serde::FieldOptions{}>{},
            serde::FieldDesc<"connector_name", &app::dto::admin::MerchantConnectorResponse::get_connector_name, &app::dto::admin::MerchantConnectorResponse::set_connector_name, serde::FieldOptions{}>{},
            serde::FieldDesc<"connector_label", &app::dto::admin::MerchantConnectorResponse::get_connector_label, &app::dto::admin::MerchantConnectorResponse::set_connector_label, serde::FieldOptions{}>{},
            serde::FieldDesc<"id", &app::dto::admin::MerchantConnectorResponse::get_id, &app::dto::admin::MerchantConnectorResponse::set_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"profile_id", &app::dto::admin::MerchantConnectorResponse::get_profile_id, &app::dto::admin::MerchantConnectorResponse::set_profile_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"connector_account_details", &app::dto::admin::MerchantConnectorResponse::get_connector_account_details, &app::dto::admin::MerchantConnectorResponse::set_connector_account_details, serde::FieldOptions{}>{},
            serde::FieldDesc<"payment_methods_enabled", &app::dto::admin::MerchantConnectorResponse::get_payment_methods_enabled, &app::dto::admin::MerchantConnectorResponse::set_payment_methods_enabled, serde::FieldOptions{}>{},
            serde::FieldDesc<"connector_webhook_details", &app::dto::admin::MerchantConnectorResponse::get_connector_webhook_details, &app::dto::admin::MerchantConnectorResponse::set_connector_webhook_details, serde::FieldOptions{}>{},
            serde::FieldDesc<"metadata", &app::dto::admin::MerchantConnectorResponse::get_metadata, &app::dto::admin::MerchantConnectorResponse::set_metadata, serde::FieldOptions{}>{},
            serde::FieldDesc<"disabled", &app::dto::admin::MerchantConnectorResponse::get_disabled, &app::dto::admin::MerchantConnectorResponse::set_disabled, serde::FieldOptions{}>{},
            serde::FieldDesc<"frm_configs", &app::dto::admin::MerchantConnectorResponse::get_frm_configs, &app::dto::admin::MerchantConnectorResponse::set_frm_configs, serde::FieldOptions{}>{},
            serde::FieldDesc<"applepay_verified_domains", &app::dto::admin::MerchantConnectorResponse::get_applepay_verified_domains, &app::dto::admin::MerchantConnectorResponse::set_applepay_verified_domains, serde::FieldOptions{}>{},
            serde::FieldDesc<"pm_auth_config", &app::dto::admin::MerchantConnectorResponse::get_pm_auth_config, &app::dto::admin::MerchantConnectorResponse::set_pm_auth_config, serde::FieldOptions{}>{},
            serde::FieldDesc<"status", &app::dto::admin::MerchantConnectorResponse::get_status, &app::dto::admin::MerchantConnectorResponse::set_status, serde::FieldOptions{}>{},
            serde::FieldDesc<"additional_merchant_data", &app::dto::admin::MerchantConnectorResponse::get_additional_merchant_data, &app::dto::admin::MerchantConnectorResponse::set_additional_merchant_data, serde::FieldOptions{}>{},
            serde::FieldDesc<"connector_wallets_details", &app::dto::admin::MerchantConnectorResponse::get_connector_wallets_details, &app::dto::admin::MerchantConnectorResponse::set_connector_wallets_details, serde::FieldOptions{}>{},
            serde::FieldDesc<"feature_metadata", &app::dto::admin::MerchantConnectorResponse::get_feature_metadata, &app::dto::admin::MerchantConnectorResponse::set_feature_metadata, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "merchant_connector_response"; }
};

template<>
struct serde::Serializable<app::dto::admin::PaymentMethodsEnabled>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"payment_method", &app::dto::admin::PaymentMethodsEnabled::get_payment_method, &app::dto::admin::PaymentMethodsEnabled::set_payment_method, serde::FieldOptions{}>{},
            serde::FieldDesc<"payment_method_types", &app::dto::admin::PaymentMethodsEnabled::get_payment_method_types, &app::dto::admin::PaymentMethodsEnabled::set_payment_method_types, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "payment_methods_enabled"; }
};

template<>
struct serde::Serializable<app::dto::admin::MerchantConnectorListResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"connector_type", &app::dto::admin::MerchantConnectorListResponse::get_connector_type, &app::dto::admin::MerchantConnectorListResponse::set_connector_type, serde::FieldOptions{}>{},
            serde::FieldDesc<"connector_name", &app::dto::admin::MerchantConnectorListResponse::get_connector_name, &app::dto::admin::MerchantConnectorListResponse::set_connector_name, serde::FieldOptions{}>{},
            serde::FieldDesc<"connector_label", &app::dto::admin::MerchantConnectorListResponse::get_connector_label, &app::dto::admin::MerchantConnectorListResponse::set_connector_label, serde::FieldOptions{}>{},
            serde::FieldDesc<"merchant_connector_id", &app::dto::admin::MerchantConnectorListResponse::get_merchant_connector_id, &app::dto::admin::MerchantConnectorListResponse::set_merchant_connector_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"profile_id", &app::dto::admin::MerchantConnectorListResponse::get_profile_id, &app::dto::admin::MerchantConnectorListResponse::set_profile_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"payment_methods_enabled", &app::dto::admin::MerchantConnectorListResponse::get_payment_methods_enabled, &app::dto::admin::MerchantConnectorListResponse::set_payment_methods_enabled, serde::FieldOptions{}>{},
            serde::FieldDesc<"test_mode", &app::dto::admin::MerchantConnectorListResponse::get_test_mode, &app::dto::admin::MerchantConnectorListResponse::set_test_mode, serde::FieldOptions{}>{},
            serde::FieldDesc<"disabled", &app::dto::admin::MerchantConnectorListResponse::get_disabled, &app::dto::admin::MerchantConnectorListResponse::set_disabled, serde::FieldOptions{}>{},
            serde::FieldDesc<"frm_configs", &app::dto::admin::MerchantConnectorListResponse::get_frm_configs, &app::dto::admin::MerchantConnectorListResponse::set_frm_configs, serde::FieldOptions{}>{},
            serde::FieldDesc<"business_country", &app::dto::admin::MerchantConnectorListResponse::get_business_country, &app::dto::admin::MerchantConnectorListResponse::set_business_country, serde::FieldOptions{}>{},
            serde::FieldDesc<"business_label", &app::dto::admin::MerchantConnectorListResponse::get_business_label, &app::dto::admin::MerchantConnectorListResponse::set_business_label, serde::FieldOptions{}>{},
            serde::FieldDesc<"business_sub_label", &app::dto::admin::MerchantConnectorListResponse::get_business_sub_label, &app::dto::admin::MerchantConnectorListResponse::set_business_sub_label, serde::FieldOptions{}>{},
            serde::FieldDesc<"applepay_verified_domains", &app::dto::admin::MerchantConnectorListResponse::get_applepay_verified_domains, &app::dto::admin::MerchantConnectorListResponse::set_applepay_verified_domains, serde::FieldOptions{}>{},
            serde::FieldDesc<"pm_auth_config", &app::dto::admin::MerchantConnectorListResponse::get_pm_auth_config, &app::dto::admin::MerchantConnectorListResponse::set_pm_auth_config, serde::FieldOptions{}>{},
            serde::FieldDesc<"status", &app::dto::admin::MerchantConnectorListResponse::get_status, &app::dto::admin::MerchantConnectorListResponse::set_status, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "merchant_connector_list_response"; }
};

template<>
struct serde::Serializable<app::dto::admin::MerchantConnectorUpdate>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"connector_type", &app::dto::admin::MerchantConnectorUpdate::get_connector_type, &app::dto::admin::MerchantConnectorUpdate::set_connector_type, serde::FieldOptions{}>{},
            serde::FieldDesc<"connector_label", &app::dto::admin::MerchantConnectorUpdate::get_connector_label, &app::dto::admin::MerchantConnectorUpdate::set_connector_label, serde::FieldOptions{}>{},
            serde::FieldDesc<"connector_account_details", &app::dto::admin::MerchantConnectorUpdate::get_connector_account_details, &app::dto::admin::MerchantConnectorUpdate::set_connector_account_details, serde::FieldOptions{}>{},
            serde::FieldDesc<"payment_methods_enabled", &app::dto::admin::MerchantConnectorUpdate::get_payment_methods_enabled, &app::dto::admin::MerchantConnectorUpdate::set_payment_methods_enabled, serde::FieldOptions{}>{},
            serde::FieldDesc<"connector_webhook_details", &app::dto::admin::MerchantConnectorUpdate::get_connector_webhook_details, &app::dto::admin::MerchantConnectorUpdate::set_connector_webhook_details, serde::FieldOptions{}>{},
            serde::FieldDesc<"metadata", &app::dto::admin::MerchantConnectorUpdate::get_metadata, &app::dto::admin::MerchantConnectorUpdate::set_metadata, serde::FieldOptions{}>{},
            serde::FieldDesc<"test_mode", &app::dto::admin::MerchantConnectorUpdate::get_test_mode, &app::dto::admin::MerchantConnectorUpdate::set_test_mode, serde::FieldOptions{}>{},
            serde::FieldDesc<"disabled", &app::dto::admin::MerchantConnectorUpdate::get_disabled, &app::dto::admin::MerchantConnectorUpdate::set_disabled, serde::FieldOptions{}>{},
            serde::FieldDesc<"frm_configs", &app::dto::admin::MerchantConnectorUpdate::get_frm_configs, &app::dto::admin::MerchantConnectorUpdate::set_frm_configs, serde::FieldOptions{}>{},
            serde::FieldDesc<"pm_auth_config", &app::dto::admin::MerchantConnectorUpdate::get_pm_auth_config, &app::dto::admin::MerchantConnectorUpdate::set_pm_auth_config, serde::FieldOptions{}>{},
            serde::FieldDesc<"status", &app::dto::admin::MerchantConnectorUpdate::get_status, &app::dto::admin::MerchantConnectorUpdate::set_status, serde::FieldOptions{}>{},
            serde::FieldDesc<"additional_merchant_data", &app::dto::admin::MerchantConnectorUpdate::get_additional_merchant_data, &app::dto::admin::MerchantConnectorUpdate::set_additional_merchant_data, serde::FieldOptions{}>{},
            serde::FieldDesc<"connector_wallets_details", &app::dto::admin::MerchantConnectorUpdate::get_connector_wallets_details, &app::dto::admin::MerchantConnectorUpdate::set_connector_wallets_details, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "merchant_connector_update"; }
};

template<>
struct serde::Serializable<app::dto::admin::MerchantConnectorDeleteResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"merchant_id", &app::dto::admin::MerchantConnectorDeleteResponse::get_merchant_id, &app::dto::admin::MerchantConnectorDeleteResponse::set_merchant_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"merchant_connector_id", &app::dto::admin::MerchantConnectorDeleteResponse::get_merchant_connector_id, &app::dto::admin::MerchantConnectorDeleteResponse::set_merchant_connector_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"deleted", &app::dto::admin::MerchantConnectorDeleteResponse::get_deleted, &app::dto::admin::MerchantConnectorDeleteResponse::set_deleted, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "merchant_connector_delete_response"; }
};

template<>
struct serde::Serializable<app::dto::admin::ToggleKVResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"merchant_id", &app::dto::admin::ToggleKVResponse::get_merchant_id, &app::dto::admin::ToggleKVResponse::set_merchant_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"kv_enabled", &app::dto::admin::ToggleKVResponse::get_kv_enabled, &app::dto::admin::ToggleKVResponse::set_kv_enabled, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "toggle_kv_response"; }
};

template<>
struct serde::Serializable<app::dto::admin::MerchantKeyTransferRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"from", &app::dto::admin::MerchantKeyTransferRequest::get_from, &app::dto::admin::MerchantKeyTransferRequest::set_from, serde::FieldOptions{}>{},
            serde::FieldDesc<"limit", &app::dto::admin::MerchantKeyTransferRequest::get_limit, &app::dto::admin::MerchantKeyTransferRequest::set_limit, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "merchant_key_transfer_request"; }
};

template<>
struct serde::Serializable<app::dto::admin::TransferKeyResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"total_transferred", &app::dto::admin::TransferKeyResponse::get_total_transferred, &app::dto::admin::TransferKeyResponse::set_total_transferred, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "transfer_key_response"; }
};

template<>
struct serde::Serializable<app::dto::admin::ToggleKVRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"merchant_id", &app::dto::admin::ToggleKVRequest::get_merchant_id, &app::dto::admin::ToggleKVRequest::set_merchant_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"kv_enabled", &app::dto::admin::ToggleKVRequest::get_kv_enabled, &app::dto::admin::ToggleKVRequest::set_kv_enabled, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "toggle_kv_request"; }
};

template<>
struct serde::Serializable<app::dto::admin::ToggleAllKVRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"kv_enabled", &app::dto::admin::ToggleAllKVRequest::get_kv_enabled, &app::dto::admin::ToggleAllKVRequest::set_kv_enabled, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "toggle_all_kv_request"; }
};

template<>
struct serde::Serializable<app::dto::admin::ToggleAllKVResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"total_updated", &app::dto::admin::ToggleAllKVResponse::get_total_updated, &app::dto::admin::ToggleAllKVResponse::set_total_updated, serde::FieldOptions{}>{},
            serde::FieldDesc<"kv_enabled", &app::dto::admin::ToggleAllKVResponse::get_kv_enabled, &app::dto::admin::ToggleAllKVResponse::set_kv_enabled, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "toggle_all_kv_response"; }
};

template<>
struct serde::Serializable<app::dto::admin::MerchantConnectorDetailsWrap>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"creds_identifier", &app::dto::admin::MerchantConnectorDetailsWrap::get_creds_identifier, &app::dto::admin::MerchantConnectorDetailsWrap::set_creds_identifier, serde::FieldOptions{}>{},
            serde::FieldDesc<"encoded_data", &app::dto::admin::MerchantConnectorDetailsWrap::get_encoded_data, &app::dto::admin::MerchantConnectorDetailsWrap::set_encoded_data, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "merchant_connector_details_wrap"; }
};

template<>
struct serde::Serializable<app::dto::admin::MerchantConnectorDetails>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"connector_account_details", &app::dto::admin::MerchantConnectorDetails::get_connector_account_details, &app::dto::admin::MerchantConnectorDetails::set_connector_account_details, serde::FieldOptions{}>{},
            serde::FieldDesc<"metadata", &app::dto::admin::MerchantConnectorDetails::get_metadata, &app::dto::admin::MerchantConnectorDetails::set_metadata, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "merchant_connector_details"; }
};

template<>
struct serde::Serializable<app::dto::admin::BusinessPayoutLinkConfig>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"config", &app::dto::admin::BusinessPayoutLinkConfig::get_config, &app::dto::admin::BusinessPayoutLinkConfig::set_config, serde::FieldOptions{}>{},
            serde::FieldDesc<"form_layout", &app::dto::admin::BusinessPayoutLinkConfig::get_form_layout, &app::dto::admin::BusinessPayoutLinkConfig::set_form_layout, serde::FieldOptions{}>{},
            serde::FieldDesc<"payout_test_mode", &app::dto::admin::BusinessPayoutLinkConfig::get_payout_test_mode, &app::dto::admin::BusinessPayoutLinkConfig::set_payout_test_mode, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "business_payout_link_config"; }
};

template<>
struct serde::Serializable<app::dto::admin::TransactionDetailsUiConfiguration>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"position", &app::dto::admin::TransactionDetailsUiConfiguration::get_position, &app::dto::admin::TransactionDetailsUiConfiguration::set_position, serde::FieldOptions{}>{},
            serde::FieldDesc<"is_key_bold", &app::dto::admin::TransactionDetailsUiConfiguration::get_is_key_bold, &app::dto::admin::TransactionDetailsUiConfiguration::set_is_key_bold, serde::FieldOptions{}>{},
            serde::FieldDesc<"is_value_bold", &app::dto::admin::TransactionDetailsUiConfiguration::get_is_value_bold, &app::dto::admin::TransactionDetailsUiConfiguration::set_is_value_bold, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "transaction_details_ui_configuration"; }
};

template<>
struct serde::Serializable<app::dto::admin::PaymentLinkTransactionDetails>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"key", &app::dto::admin::PaymentLinkTransactionDetails::get_key, &app::dto::admin::PaymentLinkTransactionDetails::set_key, serde::FieldOptions{}>{},
            serde::FieldDesc<"value", &app::dto::admin::PaymentLinkTransactionDetails::get_value, &app::dto::admin::PaymentLinkTransactionDetails::set_value, serde::FieldOptions{}>{},
            serde::FieldDesc<"ui_configuration", &app::dto::admin::PaymentLinkTransactionDetails::get_ui_configuration, &app::dto::admin::PaymentLinkTransactionDetails::set_ui_configuration, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "payment_link_transaction_details"; }
};

template<>
struct serde::Serializable<app::dto::admin::PaymentLinkBackgroundImageConfig>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"url", &app::dto::admin::PaymentLinkBackgroundImageConfig::get_url, &app::dto::admin::PaymentLinkBackgroundImageConfig::set_url, serde::FieldOptions{}>{},
            serde::FieldDesc<"position", &app::dto::admin::PaymentLinkBackgroundImageConfig::get_position, &app::dto::admin::PaymentLinkBackgroundImageConfig::set_position, serde::FieldOptions{}>{},
            serde::FieldDesc<"size", &app::dto::admin::PaymentLinkBackgroundImageConfig::get_size, &app::dto::admin::PaymentLinkBackgroundImageConfig::set_size, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "payment_link_background_image_config"; }
};

template<>
struct serde::Serializable<app::dto::admin::PaymentLinkConfigRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"theme", &app::dto::admin::PaymentLinkConfigRequest::get_theme, &app::dto::admin::PaymentLinkConfigRequest::set_theme, serde::FieldOptions{}>{},
            serde::FieldDesc<"logo", &app::dto::admin::PaymentLinkConfigRequest::get_logo, &app::dto::admin::PaymentLinkConfigRequest::set_logo, serde::FieldOptions{}>{},
            serde::FieldDesc<"seller_name", &app::dto::admin::PaymentLinkConfigRequest::get_seller_name, &app::dto::admin::PaymentLinkConfigRequest::set_seller_name, serde::FieldOptions{}>{},
            serde::FieldDesc<"sdk_layout", &app::dto::admin::PaymentLinkConfigRequest::get_sdk_layout, &app::dto::admin::PaymentLinkConfigRequest::set_sdk_layout, serde::FieldOptions{}>{},
            serde::FieldDesc<"display_sdk_only", &app::dto::admin::PaymentLinkConfigRequest::get_display_sdk_only, &app::dto::admin::PaymentLinkConfigRequest::set_display_sdk_only, serde::FieldOptions{}>{},
            serde::FieldDesc<"enabled_saved_payment_method", &app::dto::admin::PaymentLinkConfigRequest::get_enabled_saved_payment_method, &app::dto::admin::PaymentLinkConfigRequest::set_enabled_saved_payment_method, serde::FieldOptions{}>{},
            serde::FieldDesc<"hide_card_nickname_field", &app::dto::admin::PaymentLinkConfigRequest::get_hide_card_nickname_field, &app::dto::admin::PaymentLinkConfigRequest::set_hide_card_nickname_field, serde::FieldOptions{}>{},
            serde::FieldDesc<"show_card_form_by_default", &app::dto::admin::PaymentLinkConfigRequest::get_show_card_form_by_default, &app::dto::admin::PaymentLinkConfigRequest::set_show_card_form_by_default, serde::FieldOptions{}>{},
            serde::FieldDesc<"transaction_details", &app::dto::admin::PaymentLinkConfigRequest::get_transaction_details, &app::dto::admin::PaymentLinkConfigRequest::set_transaction_details, serde::FieldOptions{}>{},
            serde::FieldDesc<"background_image", &app::dto::admin::PaymentLinkConfigRequest::get_background_image, &app::dto::admin::PaymentLinkConfigRequest::set_background_image, serde::FieldOptions{}>{},
            serde::FieldDesc<"details_layout", &app::dto::admin::PaymentLinkConfigRequest::get_details_layout, &app::dto::admin::PaymentLinkConfigRequest::set_details_layout, serde::FieldOptions{}>{},
            serde::FieldDesc<"payment_button_text", &app::dto::admin::PaymentLinkConfigRequest::get_payment_button_text, &app::dto::admin::PaymentLinkConfigRequest::set_payment_button_text, serde::FieldOptions{}>{},
            serde::FieldDesc<"custom_message_for_card_terms", &app::dto::admin::PaymentLinkConfigRequest::get_custom_message_for_card_terms, &app::dto::admin::PaymentLinkConfigRequest::set_custom_message_for_card_terms, serde::FieldOptions{}>{},
            serde::FieldDesc<"custom_message_for_payment_method_types", &app::dto::admin::PaymentLinkConfigRequest::get_custom_message_for_payment_method_types, &app::dto::admin::PaymentLinkConfigRequest::set_custom_message_for_payment_method_types, serde::FieldOptions{}>{},
            serde::FieldDesc<"payment_button_colour", &app::dto::admin::PaymentLinkConfigRequest::get_payment_button_colour, &app::dto::admin::PaymentLinkConfigRequest::set_payment_button_colour, serde::FieldOptions{}>{},
            serde::FieldDesc<"skip_status_screen", &app::dto::admin::PaymentLinkConfigRequest::get_skip_status_screen, &app::dto::admin::PaymentLinkConfigRequest::set_skip_status_screen, serde::FieldOptions{}>{},
            serde::FieldDesc<"payment_button_text_colour", &app::dto::admin::PaymentLinkConfigRequest::get_payment_button_text_colour, &app::dto::admin::PaymentLinkConfigRequest::set_payment_button_text_colour, serde::FieldOptions{}>{},
            serde::FieldDesc<"background_colour", &app::dto::admin::PaymentLinkConfigRequest::get_background_colour, &app::dto::admin::PaymentLinkConfigRequest::set_background_colour, serde::FieldOptions{}>{},
            serde::FieldDesc<"sdk_ui_rules", &app::dto::admin::PaymentLinkConfigRequest::get_sdk_ui_rules, &app::dto::admin::PaymentLinkConfigRequest::set_sdk_ui_rules, serde::FieldOptions{}>{},
            serde::FieldDesc<"payment_link_ui_rules", &app::dto::admin::PaymentLinkConfigRequest::get_payment_link_ui_rules, &app::dto::admin::PaymentLinkConfigRequest::set_payment_link_ui_rules, serde::FieldOptions{}>{},
            serde::FieldDesc<"enable_button_only_on_form_ready", &app::dto::admin::PaymentLinkConfigRequest::get_enable_button_only_on_form_ready, &app::dto::admin::PaymentLinkConfigRequest::set_enable_button_only_on_form_ready, serde::FieldOptions{}>{},
            serde::FieldDesc<"payment_form_header_text", &app::dto::admin::PaymentLinkConfigRequest::get_payment_form_header_text, &app::dto::admin::PaymentLinkConfigRequest::set_payment_form_header_text, serde::FieldOptions{}>{},
            serde::FieldDesc<"payment_form_label_type", &app::dto::admin::PaymentLinkConfigRequest::get_payment_form_label_type, &app::dto::admin::PaymentLinkConfigRequest::set_payment_form_label_type, serde::FieldOptions{}>{},
            serde::FieldDesc<"show_card_terms", &app::dto::admin::PaymentLinkConfigRequest::get_show_card_terms, &app::dto::admin::PaymentLinkConfigRequest::set_show_card_terms, serde::FieldOptions{}>{},
            serde::FieldDesc<"is_setup_mandate_flow", &app::dto::admin::PaymentLinkConfigRequest::get_is_setup_mandate_flow, &app::dto::admin::PaymentLinkConfigRequest::set_is_setup_mandate_flow, serde::FieldOptions{}>{},
            serde::FieldDesc<"color_icon_card_cvc_error", &app::dto::admin::PaymentLinkConfigRequest::get_color_icon_card_cvc_error, &app::dto::admin::PaymentLinkConfigRequest::set_color_icon_card_cvc_error, serde::FieldOptions{}>{},
            serde::FieldDesc<"show_merchant_name", &app::dto::admin::PaymentLinkConfigRequest::get_show_merchant_name, &app::dto::admin::PaymentLinkConfigRequest::set_show_merchant_name, serde::FieldOptions{}>{},
            serde::FieldDesc<"payment_methods_separator_text", &app::dto::admin::PaymentLinkConfigRequest::get_payment_methods_separator_text, &app::dto::admin::PaymentLinkConfigRequest::set_payment_methods_separator_text, serde::FieldOptions{}>{},
            serde::FieldDesc<"redirect_delay_seconds", &app::dto::admin::PaymentLinkConfigRequest::get_redirect_delay_seconds, &app::dto::admin::PaymentLinkConfigRequest::set_redirect_delay_seconds, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "payment_link_config_request"; }
};

template<>
struct serde::Serializable<app::dto::admin::BusinessPaymentLinkConfig>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"domain_name", &app::dto::admin::BusinessPaymentLinkConfig::get_domain_name, &app::dto::admin::BusinessPaymentLinkConfig::set_domain_name, serde::FieldOptions{}>{},
            serde::FieldDesc<"default_config", &app::dto::admin::BusinessPaymentLinkConfig::get_default_config, &app::dto::admin::BusinessPaymentLinkConfig::set_default_config, serde::FieldOptions{}>{},
            serde::FieldDesc<"business_specific_configs", &app::dto::admin::BusinessPaymentLinkConfig::get_business_specific_configs, &app::dto::admin::BusinessPaymentLinkConfig::set_business_specific_configs, serde::FieldOptions{}>{},
            serde::FieldDesc<"allowed_domains", &app::dto::admin::BusinessPaymentLinkConfig::get_allowed_domains, &app::dto::admin::BusinessPaymentLinkConfig::set_allowed_domains, serde::FieldOptions{}>{},
            serde::FieldDesc<"branding_visibility", &app::dto::admin::BusinessPaymentLinkConfig::get_branding_visibility, &app::dto::admin::BusinessPaymentLinkConfig::set_branding_visibility, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "business_payment_link_config"; }
};

template<>
struct serde::Serializable<app::dto::admin::ProfileCreate>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"profile_name", &app::dto::admin::ProfileCreate::get_profile_name, &app::dto::admin::ProfileCreate::set_profile_name, serde::FieldOptions{}>{},
            serde::FieldDesc<"return_url", &app::dto::admin::ProfileCreate::get_return_url, &app::dto::admin::ProfileCreate::set_return_url, serde::FieldOptions{}>{},
            serde::FieldDesc<"enable_payment_response_hash", &app::dto::admin::ProfileCreate::get_enable_payment_response_hash, &app::dto::admin::ProfileCreate::set_enable_payment_response_hash, serde::FieldOptions{}>{},
            serde::FieldDesc<"payment_response_hash_key", &app::dto::admin::ProfileCreate::get_payment_response_hash_key, &app::dto::admin::ProfileCreate::set_payment_response_hash_key, serde::FieldOptions{}>{},
            serde::FieldDesc<"redirect_to_merchant_with_http_post", &app::dto::admin::ProfileCreate::get_redirect_to_merchant_with_http_post, &app::dto::admin::ProfileCreate::set_redirect_to_merchant_with_http_post, serde::FieldOptions{}>{},
            serde::FieldDesc<"webhook_details", &app::dto::admin::ProfileCreate::get_webhook_details, &app::dto::admin::ProfileCreate::set_webhook_details, serde::FieldOptions{}>{},
            serde::FieldDesc<"metadata", &app::dto::admin::ProfileCreate::get_metadata, &app::dto::admin::ProfileCreate::set_metadata, serde::FieldOptions{}>{},
            serde::FieldDesc<"routing_algorithm", &app::dto::admin::ProfileCreate::get_routing_algorithm, &app::dto::admin::ProfileCreate::set_routing_algorithm, serde::FieldOptions{}>{},
            serde::FieldDesc<"intent_fulfillment_time", &app::dto::admin::ProfileCreate::get_intent_fulfillment_time, &app::dto::admin::ProfileCreate::set_intent_fulfillment_time, serde::FieldOptions{}>{},
            serde::FieldDesc<"frm_routing_algorithm", &app::dto::admin::ProfileCreate::get_frm_routing_algorithm, &app::dto::admin::ProfileCreate::set_frm_routing_algorithm, serde::FieldOptions{}>{},
            serde::FieldDesc<"payout_routing_algorithm", &app::dto::admin::ProfileCreate::get_payout_routing_algorithm, &app::dto::admin::ProfileCreate::set_payout_routing_algorithm, serde::FieldOptions{}>{},
            serde::FieldDesc<"applepay_verified_domains", &app::dto::admin::ProfileCreate::get_applepay_verified_domains, &app::dto::admin::ProfileCreate::set_applepay_verified_domains, serde::FieldOptions{}>{},
            serde::FieldDesc<"session_expiry", &app::dto::admin::ProfileCreate::get_session_expiry, &app::dto::admin::ProfileCreate::set_session_expiry, serde::FieldOptions{}>{},
            serde::FieldDesc<"payment_link_config", &app::dto::admin::ProfileCreate::get_payment_link_config, &app::dto::admin::ProfileCreate::set_payment_link_config, serde::FieldOptions{}>{},
            serde::FieldDesc<"authentication_connector_details", &app::dto::admin::ProfileCreate::get_authentication_connector_details, &app::dto::admin::ProfileCreate::set_authentication_connector_details, serde::FieldOptions{}>{},
            serde::FieldDesc<"use_billing_as_payment_method_billing", &app::dto::admin::ProfileCreate::get_use_billing_as_payment_method_billing, &app::dto::admin::ProfileCreate::set_use_billing_as_payment_method_billing, serde::FieldOptions{}>{},
            serde::FieldDesc<"collect_shipping_details_from_wallet_connector", &app::dto::admin::ProfileCreate::get_collect_shipping_details_from_wallet_connector, &app::dto::admin::ProfileCreate::set_collect_shipping_details_from_wallet_connector, serde::FieldOptions{}>{},
            serde::FieldDesc<"collect_billing_details_from_wallet_connector", &app::dto::admin::ProfileCreate::get_collect_billing_details_from_wallet_connector, &app::dto::admin::ProfileCreate::set_collect_billing_details_from_wallet_connector, serde::FieldOptions{}>{},
            serde::FieldDesc<"always_collect_shipping_details_from_wallet_connector", &app::dto::admin::ProfileCreate::get_always_collect_shipping_details_from_wallet_connector, &app::dto::admin::ProfileCreate::set_always_collect_shipping_details_from_wallet_connector, serde::FieldOptions{}>{},
            serde::FieldDesc<"always_collect_billing_details_from_wallet_connector", &app::dto::admin::ProfileCreate::get_always_collect_billing_details_from_wallet_connector, &app::dto::admin::ProfileCreate::set_always_collect_billing_details_from_wallet_connector, serde::FieldOptions{}>{},
            serde::FieldDesc<"is_connector_agnostic_mit_enabled", &app::dto::admin::ProfileCreate::get_is_connector_agnostic_mit_enabled, &app::dto::admin::ProfileCreate::set_is_connector_agnostic_mit_enabled, serde::FieldOptions{}>{},
            serde::FieldDesc<"payout_link_config", &app::dto::admin::ProfileCreate::get_payout_link_config, &app::dto::admin::ProfileCreate::set_payout_link_config, serde::FieldOptions{}>{},
            serde::FieldDesc<"outgoing_webhook_custom_http_headers", &app::dto::admin::ProfileCreate::get_outgoing_webhook_custom_http_headers, &app::dto::admin::ProfileCreate::set_outgoing_webhook_custom_http_headers, serde::FieldOptions{}>{},
            serde::FieldDesc<"tax_connector_id", &app::dto::admin::ProfileCreate::get_tax_connector_id, &app::dto::admin::ProfileCreate::set_tax_connector_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"is_tax_connector_enabled", &app::dto::admin::ProfileCreate::get_is_tax_connector_enabled, &app::dto::admin::ProfileCreate::set_is_tax_connector_enabled, serde::FieldOptions{}>{},
            serde::FieldDesc<"is_network_tokenization_enabled", &app::dto::admin::ProfileCreate::get_is_network_tokenization_enabled, &app::dto::admin::ProfileCreate::set_is_network_tokenization_enabled, serde::FieldOptions{}>{},
            serde::FieldDesc<"is_auto_retries_enabled", &app::dto::admin::ProfileCreate::get_is_auto_retries_enabled, &app::dto::admin::ProfileCreate::set_is_auto_retries_enabled, serde::FieldOptions{}>{},
            serde::FieldDesc<"max_auto_retries_enabled", &app::dto::admin::ProfileCreate::get_max_auto_retries_enabled, &app::dto::admin::ProfileCreate::set_max_auto_retries_enabled, serde::FieldOptions{}>{},
            serde::FieldDesc<"always_request_extended_authorization", &app::dto::admin::ProfileCreate::get_always_request_extended_authorization, &app::dto::admin::ProfileCreate::set_always_request_extended_authorization, serde::FieldOptions{}>{},
            serde::FieldDesc<"is_click_to_pay_enabled", &app::dto::admin::ProfileCreate::get_is_click_to_pay_enabled, &app::dto::admin::ProfileCreate::set_is_click_to_pay_enabled, serde::FieldOptions{}>{},
            serde::FieldDesc<"authentication_product_ids", &app::dto::admin::ProfileCreate::get_authentication_product_ids, &app::dto::admin::ProfileCreate::set_authentication_product_ids, serde::FieldOptions{}>{},
            serde::FieldDesc<"card_testing_guard_config", &app::dto::admin::ProfileCreate::get_card_testing_guard_config, &app::dto::admin::ProfileCreate::set_card_testing_guard_config, serde::FieldOptions{}>{},
            serde::FieldDesc<"is_clear_pan_retries_enabled", &app::dto::admin::ProfileCreate::get_is_clear_pan_retries_enabled, &app::dto::admin::ProfileCreate::set_is_clear_pan_retries_enabled, serde::FieldOptions{}>{},
            serde::FieldDesc<"force_3ds_challenge", &app::dto::admin::ProfileCreate::get_force_3ds_challenge, &app::dto::admin::ProfileCreate::set_force_3ds_challenge, serde::FieldOptions{}>{},
            serde::FieldDesc<"is_debit_routing_enabled", &app::dto::admin::ProfileCreate::get_is_debit_routing_enabled, &app::dto::admin::ProfileCreate::set_is_debit_routing_enabled, serde::FieldOptions{}>{},
            serde::FieldDesc<"merchant_business_country", &app::dto::admin::ProfileCreate::get_merchant_business_country, &app::dto::admin::ProfileCreate::set_merchant_business_country, serde::FieldOptions{}>{},
            serde::FieldDesc<"is_iframe_redirection_enabled", &app::dto::admin::ProfileCreate::get_is_iframe_redirection_enabled, &app::dto::admin::ProfileCreate::set_is_iframe_redirection_enabled, serde::FieldOptions{}>{},
            serde::FieldDesc<"is_pre_network_tokenization_enabled", &app::dto::admin::ProfileCreate::get_is_pre_network_tokenization_enabled, &app::dto::admin::ProfileCreate::set_is_pre_network_tokenization_enabled, serde::FieldOptions{}>{},
            serde::FieldDesc<"merchant_category_code", &app::dto::admin::ProfileCreate::get_merchant_category_code, &app::dto::admin::ProfileCreate::set_merchant_category_code, serde::FieldOptions{}>{},
            serde::FieldDesc<"merchant_country_code", &app::dto::admin::ProfileCreate::get_merchant_country_code, &app::dto::admin::ProfileCreate::set_merchant_country_code, serde::FieldOptions{}>{},
            serde::FieldDesc<"dispute_polling_interval", &app::dto::admin::ProfileCreate::get_dispute_polling_interval, &app::dto::admin::ProfileCreate::set_dispute_polling_interval, serde::FieldOptions{}>{},
            serde::FieldDesc<"is_manual_retry_enabled", &app::dto::admin::ProfileCreate::get_is_manual_retry_enabled, &app::dto::admin::ProfileCreate::set_is_manual_retry_enabled, serde::FieldOptions{}>{},
            serde::FieldDesc<"auto_fallback_capture_method", &app::dto::admin::ProfileCreate::get_auto_fallback_capture_method, &app::dto::admin::ProfileCreate::set_auto_fallback_capture_method, serde::FieldOptions{}>{},
            serde::FieldDesc<"always_enable_overcapture", &app::dto::admin::ProfileCreate::get_always_enable_overcapture, &app::dto::admin::ProfileCreate::set_always_enable_overcapture, serde::FieldOptions{}>{},
            serde::FieldDesc<"is_external_vault_enabled", &app::dto::admin::ProfileCreate::get_is_external_vault_enabled, &app::dto::admin::ProfileCreate::set_is_external_vault_enabled, serde::FieldOptions{}>{},
            serde::FieldDesc<"external_vault_connector_details", &app::dto::admin::ProfileCreate::get_external_vault_connector_details, &app::dto::admin::ProfileCreate::set_external_vault_connector_details, serde::FieldOptions{}>{},
            serde::FieldDesc<"billing_processor_id", &app::dto::admin::ProfileCreate::get_billing_processor_id, &app::dto::admin::ProfileCreate::set_billing_processor_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"surcharge_connector_details", &app::dto::admin::ProfileCreate::get_surcharge_connector_details, &app::dto::admin::ProfileCreate::set_surcharge_connector_details, serde::FieldOptions{}>{},
            serde::FieldDesc<"is_l2_l3_enabled", &app::dto::admin::ProfileCreate::get_is_l2_l3_enabled, &app::dto::admin::ProfileCreate::set_is_l2_l3_enabled, serde::FieldOptions{}>{},
            serde::FieldDesc<"network_tokenization_credentials", &app::dto::admin::ProfileCreate::get_network_tokenization_credentials, &app::dto::admin::ProfileCreate::set_network_tokenization_credentials, serde::FieldOptions{}>{},
            serde::FieldDesc<"payment_method_blocking", &app::dto::admin::ProfileCreate::get_payment_method_blocking, &app::dto::admin::ProfileCreate::set_payment_method_blocking, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "profile_create"; }
};

template<>
struct serde::Serializable<app::dto::admin::OrderFulfillmentTime>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"value", &app::dto::admin::OrderFulfillmentTime::get_value, &app::dto::admin::OrderFulfillmentTime::set_value, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "order_fulfillment_time"; }
};

template<>
struct serde::Serializable<app::dto::admin::MaskedHeaders>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"value", &app::dto::admin::MaskedHeaders::get_value, &app::dto::admin::MaskedHeaders::set_value, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "masked_headers"; }
};

template<>
struct serde::Serializable<app::dto::admin::TtlForExtendedCardInfo>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"value", &app::dto::admin::TtlForExtendedCardInfo::get_value, &app::dto::admin::TtlForExtendedCardInfo::set_value, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "ttl_for_extended_card_info"; }
};

template<>
struct serde::Serializable<app::dto::admin::ExtendedCardInfoConfig>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"public_key", &app::dto::admin::ExtendedCardInfoConfig::get_public_key, &app::dto::admin::ExtendedCardInfoConfig::set_public_key, serde::FieldOptions{}>{},
            serde::FieldDesc<"ttl_in_secs", &app::dto::admin::ExtendedCardInfoConfig::get_ttl_in_secs, &app::dto::admin::ExtendedCardInfoConfig::set_ttl_in_secs, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "extended_card_info_config"; }
};

template<>
struct serde::Serializable<app::dto::admin::ProfileResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"merchant_id", &app::dto::admin::ProfileResponse::get_merchant_id, &app::dto::admin::ProfileResponse::set_merchant_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"profile_id", &app::dto::admin::ProfileResponse::get_profile_id, &app::dto::admin::ProfileResponse::set_profile_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"profile_name", &app::dto::admin::ProfileResponse::get_profile_name, &app::dto::admin::ProfileResponse::set_profile_name, serde::FieldOptions{}>{},
            serde::FieldDesc<"return_url", &app::dto::admin::ProfileResponse::get_return_url, &app::dto::admin::ProfileResponse::set_return_url, serde::FieldOptions{}>{},
            serde::FieldDesc<"enable_payment_response_hash", &app::dto::admin::ProfileResponse::get_enable_payment_response_hash, &app::dto::admin::ProfileResponse::set_enable_payment_response_hash, serde::FieldOptions{}>{},
            serde::FieldDesc<"payment_response_hash_key", &app::dto::admin::ProfileResponse::get_payment_response_hash_key, &app::dto::admin::ProfileResponse::set_payment_response_hash_key, serde::FieldOptions{}>{},
            serde::FieldDesc<"redirect_to_merchant_with_http_post", &app::dto::admin::ProfileResponse::get_redirect_to_merchant_with_http_post, &app::dto::admin::ProfileResponse::set_redirect_to_merchant_with_http_post, serde::FieldOptions{}>{},
            serde::FieldDesc<"webhook_details", &app::dto::admin::ProfileResponse::get_webhook_details, &app::dto::admin::ProfileResponse::set_webhook_details, serde::FieldOptions{}>{},
            serde::FieldDesc<"metadata", &app::dto::admin::ProfileResponse::get_metadata, &app::dto::admin::ProfileResponse::set_metadata, serde::FieldOptions{}>{},
            serde::FieldDesc<"routing_algorithm", &app::dto::admin::ProfileResponse::get_routing_algorithm, &app::dto::admin::ProfileResponse::set_routing_algorithm, serde::FieldOptions{}>{},
            serde::FieldDesc<"intent_fulfillment_time", &app::dto::admin::ProfileResponse::get_intent_fulfillment_time, &app::dto::admin::ProfileResponse::set_intent_fulfillment_time, serde::FieldOptions{}>{},
            serde::FieldDesc<"frm_routing_algorithm", &app::dto::admin::ProfileResponse::get_frm_routing_algorithm, &app::dto::admin::ProfileResponse::set_frm_routing_algorithm, serde::FieldOptions{}>{},
            serde::FieldDesc<"payout_routing_algorithm", &app::dto::admin::ProfileResponse::get_payout_routing_algorithm, &app::dto::admin::ProfileResponse::set_payout_routing_algorithm, serde::FieldOptions{}>{},
            serde::FieldDesc<"applepay_verified_domains", &app::dto::admin::ProfileResponse::get_applepay_verified_domains, &app::dto::admin::ProfileResponse::set_applepay_verified_domains, serde::FieldOptions{}>{},
            serde::FieldDesc<"session_expiry", &app::dto::admin::ProfileResponse::get_session_expiry, &app::dto::admin::ProfileResponse::set_session_expiry, serde::FieldOptions{}>{},
            serde::FieldDesc<"payment_link_config", &app::dto::admin::ProfileResponse::get_payment_link_config, &app::dto::admin::ProfileResponse::set_payment_link_config, serde::FieldOptions{}>{},
            serde::FieldDesc<"authentication_connector_details", &app::dto::admin::ProfileResponse::get_authentication_connector_details, &app::dto::admin::ProfileResponse::set_authentication_connector_details, serde::FieldOptions{}>{},
            serde::FieldDesc<"use_billing_as_payment_method_billing", &app::dto::admin::ProfileResponse::get_use_billing_as_payment_method_billing, &app::dto::admin::ProfileResponse::set_use_billing_as_payment_method_billing, serde::FieldOptions{}>{},
            serde::FieldDesc<"extended_card_info_config", &app::dto::admin::ProfileResponse::get_extended_card_info_config, &app::dto::admin::ProfileResponse::set_extended_card_info_config, serde::FieldOptions{}>{},
            serde::FieldDesc<"collect_shipping_details_from_wallet_connector", &app::dto::admin::ProfileResponse::get_collect_shipping_details_from_wallet_connector, &app::dto::admin::ProfileResponse::set_collect_shipping_details_from_wallet_connector, serde::FieldOptions{}>{},
            serde::FieldDesc<"collect_billing_details_from_wallet_connector", &app::dto::admin::ProfileResponse::get_collect_billing_details_from_wallet_connector, &app::dto::admin::ProfileResponse::set_collect_billing_details_from_wallet_connector, serde::FieldOptions{}>{},
            serde::FieldDesc<"always_collect_shipping_details_from_wallet_connector", &app::dto::admin::ProfileResponse::get_always_collect_shipping_details_from_wallet_connector, &app::dto::admin::ProfileResponse::set_always_collect_shipping_details_from_wallet_connector, serde::FieldOptions{}>{},
            serde::FieldDesc<"always_collect_billing_details_from_wallet_connector", &app::dto::admin::ProfileResponse::get_always_collect_billing_details_from_wallet_connector, &app::dto::admin::ProfileResponse::set_always_collect_billing_details_from_wallet_connector, serde::FieldOptions{}>{},
            serde::FieldDesc<"is_connector_agnostic_mit_enabled", &app::dto::admin::ProfileResponse::get_is_connector_agnostic_mit_enabled, &app::dto::admin::ProfileResponse::set_is_connector_agnostic_mit_enabled, serde::FieldOptions{}>{},
            serde::FieldDesc<"payout_link_config", &app::dto::admin::ProfileResponse::get_payout_link_config, &app::dto::admin::ProfileResponse::set_payout_link_config, serde::FieldOptions{}>{},
            serde::FieldDesc<"outgoing_webhook_custom_http_headers", &app::dto::admin::ProfileResponse::get_outgoing_webhook_custom_http_headers, &app::dto::admin::ProfileResponse::set_outgoing_webhook_custom_http_headers, serde::FieldOptions{}>{},
            serde::FieldDesc<"tax_connector_id", &app::dto::admin::ProfileResponse::get_tax_connector_id, &app::dto::admin::ProfileResponse::set_tax_connector_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"is_tax_connector_enabled", &app::dto::admin::ProfileResponse::get_is_tax_connector_enabled, &app::dto::admin::ProfileResponse::set_is_tax_connector_enabled, serde::FieldOptions{}>{},
            serde::FieldDesc<"is_network_tokenization_enabled", &app::dto::admin::ProfileResponse::get_is_network_tokenization_enabled, &app::dto::admin::ProfileResponse::set_is_network_tokenization_enabled, serde::FieldOptions{}>{},
            serde::FieldDesc<"is_auto_retries_enabled", &app::dto::admin::ProfileResponse::get_is_auto_retries_enabled, &app::dto::admin::ProfileResponse::set_is_auto_retries_enabled, serde::FieldOptions{}>{},
            serde::FieldDesc<"max_auto_retries_enabled", &app::dto::admin::ProfileResponse::get_max_auto_retries_enabled, &app::dto::admin::ProfileResponse::set_max_auto_retries_enabled, serde::FieldOptions{}>{},
            serde::FieldDesc<"always_request_extended_authorization", &app::dto::admin::ProfileResponse::get_always_request_extended_authorization, &app::dto::admin::ProfileResponse::set_always_request_extended_authorization, serde::FieldOptions{}>{},
            serde::FieldDesc<"is_click_to_pay_enabled", &app::dto::admin::ProfileResponse::get_is_click_to_pay_enabled, &app::dto::admin::ProfileResponse::set_is_click_to_pay_enabled, serde::FieldOptions{}>{},
            serde::FieldDesc<"authentication_product_ids", &app::dto::admin::ProfileResponse::get_authentication_product_ids, &app::dto::admin::ProfileResponse::set_authentication_product_ids, serde::FieldOptions{}>{},
            serde::FieldDesc<"card_testing_guard_config", &app::dto::admin::ProfileResponse::get_card_testing_guard_config, &app::dto::admin::ProfileResponse::set_card_testing_guard_config, serde::FieldOptions{}>{},
            serde::FieldDesc<"is_clear_pan_retries_enabled", &app::dto::admin::ProfileResponse::get_is_clear_pan_retries_enabled, &app::dto::admin::ProfileResponse::set_is_clear_pan_retries_enabled, serde::FieldOptions{}>{},
            serde::FieldDesc<"force_3ds_challenge", &app::dto::admin::ProfileResponse::get_force_3ds_challenge, &app::dto::admin::ProfileResponse::set_force_3ds_challenge, serde::FieldOptions{}>{},
            serde::FieldDesc<"is_debit_routing_enabled", &app::dto::admin::ProfileResponse::get_is_debit_routing_enabled, &app::dto::admin::ProfileResponse::set_is_debit_routing_enabled, serde::FieldOptions{}>{},
            serde::FieldDesc<"merchant_business_country", &app::dto::admin::ProfileResponse::get_merchant_business_country, &app::dto::admin::ProfileResponse::set_merchant_business_country, serde::FieldOptions{}>{},
            serde::FieldDesc<"is_pre_network_tokenization_enabled", &app::dto::admin::ProfileResponse::get_is_pre_network_tokenization_enabled, &app::dto::admin::ProfileResponse::set_is_pre_network_tokenization_enabled, serde::FieldOptions{}>{},
            serde::FieldDesc<"acquirer_configs", &app::dto::admin::ProfileResponse::get_acquirer_configs, &app::dto::admin::ProfileResponse::set_acquirer_configs, serde::FieldOptions{}>{},
            serde::FieldDesc<"acquirer_config_bucket", &app::dto::admin::ProfileResponse::get_acquirer_config_bucket, &app::dto::admin::ProfileResponse::set_acquirer_config_bucket, serde::FieldOptions{}>{},
            serde::FieldDesc<"is_iframe_redirection_enabled", &app::dto::admin::ProfileResponse::get_is_iframe_redirection_enabled, &app::dto::admin::ProfileResponse::set_is_iframe_redirection_enabled, serde::FieldOptions{}>{},
            serde::FieldDesc<"merchant_category_code", &app::dto::admin::ProfileResponse::get_merchant_category_code, &app::dto::admin::ProfileResponse::set_merchant_category_code, serde::FieldOptions{}>{},
            serde::FieldDesc<"merchant_country_code", &app::dto::admin::ProfileResponse::get_merchant_country_code, &app::dto::admin::ProfileResponse::set_merchant_country_code, serde::FieldOptions{}>{},
            serde::FieldDesc<"dispute_polling_interval", &app::dto::admin::ProfileResponse::get_dispute_polling_interval, &app::dto::admin::ProfileResponse::set_dispute_polling_interval, serde::FieldOptions{}>{},
            serde::FieldDesc<"is_manual_retry_enabled", &app::dto::admin::ProfileResponse::get_is_manual_retry_enabled, &app::dto::admin::ProfileResponse::set_is_manual_retry_enabled, serde::FieldOptions{}>{},
            serde::FieldDesc<"auto_fallback_capture_method", &app::dto::admin::ProfileResponse::get_auto_fallback_capture_method, &app::dto::admin::ProfileResponse::set_auto_fallback_capture_method, serde::FieldOptions{}>{},
            serde::FieldDesc<"always_enable_overcapture", &app::dto::admin::ProfileResponse::get_always_enable_overcapture, &app::dto::admin::ProfileResponse::set_always_enable_overcapture, serde::FieldOptions{}>{},
            serde::FieldDesc<"is_external_vault_enabled", &app::dto::admin::ProfileResponse::get_is_external_vault_enabled, &app::dto::admin::ProfileResponse::set_is_external_vault_enabled, serde::FieldOptions{}>{},
            serde::FieldDesc<"external_vault_connector_details", &app::dto::admin::ProfileResponse::get_external_vault_connector_details, &app::dto::admin::ProfileResponse::set_external_vault_connector_details, serde::FieldOptions{}>{},
            serde::FieldDesc<"billing_processor_id", &app::dto::admin::ProfileResponse::get_billing_processor_id, &app::dto::admin::ProfileResponse::set_billing_processor_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"surcharge_connector_details", &app::dto::admin::ProfileResponse::get_surcharge_connector_details, &app::dto::admin::ProfileResponse::set_surcharge_connector_details, serde::FieldOptions{}>{},
            serde::FieldDesc<"is_l2_l3_enabled", &app::dto::admin::ProfileResponse::get_is_l2_l3_enabled, &app::dto::admin::ProfileResponse::set_is_l2_l3_enabled, serde::FieldOptions{}>{},
            serde::FieldDesc<"network_tokenization_credentials", &app::dto::admin::ProfileResponse::get_network_tokenization_credentials, &app::dto::admin::ProfileResponse::set_network_tokenization_credentials, serde::FieldOptions{}>{},
            serde::FieldDesc<"payment_method_blocking", &app::dto::admin::ProfileResponse::get_payment_method_blocking, &app::dto::admin::ProfileResponse::set_payment_method_blocking, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "profile_response"; }
};

template<>
struct serde::Serializable<app::dto::admin::ProfileUpdate>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"profile_name", &app::dto::admin::ProfileUpdate::get_profile_name, &app::dto::admin::ProfileUpdate::set_profile_name, serde::FieldOptions{}>{},
            serde::FieldDesc<"return_url", &app::dto::admin::ProfileUpdate::get_return_url, &app::dto::admin::ProfileUpdate::set_return_url, serde::FieldOptions{}>{},
            serde::FieldDesc<"enable_payment_response_hash", &app::dto::admin::ProfileUpdate::get_enable_payment_response_hash, &app::dto::admin::ProfileUpdate::set_enable_payment_response_hash, serde::FieldOptions{}>{},
            serde::FieldDesc<"payment_response_hash_key", &app::dto::admin::ProfileUpdate::get_payment_response_hash_key, &app::dto::admin::ProfileUpdate::set_payment_response_hash_key, serde::FieldOptions{}>{},
            serde::FieldDesc<"redirect_to_merchant_with_http_post", &app::dto::admin::ProfileUpdate::get_redirect_to_merchant_with_http_post, &app::dto::admin::ProfileUpdate::set_redirect_to_merchant_with_http_post, serde::FieldOptions{}>{},
            serde::FieldDesc<"webhook_details", &app::dto::admin::ProfileUpdate::get_webhook_details, &app::dto::admin::ProfileUpdate::set_webhook_details, serde::FieldOptions{}>{},
            serde::FieldDesc<"metadata", &app::dto::admin::ProfileUpdate::get_metadata, &app::dto::admin::ProfileUpdate::set_metadata, serde::FieldOptions{}>{},
            serde::FieldDesc<"routing_algorithm", &app::dto::admin::ProfileUpdate::get_routing_algorithm, &app::dto::admin::ProfileUpdate::set_routing_algorithm, serde::FieldOptions{}>{},
            serde::FieldDesc<"intent_fulfillment_time", &app::dto::admin::ProfileUpdate::get_intent_fulfillment_time, &app::dto::admin::ProfileUpdate::set_intent_fulfillment_time, serde::FieldOptions{}>{},
            serde::FieldDesc<"frm_routing_algorithm", &app::dto::admin::ProfileUpdate::get_frm_routing_algorithm, &app::dto::admin::ProfileUpdate::set_frm_routing_algorithm, serde::FieldOptions{}>{},
            serde::FieldDesc<"payout_routing_algorithm", &app::dto::admin::ProfileUpdate::get_payout_routing_algorithm, &app::dto::admin::ProfileUpdate::set_payout_routing_algorithm, serde::FieldOptions{}>{},
            serde::FieldDesc<"applepay_verified_domains", &app::dto::admin::ProfileUpdate::get_applepay_verified_domains, &app::dto::admin::ProfileUpdate::set_applepay_verified_domains, serde::FieldOptions{}>{},
            serde::FieldDesc<"session_expiry", &app::dto::admin::ProfileUpdate::get_session_expiry, &app::dto::admin::ProfileUpdate::set_session_expiry, serde::FieldOptions{}>{},
            serde::FieldDesc<"payment_link_config", &app::dto::admin::ProfileUpdate::get_payment_link_config, &app::dto::admin::ProfileUpdate::set_payment_link_config, serde::FieldOptions{}>{},
            serde::FieldDesc<"authentication_connector_details", &app::dto::admin::ProfileUpdate::get_authentication_connector_details, &app::dto::admin::ProfileUpdate::set_authentication_connector_details, serde::FieldOptions{}>{},
            serde::FieldDesc<"extended_card_info_config", &app::dto::admin::ProfileUpdate::get_extended_card_info_config, &app::dto::admin::ProfileUpdate::set_extended_card_info_config, serde::FieldOptions{}>{},
            serde::FieldDesc<"use_billing_as_payment_method_billing", &app::dto::admin::ProfileUpdate::get_use_billing_as_payment_method_billing, &app::dto::admin::ProfileUpdate::set_use_billing_as_payment_method_billing, serde::FieldOptions{}>{},
            serde::FieldDesc<"collect_shipping_details_from_wallet_connector", &app::dto::admin::ProfileUpdate::get_collect_shipping_details_from_wallet_connector, &app::dto::admin::ProfileUpdate::set_collect_shipping_details_from_wallet_connector, serde::FieldOptions{}>{},
            serde::FieldDesc<"collect_billing_details_from_wallet_connector", &app::dto::admin::ProfileUpdate::get_collect_billing_details_from_wallet_connector, &app::dto::admin::ProfileUpdate::set_collect_billing_details_from_wallet_connector, serde::FieldOptions{}>{},
            serde::FieldDesc<"always_collect_shipping_details_from_wallet_connector", &app::dto::admin::ProfileUpdate::get_always_collect_shipping_details_from_wallet_connector, &app::dto::admin::ProfileUpdate::set_always_collect_shipping_details_from_wallet_connector, serde::FieldOptions{}>{},
            serde::FieldDesc<"always_collect_billing_details_from_wallet_connector", &app::dto::admin::ProfileUpdate::get_always_collect_billing_details_from_wallet_connector, &app::dto::admin::ProfileUpdate::set_always_collect_billing_details_from_wallet_connector, serde::FieldOptions{}>{},
            serde::FieldDesc<"always_request_extended_authorization", &app::dto::admin::ProfileUpdate::get_always_request_extended_authorization, &app::dto::admin::ProfileUpdate::set_always_request_extended_authorization, serde::FieldOptions{}>{},
            serde::FieldDesc<"is_connector_agnostic_mit_enabled", &app::dto::admin::ProfileUpdate::get_is_connector_agnostic_mit_enabled, &app::dto::admin::ProfileUpdate::set_is_connector_agnostic_mit_enabled, serde::FieldOptions{}>{},
            serde::FieldDesc<"payout_link_config", &app::dto::admin::ProfileUpdate::get_payout_link_config, &app::dto::admin::ProfileUpdate::set_payout_link_config, serde::FieldOptions{}>{},
            serde::FieldDesc<"outgoing_webhook_custom_http_headers", &app::dto::admin::ProfileUpdate::get_outgoing_webhook_custom_http_headers, &app::dto::admin::ProfileUpdate::set_outgoing_webhook_custom_http_headers, serde::FieldOptions{}>{},
            serde::FieldDesc<"tax_connector_id", &app::dto::admin::ProfileUpdate::get_tax_connector_id, &app::dto::admin::ProfileUpdate::set_tax_connector_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"is_tax_connector_enabled", &app::dto::admin::ProfileUpdate::get_is_tax_connector_enabled, &app::dto::admin::ProfileUpdate::set_is_tax_connector_enabled, serde::FieldOptions{}>{},
            serde::FieldDesc<"dynamic_routing_algorithm", &app::dto::admin::ProfileUpdate::get_dynamic_routing_algorithm, &app::dto::admin::ProfileUpdate::set_dynamic_routing_algorithm, serde::FieldOptions{}>{},
            serde::FieldDesc<"is_network_tokenization_enabled", &app::dto::admin::ProfileUpdate::get_is_network_tokenization_enabled, &app::dto::admin::ProfileUpdate::set_is_network_tokenization_enabled, serde::FieldOptions{}>{},
            serde::FieldDesc<"is_auto_retries_enabled", &app::dto::admin::ProfileUpdate::get_is_auto_retries_enabled, &app::dto::admin::ProfileUpdate::set_is_auto_retries_enabled, serde::FieldOptions{}>{},
            serde::FieldDesc<"max_auto_retries_enabled", &app::dto::admin::ProfileUpdate::get_max_auto_retries_enabled, &app::dto::admin::ProfileUpdate::set_max_auto_retries_enabled, serde::FieldOptions{}>{},
            serde::FieldDesc<"is_click_to_pay_enabled", &app::dto::admin::ProfileUpdate::get_is_click_to_pay_enabled, &app::dto::admin::ProfileUpdate::set_is_click_to_pay_enabled, serde::FieldOptions{}>{},
            serde::FieldDesc<"authentication_product_ids", &app::dto::admin::ProfileUpdate::get_authentication_product_ids, &app::dto::admin::ProfileUpdate::set_authentication_product_ids, serde::FieldOptions{}>{},
            serde::FieldDesc<"card_testing_guard_config", &app::dto::admin::ProfileUpdate::get_card_testing_guard_config, &app::dto::admin::ProfileUpdate::set_card_testing_guard_config, serde::FieldOptions{}>{},
            serde::FieldDesc<"is_clear_pan_retries_enabled", &app::dto::admin::ProfileUpdate::get_is_clear_pan_retries_enabled, &app::dto::admin::ProfileUpdate::set_is_clear_pan_retries_enabled, serde::FieldOptions{}>{},
            serde::FieldDesc<"force_3ds_challenge", &app::dto::admin::ProfileUpdate::get_force_3ds_challenge, &app::dto::admin::ProfileUpdate::set_force_3ds_challenge, serde::FieldOptions{}>{},
            serde::FieldDesc<"is_debit_routing_enabled", &app::dto::admin::ProfileUpdate::get_is_debit_routing_enabled, &app::dto::admin::ProfileUpdate::set_is_debit_routing_enabled, serde::FieldOptions{}>{},
            serde::FieldDesc<"merchant_business_country", &app::dto::admin::ProfileUpdate::get_merchant_business_country, &app::dto::admin::ProfileUpdate::set_merchant_business_country, serde::FieldOptions{}>{},
            serde::FieldDesc<"is_iframe_redirection_enabled", &app::dto::admin::ProfileUpdate::get_is_iframe_redirection_enabled, &app::dto::admin::ProfileUpdate::set_is_iframe_redirection_enabled, serde::FieldOptions{}>{},
            serde::FieldDesc<"is_pre_network_tokenization_enabled", &app::dto::admin::ProfileUpdate::get_is_pre_network_tokenization_enabled, &app::dto::admin::ProfileUpdate::set_is_pre_network_tokenization_enabled, serde::FieldOptions{}>{},
            serde::FieldDesc<"merchant_category_code", &app::dto::admin::ProfileUpdate::get_merchant_category_code, &app::dto::admin::ProfileUpdate::set_merchant_category_code, serde::FieldOptions{}>{},
            serde::FieldDesc<"merchant_country_code", &app::dto::admin::ProfileUpdate::get_merchant_country_code, &app::dto::admin::ProfileUpdate::set_merchant_country_code, serde::FieldOptions{}>{},
            serde::FieldDesc<"dispute_polling_interval", &app::dto::admin::ProfileUpdate::get_dispute_polling_interval, &app::dto::admin::ProfileUpdate::set_dispute_polling_interval, serde::FieldOptions{}>{},
            serde::FieldDesc<"is_manual_retry_enabled", &app::dto::admin::ProfileUpdate::get_is_manual_retry_enabled, &app::dto::admin::ProfileUpdate::set_is_manual_retry_enabled, serde::FieldOptions{}>{},
            serde::FieldDesc<"auto_fallback_capture_method", &app::dto::admin::ProfileUpdate::get_auto_fallback_capture_method, &app::dto::admin::ProfileUpdate::set_auto_fallback_capture_method, serde::FieldOptions{}>{},
            serde::FieldDesc<"always_enable_overcapture", &app::dto::admin::ProfileUpdate::get_always_enable_overcapture, &app::dto::admin::ProfileUpdate::set_always_enable_overcapture, serde::FieldOptions{}>{},
            serde::FieldDesc<"is_external_vault_enabled", &app::dto::admin::ProfileUpdate::get_is_external_vault_enabled, &app::dto::admin::ProfileUpdate::set_is_external_vault_enabled, serde::FieldOptions{}>{},
            serde::FieldDesc<"external_vault_connector_details", &app::dto::admin::ProfileUpdate::get_external_vault_connector_details, &app::dto::admin::ProfileUpdate::set_external_vault_connector_details, serde::FieldOptions{}>{},
            serde::FieldDesc<"billing_processor_id", &app::dto::admin::ProfileUpdate::get_billing_processor_id, &app::dto::admin::ProfileUpdate::set_billing_processor_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"surcharge_connector_details", &app::dto::admin::ProfileUpdate::get_surcharge_connector_details, &app::dto::admin::ProfileUpdate::set_surcharge_connector_details, serde::FieldOptions{}>{},
            serde::FieldDesc<"is_l2_l3_enabled", &app::dto::admin::ProfileUpdate::get_is_l2_l3_enabled, &app::dto::admin::ProfileUpdate::set_is_l2_l3_enabled, serde::FieldOptions{}>{},
            serde::FieldDesc<"network_tokenization_credentials", &app::dto::admin::ProfileUpdate::get_network_tokenization_credentials, &app::dto::admin::ProfileUpdate::set_network_tokenization_credentials, serde::FieldOptions{}>{},
            serde::FieldDesc<"payment_method_blocking", &app::dto::admin::ProfileUpdate::get_payment_method_blocking, &app::dto::admin::ProfileUpdate::set_payment_method_blocking, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "profile_update"; }
};

template<>
struct serde::Serializable<app::dto::admin::PaymentLinkConfig>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"theme", &app::dto::admin::PaymentLinkConfig::get_theme, &app::dto::admin::PaymentLinkConfig::set_theme, serde::FieldOptions{}>{},
            serde::FieldDesc<"logo", &app::dto::admin::PaymentLinkConfig::get_logo, &app::dto::admin::PaymentLinkConfig::set_logo, serde::FieldOptions{}>{},
            serde::FieldDesc<"seller_name", &app::dto::admin::PaymentLinkConfig::get_seller_name, &app::dto::admin::PaymentLinkConfig::set_seller_name, serde::FieldOptions{}>{},
            serde::FieldDesc<"sdk_layout", &app::dto::admin::PaymentLinkConfig::get_sdk_layout, &app::dto::admin::PaymentLinkConfig::set_sdk_layout, serde::FieldOptions{}>{},
            serde::FieldDesc<"display_sdk_only", &app::dto::admin::PaymentLinkConfig::get_display_sdk_only, &app::dto::admin::PaymentLinkConfig::set_display_sdk_only, serde::FieldOptions{}>{},
            serde::FieldDesc<"enabled_saved_payment_method", &app::dto::admin::PaymentLinkConfig::get_enabled_saved_payment_method, &app::dto::admin::PaymentLinkConfig::set_enabled_saved_payment_method, serde::FieldOptions{}>{},
            serde::FieldDesc<"hide_card_nickname_field", &app::dto::admin::PaymentLinkConfig::get_hide_card_nickname_field, &app::dto::admin::PaymentLinkConfig::set_hide_card_nickname_field, serde::FieldOptions{}>{},
            serde::FieldDesc<"show_card_form_by_default", &app::dto::admin::PaymentLinkConfig::get_show_card_form_by_default, &app::dto::admin::PaymentLinkConfig::set_show_card_form_by_default, serde::FieldOptions{}>{},
            serde::FieldDesc<"allowed_domains", &app::dto::admin::PaymentLinkConfig::get_allowed_domains, &app::dto::admin::PaymentLinkConfig::set_allowed_domains, serde::FieldOptions{}>{},
            serde::FieldDesc<"transaction_details", &app::dto::admin::PaymentLinkConfig::get_transaction_details, &app::dto::admin::PaymentLinkConfig::set_transaction_details, serde::FieldOptions{}>{},
            serde::FieldDesc<"background_image", &app::dto::admin::PaymentLinkConfig::get_background_image, &app::dto::admin::PaymentLinkConfig::set_background_image, serde::FieldOptions{}>{},
            serde::FieldDesc<"details_layout", &app::dto::admin::PaymentLinkConfig::get_details_layout, &app::dto::admin::PaymentLinkConfig::set_details_layout, serde::FieldOptions{}>{},
            serde::FieldDesc<"branding_visibility", &app::dto::admin::PaymentLinkConfig::get_branding_visibility, &app::dto::admin::PaymentLinkConfig::set_branding_visibility, serde::FieldOptions{}>{},
            serde::FieldDesc<"payment_button_text", &app::dto::admin::PaymentLinkConfig::get_payment_button_text, &app::dto::admin::PaymentLinkConfig::set_payment_button_text, serde::FieldOptions{}>{},
            serde::FieldDesc<"custom_message_for_card_terms", &app::dto::admin::PaymentLinkConfig::get_custom_message_for_card_terms, &app::dto::admin::PaymentLinkConfig::set_custom_message_for_card_terms, serde::FieldOptions{}>{},
            serde::FieldDesc<"custom_message_for_payment_method_types", &app::dto::admin::PaymentLinkConfig::get_custom_message_for_payment_method_types, &app::dto::admin::PaymentLinkConfig::set_custom_message_for_payment_method_types, serde::FieldOptions{}>{},
            serde::FieldDesc<"payment_button_colour", &app::dto::admin::PaymentLinkConfig::get_payment_button_colour, &app::dto::admin::PaymentLinkConfig::set_payment_button_colour, serde::FieldOptions{}>{},
            serde::FieldDesc<"skip_status_screen", &app::dto::admin::PaymentLinkConfig::get_skip_status_screen, &app::dto::admin::PaymentLinkConfig::set_skip_status_screen, serde::FieldOptions{}>{},
            serde::FieldDesc<"payment_button_text_colour", &app::dto::admin::PaymentLinkConfig::get_payment_button_text_colour, &app::dto::admin::PaymentLinkConfig::set_payment_button_text_colour, serde::FieldOptions{}>{},
            serde::FieldDesc<"background_colour", &app::dto::admin::PaymentLinkConfig::get_background_colour, &app::dto::admin::PaymentLinkConfig::set_background_colour, serde::FieldOptions{}>{},
            serde::FieldDesc<"sdk_ui_rules", &app::dto::admin::PaymentLinkConfig::get_sdk_ui_rules, &app::dto::admin::PaymentLinkConfig::set_sdk_ui_rules, serde::FieldOptions{}>{},
            serde::FieldDesc<"payment_link_ui_rules", &app::dto::admin::PaymentLinkConfig::get_payment_link_ui_rules, &app::dto::admin::PaymentLinkConfig::set_payment_link_ui_rules, serde::FieldOptions{}>{},
            serde::FieldDesc<"enable_button_only_on_form_ready", &app::dto::admin::PaymentLinkConfig::get_enable_button_only_on_form_ready, &app::dto::admin::PaymentLinkConfig::set_enable_button_only_on_form_ready, serde::FieldOptions{}>{},
            serde::FieldDesc<"payment_form_header_text", &app::dto::admin::PaymentLinkConfig::get_payment_form_header_text, &app::dto::admin::PaymentLinkConfig::set_payment_form_header_text, serde::FieldOptions{}>{},
            serde::FieldDesc<"payment_form_label_type", &app::dto::admin::PaymentLinkConfig::get_payment_form_label_type, &app::dto::admin::PaymentLinkConfig::set_payment_form_label_type, serde::FieldOptions{}>{},
            serde::FieldDesc<"show_card_terms", &app::dto::admin::PaymentLinkConfig::get_show_card_terms, &app::dto::admin::PaymentLinkConfig::set_show_card_terms, serde::FieldOptions{}>{},
            serde::FieldDesc<"is_setup_mandate_flow", &app::dto::admin::PaymentLinkConfig::get_is_setup_mandate_flow, &app::dto::admin::PaymentLinkConfig::set_is_setup_mandate_flow, serde::FieldOptions{}>{},
            serde::FieldDesc<"color_icon_card_cvc_error", &app::dto::admin::PaymentLinkConfig::get_color_icon_card_cvc_error, &app::dto::admin::PaymentLinkConfig::set_color_icon_card_cvc_error, serde::FieldOptions{}>{},
            serde::FieldDesc<"show_merchant_name", &app::dto::admin::PaymentLinkConfig::get_show_merchant_name, &app::dto::admin::PaymentLinkConfig::set_show_merchant_name, serde::FieldOptions{}>{},
            serde::FieldDesc<"payment_methods_separator_text", &app::dto::admin::PaymentLinkConfig::get_payment_methods_separator_text, &app::dto::admin::PaymentLinkConfig::set_payment_methods_separator_text, serde::FieldOptions{}>{},
            serde::FieldDesc<"redirect_delay_seconds", &app::dto::admin::PaymentLinkConfig::get_redirect_delay_seconds, &app::dto::admin::PaymentLinkConfig::set_redirect_delay_seconds, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "payment_link_config"; }
};

template<>
struct serde::Serializable<app::dto::admin::ExtendedCardInfoChoice>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"enabled", &app::dto::admin::ExtendedCardInfoChoice::get_enabled, &app::dto::admin::ExtendedCardInfoChoice::set_enabled, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "extended_card_info_choice"; }
};

template<>
struct serde::Serializable<app::dto::admin::ConnectorAgnosticMitChoice>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"enabled", &app::dto::admin::ConnectorAgnosticMitChoice::get_enabled, &app::dto::admin::ConnectorAgnosticMitChoice::set_enabled, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "connector_agnostic_mit_choice"; }
};

template<>
struct serde::Serializable<app::dto::admin::MCACGraphData>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"connector_name", &app::dto::admin::MCACGraphData::get_connector_name, &app::dto::admin::MCACGraphData::set_connector_name, serde::FieldOptions{}>{},
            serde::FieldDesc<"payment_methods_enabled", &app::dto::admin::MCACGraphData::get_payment_methods_enabled, &app::dto::admin::MCACGraphData::set_payment_methods_enabled, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "mcac_graph_data"; }
};

#ifdef CONGELADO_TEST
namespace app::dto::admin::tests {
using namespace boost::ut;
suite<"app_dto_admin"> app_dto_admin_suite = [] {
    "reflects converted types"_test = [] {
        expect(std::tuple_size_v<decltype(serde::Serializable<app::dto::admin::MerchantAccountListRequest>::fields())> > 0);
    };
};
}
#endif
