module;
export module app_dto:client_secret;

import std;
import core_serde;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace app::dto::client_secret {

class ClientSecretIdValue
{
public:
    void set_value(std::string v) { value = std::move(v); }
    [[nodiscard]] const std::string& get_value() const noexcept { return value; }
    std::string value{};
};

class PrimitiveDateTimeValue
{
public:
    void set_value(std::string v) { value = std::move(v); }
    [[nodiscard]] const std::string& get_value() const noexcept { return value; }
    std::string value{};
};

enum class ResourceId
{
    Customer,
};

class ClientSecretCreateRequest
{
public:
    void set_resource_id(ResourceId value) { resource_id = std::move(value); }
    [[nodiscard]] const ResourceId& get_resource_id() const noexcept { return resource_id; }
    ResourceId resource_id{};
};

class ClientSecretResponse
{
public:
    void set_id(ClientSecretIdValue value) { id = std::move(value); }
    [[nodiscard]] const ClientSecretIdValue& get_id() const noexcept { return id; }
    void set_resource_id(ResourceId value) { resource_id = std::move(value); }
    [[nodiscard]] const ResourceId& get_resource_id() const noexcept { return resource_id; }
    void set_created_at(PrimitiveDateTimeValue value) { created_at = std::move(value); }
    [[nodiscard]] const PrimitiveDateTimeValue& get_created_at() const noexcept { return created_at; }
    void set_expires(PrimitiveDateTimeValue value) { expires = std::move(value); }
    [[nodiscard]] const PrimitiveDateTimeValue& get_expires() const noexcept { return expires; }
    void set_secret(std::string value) { secret = std::move(value); }
    [[nodiscard]] const std::string& get_secret() const noexcept { return secret; }
    ClientSecretIdValue id{};
    ResourceId resource_id{};
    PrimitiveDateTimeValue created_at{};
    PrimitiveDateTimeValue expires{};
    std::string secret{};
};
} // namespace app::dto::client_secret

template<>
struct serde::Serializable<app::dto::client_secret::ClientSecretCreateRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"resource_id", &app::dto::client_secret::ClientSecretCreateRequest::get_resource_id, &app::dto::client_secret::ClientSecretCreateRequest::set_resource_id, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "client_secret_create_request"; }
};

template<>
struct serde::Serializable<app::dto::client_secret::ClientSecretResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"id", &app::dto::client_secret::ClientSecretResponse::get_id, &app::dto::client_secret::ClientSecretResponse::set_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"resource_id", &app::dto::client_secret::ClientSecretResponse::get_resource_id, &app::dto::client_secret::ClientSecretResponse::set_resource_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"created_at", &app::dto::client_secret::ClientSecretResponse::get_created_at, &app::dto::client_secret::ClientSecretResponse::set_created_at, serde::FieldOptions{}>{},
            serde::FieldDesc<"expires", &app::dto::client_secret::ClientSecretResponse::get_expires, &app::dto::client_secret::ClientSecretResponse::set_expires, serde::FieldOptions{}>{},
            serde::FieldDesc<"secret", &app::dto::client_secret::ClientSecretResponse::get_secret, &app::dto::client_secret::ClientSecretResponse::set_secret, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "client_secret_response"; }
};

#ifdef CONGELADO_TEST
namespace app::dto::client_secret::tests {
using namespace boost::ut;
suite<"app_dto_client_secret"> app_dto_client_secret_suite = [] {
    "reflects converted types"_test = [] {
        expect(std::tuple_size_v<decltype(serde::Serializable<app::dto::client_secret::ClientSecretCreateRequest>::fields())> > 0);
    };
};
}
#endif
