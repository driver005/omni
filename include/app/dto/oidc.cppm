module;
export module app_dto:oidc;

import core_serde;
import core_behavior;

import std;
import core_serde;
import core_behavior;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace app::dto::oidc {

enum class Claim
{
    Aud,
    Email,
    EmailVerified,
    Exp,
    Iat,
    Iss,
    Sub,
    PreferredUsername,
};

enum class GrantType
{
    AuthorizationCode,
};

enum class KeyType
{
    Rsa,
};

enum class KeyUse
{
    Sig,
};

class OidcTokenResponse : public core::behavior::Serializer<OidcTokenResponse>
{
public:
    void set_access_token(std::string value)
    {
        access_token = std::move(value);
    }

    [[nodiscard]] const std::string& get_access_token() const noexcept
    {
        return access_token;
    }

    void set_id_token(std::string value)
    {
        id_token = std::move(value);
    }

    [[nodiscard]] const std::string& get_id_token() const noexcept
    {
        return id_token;
    }

    void set_token_type(std::string value)
    {
        token_type = std::move(value);
    }

    [[nodiscard]] const std::string& get_token_type() const noexcept
    {
        return token_type;
    }

    void set_expires_in(std::uint64_t value)
    {
        expires_in = std::move(value);
    }

    [[nodiscard]] const std::uint64_t& get_expires_in() const noexcept
    {
        return expires_in;
    }

    std::string access_token{};
    std::string id_token{};
    std::string token_type{};
    std::uint64_t expires_in{};
};

enum class ResponseMode
{
    Query,
};

enum class ResponseType
{
    Code,
};

enum class Scope
{
    Openid,
    Email,
    Profile,
};

enum class SigningAlgorithm
{
    Rs256,
};

enum class SubjectType
{
    Public,
};

enum class TokenAuthMethod
{
    ClientSecretBasic,
};

class Jwk : public core::behavior::Serializer<Jwk>
{
public:
    void set_kty(KeyType value)
    {
        kty = std::move(value);
    }

    [[nodiscard]] const KeyType& get_kty() const noexcept
    {
        return kty;
    }

    void set_kid(std::string value)
    {
        kid = std::move(value);
    }

    [[nodiscard]] const std::string& get_kid() const noexcept
    {
        return kid;
    }

    void set_key_use(KeyUse value)
    {
        key_use = std::move(value);
    }

    [[nodiscard]] const KeyUse& get_key_use() const noexcept
    {
        return key_use;
    }

    void set_alg(SigningAlgorithm value)
    {
        alg = std::move(value);
    }

    [[nodiscard]] const SigningAlgorithm& get_alg() const noexcept
    {
        return alg;
    }

    void set_n(std::string value)
    {
        n = std::move(value);
    }

    [[nodiscard]] const std::string& get_n() const noexcept
    {
        return n;
    }

    void set_e(std::string value)
    {
        e = std::move(value);
    }

    [[nodiscard]] const std::string& get_e() const noexcept
    {
        return e;
    }

    KeyType kty{};
    std::string kid{};
    KeyUse key_use{};
    SigningAlgorithm alg{};
    std::string n{};
    std::string e{};
};

class OidcAuthorizeQuery : public core::behavior::Serializer<OidcAuthorizeQuery>
{
public:
    void set_response_type(ResponseType value)
    {
        response_type = std::move(value);
    }

    [[nodiscard]] const ResponseType& get_response_type() const noexcept
    {
        return response_type;
    }

    void set_client_id(std::string value)
    {
        client_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_client_id() const noexcept
    {
        return client_id;
    }

    void set_redirect_uri(std::string value)
    {
        redirect_uri = std::move(value);
    }

    [[nodiscard]] const std::string& get_redirect_uri() const noexcept
    {
        return redirect_uri;
    }

    void set_scope(std::vector<Scope> value)
    {
        scope = std::move(value);
    }

    [[nodiscard]] const std::vector<Scope>& get_scope() const noexcept
    {
        return scope;
    }

    void set_state(std::string value)
    {
        state = std::move(value);
    }

    [[nodiscard]] const std::string& get_state() const noexcept
    {
        return state;
    }

    void set_nonce(std::string value)
    {
        nonce = std::move(value);
    }

    [[nodiscard]] const std::string& get_nonce() const noexcept
    {
        return nonce;
    }

    ResponseType response_type{};
    std::string client_id{};
    std::string redirect_uri{};
    std::vector<Scope> scope{};
    std::string state{};
    std::string nonce{};
};

class OidcDiscoveryResponse : public core::behavior::Serializer<OidcDiscoveryResponse>
{
public:
    void set_issuer(std::string value)
    {
        issuer = std::move(value);
    }

    [[nodiscard]] const std::string& get_issuer() const noexcept
    {
        return issuer;
    }

    void set_authorization_endpoint(std::string value)
    {
        authorization_endpoint = std::move(value);
    }

    [[nodiscard]] const std::string& get_authorization_endpoint() const noexcept
    {
        return authorization_endpoint;
    }

    void set_token_endpoint(std::string value)
    {
        token_endpoint = std::move(value);
    }

    [[nodiscard]] const std::string& get_token_endpoint() const noexcept
    {
        return token_endpoint;
    }

    void set_jwks_uri(std::string value)
    {
        jwks_uri = std::move(value);
    }

    [[nodiscard]] const std::string& get_jwks_uri() const noexcept
    {
        return jwks_uri;
    }

    void set_response_types_supported(std::vector<ResponseType> value)
    {
        response_types_supported = std::move(value);
    }

    [[nodiscard]] const std::vector<ResponseType>& get_response_types_supported() const noexcept
    {
        return response_types_supported;
    }

    void set_response_modes_supported(std::vector<ResponseMode> value)
    {
        response_modes_supported = std::move(value);
    }

    [[nodiscard]] const std::vector<ResponseMode>& get_response_modes_supported() const noexcept
    {
        return response_modes_supported;
    }

    void set_subject_types_supported(std::vector<SubjectType> value)
    {
        subject_types_supported = std::move(value);
    }

    [[nodiscard]] const std::vector<SubjectType>& get_subject_types_supported() const noexcept
    {
        return subject_types_supported;
    }

    void set_id_token_signing_alg_values_supported(std::vector<SigningAlgorithm> value)
    {
        id_token_signing_alg_values_supported = std::move(value);
    }

    [[nodiscard]] const std::vector<SigningAlgorithm>&
    get_id_token_signing_alg_values_supported() const noexcept
    {
        return id_token_signing_alg_values_supported;
    }

    void set_grant_types_supported(std::vector<GrantType> value)
    {
        grant_types_supported = std::move(value);
    }

    [[nodiscard]] const std::vector<GrantType>& get_grant_types_supported() const noexcept
    {
        return grant_types_supported;
    }

    void set_scopes_supported(std::vector<Scope> value)
    {
        scopes_supported = std::move(value);
    }

    [[nodiscard]] const std::vector<Scope>& get_scopes_supported() const noexcept
    {
        return scopes_supported;
    }

    void set_token_endpoint_auth_methods_supported(std::vector<TokenAuthMethod> value)
    {
        token_endpoint_auth_methods_supported = std::move(value);
    }

    [[nodiscard]] const std::vector<TokenAuthMethod>&
    get_token_endpoint_auth_methods_supported() const noexcept
    {
        return token_endpoint_auth_methods_supported;
    }

    void set_claims_supported(std::vector<Claim> value)
    {
        claims_supported = std::move(value);
    }

    [[nodiscard]] const std::vector<Claim>& get_claims_supported() const noexcept
    {
        return claims_supported;
    }

    void set_claims_parameter_supported(bool value)
    {
        claims_parameter_supported = std::move(value);
    }

    [[nodiscard]] const bool& get_claims_parameter_supported() const noexcept
    {
        return claims_parameter_supported;
    }

    void set_request_parameter_supported(bool value)
    {
        request_parameter_supported = std::move(value);
    }

    [[nodiscard]] const bool& get_request_parameter_supported() const noexcept
    {
        return request_parameter_supported;
    }

    void set_request_uri_parameter_supported(bool value)
    {
        request_uri_parameter_supported = std::move(value);
    }

    [[nodiscard]] const bool& get_request_uri_parameter_supported() const noexcept
    {
        return request_uri_parameter_supported;
    }

    std::string issuer{};
    std::string authorization_endpoint{};
    std::string token_endpoint{};
    std::string jwks_uri{};
    std::vector<ResponseType> response_types_supported{};
    std::vector<ResponseMode> response_modes_supported{};
    std::vector<SubjectType> subject_types_supported{};
    std::vector<SigningAlgorithm> id_token_signing_alg_values_supported{};
    std::vector<GrantType> grant_types_supported{};
    std::vector<Scope> scopes_supported{};
    std::vector<TokenAuthMethod> token_endpoint_auth_methods_supported{};
    std::vector<Claim> claims_supported{};
    bool claims_parameter_supported{};
    bool request_parameter_supported{};
    bool request_uri_parameter_supported{};
};

class OidcTokenRequest : public core::behavior::Serializer<OidcTokenRequest>
{
public:
    void set_grant_type(GrantType value)
    {
        grant_type = std::move(value);
    }

    [[nodiscard]] const GrantType& get_grant_type() const noexcept
    {
        return grant_type;
    }

    void set_code(std::string value)
    {
        code = std::move(value);
    }

    [[nodiscard]] const std::string& get_code() const noexcept
    {
        return code;
    }

    void set_redirect_uri(std::string value)
    {
        redirect_uri = std::move(value);
    }

    [[nodiscard]] const std::string& get_redirect_uri() const noexcept
    {
        return redirect_uri;
    }

    GrantType grant_type{};
    std::string code{};
    std::string redirect_uri{};
};

class JwksResponse : public core::behavior::Serializer<JwksResponse>
{
public:
    void set_keys(std::vector<Jwk> value)
    {
        keys = std::move(value);
    }

    [[nodiscard]] const std::vector<Jwk>& get_keys() const noexcept
    {
        return keys;
    }

    std::vector<Jwk> keys{};
};
} // namespace app::dto::oidc

template<>
struct serde::Serializable<app::dto::oidc::OidcDiscoveryResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "issuer",
                &app::dto::oidc::OidcDiscoveryResponse::get_issuer,
                &app::dto::oidc::OidcDiscoveryResponse::set_issuer,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "authorization_endpoint",
                &app::dto::oidc::OidcDiscoveryResponse::get_authorization_endpoint,
                &app::dto::oidc::OidcDiscoveryResponse::set_authorization_endpoint,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "token_endpoint",
                &app::dto::oidc::OidcDiscoveryResponse::get_token_endpoint,
                &app::dto::oidc::OidcDiscoveryResponse::set_token_endpoint,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "jwks_uri",
                &app::dto::oidc::OidcDiscoveryResponse::get_jwks_uri,
                &app::dto::oidc::OidcDiscoveryResponse::set_jwks_uri,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "response_types_supported",
                &app::dto::oidc::OidcDiscoveryResponse::get_response_types_supported,
                &app::dto::oidc::OidcDiscoveryResponse::set_response_types_supported,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "response_modes_supported",
                &app::dto::oidc::OidcDiscoveryResponse::get_response_modes_supported,
                &app::dto::oidc::OidcDiscoveryResponse::set_response_modes_supported,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "subject_types_supported",
                &app::dto::oidc::OidcDiscoveryResponse::get_subject_types_supported,
                &app::dto::oidc::OidcDiscoveryResponse::set_subject_types_supported,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "id_token_signing_alg_values_supported",
                &app::dto::oidc::OidcDiscoveryResponse::get_id_token_signing_alg_values_supported,
                &app::dto::oidc::OidcDiscoveryResponse::set_id_token_signing_alg_values_supported,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "grant_types_supported",
                &app::dto::oidc::OidcDiscoveryResponse::get_grant_types_supported,
                &app::dto::oidc::OidcDiscoveryResponse::set_grant_types_supported,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "scopes_supported",
                &app::dto::oidc::OidcDiscoveryResponse::get_scopes_supported,
                &app::dto::oidc::OidcDiscoveryResponse::set_scopes_supported,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "token_endpoint_auth_methods_supported",
                &app::dto::oidc::OidcDiscoveryResponse::get_token_endpoint_auth_methods_supported,
                &app::dto::oidc::OidcDiscoveryResponse::set_token_endpoint_auth_methods_supported,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "claims_supported",
                &app::dto::oidc::OidcDiscoveryResponse::get_claims_supported,
                &app::dto::oidc::OidcDiscoveryResponse::set_claims_supported,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "claims_parameter_supported",
                &app::dto::oidc::OidcDiscoveryResponse::get_claims_parameter_supported,
                &app::dto::oidc::OidcDiscoveryResponse::set_claims_parameter_supported,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "request_parameter_supported",
                &app::dto::oidc::OidcDiscoveryResponse::get_request_parameter_supported,
                &app::dto::oidc::OidcDiscoveryResponse::set_request_parameter_supported,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "request_uri_parameter_supported",
                &app::dto::oidc::OidcDiscoveryResponse::get_request_uri_parameter_supported,
                &app::dto::oidc::OidcDiscoveryResponse::set_request_uri_parameter_supported,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "oidc_discovery_response";
    }
};

template<>
struct serde::Serializable<app::dto::oidc::Jwk>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "kty",
                &app::dto::oidc::Jwk::get_kty,
                &app::dto::oidc::Jwk::set_kty,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "kid",
                &app::dto::oidc::Jwk::get_kid,
                &app::dto::oidc::Jwk::set_kid,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "use",
                &app::dto::oidc::Jwk::get_key_use,
                &app::dto::oidc::Jwk::set_key_use,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "alg",
                &app::dto::oidc::Jwk::get_alg,
                &app::dto::oidc::Jwk::set_alg,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "n",
                &app::dto::oidc::Jwk::get_n,
                &app::dto::oidc::Jwk::set_n,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "e",
                &app::dto::oidc::Jwk::get_e,
                &app::dto::oidc::Jwk::set_e,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "jwk";
    }
};

template<>
struct serde::Serializable<app::dto::oidc::JwksResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<
            "keys",
            &app::dto::oidc::JwksResponse::get_keys,
            &app::dto::oidc::JwksResponse::set_keys,
            serde::FieldOptions{}>{}};
    }

    static constexpr std::string_view table_name()
    {
        return "jwks_response";
    }
};

template<>
struct serde::Serializable<app::dto::oidc::OidcAuthorizeQuery>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "response_type",
                &app::dto::oidc::OidcAuthorizeQuery::get_response_type,
                &app::dto::oidc::OidcAuthorizeQuery::set_response_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "client_id",
                &app::dto::oidc::OidcAuthorizeQuery::get_client_id,
                &app::dto::oidc::OidcAuthorizeQuery::set_client_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "redirect_uri",
                &app::dto::oidc::OidcAuthorizeQuery::get_redirect_uri,
                &app::dto::oidc::OidcAuthorizeQuery::set_redirect_uri,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "scope",
                &app::dto::oidc::OidcAuthorizeQuery::get_scope,
                &app::dto::oidc::OidcAuthorizeQuery::set_scope,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "state",
                &app::dto::oidc::OidcAuthorizeQuery::get_state,
                &app::dto::oidc::OidcAuthorizeQuery::set_state,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "nonce",
                &app::dto::oidc::OidcAuthorizeQuery::get_nonce,
                &app::dto::oidc::OidcAuthorizeQuery::set_nonce,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "oidc_authorize_query";
    }
};

template<>
struct serde::Serializable<app::dto::oidc::OidcTokenRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "grant_type",
                &app::dto::oidc::OidcTokenRequest::get_grant_type,
                &app::dto::oidc::OidcTokenRequest::set_grant_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "code",
                &app::dto::oidc::OidcTokenRequest::get_code,
                &app::dto::oidc::OidcTokenRequest::set_code,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "redirect_uri",
                &app::dto::oidc::OidcTokenRequest::get_redirect_uri,
                &app::dto::oidc::OidcTokenRequest::set_redirect_uri,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "oidc_token_request";
    }
};

template<>
struct serde::Serializable<app::dto::oidc::OidcTokenResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "access_token",
                &app::dto::oidc::OidcTokenResponse::get_access_token,
                &app::dto::oidc::OidcTokenResponse::set_access_token,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "id_token",
                &app::dto::oidc::OidcTokenResponse::get_id_token,
                &app::dto::oidc::OidcTokenResponse::set_id_token,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "token_type",
                &app::dto::oidc::OidcTokenResponse::get_token_type,
                &app::dto::oidc::OidcTokenResponse::set_token_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "expires_in",
                &app::dto::oidc::OidcTokenResponse::get_expires_in,
                &app::dto::oidc::OidcTokenResponse::set_expires_in,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "oidc_token_response";
    }
};

#ifdef CONGELADO_TEST
namespace app::dto::oidc::tests {
using namespace boost::ut;
suite<"app_dto_oidc"> app_dto_oidc_suite = []
{
    "reflects converted types"_test = []
    {
        expect(
            std::tuple_size_v<
                decltype(serde::Serializable<app::dto::oidc::OidcDiscoveryResponse>::fields())> > 0
        );
    };
};
} // namespace app::dto::oidc::tests
#endif
