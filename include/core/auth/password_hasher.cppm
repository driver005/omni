module;
#include <sodium.h>

export module core_auth:password_hasher;

import std;
import :auth_status;
import :password_hash;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace core::auth {

/**
 * @brief Argon2id hasher backed by libsodium (faster than bcrypt at interactive
 * parameters and memory-hard). All methods are instance methods.
 */
class PasswordHasher
{
public:
    PasswordHasher()
    {
        [[maybe_unused]] const int init_result = sodium_init();
    }

    [[nodiscard]] std::optional<PasswordHash> hash(std::string_view password) const
    {
        std::array<char, crypto_pwhash_STRBYTES> encoded{};
        if (crypto_pwhash_str(
                encoded.data(),
                password.data(),
                password.size(),
                crypto_pwhash_OPSLIMIT_INTERACTIVE,
                crypto_pwhash_MEMLIMIT_INTERACTIVE
            ) != 0) {
            return std::nullopt;
        }
        return PasswordHash{std::string{encoded.data()}};
    }

    [[nodiscard]] bool verify(std::string_view password, const PasswordHash& stored) const
    {
        return stored.verify(password);
    }

    /** @brief Selects the meaningful status for a verification result. */
    [[nodiscard]] AuthStatus check(std::string_view password, const PasswordHash& stored) const
    {
        if (!stored.valid()) {
            return AuthStatus::HASH_FAILED;
        }
        return verify(password, stored) ? AuthStatus{AuthStatus::OK}
                                        : AuthStatus::INVALID_CREDENTIALS;
    }
};

} // namespace core::auth

#ifdef CONGELADO_TEST
namespace core::auth::tests {
using namespace boost::ut;

suite<"core_auth_password_hasher"> core_auth_password_hasher_suite = []
{
    "hashes and verifies a password"_test = []
    {
        const PasswordHasher hasher;
        const auto stored = hasher.hash("correct horse battery staple");
        expect(stored.has_value());
        expect(hasher.verify("correct horse battery staple", *stored));
        expect(!hasher.verify("wrong", *stored));
        expect(hasher.check("correct horse battery staple", *stored) == AuthStatus::OK);
        expect(hasher.check("wrong", *stored) == AuthStatus::INVALID_CREDENTIALS);
    };
};

} // namespace core::auth::tests
#endif
