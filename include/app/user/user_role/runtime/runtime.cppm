export module app_user:user_role_runtime;

import std;
import core_error;
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

    [[nodiscard]] std::expected<GetAuthorizationInfoResult, core::error::AppError>
    get_authorization_info(const GetAuthorizationInfoInput& input) const
    {
        return m_get_authorization_info.get_authorization_info(input);
    }

    [[nodiscard]] std::expected<GetRoleInformationResult, core::error::AppError>
    get_role_information(const GetRoleInformationInput& input) const
    {
        return m_get_role_information.get_role_information(input);
    }

    [[nodiscard]] std::expected<GetParentGroupInfoResult, core::error::AppError>
    get_parent_group_info(const GetParentGroupInfoInput& input) const
    {
        return m_get_parent_group_info.get_parent_group_info(input);
    }

    [[nodiscard]] std::expected<GetRoleFromTokenResult, core::error::AppError>
    get_role_from_token(const GetRoleFromTokenInput& input) const
    {
        return m_get_role_from_token.get_role_from_token(input);
    }

    [[nodiscard]] std::expected<GetGroupsAndResourcesForRoleFromTokenResult, core::error::AppError>
    get_groups_and_resources_for_role_from_token(
        const GetGroupsAndResourcesForRoleFromTokenInput& input
    ) const
    {
        return m_get_groups_and_resources_for_role_from_token
            .get_groups_and_resources_for_role_from_token(input);
    }

    [[nodiscard]] std::expected<GetParentGroupsInfoForRoleFromTokenResult, core::error::AppError>
    get_parent_groups_info_for_role_from_token(
        const GetParentGroupsInfoForRoleFromTokenInput& input
    ) const
    {
        return m_get_parent_groups_info_for_role_from_token
            .get_parent_groups_info_for_role_from_token(input);
    }

    [[nodiscard]] std::expected<CreateRoleResult, core::error::AppError>
    create_role(const CreateRoleInput& input) const
    {
        return m_create_role.create_role(input);
    }

    [[nodiscard]] std::expected<CreateRoleV2Result, core::error::AppError>
    create_role_v2(const CreateRoleV2Input& input) const
    {
        return m_create_role_v2.create_role_v2(input);
    }

    [[nodiscard]] std::expected<GetRoleResult, core::error::AppError>
    get_role(const GetRoleInput& input) const
    {
        return m_get_role.get_role(input);
    }

    [[nodiscard]] std::expected<GetParentInfoForRoleResult, core::error::AppError>
    get_parent_info_for_role(const GetParentInfoForRoleInput& input) const
    {
        return m_get_parent_info_for_role.get_parent_info_for_role(input);
    }

    [[nodiscard]] std::expected<UpdateRoleResult, core::error::AppError>
    update_role(const UpdateRoleInput& input) const
    {
        return m_update_role.update_role(input);
    }

    [[nodiscard]] std::expected<UpdateUserRoleResult, core::error::AppError>
    update_user_role(const UpdateUserRoleInput& input) const
    {
        return m_update_user_role.update_user_role(input);
    }

    [[nodiscard]] std::expected<AcceptInvitationsV2Result, core::error::AppError>
    accept_invitations_v2(const AcceptInvitationsV2Input& input) const
    {
        return m_accept_invitations_v2.accept_invitations_v2(input);
    }

    [[nodiscard]] std::expected<AcceptInvitationsPreAuthResult, core::error::AppError>
    accept_invitations_pre_auth(const AcceptInvitationsPreAuthInput& input) const
    {
        return m_accept_invitations_pre_auth.accept_invitations_pre_auth(input);
    }

    [[nodiscard]] std::expected<DeleteUserRoleResult, core::error::AppError>
    delete_user_role(const DeleteUserRoleInput& input) const
    {
        return m_delete_user_role.delete_user_role(input);
    }

    [[nodiscard]] std::expected<ListUsersInLineageResult, core::error::AppError>
    list_users_in_lineage(const ListUsersInLineageInput& input) const
    {
        return m_list_users_in_lineage.list_users_in_lineage(input);
    }

    [[nodiscard]] std::expected<ListRolesWithInfoResult, core::error::AppError>
    list_roles_with_info(const ListRolesWithInfoInput& input) const
    {
        return m_list_roles_with_info.list_roles_with_info(input);
    }

    [[nodiscard]] std::expected<ListInvitableRolesAtEntityLevelResult, core::error::AppError>
    list_invitable_roles_at_entity_level(const ListInvitableRolesAtEntityLevelInput& input) const
    {
        return m_list_invitable_roles_at_entity_level.list_invitable_roles_at_entity_level(input);
    }

    [[nodiscard]] std::expected<ListUpdatableRolesAtEntityLevelResult, core::error::AppError>
    list_updatable_roles_at_entity_level(const ListUpdatableRolesAtEntityLevelInput& input) const
    {
        return m_list_updatable_roles_at_entity_level.list_updatable_roles_at_entity_level(input);
    }

    [[nodiscard]] std::expected<ListInvitationsForUserResult, core::error::AppError>
    list_invitations_for_user(const ListInvitationsForUserInput& input) const
    {
        return m_list_invitations_for_user.list_invitations_for_user(input);
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
