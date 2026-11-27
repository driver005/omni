module;
export module app_connector:model_shared_payouts;

import std;
import core_serde;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace app::model::payouts {

class BankNamesValue : public serde::Serializer<BankNamesValue>
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

class BankTypeValue : public serde::Serializer<BankTypeValue>
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

class CountryAlpha2Value : public serde::Serializer<CountryAlpha2Value>
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

class EmailValue : public serde::Serializer<EmailValue>
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

class MerchantConnectorInfoValue : public serde::Serializer<MerchantConnectorInfoValue>
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

/**
 * @brief Wire mirror of upstream `admin::MerchantConnectorInfo`
 * (`api_models/src/admin.rs:1626-1633`). Used as the value type of
 * `PayoutListFiltersV2.connector` so `GET /payouts/v2/{profile/}filter` emits the exact
 * upstream JSON shape: `{connector_label, merchant_connector_id, connector_type}`.
 */
class MerchantConnectorInfo : public serde::Serializer<MerchantConnectorInfo>
{
public:
    void set_connector_label(std::string value)
    {
        connector_label = std::move(value);
    }

    [[nodiscard]] const std::string& get_connector_label() const noexcept
    {
        return connector_label;
    }

    void set_merchant_connector_id(std::string value)
    {
        merchant_connector_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_merchant_connector_id() const noexcept
    {
        return merchant_connector_id;
    }

    void set_connector_type(std::string value)
    {
        connector_type = std::move(value);
    }

    [[nodiscard]] const std::string& get_connector_type() const noexcept
    {
        return connector_type;
    }

    std::string connector_label{};
    std::string merchant_connector_id{};
    std::string connector_type{};
};

class PayoutConnectorsValue : public serde::Serializer<PayoutConnectorsValue>
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

class SecretValue : public serde::Serializer<SecretValue>
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

class Interac : public serde::Serializer<Interac>
{
public:
    void set_email(EmailValue value)
    {
        email = std::move(value);
    }

    [[nodiscard]] const EmailValue& get_email() const noexcept
    {
        return email;
    }

    EmailValue email{};
};

class OpenBanking : public serde::Serializer<OpenBanking>
{
public:
    void set_account_holder_name(SecretValue value)
    {
        account_holder_name = std::move(value);
    }

    [[nodiscard]] const SecretValue& get_account_holder_name() const noexcept
    {
        return account_holder_name;
    }

    void set_iban(SecretValue value)
    {
        iban = std::move(value);
    }

    [[nodiscard]] const SecretValue& get_iban() const noexcept
    {
        return iban;
    }

    SecretValue account_holder_name{};
    SecretValue iban{};
};

class OpenBankingUk : public serde::Serializer<OpenBankingUk>
{
public:
    void set_account_holder_name(SecretValue value)
    {
        account_holder_name = std::move(value);
    }

    [[nodiscard]] const SecretValue& get_account_holder_name() const noexcept
    {
        return account_holder_name;
    }

    void set_iban(SecretValue value)
    {
        iban = std::move(value);
    }

    [[nodiscard]] const SecretValue& get_iban() const noexcept
    {
        return iban;
    }

    SecretValue account_holder_name{};
    SecretValue iban{};
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

} // namespace app::model::payouts

template<>
struct serde::Serializable<app::model::payouts::Interac>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<
            "email",
            &app::model::payouts::Interac::get_email,
            &app::model::payouts::Interac::set_email,
            serde::FieldOptions{}>{}};
    }

    static constexpr std::string_view table_name()
    {
        return "interac";
    }
};

template<>
struct serde::Serializable<app::model::payouts::OpenBankingUk>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "account_holder_name",
                &app::model::payouts::OpenBankingUk::get_account_holder_name,
                &app::model::payouts::OpenBankingUk::set_account_holder_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "iban",
                &app::model::payouts::OpenBankingUk::get_iban,
                &app::model::payouts::OpenBankingUk::set_iban,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "open_banking_uk";
    }
};

template<>
struct serde::Serializable<app::model::payouts::OpenBanking>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "account_holder_name",
                &app::model::payouts::OpenBanking::get_account_holder_name,
                &app::model::payouts::OpenBanking::set_account_holder_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "iban",
                &app::model::payouts::OpenBanking::get_iban,
                &app::model::payouts::OpenBanking::set_iban,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "open_banking";
    }
};

template<>
struct serde::Serializable<app::model::payouts::MerchantConnectorInfo>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "connector_label",
                &app::model::payouts::MerchantConnectorInfo::get_connector_label,
                &app::model::payouts::MerchantConnectorInfo::set_connector_label,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "merchant_connector_id",
                &app::model::payouts::MerchantConnectorInfo::get_merchant_connector_id,
                &app::model::payouts::MerchantConnectorInfo::set_merchant_connector_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "connector_type",
                &app::model::payouts::MerchantConnectorInfo::get_connector_type,
                &app::model::payouts::MerchantConnectorInfo::set_connector_type,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "merchant_connector_info";
    }
};
