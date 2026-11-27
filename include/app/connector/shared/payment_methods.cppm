module;
export module app_connector:model_shared_payment_methods;

import std;
import core_serde;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace app::model::payment_methods {

enum class BankAccountAccessCreds
{
    AccessToken
};

class BankCodeResponseValue : public serde::Serializer<BankCodeResponseValue>
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

enum class BankDebitDataForClient
{
    AchBankDebit
};

enum class BankDebitDetail
{
    Ach
};

enum class BankDebitDetailUpdate
{
    Ach
};

enum class BankDebitDetailsPaymentMethod
{
    AchBankDebit
};

class BankDebitTypes : public serde::Serializer<BankDebitTypes>
{
public:
    void set_eligible_connectors(std::vector<std::string> value)
    {
        eligible_connectors = std::move(value);
    }

    [[nodiscard]] const std::vector<std::string>& get_eligible_connectors() const noexcept
    {
        return eligible_connectors;
    }

    std::vector<std::string> eligible_connectors{};
};

class BankNamesValue : public serde::Serializer<BankNamesValue>
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

enum class BankRedirectData
{
    OpenBanking
};

enum class BankRedirectDetail
{
    BancontactCard
};

class BankTransferDataValue : public serde::Serializer<BankTransferDataValue>
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

class BankTransferTypes : public serde::Serializer<BankTransferTypes>
{
public:
    void set_eligible_connectors(std::vector<std::string> value)
    {
        eligible_connectors = std::move(value);
    }

    [[nodiscard]] const std::vector<std::string>& get_eligible_connectors() const noexcept
    {
        return eligible_connectors;
    }

    std::vector<std::string> eligible_connectors{};
};

class BankTransferValue : public serde::Serializer<BankTransferValue>
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

class CardNetworkValue : public serde::Serializer<CardNetworkValue>
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

class CardNumberValue : public serde::Serializer<CardNumberValue>
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

class CardSegmentTypeValue : public serde::Serializer<CardSegmentTypeValue>
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

class ConnectorTokenStatusValue : public serde::Serializer<ConnectorTokenStatusValue>
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

class ConnectorValue : public serde::Serializer<ConnectorValue>
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

class CountryAlpha2Value : public serde::Serializer<CountryAlpha2Value>
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

class CurrencyValue : public serde::Serializer<CurrencyValue>
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

class CustomerIdValue : public serde::Serializer<CustomerIdValue>
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

class EmailValue : public serde::Serializer<EmailValue>
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

class FundingSourceValue : public serde::Serializer<FundingSourceValue>
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

class GPayPredecryptDataValue : public serde::Serializer<GPayPredecryptDataValue>
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

class MerchantConnectorAccountIdValue : public serde::Serializer<MerchantConnectorAccountIdValue>
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

class MinorUnitValue : public serde::Serializer<MinorUnitValue>
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

class PaymentMethodDataRequestValue : public serde::Serializer<PaymentMethodDataRequestValue>
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

class PaymentMethodTypeValue : public serde::Serializer<PaymentMethodTypeValue>
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

class PaymentMethodValue : public serde::Serializer<PaymentMethodValue>
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

class RegulatedNameValue : public serde::Serializer<RegulatedNameValue>
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

class SecretSerdeValueValue : public serde::Serializer<SecretSerdeValueValue>
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

class TokenizationTypeValue : public serde::Serializer<TokenizationTypeValue>
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

class WalletDataValue : public serde::Serializer<WalletDataValue>
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

enum class WalletPaymentMethodData
{
    ApplePay,
    ApplePayDecrypted,
    GooglePay,
    GooglePayDecrypted,
    PayPal
};

enum class WalletPaymentMethodDataForClient
{
    ApplePay,
    GooglePay,
    PayPal
};

enum class WalletProviderDetails
{
    PayPal
};

class BankAccountConnectorDetails : public serde::Serializer<BankAccountConnectorDetails>
{
public:
    void set_connector(std::string value)
    {
        connector = std::move(value);
    }

    [[nodiscard]] const std::string& get_connector() const noexcept
    {
        return connector;
    }

    void set_account_id(std::string value)
    {
        account_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_account_id() const noexcept
    {
        return account_id;
    }

    void set_mca_id(MerchantConnectorAccountIdValue value)
    {
        mca_id = std::move(value);
    }

    [[nodiscard]] const MerchantConnectorAccountIdValue& get_mca_id() const noexcept
    {
        return mca_id;
    }

    void set_access_token(BankAccountAccessCreds value)
    {
        access_token = std::move(value);
    }

    [[nodiscard]] const BankAccountAccessCreds& get_access_token() const noexcept
    {
        return access_token;
    }

    std::string connector{};
    std::string account_id{};
    MerchantConnectorAccountIdValue mca_id{};
    BankAccountAccessCreds access_token{};
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

    void set_name_on_card(std::optional<std::string> value)
    {
        name_on_card = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_name_on_card() const noexcept
    {
        return name_on_card;
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

    void set_card_brand(std::optional<std::string> value)
    {
        card_brand = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_card_brand() const noexcept
    {
        return card_brand;
    }

    void set_card_isin(std::optional<std::string> value)
    {
        card_isin = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_card_isin() const noexcept
    {
        return card_isin;
    }

    void set_nick_name(std::optional<std::string> value)
    {
        nick_name = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_nick_name() const noexcept
    {
        return nick_name;
    }

    CardNumberValue card_number{};
    std::optional<std::string> name_on_card{};
    std::string card_exp_month{};
    std::string card_exp_year{};
    std::optional<std::string> card_brand{};
    std::optional<std::string> card_isin{};
    std::optional<std::string> nick_name{};
};

class CoBadgedCardDataToBeSaved : public serde::Serializer<CoBadgedCardDataToBeSaved>
{
public:
    void set_co_badged_card_networks(std::vector<CardNetworkValue> value)
    {
        co_badged_card_networks = std::move(value);
    }

    [[nodiscard]] const std::vector<CardNetworkValue>& get_co_badged_card_networks() const noexcept
    {
        return co_badged_card_networks;
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

    std::vector<CardNetworkValue> co_badged_card_networks{};
    CountryAlpha2Value issuer_country_code{};
    bool is_regulated{};
    std::optional<RegulatedNameValue> regulated_name{};
};

class ConnectorTokenDetails : public serde::Serializer<ConnectorTokenDetails>
{
public:
    void set_connector_id(MerchantConnectorAccountIdValue value)
    {
        connector_id = std::move(value);
    }

    [[nodiscard]] const MerchantConnectorAccountIdValue& get_connector_id() const noexcept
    {
        return connector_id;
    }

    void set_token_type(TokenizationTypeValue value)
    {
        token_type = std::move(value);
    }

    [[nodiscard]] const TokenizationTypeValue& get_token_type() const noexcept
    {
        return token_type;
    }

    void set_status(ConnectorTokenStatusValue value)
    {
        status = std::move(value);
    }

    [[nodiscard]] const ConnectorTokenStatusValue& get_status() const noexcept
    {
        return status;
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

    void set_original_payment_authorized_amount(std::optional<MinorUnitValue> value)
    {
        original_payment_authorized_amount = std::move(value);
    }

    [[nodiscard]] const std::optional<MinorUnitValue>&
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

    void set_metadata(std::optional<SecretSerdeValueValue> value)
    {
        metadata = std::move(value);
    }

    [[nodiscard]] const std::optional<SecretSerdeValueValue>& get_metadata() const noexcept
    {
        return metadata;
    }

    void set_connector_customer_id(std::optional<std::string> value)
    {
        connector_customer_id = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_connector_customer_id() const noexcept
    {
        return connector_customer_id;
    }

    void set_token(std::string value)
    {
        token = std::move(value);
    }

    [[nodiscard]] const std::string& get_token() const noexcept
    {
        return token;
    }

    MerchantConnectorAccountIdValue connector_id{};
    TokenizationTypeValue token_type{};
    ConnectorTokenStatusValue status{};
    std::optional<std::string> connector_token_request_reference_id{};
    std::optional<MinorUnitValue> original_payment_authorized_amount{};
    std::optional<CurrencyValue> original_payment_authorized_currency{};
    std::optional<SecretSerdeValueValue> metadata{};
    std::optional<std::string> connector_customer_id{};
    std::string token{};
};

class PaymentMethodDataWalletInfo : public serde::Serializer<PaymentMethodDataWalletInfo>
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

    void set_card_network(std::optional<std::string> value)
    {
        card_network = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_card_network() const noexcept
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

    void set_auth_code(std::optional<std::string> value)
    {
        auth_code = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_auth_code() const noexcept
    {
        return auth_code;
    }

    void set_email(std::optional<EmailValue> value)
    {
        email = std::move(value);
    }

    [[nodiscard]] const std::optional<EmailValue>& get_email() const noexcept
    {
        return email;
    }

    void set_provider_details(std::optional<WalletProviderDetails> value)
    {
        provider_details = std::move(value);
    }

    [[nodiscard]] const std::optional<WalletProviderDetails>& get_provider_details() const noexcept
    {
        return provider_details;
    }

    std::optional<std::string> last4{};
    std::optional<std::string> card_network{};
    std::optional<std::string> card_type{};
    std::optional<std::string> card_exp_month{};
    std::optional<std::string> card_exp_year{};
    std::optional<std::string> auth_code{};
    std::optional<EmailValue> email{};
    std::optional<WalletProviderDetails> provider_details{};
};

class ProxyCardDetails : public serde::Serializer<ProxyCardDetails>
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

    void set_card_cvc(std::optional<std::string> value)
    {
        card_cvc = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_card_cvc() const noexcept
    {
        return card_cvc;
    }

    std::string card_number{};
    std::string card_exp_month{};
    std::string card_exp_year{};
    std::optional<std::string> bin_number{};
    std::optional<std::string> last_four{};
    std::optional<std::string> card_issuer{};
    std::optional<CardNetworkValue> card_network{};
    std::optional<std::string> card_type{};
    std::optional<std::string> card_issuing_country{};
    std::optional<std::string> nick_name{};
    std::optional<std::string> card_holder_name{};
    std::optional<std::string> card_cvc{};
};

class TokenizedBankRedirectValue1 : public serde::Serializer<TokenizedBankRedirectValue1>
{
public:
    void set_data(BankRedirectData value)
    {
        data = std::move(value);
    }

    [[nodiscard]] const BankRedirectData& get_data() const noexcept
    {
        return data;
    }

    BankRedirectData data{};
};

class TokenizedBankRedirectValue2 : public serde::Serializer<TokenizedBankRedirectValue2>
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

    std::optional<CustomerIdValue> customer_id{};
};

class TokenizedBankTransferValue1 : public serde::Serializer<TokenizedBankTransferValue1>
{
public:
    void set_data(BankTransferDataValue value)
    {
        data = std::move(value);
    }

    [[nodiscard]] const BankTransferDataValue& get_data() const noexcept
    {
        return data;
    }

    BankTransferDataValue data{};
};

class TokenizedBankTransferValue2 : public serde::Serializer<TokenizedBankTransferValue2>
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

    std::optional<CustomerIdValue> customer_id{};
};

class TokenizedCardValue1 : public serde::Serializer<TokenizedCardValue1>
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

    void set_exp_year(std::string value)
    {
        exp_year = std::move(value);
    }

    [[nodiscard]] const std::string& get_exp_year() const noexcept
    {
        return exp_year;
    }

    void set_exp_month(std::string value)
    {
        exp_month = std::move(value);
    }

    [[nodiscard]] const std::string& get_exp_month() const noexcept
    {
        return exp_month;
    }

    void set_name_on_card(std::optional<std::string> value)
    {
        name_on_card = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_name_on_card() const noexcept
    {
        return name_on_card;
    }

    void set_nickname(std::optional<std::string> value)
    {
        nickname = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_nickname() const noexcept
    {
        return nickname;
    }

    void set_card_last_four(std::optional<std::string> value)
    {
        card_last_four = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_card_last_four() const noexcept
    {
        return card_last_four;
    }

    void set_card_token(std::optional<std::string> value)
    {
        card_token = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_card_token() const noexcept
    {
        return card_token;
    }

    void set_card_network(std::optional<CardNetworkValue> value)
    {
        card_network = std::move(value);
    }

    [[nodiscard]] const std::optional<CardNetworkValue>& get_card_network() const noexcept
    {
        return card_network;
    }

    std::string card_number{};
    std::string exp_year{};
    std::string exp_month{};
    std::optional<std::string> name_on_card{};
    std::optional<std::string> nickname{};
    std::optional<std::string> card_last_four{};
    std::optional<std::string> card_token{};
    std::optional<CardNetworkValue> card_network{};
};

class TokenizedCardValue2 : public serde::Serializer<TokenizedCardValue2>
{
public:
    void set_card_security_code(std::optional<std::string> value)
    {
        card_security_code = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_card_security_code() const noexcept
    {
        return card_security_code;
    }

    void set_card_fingerprint(std::optional<std::string> value)
    {
        card_fingerprint = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_card_fingerprint() const noexcept
    {
        return card_fingerprint;
    }

    void set_external_id(std::optional<std::string> value)
    {
        external_id = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_external_id() const noexcept
    {
        return external_id;
    }

    void set_customer_id(std::optional<CustomerIdValue> value)
    {
        customer_id = std::move(value);
    }

    [[nodiscard]] const std::optional<CustomerIdValue>& get_customer_id() const noexcept
    {
        return customer_id;
    }

    void set_payment_method_id(std::optional<std::string> value)
    {
        payment_method_id = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_payment_method_id() const noexcept
    {
        return payment_method_id;
    }

    std::optional<std::string> card_security_code{};
    std::optional<std::string> card_fingerprint{};
    std::optional<std::string> external_id{};
    std::optional<CustomerIdValue> customer_id{};
    std::optional<std::string> payment_method_id{};
};

class TokenizedWalletValue1 : public serde::Serializer<TokenizedWalletValue1>
{
public:
    void set_data(WalletDataValue value)
    {
        data = std::move(value);
    }

    [[nodiscard]] const WalletDataValue& get_data() const noexcept
    {
        return data;
    }

    WalletDataValue data{};
};

class TokenizedWalletValue2 : public serde::Serializer<TokenizedWalletValue2>
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

    std::optional<CustomerIdValue> customer_id{};
};

class BankAccountTokenData : public serde::Serializer<BankAccountTokenData>
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

    void set_payment_method(PaymentMethodValue value)
    {
        payment_method = std::move(value);
    }

    [[nodiscard]] const PaymentMethodValue& get_payment_method() const noexcept
    {
        return payment_method;
    }

    void set_connector_details(BankAccountConnectorDetails value)
    {
        connector_details = std::move(value);
    }

    [[nodiscard]] const BankAccountConnectorDetails& get_connector_details() const noexcept
    {
        return connector_details;
    }

    PaymentMethodTypeValue payment_method_type{};
    PaymentMethodValue payment_method{};
    BankAccountConnectorDetails connector_details{};
};

class CardDetailsPaymentMethod : public serde::Serializer<CardDetailsPaymentMethod>
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

    void set_co_badged_card_data(std::optional<CoBadgedCardDataToBeSaved> value)
    {
        co_badged_card_data = std::move(value);
    }

    [[nodiscard]] const std::optional<CoBadgedCardDataToBeSaved>&
    get_co_badged_card_data() const noexcept
    {
        return co_badged_card_data;
    }

    std::optional<std::string> last4_digits{};
    std::optional<std::string> issuer_country{};
    std::optional<std::string> issuer_country_code{};
    std::optional<std::string> expiry_month{};
    std::optional<std::string> expiry_year{};
    std::optional<std::string> nick_name{};
    std::optional<std::string> card_holder_name{};
    std::optional<std::string> card_isin{};
    std::optional<std::string> card_issuer{};
    std::optional<CardNetworkValue> card_network{};
    std::optional<std::string> card_type{};
    std::optional<std::string> card_subtype{};
    std::optional<CardSegmentTypeValue> card_segment_type{};
    std::optional<FundingSourceValue> funding_source{};
    bool saved_to_locker{};
    std::optional<CoBadgedCardDataToBeSaved> co_badged_card_data{};
};

enum class CustomerPaymentMethodDataForClient
{
    Card,
    Wallet,
    BankDebit,
    BankRedirect
};

class PaymentMethodDataBankCreds : public serde::Serializer<PaymentMethodDataBankCreds>
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

    void set_hash(std::string value)
    {
        hash = std::move(value);
    }

    [[nodiscard]] const std::string& get_hash() const noexcept
    {
        return hash;
    }

    void set_account_type(std::optional<std::string> value)
    {
        account_type = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_account_type() const noexcept
    {
        return account_type;
    }

    void set_account_name(std::optional<std::string> value)
    {
        account_name = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_account_name() const noexcept
    {
        return account_name;
    }

    void set_payment_method_type(PaymentMethodTypeValue value)
    {
        payment_method_type = std::move(value);
    }

    [[nodiscard]] const PaymentMethodTypeValue& get_payment_method_type() const noexcept
    {
        return payment_method_type;
    }

    void set_connector_details(std::vector<BankAccountConnectorDetails> value)
    {
        connector_details = std::move(value);
    }

    [[nodiscard]] const std::vector<BankAccountConnectorDetails>&
    get_connector_details() const noexcept
    {
        return connector_details;
    }

    std::string mask{};
    std::string hash{};
    std::optional<std::string> account_type{};
    std::optional<std::string> account_name{};
    PaymentMethodTypeValue payment_method_type{};
    std::vector<BankAccountConnectorDetails> connector_details{};
};

enum class RawPaymentMethodData
{
    Card,
    CardWithNT,
    BankDebit,
    Wallet,
    ProxyCard
};

enum class TokenizeDataRequest
{
    Card,
    ExistingPaymentMethod
};

} // namespace app::model::payment_methods

template<>
struct serde::Serializable<app::model::payment_methods::PaymentMethodDataWalletInfo>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "last4",
                &app::model::payment_methods::PaymentMethodDataWalletInfo::get_last4,
                &app::model::payment_methods::PaymentMethodDataWalletInfo::set_last4,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_network",
                &app::model::payment_methods::PaymentMethodDataWalletInfo::get_card_network,
                &app::model::payment_methods::PaymentMethodDataWalletInfo::set_card_network,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "type",
                &app::model::payment_methods::PaymentMethodDataWalletInfo::get_card_type,
                &app::model::payment_methods::PaymentMethodDataWalletInfo::set_card_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_exp_month",
                &app::model::payment_methods::PaymentMethodDataWalletInfo::get_card_exp_month,
                &app::model::payment_methods::PaymentMethodDataWalletInfo::set_card_exp_month,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_exp_year",
                &app::model::payment_methods::PaymentMethodDataWalletInfo::get_card_exp_year,
                &app::model::payment_methods::PaymentMethodDataWalletInfo::set_card_exp_year,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "auth_code",
                &app::model::payment_methods::PaymentMethodDataWalletInfo::get_auth_code,
                &app::model::payment_methods::PaymentMethodDataWalletInfo::set_auth_code,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "email",
                &app::model::payment_methods::PaymentMethodDataWalletInfo::get_email,
                &app::model::payment_methods::PaymentMethodDataWalletInfo::set_email,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "provider_details",
                &app::model::payment_methods::PaymentMethodDataWalletInfo::get_provider_details,
                &app::model::payment_methods::PaymentMethodDataWalletInfo::set_provider_details,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "payment_method_data_wallet_info";
    }
};

template<>
struct serde::Serializable<app::model::payment_methods::ProxyCardDetails>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "card_number",
                &app::model::payment_methods::ProxyCardDetails::get_card_number,
                &app::model::payment_methods::ProxyCardDetails::set_card_number,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_exp_month",
                &app::model::payment_methods::ProxyCardDetails::get_card_exp_month,
                &app::model::payment_methods::ProxyCardDetails::set_card_exp_month,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_exp_year",
                &app::model::payment_methods::ProxyCardDetails::get_card_exp_year,
                &app::model::payment_methods::ProxyCardDetails::set_card_exp_year,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "bin_number",
                &app::model::payment_methods::ProxyCardDetails::get_bin_number,
                &app::model::payment_methods::ProxyCardDetails::set_bin_number,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "last_four",
                &app::model::payment_methods::ProxyCardDetails::get_last_four,
                &app::model::payment_methods::ProxyCardDetails::set_last_four,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_issuer",
                &app::model::payment_methods::ProxyCardDetails::get_card_issuer,
                &app::model::payment_methods::ProxyCardDetails::set_card_issuer,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_network",
                &app::model::payment_methods::ProxyCardDetails::get_card_network,
                &app::model::payment_methods::ProxyCardDetails::set_card_network,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_type",
                &app::model::payment_methods::ProxyCardDetails::get_card_type,
                &app::model::payment_methods::ProxyCardDetails::set_card_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_issuing_country",
                &app::model::payment_methods::ProxyCardDetails::get_card_issuing_country,
                &app::model::payment_methods::ProxyCardDetails::set_card_issuing_country,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "nick_name",
                &app::model::payment_methods::ProxyCardDetails::get_nick_name,
                &app::model::payment_methods::ProxyCardDetails::set_nick_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_holder_name",
                &app::model::payment_methods::ProxyCardDetails::get_card_holder_name,
                &app::model::payment_methods::ProxyCardDetails::set_card_holder_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_cvc",
                &app::model::payment_methods::ProxyCardDetails::get_card_cvc,
                &app::model::payment_methods::ProxyCardDetails::set_card_cvc,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "proxy_card_details";
    }
};

template<>
struct serde::Serializable<app::model::payment_methods::ConnectorTokenDetails>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "connector_id",
                &app::model::payment_methods::ConnectorTokenDetails::get_connector_id,
                &app::model::payment_methods::ConnectorTokenDetails::set_connector_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "token_type",
                &app::model::payment_methods::ConnectorTokenDetails::get_token_type,
                &app::model::payment_methods::ConnectorTokenDetails::set_token_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "status",
                &app::model::payment_methods::ConnectorTokenDetails::get_status,
                &app::model::payment_methods::ConnectorTokenDetails::set_status,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "connector_token_request_reference_id",
                &app::model::payment_methods::ConnectorTokenDetails::
                    get_connector_token_request_reference_id,
                &app::model::payment_methods::ConnectorTokenDetails::
                    set_connector_token_request_reference_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "original_payment_authorized_amount",
                &app::model::payment_methods::ConnectorTokenDetails::
                    get_original_payment_authorized_amount,
                &app::model::payment_methods::ConnectorTokenDetails::
                    set_original_payment_authorized_amount,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "original_payment_authorized_currency",
                &app::model::payment_methods::ConnectorTokenDetails::
                    get_original_payment_authorized_currency,
                &app::model::payment_methods::ConnectorTokenDetails::
                    set_original_payment_authorized_currency,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "metadata",
                &app::model::payment_methods::ConnectorTokenDetails::get_metadata,
                &app::model::payment_methods::ConnectorTokenDetails::set_metadata,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "connector_customer_id",
                &app::model::payment_methods::ConnectorTokenDetails::get_connector_customer_id,
                &app::model::payment_methods::ConnectorTokenDetails::set_connector_customer_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "token",
                &app::model::payment_methods::ConnectorTokenDetails::get_token,
                &app::model::payment_methods::ConnectorTokenDetails::set_token,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "connector_token_details";
    }
};

template<>
struct serde::Serializable<app::model::payment_methods::CoBadgedCardDataToBeSaved>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "co_badged_card_networks",
                &app::model::payment_methods::CoBadgedCardDataToBeSaved::
                    get_co_badged_card_networks,
                &app::model::payment_methods::CoBadgedCardDataToBeSaved::
                    set_co_badged_card_networks,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "issuer_country_code",
                &app::model::payment_methods::CoBadgedCardDataToBeSaved::get_issuer_country_code,
                &app::model::payment_methods::CoBadgedCardDataToBeSaved::set_issuer_country_code,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "is_regulated",
                &app::model::payment_methods::CoBadgedCardDataToBeSaved::get_is_regulated,
                &app::model::payment_methods::CoBadgedCardDataToBeSaved::set_is_regulated,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "regulated_name",
                &app::model::payment_methods::CoBadgedCardDataToBeSaved::get_regulated_name,
                &app::model::payment_methods::CoBadgedCardDataToBeSaved::set_regulated_name,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "co_badged_card_data_to_be_saved";
    }
};

template<>
struct serde::Serializable<app::model::payment_methods::CardDetailsPaymentMethod>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "last4_digits",
                &app::model::payment_methods::CardDetailsPaymentMethod::get_last4_digits,
                &app::model::payment_methods::CardDetailsPaymentMethod::set_last4_digits,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "issuer_country",
                &app::model::payment_methods::CardDetailsPaymentMethod::get_issuer_country,
                &app::model::payment_methods::CardDetailsPaymentMethod::set_issuer_country,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "issuer_country_code",
                &app::model::payment_methods::CardDetailsPaymentMethod::get_issuer_country_code,
                &app::model::payment_methods::CardDetailsPaymentMethod::set_issuer_country_code,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "expiry_month",
                &app::model::payment_methods::CardDetailsPaymentMethod::get_expiry_month,
                &app::model::payment_methods::CardDetailsPaymentMethod::set_expiry_month,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "expiry_year",
                &app::model::payment_methods::CardDetailsPaymentMethod::get_expiry_year,
                &app::model::payment_methods::CardDetailsPaymentMethod::set_expiry_year,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "nick_name",
                &app::model::payment_methods::CardDetailsPaymentMethod::get_nick_name,
                &app::model::payment_methods::CardDetailsPaymentMethod::set_nick_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_holder_name",
                &app::model::payment_methods::CardDetailsPaymentMethod::get_card_holder_name,
                &app::model::payment_methods::CardDetailsPaymentMethod::set_card_holder_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_isin",
                &app::model::payment_methods::CardDetailsPaymentMethod::get_card_isin,
                &app::model::payment_methods::CardDetailsPaymentMethod::set_card_isin,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_issuer",
                &app::model::payment_methods::CardDetailsPaymentMethod::get_card_issuer,
                &app::model::payment_methods::CardDetailsPaymentMethod::set_card_issuer,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_network",
                &app::model::payment_methods::CardDetailsPaymentMethod::get_card_network,
                &app::model::payment_methods::CardDetailsPaymentMethod::set_card_network,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_type",
                &app::model::payment_methods::CardDetailsPaymentMethod::get_card_type,
                &app::model::payment_methods::CardDetailsPaymentMethod::set_card_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_subtype",
                &app::model::payment_methods::CardDetailsPaymentMethod::get_card_subtype,
                &app::model::payment_methods::CardDetailsPaymentMethod::set_card_subtype,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_segment_type",
                &app::model::payment_methods::CardDetailsPaymentMethod::get_card_segment_type,
                &app::model::payment_methods::CardDetailsPaymentMethod::set_card_segment_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "funding_source",
                &app::model::payment_methods::CardDetailsPaymentMethod::get_funding_source,
                &app::model::payment_methods::CardDetailsPaymentMethod::set_funding_source,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "saved_to_locker",
                &app::model::payment_methods::CardDetailsPaymentMethod::get_saved_to_locker,
                &app::model::payment_methods::CardDetailsPaymentMethod::set_saved_to_locker,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "co_badged_card_data",
                &app::model::payment_methods::CardDetailsPaymentMethod::get_co_badged_card_data,
                &app::model::payment_methods::CardDetailsPaymentMethod::set_co_badged_card_data,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "card_details_payment_method";
    }
};

template<>
struct serde::Serializable<app::model::payment_methods::BankAccountConnectorDetails>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "connector",
                &app::model::payment_methods::BankAccountConnectorDetails::get_connector,
                &app::model::payment_methods::BankAccountConnectorDetails::set_connector,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "account_id",
                &app::model::payment_methods::BankAccountConnectorDetails::get_account_id,
                &app::model::payment_methods::BankAccountConnectorDetails::set_account_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "mca_id",
                &app::model::payment_methods::BankAccountConnectorDetails::get_mca_id,
                &app::model::payment_methods::BankAccountConnectorDetails::set_mca_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "access_token",
                &app::model::payment_methods::BankAccountConnectorDetails::get_access_token,
                &app::model::payment_methods::BankAccountConnectorDetails::set_access_token,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "bank_account_connector_details";
    }
};

template<>
struct serde::Serializable<app::model::payment_methods::PaymentMethodDataBankCreds>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "mask",
                &app::model::payment_methods::PaymentMethodDataBankCreds::get_mask,
                &app::model::payment_methods::PaymentMethodDataBankCreds::set_mask,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "hash",
                &app::model::payment_methods::PaymentMethodDataBankCreds::get_hash,
                &app::model::payment_methods::PaymentMethodDataBankCreds::set_hash,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "account_type",
                &app::model::payment_methods::PaymentMethodDataBankCreds::get_account_type,
                &app::model::payment_methods::PaymentMethodDataBankCreds::set_account_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "account_name",
                &app::model::payment_methods::PaymentMethodDataBankCreds::get_account_name,
                &app::model::payment_methods::PaymentMethodDataBankCreds::set_account_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_method_type",
                &app::model::payment_methods::PaymentMethodDataBankCreds::get_payment_method_type,
                &app::model::payment_methods::PaymentMethodDataBankCreds::set_payment_method_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "connector_details",
                &app::model::payment_methods::PaymentMethodDataBankCreds::get_connector_details,
                &app::model::payment_methods::PaymentMethodDataBankCreds::set_connector_details,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "payment_method_data_bank_creds";
    }
};

template<>
struct serde::Serializable<app::model::payment_methods::BankAccountTokenData>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "payment_method_type",
                &app::model::payment_methods::BankAccountTokenData::get_payment_method_type,
                &app::model::payment_methods::BankAccountTokenData::set_payment_method_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_method",
                &app::model::payment_methods::BankAccountTokenData::get_payment_method,
                &app::model::payment_methods::BankAccountTokenData::set_payment_method,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "connector_details",
                &app::model::payment_methods::BankAccountTokenData::get_connector_details,
                &app::model::payment_methods::BankAccountTokenData::set_connector_details,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "bank_account_token_data";
    }
};

template<>
struct serde::Serializable<app::model::payment_methods::Card>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "card_number",
                &app::model::payment_methods::Card::get_card_number,
                &app::model::payment_methods::Card::set_card_number,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "name_on_card",
                &app::model::payment_methods::Card::get_name_on_card,
                &app::model::payment_methods::Card::set_name_on_card,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_exp_month",
                &app::model::payment_methods::Card::get_card_exp_month,
                &app::model::payment_methods::Card::set_card_exp_month,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_exp_year",
                &app::model::payment_methods::Card::get_card_exp_year,
                &app::model::payment_methods::Card::set_card_exp_year,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_brand",
                &app::model::payment_methods::Card::get_card_brand,
                &app::model::payment_methods::Card::set_card_brand,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_isin",
                &app::model::payment_methods::Card::get_card_isin,
                &app::model::payment_methods::Card::set_card_isin,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "nick_name",
                &app::model::payment_methods::Card::get_nick_name,
                &app::model::payment_methods::Card::set_nick_name,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "card";
    }
};

template<>
struct serde::Serializable<app::model::payment_methods::BankDebitTypes>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<
            "eligible_connectors",
            &app::model::payment_methods::BankDebitTypes::get_eligible_connectors,
            &app::model::payment_methods::BankDebitTypes::set_eligible_connectors,
            serde::FieldOptions{}>{}};
    }

    static constexpr std::string_view table_name()
    {
        return "bank_debit_types";
    }
};

template<>
struct serde::Serializable<app::model::payment_methods::BankTransferTypes>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<
            "eligible_connectors",
            &app::model::payment_methods::BankTransferTypes::get_eligible_connectors,
            &app::model::payment_methods::BankTransferTypes::set_eligible_connectors,
            serde::FieldOptions{}>{}};
    }

    static constexpr std::string_view table_name()
    {
        return "bank_transfer_types";
    }
};

template<>
struct serde::Serializable<app::model::payment_methods::TokenizedCardValue1>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "cardNumber",
                &app::model::payment_methods::TokenizedCardValue1::get_card_number,
                &app::model::payment_methods::TokenizedCardValue1::set_card_number,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "expYear",
                &app::model::payment_methods::TokenizedCardValue1::get_exp_year,
                &app::model::payment_methods::TokenizedCardValue1::set_exp_year,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "expMonth",
                &app::model::payment_methods::TokenizedCardValue1::get_exp_month,
                &app::model::payment_methods::TokenizedCardValue1::set_exp_month,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "nameOnCard",
                &app::model::payment_methods::TokenizedCardValue1::get_name_on_card,
                &app::model::payment_methods::TokenizedCardValue1::set_name_on_card,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "nickname",
                &app::model::payment_methods::TokenizedCardValue1::get_nickname,
                &app::model::payment_methods::TokenizedCardValue1::set_nickname,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "cardLastFour",
                &app::model::payment_methods::TokenizedCardValue1::get_card_last_four,
                &app::model::payment_methods::TokenizedCardValue1::set_card_last_four,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "cardToken",
                &app::model::payment_methods::TokenizedCardValue1::get_card_token,
                &app::model::payment_methods::TokenizedCardValue1::set_card_token,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "cardNetwork",
                &app::model::payment_methods::TokenizedCardValue1::get_card_network,
                &app::model::payment_methods::TokenizedCardValue1::set_card_network,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "tokenized_card_value1";
    }
};

template<>
struct serde::Serializable<app::model::payment_methods::TokenizedCardValue2>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "cardSecurityCode",
                &app::model::payment_methods::TokenizedCardValue2::get_card_security_code,
                &app::model::payment_methods::TokenizedCardValue2::set_card_security_code,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "cardFingerprint",
                &app::model::payment_methods::TokenizedCardValue2::get_card_fingerprint,
                &app::model::payment_methods::TokenizedCardValue2::set_card_fingerprint,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "externalId",
                &app::model::payment_methods::TokenizedCardValue2::get_external_id,
                &app::model::payment_methods::TokenizedCardValue2::set_external_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "customerId",
                &app::model::payment_methods::TokenizedCardValue2::get_customer_id,
                &app::model::payment_methods::TokenizedCardValue2::set_customer_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "paymentMethodId",
                &app::model::payment_methods::TokenizedCardValue2::get_payment_method_id,
                &app::model::payment_methods::TokenizedCardValue2::set_payment_method_id,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "tokenized_card_value2";
    }
};

template<>
struct serde::Serializable<app::model::payment_methods::TokenizedWalletValue1>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<
            "data",
            &app::model::payment_methods::TokenizedWalletValue1::get_data,
            &app::model::payment_methods::TokenizedWalletValue1::set_data,
            serde::FieldOptions{}>{}};
    }

    static constexpr std::string_view table_name()
    {
        return "tokenized_wallet_value1";
    }
};

template<>
struct serde::Serializable<app::model::payment_methods::TokenizedWalletValue2>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<
            "customer_id",
            &app::model::payment_methods::TokenizedWalletValue2::get_customer_id,
            &app::model::payment_methods::TokenizedWalletValue2::set_customer_id,
            serde::FieldOptions{}>{}};
    }

    static constexpr std::string_view table_name()
    {
        return "tokenized_wallet_value2";
    }
};

template<>
struct serde::Serializable<app::model::payment_methods::TokenizedBankTransferValue1>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<
            "data",
            &app::model::payment_methods::TokenizedBankTransferValue1::get_data,
            &app::model::payment_methods::TokenizedBankTransferValue1::set_data,
            serde::FieldOptions{}>{}};
    }

    static constexpr std::string_view table_name()
    {
        return "tokenized_bank_transfer_value1";
    }
};

template<>
struct serde::Serializable<app::model::payment_methods::TokenizedBankTransferValue2>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<
            "customer_id",
            &app::model::payment_methods::TokenizedBankTransferValue2::get_customer_id,
            &app::model::payment_methods::TokenizedBankTransferValue2::set_customer_id,
            serde::FieldOptions{}>{}};
    }

    static constexpr std::string_view table_name()
    {
        return "tokenized_bank_transfer_value2";
    }
};

template<>
struct serde::Serializable<app::model::payment_methods::TokenizedBankRedirectValue1>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<
            "data",
            &app::model::payment_methods::TokenizedBankRedirectValue1::get_data,
            &app::model::payment_methods::TokenizedBankRedirectValue1::set_data,
            serde::FieldOptions{}>{}};
    }

    static constexpr std::string_view table_name()
    {
        return "tokenized_bank_redirect_value1";
    }
};

template<>
struct serde::Serializable<app::model::payment_methods::TokenizedBankRedirectValue2>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<
            "customer_id",
            &app::model::payment_methods::TokenizedBankRedirectValue2::get_customer_id,
            &app::model::payment_methods::TokenizedBankRedirectValue2::set_customer_id,
            serde::FieldOptions{}>{}};
    }

    static constexpr std::string_view table_name()
    {
        return "tokenized_bank_redirect_value2";
    }
};
