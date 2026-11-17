export module blog_auth:auth;

import std;
import core_auth;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace blog::auth {

/** @brief Ghost authentication service over the core auth facade. */
class AuthService
{
public:
    AuthService() :
        m_auth{core::auth::AuthBuilder{}.build()}
    {
    }

    [[nodiscard]] std::optional<core::auth::PasswordHash> hash_password(std::string_view password) const
    {
        return m_auth.hash_password(password);
    }

    [[nodiscard]] core::auth::AuthStatus verify(std::string_view password, const core::auth::PasswordHash& stored) const
    {
        return m_auth.verify_password(password, stored);
    }

    [[nodiscard]] std::string issue_token(std::string_view subject) const
    {
        return m_auth.sign(std::format("{{\"sub\":\"{}\"}}", subject));
    }

    [[nodiscard]] bool valid_token(std::string_view token) const
    {
        return m_auth.verify(token).has_value();
    }

private:
    core::auth::Auth m_auth{};
};

} // namespace blog::auth

#ifdef CONGELADO_TEST
namespace blog::auth::tests {
using namespace boost::ut;
suite<"blog_auth_auth"> blog_auth_auth_suite = []
{
    "hashes and issues tokens"_test = []
    {
        AuthService service;
        const auto stored = service.hash_password("secret");
        expect(stored.has_value());
        expect(service.verify("secret", *stored) == core::auth::AuthStatus::OK);
        const auto token = service.issue_token("7");
        expect(service.valid_token(token));
    };
};
} // namespace blog::auth::tests
#endif
