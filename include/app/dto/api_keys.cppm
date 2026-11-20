module;
export module app_dto:api_keys;

import std;
import core_serde;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace app::dto::api_keys {

enum class ApiKeyExpiration
{
    Never,
    DateTime
};

class ApiKeyIdValue
{
public:
    void set_value(std::string v) { value = std::move(v); }
    [[nodiscard]] const std::string& get_value() const noexcept { return value; }
    std::string value{};
};

class ListApiKeyConstraints
{
public:
    void set_limit(std::optional<std::int64_t> value) { limit = std::move(value); }
    [[nodiscard]] const std::optional<std::int64_t>& get_limit() const noexcept { return limit; }
    void set_skip(std::optional<std::int64_t> value) { skip = std::move(value); }
    [[nodiscard]] const std::optional<std::int64_t>& get_skip() const noexcept { return skip; }
    std::optional<std::int64_t> limit{};
    std::optional<std::int64_t> skip{};
};

class MerchantIdValue
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

class CreateApiKeyRequest
{
public:
    void set_name(std::string value) { name = std::move(value); }
    [[nodiscard]] const std::string& get_name() const noexcept { return name; }
    void set_description(std::optional<std::string> value) { description = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_description() const noexcept { return description; }
    void set_expiration(ApiKeyExpiration value) { expiration = value; }
    [[nodiscard]] const ApiKeyExpiration& get_expiration() const noexcept { return expiration; }
    std::string name{};
    std::optional<std::string> description{};
    ApiKeyExpiration expiration{};
};

class CreateApiKeyResponse
{
public:
    void set_key_id(ApiKeyIdValue value) { key_id = std::move(value); }
    [[nodiscard]] const ApiKeyIdValue& get_key_id() const noexcept { return key_id; }
    void set_merchant_id(MerchantIdValue value) { merchant_id = std::move(value); }
    [[nodiscard]] const MerchantIdValue& get_merchant_id() const noexcept { return merchant_id; }
    void set_name(std::string value) { name = std::move(value); }
    [[nodiscard]] const std::string& get_name() const noexcept { return name; }
    void set_description(std::optional<std::string> value) { description = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_description() const noexcept { return description; }
    void set_api_key(std::string value) { api_key = std::move(value); }
    [[nodiscard]] const std::string& get_api_key() const noexcept { return api_key; }
    void set_created(PrimitiveDateTimeValue value) { created = std::move(value); }
    [[nodiscard]] const PrimitiveDateTimeValue& get_created() const noexcept { return created; }
    void set_expiration(ApiKeyExpiration value) { expiration = value; }
    [[nodiscard]] const ApiKeyExpiration& get_expiration() const noexcept { return expiration; }
    ApiKeyIdValue key_id{};
    MerchantIdValue merchant_id{};
    std::string name{};
    std::optional<std::string> description{};
    std::string api_key{};
    PrimitiveDateTimeValue created{};
    ApiKeyExpiration expiration{};
};

class RetrieveApiKeyResponse
{
public:
    void set_key_id(ApiKeyIdValue value) { key_id = std::move(value); }
    [[nodiscard]] const ApiKeyIdValue& get_key_id() const noexcept { return key_id; }
    void set_merchant_id(MerchantIdValue value) { merchant_id = std::move(value); }
    [[nodiscard]] const MerchantIdValue& get_merchant_id() const noexcept { return merchant_id; }
    void set_name(std::string value) { name = std::move(value); }
    [[nodiscard]] const std::string& get_name() const noexcept { return name; }
    void set_description(std::optional<std::string> value) { description = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_description() const noexcept { return description; }
    void set_prefix(std::string value) { prefix = std::move(value); }
    [[nodiscard]] const std::string& get_prefix() const noexcept { return prefix; }
    void set_created(PrimitiveDateTimeValue value) { created = std::move(value); }
    [[nodiscard]] const PrimitiveDateTimeValue& get_created() const noexcept { return created; }
    void set_expiration(ApiKeyExpiration value) { expiration = value; }
    [[nodiscard]] const ApiKeyExpiration& get_expiration() const noexcept { return expiration; }
    ApiKeyIdValue key_id{};
    MerchantIdValue merchant_id{};
    std::string name{};
    std::optional<std::string> description{};
    std::string prefix{};
    PrimitiveDateTimeValue created{};
    ApiKeyExpiration expiration{};
};

class RevokeApiKeyResponse
{
public:
    void set_merchant_id(MerchantIdValue value) { merchant_id = std::move(value); }
    [[nodiscard]] const MerchantIdValue& get_merchant_id() const noexcept { return merchant_id; }
    void set_key_id(ApiKeyIdValue value) { key_id = std::move(value); }
    [[nodiscard]] const ApiKeyIdValue& get_key_id() const noexcept { return key_id; }
    void set_revoked(bool value) { revoked = value; }
    [[nodiscard]] const bool& get_revoked() const noexcept { return revoked; }
    MerchantIdValue merchant_id{};
    ApiKeyIdValue key_id{};
    bool revoked{};
};

class UpdateApiKeyRequest
{
public:
    void set_name(std::optional<std::string> value) { name = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_name() const noexcept { return name; }
    void set_description(std::optional<std::string> value) { description = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_description() const noexcept { return description; }
    void set_expiration(std::optional<ApiKeyExpiration> value) { expiration = std::move(value); }
    [[nodiscard]] const std::optional<ApiKeyExpiration>& get_expiration() const noexcept { return expiration; }
    void set_key_id(ApiKeyIdValue value) { key_id = std::move(value); }
    [[nodiscard]] const ApiKeyIdValue& get_key_id() const noexcept { return key_id; }
    void set_merchant_id(MerchantIdValue value) { merchant_id = std::move(value); }
    [[nodiscard]] const MerchantIdValue& get_merchant_id() const noexcept { return merchant_id; }
    std::optional<std::string> name{};
    std::optional<std::string> description{};
    std::optional<ApiKeyExpiration> expiration{};
    ApiKeyIdValue key_id{};
    MerchantIdValue merchant_id{};
};
} // namespace app::dto::api_keys

template<>
struct serde::Serializable<app::dto::api_keys::ApiKeyIdValue>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"value", &app::dto::api_keys::ApiKeyIdValue::get_value, &app::dto::api_keys::ApiKeyIdValue::set_value, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "api_key_id_value"; }
};

template<>
struct serde::Serializable<app::dto::api_keys::MerchantIdValue>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"value", &app::dto::api_keys::MerchantIdValue::get_value, &app::dto::api_keys::MerchantIdValue::set_value, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "merchant_id_value"; }
};

template<>
struct serde::Serializable<app::dto::api_keys::PrimitiveDateTimeValue>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"value", &app::dto::api_keys::PrimitiveDateTimeValue::get_value, &app::dto::api_keys::PrimitiveDateTimeValue::set_value, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "primitive_date_time_value"; }
};

template<>
struct serde::Serializable<app::dto::api_keys::CreateApiKeyRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"name", &app::dto::api_keys::CreateApiKeyRequest::get_name, &app::dto::api_keys::CreateApiKeyRequest::set_name, serde::FieldOptions{}>{},
            serde::FieldDesc<"description", &app::dto::api_keys::CreateApiKeyRequest::get_description, &app::dto::api_keys::CreateApiKeyRequest::set_description, serde::FieldOptions{}>{},
            serde::FieldDesc<"expiration", &app::dto::api_keys::CreateApiKeyRequest::get_expiration, &app::dto::api_keys::CreateApiKeyRequest::set_expiration, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "create_api_key_request"; }
};

template<>
struct serde::Serializable<app::dto::api_keys::CreateApiKeyResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"key_id", &app::dto::api_keys::CreateApiKeyResponse::get_key_id, &app::dto::api_keys::CreateApiKeyResponse::set_key_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"merchant_id", &app::dto::api_keys::CreateApiKeyResponse::get_merchant_id, &app::dto::api_keys::CreateApiKeyResponse::set_merchant_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"name", &app::dto::api_keys::CreateApiKeyResponse::get_name, &app::dto::api_keys::CreateApiKeyResponse::set_name, serde::FieldOptions{}>{},
            serde::FieldDesc<"description", &app::dto::api_keys::CreateApiKeyResponse::get_description, &app::dto::api_keys::CreateApiKeyResponse::set_description, serde::FieldOptions{}>{},
            serde::FieldDesc<"api_key", &app::dto::api_keys::CreateApiKeyResponse::get_api_key, &app::dto::api_keys::CreateApiKeyResponse::set_api_key, serde::FieldOptions{}>{},
            serde::FieldDesc<"created", &app::dto::api_keys::CreateApiKeyResponse::get_created, &app::dto::api_keys::CreateApiKeyResponse::set_created, serde::FieldOptions{}>{},
            serde::FieldDesc<"expiration", &app::dto::api_keys::CreateApiKeyResponse::get_expiration, &app::dto::api_keys::CreateApiKeyResponse::set_expiration, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "create_api_key_response"; }
};

template<>
struct serde::Serializable<app::dto::api_keys::RetrieveApiKeyResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"key_id", &app::dto::api_keys::RetrieveApiKeyResponse::get_key_id, &app::dto::api_keys::RetrieveApiKeyResponse::set_key_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"merchant_id", &app::dto::api_keys::RetrieveApiKeyResponse::get_merchant_id, &app::dto::api_keys::RetrieveApiKeyResponse::set_merchant_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"name", &app::dto::api_keys::RetrieveApiKeyResponse::get_name, &app::dto::api_keys::RetrieveApiKeyResponse::set_name, serde::FieldOptions{}>{},
            serde::FieldDesc<"description", &app::dto::api_keys::RetrieveApiKeyResponse::get_description, &app::dto::api_keys::RetrieveApiKeyResponse::set_description, serde::FieldOptions{}>{},
            serde::FieldDesc<"prefix", &app::dto::api_keys::RetrieveApiKeyResponse::get_prefix, &app::dto::api_keys::RetrieveApiKeyResponse::set_prefix, serde::FieldOptions{}>{},
            serde::FieldDesc<"created", &app::dto::api_keys::RetrieveApiKeyResponse::get_created, &app::dto::api_keys::RetrieveApiKeyResponse::set_created, serde::FieldOptions{}>{},
            serde::FieldDesc<"expiration", &app::dto::api_keys::RetrieveApiKeyResponse::get_expiration, &app::dto::api_keys::RetrieveApiKeyResponse::set_expiration, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "retrieve_api_key_response"; }
};

template<>
struct serde::Serializable<app::dto::api_keys::UpdateApiKeyRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"name", &app::dto::api_keys::UpdateApiKeyRequest::get_name, &app::dto::api_keys::UpdateApiKeyRequest::set_name, serde::FieldOptions{}>{},
            serde::FieldDesc<"description", &app::dto::api_keys::UpdateApiKeyRequest::get_description, &app::dto::api_keys::UpdateApiKeyRequest::set_description, serde::FieldOptions{}>{},
            serde::FieldDesc<"expiration", &app::dto::api_keys::UpdateApiKeyRequest::get_expiration, &app::dto::api_keys::UpdateApiKeyRequest::set_expiration, serde::FieldOptions{}>{},
            serde::FieldDesc<"key_id", &app::dto::api_keys::UpdateApiKeyRequest::get_key_id, &app::dto::api_keys::UpdateApiKeyRequest::set_key_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"merchant_id", &app::dto::api_keys::UpdateApiKeyRequest::get_merchant_id, &app::dto::api_keys::UpdateApiKeyRequest::set_merchant_id, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "update_api_key_request"; }
};

template<>
struct serde::Serializable<app::dto::api_keys::RevokeApiKeyResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"merchant_id", &app::dto::api_keys::RevokeApiKeyResponse::get_merchant_id, &app::dto::api_keys::RevokeApiKeyResponse::set_merchant_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"key_id", &app::dto::api_keys::RevokeApiKeyResponse::get_key_id, &app::dto::api_keys::RevokeApiKeyResponse::set_key_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"revoked", &app::dto::api_keys::RevokeApiKeyResponse::get_revoked, &app::dto::api_keys::RevokeApiKeyResponse::set_revoked, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "revoke_api_key_response"; }
};

template<>
struct serde::Serializable<app::dto::api_keys::ListApiKeyConstraints>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"limit", &app::dto::api_keys::ListApiKeyConstraints::get_limit, &app::dto::api_keys::ListApiKeyConstraints::set_limit, serde::FieldOptions{}>{},
            serde::FieldDesc<"skip", &app::dto::api_keys::ListApiKeyConstraints::get_skip, &app::dto::api_keys::ListApiKeyConstraints::set_skip, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "list_api_key_constraints"; }
};

#ifdef CONGELADO_TEST
namespace app::dto::api_keys::tests {
using namespace boost::ut;
suite<<"app_dto_api_keys"> app_dto_api_keys_suite = [] {
    "reflects converted types"_test = [] {
        expect(std::tuple_size_v<decltype(serde::Serializable<app::dto::api_keys::CreateApiKeyRequest>::fields())> > 0);
    };
};
}
#endif
