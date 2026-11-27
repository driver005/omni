export module app_auth:oidc_result;

import std;
import interfaces;
import core_serde;
import core_error;
import app_dto;

export namespace app::handlers::oidc {

/**
 * @brief Exact wire field lists captured from the retired Result classes' Serializable
 * specializations. Each mirrors the corresponding DTO FieldDesc tuple in declaration order.
 */
[[nodiscard]] inline constexpr auto discovery_wire_fields()
{
    return std::array<std::string_view, 15>{
        "issuer",
        "authorization_endpoint",
        "token_endpoint",
        "jwks_uri",
        "response_types_supported",
        "response_modes_supported",
        "subject_types_supported",
        "id_token_signing_alg_values_supported",
        "grant_types_supported",
        "scopes_supported",
        "token_endpoint_auth_methods_supported",
        "claims_supported",
        "claims_parameter_supported",
        "request_parameter_supported",
        "request_uri_parameter_supported"
    };
}

[[nodiscard]] inline constexpr auto jwks_wire_fields()
{
    return std::array<std::string_view, 1>{"keys"};
}

[[nodiscard]] inline constexpr auto token_wire_fields()
{
    return std::array<std::string_view, 4>{"access_token", "id_token", "token_type", "expires_in"};
}

// ── RFC 6749 error wire shapes from core/errors/oidc.rs ─────────────────────
// Error bodies travel as core::error::AppError: kind_name carries the upstream code
// (OI_01..OI_08, IR_51), message carries the human-readable text, and get_status()
// supplies the HTTP status. The runtime turns an OidcErrorKind into the same AppError and
// returns it as std::unexpected; the handler sends it unchanged.

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

} // namespace app::handlers::oidc
