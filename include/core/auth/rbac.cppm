export module core_auth:rbac;

import std;
import :status;
import :permission;
import :role;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace core::auth {

/** @brief Role-based access control registry with instance-based checks. */
class Rbac
{
public:
    void add(Role role)
    {
        m_roles.emplace(role.name(), std::move(role));
    }

    [[nodiscard]] bool allowed(std::string_view role_name, Permission permission) const
    {
        const auto it = m_roles.find(std::string{role_name});
        return it != m_roles.end() && it->second.has(permission);
    }

    [[nodiscard]] AuthStatus check(std::string_view role_name, Permission permission) const
    {
        return allowed(role_name, permission) ? AuthStatus{AuthStatus::OK}
                                              : AuthStatus::FORBIDDEN;
    }

    [[nodiscard]] std::size_t size() const noexcept
    {
        return m_roles.size();
    }

private:
    std::map<std::string, Role> m_roles{};
};

} // namespace core::auth

#ifdef CONGELADO_TEST
namespace core::auth::tests {
using namespace boost::ut;

suite<"core_auth_rbac"> core_auth_rbac_suite = []
{
    "checks roles and permissions"_test = []
    {
        Rbac rbac;
        Role admin{"admin"};
        admin.grant(Permission::ADMIN).grant(Permission::SETTINGS);
        rbac.add(std::move(admin));
        expect(rbac.size() == 1);
        expect(rbac.allowed("admin", Permission::SETTINGS));
        expect(!rbac.allowed("admin", Permission::BILLING));
        expect(rbac.check("admin", Permission::SETTINGS) == AuthStatus::OK);
        expect(rbac.check("nobody", Permission::SETTINGS) == AuthStatus::FORBIDDEN);
    };
};

} // namespace core::auth::tests
#endif
