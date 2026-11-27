module;
export module app_connector:model_shared_mandates;

import std;
import core_serde;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace app::model::mandates {

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

class CardNumberValue : public serde::Serializer<CardNumberValue>
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

class CardSegmentTypeValue : public serde::Serializer<CardSegmentTypeValue>
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

class MerchantConnectorAccountIdValue : public serde::Serializer<MerchantConnectorAccountIdValue>
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

class MandateCardDetails : public serde::Serializer<MandateCardDetails>
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

    void set_card_token(std::optional<std::string> value)
    {
        card_token = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_card_token() const noexcept
    {
        return card_token;
    }

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

    void set_card_fingerprint(std::optional<std::string> value)
    {
        card_fingerprint = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_card_fingerprint() const noexcept
    {
        return card_fingerprint;
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

    void set_nick_name(std::optional<std::string> value)
    {
        nick_name = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_nick_name() const noexcept
    {
        return nick_name;
    }

    std::optional<std::string> last4_digits{};
    std::optional<std::string> card_exp_month{};
    std::optional<std::string> card_exp_year{};
    std::optional<std::string> card_holder_name{};
    std::optional<std::string> card_token{};
    std::optional<std::string> scheme{};
    std::optional<std::string> issuer_country{};
    std::optional<std::string> card_fingerprint{};
    std::optional<std::string> card_isin{};
    std::optional<std::string> card_issuer{};
    std::optional<CardNetworkValue> card_network{};
    std::optional<std::string> card_type{};
    std::optional<std::string> nick_name{};
};

class NetworkTransactionIdAndCardDetails :
    public serde::Serializer<NetworkTransactionIdAndCardDetails>
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

    void set_card_holder_name(std::optional<std::string> value)
    {
        card_holder_name = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_card_holder_name() const noexcept
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

    void set_nick_name(std::optional<std::string> value)
    {
        nick_name = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_nick_name() const noexcept
    {
        return nick_name;
    }

    void set_network_transaction_id(std::string value)
    {
        network_transaction_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_network_transaction_id() const noexcept
    {
        return network_transaction_id;
    }

    void set_transaction_link_id(std::optional<std::string> value)
    {
        transaction_link_id = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_transaction_link_id() const noexcept
    {
        return transaction_link_id;
    }

    CardNumberValue card_number{};
    std::string card_exp_month{};
    std::string card_exp_year{};
    std::optional<std::string> card_holder_name{};
    std::optional<std::string> card_issuer{};
    std::optional<CardNetworkValue> card_network{};
    std::optional<std::string> card_type{};
    std::optional<std::string> card_subtype{};
    std::optional<CardSegmentTypeValue> card_segment_type{};
    std::optional<FundingSourceValue> funding_source{};
    std::optional<std::string> card_issuing_country{};
    std::optional<std::string> card_issuing_country_code{};
    std::optional<std::string> bank_code{};
    std::optional<std::string> nick_name{};
    std::string network_transaction_id{};
    std::optional<std::string> transaction_link_id{};
};

class VaultCardData : public serde::Serializer<VaultCardData>
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

    void set_nick_name(std::optional<std::string> value)
    {
        nick_name = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_nick_name() const noexcept
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

    std::string card_number{};
    std::string card_exp_month{};
    std::string card_exp_year{};
    std::optional<std::string> card_holder_name{};
    std::optional<std::string> card_issuer{};
    std::optional<CardNetworkValue> card_network{};
    std::optional<std::string> card_type{};
    std::optional<std::string> card_issuing_country{};
    std::optional<std::string> bank_code{};
    std::optional<std::string> nick_name{};
    std::optional<std::string> bin_number{};
    std::optional<std::string> last_four{};
};

class NetworkTransactionIdAndVaultCardDetails :
    public serde::Serializer<NetworkTransactionIdAndVaultCardDetails>
{
public:
    void set_vault_card_data(VaultCardData value)
    {
        vault_card_data = std::move(value);
    }

    [[nodiscard]] const VaultCardData& get_vault_card_data() const noexcept
    {
        return vault_card_data;
    }

    void set_network_transaction_id(std::string value)
    {
        network_transaction_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_network_transaction_id() const noexcept
    {
        return network_transaction_id;
    }

    void set_transaction_link_id(std::optional<std::string> value)
    {
        transaction_link_id = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_transaction_link_id() const noexcept
    {
        return transaction_link_id;
    }

    VaultCardData vault_card_data{};
    std::string network_transaction_id{};
    std::optional<std::string> transaction_link_id{};
};

} // namespace app::model::mandates

template<>
struct serde::Serializable<app::model::mandates::MandateCardDetails>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "last4_digits",
                &app::model::mandates::MandateCardDetails::get_last4_digits,
                &app::model::mandates::MandateCardDetails::set_last4_digits,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_exp_month",
                &app::model::mandates::MandateCardDetails::get_card_exp_month,
                &app::model::mandates::MandateCardDetails::set_card_exp_month,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_exp_year",
                &app::model::mandates::MandateCardDetails::get_card_exp_year,
                &app::model::mandates::MandateCardDetails::set_card_exp_year,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_holder_name",
                &app::model::mandates::MandateCardDetails::get_card_holder_name,
                &app::model::mandates::MandateCardDetails::set_card_holder_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_token",
                &app::model::mandates::MandateCardDetails::get_card_token,
                &app::model::mandates::MandateCardDetails::set_card_token,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "scheme",
                &app::model::mandates::MandateCardDetails::get_scheme,
                &app::model::mandates::MandateCardDetails::set_scheme,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "issuer_country",
                &app::model::mandates::MandateCardDetails::get_issuer_country,
                &app::model::mandates::MandateCardDetails::set_issuer_country,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_fingerprint",
                &app::model::mandates::MandateCardDetails::get_card_fingerprint,
                &app::model::mandates::MandateCardDetails::set_card_fingerprint,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_isin",
                &app::model::mandates::MandateCardDetails::get_card_isin,
                &app::model::mandates::MandateCardDetails::set_card_isin,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_issuer",
                &app::model::mandates::MandateCardDetails::get_card_issuer,
                &app::model::mandates::MandateCardDetails::set_card_issuer,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_network",
                &app::model::mandates::MandateCardDetails::get_card_network,
                &app::model::mandates::MandateCardDetails::set_card_network,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_type",
                &app::model::mandates::MandateCardDetails::get_card_type,
                &app::model::mandates::MandateCardDetails::set_card_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "nick_name",
                &app::model::mandates::MandateCardDetails::get_nick_name,
                &app::model::mandates::MandateCardDetails::set_nick_name,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "mandate_card_details";
    }
};

template<>
struct serde::Serializable<app::model::mandates::NetworkTransactionIdAndCardDetails>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "card_number",
                &app::model::mandates::NetworkTransactionIdAndCardDetails::get_card_number,
                &app::model::mandates::NetworkTransactionIdAndCardDetails::set_card_number,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_exp_month",
                &app::model::mandates::NetworkTransactionIdAndCardDetails::get_card_exp_month,
                &app::model::mandates::NetworkTransactionIdAndCardDetails::set_card_exp_month,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_exp_year",
                &app::model::mandates::NetworkTransactionIdAndCardDetails::get_card_exp_year,
                &app::model::mandates::NetworkTransactionIdAndCardDetails::set_card_exp_year,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_holder_name",
                &app::model::mandates::NetworkTransactionIdAndCardDetails::get_card_holder_name,
                &app::model::mandates::NetworkTransactionIdAndCardDetails::set_card_holder_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_issuer",
                &app::model::mandates::NetworkTransactionIdAndCardDetails::get_card_issuer,
                &app::model::mandates::NetworkTransactionIdAndCardDetails::set_card_issuer,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_network",
                &app::model::mandates::NetworkTransactionIdAndCardDetails::get_card_network,
                &app::model::mandates::NetworkTransactionIdAndCardDetails::set_card_network,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_type",
                &app::model::mandates::NetworkTransactionIdAndCardDetails::get_card_type,
                &app::model::mandates::NetworkTransactionIdAndCardDetails::set_card_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_subtype",
                &app::model::mandates::NetworkTransactionIdAndCardDetails::get_card_subtype,
                &app::model::mandates::NetworkTransactionIdAndCardDetails::set_card_subtype,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_segment_type",
                &app::model::mandates::NetworkTransactionIdAndCardDetails::get_card_segment_type,
                &app::model::mandates::NetworkTransactionIdAndCardDetails::set_card_segment_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "funding_source",
                &app::model::mandates::NetworkTransactionIdAndCardDetails::get_funding_source,
                &app::model::mandates::NetworkTransactionIdAndCardDetails::set_funding_source,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_issuing_country",
                &app::model::mandates::NetworkTransactionIdAndCardDetails::get_card_issuing_country,
                &app::model::mandates::NetworkTransactionIdAndCardDetails::set_card_issuing_country,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_issuing_country_code",
                &app::model::mandates::NetworkTransactionIdAndCardDetails::
                    get_card_issuing_country_code,
                &app::model::mandates::NetworkTransactionIdAndCardDetails::
                    set_card_issuing_country_code,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "bank_code",
                &app::model::mandates::NetworkTransactionIdAndCardDetails::get_bank_code,
                &app::model::mandates::NetworkTransactionIdAndCardDetails::set_bank_code,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "nick_name",
                &app::model::mandates::NetworkTransactionIdAndCardDetails::get_nick_name,
                &app::model::mandates::NetworkTransactionIdAndCardDetails::set_nick_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "network_transaction_id",
                &app::model::mandates::NetworkTransactionIdAndCardDetails::
                    get_network_transaction_id,
                &app::model::mandates::NetworkTransactionIdAndCardDetails::
                    set_network_transaction_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "transaction_link_id",
                &app::model::mandates::NetworkTransactionIdAndCardDetails::get_transaction_link_id,
                &app::model::mandates::NetworkTransactionIdAndCardDetails::set_transaction_link_id,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "network_transaction_id_and_card_details";
    }
};

template<>
struct serde::Serializable<app::model::mandates::VaultCardData>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "card_number",
                &app::model::mandates::VaultCardData::get_card_number,
                &app::model::mandates::VaultCardData::set_card_number,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_exp_month",
                &app::model::mandates::VaultCardData::get_card_exp_month,
                &app::model::mandates::VaultCardData::set_card_exp_month,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_exp_year",
                &app::model::mandates::VaultCardData::get_card_exp_year,
                &app::model::mandates::VaultCardData::set_card_exp_year,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_holder_name",
                &app::model::mandates::VaultCardData::get_card_holder_name,
                &app::model::mandates::VaultCardData::set_card_holder_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_issuer",
                &app::model::mandates::VaultCardData::get_card_issuer,
                &app::model::mandates::VaultCardData::set_card_issuer,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_network",
                &app::model::mandates::VaultCardData::get_card_network,
                &app::model::mandates::VaultCardData::set_card_network,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_type",
                &app::model::mandates::VaultCardData::get_card_type,
                &app::model::mandates::VaultCardData::set_card_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_issuing_country",
                &app::model::mandates::VaultCardData::get_card_issuing_country,
                &app::model::mandates::VaultCardData::set_card_issuing_country,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "bank_code",
                &app::model::mandates::VaultCardData::get_bank_code,
                &app::model::mandates::VaultCardData::set_bank_code,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "nick_name",
                &app::model::mandates::VaultCardData::get_nick_name,
                &app::model::mandates::VaultCardData::set_nick_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "bin_number",
                &app::model::mandates::VaultCardData::get_bin_number,
                &app::model::mandates::VaultCardData::set_bin_number,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "last_four",
                &app::model::mandates::VaultCardData::get_last_four,
                &app::model::mandates::VaultCardData::set_last_four,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "vault_card_data";
    }
};

template<>
struct serde::Serializable<app::model::mandates::NetworkTransactionIdAndVaultCardDetails>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "vault_card_data",
                &app::model::mandates::NetworkTransactionIdAndVaultCardDetails::get_vault_card_data,
                &app::model::mandates::NetworkTransactionIdAndVaultCardDetails::set_vault_card_data,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "network_transaction_id",
                &app::model::mandates::NetworkTransactionIdAndVaultCardDetails::
                    get_network_transaction_id,
                &app::model::mandates::NetworkTransactionIdAndVaultCardDetails::
                    set_network_transaction_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "transaction_link_id",
                &app::model::mandates::NetworkTransactionIdAndVaultCardDetails::
                    get_transaction_link_id,
                &app::model::mandates::NetworkTransactionIdAndVaultCardDetails::
                    set_transaction_link_id,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "network_transaction_id_and_vault_card_details";
    }
};
