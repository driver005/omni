module;
#include <sodium.h>

export module core_auth:session_store;

import std;
import :auth_status;
import :token;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace core::auth {

/** @brief In-memory session store with opaque tokens and expiries. */
class SessionStore
{
public:
    SessionStore()
    {
        [[maybe_unused]] const int init_result = sodium_init();
    }

    [[nodiscard]] Token create(std::string user_id, std::chrono::minutes ttl)
    {
        std::array<unsigned char, 32> bytes{};
        randombytes_buf(bytes.data(), bytes.size());
        std::string value;
        value.reserve(bytes.size() * 2);
        for (const auto byte: bytes) {
            value += std::format("{:02x}", byte);
        }
        const Token token{value, Token::Clock::now() + ttl};
        m_sessions.emplace(value, Session{std::move(user_id), token.expires()});
        return token;
    }

    [[nodiscard]] std::optional<std::string> find(std::string_view token) const
    {
        const auto it = m_sessions.find(std::string{token});
        if (it == m_sessions.end() || Token::Clock::now() >= it->second.expires) {
            return std::nullopt;
        }
        return it->second.user_id;
    }

    bool remove(std::string_view token)
    {
        return m_sessions.erase(std::string{token}) != 0;
    }

    [[nodiscard]] std::size_t size() const noexcept
    {
        return m_sessions.size();
    }

    void clear() noexcept
    {
        m_sessions.clear();
    }

private:
    struct Session
    {
        std::string user_id{};
        Token::Clock::time_point expires{Token::Clock::time_point::min()};
    };

    std::map<std::string, Session> m_sessions{};
};

} // namespace core::auth

#ifdef CONGELADO_TEST
namespace core::auth::tests {
using namespace boost::ut;

suite<"core_auth_session_store"> core_auth_session_store_suite = []
{
    "creates, finds and removes sessions"_test = []
    {
        SessionStore store;
        const auto token = store.create("user-7", std::chrono::minutes{5});
        expect(store.size() == 1);
        expect(store.find(token.value()) == std::string{"user-7"});
        expect(store.remove(token.value()));
        expect(!store.find(token.value()).has_value());
        expect(store.size() == 0);
    };

    "expired sessions are not found"_test = []
    {
        SessionStore store;
        const auto token = store.create("user-7", std::chrono::minutes{-1});
        expect(!store.find(token.value()).has_value());
    };
};

} // namespace core::auth::tests
#endif
