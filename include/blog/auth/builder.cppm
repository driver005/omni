export module blog_auth:builder;

import std;
import :auth;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace blog::auth {

/** @brief Fluent entry point for the Ghost auth service. */
class AuthServiceBuilder
{
public:
    [[nodiscard]] AuthService build() const
    {
        return AuthService{};
    }
};

} // namespace blog::auth

#ifdef CONGELADO_TEST
namespace blog::auth::tests {
using namespace boost::ut;
suite<"blog_auth_builder"> blog_auth_builder_suite = []
{
    "builds an auth service"_test = []
    {
        const AuthService service = AuthServiceBuilder{}.build();
        expect(service.valid_token(service.issue_token("x")));
    };
};
} // namespace blog::auth::tests
#endif
