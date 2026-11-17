export module core_auth:auth_status;

import std;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace core::auth {

/** @brief Outcome codes for auth operations. */
enum class AuthStatus : std::uint8_t
{
    OK,
    INVALID_CREDENTIALS,
    EXPIRED,
    FORBIDDEN,
    NOT_FOUND,
    INVALID_TOKEN,
    HASH_FAILED
};

/** @brief Instance functor naming an AuthStatus — no free/static functions. */
class AuthStatusName
{
public:
    [[nodiscard]] std::string_view operator()(AuthStatus status) const noexcept
    {
        switch (status) {
            case AuthStatus::OK:
                return "ok";
            case AuthStatus::INVALID_CREDENTIALS:
                return "invalid_credentials";
            case AuthStatus::EXPIRED:
                return "expired";
            case AuthStatus::FORBIDDEN:
                return "forbidden";
            case AuthStatus::NOT_FOUND:
                return "not_found";
            case AuthStatus::INVALID_TOKEN:
                return "invalid_token";
            case AuthStatus::HASH_FAILED:
                return "hash_failed";
        }
        return "unknown";
    }
};

} // namespace core::auth

#ifdef CONGELADO_TEST
namespace core::auth::tests {
using namespace boost::ut;

suite<"core_auth_status"> core_auth_status_suite = []
{
    "names every status"_test = []
    {
        const AuthStatusName name;
        expect(name(AuthStatus::OK) == "ok");
        expect(name(AuthStatus::FORBIDDEN) == "forbidden");
        expect(name(AuthStatus::HASH_FAILED) == "hash_failed");
    };
};

} // namespace core::auth::tests
#endif
