module;
export module app_model:cards_info;

import core_serde;

import std;
import core_serde;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace app::model::cards_info {

class CardAuthenticationValue : public serde::Serializer<CardAuthenticationValue>
{
public:
    void set_value(std::string value)
    {
        this->value = std::move(value);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

class CardCostValue : public serde::Serializer<CardCostValue>
{
public:
    void set_value(std::string value)
    {
        this->value = std::move(value);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

enum class CardInfoMigrationStatus
{
    Success,
    Failed,
};

class CardNetworkValue : public serde::Serializer<CardNetworkValue>
{
public:
    void set_value(std::string value)
    {
        this->value = std::move(value);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

class CardsInfoRequest : public serde::Serializer<CardsInfoRequest>
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

    void set_card_iin(std::string value)
    {
        card_iin = std::move(value);
    }

    [[nodiscard]] const std::string& get_card_iin() const noexcept
    {
        return card_iin;
    }

    std::optional<std::string> client_secret{};
    std::string card_iin{};
};

class CardsInfoRequestParams : public serde::Serializer<CardsInfoRequestParams>
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

class CoBadgedCardNetworkMetadataValue : public serde::Serializer<CoBadgedCardNetworkMetadataValue>
{
public:
    void set_value(std::string value)
    {
        this->value = std::move(value);
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
    void set_value(std::string value)
    {
        this->value = std::move(value);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

class PanOrTokenValue : public serde::Serializer<PanOrTokenValue>
{
public:
    void set_value(std::string value)
    {
        this->value = std::move(value);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

class CardInfoCreateRequest : public serde::Serializer<CardInfoCreateRequest>
{
public:
    void set_card_iin(std::string value)
    {
        card_iin = std::move(value);
    }

    [[nodiscard]] const std::string& get_card_iin() const noexcept
    {
        return card_iin;
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

    void set_card_issuing_country(std::optional<std::string> value)
    {
        card_issuing_country = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_card_issuing_country() const noexcept
    {
        return card_issuing_country;
    }

    void set_bank_code_id(std::optional<std::string> value)
    {
        bank_code_id = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_bank_code_id() const noexcept
    {
        return bank_code_id;
    }

    void set_bank_code(std::optional<std::string> value)
    {
        bank_code = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_bank_code() const noexcept
    {
        return bank_code;
    }

    void set_country_code(std::optional<std::string> value)
    {
        country_code = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_country_code() const noexcept
    {
        return country_code;
    }

    void set_last_updated_provider(std::optional<std::string> value)
    {
        last_updated_provider = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_last_updated_provider() const noexcept
    {
        return last_updated_provider;
    }

    void set_funding_source(std::optional<FundingSourceValue> value)
    {
        funding_source = std::move(value);
    }

    [[nodiscard]] const std::optional<FundingSourceValue>& get_funding_source() const noexcept
    {
        return funding_source;
    }

    void set_card_iin_type(std::optional<PanOrTokenValue> value)
    {
        card_iin_type = std::move(value);
    }

    [[nodiscard]] const std::optional<PanOrTokenValue>& get_card_iin_type() const noexcept
    {
        return card_iin_type;
    }

    void set_virtual_card(std::optional<bool> value)
    {
        virtual_card = std::move(value);
    }

    [[nodiscard]] const std::optional<bool>& get_virtual_card() const noexcept
    {
        return virtual_card;
    }

    void set_gambling_blocked(std::optional<bool> value)
    {
        gambling_blocked = std::move(value);
    }

    [[nodiscard]] const std::optional<bool>& get_gambling_blocked() const noexcept
    {
        return gambling_blocked;
    }

    void set_co_badged_card_networks(std::optional<CoBadgedCardNetworkMetadataValue> value)
    {
        co_badged_card_networks = std::move(value);
    }

    [[nodiscard]] const std::optional<CoBadgedCardNetworkMetadataValue>&
    get_co_badged_card_networks() const noexcept
    {
        return co_badged_card_networks;
    }

    void set_card_segment_type(std::optional<std::string> value)
    {
        card_segment_type = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_card_segment_type() const noexcept
    {
        return card_segment_type;
    }

    void set_numeric_country_code(std::optional<std::string> value)
    {
        numeric_country_code = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_numeric_country_code() const noexcept
    {
        return numeric_country_code;
    }

    void set_prepaid(std::optional<bool> value)
    {
        prepaid = std::move(value);
    }

    [[nodiscard]] const std::optional<bool>& get_prepaid() const noexcept
    {
        return prepaid;
    }

    void set_regulated(std::optional<bool> value)
    {
        regulated = std::move(value);
    }

    [[nodiscard]] const std::optional<bool>& get_regulated() const noexcept
    {
        return regulated;
    }

    void set_issuer_phone(std::optional<std::string> value)
    {
        issuer_phone = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_issuer_phone() const noexcept
    {
        return issuer_phone;
    }

    void set_issuer_url(std::optional<std::string> value)
    {
        issuer_url = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_issuer_url() const noexcept
    {
        return issuer_url;
    }

    void set_regulated_name(std::optional<std::string> value)
    {
        regulated_name = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_regulated_name() const noexcept
    {
        return regulated_name;
    }

    void set_reloadable_prepaid(std::optional<bool> value)
    {
        reloadable_prepaid = std::move(value);
    }

    [[nodiscard]] const std::optional<bool>& get_reloadable_prepaid() const noexcept
    {
        return reloadable_prepaid;
    }

    void set_account_updater(std::optional<bool> value)
    {
        account_updater = std::move(value);
    }

    [[nodiscard]] const std::optional<bool>& get_account_updater() const noexcept
    {
        return account_updater;
    }

    void set_account_level_management(std::optional<bool> value)
    {
        account_level_management = std::move(value);
    }

    [[nodiscard]] const std::optional<bool>& get_account_level_management() const noexcept
    {
        return account_level_management;
    }

    void set_domestic_only(std::optional<bool> value)
    {
        domestic_only = std::move(value);
    }

    [[nodiscard]] const std::optional<bool>& get_domestic_only() const noexcept
    {
        return domestic_only;
    }

    void set_level_two_supported(std::optional<bool> value)
    {
        level_two_supported = std::move(value);
    }

    [[nodiscard]] const std::optional<bool>& get_level_two_supported() const noexcept
    {
        return level_two_supported;
    }

    void set_level_three_supported(std::optional<bool> value)
    {
        level_three_supported = std::move(value);
    }

    [[nodiscard]] const std::optional<bool>& get_level_three_supported() const noexcept
    {
        return level_three_supported;
    }

    void set_issuer_currency(std::optional<std::string> value)
    {
        issuer_currency = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_issuer_currency() const noexcept
    {
        return issuer_currency;
    }

    void set_combo_card(std::optional<std::string> value)
    {
        combo_card = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_combo_card() const noexcept
    {
        return combo_card;
    }

    void set_authentication(std::optional<CardAuthenticationValue> value)
    {
        authentication = std::move(value);
    }

    [[nodiscard]] const std::optional<CardAuthenticationValue>& get_authentication() const noexcept
    {
        return authentication;
    }

    void set_cost(std::optional<CardCostValue> value)
    {
        cost = std::move(value);
    }

    [[nodiscard]] const std::optional<CardCostValue>& get_cost() const noexcept
    {
        return cost;
    }

    void set_issuer_supports_tokenization(std::optional<bool> value)
    {
        issuer_supports_tokenization = std::move(value);
    }

    [[nodiscard]] const std::optional<bool>& get_issuer_supports_tokenization() const noexcept
    {
        return issuer_supports_tokenization;
    }

    void set_billpay_enabled(std::optional<bool> value)
    {
        billpay_enabled = std::move(value);
    }

    [[nodiscard]] const std::optional<bool>& get_billpay_enabled() const noexcept
    {
        return billpay_enabled;
    }

    void set_ecom_enabled(std::optional<bool> value)
    {
        ecom_enabled = std::move(value);
    }

    [[nodiscard]] const std::optional<bool>& get_ecom_enabled() const noexcept
    {
        return ecom_enabled;
    }

    void set_flexible_credential_supported(std::optional<bool> value)
    {
        flexible_credential_supported = std::move(value);
    }

    [[nodiscard]] const std::optional<bool>& get_flexible_credential_supported() const noexcept
    {
        return flexible_credential_supported;
    }

    void set_card_subtype_code(std::optional<std::string> value)
    {
        card_subtype_code = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_card_subtype_code() const noexcept
    {
        return card_subtype_code;
    }

    void set_multi_account_access_indicator(std::optional<std::string> value)
    {
        multi_account_access_indicator = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>&
    get_multi_account_access_indicator() const noexcept
    {
        return multi_account_access_indicator;
    }

    std::string card_iin{};
    std::optional<std::string> card_issuer{};
    std::optional<CardNetworkValue> card_network{};
    std::optional<std::string> card_type{};
    std::optional<std::string> card_subtype{};
    std::optional<std::string> card_issuing_country{};
    std::optional<std::string> bank_code_id{};
    std::optional<std::string> bank_code{};
    std::optional<std::string> country_code{};
    std::optional<std::string> last_updated_provider{};
    std::optional<FundingSourceValue> funding_source{};
    std::optional<PanOrTokenValue> card_iin_type{};
    std::optional<bool> virtual_card{};
    std::optional<bool> gambling_blocked{};
    std::optional<CoBadgedCardNetworkMetadataValue> co_badged_card_networks{};
    std::optional<std::string> card_segment_type{};
    std::optional<std::string> numeric_country_code{};
    std::optional<bool> prepaid{};
    std::optional<bool> regulated{};
    std::optional<std::string> issuer_phone{};
    std::optional<std::string> issuer_url{};
    std::optional<std::string> regulated_name{};
    std::optional<bool> reloadable_prepaid{};
    std::optional<bool> account_updater{};
    std::optional<bool> account_level_management{};
    std::optional<bool> domestic_only{};
    std::optional<bool> level_two_supported{};
    std::optional<bool> level_three_supported{};
    std::optional<std::string> issuer_currency{};
    std::optional<std::string> combo_card{};
    std::optional<CardAuthenticationValue> authentication{};
    std::optional<CardCostValue> cost{};
    std::optional<bool> issuer_supports_tokenization{};
    std::optional<bool> billpay_enabled{};
    std::optional<bool> ecom_enabled{};
    std::optional<bool> flexible_credential_supported{};
    std::optional<std::string> card_subtype_code{};
    std::optional<std::string> multi_account_access_indicator{};
};

class CardInfoMigrateResponseRecord : public serde::Serializer<CardInfoMigrateResponseRecord>
{
public:
    void set_card_iin(std::optional<std::string> value)
    {
        card_iin = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_card_iin() const noexcept
    {
        return card_iin;
    }

    void set_card_issuer(std::optional<std::string> value)
    {
        card_issuer = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_card_issuer() const noexcept
    {
        return card_issuer;
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

    void set_card_sub_type(std::optional<std::string> value)
    {
        card_sub_type = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_card_sub_type() const noexcept
    {
        return card_sub_type;
    }

    void set_card_issuing_country(std::optional<std::string> value)
    {
        card_issuing_country = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_card_issuing_country() const noexcept
    {
        return card_issuing_country;
    }

    void set_country_code(std::optional<std::string> value)
    {
        country_code = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_country_code() const noexcept
    {
        return country_code;
    }

    void set_funding_source(std::optional<std::string> value)
    {
        funding_source = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_funding_source() const noexcept
    {
        return funding_source;
    }

    void set_card_iin_type(std::optional<std::string> value)
    {
        card_iin_type = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_card_iin_type() const noexcept
    {
        return card_iin_type;
    }

    void set_virtual_card(std::optional<bool> value)
    {
        virtual_card = std::move(value);
    }

    [[nodiscard]] const std::optional<bool>& get_virtual_card() const noexcept
    {
        return virtual_card;
    }

    void set_gambling_blocked(std::optional<bool> value)
    {
        gambling_blocked = std::move(value);
    }

    [[nodiscard]] const std::optional<bool>& get_gambling_blocked() const noexcept
    {
        return gambling_blocked;
    }

    void set_co_badged_card_networks(std::optional<CoBadgedCardNetworkMetadataValue> value)
    {
        co_badged_card_networks = std::move(value);
    }

    [[nodiscard]] const std::optional<CoBadgedCardNetworkMetadataValue>&
    get_co_badged_card_networks() const noexcept
    {
        return co_badged_card_networks;
    }

    void set_card_segment_type(std::optional<std::string> value)
    {
        card_segment_type = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_card_segment_type() const noexcept
    {
        return card_segment_type;
    }

    void set_numeric_country_code(std::optional<std::string> value)
    {
        numeric_country_code = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_numeric_country_code() const noexcept
    {
        return numeric_country_code;
    }

    void set_prepaid(std::optional<bool> value)
    {
        prepaid = std::move(value);
    }

    [[nodiscard]] const std::optional<bool>& get_prepaid() const noexcept
    {
        return prepaid;
    }

    void set_regulated(std::optional<bool> value)
    {
        regulated = std::move(value);
    }

    [[nodiscard]] const std::optional<bool>& get_regulated() const noexcept
    {
        return regulated;
    }

    void set_issuer_phone(std::optional<std::string> value)
    {
        issuer_phone = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_issuer_phone() const noexcept
    {
        return issuer_phone;
    }

    void set_issuer_url(std::optional<std::string> value)
    {
        issuer_url = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_issuer_url() const noexcept
    {
        return issuer_url;
    }

    void set_regulated_name(std::optional<std::string> value)
    {
        regulated_name = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_regulated_name() const noexcept
    {
        return regulated_name;
    }

    void set_reloadable_prepaid(std::optional<bool> value)
    {
        reloadable_prepaid = std::move(value);
    }

    [[nodiscard]] const std::optional<bool>& get_reloadable_prepaid() const noexcept
    {
        return reloadable_prepaid;
    }

    void set_account_updater(std::optional<bool> value)
    {
        account_updater = std::move(value);
    }

    [[nodiscard]] const std::optional<bool>& get_account_updater() const noexcept
    {
        return account_updater;
    }

    void set_account_level_management(std::optional<bool> value)
    {
        account_level_management = std::move(value);
    }

    [[nodiscard]] const std::optional<bool>& get_account_level_management() const noexcept
    {
        return account_level_management;
    }

    void set_domestic_only(std::optional<bool> value)
    {
        domestic_only = std::move(value);
    }

    [[nodiscard]] const std::optional<bool>& get_domestic_only() const noexcept
    {
        return domestic_only;
    }

    void set_level_two_supported(std::optional<bool> value)
    {
        level_two_supported = std::move(value);
    }

    [[nodiscard]] const std::optional<bool>& get_level_two_supported() const noexcept
    {
        return level_two_supported;
    }

    void set_level_three_supported(std::optional<bool> value)
    {
        level_three_supported = std::move(value);
    }

    [[nodiscard]] const std::optional<bool>& get_level_three_supported() const noexcept
    {
        return level_three_supported;
    }

    void set_issuer_currency(std::optional<std::string> value)
    {
        issuer_currency = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_issuer_currency() const noexcept
    {
        return issuer_currency;
    }

    void set_combo_card(std::optional<std::string> value)
    {
        combo_card = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_combo_card() const noexcept
    {
        return combo_card;
    }

    void set_authentication(std::optional<CardAuthenticationValue> value)
    {
        authentication = std::move(value);
    }

    [[nodiscard]] const std::optional<CardAuthenticationValue>& get_authentication() const noexcept
    {
        return authentication;
    }

    void set_cost(std::optional<CardCostValue> value)
    {
        cost = std::move(value);
    }

    [[nodiscard]] const std::optional<CardCostValue>& get_cost() const noexcept
    {
        return cost;
    }

    void set_issuer_supports_tokenization(std::optional<bool> value)
    {
        issuer_supports_tokenization = std::move(value);
    }

    [[nodiscard]] const std::optional<bool>& get_issuer_supports_tokenization() const noexcept
    {
        return issuer_supports_tokenization;
    }

    void set_billpay_enabled(std::optional<bool> value)
    {
        billpay_enabled = std::move(value);
    }

    [[nodiscard]] const std::optional<bool>& get_billpay_enabled() const noexcept
    {
        return billpay_enabled;
    }

    void set_ecom_enabled(std::optional<bool> value)
    {
        ecom_enabled = std::move(value);
    }

    [[nodiscard]] const std::optional<bool>& get_ecom_enabled() const noexcept
    {
        return ecom_enabled;
    }

    void set_flexible_credential_supported(std::optional<bool> value)
    {
        flexible_credential_supported = std::move(value);
    }

    [[nodiscard]] const std::optional<bool>& get_flexible_credential_supported() const noexcept
    {
        return flexible_credential_supported;
    }

    void set_card_subtype_code(std::optional<std::string> value)
    {
        card_subtype_code = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_card_subtype_code() const noexcept
    {
        return card_subtype_code;
    }

    void set_multi_account_access_indicator(std::optional<std::string> value)
    {
        multi_account_access_indicator = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>&
    get_multi_account_access_indicator() const noexcept
    {
        return multi_account_access_indicator;
    }

    std::optional<std::string> card_iin{};
    std::optional<std::string> card_issuer{};
    std::optional<std::string> card_network{};
    std::optional<std::string> card_type{};
    std::optional<std::string> card_sub_type{};
    std::optional<std::string> card_issuing_country{};
    std::optional<std::string> country_code{};
    std::optional<std::string> funding_source{};
    std::optional<std::string> card_iin_type{};
    std::optional<bool> virtual_card{};
    std::optional<bool> gambling_blocked{};
    std::optional<CoBadgedCardNetworkMetadataValue> co_badged_card_networks{};
    std::optional<std::string> card_segment_type{};
    std::optional<std::string> numeric_country_code{};
    std::optional<bool> prepaid{};
    std::optional<bool> regulated{};
    std::optional<std::string> issuer_phone{};
    std::optional<std::string> issuer_url{};
    std::optional<std::string> regulated_name{};
    std::optional<bool> reloadable_prepaid{};
    std::optional<bool> account_updater{};
    std::optional<bool> account_level_management{};
    std::optional<bool> domestic_only{};
    std::optional<bool> level_two_supported{};
    std::optional<bool> level_three_supported{};
    std::optional<std::string> issuer_currency{};
    std::optional<std::string> combo_card{};
    std::optional<CardAuthenticationValue> authentication{};
    std::optional<CardCostValue> cost{};
    std::optional<bool> issuer_supports_tokenization{};
    std::optional<bool> billpay_enabled{};
    std::optional<bool> ecom_enabled{};
    std::optional<bool> flexible_credential_supported{};
    std::optional<std::string> card_subtype_code{};
    std::optional<std::string> multi_account_access_indicator{};
};

class CardInfoMigrationResponse : public serde::Serializer<CardInfoMigrationResponse>
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

    void set_card_iin(std::string value)
    {
        card_iin = std::move(value);
    }

    [[nodiscard]] const std::string& get_card_iin() const noexcept
    {
        return card_iin;
    }

    void set_card_issuer(std::optional<std::string> value)
    {
        card_issuer = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_card_issuer() const noexcept
    {
        return card_issuer;
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

    void set_card_sub_type(std::optional<std::string> value)
    {
        card_sub_type = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_card_sub_type() const noexcept
    {
        return card_sub_type;
    }

    void set_card_issuing_country(std::optional<std::string> value)
    {
        card_issuing_country = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_card_issuing_country() const noexcept
    {
        return card_issuing_country;
    }

    void set_country_code(std::optional<std::string> value)
    {
        country_code = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_country_code() const noexcept
    {
        return country_code;
    }

    void set_funding_source(std::optional<std::string> value)
    {
        funding_source = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_funding_source() const noexcept
    {
        return funding_source;
    }

    void set_card_iin_type(std::optional<std::string> value)
    {
        card_iin_type = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_card_iin_type() const noexcept
    {
        return card_iin_type;
    }

    void set_virtual_card(std::optional<bool> value)
    {
        virtual_card = std::move(value);
    }

    [[nodiscard]] const std::optional<bool>& get_virtual_card() const noexcept
    {
        return virtual_card;
    }

    void set_gambling_blocked(std::optional<bool> value)
    {
        gambling_blocked = std::move(value);
    }

    [[nodiscard]] const std::optional<bool>& get_gambling_blocked() const noexcept
    {
        return gambling_blocked;
    }

    void set_co_badged_card_networks(std::optional<CoBadgedCardNetworkMetadataValue> value)
    {
        co_badged_card_networks = std::move(value);
    }

    [[nodiscard]] const std::optional<CoBadgedCardNetworkMetadataValue>&
    get_co_badged_card_networks() const noexcept
    {
        return co_badged_card_networks;
    }

    void set_card_segment_type(std::optional<std::string> value)
    {
        card_segment_type = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_card_segment_type() const noexcept
    {
        return card_segment_type;
    }

    void set_numeric_country_code(std::optional<std::string> value)
    {
        numeric_country_code = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_numeric_country_code() const noexcept
    {
        return numeric_country_code;
    }

    void set_prepaid(std::optional<bool> value)
    {
        prepaid = std::move(value);
    }

    [[nodiscard]] const std::optional<bool>& get_prepaid() const noexcept
    {
        return prepaid;
    }

    void set_regulated(std::optional<bool> value)
    {
        regulated = std::move(value);
    }

    [[nodiscard]] const std::optional<bool>& get_regulated() const noexcept
    {
        return regulated;
    }

    void set_issuer_phone(std::optional<std::string> value)
    {
        issuer_phone = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_issuer_phone() const noexcept
    {
        return issuer_phone;
    }

    void set_issuer_url(std::optional<std::string> value)
    {
        issuer_url = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_issuer_url() const noexcept
    {
        return issuer_url;
    }

    void set_regulated_name(std::optional<std::string> value)
    {
        regulated_name = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_regulated_name() const noexcept
    {
        return regulated_name;
    }

    void set_reloadable_prepaid(std::optional<bool> value)
    {
        reloadable_prepaid = std::move(value);
    }

    [[nodiscard]] const std::optional<bool>& get_reloadable_prepaid() const noexcept
    {
        return reloadable_prepaid;
    }

    void set_account_updater(std::optional<bool> value)
    {
        account_updater = std::move(value);
    }

    [[nodiscard]] const std::optional<bool>& get_account_updater() const noexcept
    {
        return account_updater;
    }

    void set_account_level_management(std::optional<bool> value)
    {
        account_level_management = std::move(value);
    }

    [[nodiscard]] const std::optional<bool>& get_account_level_management() const noexcept
    {
        return account_level_management;
    }

    void set_domestic_only(std::optional<bool> value)
    {
        domestic_only = std::move(value);
    }

    [[nodiscard]] const std::optional<bool>& get_domestic_only() const noexcept
    {
        return domestic_only;
    }

    void set_level_two_supported(std::optional<bool> value)
    {
        level_two_supported = std::move(value);
    }

    [[nodiscard]] const std::optional<bool>& get_level_two_supported() const noexcept
    {
        return level_two_supported;
    }

    void set_level_three_supported(std::optional<bool> value)
    {
        level_three_supported = std::move(value);
    }

    [[nodiscard]] const std::optional<bool>& get_level_three_supported() const noexcept
    {
        return level_three_supported;
    }

    void set_issuer_currency(std::optional<std::string> value)
    {
        issuer_currency = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_issuer_currency() const noexcept
    {
        return issuer_currency;
    }

    void set_combo_card(std::optional<std::string> value)
    {
        combo_card = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_combo_card() const noexcept
    {
        return combo_card;
    }

    void set_authentication(std::optional<CardAuthenticationValue> value)
    {
        authentication = std::move(value);
    }

    [[nodiscard]] const std::optional<CardAuthenticationValue>& get_authentication() const noexcept
    {
        return authentication;
    }

    void set_cost(std::optional<CardCostValue> value)
    {
        cost = std::move(value);
    }

    [[nodiscard]] const std::optional<CardCostValue>& get_cost() const noexcept
    {
        return cost;
    }

    void set_issuer_supports_tokenization(std::optional<bool> value)
    {
        issuer_supports_tokenization = std::move(value);
    }

    [[nodiscard]] const std::optional<bool>& get_issuer_supports_tokenization() const noexcept
    {
        return issuer_supports_tokenization;
    }

    void set_billpay_enabled(std::optional<bool> value)
    {
        billpay_enabled = std::move(value);
    }

    [[nodiscard]] const std::optional<bool>& get_billpay_enabled() const noexcept
    {
        return billpay_enabled;
    }

    void set_ecom_enabled(std::optional<bool> value)
    {
        ecom_enabled = std::move(value);
    }

    [[nodiscard]] const std::optional<bool>& get_ecom_enabled() const noexcept
    {
        return ecom_enabled;
    }

    void set_flexible_credential_supported(std::optional<bool> value)
    {
        flexible_credential_supported = std::move(value);
    }

    [[nodiscard]] const std::optional<bool>& get_flexible_credential_supported() const noexcept
    {
        return flexible_credential_supported;
    }

    void set_card_subtype_code(std::optional<std::string> value)
    {
        card_subtype_code = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_card_subtype_code() const noexcept
    {
        return card_subtype_code;
    }

    void set_multi_account_access_indicator(std::optional<std::string> value)
    {
        multi_account_access_indicator = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>&
    get_multi_account_access_indicator() const noexcept
    {
        return multi_account_access_indicator;
    }

    void set_migration_error(std::optional<std::string> value)
    {
        migration_error = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_migration_error() const noexcept
    {
        return migration_error;
    }

    void set_migration_status(CardInfoMigrationStatus value)
    {
        migration_status = std::move(value);
    }

    [[nodiscard]] const CardInfoMigrationStatus& get_migration_status() const noexcept
    {
        return migration_status;
    }

    std::optional<std::int64_t> line_number{};
    std::string card_iin{};
    std::optional<std::string> card_issuer{};
    std::optional<std::string> card_network{};
    std::optional<std::string> card_type{};
    std::optional<std::string> card_sub_type{};
    std::optional<std::string> card_issuing_country{};
    std::optional<std::string> country_code{};
    std::optional<std::string> funding_source{};
    std::optional<std::string> card_iin_type{};
    std::optional<bool> virtual_card{};
    std::optional<bool> gambling_blocked{};
    std::optional<CoBadgedCardNetworkMetadataValue> co_badged_card_networks{};
    std::optional<std::string> card_segment_type{};
    std::optional<std::string> numeric_country_code{};
    std::optional<bool> prepaid{};
    std::optional<bool> regulated{};
    std::optional<std::string> issuer_phone{};
    std::optional<std::string> issuer_url{};
    std::optional<std::string> regulated_name{};
    std::optional<bool> reloadable_prepaid{};
    std::optional<bool> account_updater{};
    std::optional<bool> account_level_management{};
    std::optional<bool> domestic_only{};
    std::optional<bool> level_two_supported{};
    std::optional<bool> level_three_supported{};
    std::optional<std::string> issuer_currency{};
    std::optional<std::string> combo_card{};
    std::optional<CardAuthenticationValue> authentication{};
    std::optional<CardCostValue> cost{};
    std::optional<bool> issuer_supports_tokenization{};
    std::optional<bool> billpay_enabled{};
    std::optional<bool> ecom_enabled{};
    std::optional<bool> flexible_credential_supported{};
    std::optional<std::string> card_subtype_code{};
    std::optional<std::string> multi_account_access_indicator{};
    std::optional<std::string> migration_error{};
    CardInfoMigrationStatus migration_status{};
};

class CardInfoResponse : public serde::Serializer<CardInfoResponse>
{
public:
    void set_card_iin(std::string value)
    {
        card_iin = std::move(value);
    }

    [[nodiscard]] const std::string& get_card_iin() const noexcept
    {
        return card_iin;
    }

    void set_card_issuer(std::optional<std::string> value)
    {
        card_issuer = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_card_issuer() const noexcept
    {
        return card_issuer;
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

    void set_card_sub_type(std::optional<std::string> value)
    {
        card_sub_type = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_card_sub_type() const noexcept
    {
        return card_sub_type;
    }

    void set_card_issuing_country(std::optional<std::string> value)
    {
        card_issuing_country = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_card_issuing_country() const noexcept
    {
        return card_issuing_country;
    }

    void set_country_code(std::optional<std::string> value)
    {
        country_code = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_country_code() const noexcept
    {
        return country_code;
    }

    void set_funding_source(std::optional<std::string> value)
    {
        funding_source = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_funding_source() const noexcept
    {
        return funding_source;
    }

    void set_card_iin_type(std::optional<std::string> value)
    {
        card_iin_type = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_card_iin_type() const noexcept
    {
        return card_iin_type;
    }

    void set_virtual_card(std::optional<bool> value)
    {
        virtual_card = std::move(value);
    }

    [[nodiscard]] const std::optional<bool>& get_virtual_card() const noexcept
    {
        return virtual_card;
    }

    void set_gambling_blocked(std::optional<bool> value)
    {
        gambling_blocked = std::move(value);
    }

    [[nodiscard]] const std::optional<bool>& get_gambling_blocked() const noexcept
    {
        return gambling_blocked;
    }

    void set_co_badged_card_networks(std::optional<CoBadgedCardNetworkMetadataValue> value)
    {
        co_badged_card_networks = std::move(value);
    }

    [[nodiscard]] const std::optional<CoBadgedCardNetworkMetadataValue>&
    get_co_badged_card_networks() const noexcept
    {
        return co_badged_card_networks;
    }

    void set_card_segment_type(std::optional<std::string> value)
    {
        card_segment_type = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_card_segment_type() const noexcept
    {
        return card_segment_type;
    }

    void set_numeric_country_code(std::optional<std::string> value)
    {
        numeric_country_code = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_numeric_country_code() const noexcept
    {
        return numeric_country_code;
    }

    void set_prepaid(std::optional<bool> value)
    {
        prepaid = std::move(value);
    }

    [[nodiscard]] const std::optional<bool>& get_prepaid() const noexcept
    {
        return prepaid;
    }

    void set_regulated(std::optional<bool> value)
    {
        regulated = std::move(value);
    }

    [[nodiscard]] const std::optional<bool>& get_regulated() const noexcept
    {
        return regulated;
    }

    void set_issuer_phone(std::optional<std::string> value)
    {
        issuer_phone = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_issuer_phone() const noexcept
    {
        return issuer_phone;
    }

    void set_issuer_url(std::optional<std::string> value)
    {
        issuer_url = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_issuer_url() const noexcept
    {
        return issuer_url;
    }

    void set_regulated_name(std::optional<std::string> value)
    {
        regulated_name = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_regulated_name() const noexcept
    {
        return regulated_name;
    }

    void set_reloadable_prepaid(std::optional<bool> value)
    {
        reloadable_prepaid = std::move(value);
    }

    [[nodiscard]] const std::optional<bool>& get_reloadable_prepaid() const noexcept
    {
        return reloadable_prepaid;
    }

    void set_account_updater(std::optional<bool> value)
    {
        account_updater = std::move(value);
    }

    [[nodiscard]] const std::optional<bool>& get_account_updater() const noexcept
    {
        return account_updater;
    }

    void set_account_level_management(std::optional<bool> value)
    {
        account_level_management = std::move(value);
    }

    [[nodiscard]] const std::optional<bool>& get_account_level_management() const noexcept
    {
        return account_level_management;
    }

    void set_domestic_only(std::optional<bool> value)
    {
        domestic_only = std::move(value);
    }

    [[nodiscard]] const std::optional<bool>& get_domestic_only() const noexcept
    {
        return domestic_only;
    }

    void set_level_two_supported(std::optional<bool> value)
    {
        level_two_supported = std::move(value);
    }

    [[nodiscard]] const std::optional<bool>& get_level_two_supported() const noexcept
    {
        return level_two_supported;
    }

    void set_level_three_supported(std::optional<bool> value)
    {
        level_three_supported = std::move(value);
    }

    [[nodiscard]] const std::optional<bool>& get_level_three_supported() const noexcept
    {
        return level_three_supported;
    }

    void set_issuer_currency(std::optional<std::string> value)
    {
        issuer_currency = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_issuer_currency() const noexcept
    {
        return issuer_currency;
    }

    void set_combo_card(std::optional<std::string> value)
    {
        combo_card = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_combo_card() const noexcept
    {
        return combo_card;
    }

    void set_authentication(std::optional<CardAuthenticationValue> value)
    {
        authentication = std::move(value);
    }

    [[nodiscard]] const std::optional<CardAuthenticationValue>& get_authentication() const noexcept
    {
        return authentication;
    }

    void set_cost(std::optional<CardCostValue> value)
    {
        cost = std::move(value);
    }

    [[nodiscard]] const std::optional<CardCostValue>& get_cost() const noexcept
    {
        return cost;
    }

    void set_issuer_supports_tokenization(std::optional<bool> value)
    {
        issuer_supports_tokenization = std::move(value);
    }

    [[nodiscard]] const std::optional<bool>& get_issuer_supports_tokenization() const noexcept
    {
        return issuer_supports_tokenization;
    }

    void set_billpay_enabled(std::optional<bool> value)
    {
        billpay_enabled = std::move(value);
    }

    [[nodiscard]] const std::optional<bool>& get_billpay_enabled() const noexcept
    {
        return billpay_enabled;
    }

    void set_ecom_enabled(std::optional<bool> value)
    {
        ecom_enabled = std::move(value);
    }

    [[nodiscard]] const std::optional<bool>& get_ecom_enabled() const noexcept
    {
        return ecom_enabled;
    }

    void set_flexible_credential_supported(std::optional<bool> value)
    {
        flexible_credential_supported = std::move(value);
    }

    [[nodiscard]] const std::optional<bool>& get_flexible_credential_supported() const noexcept
    {
        return flexible_credential_supported;
    }

    void set_card_subtype_code(std::optional<std::string> value)
    {
        card_subtype_code = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_card_subtype_code() const noexcept
    {
        return card_subtype_code;
    }

    void set_multi_account_access_indicator(std::optional<std::string> value)
    {
        multi_account_access_indicator = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>&
    get_multi_account_access_indicator() const noexcept
    {
        return multi_account_access_indicator;
    }

    std::string card_iin{};
    std::optional<std::string> card_issuer{};
    std::optional<std::string> card_network{};
    std::optional<std::string> card_type{};
    std::optional<std::string> card_sub_type{};
    std::optional<std::string> card_issuing_country{};
    std::optional<std::string> country_code{};
    std::optional<std::string> funding_source{};
    std::optional<std::string> card_iin_type{};
    std::optional<bool> virtual_card{};
    std::optional<bool> gambling_blocked{};
    std::optional<CoBadgedCardNetworkMetadataValue> co_badged_card_networks{};
    std::optional<std::string> card_segment_type{};
    std::optional<std::string> numeric_country_code{};
    std::optional<bool> prepaid{};
    std::optional<bool> regulated{};
    std::optional<std::string> issuer_phone{};
    std::optional<std::string> issuer_url{};
    std::optional<std::string> regulated_name{};
    std::optional<bool> reloadable_prepaid{};
    std::optional<bool> account_updater{};
    std::optional<bool> account_level_management{};
    std::optional<bool> domestic_only{};
    std::optional<bool> level_two_supported{};
    std::optional<bool> level_three_supported{};
    std::optional<std::string> issuer_currency{};
    std::optional<std::string> combo_card{};
    std::optional<CardAuthenticationValue> authentication{};
    std::optional<CardCostValue> cost{};
    std::optional<bool> issuer_supports_tokenization{};
    std::optional<bool> billpay_enabled{};
    std::optional<bool> ecom_enabled{};
    std::optional<bool> flexible_credential_supported{};
    std::optional<std::string> card_subtype_code{};
    std::optional<std::string> multi_account_access_indicator{};
};

class CardInfoUpdateRequest : public serde::Serializer<CardInfoUpdateRequest>
{
public:
    void set_card_iin(std::string value)
    {
        card_iin = std::move(value);
    }

    [[nodiscard]] const std::string& get_card_iin() const noexcept
    {
        return card_iin;
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

    void set_card_issuing_country(std::optional<std::string> value)
    {
        card_issuing_country = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_card_issuing_country() const noexcept
    {
        return card_issuing_country;
    }

    void set_bank_code_id(std::optional<std::string> value)
    {
        bank_code_id = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_bank_code_id() const noexcept
    {
        return bank_code_id;
    }

    void set_bank_code(std::optional<std::string> value)
    {
        bank_code = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_bank_code() const noexcept
    {
        return bank_code;
    }

    void set_country_code(std::optional<std::string> value)
    {
        country_code = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_country_code() const noexcept
    {
        return country_code;
    }

    void set_last_updated_provider(std::optional<std::string> value)
    {
        last_updated_provider = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_last_updated_provider() const noexcept
    {
        return last_updated_provider;
    }

    void set_funding_source(std::optional<FundingSourceValue> value)
    {
        funding_source = std::move(value);
    }

    [[nodiscard]] const std::optional<FundingSourceValue>& get_funding_source() const noexcept
    {
        return funding_source;
    }

    void set_card_iin_type(std::optional<PanOrTokenValue> value)
    {
        card_iin_type = std::move(value);
    }

    [[nodiscard]] const std::optional<PanOrTokenValue>& get_card_iin_type() const noexcept
    {
        return card_iin_type;
    }

    void set_virtual_card(std::optional<bool> value)
    {
        virtual_card = std::move(value);
    }

    [[nodiscard]] const std::optional<bool>& get_virtual_card() const noexcept
    {
        return virtual_card;
    }

    void set_gambling_blocked(std::optional<bool> value)
    {
        gambling_blocked = std::move(value);
    }

    [[nodiscard]] const std::optional<bool>& get_gambling_blocked() const noexcept
    {
        return gambling_blocked;
    }

    void set_co_badged_card_networks(std::optional<CoBadgedCardNetworkMetadataValue> value)
    {
        co_badged_card_networks = std::move(value);
    }

    [[nodiscard]] const std::optional<CoBadgedCardNetworkMetadataValue>&
    get_co_badged_card_networks() const noexcept
    {
        return co_badged_card_networks;
    }

    void set_card_segment_type(std::optional<std::string> value)
    {
        card_segment_type = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_card_segment_type() const noexcept
    {
        return card_segment_type;
    }

    void set_numeric_country_code(std::optional<std::string> value)
    {
        numeric_country_code = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_numeric_country_code() const noexcept
    {
        return numeric_country_code;
    }

    void set_prepaid(std::optional<bool> value)
    {
        prepaid = std::move(value);
    }

    [[nodiscard]] const std::optional<bool>& get_prepaid() const noexcept
    {
        return prepaid;
    }

    void set_regulated(std::optional<bool> value)
    {
        regulated = std::move(value);
    }

    [[nodiscard]] const std::optional<bool>& get_regulated() const noexcept
    {
        return regulated;
    }

    void set_issuer_phone(std::optional<std::string> value)
    {
        issuer_phone = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_issuer_phone() const noexcept
    {
        return issuer_phone;
    }

    void set_issuer_url(std::optional<std::string> value)
    {
        issuer_url = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_issuer_url() const noexcept
    {
        return issuer_url;
    }

    void set_regulated_name(std::optional<std::string> value)
    {
        regulated_name = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_regulated_name() const noexcept
    {
        return regulated_name;
    }

    void set_reloadable_prepaid(std::optional<bool> value)
    {
        reloadable_prepaid = std::move(value);
    }

    [[nodiscard]] const std::optional<bool>& get_reloadable_prepaid() const noexcept
    {
        return reloadable_prepaid;
    }

    void set_account_updater(std::optional<bool> value)
    {
        account_updater = std::move(value);
    }

    [[nodiscard]] const std::optional<bool>& get_account_updater() const noexcept
    {
        return account_updater;
    }

    void set_account_level_management(std::optional<bool> value)
    {
        account_level_management = std::move(value);
    }

    [[nodiscard]] const std::optional<bool>& get_account_level_management() const noexcept
    {
        return account_level_management;
    }

    void set_domestic_only(std::optional<bool> value)
    {
        domestic_only = std::move(value);
    }

    [[nodiscard]] const std::optional<bool>& get_domestic_only() const noexcept
    {
        return domestic_only;
    }

    void set_level_two_supported(std::optional<bool> value)
    {
        level_two_supported = std::move(value);
    }

    [[nodiscard]] const std::optional<bool>& get_level_two_supported() const noexcept
    {
        return level_two_supported;
    }

    void set_level_three_supported(std::optional<bool> value)
    {
        level_three_supported = std::move(value);
    }

    [[nodiscard]] const std::optional<bool>& get_level_three_supported() const noexcept
    {
        return level_three_supported;
    }

    void set_issuer_currency(std::optional<std::string> value)
    {
        issuer_currency = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_issuer_currency() const noexcept
    {
        return issuer_currency;
    }

    void set_combo_card(std::optional<std::string> value)
    {
        combo_card = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_combo_card() const noexcept
    {
        return combo_card;
    }

    void set_authentication(std::optional<CardAuthenticationValue> value)
    {
        authentication = std::move(value);
    }

    [[nodiscard]] const std::optional<CardAuthenticationValue>& get_authentication() const noexcept
    {
        return authentication;
    }

    void set_cost(std::optional<CardCostValue> value)
    {
        cost = std::move(value);
    }

    [[nodiscard]] const std::optional<CardCostValue>& get_cost() const noexcept
    {
        return cost;
    }

    void set_issuer_supports_tokenization(std::optional<bool> value)
    {
        issuer_supports_tokenization = std::move(value);
    }

    [[nodiscard]] const std::optional<bool>& get_issuer_supports_tokenization() const noexcept
    {
        return issuer_supports_tokenization;
    }

    void set_billpay_enabled(std::optional<bool> value)
    {
        billpay_enabled = std::move(value);
    }

    [[nodiscard]] const std::optional<bool>& get_billpay_enabled() const noexcept
    {
        return billpay_enabled;
    }

    void set_ecom_enabled(std::optional<bool> value)
    {
        ecom_enabled = std::move(value);
    }

    [[nodiscard]] const std::optional<bool>& get_ecom_enabled() const noexcept
    {
        return ecom_enabled;
    }

    void set_flexible_credential_supported(std::optional<bool> value)
    {
        flexible_credential_supported = std::move(value);
    }

    [[nodiscard]] const std::optional<bool>& get_flexible_credential_supported() const noexcept
    {
        return flexible_credential_supported;
    }

    void set_card_subtype_code(std::optional<std::string> value)
    {
        card_subtype_code = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_card_subtype_code() const noexcept
    {
        return card_subtype_code;
    }

    void set_multi_account_access_indicator(std::optional<std::string> value)
    {
        multi_account_access_indicator = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>&
    get_multi_account_access_indicator() const noexcept
    {
        return multi_account_access_indicator;
    }

    void set_line_number(std::optional<std::int64_t> value)
    {
        line_number = std::move(value);
    }

    [[nodiscard]] const std::optional<std::int64_t>& get_line_number() const noexcept
    {
        return line_number;
    }

    std::string card_iin{};
    std::optional<std::string> card_issuer{};
    std::optional<CardNetworkValue> card_network{};
    std::optional<std::string> card_type{};
    std::optional<std::string> card_subtype{};
    std::optional<std::string> card_issuing_country{};
    std::optional<std::string> bank_code_id{};
    std::optional<std::string> bank_code{};
    std::optional<std::string> country_code{};
    std::optional<std::string> last_updated_provider{};
    std::optional<FundingSourceValue> funding_source{};
    std::optional<PanOrTokenValue> card_iin_type{};
    std::optional<bool> virtual_card{};
    std::optional<bool> gambling_blocked{};
    std::optional<CoBadgedCardNetworkMetadataValue> co_badged_card_networks{};
    std::optional<std::string> card_segment_type{};
    std::optional<std::string> numeric_country_code{};
    std::optional<bool> prepaid{};
    std::optional<bool> regulated{};
    std::optional<std::string> issuer_phone{};
    std::optional<std::string> issuer_url{};
    std::optional<std::string> regulated_name{};
    std::optional<bool> reloadable_prepaid{};
    std::optional<bool> account_updater{};
    std::optional<bool> account_level_management{};
    std::optional<bool> domestic_only{};
    std::optional<bool> level_two_supported{};
    std::optional<bool> level_three_supported{};
    std::optional<std::string> issuer_currency{};
    std::optional<std::string> combo_card{};
    std::optional<CardAuthenticationValue> authentication{};
    std::optional<CardCostValue> cost{};
    std::optional<bool> issuer_supports_tokenization{};
    std::optional<bool> billpay_enabled{};
    std::optional<bool> ecom_enabled{};
    std::optional<bool> flexible_credential_supported{};
    std::optional<std::string> card_subtype_code{};
    std::optional<std::string> multi_account_access_indicator{};
    std::optional<std::int64_t> line_number{};
};
} // namespace app::model::cards_info

template<>
struct serde::Serializable<app::model::cards_info::CardsInfoRequestParams>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<
            "client_secret",
            &app::model::cards_info::CardsInfoRequestParams::get_client_secret,
            &app::model::cards_info::CardsInfoRequestParams::set_client_secret,
            serde::FieldOptions{}>{}};
    }

    static constexpr std::string_view table_name()
    {
        return "cards_info_request_params";
    }
};

template<>
struct serde::Serializable<app::model::cards_info::CardsInfoRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "client_secret",
                &app::model::cards_info::CardsInfoRequest::get_client_secret,
                &app::model::cards_info::CardsInfoRequest::set_client_secret,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_iin",
                &app::model::cards_info::CardsInfoRequest::get_card_iin,
                &app::model::cards_info::CardsInfoRequest::set_card_iin,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "cards_info_request";
    }
};

template<>
struct serde::Serializable<app::model::cards_info::CardInfoResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "card_iin",
                &app::model::cards_info::CardInfoResponse::get_card_iin,
                &app::model::cards_info::CardInfoResponse::set_card_iin,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_issuer",
                &app::model::cards_info::CardInfoResponse::get_card_issuer,
                &app::model::cards_info::CardInfoResponse::set_card_issuer,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_network",
                &app::model::cards_info::CardInfoResponse::get_card_network,
                &app::model::cards_info::CardInfoResponse::set_card_network,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_type",
                &app::model::cards_info::CardInfoResponse::get_card_type,
                &app::model::cards_info::CardInfoResponse::set_card_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_sub_type",
                &app::model::cards_info::CardInfoResponse::get_card_sub_type,
                &app::model::cards_info::CardInfoResponse::set_card_sub_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_issuing_country",
                &app::model::cards_info::CardInfoResponse::get_card_issuing_country,
                &app::model::cards_info::CardInfoResponse::set_card_issuing_country,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "country_code",
                &app::model::cards_info::CardInfoResponse::get_country_code,
                &app::model::cards_info::CardInfoResponse::set_country_code,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "funding_source",
                &app::model::cards_info::CardInfoResponse::get_funding_source,
                &app::model::cards_info::CardInfoResponse::set_funding_source,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_iin_type",
                &app::model::cards_info::CardInfoResponse::get_card_iin_type,
                &app::model::cards_info::CardInfoResponse::set_card_iin_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "virtual_card",
                &app::model::cards_info::CardInfoResponse::get_virtual_card,
                &app::model::cards_info::CardInfoResponse::set_virtual_card,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "gambling_blocked",
                &app::model::cards_info::CardInfoResponse::get_gambling_blocked,
                &app::model::cards_info::CardInfoResponse::set_gambling_blocked,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "co_badged_card_networks",
                &app::model::cards_info::CardInfoResponse::get_co_badged_card_networks,
                &app::model::cards_info::CardInfoResponse::set_co_badged_card_networks,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_segment_type",
                &app::model::cards_info::CardInfoResponse::get_card_segment_type,
                &app::model::cards_info::CardInfoResponse::set_card_segment_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "numeric_country_code",
                &app::model::cards_info::CardInfoResponse::get_numeric_country_code,
                &app::model::cards_info::CardInfoResponse::set_numeric_country_code,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "prepaid",
                &app::model::cards_info::CardInfoResponse::get_prepaid,
                &app::model::cards_info::CardInfoResponse::set_prepaid,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "regulated",
                &app::model::cards_info::CardInfoResponse::get_regulated,
                &app::model::cards_info::CardInfoResponse::set_regulated,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "issuer_phone",
                &app::model::cards_info::CardInfoResponse::get_issuer_phone,
                &app::model::cards_info::CardInfoResponse::set_issuer_phone,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "issuer_url",
                &app::model::cards_info::CardInfoResponse::get_issuer_url,
                &app::model::cards_info::CardInfoResponse::set_issuer_url,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "regulated_name",
                &app::model::cards_info::CardInfoResponse::get_regulated_name,
                &app::model::cards_info::CardInfoResponse::set_regulated_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "reloadable_prepaid",
                &app::model::cards_info::CardInfoResponse::get_reloadable_prepaid,
                &app::model::cards_info::CardInfoResponse::set_reloadable_prepaid,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "account_updater",
                &app::model::cards_info::CardInfoResponse::get_account_updater,
                &app::model::cards_info::CardInfoResponse::set_account_updater,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "account_level_management",
                &app::model::cards_info::CardInfoResponse::get_account_level_management,
                &app::model::cards_info::CardInfoResponse::set_account_level_management,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "domestic_only",
                &app::model::cards_info::CardInfoResponse::get_domestic_only,
                &app::model::cards_info::CardInfoResponse::set_domestic_only,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "level_two_supported",
                &app::model::cards_info::CardInfoResponse::get_level_two_supported,
                &app::model::cards_info::CardInfoResponse::set_level_two_supported,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "level_three_supported",
                &app::model::cards_info::CardInfoResponse::get_level_three_supported,
                &app::model::cards_info::CardInfoResponse::set_level_three_supported,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "issuer_currency",
                &app::model::cards_info::CardInfoResponse::get_issuer_currency,
                &app::model::cards_info::CardInfoResponse::set_issuer_currency,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "combo_card",
                &app::model::cards_info::CardInfoResponse::get_combo_card,
                &app::model::cards_info::CardInfoResponse::set_combo_card,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "authentication",
                &app::model::cards_info::CardInfoResponse::get_authentication,
                &app::model::cards_info::CardInfoResponse::set_authentication,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "cost",
                &app::model::cards_info::CardInfoResponse::get_cost,
                &app::model::cards_info::CardInfoResponse::set_cost,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "issuer_supports_tokenization",
                &app::model::cards_info::CardInfoResponse::get_issuer_supports_tokenization,
                &app::model::cards_info::CardInfoResponse::set_issuer_supports_tokenization,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "billpay_enabled",
                &app::model::cards_info::CardInfoResponse::get_billpay_enabled,
                &app::model::cards_info::CardInfoResponse::set_billpay_enabled,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "ecom_enabled",
                &app::model::cards_info::CardInfoResponse::get_ecom_enabled,
                &app::model::cards_info::CardInfoResponse::set_ecom_enabled,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "flexible_credential_supported",
                &app::model::cards_info::CardInfoResponse::get_flexible_credential_supported,
                &app::model::cards_info::CardInfoResponse::set_flexible_credential_supported,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_subtype_code",
                &app::model::cards_info::CardInfoResponse::get_card_subtype_code,
                &app::model::cards_info::CardInfoResponse::set_card_subtype_code,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "multi_account_access_indicator",
                &app::model::cards_info::CardInfoResponse::get_multi_account_access_indicator,
                &app::model::cards_info::CardInfoResponse::set_multi_account_access_indicator,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "card_info_response";
    }
};

template<>
struct serde::Serializable<app::model::cards_info::CardInfoMigrateResponseRecord>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "card_iin",
                &app::model::cards_info::CardInfoMigrateResponseRecord::get_card_iin,
                &app::model::cards_info::CardInfoMigrateResponseRecord::set_card_iin,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_issuer",
                &app::model::cards_info::CardInfoMigrateResponseRecord::get_card_issuer,
                &app::model::cards_info::CardInfoMigrateResponseRecord::set_card_issuer,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_network",
                &app::model::cards_info::CardInfoMigrateResponseRecord::get_card_network,
                &app::model::cards_info::CardInfoMigrateResponseRecord::set_card_network,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_type",
                &app::model::cards_info::CardInfoMigrateResponseRecord::get_card_type,
                &app::model::cards_info::CardInfoMigrateResponseRecord::set_card_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_sub_type",
                &app::model::cards_info::CardInfoMigrateResponseRecord::get_card_sub_type,
                &app::model::cards_info::CardInfoMigrateResponseRecord::set_card_sub_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_issuing_country",
                &app::model::cards_info::CardInfoMigrateResponseRecord::get_card_issuing_country,
                &app::model::cards_info::CardInfoMigrateResponseRecord::set_card_issuing_country,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "country_code",
                &app::model::cards_info::CardInfoMigrateResponseRecord::get_country_code,
                &app::model::cards_info::CardInfoMigrateResponseRecord::set_country_code,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "funding_source",
                &app::model::cards_info::CardInfoMigrateResponseRecord::get_funding_source,
                &app::model::cards_info::CardInfoMigrateResponseRecord::set_funding_source,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_iin_type",
                &app::model::cards_info::CardInfoMigrateResponseRecord::get_card_iin_type,
                &app::model::cards_info::CardInfoMigrateResponseRecord::set_card_iin_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "virtual_card",
                &app::model::cards_info::CardInfoMigrateResponseRecord::get_virtual_card,
                &app::model::cards_info::CardInfoMigrateResponseRecord::set_virtual_card,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "gambling_blocked",
                &app::model::cards_info::CardInfoMigrateResponseRecord::get_gambling_blocked,
                &app::model::cards_info::CardInfoMigrateResponseRecord::set_gambling_blocked,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "co_badged_card_networks",
                &app::model::cards_info::CardInfoMigrateResponseRecord::get_co_badged_card_networks,
                &app::model::cards_info::CardInfoMigrateResponseRecord::set_co_badged_card_networks,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_segment_type",
                &app::model::cards_info::CardInfoMigrateResponseRecord::get_card_segment_type,
                &app::model::cards_info::CardInfoMigrateResponseRecord::set_card_segment_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "numeric_country_code",
                &app::model::cards_info::CardInfoMigrateResponseRecord::get_numeric_country_code,
                &app::model::cards_info::CardInfoMigrateResponseRecord::set_numeric_country_code,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "prepaid",
                &app::model::cards_info::CardInfoMigrateResponseRecord::get_prepaid,
                &app::model::cards_info::CardInfoMigrateResponseRecord::set_prepaid,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "regulated",
                &app::model::cards_info::CardInfoMigrateResponseRecord::get_regulated,
                &app::model::cards_info::CardInfoMigrateResponseRecord::set_regulated,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "issuer_phone",
                &app::model::cards_info::CardInfoMigrateResponseRecord::get_issuer_phone,
                &app::model::cards_info::CardInfoMigrateResponseRecord::set_issuer_phone,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "issuer_url",
                &app::model::cards_info::CardInfoMigrateResponseRecord::get_issuer_url,
                &app::model::cards_info::CardInfoMigrateResponseRecord::set_issuer_url,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "regulated_name",
                &app::model::cards_info::CardInfoMigrateResponseRecord::get_regulated_name,
                &app::model::cards_info::CardInfoMigrateResponseRecord::set_regulated_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "reloadable_prepaid",
                &app::model::cards_info::CardInfoMigrateResponseRecord::get_reloadable_prepaid,
                &app::model::cards_info::CardInfoMigrateResponseRecord::set_reloadable_prepaid,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "account_updater",
                &app::model::cards_info::CardInfoMigrateResponseRecord::get_account_updater,
                &app::model::cards_info::CardInfoMigrateResponseRecord::set_account_updater,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "account_level_management",
                &app::model::cards_info::CardInfoMigrateResponseRecord::
                    get_account_level_management,
                &app::model::cards_info::CardInfoMigrateResponseRecord::
                    set_account_level_management,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "domestic_only",
                &app::model::cards_info::CardInfoMigrateResponseRecord::get_domestic_only,
                &app::model::cards_info::CardInfoMigrateResponseRecord::set_domestic_only,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "level_two_supported",
                &app::model::cards_info::CardInfoMigrateResponseRecord::get_level_two_supported,
                &app::model::cards_info::CardInfoMigrateResponseRecord::set_level_two_supported,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "level_three_supported",
                &app::model::cards_info::CardInfoMigrateResponseRecord::get_level_three_supported,
                &app::model::cards_info::CardInfoMigrateResponseRecord::set_level_three_supported,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "issuer_currency",
                &app::model::cards_info::CardInfoMigrateResponseRecord::get_issuer_currency,
                &app::model::cards_info::CardInfoMigrateResponseRecord::set_issuer_currency,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "combo_card",
                &app::model::cards_info::CardInfoMigrateResponseRecord::get_combo_card,
                &app::model::cards_info::CardInfoMigrateResponseRecord::set_combo_card,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "authentication",
                &app::model::cards_info::CardInfoMigrateResponseRecord::get_authentication,
                &app::model::cards_info::CardInfoMigrateResponseRecord::set_authentication,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "cost",
                &app::model::cards_info::CardInfoMigrateResponseRecord::get_cost,
                &app::model::cards_info::CardInfoMigrateResponseRecord::set_cost,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "issuer_supports_tokenization",
                &app::model::cards_info::CardInfoMigrateResponseRecord::
                    get_issuer_supports_tokenization,
                &app::model::cards_info::CardInfoMigrateResponseRecord::
                    set_issuer_supports_tokenization,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "billpay_enabled",
                &app::model::cards_info::CardInfoMigrateResponseRecord::get_billpay_enabled,
                &app::model::cards_info::CardInfoMigrateResponseRecord::set_billpay_enabled,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "ecom_enabled",
                &app::model::cards_info::CardInfoMigrateResponseRecord::get_ecom_enabled,
                &app::model::cards_info::CardInfoMigrateResponseRecord::set_ecom_enabled,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "flexible_credential_supported",
                &app::model::cards_info::CardInfoMigrateResponseRecord::
                    get_flexible_credential_supported,
                &app::model::cards_info::CardInfoMigrateResponseRecord::
                    set_flexible_credential_supported,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_subtype_code",
                &app::model::cards_info::CardInfoMigrateResponseRecord::get_card_subtype_code,
                &app::model::cards_info::CardInfoMigrateResponseRecord::set_card_subtype_code,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "multi_account_access_indicator",
                &app::model::cards_info::CardInfoMigrateResponseRecord::
                    get_multi_account_access_indicator,
                &app::model::cards_info::CardInfoMigrateResponseRecord::
                    set_multi_account_access_indicator,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "card_info_migrate_response_record";
    }
};

template<>
struct serde::Serializable<app::model::cards_info::CardInfoCreateRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "card_iin",
                &app::model::cards_info::CardInfoCreateRequest::get_card_iin,
                &app::model::cards_info::CardInfoCreateRequest::set_card_iin,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_issuer",
                &app::model::cards_info::CardInfoCreateRequest::get_card_issuer,
                &app::model::cards_info::CardInfoCreateRequest::set_card_issuer,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_network",
                &app::model::cards_info::CardInfoCreateRequest::get_card_network,
                &app::model::cards_info::CardInfoCreateRequest::set_card_network,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_type",
                &app::model::cards_info::CardInfoCreateRequest::get_card_type,
                &app::model::cards_info::CardInfoCreateRequest::set_card_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_subtype",
                &app::model::cards_info::CardInfoCreateRequest::get_card_subtype,
                &app::model::cards_info::CardInfoCreateRequest::set_card_subtype,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_issuing_country",
                &app::model::cards_info::CardInfoCreateRequest::get_card_issuing_country,
                &app::model::cards_info::CardInfoCreateRequest::set_card_issuing_country,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "bank_code_id",
                &app::model::cards_info::CardInfoCreateRequest::get_bank_code_id,
                &app::model::cards_info::CardInfoCreateRequest::set_bank_code_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "bank_code",
                &app::model::cards_info::CardInfoCreateRequest::get_bank_code,
                &app::model::cards_info::CardInfoCreateRequest::set_bank_code,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "country_code",
                &app::model::cards_info::CardInfoCreateRequest::get_country_code,
                &app::model::cards_info::CardInfoCreateRequest::set_country_code,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "last_updated_provider",
                &app::model::cards_info::CardInfoCreateRequest::get_last_updated_provider,
                &app::model::cards_info::CardInfoCreateRequest::set_last_updated_provider,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "funding_source",
                &app::model::cards_info::CardInfoCreateRequest::get_funding_source,
                &app::model::cards_info::CardInfoCreateRequest::set_funding_source,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_iin_type",
                &app::model::cards_info::CardInfoCreateRequest::get_card_iin_type,
                &app::model::cards_info::CardInfoCreateRequest::set_card_iin_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "virtual_card",
                &app::model::cards_info::CardInfoCreateRequest::get_virtual_card,
                &app::model::cards_info::CardInfoCreateRequest::set_virtual_card,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "gambling_blocked",
                &app::model::cards_info::CardInfoCreateRequest::get_gambling_blocked,
                &app::model::cards_info::CardInfoCreateRequest::set_gambling_blocked,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "co_badged_card_networks",
                &app::model::cards_info::CardInfoCreateRequest::get_co_badged_card_networks,
                &app::model::cards_info::CardInfoCreateRequest::set_co_badged_card_networks,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_segment_type",
                &app::model::cards_info::CardInfoCreateRequest::get_card_segment_type,
                &app::model::cards_info::CardInfoCreateRequest::set_card_segment_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "numeric_country_code",
                &app::model::cards_info::CardInfoCreateRequest::get_numeric_country_code,
                &app::model::cards_info::CardInfoCreateRequest::set_numeric_country_code,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "prepaid",
                &app::model::cards_info::CardInfoCreateRequest::get_prepaid,
                &app::model::cards_info::CardInfoCreateRequest::set_prepaid,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "regulated",
                &app::model::cards_info::CardInfoCreateRequest::get_regulated,
                &app::model::cards_info::CardInfoCreateRequest::set_regulated,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "issuer_phone",
                &app::model::cards_info::CardInfoCreateRequest::get_issuer_phone,
                &app::model::cards_info::CardInfoCreateRequest::set_issuer_phone,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "issuer_url",
                &app::model::cards_info::CardInfoCreateRequest::get_issuer_url,
                &app::model::cards_info::CardInfoCreateRequest::set_issuer_url,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "regulated_name",
                &app::model::cards_info::CardInfoCreateRequest::get_regulated_name,
                &app::model::cards_info::CardInfoCreateRequest::set_regulated_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "reloadable_prepaid",
                &app::model::cards_info::CardInfoCreateRequest::get_reloadable_prepaid,
                &app::model::cards_info::CardInfoCreateRequest::set_reloadable_prepaid,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "account_updater",
                &app::model::cards_info::CardInfoCreateRequest::get_account_updater,
                &app::model::cards_info::CardInfoCreateRequest::set_account_updater,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "account_level_management",
                &app::model::cards_info::CardInfoCreateRequest::get_account_level_management,
                &app::model::cards_info::CardInfoCreateRequest::set_account_level_management,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "domestic_only",
                &app::model::cards_info::CardInfoCreateRequest::get_domestic_only,
                &app::model::cards_info::CardInfoCreateRequest::set_domestic_only,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "level_two_supported",
                &app::model::cards_info::CardInfoCreateRequest::get_level_two_supported,
                &app::model::cards_info::CardInfoCreateRequest::set_level_two_supported,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "level_three_supported",
                &app::model::cards_info::CardInfoCreateRequest::get_level_three_supported,
                &app::model::cards_info::CardInfoCreateRequest::set_level_three_supported,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "issuer_currency",
                &app::model::cards_info::CardInfoCreateRequest::get_issuer_currency,
                &app::model::cards_info::CardInfoCreateRequest::set_issuer_currency,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "combo_card",
                &app::model::cards_info::CardInfoCreateRequest::get_combo_card,
                &app::model::cards_info::CardInfoCreateRequest::set_combo_card,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "authentication",
                &app::model::cards_info::CardInfoCreateRequest::get_authentication,
                &app::model::cards_info::CardInfoCreateRequest::set_authentication,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "cost",
                &app::model::cards_info::CardInfoCreateRequest::get_cost,
                &app::model::cards_info::CardInfoCreateRequest::set_cost,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "issuer_supports_tokenization",
                &app::model::cards_info::CardInfoCreateRequest::get_issuer_supports_tokenization,
                &app::model::cards_info::CardInfoCreateRequest::set_issuer_supports_tokenization,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "billpay_enabled",
                &app::model::cards_info::CardInfoCreateRequest::get_billpay_enabled,
                &app::model::cards_info::CardInfoCreateRequest::set_billpay_enabled,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "ecom_enabled",
                &app::model::cards_info::CardInfoCreateRequest::get_ecom_enabled,
                &app::model::cards_info::CardInfoCreateRequest::set_ecom_enabled,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "flexible_credential_supported",
                &app::model::cards_info::CardInfoCreateRequest::get_flexible_credential_supported,
                &app::model::cards_info::CardInfoCreateRequest::set_flexible_credential_supported,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_subtype_code",
                &app::model::cards_info::CardInfoCreateRequest::get_card_subtype_code,
                &app::model::cards_info::CardInfoCreateRequest::set_card_subtype_code,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "multi_account_access_indicator",
                &app::model::cards_info::CardInfoCreateRequest::get_multi_account_access_indicator,
                &app::model::cards_info::CardInfoCreateRequest::set_multi_account_access_indicator,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "card_info_create_request";
    }
};

template<>
struct serde::Serializable<app::model::cards_info::CardInfoUpdateRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "card_iin",
                &app::model::cards_info::CardInfoUpdateRequest::get_card_iin,
                &app::model::cards_info::CardInfoUpdateRequest::set_card_iin,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_issuer",
                &app::model::cards_info::CardInfoUpdateRequest::get_card_issuer,
                &app::model::cards_info::CardInfoUpdateRequest::set_card_issuer,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_network",
                &app::model::cards_info::CardInfoUpdateRequest::get_card_network,
                &app::model::cards_info::CardInfoUpdateRequest::set_card_network,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_type",
                &app::model::cards_info::CardInfoUpdateRequest::get_card_type,
                &app::model::cards_info::CardInfoUpdateRequest::set_card_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_subtype",
                &app::model::cards_info::CardInfoUpdateRequest::get_card_subtype,
                &app::model::cards_info::CardInfoUpdateRequest::set_card_subtype,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_issuing_country",
                &app::model::cards_info::CardInfoUpdateRequest::get_card_issuing_country,
                &app::model::cards_info::CardInfoUpdateRequest::set_card_issuing_country,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "bank_code_id",
                &app::model::cards_info::CardInfoUpdateRequest::get_bank_code_id,
                &app::model::cards_info::CardInfoUpdateRequest::set_bank_code_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "bank_code",
                &app::model::cards_info::CardInfoUpdateRequest::get_bank_code,
                &app::model::cards_info::CardInfoUpdateRequest::set_bank_code,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "country_code",
                &app::model::cards_info::CardInfoUpdateRequest::get_country_code,
                &app::model::cards_info::CardInfoUpdateRequest::set_country_code,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "last_updated_provider",
                &app::model::cards_info::CardInfoUpdateRequest::get_last_updated_provider,
                &app::model::cards_info::CardInfoUpdateRequest::set_last_updated_provider,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "funding_source",
                &app::model::cards_info::CardInfoUpdateRequest::get_funding_source,
                &app::model::cards_info::CardInfoUpdateRequest::set_funding_source,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_iin_type",
                &app::model::cards_info::CardInfoUpdateRequest::get_card_iin_type,
                &app::model::cards_info::CardInfoUpdateRequest::set_card_iin_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "virtual_card",
                &app::model::cards_info::CardInfoUpdateRequest::get_virtual_card,
                &app::model::cards_info::CardInfoUpdateRequest::set_virtual_card,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "gambling_blocked",
                &app::model::cards_info::CardInfoUpdateRequest::get_gambling_blocked,
                &app::model::cards_info::CardInfoUpdateRequest::set_gambling_blocked,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "co_badged_card_networks",
                &app::model::cards_info::CardInfoUpdateRequest::get_co_badged_card_networks,
                &app::model::cards_info::CardInfoUpdateRequest::set_co_badged_card_networks,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_segment_type",
                &app::model::cards_info::CardInfoUpdateRequest::get_card_segment_type,
                &app::model::cards_info::CardInfoUpdateRequest::set_card_segment_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "numeric_country_code",
                &app::model::cards_info::CardInfoUpdateRequest::get_numeric_country_code,
                &app::model::cards_info::CardInfoUpdateRequest::set_numeric_country_code,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "prepaid",
                &app::model::cards_info::CardInfoUpdateRequest::get_prepaid,
                &app::model::cards_info::CardInfoUpdateRequest::set_prepaid,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "regulated",
                &app::model::cards_info::CardInfoUpdateRequest::get_regulated,
                &app::model::cards_info::CardInfoUpdateRequest::set_regulated,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "issuer_phone",
                &app::model::cards_info::CardInfoUpdateRequest::get_issuer_phone,
                &app::model::cards_info::CardInfoUpdateRequest::set_issuer_phone,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "issuer_url",
                &app::model::cards_info::CardInfoUpdateRequest::get_issuer_url,
                &app::model::cards_info::CardInfoUpdateRequest::set_issuer_url,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "regulated_name",
                &app::model::cards_info::CardInfoUpdateRequest::get_regulated_name,
                &app::model::cards_info::CardInfoUpdateRequest::set_regulated_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "reloadable_prepaid",
                &app::model::cards_info::CardInfoUpdateRequest::get_reloadable_prepaid,
                &app::model::cards_info::CardInfoUpdateRequest::set_reloadable_prepaid,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "account_updater",
                &app::model::cards_info::CardInfoUpdateRequest::get_account_updater,
                &app::model::cards_info::CardInfoUpdateRequest::set_account_updater,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "account_level_management",
                &app::model::cards_info::CardInfoUpdateRequest::get_account_level_management,
                &app::model::cards_info::CardInfoUpdateRequest::set_account_level_management,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "domestic_only",
                &app::model::cards_info::CardInfoUpdateRequest::get_domestic_only,
                &app::model::cards_info::CardInfoUpdateRequest::set_domestic_only,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "level_two_supported",
                &app::model::cards_info::CardInfoUpdateRequest::get_level_two_supported,
                &app::model::cards_info::CardInfoUpdateRequest::set_level_two_supported,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "level_three_supported",
                &app::model::cards_info::CardInfoUpdateRequest::get_level_three_supported,
                &app::model::cards_info::CardInfoUpdateRequest::set_level_three_supported,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "issuer_currency",
                &app::model::cards_info::CardInfoUpdateRequest::get_issuer_currency,
                &app::model::cards_info::CardInfoUpdateRequest::set_issuer_currency,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "combo_card",
                &app::model::cards_info::CardInfoUpdateRequest::get_combo_card,
                &app::model::cards_info::CardInfoUpdateRequest::set_combo_card,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "authentication",
                &app::model::cards_info::CardInfoUpdateRequest::get_authentication,
                &app::model::cards_info::CardInfoUpdateRequest::set_authentication,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "cost",
                &app::model::cards_info::CardInfoUpdateRequest::get_cost,
                &app::model::cards_info::CardInfoUpdateRequest::set_cost,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "issuer_supports_tokenization",
                &app::model::cards_info::CardInfoUpdateRequest::get_issuer_supports_tokenization,
                &app::model::cards_info::CardInfoUpdateRequest::set_issuer_supports_tokenization,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "billpay_enabled",
                &app::model::cards_info::CardInfoUpdateRequest::get_billpay_enabled,
                &app::model::cards_info::CardInfoUpdateRequest::set_billpay_enabled,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "ecom_enabled",
                &app::model::cards_info::CardInfoUpdateRequest::get_ecom_enabled,
                &app::model::cards_info::CardInfoUpdateRequest::set_ecom_enabled,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "flexible_credential_supported",
                &app::model::cards_info::CardInfoUpdateRequest::get_flexible_credential_supported,
                &app::model::cards_info::CardInfoUpdateRequest::set_flexible_credential_supported,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_subtype_code",
                &app::model::cards_info::CardInfoUpdateRequest::get_card_subtype_code,
                &app::model::cards_info::CardInfoUpdateRequest::set_card_subtype_code,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "multi_account_access_indicator",
                &app::model::cards_info::CardInfoUpdateRequest::get_multi_account_access_indicator,
                &app::model::cards_info::CardInfoUpdateRequest::set_multi_account_access_indicator,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "line_number",
                &app::model::cards_info::CardInfoUpdateRequest::get_line_number,
                &app::model::cards_info::CardInfoUpdateRequest::set_line_number,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "card_info_update_request";
    }
};

template<>
struct serde::Serializable<app::model::cards_info::CardInfoMigrationResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "line_number",
                &app::model::cards_info::CardInfoMigrationResponse::get_line_number,
                &app::model::cards_info::CardInfoMigrationResponse::set_line_number,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_iin",
                &app::model::cards_info::CardInfoMigrationResponse::get_card_iin,
                &app::model::cards_info::CardInfoMigrationResponse::set_card_iin,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_issuer",
                &app::model::cards_info::CardInfoMigrationResponse::get_card_issuer,
                &app::model::cards_info::CardInfoMigrationResponse::set_card_issuer,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_network",
                &app::model::cards_info::CardInfoMigrationResponse::get_card_network,
                &app::model::cards_info::CardInfoMigrationResponse::set_card_network,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_type",
                &app::model::cards_info::CardInfoMigrationResponse::get_card_type,
                &app::model::cards_info::CardInfoMigrationResponse::set_card_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_sub_type",
                &app::model::cards_info::CardInfoMigrationResponse::get_card_sub_type,
                &app::model::cards_info::CardInfoMigrationResponse::set_card_sub_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_issuing_country",
                &app::model::cards_info::CardInfoMigrationResponse::get_card_issuing_country,
                &app::model::cards_info::CardInfoMigrationResponse::set_card_issuing_country,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "country_code",
                &app::model::cards_info::CardInfoMigrationResponse::get_country_code,
                &app::model::cards_info::CardInfoMigrationResponse::set_country_code,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "funding_source",
                &app::model::cards_info::CardInfoMigrationResponse::get_funding_source,
                &app::model::cards_info::CardInfoMigrationResponse::set_funding_source,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_iin_type",
                &app::model::cards_info::CardInfoMigrationResponse::get_card_iin_type,
                &app::model::cards_info::CardInfoMigrationResponse::set_card_iin_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "virtual_card",
                &app::model::cards_info::CardInfoMigrationResponse::get_virtual_card,
                &app::model::cards_info::CardInfoMigrationResponse::set_virtual_card,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "gambling_blocked",
                &app::model::cards_info::CardInfoMigrationResponse::get_gambling_blocked,
                &app::model::cards_info::CardInfoMigrationResponse::set_gambling_blocked,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "co_badged_card_networks",
                &app::model::cards_info::CardInfoMigrationResponse::get_co_badged_card_networks,
                &app::model::cards_info::CardInfoMigrationResponse::set_co_badged_card_networks,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_segment_type",
                &app::model::cards_info::CardInfoMigrationResponse::get_card_segment_type,
                &app::model::cards_info::CardInfoMigrationResponse::set_card_segment_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "numeric_country_code",
                &app::model::cards_info::CardInfoMigrationResponse::get_numeric_country_code,
                &app::model::cards_info::CardInfoMigrationResponse::set_numeric_country_code,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "prepaid",
                &app::model::cards_info::CardInfoMigrationResponse::get_prepaid,
                &app::model::cards_info::CardInfoMigrationResponse::set_prepaid,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "regulated",
                &app::model::cards_info::CardInfoMigrationResponse::get_regulated,
                &app::model::cards_info::CardInfoMigrationResponse::set_regulated,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "issuer_phone",
                &app::model::cards_info::CardInfoMigrationResponse::get_issuer_phone,
                &app::model::cards_info::CardInfoMigrationResponse::set_issuer_phone,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "issuer_url",
                &app::model::cards_info::CardInfoMigrationResponse::get_issuer_url,
                &app::model::cards_info::CardInfoMigrationResponse::set_issuer_url,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "regulated_name",
                &app::model::cards_info::CardInfoMigrationResponse::get_regulated_name,
                &app::model::cards_info::CardInfoMigrationResponse::set_regulated_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "reloadable_prepaid",
                &app::model::cards_info::CardInfoMigrationResponse::get_reloadable_prepaid,
                &app::model::cards_info::CardInfoMigrationResponse::set_reloadable_prepaid,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "account_updater",
                &app::model::cards_info::CardInfoMigrationResponse::get_account_updater,
                &app::model::cards_info::CardInfoMigrationResponse::set_account_updater,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "account_level_management",
                &app::model::cards_info::CardInfoMigrationResponse::get_account_level_management,
                &app::model::cards_info::CardInfoMigrationResponse::set_account_level_management,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "domestic_only",
                &app::model::cards_info::CardInfoMigrationResponse::get_domestic_only,
                &app::model::cards_info::CardInfoMigrationResponse::set_domestic_only,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "level_two_supported",
                &app::model::cards_info::CardInfoMigrationResponse::get_level_two_supported,
                &app::model::cards_info::CardInfoMigrationResponse::set_level_two_supported,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "level_three_supported",
                &app::model::cards_info::CardInfoMigrationResponse::get_level_three_supported,
                &app::model::cards_info::CardInfoMigrationResponse::set_level_three_supported,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "issuer_currency",
                &app::model::cards_info::CardInfoMigrationResponse::get_issuer_currency,
                &app::model::cards_info::CardInfoMigrationResponse::set_issuer_currency,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "combo_card",
                &app::model::cards_info::CardInfoMigrationResponse::get_combo_card,
                &app::model::cards_info::CardInfoMigrationResponse::set_combo_card,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "authentication",
                &app::model::cards_info::CardInfoMigrationResponse::get_authentication,
                &app::model::cards_info::CardInfoMigrationResponse::set_authentication,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "cost",
                &app::model::cards_info::CardInfoMigrationResponse::get_cost,
                &app::model::cards_info::CardInfoMigrationResponse::set_cost,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "issuer_supports_tokenization",
                &app::model::cards_info::CardInfoMigrationResponse::
                    get_issuer_supports_tokenization,
                &app::model::cards_info::CardInfoMigrationResponse::
                    set_issuer_supports_tokenization,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "billpay_enabled",
                &app::model::cards_info::CardInfoMigrationResponse::get_billpay_enabled,
                &app::model::cards_info::CardInfoMigrationResponse::set_billpay_enabled,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "ecom_enabled",
                &app::model::cards_info::CardInfoMigrationResponse::get_ecom_enabled,
                &app::model::cards_info::CardInfoMigrationResponse::set_ecom_enabled,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "flexible_credential_supported",
                &app::model::cards_info::CardInfoMigrationResponse::
                    get_flexible_credential_supported,
                &app::model::cards_info::CardInfoMigrationResponse::
                    set_flexible_credential_supported,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_subtype_code",
                &app::model::cards_info::CardInfoMigrationResponse::get_card_subtype_code,
                &app::model::cards_info::CardInfoMigrationResponse::set_card_subtype_code,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "multi_account_access_indicator",
                &app::model::cards_info::CardInfoMigrationResponse::
                    get_multi_account_access_indicator,
                &app::model::cards_info::CardInfoMigrationResponse::
                    set_multi_account_access_indicator,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "migration_error",
                &app::model::cards_info::CardInfoMigrationResponse::get_migration_error,
                &app::model::cards_info::CardInfoMigrationResponse::set_migration_error,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "migration_status",
                &app::model::cards_info::CardInfoMigrationResponse::get_migration_status,
                &app::model::cards_info::CardInfoMigrationResponse::set_migration_status,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "card_info_migration_response";
    }
};

#ifdef CONGELADO_TEST
namespace app::model::cards_info::tests {
using namespace boost::ut;
suite<"app_model_cards_info"> app_model_cards_info_suite = []
{
    "reflects converted types"_test = []
    {
        expect(
            std::tuple_size_v<decltype(serde::Serializable<
                                       app::model::cards_info::CardsInfoRequestParams>::fields())> >
            0
        );
    };
};
} // namespace app::model::cards_info::tests
#endif
