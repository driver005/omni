export module app_user:user_role_runtime;

import std;
import core_behavior;
import core_error;
import core_serde;
import app_context;

import :user_role_input;
import :user_role_result;
import :user_role_runtime_common;
import :user_role_runtime_get_authorization_info;
import :user_role_runtime_get_role_information;
import :user_role_runtime_get_parent_group_info;
import :user_role_runtime_get_role_from_token;
import :user_role_runtime_get_groups_and_resources_for_role_from_token;
import :user_role_runtime_get_parent_groups_info_for_role_from_token;
import :user_role_runtime_create_role;
import :user_role_runtime_create_role_v2;
import :user_role_runtime_get_role;
import :user_role_runtime_get_parent_info_for_role;
import :user_role_runtime_update_role;
import :user_role_runtime_update_user_role;
import :user_role_runtime_accept_invitations_v2;
import :user_role_runtime_accept_invitations_pre_auth;
import :user_role_runtime_delete_user_role;
import :user_role_runtime_list_users_in_lineage;
import :user_role_runtime_list_roles_with_info;
import :user_role_runtime_list_invitable_roles_at_entity_level;
import :user_role_runtime_list_updatable_roles_at_entity_level;
import :user_role_runtime_list_invitations_for_user;

export namespace app::handlers::user_role {

class UserRoleRuntime
{
public:
    explicit UserRoleRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_get_authorization_info{context},
        m_get_role_information{context},
        m_get_parent_group_info{context},
        m_get_role_from_token{context},
        m_get_groups_and_resources_for_role_from_token{context},
        m_get_parent_groups_info_for_role_from_token{context},
        m_create_role{context},
        m_create_role_v2{context},
        m_get_role{context},
        m_get_parent_info_for_role{context},
        m_update_role{context},
        m_update_user_role{context},
        m_accept_invitations_v2{context},
        m_accept_invitations_pre_auth{context},
        m_delete_user_role{context},
        m_list_users_in_lineage{context},
        m_list_roles_with_info{context},
        m_list_invitable_roles_at_entity_level{context},
        m_list_updatable_roles_at_entity_level{context},
        m_list_invitations_for_user{context}
    {
    }

    [[nodiscard]] core::behavior::Outcome<serde::Value>
    get_authorization_info(const ::core::behavior::CallContext& ctx) const
    {
        return m_get_authorization_info.get_authorization_info(ctx);
    }

    [[nodiscard]] core::behavior::Outcome<serde::Value>
    get_role_information(const ::core::behavior::CallContext& ctx) const
    {
        return m_get_role_information.get_role_information(ctx);
    }

    [[nodiscard]] core::behavior::Outcome<serde::Value>
    get_parent_group_info(const ::core::behavior::CallContext& ctx) const
    {
        return m_get_parent_group_info.get_parent_group_info(ctx);
    }

    [[nodiscard]] core::behavior::Outcome<serde::Value>
    get_role_from_token(const ::core::behavior::CallContext& ctx) const
    {
        return m_get_role_from_token.get_role_from_token(ctx);
    }

    [[nodiscard]] core::behavior::Outcome<GroupsAndResourcesWire>
    get_groups_and_resources_for_role_from_token(
        const ::core::behavior::CallContext& ctx
    ) const
    {
        return m_get_groups_and_resources_for_role_from_token
            .get_groups_and_resources_for_role_from_token(request, ctx);
    }

    [[nodiscard]] core::behavior::Outcome<serde::Value>
    get_parent_groups_info_for_role_from_token(
        const ::core::behavior::CallContext& ctx
    ) const
    {
        return m_get_parent_groups_info_for_role_from_token
            .get_parent_groups_info_for_role_from_token(request, ctx);
    }

    [[nodiscard]] core::behavior::Outcome<RoleInfoWithGroupsWire>
    create_role(const CreateRoleInputWire& request,
        const ::core::behavior::CallContext& ctx) const
    {
        return m_create_role.create_role(request, ctx);
    }

    [[nodiscard]] core::behavior::Outcome<RoleInfoWithParentsWire>
    create_role_v2(const CreateRoleInputWire& request,
        const ::core::behavior::CallContext& ctx) const
    {
        return m_create_role_v2.create_role_v2(request, ctx);
    }

    [[nodiscard]] core::behavior::Outcome<RoleInfoWithGroupsWire>
    get_role(const ::core::behavior::CallContext& ctx) const
    {
        return m_get_role.get_role(ctx);
    }

    [[nodiscard]] core::behavior::Outcome<RoleInfoWithParentsWire>
    get_parent_info_for_role(const ::core::behavior::CallContext& ctx) const
    {
        return m_get_parent_info_for_role.get_parent_info_for_role(ctx);
    }

    [[nodiscard]] core::behavior::Outcome<RoleInfoWithGroupsWire>
    update_role(const UpdateRoleInputWire& request,
        const ::core::behavior::CallContext& ctx) const
    {
        return m_update_role.update_role(request, ctx);
    }

    [[nodiscard]] core::behavior::Outcome<StatusOkWire>
    update_user_role(const ::app::dto::user_role::UpdateUserRoleRequest& request,
        const ::core::behavior::CallContext& ctx) const
    {
        return m_update_user_role.update_user_role(request, ctx);
    }

    [[nodiscard]] core::behavior::Outcome<StatusOkWire>
    accept_invitations_v2(const std::vector<AcceptEntityWire>& request,
        const ::core::behavior::CallContext& ctx) const
    {
        return m_accept_invitations_v2.accept_invitations_v2(request, ctx);
    }

    [[nodiscard]] core::behavior::Outcome<std::pair<TokenResponseWire, std::string>>
    accept_invitations_pre_auth(const std::vector<AcceptEntityWire>& request,
        const ::core::behavior::CallContext& ctx) const
    {
        return m_accept_invitations_pre_auth.accept_invitations_pre_auth(request, ctx);
    }

    [[nodiscard]] core::behavior::Outcome<::app::dto::user_role::DeleteUserRoleResponse>
        delete_user_role(const ::app::dto::user_role::DeleteUserRoleRequest& request,
        const ::core::behavior::CallContext& ctx) const
    {
        return m_delete_user_role.delete_user_role(request, ctx);
    }

    [[nodiscard]] core::behavior::Outcome<serde::Value>
    list_users_in_lineage(const ::core::behavior::CallContext& ctx) const
    {
        return m_list_users_in_lineage.list_users_in_lineage(ctx);
    }

    [[nodiscard]] core::behavior::Outcome<serde::Value>
    list_roles_with_info(const ::core::behavior::CallContext& ctx) const
    {
        return m_list_roles_with_info.list_roles_with_info(ctx);
    }

    [[nodiscard]] core::behavior::Outcome<serde::Value>
    list_invitable_roles_at_entity_level(const ::core::behavior::CallContext& ctx) const
    {
        return m_list_invitable_roles_at_entity_level.list_invitable_roles_at_entity_level(ctx);
    }

    [[nodiscard]] core::behavior::Outcome<serde::Value>
    list_updatable_roles_at_entity_level(const ::core::behavior::CallContext& ctx) const
    {
        return m_list_updatable_roles_at_entity_level.list_updatable_roles_at_entity_level(ctx);
    }

    [[nodiscard]] core::behavior::Outcome<serde::Value>
    list_invitations_for_user(const ::core::behavior::CallContext& ctx) const
    {
        return m_list_invitations_for_user.list_invitations_for_user(ctx);
    }

private:
    ::app::context::AppContext& m_context;
    GetAuthorizationInfoRuntime m_get_authorization_info;
    GetRoleInformationRuntime m_get_role_information;
    GetParentGroupInfoRuntime m_get_parent_group_info;
    GetRoleFromTokenRuntime m_get_role_from_token;
    GetGroupsAndResourcesForRoleFromTokenRuntime m_get_groups_and_resources_for_role_from_token;
    GetParentGroupsInfoForRoleFromTokenRuntime m_get_parent_groups_info_for_role_from_token;
    CreateRoleRuntime m_create_role;
    CreateRoleV2Runtime m_create_role_v2;
    GetRoleRuntime m_get_role;
    GetParentInfoForRoleRuntime m_get_parent_info_for_role;
    UpdateRoleRuntime m_update_role;
    UpdateUserRoleRuntime m_update_user_role;
    AcceptInvitationsV2Runtime m_accept_invitations_v2;
    AcceptInvitationsPreAuthRuntime m_accept_invitations_pre_auth;
    DeleteUserRoleRuntime m_delete_user_role;
    ListUsersInLineageRuntime m_list_users_in_lineage;
    ListRolesWithInfoRuntime m_list_roles_with_info;
    ListInvitableRolesAtEntityLevelRuntime m_list_invitable_roles_at_entity_level;
    ListUpdatableRolesAtEntityLevelRuntime m_list_updatable_roles_at_entity_level;
    ListInvitationsForUserRuntime m_list_invitations_for_user;
};

} // namespace app::handlers::user_role
