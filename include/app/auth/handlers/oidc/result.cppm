export module app_auth:oidc_result;

import std;
import interfaces;
import core_serde;
import core_error;
import app_dto;

export namespace app::handlers::oidc {

/**
 * @brief Serializable shape of the discovery response. Derives from the exact DTO the old
 * handler built, so field names, order and wrapper types stay byte-identical.
 */
class OidcDiscoveryResult : public ::app::dto::oidc::OidcDiscoveryResponse
{
public:
    OidcDiscoveryResult() = default;

    explicit OidcDiscoveryResult(::app::dto::oidc::OidcDiscoveryResponse value) noexcept :
        OidcDiscoveryResponse{std::move(value)}
    {
    }
};

// ── RFC 6749 error wire shapes from core/errors/oidc.rs ─────────────────────
// Error bodies travel as core::error::AppError: kind_name carries the upstream code
// (OI_01..OI_08, IR_51), message carries the human-readable text, and get_status()
// supplies the HTTP status. The runtime still puts OidcErrorKind on the Result so the
// handler can build the same AppError without touching raw JSON text.

namespace oidc_wire {

    enum class OidcErrorKind : std::uint8_t
    {
        InvalidRequest,
        UnauthorizedClient,
        AccessDenied,
        InvalidScope,
        ServerError,
        InvalidTokenRequest,
        InvalidClient,
        InvalidGrant,
        InvalidBasicAuth,
    };

    inline core::error::AppError oidc_error(OidcErrorKind kind)
    {
        using AppErrorKind = core::error::AppErrorKind;
        std::string_view code{};
        std::string_view description{};
        int identifier{0};
        AppErrorKind app_kind{AppErrorKind::INVALID_REQUEST};

        switch (kind) {
            case OidcErrorKind::InvalidRequest:
                code = "invalid_request";
                description =
                    "The request is missing a required parameter, includes an invalid parameter "
                    "value, includes a parameter more than once, or is otherwise malformed";
                identifier = 1;
                app_kind = AppErrorKind::INVALID_REQUEST;
                break;
            case OidcErrorKind::UnauthorizedClient:
                code = "unauthorized_client";
                description = "The client is not authorized to request an authorization code using "
                              "this method";
                identifier = 2;
                app_kind = AppErrorKind::UNAUTHORIZED;
                break;
            case OidcErrorKind::AccessDenied:
                code = "access_denied";
                description = "The resource owner or authorization server denied the request";
                identifier = 3;
                app_kind = AppErrorKind::UNAUTHORIZED;
                break;
            case OidcErrorKind::InvalidScope:
                code = "invalid_scope";
                description = "The requested scope is invalid, unknown, or malformed";
                identifier = 4;
                app_kind = AppErrorKind::INVALID_REQUEST;
                break;
            case OidcErrorKind::ServerError:
                code = "server_error";
                description = "The authorization server encountered an unexpected condition that "
                              "prevented it from fulfilling the request";
                identifier = 5;
                app_kind = AppErrorKind::INTERNAL;
                break;
            case OidcErrorKind::InvalidTokenRequest:
                code = "invalid_request";
                description =
                    "The request is missing a required parameter, includes an invalid parameter "
                    "value, includes a parameter more than once, or is otherwise malformed";
                identifier = 6;
                app_kind = AppErrorKind::INVALID_REQUEST;
                break;
            case OidcErrorKind::InvalidClient:
                code = "invalid_client";
                description = "Client authentication failed";
                identifier = 7;
                app_kind = AppErrorKind::UNAUTHORIZED;
                break;
            case OidcErrorKind::InvalidGrant:
                code = "invalid_grant";
                description =
                    "The provided authorization grant is invalid, expired, revoked, or does not "
                    "match the redirection URI used in the authorization request";
                identifier = 8;
                app_kind = AppErrorKind::INVALID_REQUEST;
                break;
            case OidcErrorKind::InvalidBasicAuth:
                code = "Access forbidden, invalid Basic authentication credentials";
                description = {};
                identifier = 51;
                app_kind = AppErrorKind::UNAUTHORIZED;
                break;
        }

        const std::string message = description.empty()
                                        ? std::string{code}
                                        : std::string{code} + ": " + std::string{description};
        const std::string error_code =
            identifier == 51
                ? std::string{"IR_"} + std::to_string(identifier)
                : std::string{"OI_"} + (identifier < 10 ? "0" : "") + std::to_string(identifier);
        auto error = core::error::AppErrorBuilder{}(app_kind, message);
        error.set_kind_name(error_code);
        return error;
    }

} // namespace oidc_wire

// JWKS is a flat {"keys":[...]} document. Derive from the DTO so the handler can serialize
// the typed result directly; the runtime decodes the cached text back into that DTO.
class JwksEndpointResult : public ::app::dto::oidc::JwksResponse
{
public:
    JwksEndpointResult() = default;

    explicit JwksEndpointResult(::app::dto::oidc::JwksResponse document) noexcept :
        JwksResponse{std::move(document)}
    {
    }
};

// authorize has two wire outputs: an RFC 6749 error AppError (varying status) or the
// ApplicationResponse::JsonForRedirection 302 body with a LOCATION header. The success body is
// a typed serde::Value document (Object with two empty Arrays); m_is_error discriminates them.
class OidcAuthorizeResult
{
public:
    OidcAuthorizeResult() = default;

    OidcAuthorizeResult(std::string location, serde::Value document) noexcept :
        m_location{std::move(location)},
        m_document{std::move(document)}
    {
    }

    explicit OidcAuthorizeResult(oidc_wire::OidcErrorKind kind) noexcept :
        m_error_kind{kind},
        m_is_error{true}
    {
    }

    [[nodiscard]] bool is_error() const noexcept
    {
        return m_is_error;
    }

    [[nodiscard]] oidc_wire::OidcErrorKind get_error_kind() const noexcept
    {
        return m_error_kind;
    }

    [[nodiscard]] const std::string& get_location() const noexcept
    {
        return m_location;
    }

    [[nodiscard]] const serde::Value& get_document() const noexcept
    {
        return m_document;
    }

private:
    std::string m_location{};
    serde::Value m_document{};
    oidc_wire::OidcErrorKind m_error_kind{oidc_wire::OidcErrorKind::InvalidRequest};
    bool m_is_error{};
};

// token likewise has an RFC 6749 error branch and a success branch carrying the typed
// OidcTokenResponse; m_is_error discriminates them.
class OidcTokenResult : public ::app::dto::oidc::OidcTokenResponse
{
public:
    OidcTokenResult() = default;

    explicit OidcTokenResult(::app::dto::oidc::OidcTokenResponse response) noexcept :
        OidcTokenResponse{std::move(response)}
    {
    }

    explicit OidcTokenResult(oidc_wire::OidcErrorKind kind) noexcept :
        m_error_kind{kind},
        m_is_error{true}
    {
    }

    [[nodiscard]] bool is_error() const noexcept
    {
        return m_is_error;
    }

    [[nodiscard]] oidc_wire::OidcErrorKind get_error_kind() const noexcept
    {
        return m_error_kind;
    }

private:
    oidc_wire::OidcErrorKind m_error_kind{oidc_wire::OidcErrorKind::InvalidRequest};
    bool m_is_error{};
};

} // namespace app::handlers::oidc

template<>
struct serde::Serializable<::app::handlers::oidc::OidcDiscoveryResult>
{
    static constexpr auto fields()
    {
        return serde::Serializable<::app::dto::oidc::OidcDiscoveryResponse>::fields();
    }

    static constexpr std::string_view table_name()
    {
        return "oidc_discovery_result";
    }
};

template<>
struct serde::Serializable<::app::handlers::oidc::JwksEndpointResult>
{
    static constexpr auto fields()
    {
        return serde::Serializable<::app::dto::oidc::JwksResponse>::fields();
    }

    static constexpr std::string_view table_name()
    {
        return "jwks_endpoint_result";
    }
};

template<>
struct serde::Serializable<::app::handlers::oidc::OidcTokenResult>
{
    static constexpr auto fields()
    {
        return serde::Serializable<::app::dto::oidc::OidcTokenResponse>::fields();
    }

    static constexpr std::string_view table_name()
    {
        return "oidc_token_result";
    }
};
