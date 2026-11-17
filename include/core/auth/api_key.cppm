export module core_auth:api_key;

import std;
import :permission;
import :password_hash;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace core::auth {

/** @brief An API key: public id, hashed secret and granted permissions. */
class ApiKey
{
public:
    ApiKey() = default;

    ApiKey(std::string id, PasswordHash secret_hash, Permission permissions) :
        m_id{std::move(id)},
        m_secret_hash{std::move(secret_hash)},
        m_permissions{permissions}
    {
    }

    [[nodiscard]] const std::string& id() const noexcept
    {
        return m_id;
    }

    [[nodiscard]] bool matches(std::string_view secret) const noexcept
    {
        return m_secret_hash.verify(secret);
    }

    [[nodiscard]] bool permits(Permission permission) const noexcept
    {
        const PermissionHas has;
        return has(m_permissions, permission);
    }

private:
    std::string m_id{};
    PasswordHash m_secret_hash{};
    Permission m_permissions{Permission::NONE};
};

} // namespace core::auth

#ifdef CONGELADO_TEST
namespace core::auth::tests {
using namespace boost::ut;

suite<"core_auth_api_key"> core_auth_api_key_suite = []
{
    "matches a secret and checks permissions"_test = []
    {
        const std::string encoded = "$argon2id$v=19$m=65536,t=2,p=1$..."
                                    "c29tZWhhc2g";
        const ApiKey key{"key-1", PasswordHash{encoded}, Permission::READ};
        expect(key.id() == "key-1");
        expect(key.permits(Permission::READ));
        expect(!key.permits(Permission::ADMIN));
    };
};

} // namespace core::auth::tests
#endif
