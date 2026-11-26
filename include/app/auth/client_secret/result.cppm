export module app_auth:client_secret_result;

import std;
import core_serde;
import app_dto;

export namespace app::handlers::client_secret {

// Both routes return the exact DTO the old handler encoded with
// `Ser::encode_json(response)`. Deriving from that DTO and reusing its FieldDesc
// tuple keeps field names, order and wire types byte-identical.

class ClientSecretCreateResult : public ::app::dto::client_secret::ClientSecretResponse
{
public:
    ClientSecretCreateResult() = default;

    explicit ClientSecretCreateResult(
        ::app::dto::client_secret::ClientSecretResponse value
    ) noexcept :
        ClientSecretResponse{std::move(value)}
    {
    }
};

class ClientSecretDeleteResult : public ::app::dto::client_secret::ClientSecretResponse
{
public:
    ClientSecretDeleteResult() = default;

    explicit ClientSecretDeleteResult(
        ::app::dto::client_secret::ClientSecretResponse value
    ) noexcept :
        ClientSecretResponse{std::move(value)}
    {
    }
};

} // namespace app::handlers::client_secret

template<>
struct serde::Serializable<::app::handlers::client_secret::ClientSecretCreateResult>
{
    static constexpr auto fields()
    {
        return serde::Serializable<::app::dto::client_secret::ClientSecretResponse>::fields();
    }

    static constexpr std::string_view table_name()
    {
        return "client_secret_create_result";
    }
};

template<>
struct serde::Serializable<::app::handlers::client_secret::ClientSecretDeleteResult>
{
    static constexpr auto fields()
    {
        return serde::Serializable<::app::dto::client_secret::ClientSecretResponse>::fields();
    }

    static constexpr std::string_view table_name()
    {
        return "client_secret_delete_result";
    }
};
