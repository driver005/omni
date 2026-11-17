export module core_auth:auth_runtime;

import std;
import :auth_status;
import :password_hash;
import :password_hasher;
import :token;
import :jwt_signer;
import :session_store;
import :permission;
import :role;
import :rbac;
import :api_key;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace core::auth::detail {
class AuthRuntime;
}

namespace core::auth::detail {

/** @brief Internal state owned by the facade: hasher, signer, sessions, RBAC. */
class AuthRuntime
{
public:
    PasswordHasher& hasher() noexcept
    {
        return m_hasher;
    }

    JwtSigner& signer() noexcept
    {
        return m_signer;
    }

    SessionStore& sessions() noexcept
    {
        return m_sessions;
    }

    Rbac& rbac() noexcept
    {
        return m_rbac;
    }

    void add_key(ApiKey key)
    {
        m_keys.emplace(key.id(), std::move(key));
    }

    [[nodiscard]] const ApiKey* key(std::string_view id) const
    {
        const auto it = m_keys.find(std::string{id});
        return it == m_keys.end() ? nullptr : &it->second;
    }

private:
    PasswordHasher m_hasher{};
    JwtSigner m_signer{};
    SessionStore m_sessions{};
    Rbac m_rbac{};
    std::map<std::string, ApiKey> m_keys{};
};

} // namespace core::auth::detail

#ifdef CONGELADO_TEST
namespace core::auth::detail::tests {
using namespace boost::ut;

suite<"core_auth_runtime"> core_auth_runtime_suite = []
{
    "owns auth primitives"_test = []
    {
        AuthRuntime runtime;
        expect(runtime.sessions().size() == 0);
        expect(runtime.rbac().size() == 0);
    };
};

} // namespace core::auth::detail::tests
#endif
