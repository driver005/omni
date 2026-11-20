module;
export module app_crypto:secret;

import std;
import :hash;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace app::crypto {

/** @brief Field classes upstream masks in logs, DTOs and diagnostics.
 *
 *  API keys, connector credentials, webhook secrets and client secrets keep
 *  four leading and four trailing characters. Tax IDs keep only the last four
 *  digits. Document details are fully masked because they carry PII.
 */
enum class SecretFieldKind
{
    Generic,
    ApiKey,
    ConnectorCredential,
    WebhookSecret,
    ClientSecret,
    TaxId,
    DocumentDetail
};

/** @brief Owning secret string whose default presentation is masked. */
class SecretString
{
public:
    SecretString() = default;

    explicit SecretString(std::string value) :
        m_value{std::move(value)}
    {
    }

    /** @brief Full value; callers must opt into revealing it explicitly. */
    [[nodiscard]] const std::string& value() const noexcept
    {
        return m_value;
    }

    void set(std::string value)
    {
        m_value = std::move(value);
    }

    /** @brief First four and last four characters, stars in between.
     *
     *  Short values (8 chars or fewer) are fully masked.
     */
    [[nodiscard]] std::string masked() const
    {
        return masked_as(SecretFieldKind::Generic);
    }

    /** @brief Mask according to the upstream field class. */
    [[nodiscard]] std::string masked_as(SecretFieldKind kind) const
    {
        if (m_value.empty()) {
            return std::string{};
        }
        if (kind == SecretFieldKind::DocumentDetail) {
            return std::string(m_value.size(), '*');
        }
        if (kind == SecretFieldKind::TaxId) {
            return mask_tail(m_value, 4);
        }
        if (m_value.size() <= 8) {
            return std::string(m_value.size(), '*');
        }
        std::string out{};
        out.reserve(m_value.size());
        out.append(m_value, 0, 4);
        out.append(m_value.size() - 8, '*');
        out.append(m_value, m_value.size() - 4, 4);
        return out;
    }

    /** @brief Non-revealing, domain-separated cache-key material.
     *
     *  Never returns any part of the secret. A caller that needs an
     *  idempotency or lookup key must use this instead of value().
     */
    [[nodiscard]] std::string cache_key(
        HashDomain domain = HashDomain::Idempotency) const
    {
        return KeyedHasher{"app-secret-cache-key"}.hash_with_domain(domain, m_value);
    }

    [[nodiscard]] explicit operator std::string_view() const noexcept
    {
        return m_value;
    }

private:
    [[nodiscard]] std::string mask_tail(std::string_view value, std::size_t keep) const
    {
        const std::size_t visible = keep < value.size() ? keep : value.size();
        std::string out(value.size() - visible, '*');
        out.append(value, value.size() - visible, visible);
        return out;
    }

    std::string m_value{};
};

/** @brief Instance functor for masking raw field values before they are logged. */
class SecretMasker
{
public:
    [[nodiscard]] std::string mask(
        SecretFieldKind kind, std::string_view value) const
    {
        return SecretString{std::string{value}}.masked_as(kind);
    }
};

/** @brief Instance builder/functor for SecretString values. */
class SecretStringBuilder
{
public:
    SecretStringBuilder& value(std::string value)
    {
        m_value = std::move(value);
        return *this;
    }

    [[nodiscard]] SecretString build()
    {
        return SecretString{std::move(m_value)};
    }

    [[nodiscard]] SecretString operator()(std::string value) const
    {
        return SecretString{std::move(value)};
    }

private:
    std::string m_value{};
};

} // namespace app::crypto

#ifdef CONGELADO_TEST
namespace app::crypto::tests {
using namespace boost::ut;

suite<"app_crypto_secret"> secret_suite = []
{
    "masked keeps the edges and hides the middle"_test = []
    {
        const SecretString secret{"abcd1234efgh"};
        expect(secret.masked() == "abcd****efgh");
        expect(secret.masked().size() == secret.value().size());
        expect(secret.value() == "abcd1234efgh");
        expect(static_cast<std::string_view>(secret) == "abcd1234efgh");
    };

    "short secrets are fully masked"_test = []
    {
        expect(SecretString{"abc"}.masked() == "***");
        expect(SecretString{"12345678"}.masked() == "********");
        expect(SecretString{}.masked().empty());
    };

    "builder and functor create secret strings"_test = []
    {
        SecretStringBuilder builder;
        builder.value("token-value");
        expect(builder.build().value() == "token-value");
        expect(SecretStringBuilder{}.operator()("token-value").masked() == "toke***alue");
    };

    "every masked upstream field class hides its middle or tail"_test = []
    {
        const SecretString secret{"abcd1234efgh"};
        expect(secret.masked_as(SecretFieldKind::ApiKey) == "abcd****efgh");
        expect(secret.masked_as(SecretFieldKind::ConnectorCredential) == "abcd****efgh");
        expect(secret.masked_as(SecretFieldKind::WebhookSecret) == "abcd****efgh");
        expect(secret.masked_as(SecretFieldKind::ClientSecret) == "abcd****efgh");
        expect(secret.masked_as(SecretFieldKind::TaxId) == "********efgh");
        expect(secret.masked_as(SecretFieldKind::DocumentDetail) == "************");

        const SecretMasker masker;
        expect(masker.mask(SecretFieldKind::DocumentDetail, "A1234567") == "********");
        expect(masker.mask(SecretFieldKind::TaxId, "123") == "123");
    };

    "cache keys are stable digests that never contain the secret"_test = []
    {
        const SecretString secret{"super-sensitive-value"};
        const std::string key = secret.cache_key(HashDomain::Idempotency);
        expect(key.size() == 16);
        expect(key.find("super") == std::string::npos);
        expect(key == secret.cache_key(HashDomain::Idempotency));
        expect(key != secret.cache_key(HashDomain::Token));
        expect(key != SecretString{"other-value"}.cache_key(HashDomain::Idempotency));
    };
};

} // namespace app::crypto::tests
#endif
