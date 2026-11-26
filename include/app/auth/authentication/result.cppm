export module app_auth:authentication_result;

import std;
import core_serde;
import app_dto;

export namespace app::handlers::authentication {

// The three DTO-shaped responses inherit their wire fields from the exact DTO
// the old core built and encoded, so field names, order and wrapper types stay
// byte-identical. The Serializable specialization reuses the DTO's FieldDesc
// tuple; those descriptors apply to the derived object just as they do to the
// base object.

class AuthenticationCreateResult : public ::app::dto::authentication::AuthenticationResponse
{
public:
    AuthenticationCreateResult() = default;

    explicit AuthenticationCreateResult(
        ::app::dto::authentication::AuthenticationResponse value
    ) noexcept :
        AuthenticationResponse{std::move(value)}
    {
    }
};

class AuthenticationAuthenticateResult :
    public ::app::dto::authentication::AuthenticationAuthenticateResponse
{
public:
    AuthenticationAuthenticateResult() = default;

    explicit AuthenticationAuthenticateResult(
        ::app::dto::authentication::AuthenticationAuthenticateResponse value
    ) noexcept :
        AuthenticationAuthenticateResponse{std::move(value)}
    {
    }
};

class AuthenticationSyncResult : public ::app::dto::authentication::AuthenticationSyncResponse
{
public:
    AuthenticationSyncResult() = default;

    explicit AuthenticationSyncResult(
        ::app::dto::authentication::AuthenticationSyncResponse value
    ) noexcept :
        AuthenticationSyncResponse{std::move(value)}
    {
    }
};

// Externally-tagged / hand-built wire shapes cannot be expressed by the flat
// DTO enums. These results carry a typed serde::Value document (built from
// typed field assignments in the runtime); the handler encodes it with
// serde::Ser::serialize. No hand-written JSON text crosses this boundary.
class AuthenticationEligibilityResult
{
public:
    AuthenticationEligibilityResult() = default;

    explicit AuthenticationEligibilityResult(serde::Value document) noexcept :
        m_document{std::move(document)}
    {
    }

    [[nodiscard]] const serde::Value& get_document() const noexcept
    {
        return m_document;
    }

    void set_document(serde::Value value)
    {
        m_document = std::move(value);
    }

private:
    serde::Value m_document{};
};

class AuthenticationEligibilityCheckResult
{
public:
    AuthenticationEligibilityCheckResult() = default;

    explicit AuthenticationEligibilityCheckResult(serde::Value document) noexcept :
        m_document{std::move(document)}
    {
    }

    [[nodiscard]] const serde::Value& get_document() const noexcept
    {
        return m_document;
    }

    void set_document(serde::Value value)
    {
        m_document = std::move(value);
    }

private:
    serde::Value m_document{};
};

class AuthenticationRetrieveEligibilityCheckResult
{
public:
    AuthenticationRetrieveEligibilityCheckResult() = default;

    explicit AuthenticationRetrieveEligibilityCheckResult(serde::Value document) noexcept :
        m_document{std::move(document)}
    {
    }

    [[nodiscard]] const serde::Value& get_document() const noexcept
    {
        return m_document;
    }

    void set_document(serde::Value value)
    {
        m_document = std::move(value);
    }

private:
    serde::Value m_document{};
};

class AuthenticationSessionTokenResult
{
public:
    AuthenticationSessionTokenResult() = default;

    explicit AuthenticationSessionTokenResult(serde::Value document) noexcept :
        m_document{std::move(document)}
    {
    }

    [[nodiscard]] const serde::Value& get_document() const noexcept
    {
        return m_document;
    }

    void set_document(serde::Value value)
    {
        m_document = std::move(value);
    }

private:
    serde::Value m_document{};
};

// 302 RedirectionResponse: body is a typed wire document, location is the query URL.
class AuthenticationSyncPostUpdateResult
{
public:
    AuthenticationSyncPostUpdateResult() = default;

    AuthenticationSyncPostUpdateResult(serde::Value document, std::string location) noexcept :
        m_document{std::move(document)},
        m_location{std::move(location)}
    {
    }

    [[nodiscard]] const serde::Value& get_document() const noexcept
    {
        return m_document;
    }

    void set_document(serde::Value value)
    {
        m_document = std::move(value);
    }

    [[nodiscard]] const std::string& get_location() const noexcept
    {
        return m_location;
    }

    void set_location(std::string value)
    {
        m_location = std::move(value);
    }

private:
    serde::Value m_document{};
    std::string m_location{};
};

} // namespace app::handlers::authentication

template<>
struct serde::Serializable<::app::handlers::authentication::AuthenticationCreateResult>
{
    static constexpr auto fields()
    {
        return serde::Serializable<::app::dto::authentication::AuthenticationResponse>::fields();
    }

    static constexpr std::string_view table_name()
    {
        return "authentication_create_result";
    }
};

template<>
struct serde::Serializable<::app::handlers::authentication::AuthenticationAuthenticateResult>
{
    static constexpr auto fields()
    {
        return serde::Serializable<
            ::app::dto::authentication::AuthenticationAuthenticateResponse>::fields();
    }

    static constexpr std::string_view table_name()
    {
        return "authentication_authenticate_result";
    }
};

template<>
struct serde::Serializable<::app::handlers::authentication::AuthenticationSyncResult>
{
    static constexpr auto fields()
    {
        return serde::Serializable<
            ::app::dto::authentication::AuthenticationSyncResponse>::fields();
    }

    static constexpr std::string_view table_name()
    {
        return "authentication_sync_result";
    }
};
