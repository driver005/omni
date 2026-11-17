export module core_auth:role;

import std;
import :permission;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace core::auth {

/** @brief A named role carrying a set of permissions. */
class Role
{
public:
    Role() = default;

    explicit Role(std::string name) :
        m_name{std::move(name)}
    {
    }

    [[nodiscard]] const std::string& name() const noexcept
    {
        return m_name;
    }

    Role& grant(Permission permission)
    {
        m_permissions.insert(permission);
        return *this;
    }

    [[nodiscard]] bool has(Permission permission) const noexcept
    {
        return m_permissions.contains(permission);
    }

private:
    std::string m_name{};
    std::set<Permission> m_permissions{};
};

} // namespace core::auth

#ifdef CONGELADO_TEST
namespace core::auth::tests {
using namespace boost::ut;

suite<"core_auth_role"> core_auth_role_suite = []
{
    "grants and checks permissions"_test = []
    {
        Role role{"editor"};
        role.grant(Permission::READ).grant(Permission::WRITE);
        expect(role.name() == "editor");
        expect(role.has(Permission::WRITE));
        expect(!role.has(Permission::ADMIN));
    };
};

} // namespace core::auth::tests
#endif
