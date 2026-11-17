module;
#include <sodium.h>

export module core_auth:jwt_signer;

import std;
import :status;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace core::auth {

/**
 * @brief HMAC-SHA256 JWT signer/verifier using libsodium. The signing key is
 * generated at construction, never copied out, and no crypto primitive is
 * hand-written.
 */
class JwtSigner
{
public:
    JwtSigner()
    {
        [[maybe_unused]] const int init_result = sodium_init();
        randombytes_buf(m_key.data(), m_key.size());
    }

    /** @brief Signs a payload string, returning "<header>.<payload>.<sig>". */
    [[nodiscard]] std::string sign(std::string_view payload) const
    {
        const std::string header = b64url(R"({"alg":"HS256","typ":"JWT"})");
        const std::string body = b64url(payload);
        const std::string signing_input = header + "." + body;
        std::array<unsigned char, crypto_auth_hmacsha256_BYTES> signature{};
        crypto_auth_hmacsha256(
            signature.data(),
            reinterpret_cast<const unsigned char*>(signing_input.data()),
            signing_input.size(),
            m_key.data()
        );
        return signing_input + "." + b64url(
            std::string_view{reinterpret_cast<const char*>(signature.data()), signature.size()}
        );
    }

    /** @brief Verifies signature and returns the payload, or nullopt. */
    [[nodiscard]] std::optional<std::string> verify(std::string_view token) const
    {
        const auto first = token.find('.');
        const auto second = first == std::string_view::npos
                                ? std::string_view::npos
                                : token.find('.', first + 1);
        if (first == std::string_view::npos || second == std::string_view::npos) {
            return std::nullopt;
        }
        const auto signing_input = token.substr(0, second);
        const auto encoded_signature = token.substr(second + 1);
        const auto decoded = b64url_decode(encoded_signature);
        if (!decoded || decoded->size() != crypto_auth_hmacsha256_BYTES) {
            return std::nullopt;
        }
        std::array<unsigned char, crypto_auth_hmacsha256_BYTES> expected{};
        crypto_auth_hmacsha256(
            expected.data(),
            reinterpret_cast<const unsigned char*>(signing_input.data()),
            signing_input.size(),
            m_key.data()
        );
        if (sodium_memcmp(expected.data(), decoded->data(), expected.size()) != 0) {
            return std::nullopt;
        }
        return b64url_decode(token.substr(first + 1, second - first - 1));
    }

private:
    [[nodiscard]] std::string b64url(std::string_view input) const
    {
        static constexpr char TABLE[] =
            "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";
        std::string out;
        out.reserve((input.size() * 4 / 3) + 4);
        std::size_t i = 0;
        while (i + 2 < input.size()) {
            const unsigned value =
                (static_cast<unsigned char>(input[i]) << 16) |
                (static_cast<unsigned char>(input[i + 1]) << 8) |
                static_cast<unsigned char>(input[i + 2]);
            out += TABLE[(value >> 18) & 63];
            out += TABLE[(value >> 12) & 63];
            out += TABLE[(value >> 6) & 63];
            out += TABLE[value & 63];
            i += 3;
        }
        if (i < input.size()) {
            unsigned value = static_cast<unsigned char>(input[i]) << 16;
            if (i + 1 < input.size()) {
                value |= static_cast<unsigned char>(input[i + 1]) << 8;
            }
            out += TABLE[(value >> 18) & 63];
            out += TABLE[(value >> 12) & 63];
            if (i + 1 < input.size()) {
                out += TABLE[(value >> 6) & 63];
            }
        }
        return out;
    }

    [[nodiscard]] std::optional<std::string> b64url_decode(std::string_view input) const
    {
        if (input.size() % 4 == 1) {
            return std::nullopt;
        }
        std::string out;
        out.reserve(input.size() * 3 / 4);
        unsigned buffer = 0;
        int bits = 0;
        for (const char c: input) {
            int value = -1;
            if (c >= 'A' && c <= 'Z') {
                value = c - 'A';
            } else if (c >= 'a' && c <= 'z') {
                value = c - 'a' + 26;
            } else if (c >= '0' && c <= '9') {
                value = c - '0' + 52;
            } else if (c == '-') {
                value = 62;
            } else if (c == '_') {
                value = 63;
            }
            if (value < 0) {
                return std::nullopt;
            }
            buffer = (buffer << 6) | static_cast<unsigned>(value);
            bits += 6;
            if (bits >= 8) {
                bits -= 8;
                out.push_back(static_cast<char>((buffer >> bits) & 0xFF));
            }
        }
        return out;
    }

    std::array<unsigned char, crypto_auth_hmacsha256_KEYBYTES> m_key{};
};

} // namespace core::auth

#ifdef CONGELADO_TEST
namespace core::auth::tests {
using namespace boost::ut;

suite<"core_auth_jwt_signer"> core_auth_jwt_signer_suite = []
{
    "signs and verifies a payload"_test = []
    {
        const JwtSigner signer;
        const auto token = signer.sign(R"({"sub":"42"})");
        expect(token.find('.') != std::string::npos);
        const auto payload = signer.verify(token);
        expect(payload.has_value());
        expect(*payload == R"({"sub":"42"})");
        expect(!signer.verify("garbage").has_value());
    };

    "rejects a tampered signature"_test = []
    {
        const JwtSigner signer;
        auto token = signer.sign("payload");
        token.back() = token.back() == 'A' ? 'B' : 'A';
        expect(!signer.verify(token).has_value());
    };
};

} // namespace core::auth::tests
#endif
