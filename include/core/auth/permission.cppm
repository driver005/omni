export module core_auth:permission;

import std;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace core::auth {

/** @brief Fine-grained permission bits used by RBAC checks. */
enum class Permission : std::uint32_t
{
    NONE = 0,
    READ = 1,
    WRITE = 2,
    DELETE = 4,
    ADMIN = 8,
    MEMBERS = 16,
    SETTINGS = 32,
    BILLING = 64
};

/** @brief Instance functor combining two Permission values. */
class PermissionCombine
{
public:
    [[nodiscard]] Permission operator()(Permission a, Permission b) const noexcept
    {
        return static_cast<Permission>(
            static_cast<std::uint32_t>(a) | static_cast<std::uint32_t>(b)
        );
    }
};

/** @brief Instance functor testing membership in a Permission mask. */
class PermissionHas
{
public:
    [[nodiscard]] bool operator()(Permission mask, Permission flag) const noexcept
    {
        return (static_cast<std::uint32_t>(mask) & static_cast<std::uint32_t>(flag)) != 0;
    }
};

} // namespace core::auth

#ifdef CONGELADO_TEST
namespace core::auth::tests {
using namespace boost::ut;

suite<"core_auth_permission"> core_auth_permission_suite = []
{
    "combines and tests permission masks"_test = []
    {
        const PermissionCombine combine;
        const PermissionHas has;
        const Permission mask = combine(Permission::READ, Permission::WRITE);
        expect(has(mask, Permission::READ));
        expect(has(mask, Permission::WRITE));
        expect(!has(mask, Permission::ADMIN));
    };
};

} // namespace core::auth::tests
#endif
