module;
#include <sodium.h>

export module core_auth:password_hash;

import std;
import :status;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace core::auth {

/**
 * @brief An encoded Argon2id password hash (libsodium PHC string), with an
 * instance verifier. Holds no raw password and nothing backend-specific escapes.
 */
class PasswordHash
{
public:
    PasswordHash() = default;

    explicit PasswordHash(std::string encoded) :
        m_encoded{std::move(encoded)}
    {
    }

    [[nodiscard]] const std::string& encoded() const noexcept
    {
        return m_encoded;
    }

    [[nodiscard]] bool valid() const noexcept
    {
        return !m_encoded.empty();
    }

    [[nodiscard]] bool verify(std::string_view password) const noexcept
    {
        if (m_encoded.empty()) {
            return false;
        }
        return crypto_pwhash_str_verify(m_encoded.c_str(), password.data(), password.size()) == 0;
    }

private:
    std::string m_encoded{};
};

} // namespace core::auth

#ifdef CONGELADO_TEST
namespace core::auth::tests {
using namespace boost::ut;

suite<"core_auth_password_hash"> core_auth_password_hash_suite = []
{
    "empty hash is invalid and rejects everything"_test = []
    {
        const PasswordHash hash;
        expect(!hash.valid());
        expect(!hash.verify("secret"));
    };
};

} // namespace core::auth::tests
#endif
