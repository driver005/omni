module;
export module app_crypto:hash;

import std;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace app::crypto {

/** @brief Named domains for keyed hashes.
 *
 *  Domain separation keeps a digest computed for one feature from colliding
 *  with a digest of the same bytes computed for another feature. The names
 *  mirror the upstream feature boundaries named in the parity plan.
 */
enum class HashDomain
{
    Generic,
    ApiKey,
    ClientSecret,
    WebhookSignature,
    Idempotency,
    Token,
    Otp,
    RecoveryCode
};

/** @brief Deterministic keyed FNV-1a stand-in.
 *
 *  The 64-bit FNV-1a state is seeded with the key and finalized with a few
 *  avalanche steps. This mirrors the shape of a keyed digest for parity tests;
 *  it is NOT a cryptographic MAC and must not protect real data.
 */
class KeyedHasher
{
public:
    explicit KeyedHasher(std::string key) :
        m_key{std::move(key)}
    {
    }

    /** @brief 16 lowercase hex digits derived from key + input. */
    [[nodiscard]] std::string hash(std::string_view input) const
    {
        constexpr std::uint64_t offset{14695981039346656037ULL};
        constexpr std::uint64_t prime{1099511628211ULL};

        std::uint64_t state{offset};
        for (const char c : m_key) {
            state ^= static_cast<std::uint64_t>(static_cast<unsigned char>(c));
            state *= prime;
        }
        state ^= 0xA5A5A5A5A5A5A5A5ULL;
        state *= prime;
        for (const char c : input) {
            state ^= static_cast<std::uint64_t>(static_cast<unsigned char>(c));
            state *= prime;
        }
        state ^= state >> 33;
        state *= 0xFF51AFD7ED558CCDULL;
        state ^= state >> 33;
        return to_hex(state);
    }

    /** @brief Domain-separated digest. NOT a cryptographic MAC.
     *
     *  The domain, key and input are each length-framed before absorption, so
     *  ("ab", "c") and ("a", "bc") cannot produce the same state. Use this for
     *  API keys, client secrets, webhook signatures, idempotency keys and token
     *  material; use hash() only where the legacy undomain-separated digest is
     *  required for compatibility.
     */
    [[nodiscard]] std::string hash_with_domain(
        std::string_view domain, std::string_view input) const
    {
        constexpr std::uint64_t offset{14695981039346656037ULL};
        constexpr std::uint64_t prime{1099511628211ULL};

        std::uint64_t state{offset};
        absorb(state, domain, prime);
        state ^= 0xA5A5A5A5A5A5A5A5ULL;
        state *= prime;
        absorb(state, m_key, prime);
        state ^= 0x5A5A5A5A5A5A5A5AULL;
        state *= prime;
        absorb(state, input, prime);
        state ^= state >> 33;
        state *= 0xFF51AFD7ED558CCDULL;
        state ^= state >> 33;
        return to_hex(state);
    }

    /** @brief Typed-domain overload; see HashDomain. */
    [[nodiscard]] std::string hash_with_domain(
        HashDomain domain, std::string_view input) const
    {
        return hash_with_domain(domain_name(domain), input);
    }

    /** @brief Constant-shape comparison of a hash against an expected hex digest. */
    [[nodiscard]] bool verify(std::string_view input, std::string_view expected) const
    {
        return equal_digest(hash(input), expected);
    }

    /** @brief Constant-shape comparison of a domain-separated digest. */
    [[nodiscard]] bool verify_with_domain(
        std::string_view domain, std::string_view input, std::string_view expected) const
    {
        return equal_digest(hash_with_domain(domain, input), expected);
    }

    /** @brief Typed-domain verification overload. */
    [[nodiscard]] bool verify_with_domain(
        HashDomain domain, std::string_view input, std::string_view expected) const
    {
        return verify_with_domain(domain_name(domain), input, expected);
    }

private:
    /** @brief Stable spelling for each named domain. */
    [[nodiscard]] std::string_view domain_name(HashDomain domain) const
    {
        switch (domain) {
            case HashDomain::ApiKey: return "api_key";
            case HashDomain::ClientSecret: return "client_secret";
            case HashDomain::WebhookSignature: return "webhook_signature";
            case HashDomain::Idempotency: return "idempotency";
            case HashDomain::Token: return "token";
            case HashDomain::Otp: return "otp";
            case HashDomain::RecoveryCode: return "recovery_code";
            case HashDomain::Generic:
            default: return "generic";
        }
    }

    /** @brief Absorb bytes plus a length frame so field boundaries cannot alias. */
    void absorb(std::uint64_t& state, std::string_view bytes, std::uint64_t prime) const
    {
        for (const char c : bytes) {
            state ^= static_cast<std::uint64_t>(static_cast<unsigned char>(c));
            state *= prime;
        }
        state ^= static_cast<std::uint64_t>(bytes.size()) + 0x9E3779B97F4A7C15ULL;
        state *= prime;
    }

    [[nodiscard]] bool equal_digest(
        std::string_view actual, std::string_view expected) const
    {
        if (actual.size() != expected.size()) {
            return false;
        }
        unsigned diff{0};
        for (std::size_t i{0}; i < actual.size(); ++i) {
            diff |= static_cast<unsigned>(
                static_cast<unsigned char>(actual[i]) ^
                static_cast<unsigned char>(expected[i]));
        }
        return diff == 0;
    }

    [[nodiscard]] std::string to_hex(std::uint64_t value) const
    {
        constexpr std::string_view digits{"0123456789abcdef"};
        std::string out{};
        out.reserve(16);
        for (int shift{60}; shift >= 0; shift -= 4) {
            out.push_back(digits[static_cast<std::size_t>((value >> shift) & 0xFULL)]);
        }
        return out;
    }

    std::string m_key{};
};

} // namespace app::crypto

#ifdef CONGELADO_TEST
namespace app::crypto::tests {
using namespace boost::ut;

suite<"app_crypto_hash"> hash_suite = []
{
    "hash is stable, fixed-width and key-dependent"_test = []
    {
        const KeyedHasher hasher{"secret"};
        const std::string first = hasher.hash("payload");
        expect(first.size() == 16);
        expect(first == hasher.hash("payload"));
        expect(first != hasher.hash("other"));
        expect(first != KeyedHasher{"different"}.hash("payload"));
    };

    "verify accepts the digest and rejects tampering"_test = []
    {
        const KeyedHasher hasher{"secret"};
        const std::string digest = hasher.hash("payload");
        expect(hasher.verify("payload", digest));
        expect(!hasher.verify("tampered", digest));
        expect(!hasher.verify("payload", "deadbeef"));
    };

    "domain separation changes the digest and verifies per domain"_test = []
    {
        const KeyedHasher hasher{"secret"};
        const std::string api = hasher.hash_with_domain(HashDomain::ApiKey, "payload");
        const std::string secret = hasher.hash_with_domain(HashDomain::ClientSecret, "payload");
        const std::string webhook = hasher.hash_with_domain("webhook_signature", "payload");
        expect(api.size() == 16);
        expect(api != secret);
        expect(api != webhook);
        expect(secret != webhook);
        expect(api != hasher.hash("payload"));
        expect(hasher.verify_with_domain(HashDomain::ApiKey, "payload", api));
        expect(!hasher.verify_with_domain(HashDomain::ClientSecret, "payload", api));
        expect(hasher.verify_with_domain("webhook_signature", "payload", webhook));
    };

    "domain and input boundaries are length-framed"_test = []
    {
        const KeyedHasher hasher{"k"};
        const std::string lhs = hasher.hash_with_domain("ab", "c");
        const std::string rhs = hasher.hash_with_domain("a", "bc");
        expect(lhs != rhs);
    };
};

} // namespace app::crypto::tests
#endif
