export module app_auth:oidc_runtime_common;

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
import app_context_core;
import app_http;
import app_crypto;
import app_metrics;
import app_context_core;
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
    void set_sub(std::string value)
    {
        sub = std::move(value);
    }

    [[nodiscard]] const std::string& get_sub() const noexcept
    {
        return sub;
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

    void set_scope(std::vector<::app::dto::oidc::Scope> value)
    {
        scope = std::move(value);
    }

    [[nodiscard]] const std::vector<::app::dto::oidc::Scope>& get_scope() const noexcept
    {
        return scope;
    }

    void set_nonce(std::string value)
    {
        nonce = std::move(value);
    }

    [[nodiscard]] const std::string& get_nonce() const noexcept
    {
        return nonce;
    }

    void set_email(std::string value)
    {
        email = std::move(value);
    }

    [[nodiscard]] const std::string& get_email() const noexcept
    {
        return email;
    }

    void set_is_verified(bool value)
    {
        is_verified = value;
    }

    [[nodiscard]] bool get_is_verified() const noexcept
    {
        return is_verified;
    }

    std::string sub{};
    std::string client_id{};
    std::string redirect_uri{};
    std::vector<::app::dto::oidc::Scope> scope{};
    std::string nonce{};
    std::string email{};
    bool is_verified{};
};

// Moved verbatim from the facade's private constants.
constexpr std::string_view REDIS_AUTH_CODE_PREFIX{"OIDC_AUTH_CODE_"};
constexpr std::int64_t AUTH_CODE_TTL_IN_SECS{60 * 5};
constexpr std::uint64_t ID_TOKEN_TTL_IN_SECS{60 * 15};
constexpr std::uint64_t ACCESS_TOKEN_TTL_IN_SECS{60 * 60};
constexpr std::string_view TOKEN_TYPE_BEARER{"Bearer"};

/** @brief Deterministic local stand-in for conf.oidc.key (real RSA PEM). */
constexpr std::string_view DEFAULT_RSA_PRIVATE_KEY{
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

namespace oidc_support {

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
                    case 'n':
                        out.push_back('\n');
                        break;
                    case 'r':
                        out.push_back('\r');
                        break;
                    case 't':
                        out.push_back('\t');
                        break;
                    default:
                        out.push_back(escaped);
                        break;
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

    // ── standard base64 (PEM / HTTP Basic), distinct from ::app::crypto::Base64Url ─

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
        for (const char ch: input) {
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

    inline std::optional<std::string>
    der_integer_bytes(std::string_view buffer, std::size_t pos, std::size_t& end)
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
     * without padding using ::app::crypto::Base64Url.
     */
    inline std::optional<std::pair<std::string, std::string>>
    extract_rsa_public_key_components(std::string_view private_key_pem)
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
        for (const char ch: private_key_pem.substr(body_begin + 1, footer - (body_begin + 1))) {
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
        const ::app::crypto::Base64Url codec{};
        return std::make_pair(codec.encode(*modulus), codec.encode(*exponent));
    }

    // ── URL helpers ─────────────────────────────────────────────────────────────

    inline std::string form_url_encode(std::string_view value)
    {
        constexpr std::string_view hex{"0123456789ABCDEF"};
        std::string out{};
        for (const char raw: value) {
            const auto ch = static_cast<unsigned char>(raw);
            if ((ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z') || (ch >= '0' && ch <= '9') ||
                ch == '-' || ch == '_' || ch == '.' || ch == '~') {
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
            std::transform(
                scheme.begin(),
                scheme.end(),
                scheme.begin(),
                [](unsigned char ch)
                {
                    return static_cast<char>(std::tolower(ch));
                }
            );
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
            std::transform(
                host.begin(),
                host.end(),
                host.begin(),
                [](unsigned char ch)
                {
                    return static_cast<char>(std::tolower(ch));
                }
            );
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
        std::string_view redirect_uri,
        std::string_view code,
        std::string_view state
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
            "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789"
        };
        std::string out{};
        out.reserve(length);
        std::uint64_t state{14'695'981'039'346'656'037ULL};
        for (const char ch: seed) {
            state ^= static_cast<unsigned char>(ch);
            state *= 1'099'511'628'211ULL;
        }
        for (std::size_t i{0}; i < length; ++i) {
            state ^= state << 13;
            state ^= state >> 7;
            state ^= state << 17;
            out.push_back(alphabet[static_cast<std::size_t>(state % alphabet.size())]);
        }
        return out;
    }

} // namespace oidc_support

/**
 * @brief Shared config/token/claims support for the oidc sub-runtimes.
 */
class OidcRuntimeSupport
{
public:
    explicit OidcRuntimeSupport(::app::context::AppContext& context) noexcept :
        m_context{context}
    {
    }

    [[nodiscard]] std::string tenant_base_url() const
    {
        return m_context.config_store()
            .get_value("tenant", "base_url")
            .value_or("https://sandbox.hyperswitch.io");
    }

    [[nodiscard]] std::string control_center_url() const
    {
        return m_context.config_store()
            .get_value("user", "base_url")
            .value_or("https://control-center.hyperswitch.io");
    }

    void ensure_oidc_config() const
    {
        if (!m_context.config_store().get("oidc_key", "oidc-signing-key")) {
            m_context.config_store()
                .put("oidc_key", "oidc-signing-key", std::string{DEFAULT_RSA_PRIVATE_KEY});
        }
        if (!m_context.config_store().get("oidc_client", "oidc-client")) {
            m_context.config_store().put(
                "oidc_client",
                "oidc-client",
                R"({"client_id":"oidc-client","client_secret":"oidc-secret","redirect_uri":"https://sandbox.hyperswitch.io/oidc/callback"})"
            );
        }
        // The shared KeyStore is the secret resolver for the local JwtSigner
        // stand-in. Seed it with the configured RSA key material so the token
        // endpoint signs with the configured key set the JWKS exposes.
        if (!m_context.key_store().get("oidc-jwt", "oidc-signing-key")) {
            m_context.key_store().put(
                "oidc-jwt",
                "oidc-signing-key",
                "RS256",
                ::app::crypto::SecretString{signing_key_material()}
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
        std::sort(
            keys.begin(),
            keys.end(),
            [](const ConfiguredKey& left, const ConfiguredKey& right)
            {
                return left.kid < right.kid;
            }
        );
        return keys.front().private_key;
    }

    /** @brief Local enumeration of conf.oidc.key (shared KeyStore has no enumeration). */
    [[nodiscard]] std::vector<ConfiguredKey> configured_oidc_keys() const
    {
        std::vector<ConfiguredKey> keys{};
        for (const auto& record: m_context.config_store().list("oidc_key")) {
            ConfiguredKey key{};
            key.kid = record.get_config_key();
            const auto& value = record.get_value();
            if (const auto kid = oidc_support::json_string_field(value, "kid")) {
                key.kid = *kid;
            }
            if (const auto private_key = oidc_support::json_string_field(value, "private_key")) {
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
    [[nodiscard]] std::optional<ConfiguredClient>
    configured_oidc_client(std::string_view client_id) const
    {
        for (const auto& record: m_context.config_store().list("oidc_client")) {
            const auto& value = record.get_value();
            const auto id = oidc_support::json_string_field(value, "client_id")
                                .value_or(record.get_config_key());
            if (id != client_id) {
                continue;
            }
            ConfiguredClient client{};
            client.client_id = id;
            client.client_secret =
                oidc_support::json_string_field(value, "client_secret").value_or("");
            client.redirect_uri =
                oidc_support::json_string_field(value, "redirect_uri").value_or(value);
            return client;
        }
        return std::nullopt;
    }

    /** @brief Local stand-in for sign_oidc_token with a configured RSA key id. */
    [[nodiscard]] ::app::crypto::JwtSigner token_signer() const
    {
        auto signer =
            ::app::crypto::JwtSigner{m_context.secret_resolver(), "oidc-signing-key", "oidc-jwt"};
        signer.set_algorithm("RS256");
        return signer;
    }

    void append_json_string(std::string& out, std::string_view value) const
    {
        out += serde::Ser::encode_json(std::string{value});
    }

    void append_json_bool(std::string& out, bool value) const
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
        text += R"("iss":)";
        append_json_string(text, issuer);
        text += R"(,"sub":)";
        append_json_string(text, code.get_sub());
        text += R"(,"aud":)";
        append_json_string(text, code.get_client_id());
        text += R"(,"iat":)" + std::to_string(issued_at);
        text += R"(,"exp":)" + std::to_string(expires_at);
        if (has_scope(code.get_scope(), ::app::dto::oidc::Scope::Email)) {
            text += R"(,"email":)";
            append_json_string(text, code.get_email());
            text += R"(,"email_verified":)";
            append_json_bool(text, code.get_is_verified());
        }
        if (has_scope(code.get_scope(), ::app::dto::oidc::Scope::Profile)) {
            text += R"(,"preferred_username":)";
            append_json_string(text, code.get_email());
        }
        text += R"(,"nonce":)";
        append_json_string(text, code.get_nonce());
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
        text += R"("iss":)";
        append_json_string(text, issuer);
        text += R"(,"sub":)";
        append_json_string(text, code.get_sub());
        text += R"(,"aud":)";
        append_json_string(text, code.get_client_id());
        text += R"(,"iat":)" + std::to_string(issued_at);
        text += R"(,"exp":)" + std::to_string(expires_at);
        text += R"(,"email":)";
        append_json_string(text, code.get_email());
        if (has_scope(code.get_scope(), ::app::dto::oidc::Scope::Profile)) {
            text += R"(,"preferred_username":)";
            append_json_string(text, code.get_email());
        }
        text += R"(,"scope":[)";
        bool first{true};
        for (const auto scope: code.get_scope()) {
            if (!first) {
                text.push_back(',');
            }
            first = false;
            switch (scope) {
                case ::app::dto::oidc::Scope::Openid:
                    text += R"("openid")";
                    break;
                case ::app::dto::oidc::Scope::Email:
                    text += R"("email")";
                    break;
                case ::app::dto::oidc::Scope::Profile:
                    text += R"("profile")";
                    break;
            }
        }
        text.push_back(']');
        text.push_back('}');
        return text;
    }

    [[nodiscard]] bool has_scope(
        const std::vector<::app::dto::oidc::Scope>& scopes,
        ::app::dto::oidc::Scope wanted
    ) const
    {
        return std::find(scopes.begin(), scopes.end(), wanted) != scopes.end();
    }

private:
    ::app::context::AppContext& m_context;
};

} // namespace app::handlers::oidc

template<>
struct serde::Serializable<::app::handlers::oidc::StoredAuthCode>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "sub",
                &::app::handlers::oidc::StoredAuthCode::get_sub,
                &::app::handlers::oidc::StoredAuthCode::set_sub,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "client_id",
                &::app::handlers::oidc::StoredAuthCode::get_client_id,
                &::app::handlers::oidc::StoredAuthCode::set_client_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "redirect_uri",
                &::app::handlers::oidc::StoredAuthCode::get_redirect_uri,
                &::app::handlers::oidc::StoredAuthCode::set_redirect_uri,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "scope",
                &::app::handlers::oidc::StoredAuthCode::get_scope,
                &::app::handlers::oidc::StoredAuthCode::set_scope,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "nonce",
                &::app::handlers::oidc::StoredAuthCode::get_nonce,
                &::app::handlers::oidc::StoredAuthCode::set_nonce,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "email",
                &::app::handlers::oidc::StoredAuthCode::get_email,
                &::app::handlers::oidc::StoredAuthCode::set_email,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "is_verified",
                &::app::handlers::oidc::StoredAuthCode::get_is_verified,
                &::app::handlers::oidc::StoredAuthCode::set_is_verified,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "oidc_auth_code_data";
    }
};
