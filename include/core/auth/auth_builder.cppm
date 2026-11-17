export module core_auth:auth_builder;

import std;
import :auth;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace core::auth {

/** @brief Fluent entry point — sole public way to construct an Auth handle. */
class AuthBuilder
{
public:
    AuthBuilder& option(std::string key, std::string value)
    {
        m_options.insert_or_assign(std::move(key), std::move(value));
        return *this;
    }

    [[nodiscard]] Auth build() const
    {
        return Auth{};
    }

private:
    std::map<std::string, std::string> m_options{};
};

} // namespace core::auth

#ifdef CONGELADO_TEST
namespace core::auth::tests {
using namespace boost::ut;

suite<"core_auth_auth_builder"> core_auth_auth_builder_suite = []
{
    "builds a working Auth handle"_test = []
    {
        const Auth auth = AuthBuilder{}.option("issuer", "ghost").build();
        expect(auth.verify(auth.sign("payload")).has_value());
    };
};

} // namespace core::auth::tests
#endif
