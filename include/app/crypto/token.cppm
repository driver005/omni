module;
export module app_crypto:token;

import std;
import :hash;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace app::crypto {

/** @brief Manual, padding-free base64url codec. */
class Base64Url
{
public:
    [[nodiscard]] std::string encode(std::string_view input) const
    {
        constexpr std::string_view alphabet{
            "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_"
        };

        std::string out{};
        out.reserve(((input.size() + 2) / 3) * 4);
        std::size_t i{0};
        while (i + 3 <= input.size()) {
            const std::uint32_t word{
                (static_cast<std::uint32_t>(byte_at(input, i)) << 16) |
                (static_cast<std::uint32_t>(byte_at(input, i + 1)) << 8) |
                static_cast<std::uint32_t>(byte_at(input, i + 2))
            };
            out.push_back(alphabet[(word >> 18) & 0x3FU]);
            out.push_back(alphabet[(word >> 12) & 0x3FU]);
            out.push_back(alphabet[(word >> 6) & 0x3FU]);
            out.push_back(alphabet[word & 0x3FU]);
            i += 3;
        }
        const std::size_t remaining = input.size() - i;
        if (remaining == 1) {
            const std::uint32_t word{static_cast<std::uint32_t>(byte_at(input, i)) << 16};
            out.push_back(alphabet[(word >> 18) & 0x3FU]);
            out.push_back(alphabet[(word >> 12) & 0x3FU]);
        } else if (remaining == 2) {
            const std::uint32_t word{
                (static_cast<std::uint32_t>(byte_at(input, i)) << 16) |
                (static_cast<std::uint32_t>(byte_at(input, i + 1)) << 8)
            };
            out.push_back(alphabet[(word >> 18) & 0x3FU]);
            out.push_back(alphabet[(word >> 12) & 0x3FU]);
            out.push_back(alphabet[(word >> 6) & 0x3FU]);
        }
        return out;
    }

    [[nodiscard]] std::string decode(std::string_view input) const
    {
        std::string out{};
        out.reserve((input.size() / 4) * 3);
        std::size_t i{0};
        while (i + 4 <= input.size()) {
            const int a = index_of(input[i]);
            const int b = index_of(input[i + 1]);
            const int c = index_of(input[i + 2]);
            const int d = index_of(input[i + 3]);
            if (a < 0 || b < 0 || c < 0 || d < 0) {
                return std::string{};
            }
            const std::uint32_t word{
                (static_cast<std::uint32_t>(a) << 18) | (static_cast<std::uint32_t>(b) << 12) |
                (static_cast<std::uint32_t>(c) << 6) | static_cast<std::uint32_t>(d)
            };
            push_bytes(out, word, 3);
            i += 4;
        }
        const std::size_t remaining = input.size() - i;
        if (remaining == 2) {
            const int a = index_of(input[i]);
            const int b = index_of(input[i + 1]);
            if (a < 0 || b < 0) {
                return std::string{};
            }
            const std::uint32_t word{
                (static_cast<std::uint32_t>(a) << 18) | (static_cast<std::uint32_t>(b) << 12)
            };
            push_bytes(out, word, 1);
        } else if (remaining == 3) {
            const int a = index_of(input[i]);
            const int b = index_of(input[i + 1]);
            const int c = index_of(input[i + 2]);
            if (a < 0 || b < 0 || c < 0) {
                return std::string{};
            }
            const std::uint32_t word{
                (static_cast<std::uint32_t>(a) << 18) | (static_cast<std::uint32_t>(b) << 12) |
                (static_cast<std::uint32_t>(c) << 6)
            };
            push_bytes(out, word, 2);
        } else if (remaining == 1) {
            return std::string{};
        }
        return out;
    }

private:
    [[nodiscard]] unsigned char byte_at(std::string_view input, std::size_t index) const
    {
        return static_cast<unsigned char>(input[index]);
    }

    [[nodiscard]] int index_of(char c) const
    {
        constexpr std::string_view alphabet{
            "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_"
        };
        const std::size_t pos = alphabet.find(c);
        return pos == std::string_view::npos ? -1 : static_cast<int>(pos);
    }

    void push_bytes(std::string& out, std::uint32_t word, std::size_t count) const
    {
        if (count >= 1) {
            out.push_back(static_cast<char>((word >> 16) & 0xFFU));
        }
        if (count >= 2) {
            out.push_back(static_cast<char>((word >> 8) & 0xFFU));
        }
        if (count >= 3) {
            out.push_back(static_cast<char>(word & 0xFFU));
        }
    }
};

/** @brief Resolves a signing secret by purpose and key id.
 *
 *  app_keystore::KeyStoreSecretResolver implements this for repository-backed
 *  keys. The interface lives here so app_crypto never depends on app_keystore.
 */
class ISecretResolver
{
public:
    virtual ~ISecretResolver() = default;

    [[nodiscard]] virtual std::optional<std::string>
    resolve(std::string_view purpose, std::string_view key_id) = 0;
};

/** @brief Typed JWT claim set used to build a signed payload. */
class JwtClaims
{
public:
    JwtClaims& issuer(std::string value)
    {
        m_issuer = std::move(value);
        return *this;
    }

    JwtClaims& audience(std::string value)
    {
        m_audience = std::move(value);
        return *this;
    }

    JwtClaims& subject(std::string value)
    {
        m_subject = std::move(value);
        return *this;
    }

    JwtClaims& expires_at(std::int64_t value)
    {
        m_expires_at = value;
        m_has_expiry = true;
        return *this;
    }

    JwtClaims& issued_at(std::int64_t value)
    {
        m_issued_at = value;
        m_has_issued_at = true;
        return *this;
    }

    JwtClaims& key_id(std::string value)
    {
        m_key_id = std::move(value);
        return *this;
    }

    [[nodiscard]] const std::string& get_issuer() const noexcept
    {
        return m_issuer;
    }

    [[nodiscard]] const std::string& get_audience() const noexcept
    {
        return m_audience;
    }

    [[nodiscard]] const std::string& get_subject() const noexcept
    {
        return m_subject;
    }

    [[nodiscard]] std::int64_t get_expires_at() const noexcept
    {
        return m_expires_at;
    }

    [[nodiscard]] std::int64_t get_issued_at() const noexcept
    {
        return m_issued_at;
    }

    [[nodiscard]] const std::string& get_key_id() const noexcept
    {
        return m_key_id;
    }

    [[nodiscard]] bool has_expiry() const noexcept
    {
        return m_has_expiry;
    }

    /** @brief Serializes the populated claims into the signed JSON payload. */
    [[nodiscard]] std::string to_json() const
    {
        std::string out{"{"};
        bool first{true};
        if (!m_issuer.empty()) {
            append_string(out, first, "iss", m_issuer);
        }
        if (!m_audience.empty()) {
            append_string(out, first, "aud", m_audience);
        }
        if (!m_subject.empty()) {
            append_string(out, first, "sub", m_subject);
        }
        if (m_has_expiry) {
            append_number(out, first, "exp", m_expires_at);
        }
        if (m_has_issued_at) {
            append_number(out, first, "iat", m_issued_at);
        }
        if (!m_key_id.empty()) {
            append_string(out, first, "kid", m_key_id);
        }
        out.push_back('}');
        return out;
    }

private:
    void append_string(
        std::string& out,
        bool& first,
        std::string_view key,
        std::string_view value
    ) const
    {
        if (!first) {
            out.push_back(',');
        }
        first = false;
        out.push_back('"');
        out.append(key);
        out.append("\":\"");
        for (const char c: value) {
            if (c == '"' || c == '\\') {
                out.push_back('\\');
            }
            out.push_back(c);
        }
        out.push_back('"');
    }

    void append_number(
        std::string& out,
        bool& first,
        std::string_view key,
        std::int64_t value
    ) const
    {
        if (!first) {
            out.push_back(',');
        }
        first = false;
        out.push_back('"');
        out.append(key);
        out.append("\":");
        out.append(std::to_string(value));
    }

    std::string m_issuer{};
    std::string m_audience{};
    std::string m_subject{};
    std::string m_key_id{};
    std::int64_t m_expires_at{0};
    std::int64_t m_issued_at{0};
    bool m_has_expiry{false};
    bool m_has_issued_at{false};
};

/** @brief Expected claims for strict verification. Empty fields are not checked. */
class JwtVerificationOptions
{
public:
    JwtVerificationOptions& expected_issuer(std::string value)
    {
        m_expected_issuer = std::move(value);
        return *this;
    }

    JwtVerificationOptions& expected_audience(std::string value)
    {
        m_expected_audience = std::move(value);
        return *this;
    }

    JwtVerificationOptions& expected_subject(std::string value)
    {
        m_expected_subject = std::move(value);
        return *this;
    }

    JwtVerificationOptions& now_seconds(std::int64_t value)
    {
        m_now_seconds = value;
        return *this;
    }

    JwtVerificationOptions& require_expiry(bool value)
    {
        m_require_expiry = value;
        return *this;
    }

    [[nodiscard]] const std::string& get_expected_issuer() const noexcept
    {
        return m_expected_issuer;
    }

    [[nodiscard]] const std::string& get_expected_audience() const noexcept
    {
        return m_expected_audience;
    }

    [[nodiscard]] const std::string& get_expected_subject() const noexcept
    {
        return m_expected_subject;
    }

    [[nodiscard]] std::int64_t get_now_seconds() const noexcept
    {
        return m_now_seconds;
    }

    [[nodiscard]] bool get_require_expiry() const noexcept
    {
        return m_require_expiry;
    }

private:
    std::string m_expected_issuer{};
    std::string m_expected_audience{};
    std::string m_expected_subject{};
    std::int64_t m_now_seconds{0};
    bool m_require_expiry{true};
};

/** @brief Outcome of strict verification; reason is a token, not an error string. */
class JwtVerifyResult
{
public:
    JwtVerifyResult() = default;

    JwtVerifyResult(bool ok, std::string reason) :
        m_ok{ok},
        m_reason{std::move(reason)}
    {
    }

    [[nodiscard]] bool ok() const noexcept
    {
        return m_ok;
    }

    [[nodiscard]] const std::string& reason() const noexcept
    {
        return m_reason;
    }

private:
    bool m_ok{false};
    std::string m_reason{};
};

/** @brief Stand-in JWT signer: header.payload.signature with a keyed digest.
 *
 *  The signature is a domain-separated keyed FNV-1a digest (see KeyedHasher)
 *  encoded as base64url, NOT a real HMAC-SHA256 or RSA signature. It provides
 *  stable shape and tamper detection for parity tests only. An algorithm such
 *  as RS256 can be selected for wire-shape parity; no RSA key operation occurs.
 *  Secrets come either from a legacy inline string or from an ISecretResolver
 *  (normally backed by app_keystore::KeyStore).
 */
class JwtSigner
{
public:
    /** @brief Legacy constructor kept for compatibility with existing callers. */
    explicit JwtSigner(std::string secret) :
        m_secret{std::move(secret)}
    {
    }

    /** @brief Legacy constructor with an explicit algorithm label (shape only). */
    JwtSigner(std::string secret, std::string algorithm) :
        m_secret{std::move(secret)},
        m_algorithm{std::move(algorithm)}
    {
    }

    /** @brief Repository-backed lookup; the resolver supplies the secret on each use. */
    JwtSigner(ISecretResolver& resolver, std::string key_id, std::string purpose = "jwt") :
        m_resolver{&resolver},
        m_key_id{std::move(key_id)},
        m_purpose{std::move(purpose)}
    {
    }

    /** @brief Selects the alg header value. RS256 here is wire-shape only. */
    void set_algorithm(std::string algorithm)
    {
        m_algorithm = std::move(algorithm);
    }

    [[nodiscard]] const std::string& get_algorithm() const noexcept
    {
        return m_algorithm;
    }

    [[nodiscard]] std::string sign(std::string_view payload) const
    {
        const std::optional<std::string> secret = resolve_secret();
        if (!secret) {
            return std::string{};
        }
        const std::string encoded_header = m_codec.encode(header());
        const std::string encoded_payload = m_codec.encode(payload);
        const std::string signing_input = encoded_header + "." + encoded_payload;
        const std::string encoded_signature =
            m_codec.encode(signature_bytes(signing_input, *secret));
        return signing_input + "." + encoded_signature;
    }

    /** @brief Builds and signs a typed claim set. */
    [[nodiscard]] std::string sign_claims(const JwtClaims& claims) const
    {
        return sign(claims.to_json());
    }

    /** @brief Legacy shape-and-signature check; kept for existing callers. */
    [[nodiscard]] bool verify(std::string_view token) const
    {
        return verify_with_options(token, JwtVerificationOptions{}.require_expiry(false)).ok();
    }

    /** @brief Strict verification of signature, expiry, issuer, audience and subject. */
    [[nodiscard]] bool verify(std::string_view token, const JwtVerificationOptions& options) const
    {
        return verify_with_options(token, options).ok();
    }

    /** @brief Detailed strict verification with a machine-readable reason. */
    [[nodiscard]] JwtVerifyResult
    verify_with_options(std::string_view token, const JwtVerificationOptions& options) const
    {
        const std::size_t first = token.find('.');
        if (first == std::string_view::npos) {
            return {false, "bad-shape"};
        }
        const std::size_t second = token.find('.', first + 1);
        if (second == std::string_view::npos ||
            token.find('.', second + 1) != std::string_view::npos) {
            return {false, "bad-shape"};
        }
        if (token.substr(0, first) != m_codec.encode(header())) {
            return {false, "bad-header"};
        }

        const std::optional<std::string> secret = resolve_secret();
        if (!secret) {
            return {false, "no-secret"};
        }
        const std::string signing_input{token.substr(0, second)};
        const std::string expected = m_codec.encode(signature_bytes(signing_input, *secret));
        if (expected != token.substr(second + 1)) {
            return {false, "bad-signature"};
        }

        const std::string payload = m_codec.decode(token.substr(first + 1, second - first - 1));
        if (payload.empty()) {
            return {false, "bad-payload"};
        }

        if (options.get_require_expiry()) {
            const std::optional<std::int64_t> exp = number_field(payload, "exp");
            if (!exp) {
                return {false, "missing-exp"};
            }
            if (*exp <= options.get_now_seconds()) {
                return {false, "expired"};
            }
        }
        if (!options.get_expected_issuer().empty() &&
            string_field(payload, "iss") != options.get_expected_issuer()) {
            return {false, "issuer-mismatch"};
        }
        if (!options.get_expected_audience().empty() &&
            string_field(payload, "aud") != options.get_expected_audience()) {
            return {false, "audience-mismatch"};
        }
        if (!options.get_expected_subject().empty() &&
            string_field(payload, "sub") != options.get_expected_subject()) {
            return {false, "subject-mismatch"};
        }
        return {true, "ok"};
    }

private:
    [[nodiscard]] std::optional<std::string> resolve_secret() const
    {
        if (m_resolver != nullptr) {
            return m_resolver->resolve(m_purpose, m_key_id);
        }
        if (m_secret.empty()) {
            return std::nullopt;
        }
        return m_secret;
    }

    [[nodiscard]] std::string header() const
    {
        return std::string{"{\"alg\":\""} + m_algorithm + "\",\"typ\":\"JWT\"}";
    }

    [[nodiscard]] std::string
    signature_bytes(std::string_view signing_input, std::string_view secret) const
    {
        return KeyedHasher{std::string{secret}}.hash_with_domain(HashDomain::Token, signing_input);
    }

    [[nodiscard]] std::size_t field_value_start(std::string_view json, std::string_view key) const
    {
        std::string needle{"\""};
        needle.append(key);
        needle.push_back('"');
        const std::size_t key_pos = json.find(needle);
        if (key_pos == std::string_view::npos) {
            return std::string_view::npos;
        }
        std::size_t pos = key_pos + needle.size();
        while (pos < json.size() && (json[pos] == ' ' || json[pos] == '\t')) {
            ++pos;
        }
        if (pos >= json.size() || json[pos] != ':') {
            return std::string_view::npos;
        }
        ++pos;
        while (pos < json.size() && (json[pos] == ' ' || json[pos] == '\t')) {
            ++pos;
        }
        return pos;
    }

    [[nodiscard]] std::optional<std::string>
    string_field(std::string_view json, std::string_view key) const
    {
        const std::size_t start = field_value_start(json, key);
        if (start == std::string_view::npos || start >= json.size() || json[start] != '"') {
            return std::nullopt;
        }
        std::string out{};
        for (std::size_t i{start + 1}; i < json.size(); ++i) {
            if (json[i] == '\\' && i + 1 < json.size()) {
                out.push_back(json[++i]);
                continue;
            }
            if (json[i] == '"') {
                return out;
            }
            out.push_back(json[i]);
        }
        return std::nullopt;
    }

    [[nodiscard]] std::optional<std::int64_t>
    number_field(std::string_view json, std::string_view key) const
    {
        const std::size_t start = field_value_start(json, key);
        if (start == std::string_view::npos || start >= json.size()) {
            return std::nullopt;
        }
        std::size_t i{start};
        bool negative{false};
        if (json[i] == '-') {
            negative = true;
            ++i;
        }
        std::int64_t value{0};
        std::size_t digits{0};
        for (; i < json.size() && json[i] >= '0' && json[i] <= '9'; ++i) {
            value = value * 10 + (json[i] - '0');
            ++digits;
        }
        if (digits == 0) {
            return std::nullopt;
        }
        return negative ? -value : value;
    }

    std::string m_secret{};
    std::string m_algorithm{"HS256"};
    std::string m_key_id{};
    std::string m_purpose{"jwt"};
    ISecretResolver* m_resolver{nullptr};
    Base64Url m_codec{};
};

} // namespace app::crypto

#ifdef CONGELADO_TEST
namespace app::crypto::tests {
using namespace boost::ut;

namespace {
    class TestSecretResolver final : public ISecretResolver
    {
    public:
        explicit TestSecretResolver(std::string secret) :
            m_secret{std::move(secret)}
        {
        }

        [[nodiscard]] std::optional<std::string>
        resolve(std::string_view purpose, std::string_view key_id) override
        {
            if (purpose == "jwt" && key_id == "key_test") {
                return m_secret;
            }
            return std::nullopt;
        }

    private:
        std::string m_secret{};
    };
} // namespace

suite<"app_crypto_token"> token_suite = []
{
    "base64url round-trips arbitrary bytes without padding"_test = []
    {
        const Base64Url codec;
        const std::string source{
            static_cast<char>(0xFB),
            static_cast<char>(0xFF),
            static_cast<char>(0xBF),
            static_cast<char>(0x00),
            static_cast<char>(0x41)
        };
        const std::string encoded = codec.encode(source);
        expect(encoded.find('+') == std::string::npos);
        expect(encoded.find('/') == std::string::npos);
        expect(encoded.find('=') == std::string::npos);
        expect(codec.decode(encoded) == source);
        expect(codec.encode("f") == "Zg");
        expect(codec.decode("Zm9v") == "foo");
        expect(codec.decode("!").empty());
    };

    "jwt signs, verifies and rejects tampering"_test = []
    {
        const JwtSigner signer{"jwt-secret"};
        const std::string token = signer.sign("{\"sub\":\"user\"}");
        expect(signer.verify(token));
        expect(token.find('.') != std::string::npos);
        expect(token.rfind('.') != token.find('.'));

        std::string tampered{token};
        tampered.back() = tampered.back() == 'A' ? 'B' : 'A';
        expect(!signer.verify(tampered));
        expect(!signer.verify("not-a-jwt"));
        expect(!JwtSigner{"other-secret"}.verify(token));
    };

    "jwt signature is deterministic for the same secret and payload"_test = []
    {
        const JwtSigner signer{"jwt-secret"};
        expect(signer.sign("payload") == signer.sign("payload"));
        expect(signer.sign("payload") != JwtSigner{"jwt-secret"}.sign("different"));
    };

    "resolver-backed signer uses KeyStore-style lookup"_test = []
    {
        TestSecretResolver resolver{"resolved-secret"};
        const JwtSigner signer{resolver, "key_test"};
        const std::string token = signer.sign("{\"sub\":\"user\"}");
        expect(!token.empty());
        expect(signer.verify(token));

        JwtSigner missing{resolver, "unknown_key"};
        expect(missing.sign("payload").empty());
        expect(!missing.verify(token));
    };

    "claims are verified for signature, expiry, issuer, audience and subject"_test = []
    {
        const JwtSigner signer{"jwt-secret"};
        const JwtClaims claims = JwtClaims{}
                                     .issuer("hyperswitch")
                                     .audience("api")
                                     .subject("user-1")
                                     .expires_at(2'000)
                                     .issued_at(1'000);
        const std::string token = signer.sign_claims(claims);
        const JwtVerificationOptions options = JwtVerificationOptions{}
                                                   .expected_issuer("hyperswitch")
                                                   .expected_audience("api")
                                                   .expected_subject("user-1")
                                                   .now_seconds(1'500);
        expect(signer.verify(token, options));
        expect(signer.verify_with_options(token, options).ok());

        expect(!signer.verify(token, JwtVerificationOptions{}.now_seconds(2'000)));
        expect(!signer.verify(
            token,
            JwtVerificationOptions{}.expected_issuer("other").now_seconds(1'500)
        ));
        expect(!signer.verify(
            token,
            JwtVerificationOptions{}.expected_audience("other").now_seconds(1'500)
        ));
        expect(!signer.verify(
            token,
            JwtVerificationOptions{}.expected_subject("other").now_seconds(1'500)
        ));
        expect(!signer.verify(signer.sign("payload"), JwtVerificationOptions{}));
    };

    "RS256 is a labelled wire-shape stand-in"_test = []
    {
        JwtSigner signer{"jwt-secret"};
        signer.set_algorithm("RS256");
        expect(signer.get_algorithm() == "RS256");
        const std::string token = signer.sign_claims(JwtClaims{}.subject("user"));
        expect(signer.verify(token));
        expect(!JwtSigner{"jwt-secret"}.verify(token));
    };
};

} // namespace app::crypto::tests
#endif
