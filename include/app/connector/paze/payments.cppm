module;
export module app_connector:model_paze_payments;

import std;
import core_serde;
import :model_shared_payments;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace app::model::payments {

class PazeMetadata : public serde::Serializer<PazeMetadata>
{
public:
    void set_client_id(std::string value)
    {
        client_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_client_id() const noexcept
    {
        return client_id;
    }

    void set_client_name(std::string value)
    {
        client_name = std::move(value);
    }

    [[nodiscard]] const std::string& get_client_name() const noexcept
    {
        return client_name;
    }

    void set_client_profile_id(std::string value)
    {
        client_profile_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_client_profile_id() const noexcept
    {
        return client_profile_id;
    }

    std::string client_id{};
    std::string client_name{};
    std::string client_profile_id{};
};

class PazeSessionTokenData : public serde::Serializer<PazeSessionTokenData>
{
public:
    void set_data(PazeMetadata value)
    {
        data = std::move(value);
    }

    [[nodiscard]] const PazeMetadata& get_data() const noexcept
    {
        return data;
    }

    PazeMetadata data{};
};

class PazeSessionTokenResponse : public serde::Serializer<PazeSessionTokenResponse>
{
public:
    void set_client_id(std::string value)
    {
        client_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_client_id() const noexcept
    {
        return client_id;
    }

    void set_client_name(std::string value)
    {
        client_name = std::move(value);
    }

    [[nodiscard]] const std::string& get_client_name() const noexcept
    {
        return client_name;
    }

    void set_client_profile_id(std::string value)
    {
        client_profile_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_client_profile_id() const noexcept
    {
        return client_profile_id;
    }

    void set_transaction_currency_code(CurrencyValue value)
    {
        transaction_currency_code = std::move(value);
    }

    [[nodiscard]] const CurrencyValue& get_transaction_currency_code() const noexcept
    {
        return transaction_currency_code;
    }

    void set_transaction_amount(StringMajorUnitValue value)
    {
        transaction_amount = std::move(value);
    }

    [[nodiscard]] const StringMajorUnitValue& get_transaction_amount() const noexcept
    {
        return transaction_amount;
    }

    void set_email_address(std::optional<EmailValue> value)
    {
        email_address = std::move(value);
    }

    [[nodiscard]] const std::optional<EmailValue>& get_email_address() const noexcept
    {
        return email_address;
    }

    std::string client_id{};
    std::string client_name{};
    std::string client_profile_id{};
    CurrencyValue transaction_currency_code{};
    StringMajorUnitValue transaction_amount{};
    std::optional<EmailValue> email_address{};
};

class PazeWalletData : public serde::Serializer<PazeWalletData>
{
public:
    void set_complete_response(SecretValue value)
    {
        complete_response = std::move(value);
    }

    [[nodiscard]] const SecretValue& get_complete_response() const noexcept
    {
        return complete_response;
    }

    SecretValue complete_response{};
};

} // namespace app::model::payments

template<>
struct serde::Serializable<app::model::payments::PazeWalletData>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<
            "complete_response",
            &app::model::payments::PazeWalletData::get_complete_response,
            &app::model::payments::PazeWalletData::set_complete_response,
            serde::FieldOptions{}>{}};
    }

    static constexpr std::string_view table_name()
    {
        return "paze_wallet_data";
    }
};

template<>
struct serde::Serializable<app::model::payments::PazeSessionTokenData>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<
            "paze",
            &app::model::payments::PazeSessionTokenData::get_data,
            &app::model::payments::PazeSessionTokenData::set_data,
            serde::FieldOptions{}>{}};
    }

    static constexpr std::string_view table_name()
    {
        return "paze_session_token_data";
    }
};

template<>
struct serde::Serializable<app::model::payments::PazeMetadata>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "client_id",
                &app::model::payments::PazeMetadata::get_client_id,
                &app::model::payments::PazeMetadata::set_client_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "client_name",
                &app::model::payments::PazeMetadata::get_client_name,
                &app::model::payments::PazeMetadata::set_client_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "client_profile_id",
                &app::model::payments::PazeMetadata::get_client_profile_id,
                &app::model::payments::PazeMetadata::set_client_profile_id,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "paze_metadata";
    }
};

template<>
struct serde::Serializable<app::model::payments::PazeSessionTokenResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "client_id",
                &app::model::payments::PazeSessionTokenResponse::get_client_id,
                &app::model::payments::PazeSessionTokenResponse::set_client_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "client_name",
                &app::model::payments::PazeSessionTokenResponse::get_client_name,
                &app::model::payments::PazeSessionTokenResponse::set_client_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "client_profile_id",
                &app::model::payments::PazeSessionTokenResponse::get_client_profile_id,
                &app::model::payments::PazeSessionTokenResponse::set_client_profile_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "transaction_currency_code",
                &app::model::payments::PazeSessionTokenResponse::get_transaction_currency_code,
                &app::model::payments::PazeSessionTokenResponse::set_transaction_currency_code,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "transaction_amount",
                &app::model::payments::PazeSessionTokenResponse::get_transaction_amount,
                &app::model::payments::PazeSessionTokenResponse::set_transaction_amount,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "email_address",
                &app::model::payments::PazeSessionTokenResponse::get_email_address,
                &app::model::payments::PazeSessionTokenResponse::set_email_address,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "paze_session_token_response";
    }
};
