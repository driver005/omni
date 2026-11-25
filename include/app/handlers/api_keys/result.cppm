export module app_handlers_api_keys:result;

import std;
import core_serde;
import app_dto;

export namespace app::handlers::api_keys {

// Every DTO-shaped result inherits the wire fields of the exact DTO the old
// core built and encoded, so field names, order and wrapper types stay
// byte-identical. The Serializable specialization reuses the DTO's FieldDesc
// tuple; those descriptors apply to the derived object as well.

class ApiKeyCreateResult : public app::dto::api_keys::CreateApiKeyResponse
{
public:
    ApiKeyCreateResult() = default;

    explicit ApiKeyCreateResult(app::dto::api_keys::CreateApiKeyResponse value) noexcept :
        CreateApiKeyResponse{std::move(value)}
    {
    }
};

class ApiKeyRetrieveResult : public app::dto::api_keys::RetrieveApiKeyResponse
{
public:
    ApiKeyRetrieveResult() = default;

    explicit ApiKeyRetrieveResult(app::dto::api_keys::RetrieveApiKeyResponse value) noexcept :
        RetrieveApiKeyResponse{std::move(value)}
    {
    }
};

class ApiKeyUpdateResult : public app::dto::api_keys::RetrieveApiKeyResponse
{
public:
    ApiKeyUpdateResult() = default;

    explicit ApiKeyUpdateResult(app::dto::api_keys::RetrieveApiKeyResponse value) noexcept :
        RetrieveApiKeyResponse{std::move(value)}
    {
    }
};

class ApiKeyRevokeResult : public app::dto::api_keys::RevokeApiKeyResponse
{
public:
    ApiKeyRevokeResult() = default;

    explicit ApiKeyRevokeResult(app::dto::api_keys::RevokeApiKeyResponse value) noexcept :
        RevokeApiKeyResponse{std::move(value)}
    {
    }
};

// The list route serialized a std::vector<RetrieveApiKeyResponse> directly.
// The result carries the same vector; the handler serializes get_keys() so
// the wire stays a top-level JSON array rather than an object wrapper.
class ApiKeyListResult
{
public:
    ApiKeyListResult() = default;

    explicit ApiKeyListResult(std::vector<app::dto::api_keys::RetrieveApiKeyResponse> keys) noexcept
        :
        m_keys{std::move(keys)}
    {
    }

    [[nodiscard]] const std::vector<app::dto::api_keys::RetrieveApiKeyResponse>&
    get_keys() const noexcept
    {
        return m_keys;
    }

    void set_keys(std::vector<app::dto::api_keys::RetrieveApiKeyResponse> keys)
    {
        m_keys = std::move(keys);
    }

private:
    std::vector<app::dto::api_keys::RetrieveApiKeyResponse> m_keys{};
};

} // namespace app::handlers::api_keys

template<>
struct serde::Serializable<app::handlers::api_keys::ApiKeyCreateResult>
{
    static constexpr auto fields()
    {
        return serde::Serializable<app::dto::api_keys::CreateApiKeyResponse>::fields();
    }

    static constexpr std::string_view table_name()
    {
        return "api_key_create_result";
    }
};

template<>
struct serde::Serializable<app::handlers::api_keys::ApiKeyRetrieveResult>
{
    static constexpr auto fields()
    {
        return serde::Serializable<app::dto::api_keys::RetrieveApiKeyResponse>::fields();
    }

    static constexpr std::string_view table_name()
    {
        return "api_key_retrieve_result";
    }
};

template<>
struct serde::Serializable<app::handlers::api_keys::ApiKeyUpdateResult>
{
    static constexpr auto fields()
    {
        return serde::Serializable<app::dto::api_keys::RetrieveApiKeyResponse>::fields();
    }

    static constexpr std::string_view table_name()
    {
        return "api_key_update_result";
    }
};

template<>
struct serde::Serializable<app::handlers::api_keys::ApiKeyRevokeResult>
{
    static constexpr auto fields()
    {
        return serde::Serializable<app::dto::api_keys::RevokeApiKeyResponse>::fields();
    }

    static constexpr std::string_view table_name()
    {
        return "api_key_revoke_result";
    }
};
