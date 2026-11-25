module;
#include <openssl/crypto.h>
#include <openssl/evp.h>
#include <openssl/hmac.h>
#include <openssl/rand.h>

export module core_activitypub:http_signature;

import std;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace core::activitypub {

/**
 * @brief HTTP Message Signatures (RFC 9421) primitive — HMAC-SHA256 via OpenSSL.
 * The signing secret is generated once at construction and never leaves the class.
 */
class HttpSignature
{
public:
    HttpSignature()
    {
        std::array<unsigned char, 32> bytes{};
        if (RAND_bytes(bytes.data(), static_cast<int>(bytes.size())) == 1) {
            m_secret.assign(bytes.begin(), bytes.end());
        }
    }

    explicit HttpSignature(std::vector<unsigned char> secret) :
        m_secret{std::move(secret)}
    {
    }

    [[nodiscard]] std::vector<unsigned char> sign(std::string_view message) const
    {
        std::vector<unsigned char> signature(EVP_MAX_MD_SIZE);
        unsigned int length{0};
        HMAC(
            EVP_sha256(),
            m_secret.data(),
            static_cast<int>(m_secret.size()),
            reinterpret_cast<const unsigned char*>(message.data()),
            message.size(),
            signature.data(),
            &length
        );
        signature.resize(length);
        return signature;
    }

    [[nodiscard]] bool
    verify(std::string_view message, std::span<const unsigned char> signature) const
    {
        const auto expected = sign(message);
        if (expected.size() != signature.size()) {
            return false;
        }
        return CRYPTO_memcmp(expected.data(), signature.data(), expected.size()) == 0;
    }

    [[nodiscard]] bool valid() const noexcept
    {
        return !m_secret.empty();
    }

private:
    std::vector<unsigned char> m_secret{};
};

} // namespace core::activitypub

#ifdef CONGELADO_TEST
namespace core::activitypub::tests {
using namespace boost::ut;

suite<"core_activitypub_http_signature"> core_activitypub_http_signature_suite = []
{
    "signs and verifies a message"_test = []
    {
        HttpSignature signature;
        expect(signature.valid());
        const auto sig = signature.sign("(request-target): post /inbox");
        expect(sig.size() > 0);
        expect(signature.verify("(request-target): post /inbox", sig));
        expect(!signature.verify("(request-target): post /other", sig));
    };
};

} // namespace core::activitypub::tests
#endif
