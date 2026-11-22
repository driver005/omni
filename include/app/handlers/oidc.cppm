module;
export module app_handlers:oidc;

import std;
import interfaces;
import core_router;
import core_serde;
import utils_openapi;
import app_model;
import app_dto;
import app_context;
import core_cache;
import core_error;
import app_auth;
import app_http;
import app_crypto;
import app_metrics;
import app_store;
import app_keystore;
import app_services;

export namespace app::handlers::oidc {

/**
 * @brief Serializable shape of upstream `types/domain/user/oidc.rs::AuthCodeData`.
 *
 * Field set and order mirror upstream exactly (sub, client_id, redirect_uri,
 * scope, nonce, email, is_verified). The injected cache stands in for Redis.
 */
class StoredAuthCode
{
public:
    void set_sub(std::string value) { sub = std::move(value); }
    [[nodiscard]] const std::string& get_sub() const noexcept { return sub; }
    void set_client_id(std::string value) { client_id = std::move(value); }
    [[nodiscard]] const std::string& get_client_id() const noexcept { return client_id; }
    void set_redirect_uri(std::string value) { redirect_uri = std::move(value); }
    [[nodiscard]] const std::string& get_redirect_uri() const noexcept { return redirect_uri; }
    void set_scope(std::vector<app::dto::oidc::Scope> value) { scope = std::move(value); }
    [[nodiscard]] const std::vector<app::dto::oidc::Scope>& get_scope() const noexcept { return scope; }
    void set_nonce(std::string value) { nonce = std::move(value); }
    [[nodiscard]] const std::string& get_nonce() const noexcept { return nonce; }
    void set_email(std::string value) { email = std::move(value); }
    [[nodiscard]] const std::string& get_email() const noexcept { return email; }
    void set_is_verified(bool value) { is_verified = value; }
    [[nodiscard]] bool get_is_verified() const noexcept { return is_verified; }

    std::string sub{};
    std::string client_id{};
    std::string redirect_uri{};
    std::vector<app::dto::oidc::Scope> scope{};
    std::string nonce{};
    std::string email{};
    bool is_verified{};
};

} // namespace app::handlers::oidc

template<>
struct serde::Serializable<app::handlers::oidc::StoredAuthCode>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"sub", &app::handlers::oidc::StoredAuthCode::get_sub, &app::handlers::oidc::StoredAuthCode::set_sub, serde::FieldOptions{}>{},
            serde::FieldDesc<"client_id", &app::handlers::oidc::StoredAuthCode::get_client_id, &app::handlers::oidc::StoredAuthCode::set_client_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"redirect_uri", &app::handlers::oidc::StoredAuthCode::get_redirect_uri, &app::handlers::oidc::StoredAuthCode::set_redirect_uri, serde::FieldOptions{}>{},
            serde::FieldDesc<"scope", &app::handlers::oidc::StoredAuthCode::get_scope, &app::handlers::oidc::StoredAuthCode::set_scope, serde::FieldOptions{}>{},
            serde::FieldDesc<"nonce", &app::handlers::oidc::StoredAuthCode::get_nonce, &app::handlers::oidc::StoredAuthCode::set_nonce, serde::FieldOptions{}>{},
            serde::FieldDesc<"email", &app::handlers::oidc::StoredAuthCode::get_email, &app::handlers::oidc::StoredAuthCode::set_email, serde::FieldOptions{}>{},
            serde::FieldDesc<"is_verified", &app::handlers::oidc::StoredAuthCode::get_is_verified, &app::handlers::oidc::StoredAuthCode::set_is_verified, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "oidc_auth_code_data"; }
};

namespace app::handlers::oidc::detail {

inline std::string_view trim(std::string_view value) noexcept
{
    while (!value.empty() && std::isspace(static_cast<unsigned char>(value.front()))) {
        value.remove_prefix(1);
    }
    while (!value.empty() && std::isspace(static_cast<unsigned char>(value.back()))) {
        value.remove_suffix(1);
    }
    return value;
}

/** @brief Minimal JSON string-field reader for local config records. */
inline std::optional<std::string> json_string_field(std::string_view json, std::string_view key)
{
    const std::string needle = std::string{"\""} + std::string{key} + "\"";
    const auto key_pos = json.find(needle);
    if (key_pos == std::string_view::npos) {
        return std::nullopt;
    }
    std::size_t pos = key_pos + needle.size();
    while (pos < json.size() && std::isspace(static_cast<unsigned char>(json[pos]))) {
        ++pos;
    }
    if (pos >= json.size() || json[pos] != ':') {
        return std::nullopt;
    }
    ++pos;
    while (pos < json.size() && std::isspace(static_cast<unsigned char>(json[pos]))) {
        ++pos;
    }
    if (pos >= json.size() || json[pos] != '"') {
        return std::nullopt;
    }
    ++pos;
    std::string out{};
    while (pos < json.size()) {
        const char ch = json[pos++];
        if (ch == '\\' && pos < json.size()) {
            const char escaped = json[pos++];
            switch (escaped) {
                case 'n': out.push_back('\n'); break;
                case 'r': out.push_back('\r'); break;
                case 't': out.push_back('\t'); break;
                default: out.push_back(escaped); break;
            }
            continue;
        }
        if (ch == '"') {
            return out;
        }
        out.push_back(ch);
    }
    return std::nullopt;
}

// ── standard base64 (PEM / HTTP Basic), distinct from app::crypto::Base64Url ─

inline std::optional<std::vector<unsigned char>> decode_standard_base64(std::string_view input)
{
    const auto value_of = [](char ch) -> int
    {
        if (ch >= 'A' && ch <= 'Z') {
            return ch - 'A';
        }
        if (ch >= 'a' && ch <= 'z') {
            return ch - 'a' + 26;
        }
        if (ch >= '0' && ch <= '9') {
            return ch - '0' + 52;
        }
        if (ch == '+') {
            return 62;
        }
        if (ch == '/') {
            return 63;
        }
        return -1;
    };
    std::vector<unsigned char> out{};
    std::uint32_t accumulator{0};
    int bits{0};
    for (const char ch : input) {
        if (ch == '=' || std::isspace(static_cast<unsigned char>(ch))) {
            continue;
        }
        const int value = value_of(ch);
        if (value < 0) {
            return std::nullopt;
        }
        accumulator = (accumulator << 6) | static_cast<std::uint32_t>(value);
        bits += 6;
        if (bits >= 8) {
            bits -= 8;
            out.push_back(static_cast<unsigned char>((accumulator >> bits) & 0xFFU));
        }
    }
    return out;
}

// ── minimal DER reader for the two PEM tags upstream accepts ───────────────

struct DerElement
{
    unsigned char tag{};
    std::string_view value{};
    std::size_t end{};
};

inline std::optional<DerElement> read_der(std::string_view buffer, std::size_t pos)
{
    if (pos + 2 > buffer.size()) {
        return std::nullopt;
    }
    DerElement element{};
    element.tag = static_cast<unsigned char>(buffer[pos++]);
    const auto length_byte = static_cast<unsigned char>(buffer[pos++]);
    std::size_t length{0};
    if ((length_byte & 0x80U) == 0) {
        length = length_byte;
    } else {
        const std::size_t count = length_byte & 0x7FU;
        if (count == 0 || count > sizeof(std::size_t) || pos + count > buffer.size()) {
            return std::nullopt;
        }
        for (std::size_t i{0}; i < count; ++i) {
            length = (length << 8) | static_cast<unsigned char>(buffer[pos++]);
        }
    }
    if (pos + length > buffer.size()) {
        return std::nullopt;
    }
    element.value = buffer.substr(pos, length);
    element.end = pos + length;
    return element;
}

inline std::optional<std::string> der_integer_bytes(
    std::string_view buffer, std::size_t pos, std::size_t& end
)
{
    const auto element = read_der(buffer, pos);
    if (!element || element->tag != 0x02U) {
        return std::nullopt;
    }
    end = element->end;
    std::size_t skip{0};
    while (skip + 1 < element->value.size() && element->value[skip] == '\0') {
        ++skip;
    }
    return std::string{element->value.substr(skip)};
}

/**
 * @brief Port of upstream `common_utils::crypto::extract_rsa_public_key_components`.
 *
 * Parses PKCS#8 ("PRIVATE KEY") and PKCS#1 ("RSA PRIVATE KEY") PEM, takes the
 * modulus and public exponent as big-endian bytes and base64url-encodes them
 * without padding using app::crypto::Base64Url.
 */
inline std::optional<std::pair<std::string, std::string>> extract_rsa_public_key_components(
    std::string_view private_key_pem
)
{
    const auto begin = private_key_pem.find("-----BEGIN ");
    if (begin == std::string_view::npos) {
        return std::nullopt;
    }
    const auto header_end = private_key_pem.find("-----", begin + 11);
    if (header_end == std::string_view::npos) {
        return std::nullopt;
    }
    const auto tag = trim(private_key_pem.substr(begin + 11, header_end - (begin + 11)));
    if (tag != "PRIVATE KEY" && tag != "RSA PRIVATE KEY") {
        return std::nullopt;
    }
    const auto body_begin = private_key_pem.find('\n', header_end + 5);
    if (body_begin == std::string_view::npos) {
        return std::nullopt;
    }
    const auto footer = private_key_pem.find("-----END", body_begin + 1);
    if (footer == std::string_view::npos) {
        return std::nullopt;
    }
    std::string body{};
    for (const char ch : private_key_pem.substr(body_begin + 1, footer - (body_begin + 1))) {
        if (!std::isspace(static_cast<unsigned char>(ch))) {
            body.push_back(ch);
        }
    }
    const auto der_bytes = decode_standard_base64(body);
    if (!der_bytes) {
        return std::nullopt;
    }
    const std::string der{der_bytes->begin(), der_bytes->end()};
    const std::string_view outer_buffer{der};

    const auto outer = read_der(outer_buffer, 0);
    if (!outer || outer->tag != 0x30U) {
        return std::nullopt;
    }
    std::size_t pos{0};
    const auto version = read_der(outer->value, pos);
    if (!version || version->tag != 0x02U) {
        return std::nullopt;
    }
    pos = version->end;

    std::string_view rsa_value = outer->value;
    if (pos < outer->value.size() && static_cast<unsigned char>(outer->value[pos]) == 0x30U) {
        // PKCS#8: version, AlgorithmIdentifier, OCTET STRING(RSAPrivateKey).
        const auto algorithm = read_der(outer->value, pos);
        if (!algorithm || algorithm->tag != 0x30U) {
            return std::nullopt;
        }
        pos = algorithm->end;
        const auto octets = read_der(outer->value, pos);
        if (!octets || octets->tag != 0x04U) {
            return std::nullopt;
        }
        rsa_value = octets->value;
        const auto inner = read_der(rsa_value, 0);
        if (!inner || inner->tag != 0x30U) {
            return std::nullopt;
        }
        rsa_value = inner->value;
    }

    std::size_t cursor{0};
    const auto inner_version = read_der(rsa_value, cursor);
    if (!inner_version || inner_version->tag != 0x02U) {
        return std::nullopt;
    }
    cursor = inner_version->end;
    std::size_t modulus_end{0};
    std::size_t exponent_end{0};
    const auto modulus = der_integer_bytes(rsa_value, cursor, modulus_end);
    if (!modulus) {
        return std::nullopt;
    }
    const auto exponent = der_integer_bytes(rsa_value, modulus_end, exponent_end);
    if (!exponent) {
        return std::nullopt;
    }
    const app::crypto::Base64Url codec{};
    return std::make_pair(codec.encode(*modulus), codec.encode(*exponent));
}

// ── URL helpers ─────────────────────────────────────────────────────────────

inline std::string form_url_encode(std::string_view value)
{
    constexpr std::string_view hex{"0123456789ABCDEF"};
    std::string out{};
    for (const char raw : value) {
        const auto ch = static_cast<unsigned char>(raw);
        if ((ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z') ||
            (ch >= '0' && ch <= '9') || ch == '-' || ch == '_' || ch == '.' || ch == '~') {
            out.push_back(static_cast<char>(ch));
        } else if (ch == ' ') {
            out.push_back('+');
        } else {
            out.push_back('%');
            out.push_back(hex[(ch >> 4) & 0xFU]);
            out.push_back(hex[ch & 0xFU]);
        }
    }
    return out;
}

/** @brief Port of upstream `validate_redirect_uri_match`. */
inline bool validate_redirect_uri_match(std::string_view registered, std::string_view provided)
{
    const auto left = trim(registered);
    const auto right = trim(provided);
    if (left == right) {
        return true;
    }
    const auto normalize = [](std::string_view url) -> std::optional<std::string>
    {
        const auto scheme_end = url.find("://");
        if (scheme_end == std::string_view::npos || scheme_end == 0) {
            return std::nullopt;
        }
        std::string scheme{url.substr(0, scheme_end)};
        std::transform(scheme.begin(), scheme.end(), scheme.begin(), [](unsigned char ch)
        {
            return static_cast<char>(std::tolower(ch));
        });
        if (scheme != "http" && scheme != "https") {
            return std::nullopt;
        }
        const auto rest = url.substr(scheme_end + 3);
        if (rest.empty()) {
            return std::nullopt;
        }
        const auto authority_end = rest.find_first_of("/?#");
        const auto authority = rest.substr(0, authority_end);
        if (authority.empty()) {
            return std::nullopt;
        }
        std::string host{authority};
        if (const auto colon = host.find(':'); colon != std::string::npos) {
            const auto port = host.substr(colon + 1);
            if ((scheme == "http" && port == "80") || (scheme == "https" && port == "443")) {
                host = host.substr(0, colon);
            }
        }
        std::transform(host.begin(), host.end(), host.begin(), [](unsigned char ch)
        {
            return static_cast<char>(std::tolower(ch));
        });
        std::string path = authority_end == std::string_view::npos
            ? std::string{"/"}
            : std::string{rest.substr(authority_end)};
        if (path.empty()) {
            path = "/";
        }
        return scheme + "://" + host + path;
    };
    const auto normalized_left = normalize(left);
    const auto normalized_right = normalize(right);
    return normalized_left && normalized_right && *normalized_left == *normalized_right;
}

/** @brief Port of upstream `build_oidc_redirect_url` (code/state query params). */
inline std::optional<std::string> build_oidc_redirect_url(
    std::string_view redirect_uri, std::string_view code, std::string_view state
)
{
    const auto url = trim(redirect_uri);
    const auto scheme_end = url.find("://");
    if (url.empty() || scheme_end == std::string_view::npos || scheme_end == 0) {
        return std::nullopt;
    }
    const auto scheme = url.substr(0, scheme_end);
    if (scheme != "http" && scheme != "https") {
        return std::nullopt;
    }
    const auto rest = url.substr(scheme_end + 3);
    const auto host_end = rest.find_first_of("/?#");
    if (rest.empty() || host_end == 0) {
        return std::nullopt;
    }
    std::string base{url};
    const auto fragment_pos = base.find('#');
    std::string fragment{};
    if (fragment_pos != std::string::npos) {
        fragment = base.substr(fragment_pos);
        base = base.substr(0, fragment_pos);
    }
    base += base.find('?') == std::string::npos ? "?" : "&";
    base += "code=" + form_url_encode(code) + "&state=" + form_url_encode(state);
    return base + fragment;
}

// ── deterministic 32-char alphanumeric authorization code ──────────────────

inline std::string make_alphanumeric_code(std::string_view seed, std::size_t length)
{
    constexpr std::string_view alphabet{
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789"};
    std::string out{};
    out.reserve(length);
    std::uint64_t state{14695981039346656037ULL};
    for (const char ch : seed) {
        state ^= static_cast<unsigned char>(ch);
        state *= 1099511628211ULL;
    }
    for (std::size_t i{0}; i < length; ++i) {
        state ^= state << 13;
        state ^= state >> 7;
        state ^= state << 17;
        out.push_back(alphabet[static_cast<std::size_t>(state % alphabet.size())]);
    }
    return out;
}

// ── RFC 6749 error wire shapes from core/errors/oidc.rs ─────────────────────

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

struct OidcErrorShape
{
    interfaces::io::types::Status status{};
    std::string body{};
};

inline OidcErrorShape oidc_error(OidcErrorKind kind)
{
    using Status = interfaces::io::types::Status;
    std::string_view code{};
    std::string_view description{};
    int identifier{0};
    bool api_type{false};
    Status status{Status::BAD_REQUEST};

    switch (kind) {
        case OidcErrorKind::InvalidRequest:
            code = "invalid_request";
            description = "The request is missing a required parameter, includes an invalid parameter value, includes a parameter more than once, or is otherwise malformed";
            identifier = 1;
            status = Status::BAD_REQUEST;
            break;
        case OidcErrorKind::UnauthorizedClient:
            code = "unauthorized_client";
            description = "The client is not authorized to request an authorization code using this method";
            identifier = 2;
            status = Status::UNAUTHORIZED;
            break;
        case OidcErrorKind::AccessDenied:
            code = "access_denied";
            description = "The resource owner or authorization server denied the request";
            identifier = 3;
            status = Status::UNAUTHORIZED;
            break;
        case OidcErrorKind::InvalidScope:
            code = "invalid_scope";
            description = "The requested scope is invalid, unknown, or malformed";
            identifier = 4;
            status = Status::BAD_REQUEST;
            break;
        case OidcErrorKind::ServerError:
            code = "server_error";
            description = "The authorization server encountered an unexpected condition that prevented it from fulfilling the request";
            identifier = 5;
            status = Status::INTERNAL_SERVER_ERROR;
            api_type = true;
            break;
        case OidcErrorKind::InvalidTokenRequest:
            code = "invalid_request";
            description = "The request is missing a required parameter, includes an invalid parameter value, includes a parameter more than once, or is otherwise malformed";
            identifier = 6;
            status = Status::BAD_REQUEST;
            break;
        case OidcErrorKind::InvalidClient:
            code = "invalid_client";
            description = "Client authentication failed";
            identifier = 7;
            status = Status::UNAUTHORIZED;
            break;
        case OidcErrorKind::InvalidGrant:
            code = "invalid_grant";
            description = "The provided authorization grant is invalid, expired, revoked, or does not match the redirection URI used in the authorization request";
            identifier = 8;
            status = Status::BAD_REQUEST;
            break;
        case OidcErrorKind::InvalidBasicAuth:
            code = "Access forbidden, invalid Basic authentication credentials";
            description = {};
            identifier = 51;
            status = Status::UNAUTHORIZED;
            break;
    }

    const std::string message = description.empty()
        ? std::string{code}
        : std::string{code} + ": " + std::string{description};
    const std::string error_code = identifier == 51
        ? std::string{"IR_"} + std::to_string(identifier)
        : std::string{"OI_"} + (identifier < 10 ? "0" : "") + std::to_string(identifier);
    std::string body = std::string{R"({"error":{"type":")"}
        + std::string{api_type ? "api" : "invalid_request"}
        + R"(","message":)"
        + serde::Ser::encode_json(message)
        + R"(,"code":")" + error_code + R"("}})";
    return {status, std::move(body)};
}

} // namespace app::handlers::oidc::detail

export namespace app::handlers::oidc {

class OidcRouteHandlers
{
public:
    void handle_oidc_discovery(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto auth_error = authorize_operation(req, "oidc_discovery")) {
            send_error(res, send, *auth_error);
            return;
        }

        m_context->metrics().record_flow("oidc_discovery");
        m_context->metrics().counter("oidc.oidc_discovery").increment();
        m_context->metrics().counter("oidc.api_event.Oidc").increment();

        using Response = app::dto::oidc::OidcDiscoveryResponse;
        // Upstream: OidcDiscoveryResponse::new(state.tenant.base_url,
        // user_utils::get_base_url(&state)). Config store supplies both values.
        const auto base = tenant_base_url();
        const auto control = control_center_url();
        Response document{};
        document.set_issuer(base);
        document.set_authorization_endpoint(control + "/oauth2/authorize");
        document.set_token_endpoint(base + "/oauth2/token");
        document.set_jwks_uri(base + "/oauth2/jwks");
        document.set_response_types_supported({app::dto::oidc::ResponseType::Code});
        document.set_response_modes_supported({app::dto::oidc::ResponseMode::Query});
        document.set_subject_types_supported({app::dto::oidc::SubjectType::Public});
        document.set_id_token_signing_alg_values_supported(
            {app::dto::oidc::SigningAlgorithm::Rs256}
        );
        document.set_grant_types_supported({app::dto::oidc::GrantType::AuthorizationCode});
        document.set_scopes_supported({
            app::dto::oidc::Scope::Openid,
            app::dto::oidc::Scope::Email,
            app::dto::oidc::Scope::Profile,
        });
        document.set_token_endpoint_auth_methods_supported(
            {app::dto::oidc::TokenAuthMethod::ClientSecretBasic}
        );
        document.set_claims_supported({
            app::dto::oidc::Claim::Aud,
            app::dto::oidc::Claim::Email,
            app::dto::oidc::Claim::EmailVerified,
            app::dto::oidc::Claim::Exp,
            app::dto::oidc::Claim::Iat,
            app::dto::oidc::Claim::Iss,
            app::dto::oidc::Claim::Sub,
            app::dto::oidc::Claim::PreferredUsername,
        });
        document.set_claims_parameter_supported(false);
        document.set_request_parameter_supported(false);
        document.set_request_uri_parameter_supported(false);
        send_json(res, send, serde::Ser::encode_json(document));
    }

    void handle_jwks_endpoint(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto auth_error = authorize_operation(req, "jwks_endpoint")) {
            send_error(res, send, *auth_error);
            return;
        }

        m_context->metrics().record_flow("jwks_endpoint");
        m_context->metrics().counter("oidc.jwks_endpoint").increment();
        m_context->metrics().counter("oidc.api_event.Oidc").increment();

        ensure_oidc_config();

        using Response = app::dto::oidc::JwksResponse;
        const auto key = std::string{"oidc.jwks_endpoint:document"};
        const auto cached = m_context->cache().get(key);
        std::string text{};
        if (cached.hit) {
            text = cached.value_or<std::string>("");
        } else {
            Response document{};
            std::vector<app::dto::oidc::Jwk> keys{};
            for (const auto& configured : configured_oidc_keys()) {
                const auto components = detail::extract_rsa_public_key_components(
                    configured.private_key
                );
                if (!components) {
                    continue;
                }
                app::dto::oidc::Jwk jwk{};
                jwk.set_kty(app::dto::oidc::KeyType::Rsa);
                jwk.set_kid(configured.kid);
                jwk.set_key_use(app::dto::oidc::KeyUse::Sig);
                jwk.set_alg(app::dto::oidc::SigningAlgorithm::Rs256);
                jwk.set_n(components->first);
                jwk.set_e(components->second);
                keys.push_back(std::move(jwk));
            }
            document.set_keys(std::move(keys));
            text = serde::Ser::encode_json(document);
            m_context->cache().put(key, core::cache::to_cache_value(text));
        }
        send_json(res, send, text);
    }

    void handle_oidc_authorize(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto auth_error = authorize_operation(req, "oidc_authorize")) {
            send_error(res, send, *auth_error);
            return;
        }

        m_context->metrics().record_flow("oidc_authorize");
        m_context->metrics().counter("oidc.oidc_authorize").increment();
        m_context->metrics().counter("oidc.api_event.Oidc").increment();

        ensure_oidc_config();

        // Upstream uses web::Query<OidcAuthorizeQuery> exclusively.
        const auto payload = parse_authorize_query(req);
        if (!payload) {
            send_oidc_error(res, send, detail::OidcErrorKind::InvalidRequest);
            return;
        }

        const auto& scopes = payload->get_scope();
        if (std::find(scopes.begin(), scopes.end(), app::dto::oidc::Scope::Openid)
            == scopes.end()) {
            send_oidc_error(res, send, detail::OidcErrorKind::InvalidScope);
            return;
        }

        const auto client_id = payload->get_client_id();
        const auto registered_client = configured_oidc_client(client_id);
        if (!registered_client) {
            send_oidc_error(res, send, detail::OidcErrorKind::UnauthorizedClient);
            return;
        }
        if (!detail::validate_redirect_uri_match(
                registered_client->redirect_uri, payload->get_redirect_uri())) {
            send_oidc_error(res, send, detail::OidcErrorKind::InvalidRequest);
            return;
        }

        // Upstream: Option<UserFromToken>; missing session means AccessDenied.
        const auto user_id = authenticated_user_id(req);
        if (user_id.empty()) {
            send_oidc_error(res, send, detail::OidcErrorKind::AccessDenied);
            return;
        }
        const auto user = m_context->repository().find_user_by_string(user_id);
        if (!user) {
            send_oidc_error(res, send, detail::OidcErrorKind::ServerError);
            return;
        }

        // Upstream: generate_cryptographically_secure_random_string(32).
        const auto code = detail::make_alphanumeric_code(
            client_id + "|" + payload->get_redirect_uri() + "|" + payload->get_state() + "|" +
                m_context->ids().uuid_v4(),
            32
        );
        StoredAuthCode stored{};
        stored.set_sub(user_id);
        stored.set_client_id(client_id);
        stored.set_redirect_uri(payload->get_redirect_uri());
        stored.set_scope(payload->get_scope());
        stored.set_nonce(payload->get_nonce());
        stored.set_email(user->get_email());
        stored.set_is_verified(user->get_status() == "active");
        m_context->cache().put(
            std::string{REDIS_AUTH_CODE_PREFIX} + code,
            core::cache::to_cache_value(serde::Ser::encode_json(stored)),
            core::cache::Ttl{}.seconds(AUTH_CODE_TTL_IN_SECS)
        );

        const auto redirect_url = detail::build_oidc_redirect_url(
            payload->get_redirect_uri(), code, payload->get_state()
        );
        if (!redirect_url) {
            send_oidc_error(res, send, detail::OidcErrorKind::InvalidRequest);
            return;
        }
        send_redirection_response(res, send, *redirect_url);
    }

    void handle_oidc_token(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        // Upstream applies OIDC_CLIENT_AUTH (HTTP Basic), not a dashboard JWT.
        // The local registry has no OIDC_CLIENT_AUTH scheme; the exact Basic
        // client validation is enforced here (see "Shared contract requests").

        m_context->metrics().record_flow("oidc_token");
        m_context->metrics().counter("oidc.oidc_token").increment();
        m_context->metrics().counter("oidc.api_event.Oidc").increment();

        ensure_oidc_config();

        const auto request = parse_token_request(req);
        if (!request) {
            send_oidc_error(res, send, detail::OidcErrorKind::InvalidRequest);
            return;
        }

        // auth::OIDC_CLIENT_AUTH: Basic client_id/client_secret vs conf.oidc.client.
        const auto basic = parse_basic_auth(req);
        if (!basic) {
            send_oidc_error(res, send, detail::OidcErrorKind::InvalidBasicAuth);
            return;
        }
        const auto client = configured_oidc_client(basic->first);
        if (!client || client->client_secret != basic->second) {
            send_oidc_error(res, send, detail::OidcErrorKind::InvalidBasicAuth);
            return;
        }
        const auto& client_id = client->client_id;

        // validate_token_request
        if (detail::trim(request->get_redirect_uri()).empty()) {
            send_oidc_error(res, send, detail::OidcErrorKind::InvalidTokenRequest);
            return;
        }
        if (!detail::validate_redirect_uri_match(
                client->redirect_uri, request->get_redirect_uri())) {
            send_oidc_error(res, send, detail::OidcErrorKind::InvalidTokenRequest);
            return;
        }

        // validate_and_consume_authorization_code
        const auto code_key = std::string{REDIS_AUTH_CODE_PREFIX} + request->get_code();
        const auto cached = m_context->cache().get(code_key);
        if (!cached.hit) {
            send_oidc_error(res, send, detail::OidcErrorKind::InvalidGrant);
            return;
        }
        const auto stored = cached.value_or<std::string>("");
        m_context->cache().erase(code_key);

        auto auth_data = serde::Ser::deserialize<StoredAuthCode>("application/json", stored);
        if (!auth_data) {
            send_oidc_error(res, send, detail::OidcErrorKind::ServerError);
            return;
        }
        if (detail::trim(auth_data->get_client_id()) != detail::trim(client_id)) {
            send_oidc_error(res, send, detail::OidcErrorKind::InvalidGrant);
            return;
        }
        if (!detail::validate_redirect_uri_match(
                auth_data->get_redirect_uri(), request->get_redirect_uri())) {
            send_oidc_error(res, send, detail::OidcErrorKind::InvalidGrant);
            return;
        }

        const auto issuer = tenant_base_url();
        const auto issued_at = static_cast<std::uint64_t>(
            std::chrono::duration_cast<std::chrono::seconds>(
                std::chrono::system_clock::now().time_since_epoch()
            ).count()
        );

        const auto id_claims = build_id_claims(
            issuer, *auth_data, issued_at, issued_at + ID_TOKEN_TTL_IN_SECS
        );
        const auto access_claims = build_access_claims(
            issuer, *auth_data, issued_at, issued_at + ACCESS_TOKEN_TTL_IN_SECS
        );

        const auto signer = token_signer();
        using Response = app::dto::oidc::OidcTokenResponse;
        Response response{};
        response.set_access_token(signer.sign(access_claims));
        response.set_id_token(signer.sign(id_claims));
        response.set_token_type(std::string{TOKEN_TYPE_BEARER});
        response.set_expires_in(ACCESS_TOKEN_TTL_IN_SECS);
        send_json(res, send, serde::Ser::encode_json(response));
    }

    void install(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        // Upstream mounts these at the root via web::scope(""). Single-segment
        // router/route literals keep the trie resolution exact.
        Router{router, ".well-known"}
            .add_route(
                Route{"openid-configuration"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_oidc_discovery(req, res, std::move(send));
                    })
                    .summary("Hyperswitch oidc_discovery")
            );
        Router{router, "oauth2"}
            .add_route(
                Route{"jwks"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_jwks_endpoint(req, res, std::move(send));
                    })
                    .summary("Hyperswitch jwks_endpoint")
            )
            .add_route(
                Route{"token"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_oidc_token(req, res, std::move(send));
                    })
                    .summary("Hyperswitch oidc_token")
            );
        Router{router, "oidc"}
            .add_route(
                Route{"authorize"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_oidc_authorize(req, res, std::move(send));
                    })
                    .summary("Hyperswitch oidc_authorize")
            );
    }

    explicit OidcRouteHandlers(app::context::AppContext& context) noexcept
        : m_context{&context}
    {
    }

private:
    static constexpr std::string_view REDIS_AUTH_CODE_PREFIX{"OIDC_AUTH_CODE_"};
    static constexpr std::int64_t AUTH_CODE_TTL_IN_SECS{60 * 5};
    static constexpr std::uint64_t ID_TOKEN_TTL_IN_SECS{60 * 15};
    static constexpr std::uint64_t ACCESS_TOKEN_TTL_IN_SECS{60 * 60};
    static constexpr std::string_view TOKEN_TYPE_BEARER{"Bearer"};

    /** @brief Deterministic local stand-in for conf.oidc.key (real RSA PEM). */
    static constexpr std::string_view DEFAULT_RSA_PRIVATE_KEY{
        "-----BEGIN PRIVATE KEY-----\n"
        "MIICdgIBADANBgkqhkiG9w0BAQEFAASCAmAwggJcAgEAAoGBAOM07IIREH/ZwxyC\n"
        "f4hGvyWelOZZVnTR97coBM0xUrS1X7iu394iUUSlyRPtcjQIJCbMv/UiYT0szvU1\n"
        "rq8Z0urKhki35H2bPwOgCBRJK4STOgDxqHLbgYOkJAPB2rXYhqtLftHk5ZLGspCM\n"
        "9ZYUkWJa/JmSVrwxH3t3xFfPu/2FAgMBAAECgYBagkuw6JFVDO9vnQCmzd/5YXnZ\n"
        "ZT9OnBH9EWEQSJS0zgfKr0cY2zzahtxaJt22ooOcPTY+tapcz0vGXNMwh63//+wg\n"
        "8qIj5jbgBovrBAJkYyvawrDKHfl/UFqh+mOobSeDAp4QyOzwwjv9PbUDJ4KtDMG7\n"
        "vMjMaxAw8Tzj3RlVJQJBAPXrFmH24OY1HwH5NqJFYKFq0xFoqya042yV9gvdwZeT\n"
        "8liaQCy5+LvBbWcNngMfPwU3WKr7ho3shWmNYMX62mMCQQDshXVcUhjERO5wTgT7\n"
        "1TOv6XWcH3PH3ZZJAn6BvI+tbif/iqVmVXS63sk2VpHqf7asOHeB1ONAIolQl4CI\n"
        "/hj3AkEAyX5/7nDcQYORDorl6vuxe1EN9Buw8JwSI+yTl5FxiAMe9N4eawdN+Hk6\n"
        "bM/JgoFdn6aDrkU9MJh1j9DZVB6ARQJAYmuajYMgCTQB4Dum+UAblPzZ0tCHtkhv\n"
        "zAu8prHXO7zV6hFAiFfRDh28rt7RMzpXwkxw9lanBb4ZU67nbpoFuwJAG4Rw37/M\n"
        "CaziuUyzO4VvT1dXLKVmQoWp3nFozql1LBFdCz+VuQT34/ajee7UDzy3vZleSvhm\n"
        "9zqm2OGN4j8Q2g==\n"
        "-----END PRIVATE KEY-----"
    };

    struct ConfiguredKey
    {
        std::string kid{};
        std::string private_key{};
    };

    struct ConfiguredClient
    {
        std::string client_id{};
        std::string client_secret{};
        std::string redirect_uri{};
    };

    [[nodiscard]] std::optional<core::error::AppError> authorize_operation(
        interfaces::io::IRequest& req,
        std::string_view operation
    ) const
    {
        const auto rule = m_context->auth_rules().rule_for(operation);
        return m_context->authorizer().authorize_route(app::auth::AuthContextBuilder{}(req), rule);
    }

    [[nodiscard]] std::string tenant_base_url() const
    {
        return m_context->config_store()
            .get_value("tenant", "base_url")
            .value_or("https://sandbox.hyperswitch.io");
    }

    [[nodiscard]] std::string control_center_url() const
    {
        return m_context->config_store()
            .get_value("user", "base_url")
            .value_or("https://control-center.hyperswitch.io");
    }

    void ensure_oidc_config() const
    {
        if (!m_context->config_store().get("oidc_key", "oidc-signing-key")) {
            m_context->config_store().put(
                "oidc_key", "oidc-signing-key", std::string{DEFAULT_RSA_PRIVATE_KEY}
            );
        }
        if (!m_context->config_store().get("oidc_client", "oidc-client")) {
            m_context->config_store().put(
                "oidc_client", "oidc-client",
                R"({"client_id":"oidc-client","client_secret":"oidc-secret","redirect_uri":"https://sandbox.hyperswitch.io/oidc/callback"})"
            );
        }
        // The shared KeyStore is the secret resolver for the local JwtSigner
        // stand-in. Seed it with the configured RSA key material so the token
        // endpoint signs with the configured key set the JWKS exposes.
        if (!m_context->key_store().get("oidc-jwt", "oidc-signing-key")) {
            m_context->key_store().put(
                "oidc-jwt", "oidc-signing-key", "RS256",
                app::crypto::SecretString{signing_key_material()}
            );
        }
    }

    /** @brief Upstream get_signing_key(): deterministic first kid of sorted key ids. */
    [[nodiscard]] std::string signing_key_material() const
    {
        auto keys = configured_oidc_keys();
        if (keys.empty()) {
            return std::string{DEFAULT_RSA_PRIVATE_KEY};
        }
        std::sort(keys.begin(), keys.end(), [](const ConfiguredKey& left, const ConfiguredKey& right)
        {
            return left.kid < right.kid;
        });
        return keys.front().private_key;
    }

    /** @brief Local enumeration of conf.oidc.key (shared KeyStore has no enumeration). */
    [[nodiscard]] std::vector<ConfiguredKey> configured_oidc_keys() const
    {
        std::vector<ConfiguredKey> keys{};
        for (const auto& record : m_context->config_store().list("oidc_key")) {
            ConfiguredKey key{};
            key.kid = record.get_config_key();
            const auto& value = record.get_value();
            if (const auto kid = detail::json_string_field(value, "kid")) {
                key.kid = *kid;
            }
            if (const auto private_key = detail::json_string_field(value, "private_key")) {
                key.private_key = *private_key;
            } else {
                key.private_key = value;
            }
            if (!key.private_key.empty()) {
                keys.push_back(std::move(key));
            }
        }
        return keys;
    }

    /** @brief Local enumeration of conf.oidc.client. */
    [[nodiscard]] std::optional<ConfiguredClient> configured_oidc_client(
        std::string_view client_id
    ) const
    {
        for (const auto& record : m_context->config_store().list("oidc_client")) {
            const auto& value = record.get_value();
            const auto id = detail::json_string_field(value, "client_id")
                .value_or(record.get_config_key());
            if (id != client_id) {
                continue;
            }
            ConfiguredClient client{};
            client.client_id = id;
            client.client_secret = detail::json_string_field(value, "client_secret")
                .value_or("");
            client.redirect_uri = detail::json_string_field(value, "redirect_uri")
                .value_or(value);
            return client;
        }
        return std::nullopt;
    }

    /** @brief web::Query<OidcAuthorizeQuery>: query only, space-separated scope. */
    [[nodiscard]] std::optional<app::dto::oidc::OidcAuthorizeQuery> parse_authorize_query(
        interfaces::io::IRequest& req
    ) const
    {
        using Input = app::dto::oidc::OidcAuthorizeQuery;
        const auto query = app::http::RequestQuery{}(req);
        if (query.get("response_type").value_or("") != "code") {
            return std::nullopt;
        }
        const auto client_id = query.get("client_id");
        const auto redirect_uri = query.get("redirect_uri");
        const auto state = query.get("state");
        const auto nonce = query.get("nonce");
        const auto scope_value = query.get("scope");
        if (!client_id || !redirect_uri || !state || !nonce || !scope_value) {
            return std::nullopt;
        }

        std::vector<app::dto::oidc::Scope> scopes{};
        std::size_t begin{0};
        while (begin < scope_value->size()) {
            while (begin < scope_value->size() &&
                   std::isspace(static_cast<unsigned char>((*scope_value)[begin]))) {
                ++begin;
            }
            if (begin >= scope_value->size()) {
                break;
            }
            std::size_t end = begin;
            while (end < scope_value->size() &&
                   !std::isspace(static_cast<unsigned char>((*scope_value)[end]))) {
                ++end;
            }
            const std::string_view token{scope_value->data() + begin, end - begin};
            if (token == "openid") {
                scopes.push_back(app::dto::oidc::Scope::Openid);
            } else if (token == "email") {
                scopes.push_back(app::dto::oidc::Scope::Email);
            } else if (token == "profile") {
                scopes.push_back(app::dto::oidc::Scope::Profile);
            } else {
                return std::nullopt;
            }
            begin = end;
        }

        Input payload{};
        payload.set_response_type(app::dto::oidc::ResponseType::Code);
        payload.set_client_id(*client_id);
        payload.set_redirect_uri(*redirect_uri);
        payload.set_scope(std::move(scopes));
        payload.set_state(*state);
        payload.set_nonce(*nonce);
        return payload;
    }

    /** @brief web::Form<OidcTokenRequest>: form-encoded only. */
    [[nodiscard]] std::optional<app::dto::oidc::OidcTokenRequest> parse_token_request(
        interfaces::io::IRequest& req
    ) const
    {
        const auto content_type = app::http::HeaderReader{}.get(req, "content-type");
        if (content_type.find("x-www-form-urlencoded") == std::string::npos) {
            return std::nullopt;
        }
        const auto body = req.body_as_string();
        const auto form = app::http::QueryParams{body};
        if (form.get("grant_type").value_or("") != "authorization_code") {
            return std::nullopt;
        }
        const auto code = form.get("code");
        const auto redirect_uri = form.get("redirect_uri");
        if (!code || !redirect_uri) {
            return std::nullopt;
        }
        app::dto::oidc::OidcTokenRequest request{};
        request.set_grant_type(app::dto::oidc::GrantType::AuthorizationCode);
        request.set_code(*code);
        request.set_redirect_uri(*redirect_uri);
        return request;
    }

    /** @brief HTTP Basic credentials; standard base64, "client_id:client_secret". */
    [[nodiscard]] std::optional<std::pair<std::string, std::string>> parse_basic_auth(
        interfaces::io::IRequest& req
    ) const
    {
        const auto header = app::http::HeaderReader{}.get(req, "authorization");
        constexpr std::string_view prefix{"Basic "};
        if (header.size() <= prefix.size()) {
            return std::nullopt;
        }
        const auto lower = [](char ch)
        {
            return ch >= 'A' && ch <= 'Z' ? static_cast<char>(ch + ('a' - 'A')) : ch;
        };
        if (!std::equal(prefix.begin(), prefix.end(), header.begin(),
                        [&lower](char expected, char actual)
                        {
                            return lower(expected) == lower(actual);
                        })) {
            return std::nullopt;
        }
        const auto decoded = detail::decode_standard_base64(header.substr(prefix.size()));
        if (!decoded) {
            return std::nullopt;
        }
        const std::string credentials{decoded->begin(), decoded->end()};
        const auto colon = credentials.find(':');
        if (colon == std::string::npos) {
            return std::nullopt;
        }
        return std::make_pair(credentials.substr(0, colon), credentials.substr(colon + 1));
    }

    [[nodiscard]] std::string authenticated_user_id(interfaces::io::IRequest& req) const
    {
        const auto auth_context = app::auth::AuthContextBuilder{}(req);
        if (const auto sub = auth_context.claim("sub"); sub && !sub->empty()) {
            return std::string{*sub};
        }
        const auto read = [&req](std::string_view name)
        {
            return app::http::HeaderReader{}.get(req, name);
        };
        for (const auto name : {"x-user-id", "x-user", "x-user-name"}) {
            if (const auto value = read(name); !value.empty()) {
                return value;
            }
        }
        return std::string{app::auth::TokenParser{}(req).value()};
    }

    /** @brief Local stand-in for sign_oidc_token with a configured RSA key id. */
    [[nodiscard]] app::crypto::JwtSigner token_signer() const
    {
        auto signer = app::crypto::JwtSigner{
            m_context->secret_resolver(), "oidc-signing-key", "oidc-jwt"
        };
        signer.set_algorithm("RS256");
        return signer;
    }

    static void append_json_string(std::string& out, std::string_view value)
    {
        out += serde::Ser::encode_json(std::string{value});
    }

    static void append_json_bool(std::string& out, bool value)
    {
        out += value ? "true" : "false";
    }

    /** @brief IdTokenClaims field order from types/domain/user/oidc.rs. */
    [[nodiscard]] std::string build_id_claims(
        std::string_view issuer,
        const StoredAuthCode& code,
        std::uint64_t issued_at,
        std::uint64_t expires_at
    ) const
    {
        std::string text{"{"};
        text += R"("iss":)"; append_json_string(text, issuer);
        text += R"(,"sub":)"; append_json_string(text, code.get_sub());
        text += R"(,"aud":)"; append_json_string(text, code.get_client_id());
        text += R"(,"iat":)" + std::to_string(issued_at);
        text += R"(,"exp":)" + std::to_string(expires_at);
        if (has_scope(code.get_scope(), app::dto::oidc::Scope::Email)) {
            text += R"(,"email":)"; append_json_string(text, code.get_email());
            text += R"(,"email_verified":)"; append_json_bool(text, code.get_is_verified());
        }
        if (has_scope(code.get_scope(), app::dto::oidc::Scope::Profile)) {
            text += R"(,"preferred_username":)"; append_json_string(text, code.get_email());
        }
        text += R"(,"nonce":)"; append_json_string(text, code.get_nonce());
        text.push_back('}');
        return text;
    }

    /** @brief AccessTokenClaims field order from types/domain/user/oidc.rs. */
    [[nodiscard]] std::string build_access_claims(
        std::string_view issuer,
        const StoredAuthCode& code,
        std::uint64_t issued_at,
        std::uint64_t expires_at
    ) const
    {
        std::string text{"{"};
        text += R"("iss":)"; append_json_string(text, issuer);
        text += R"(,"sub":)"; append_json_string(text, code.get_sub());
        text += R"(,"aud":)"; append_json_string(text, code.get_client_id());
        text += R"(,"iat":)" + std::to_string(issued_at);
        text += R"(,"exp":)" + std::to_string(expires_at);
        text += R"(,"email":)"; append_json_string(text, code.get_email());
        if (has_scope(code.get_scope(), app::dto::oidc::Scope::Profile)) {
            text += R"(,"preferred_username":)"; append_json_string(text, code.get_email());
        }
        text += R"(,"scope":[)";
        bool first{true};
        for (const auto scope : code.get_scope()) {
            if (!first) {
                text.push_back(',');
            }
            first = false;
            switch (scope) {
                case app::dto::oidc::Scope::Openid: text += R"("openid")"; break;
                case app::dto::oidc::Scope::Email: text += R"("email")"; break;
                case app::dto::oidc::Scope::Profile: text += R"("profile")"; break;
            }
        }
        text.push_back(']');
        text.push_back('}');
        return text;
    }

    [[nodiscard]] bool has_scope(
        const std::vector<app::dto::oidc::Scope>& scopes,
        app::dto::oidc::Scope wanted
    ) const
    {
        return std::find(scopes.begin(), scopes.end(), wanted) != scopes.end();
    }

    /** @brief ApplicationResponse::JsonForRedirection wire shape (api_models::payments v1). */
    void send_redirection_response(
        interfaces::io::IResponse& res,
        std::function<void()> send,
        std::string_view redirect_url
    ) const
    {
        res.set_status(interfaces::io::types::Status::FOUND);
        res.set_header(interfaces::io::types::Token::LOCATION, redirect_url);
        const auto text = std::string{R"({"return_url":"","params":[],"return_url_with_query_params":)"}
            + serde::Ser::encode_json(std::string{redirect_url})
            + R"(,"http_method":"","headers":[]})";
        auto bytes = serde::Ser::serialize_raw("application/json", text);
        res.set_body(std::move(bytes));
        send();
    }

    void send_oidc_error(
        interfaces::io::IResponse& res,
        std::function<void()> send,
        detail::OidcErrorKind kind
    ) const
    {
        const auto shape = detail::oidc_error(kind);
        res.set_status(shape.status);
        auto bytes = serde::Ser::serialize_raw("application/json", shape.body);
        res.set_body(std::move(bytes));
        send();
    }

    void send_error(
        interfaces::io::IResponse& res,
        std::function<void()> send,
        core::error::AppError error
    ) const
    {
        auto text = serde::Ser::encode_json(error);
        auto bytes = serde::Ser::serialize_raw("application/json", text);
        res.set_status(core::error::AppErrorStatus{}(error.get_kind()));
        res.set_body(std::move(bytes));
        send();
    }

    void send_json(
        interfaces::io::IResponse& res,
        std::function<void()> send,
        std::string_view text
    ) const
    {
        auto bytes = serde::Ser::serialize_raw("application/json", text);
        res.set_status(interfaces::io::types::Status::OK);
        res.set_body(std::move(bytes));
        send();
    }

    app::context::AppContext* m_context{nullptr};
};

} // namespace app::handlers::oidc
