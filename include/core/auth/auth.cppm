export module core_auth:auth;

import std;
import :auth_status;
import :password_hash;
import :token;
import :permission;
import :role;
import :api_key;
import :auth_runtime;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace core::auth {

/**
 * @brief The only handle a consumer holds. All members inline, shared_ptr pImpl,
 * no free/static functions. Built by AuthBuilder.
 */
class Auth
{
public:
    Auth() :
        m_runtime{std::make_shared<detail::AuthRuntime>()}
    {
    }

    Auth(const Auth&) = delete;
    Auth& operator=(const Auth&) = delete;
    Auth(Auth&&) noexcept = default;
    Auth& operator=(Auth&&) noexcept = default;
    ~Auth() = default;

    [[nodiscard]] std::optional<PasswordHash> hash_password(std::string_view password) const
    {
        return m_runtime->hasher().hash(password);
    }

    [[nodiscard]] AuthStatus verify_password(
        std::string_view password,
        const PasswordHash& stored
    ) const
    {
        return m_runtime->hasher().check(password, stored);
    }

    [[nodiscard]] std::string sign(std::string_view payload) const
    {
        return m_runtime->signer().sign(payload);
    }

    [[nodiscard]] std::optional<std::string> verify(std::string_view jwt) const
    {
        return m_runtime->signer().verify(jwt);
    }

    [[nodiscard]] Token create_session(std::string user_id, std::chrono::minutes ttl)
    {
        return m_runtime->sessions().create(std::move(user_id), ttl);
    }

    [[nodiscard]] std::optional<std::string> find_session(std::string_view token) const
    {
        return m_runtime->sessions().find(token);
    }

    bool revoke_session(std::string_view token)
    {
        return m_runtime->sessions().remove(token);
    }

    void add_role(Role role)
    {
        m_runtime->rbac().add(std::move(role));
    }

    [[nodiscard]] AuthStatus check_permission(
        std::string_view role,
        Permission permission
    ) const
    {
        return m_runtime->rbac().check(role, permission);
    }

    void add_key(ApiKey key)
    {
        m_runtime->add_key(std::move(key));
    }

    [[nodiscard]] const ApiKey* key(std::string_view id) const
    {
        return m_runtime->key(id);
    }

private:
    std::shared_ptr<detail::AuthRuntime> m_runtime{};
};

} // namespace core::auth

#ifdef CONGELADO_TEST
namespace core::auth::tests {
using namespace boost::ut;

suite<"core_auth_auth"> core_auth_auth_suite = []
{
    "facade wires hashing, JWT, sessions and RBAC"_test = []
    {
        Auth auth;
        const auto stored = auth.hash_password("secret");
        expect(stored.has_value());
        expect(auth.verify_password("secret", *stored) == AuthStatus::OK);

        const auto jwt = auth.sign(R"({"sub":"7"})");
        expect(auth.verify(jwt).has_value());

        const auto session = auth.create_session("7", std::chrono::minutes{5});
        expect(auth.find_session(session.value()) == std::string{"7"});

        Role editor{"editor"};
        editor.grant(Permission::WRITE);
        auth.add_role(std::move(editor));
        expect(auth.check_permission("editor", Permission::WRITE) == AuthStatus::OK);
    };
};

} // namespace core::auth::tests
#endif
