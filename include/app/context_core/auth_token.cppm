module;
export module app_context_core:auth_token;

import std;
import interfaces;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace app::auth {

/** @brief Bearer credential carried by an Authorization header. */
class BearerToken
{
public:
    BearerToken() = default;

    explicit BearerToken(std::string value) :
        m_value{std::move(value)}
    {
    }

    [[nodiscard]] const std::string& value() const noexcept
    {
        return m_value;
    }

    [[nodiscard]] bool is_empty() const noexcept
    {
        return m_value.empty();
    }

private:
    std::string m_value{};
};

/** @brief Instance functor extracting a BearerToken from an IRequest. */
class TokenParser
{
public:
    [[nodiscard]] BearerToken operator()(interfaces::io::IRequest& request) const
    {
        constexpr std::string_view scheme{"Bearer "};
        const std::string_view header = request.find_header("authorization");
        if (header.size() <= scheme.size()) {
            return BearerToken{};
        }
        const bool matches = std::equal(
            scheme.begin(),
            scheme.end(),
            header.begin(),
            [](char lhs, char rhs)
            {
                return std::tolower(static_cast<unsigned char>(lhs)) ==
                       std::tolower(static_cast<unsigned char>(rhs));
            }
        );
        if (!matches) {
            return BearerToken{};
        }
        return BearerToken{std::string{header.substr(scheme.size())}};
    }
};

} // namespace app::auth

#ifdef CONGELADO_TEST
namespace app::auth::tests {
using namespace boost::ut;

suite<"app_auth_token"> token_suite = []
{
    "bearer token reports its value and emptiness"_test = []
    {
        const BearerToken empty;
        const BearerToken filled{"abc"};
        expect(empty.is_empty());
        expect(!filled.is_empty());
        expect(filled.value() == "abc");
    };

    "parser strips the case-insensitive Bearer scheme"_test = []
    {
        struct FakeRequest : interfaces::io::IRequest
        {
            explicit FakeRequest(std::string auth) :
                interfaces::io::IRequest{0},
                m_auth{std::move(auth)}
            {
            }

            [[nodiscard]] std::string_view find_header(
                std::variant<std::string_view, interfaces::io::types::Token>
            ) const noexcept override
            {
                return m_auth;
            }

            std::string m_auth{};
        };

        const TokenParser parse;
        FakeRequest bearer{"Bearer abc.def"};
        FakeRequest lower{"bearer xyz"};
        FakeRequest missing;
        FakeRequest malformed{"abc"};

        expect(parse(bearer).value() == "abc.def");
        expect(parse(lower).value() == "xyz");
        expect(parse(missing).is_empty());
        expect(parse(malformed).is_empty());
    };
};

} // namespace app::auth::tests
#endif
