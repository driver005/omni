export module core_auth:token;

import std;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace core::auth {

/** @brief An opaque session/access token with an expiry. */
class Token
{
public:
    using Clock = std::chrono::system_clock;

    Token() = default;

    Token(std::string value, Clock::time_point expires) :
        m_value{std::move(value)},
        m_expires{expires}
    {
    }

    [[nodiscard]] const std::string& value() const noexcept
    {
        return m_value;
    }

    [[nodiscard]] Clock::time_point expires() const noexcept
    {
        return m_expires;
    }

    [[nodiscard]] bool expired(Clock::time_point now = Clock::now()) const noexcept
    {
        return now >= m_expires;
    }

private:
    std::string m_value{};
    Clock::time_point m_expires{Clock::time_point::min()};
};

} // namespace core::auth

#ifdef CONGELADO_TEST
namespace core::auth::tests {
using namespace boost::ut;

suite<"core_auth_token"> core_auth_token_suite = []
{
    "carries value and expiry"_test = []
    {
        const Token token{"abc", Token::Clock::now() + std::chrono::minutes{5}};
        expect(token.value() == "abc");
        expect(!token.expired());
        const Token past{"abc", Token::Clock::now() - std::chrono::minutes{5}};
        expect(past.expired());
    };
};

} // namespace core::auth::tests
#endif
