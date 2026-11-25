module;
#include <rfl/to_generic.hpp>

export module app_handlers_user_role:result;

import std;
import core_serde;
import app_dto;

import :input;

export namespace app::handlers::user_role {

/** @brief Builds typed dynamic response documents; no hand-written JSON strings. */
class ResponseDocument
{
public:
    template<typename T>
    [[nodiscard]] static serde::Value from(const T& value)
    {
        return rfl::to_generic(value);
    }
};

// Object responses derive from the exact wire type the old handler encoded and
// reuse its field descriptors, so field names and order stay byte-identical.
class GetGroupsAndResourcesForRoleFromTokenResult : public GroupsAndResourcesWire
{
public:
    GetGroupsAndResourcesForRoleFromTokenResult() = default;

    explicit GetGroupsAndResourcesForRoleFromTokenResult(GroupsAndResourcesWire value) noexcept :
        GroupsAndResourcesWire{std::move(value)}
    {
    }
};

class CreateRoleResult : public RoleInfoWithGroupsWire
{
public:
    CreateRoleResult() = default;

    explicit CreateRoleResult(RoleInfoWithGroupsWire value) noexcept :
        RoleInfoWithGroupsWire{std::move(value)}
    {
    }
};

class CreateRoleV2Result : public RoleInfoWithParentsWire
{
public:
    CreateRoleV2Result() = default;

    explicit CreateRoleV2Result(RoleInfoWithParentsWire value) noexcept :
        RoleInfoWithParentsWire{std::move(value)}
    {
    }
};

class GetRoleResult : public RoleInfoWithGroupsWire
{
public:
    GetRoleResult() = default;

    explicit GetRoleResult(RoleInfoWithGroupsWire value) noexcept :
        RoleInfoWithGroupsWire{std::move(value)}
    {
    }
};

class GetParentInfoForRoleResult : public RoleInfoWithParentsWire
{
public:
    GetParentInfoForRoleResult() = default;

    explicit GetParentInfoForRoleResult(RoleInfoWithParentsWire value) noexcept :
        RoleInfoWithParentsWire{std::move(value)}
    {
    }
};

class UpdateRoleResult : public RoleInfoWithGroupsWire
{
public:
    UpdateRoleResult() = default;

    explicit UpdateRoleResult(RoleInfoWithGroupsWire value) noexcept :
        RoleInfoWithGroupsWire{std::move(value)}
    {
    }
};

class UpdateUserRoleResult : public StatusOkWire
{
public:
    UpdateUserRoleResult() = default;

    explicit UpdateUserRoleResult(StatusOkWire value) noexcept :
        StatusOkWire{std::move(value)}
    {
    }
};

class AcceptInvitationsV2Result : public StatusOkWire
{
public:
    AcceptInvitationsV2Result() = default;

    explicit AcceptInvitationsV2Result(StatusOkWire value) noexcept :
        StatusOkWire{std::move(value)}
    {
    }
};

class AcceptInvitationsPreAuthResult : public TokenResponseWire
{
public:
    AcceptInvitationsPreAuthResult() = default;

    AcceptInvitationsPreAuthResult(TokenResponseWire value, std::string cookie) noexcept :
        TokenResponseWire{std::move(value)},
        m_cookie{std::move(cookie)}
    {
    }

    [[nodiscard]] const std::string& get_cookie() const noexcept
    {
        return m_cookie;
    }

    void set_cookie(std::string value)
    {
        m_cookie = std::move(value);
    }

private:
    std::string m_cookie{};
};

class DeleteUserRoleResult : public app::dto::user_role::DeleteUserRoleResponse
{
public:
    DeleteUserRoleResult() = default;

    explicit DeleteUserRoleResult(app::dto::user_role::DeleteUserRoleResponse value) noexcept :
        DeleteUserRoleResponse{std::move(value)}
    {
    }
};

// Array responses carry the exact vector the old handler encoded. The handler
// serializes get_items() so the wire stays a top-level JSON array.
class GetAuthorizationInfoResult
{
public:
    GetAuthorizationInfoResult() = default;

    explicit GetAuthorizationInfoResult(std::vector<GroupInfoWire> items) noexcept :
        m_items{std::move(items)}
    {
    }

    [[nodiscard]] const std::vector<GroupInfoWire>& get_items() const noexcept
    {
        return m_items;
    }

    void set_items(std::vector<GroupInfoWire> items)
    {
        m_items = std::move(items);
    }

private:
    std::vector<GroupInfoWire> m_items{};
};

class GetRoleInformationResult
{
public:
    GetRoleInformationResult() = default;

    explicit GetRoleInformationResult(std::vector<ParentInfoWire> items) noexcept :
        m_items{std::move(items)}
    {
    }

    [[nodiscard]] const std::vector<ParentInfoWire>& get_items() const noexcept
    {
        return m_items;
    }

    void set_items(std::vector<ParentInfoWire> items)
    {
        m_items = std::move(items);
    }

private:
    std::vector<ParentInfoWire> m_items{};
};

class GetParentGroupInfoResult
{
public:
    GetParentGroupInfoResult() = default;

    explicit GetParentGroupInfoResult(std::vector<ParentGroupDescriptionWire> items) noexcept :
        m_items{std::move(items)}
    {
    }

    [[nodiscard]] const std::vector<ParentGroupDescriptionWire>& get_items() const noexcept
    {
        return m_items;
    }

    void set_items(std::vector<ParentGroupDescriptionWire> items)
    {
        m_items = std::move(items);
    }

private:
    std::vector<ParentGroupDescriptionWire> m_items{};
};

class GetRoleFromTokenResult
{
public:
    GetRoleFromTokenResult() = default;

    explicit GetRoleFromTokenResult(std::vector<std::string> items) noexcept :
        m_items{std::move(items)}
    {
    }

    [[nodiscard]] const std::vector<std::string>& get_items() const noexcept
    {
        return m_items;
    }

    void set_items(std::vector<std::string> items)
    {
        m_items = std::move(items);
    }

private:
    std::vector<std::string> m_items{};
};

class GetParentGroupsInfoForRoleFromTokenResult
{
public:
    GetParentGroupsInfoForRoleFromTokenResult() = default;

    explicit GetParentGroupsInfoForRoleFromTokenResult(
        std::vector<ParentGroupInfoWire> items
    ) noexcept :
        m_items{std::move(items)}
    {
    }

    [[nodiscard]] const std::vector<ParentGroupInfoWire>& get_items() const noexcept
    {
        return m_items;
    }

    void set_items(std::vector<ParentGroupInfoWire> items)
    {
        m_items = std::move(items);
    }

private:
    std::vector<ParentGroupInfoWire> m_items{};
};

class ListUsersInLineageResult
{
public:
    ListUsersInLineageResult() = default;

    explicit ListUsersInLineageResult(std::vector<ListUsersInEntityWire> items) noexcept :
        m_items{std::move(items)}
    {
    }

    [[nodiscard]] const std::vector<ListUsersInEntityWire>& get_items() const noexcept
    {
        return m_items;
    }

    void set_items(std::vector<ListUsersInEntityWire> items)
    {
        m_items = std::move(items);
    }

private:
    std::vector<ListUsersInEntityWire> m_items{};
};

class ListInvitableRolesAtEntityLevelResult
{
public:
    ListInvitableRolesAtEntityLevelResult() = default;

    explicit ListInvitableRolesAtEntityLevelResult(std::vector<MinimalRoleInfoWire> items) noexcept
        :
        m_items{std::move(items)}
    {
    }

    [[nodiscard]] const std::vector<MinimalRoleInfoWire>& get_items() const noexcept
    {
        return m_items;
    }

    void set_items(std::vector<MinimalRoleInfoWire> items)
    {
        m_items = std::move(items);
    }

private:
    std::vector<MinimalRoleInfoWire> m_items{};
};

class ListUpdatableRolesAtEntityLevelResult
{
public:
    ListUpdatableRolesAtEntityLevelResult() = default;

    explicit ListUpdatableRolesAtEntityLevelResult(std::vector<MinimalRoleInfoWire> items) noexcept
        :
        m_items{std::move(items)}
    {
    }

    [[nodiscard]] const std::vector<MinimalRoleInfoWire>& get_items() const noexcept
    {
        return m_items;
    }

    void set_items(std::vector<MinimalRoleInfoWire> items)
    {
        m_items = std::move(items);
    }

private:
    std::vector<MinimalRoleInfoWire> m_items{};
};

class ListInvitationsForUserResult
{
public:
    ListInvitationsForUserResult() = default;

    explicit ListInvitationsForUserResult(std::vector<InvitationWire> items) noexcept :
        m_items{std::move(items)}
    {
    }

    [[nodiscard]] const std::vector<InvitationWire>& get_items() const noexcept
    {
        return m_items;
    }

    void set_items(std::vector<InvitationWire> items)
    {
        m_items = std::move(items);
    }

private:
    std::vector<InvitationWire> m_items{};
};

// The `groups` query flag selects one of two incompatible top-level array
// shapes. The runtime builds a typed serde::Value document from the chosen
// vector; the handler serializes that document so neither shape gains an
// object wrapper and no pre-encoded JSON text crosses the boundary.
class ListRolesWithInfoResult
{
public:
    ListRolesWithInfoResult() = default;

    explicit ListRolesWithInfoResult(serde::Value document) noexcept :
        m_document{std::move(document)}
    {
    }

    [[nodiscard]] const serde::Value& get_document() const noexcept
    {
        return m_document;
    }

    void set_document(serde::Value value)
    {
        m_document = std::move(value);
    }

private:
    serde::Value m_document{};
};

} // namespace app::handlers::user_role

template<>
struct serde::Serializable<app::handlers::user_role::GetGroupsAndResourcesForRoleFromTokenResult>
{
    static constexpr auto fields()
    {
        return serde::Serializable<app::handlers::user_role::GroupsAndResourcesWire>::fields();
    }

    static constexpr std::string_view table_name()
    {
        return "get_groups_and_resources_for_role_from_token_result";
    }
};

template<>
struct serde::Serializable<app::handlers::user_role::CreateRoleResult>
{
    static constexpr auto fields()
    {
        return serde::Serializable<app::handlers::user_role::RoleInfoWithGroupsWire>::fields();
    }

    static constexpr std::string_view table_name()
    {
        return "create_role_result";
    }
};

template<>
struct serde::Serializable<app::handlers::user_role::CreateRoleV2Result>
{
    static constexpr auto fields()
    {
        return serde::Serializable<app::handlers::user_role::RoleInfoWithParentsWire>::fields();
    }

    static constexpr std::string_view table_name()
    {
        return "create_role_v2_result";
    }
};

template<>
struct serde::Serializable<app::handlers::user_role::GetRoleResult>
{
    static constexpr auto fields()
    {
        return serde::Serializable<app::handlers::user_role::RoleInfoWithGroupsWire>::fields();
    }

    static constexpr std::string_view table_name()
    {
        return "get_role_result";
    }
};

template<>
struct serde::Serializable<app::handlers::user_role::GetParentInfoForRoleResult>
{
    static constexpr auto fields()
    {
        return serde::Serializable<app::handlers::user_role::RoleInfoWithParentsWire>::fields();
    }

    static constexpr std::string_view table_name()
    {
        return "get_parent_info_for_role_result";
    }
};

template<>
struct serde::Serializable<app::handlers::user_role::UpdateRoleResult>
{
    static constexpr auto fields()
    {
        return serde::Serializable<app::handlers::user_role::RoleInfoWithGroupsWire>::fields();
    }

    static constexpr std::string_view table_name()
    {
        return "update_role_result";
    }
};

template<>
struct serde::Serializable<app::handlers::user_role::UpdateUserRoleResult>
{
    static constexpr auto fields()
    {
        return serde::Serializable<app::handlers::user_role::StatusOkWire>::fields();
    }

    static constexpr std::string_view table_name()
    {
        return "update_user_role_result";
    }
};

template<>
struct serde::Serializable<app::handlers::user_role::AcceptInvitationsV2Result>
{
    static constexpr auto fields()
    {
        return serde::Serializable<app::handlers::user_role::StatusOkWire>::fields();
    }

    static constexpr std::string_view table_name()
    {
        return "accept_invitations_v2_result";
    }
};

template<>
struct serde::Serializable<app::handlers::user_role::AcceptInvitationsPreAuthResult>
{
    static constexpr auto fields()
    {
        return serde::Serializable<app::handlers::user_role::TokenResponseWire>::fields();
    }

    static constexpr std::string_view table_name()
    {
        return "accept_invitations_pre_auth_result";
    }
};

template<>
struct serde::Serializable<app::handlers::user_role::DeleteUserRoleResult>
{
    static constexpr auto fields()
    {
        return serde::Serializable<app::dto::user_role::DeleteUserRoleResponse>::fields();
    }

    static constexpr std::string_view table_name()
    {
        return "delete_user_role_result";
    }
};
