export module app_user:user_role_input;

import std;
import interfaces;
import core_router;
import core_serde;
import utils_openapi;
import app_model;
import app_dto;
import app_context;
import core_cache;
import core_error;
import app_auth;
import app_http;
import app_crypto;
import app_metrics;
import app_store;
import app_events;
import app_keystore;

export namespace app::handlers::user_role {

// Local typed wire shapes for upstream api_models types that have no app_dto
// mirror. Upstream `user_role.rs` / `user_role/role.rs` field names are kept.
class GroupInfoWire
{
public:
    void set_group(std::string v)
    {
        group = std::move(v);
    }

    [[nodiscard]] const std::string& get_group() const noexcept
    {
        return group;
    }

    void set_description(std::string v)
    {
        description = std::move(v);
    }

    [[nodiscard]] const std::string& get_description() const noexcept
    {
        return description;
    }

    std::string group{};
    std::string description{};
};

class ParentInfoWire
{
public:
    void set_name(std::string v)
    {
        name = std::move(v);
    }

    [[nodiscard]] const std::string& get_name() const noexcept
    {
        return name;
    }

    void set_description(std::string v)
    {
        description = std::move(v);
    }

    [[nodiscard]] const std::string& get_description() const noexcept
    {
        return description;
    }

    void set_groups(std::vector<std::string> v)
    {
        groups = std::move(v);
    }

    [[nodiscard]] const std::vector<std::string>& get_groups() const noexcept
    {
        return groups;
    }

    std::string name{};
    std::string description{};
    std::vector<std::string> groups{};
};

class ParentGroupDescriptionWire
{
public:
    void set_name(std::string v)
    {
        name = std::move(v);
    }

    [[nodiscard]] const std::string& get_name() const noexcept
    {
        return name;
    }

    void set_description(std::string v)
    {
        description = std::move(v);
    }

    [[nodiscard]] const std::string& get_description() const noexcept
    {
        return description;
    }

    void set_scopes(std::vector<std::string> v)
    {
        scopes = std::move(v);
    }

    [[nodiscard]] const std::vector<std::string>& get_scopes() const noexcept
    {
        return scopes;
    }

    std::string name{};
    std::string description{};
    std::vector<std::string> scopes{};
};

class ParentGroupInfoWire
{
public:
    void set_name(std::string v)
    {
        name = std::move(v);
    }

    [[nodiscard]] const std::string& get_name() const noexcept
    {
        return name;
    }

    void set_resources(std::vector<std::string> v)
    {
        resources = std::move(v);
    }

    [[nodiscard]] const std::vector<std::string>& get_resources() const noexcept
    {
        return resources;
    }

    void set_scopes(std::vector<std::string> v)
    {
        scopes = std::move(v);
    }

    [[nodiscard]] const std::vector<std::string>& get_scopes() const noexcept
    {
        return scopes;
    }

    std::string name{};
    std::vector<std::string> resources{};
    std::vector<std::string> scopes{};
};

class ParentGroupInfoRequestWire
{
public:
    void set_name(std::string v)
    {
        name = std::move(v);
    }

    [[nodiscard]] const std::string& get_name() const noexcept
    {
        return name;
    }

    void set_scopes(std::vector<std::string> v)
    {
        scopes = std::move(v);
    }

    [[nodiscard]] const std::vector<std::string>& get_scopes() const noexcept
    {
        return scopes;
    }

    std::string name{};
    std::vector<std::string> scopes{};
};

class CreateRoleInputWire
{
public:
    void set_role_name(std::string v)
    {
        role_name = std::move(v);
    }

    [[nodiscard]] const std::string& get_role_name() const noexcept
    {
        return role_name;
    }

    void set_groups(std::vector<std::string> v)
    {
        groups = std::move(v);
    }

    [[nodiscard]] const std::vector<std::string>& get_groups() const noexcept
    {
        return groups;
    }

    void set_role_scope(std::string v)
    {
        role_scope = std::move(v);
    }

    [[nodiscard]] const std::string& get_role_scope() const noexcept
    {
        return role_scope;
    }

    void set_entity_type(std::string v)
    {
        entity_type = std::move(v);
    }

    [[nodiscard]] const std::string& get_entity_type() const noexcept
    {
        return entity_type;
    }

    void set_parent_groups(std::vector<ParentGroupInfoRequestWire> v)
    {
        parent_groups = std::move(v);
    }

    [[nodiscard]] const std::vector<ParentGroupInfoRequestWire>& get_parent_groups() const noexcept
    {
        return parent_groups;
    }

    std::string role_name{};
    std::vector<std::string> groups{};
    std::string role_scope{};
    std::string entity_type{};
    std::vector<ParentGroupInfoRequestWire> parent_groups{};
};

class UpdateRoleInputWire
{
public:
    void set_groups(std::optional<std::vector<std::string>> v)
    {
        groups = std::move(v);
    }

    [[nodiscard]] const std::optional<std::vector<std::string>>& get_groups() const noexcept
    {
        return groups;
    }

    void set_role_name(std::optional<std::string> v)
    {
        role_name = std::move(v);
    }

    [[nodiscard]] const std::optional<std::string>& get_role_name() const noexcept
    {
        return role_name;
    }

    std::optional<std::vector<std::string>> groups{};
    std::optional<std::string> role_name{};
};

class AcceptEntityWire
{
public:
    void set_entity_id(std::string v)
    {
        entity_id = std::move(v);
    }

    [[nodiscard]] const std::string& get_entity_id() const noexcept
    {
        return entity_id;
    }

    void set_entity_type(std::string v)
    {
        entity_type = std::move(v);
    }

    [[nodiscard]] const std::string& get_entity_type() const noexcept
    {
        return entity_type;
    }

    std::string entity_id{};
    std::string entity_type{};
};

class InvitationWire
{
public:
    void set_entity_id(std::string v)
    {
        entity_id = std::move(v);
    }

    [[nodiscard]] const std::string& get_entity_id() const noexcept
    {
        return entity_id;
    }

    void set_entity_type(std::string v)
    {
        entity_type = std::move(v);
    }

    [[nodiscard]] const std::string& get_entity_type() const noexcept
    {
        return entity_type;
    }

    void set_entity_name(std::optional<std::string> v)
    {
        entity_name = std::move(v);
    }

    [[nodiscard]] const std::optional<std::string>& get_entity_name() const noexcept
    {
        return entity_name;
    }

    void set_role_id(std::string v)
    {
        role_id = std::move(v);
    }

    [[nodiscard]] const std::string& get_role_id() const noexcept
    {
        return role_id;
    }

    std::string entity_id{};
    std::string entity_type{};
    std::optional<std::string> entity_name{};
    std::string role_id{};
};

class MinimalRoleInfoWire
{
public:
    void set_role_id(std::string v)
    {
        role_id = std::move(v);
    }

    [[nodiscard]] const std::string& get_role_id() const noexcept
    {
        return role_id;
    }

    void set_role_name(std::string v)
    {
        role_name = std::move(v);
    }

    [[nodiscard]] const std::string& get_role_name() const noexcept
    {
        return role_name;
    }

    std::string role_id{};
    std::string role_name{};
};

class ListUsersInEntityWire
{
public:
    void set_email(std::string v)
    {
        email = std::move(v);
    }

    [[nodiscard]] const std::string& get_email() const noexcept
    {
        return email;
    }

    void set_roles(std::vector<MinimalRoleInfoWire> v)
    {
        roles = std::move(v);
    }

    [[nodiscard]] const std::vector<MinimalRoleInfoWire>& get_roles() const noexcept
    {
        return roles;
    }

    std::string email{};
    std::vector<MinimalRoleInfoWire> roles{};
};

class RoleInfoWithGroupsWire
{
public:
    void set_role_id(std::string v)
    {
        role_id = std::move(v);
    }

    [[nodiscard]] const std::string& get_role_id() const noexcept
    {
        return role_id;
    }

    void set_groups(std::vector<std::string> v)
    {
        groups = std::move(v);
    }

    [[nodiscard]] const std::vector<std::string>& get_groups() const noexcept
    {
        return groups;
    }

    void set_role_name(std::string v)
    {
        role_name = std::move(v);
    }

    [[nodiscard]] const std::string& get_role_name() const noexcept
    {
        return role_name;
    }

    void set_role_scope(std::string v)
    {
        role_scope = std::move(v);
    }

    [[nodiscard]] const std::string& get_role_scope() const noexcept
    {
        return role_scope;
    }

    void set_entity_type(std::string v)
    {
        entity_type = std::move(v);
    }

    [[nodiscard]] const std::string& get_entity_type() const noexcept
    {
        return entity_type;
    }

    std::string role_id{};
    std::vector<std::string> groups{};
    std::string role_name{};
    std::string role_scope{};
    std::string entity_type{};
};

class RoleInfoWithParentsWire
{
public:
    void set_role_id(std::string v)
    {
        role_id = std::move(v);
    }

    [[nodiscard]] const std::string& get_role_id() const noexcept
    {
        return role_id;
    }

    void set_parent_groups(std::vector<ParentGroupDescriptionWire> v)
    {
        parent_groups = std::move(v);
    }

    [[nodiscard]] const std::vector<ParentGroupDescriptionWire>& get_parent_groups() const noexcept
    {
        return parent_groups;
    }

    void set_role_name(std::string v)
    {
        role_name = std::move(v);
    }

    [[nodiscard]] const std::string& get_role_name() const noexcept
    {
        return role_name;
    }

    void set_role_scope(std::string v)
    {
        role_scope = std::move(v);
    }

    [[nodiscard]] const std::string& get_role_scope() const noexcept
    {
        return role_scope;
    }

    std::string role_id{};
    std::vector<ParentGroupDescriptionWire> parent_groups{};
    std::string role_name{};
    std::string role_scope{};
};

class RoleInfoResponseNewWire
{
public:
    void set_role_id(std::string v)
    {
        role_id = std::move(v);
    }

    [[nodiscard]] const std::string& get_role_id() const noexcept
    {
        return role_id;
    }

    void set_role_name(std::string v)
    {
        role_name = std::move(v);
    }

    [[nodiscard]] const std::string& get_role_name() const noexcept
    {
        return role_name;
    }

    void set_entity_type(std::string v)
    {
        entity_type = std::move(v);
    }

    [[nodiscard]] const std::string& get_entity_type() const noexcept
    {
        return entity_type;
    }

    void set_groups(std::vector<std::string> v)
    {
        groups = std::move(v);
    }

    [[nodiscard]] const std::vector<std::string>& get_groups() const noexcept
    {
        return groups;
    }

    void set_scope(std::string v)
    {
        scope = std::move(v);
    }

    [[nodiscard]] const std::string& get_scope() const noexcept
    {
        return scope;
    }

    std::string role_id{};
    std::string role_name{};
    std::string entity_type{};
    std::vector<std::string> groups{};
    std::string scope{};
};

class RoleInfoResponseWithParentsGroupWire
{
public:
    void set_role_id(std::string v)
    {
        role_id = std::move(v);
    }

    [[nodiscard]] const std::string& get_role_id() const noexcept
    {
        return role_id;
    }

    void set_role_name(std::string v)
    {
        role_name = std::move(v);
    }

    [[nodiscard]] const std::string& get_role_name() const noexcept
    {
        return role_name;
    }

    void set_entity_type(std::string v)
    {
        entity_type = std::move(v);
    }

    [[nodiscard]] const std::string& get_entity_type() const noexcept
    {
        return entity_type;
    }

    void set_parent_groups(std::vector<ParentGroupDescriptionWire> v)
    {
        parent_groups = std::move(v);
    }

    [[nodiscard]] const std::vector<ParentGroupDescriptionWire>& get_parent_groups() const noexcept
    {
        return parent_groups;
    }

    void set_role_scope(std::string v)
    {
        role_scope = std::move(v);
    }

    [[nodiscard]] const std::string& get_role_scope() const noexcept
    {
        return role_scope;
    }

    std::string role_id{};
    std::string role_name{};
    std::string entity_type{};
    std::vector<ParentGroupDescriptionWire> parent_groups{};
    std::string role_scope{};
};

class GroupsAndResourcesWire
{
public:
    void set_groups(std::vector<std::string> v)
    {
        groups = std::move(v);
    }

    [[nodiscard]] const std::vector<std::string>& get_groups() const noexcept
    {
        return groups;
    }

    void set_resources(std::vector<std::string> v)
    {
        resources = std::move(v);
    }

    [[nodiscard]] const std::vector<std::string>& get_resources() const noexcept
    {
        return resources;
    }

    std::vector<std::string> groups{};
    std::vector<std::string> resources{};
};

class StatusOkWire
{
public:
    void set_status(std::string v)
    {
        status = std::move(v);
    }

    [[nodiscard]] const std::string& get_status() const noexcept
    {
        return status;
    }

    std::string status{};
};

class TokenResponseWire
{
public:
    void set_token(std::string v)
    {
        token = std::move(v);
    }

    [[nodiscard]] const std::string& get_token() const noexcept
    {
        return token;
    }

    void set_token_type(std::string v)
    {
        token_type = std::move(v);
    }

    [[nodiscard]] const std::string& get_token_type() const noexcept
    {
        return token_type;
    }

    std::string token{};
    std::string token_type{};
};

} // namespace app::handlers::user_role

template<>
struct serde::Serializable<::app::handlers::user_role::GroupInfoWire>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "group",
                &::app::handlers::user_role::GroupInfoWire::get_group,
                &::app::handlers::user_role::GroupInfoWire::set_group,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "description",
                &::app::handlers::user_role::GroupInfoWire::get_description,
                &::app::handlers::user_role::GroupInfoWire::set_description,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "group_info_wire";
    }
};

template<>
struct serde::Serializable<::app::handlers::user_role::ParentInfoWire>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "name",
                &::app::handlers::user_role::ParentInfoWire::get_name,
                &::app::handlers::user_role::ParentInfoWire::set_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "description",
                &::app::handlers::user_role::ParentInfoWire::get_description,
                &::app::handlers::user_role::ParentInfoWire::set_description,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "groups",
                &::app::handlers::user_role::ParentInfoWire::get_groups,
                &::app::handlers::user_role::ParentInfoWire::set_groups,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "parent_info_wire";
    }
};

template<>
struct serde::Serializable<::app::handlers::user_role::ParentGroupDescriptionWire>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "name",
                &::app::handlers::user_role::ParentGroupDescriptionWire::get_name,
                &::app::handlers::user_role::ParentGroupDescriptionWire::set_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "description",
                &::app::handlers::user_role::ParentGroupDescriptionWire::get_description,
                &::app::handlers::user_role::ParentGroupDescriptionWire::set_description,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "scopes",
                &::app::handlers::user_role::ParentGroupDescriptionWire::get_scopes,
                &::app::handlers::user_role::ParentGroupDescriptionWire::set_scopes,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "parent_group_description_wire";
    }
};

template<>
struct serde::Serializable<::app::handlers::user_role::ParentGroupInfoWire>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "name",
                &::app::handlers::user_role::ParentGroupInfoWire::get_name,
                &::app::handlers::user_role::ParentGroupInfoWire::set_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "resources",
                &::app::handlers::user_role::ParentGroupInfoWire::get_resources,
                &::app::handlers::user_role::ParentGroupInfoWire::set_resources,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "scopes",
                &::app::handlers::user_role::ParentGroupInfoWire::get_scopes,
                &::app::handlers::user_role::ParentGroupInfoWire::set_scopes,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "parent_group_info_wire";
    }
};

template<>
struct serde::Serializable<::app::handlers::user_role::ParentGroupInfoRequestWire>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "name",
                &::app::handlers::user_role::ParentGroupInfoRequestWire::get_name,
                &::app::handlers::user_role::ParentGroupInfoRequestWire::set_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "scopes",
                &::app::handlers::user_role::ParentGroupInfoRequestWire::get_scopes,
                &::app::handlers::user_role::ParentGroupInfoRequestWire::set_scopes,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "parent_group_info_request_wire";
    }
};

template<>
struct serde::Serializable<::app::handlers::user_role::CreateRoleInputWire>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "role_name",
                &::app::handlers::user_role::CreateRoleInputWire::get_role_name,
                &::app::handlers::user_role::CreateRoleInputWire::set_role_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "groups",
                &::app::handlers::user_role::CreateRoleInputWire::get_groups,
                &::app::handlers::user_role::CreateRoleInputWire::set_groups,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "role_scope",
                &::app::handlers::user_role::CreateRoleInputWire::get_role_scope,
                &::app::handlers::user_role::CreateRoleInputWire::set_role_scope,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "entity_type",
                &::app::handlers::user_role::CreateRoleInputWire::get_entity_type,
                &::app::handlers::user_role::CreateRoleInputWire::set_entity_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "parent_groups",
                &::app::handlers::user_role::CreateRoleInputWire::get_parent_groups,
                &::app::handlers::user_role::CreateRoleInputWire::set_parent_groups,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "create_role_input_wire";
    }
};

template<>
struct serde::Serializable<::app::handlers::user_role::UpdateRoleInputWire>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "groups",
                &::app::handlers::user_role::UpdateRoleInputWire::get_groups,
                &::app::handlers::user_role::UpdateRoleInputWire::set_groups,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "role_name",
                &::app::handlers::user_role::UpdateRoleInputWire::get_role_name,
                &::app::handlers::user_role::UpdateRoleInputWire::set_role_name,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "update_role_input_wire";
    }
};

template<>
struct serde::Serializable<::app::handlers::user_role::AcceptEntityWire>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "entity_id",
                &::app::handlers::user_role::AcceptEntityWire::get_entity_id,
                &::app::handlers::user_role::AcceptEntityWire::set_entity_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "entity_type",
                &::app::handlers::user_role::AcceptEntityWire::get_entity_type,
                &::app::handlers::user_role::AcceptEntityWire::set_entity_type,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "accept_entity_wire";
    }
};

template<>
struct serde::Serializable<::app::handlers::user_role::InvitationWire>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "entity_id",
                &::app::handlers::user_role::InvitationWire::get_entity_id,
                &::app::handlers::user_role::InvitationWire::set_entity_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "entity_type",
                &::app::handlers::user_role::InvitationWire::get_entity_type,
                &::app::handlers::user_role::InvitationWire::set_entity_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "entity_name",
                &::app::handlers::user_role::InvitationWire::get_entity_name,
                &::app::handlers::user_role::InvitationWire::set_entity_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "role_id",
                &::app::handlers::user_role::InvitationWire::get_role_id,
                &::app::handlers::user_role::InvitationWire::set_role_id,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "invitation_wire";
    }
};

template<>
struct serde::Serializable<::app::handlers::user_role::MinimalRoleInfoWire>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "role_id",
                &::app::handlers::user_role::MinimalRoleInfoWire::get_role_id,
                &::app::handlers::user_role::MinimalRoleInfoWire::set_role_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "role_name",
                &::app::handlers::user_role::MinimalRoleInfoWire::get_role_name,
                &::app::handlers::user_role::MinimalRoleInfoWire::set_role_name,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "minimal_role_info_wire";
    }
};

template<>
struct serde::Serializable<::app::handlers::user_role::ListUsersInEntityWire>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "email",
                &::app::handlers::user_role::ListUsersInEntityWire::get_email,
                &::app::handlers::user_role::ListUsersInEntityWire::set_email,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "roles",
                &::app::handlers::user_role::ListUsersInEntityWire::get_roles,
                &::app::handlers::user_role::ListUsersInEntityWire::set_roles,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "list_users_in_entity_wire";
    }
};

template<>
struct serde::Serializable<::app::handlers::user_role::RoleInfoWithGroupsWire>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "role_id",
                &::app::handlers::user_role::RoleInfoWithGroupsWire::get_role_id,
                &::app::handlers::user_role::RoleInfoWithGroupsWire::set_role_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "groups",
                &::app::handlers::user_role::RoleInfoWithGroupsWire::get_groups,
                &::app::handlers::user_role::RoleInfoWithGroupsWire::set_groups,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "role_name",
                &::app::handlers::user_role::RoleInfoWithGroupsWire::get_role_name,
                &::app::handlers::user_role::RoleInfoWithGroupsWire::set_role_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "role_scope",
                &::app::handlers::user_role::RoleInfoWithGroupsWire::get_role_scope,
                &::app::handlers::user_role::RoleInfoWithGroupsWire::set_role_scope,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "entity_type",
                &::app::handlers::user_role::RoleInfoWithGroupsWire::get_entity_type,
                &::app::handlers::user_role::RoleInfoWithGroupsWire::set_entity_type,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "role_info_with_groups_wire";
    }
};

template<>
struct serde::Serializable<::app::handlers::user_role::RoleInfoWithParentsWire>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "role_id",
                &::app::handlers::user_role::RoleInfoWithParentsWire::get_role_id,
                &::app::handlers::user_role::RoleInfoWithParentsWire::set_role_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "parent_groups",
                &::app::handlers::user_role::RoleInfoWithParentsWire::get_parent_groups,
                &::app::handlers::user_role::RoleInfoWithParentsWire::set_parent_groups,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "role_name",
                &::app::handlers::user_role::RoleInfoWithParentsWire::get_role_name,
                &::app::handlers::user_role::RoleInfoWithParentsWire::set_role_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "role_scope",
                &::app::handlers::user_role::RoleInfoWithParentsWire::get_role_scope,
                &::app::handlers::user_role::RoleInfoWithParentsWire::set_role_scope,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "role_info_with_parents_wire";
    }
};

template<>
struct serde::Serializable<::app::handlers::user_role::RoleInfoResponseNewWire>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "role_id",
                &::app::handlers::user_role::RoleInfoResponseNewWire::get_role_id,
                &::app::handlers::user_role::RoleInfoResponseNewWire::set_role_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "role_name",
                &::app::handlers::user_role::RoleInfoResponseNewWire::get_role_name,
                &::app::handlers::user_role::RoleInfoResponseNewWire::set_role_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "entity_type",
                &::app::handlers::user_role::RoleInfoResponseNewWire::get_entity_type,
                &::app::handlers::user_role::RoleInfoResponseNewWire::set_entity_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "groups",
                &::app::handlers::user_role::RoleInfoResponseNewWire::get_groups,
                &::app::handlers::user_role::RoleInfoResponseNewWire::set_groups,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "scope",
                &::app::handlers::user_role::RoleInfoResponseNewWire::get_scope,
                &::app::handlers::user_role::RoleInfoResponseNewWire::set_scope,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "role_info_response_new_wire";
    }
};

template<>
struct serde::Serializable<::app::handlers::user_role::RoleInfoResponseWithParentsGroupWire>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "role_id",
                &::app::handlers::user_role::RoleInfoResponseWithParentsGroupWire::get_role_id,
                &::app::handlers::user_role::RoleInfoResponseWithParentsGroupWire::set_role_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "role_name",
                &::app::handlers::user_role::RoleInfoResponseWithParentsGroupWire::get_role_name,
                &::app::handlers::user_role::RoleInfoResponseWithParentsGroupWire::set_role_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "entity_type",
                &::app::handlers::user_role::RoleInfoResponseWithParentsGroupWire::get_entity_type,
                &::app::handlers::user_role::RoleInfoResponseWithParentsGroupWire::set_entity_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "parent_groups",
                &::app::handlers::user_role::RoleInfoResponseWithParentsGroupWire::
                    get_parent_groups,
                &::app::handlers::user_role::RoleInfoResponseWithParentsGroupWire::
                    set_parent_groups,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "role_scope",
                &::app::handlers::user_role::RoleInfoResponseWithParentsGroupWire::get_role_scope,
                &::app::handlers::user_role::RoleInfoResponseWithParentsGroupWire::set_role_scope,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "role_info_response_with_parents_group_wire";
    }
};

template<>
struct serde::Serializable<::app::handlers::user_role::GroupsAndResourcesWire>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "groups",
                &::app::handlers::user_role::GroupsAndResourcesWire::get_groups,
                &::app::handlers::user_role::GroupsAndResourcesWire::set_groups,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "resources",
                &::app::handlers::user_role::GroupsAndResourcesWire::get_resources,
                &::app::handlers::user_role::GroupsAndResourcesWire::set_resources,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "groups_and_resources_wire";
    }
};

template<>
struct serde::Serializable<::app::handlers::user_role::StatusOkWire>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<
            "status",
            &::app::handlers::user_role::StatusOkWire::get_status,
            &::app::handlers::user_role::StatusOkWire::set_status,
            serde::FieldOptions{}>{}};
    }

    static constexpr std::string_view table_name()
    {
        return "status_ok_wire";
    }
};

template<>
struct serde::Serializable<::app::handlers::user_role::TokenResponseWire>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "token",
                &::app::handlers::user_role::TokenResponseWire::get_token,
                &::app::handlers::user_role::TokenResponseWire::set_token,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "token_type",
                &::app::handlers::user_role::TokenResponseWire::get_token_type,
                &::app::handlers::user_role::TokenResponseWire::set_token_type,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "token_response_wire";
    }
};

export namespace app::handlers::user_role {

// Request-derived identity and lineage records (previously private to the facade).
struct RequestIdentity
{
    std::string merchant_id{};
    std::string org_id{};
    std::string profile_id{};
    std::string user_id{};
    std::string role_id{};
    std::string tenant_id{};
};

struct Lineage
{
    std::string org_id{};
    std::string merchant_id{};
    std::string profile_id{};
};

/**
 * @brief Parse/auth/path/query support for the user_role handlers.
 *
 * Turns request data into plain Input structs; business logic stays in the runtime.
 */
class UserRoleInputSupport
{
public:
    explicit UserRoleInputSupport(::app::context::AppContext& context) noexcept :
        m_context{context}
    {
    }

    [[nodiscard]] core::error::AppError
    error(core::error::AppErrorKind kind, std::string message) const
    {
        return core::error::AppErrorBuilder{}.operator()(kind, std::move(message));
    }

    // Ported stand-in for upstream `auth::decode_jwt` claim access (user.cppm:3753-3787).
    [[nodiscard]] std::string jwt_claim(std::string_view token, std::string_view key) const
    {
        const auto first = token.find('.');
        if (first == std::string_view::npos) {
            return {};
        }
        const auto second = token.find('.', first + 1);
        if (second == std::string_view::npos) {
            return {};
        }
        const std::string payload =
            ::app::crypto::Base64Url{}.decode(token.substr(first + 1, second - first - 1));
        const std::string needle = std::string{"\""} + std::string{key} + "\"";
        const auto key_pos = payload.find(needle);
        if (key_pos == std::string::npos) {
            return {};
        }
        auto pos = payload.find(':', key_pos + needle.size());
        if (pos == std::string::npos) {
            return {};
        }
        ++pos;
        while (pos < payload.size() && payload[pos] == ' ') {
            ++pos;
        }
        if (pos >= payload.size() || payload[pos] != '"') {
            return {};
        }
        ++pos;
        const auto end = payload.find('"', pos);
        if (end == std::string::npos) {
            return {};
        }
        return payload.substr(pos, end - pos);
    }

    [[nodiscard]] RequestIdentity identity_from(interfaces::io::IRequest& req) const
    {
        const auto auth = ::app::auth::AuthContextBuilder{}(req);
        RequestIdentity out{};
        out.merchant_id = ::app::http::merchant_id_from_header{}(req);
        out.org_id = ::app::http::org_id_from_header{}(req);
        if (out.merchant_id.empty()) {
            out.merchant_id = auth.merchant_id();
        }
        if (out.org_id.empty()) {
            out.org_id = auth.org_id();
        }
        out.profile_id = auth.profile_id();

        const auto token = auth.token().value();
        out.user_id = jwt_claim(token, "user_id");
        if (out.user_id.empty()) {
            out.user_id = jwt_claim(token, "sub");
        }
        out.role_id = jwt_claim(token, "role_id");
        out.tenant_id = jwt_claim(token, "tenant_id");

        if (out.user_id.empty() && !out.merchant_id.empty()) {
            for (const auto& assignment: m_context.repository().list_user_roles()) {
                if (assignment.get_merchant_id() == out.merchant_id &&
                    assignment.get_status() == "active") {
                    out.user_id = assignment.get_user_id();
                    if (out.role_id.empty()) {
                        out.role_id = assignment.get_role_id();
                    }
                    break;
                }
            }
        }
        if (out.role_id.empty() && !out.user_id.empty()) {
            for (const auto& assignment: m_context.repository().list_user_roles()) {
                if (assignment.get_user_id() == out.user_id) {
                    out.role_id = assignment.get_role_id();
                    break;
                }
            }
        }
        return out;
    }

    [[nodiscard]] std::optional<core::error::AppError>
    authorize(interfaces::io::IRequest& req, std::string_view operation) const
    {
        const auto rule = m_context.auth_rules().rule_for(operation);
        return m_context.authorizer().authorize_route(::app::auth::AuthContextBuilder{}(req), rule);
    }

    [[nodiscard]] std::string normalize_entity(std::string_view value) const
    {
        if (value == "tenant" || value == "organization" || value == "merchant" ||
            value == "profile") {
            return std::string{value};
        }
        return {};
    }

    [[nodiscard]] std::string query_value(interfaces::io::IRequest& req, std::string_view key) const
    {
        const auto query = ::app::http::RequestQuery{}(req);
        return query.get(key).value_or("");
    }

    [[nodiscard]] std::optional<std::string> role_id_from_path(interfaces::io::IRequest& req) const
    {
        const auto segments = ::app::http::RequestPath{}(req);
        for (std::size_t i{0}; i < segments.count(); ++i) {
            if (segments.at(i).value_or(std::string_view{}) == "role") {
                const auto candidate = segments.at(i + 1);
                if (candidate.has_value() && !candidate->empty()) {
                    return std::string{*candidate};
                }
            }
        }
        return std::nullopt;
    }

private:
    ::app::context::AppContext& m_context;
};

// ───────────────────────── Plain inputs ─────────────────────────
// Each struct carries exactly what the old handler read from the request.

struct GetAuthorizationInfoInput
{
    RequestIdentity identity{};
};

struct GetRoleInformationInput
{
    RequestIdentity identity{};
};

struct GetParentGroupInfoInput
{
    RequestIdentity identity{};
    std::string entity_type{};
    std::string product_type{};
};

struct GetRoleFromTokenInput
{
    RequestIdentity identity{};
};

struct GetGroupsAndResourcesForRoleFromTokenInput
{
    RequestIdentity identity{};
};

struct GetParentGroupsInfoForRoleFromTokenInput
{
    RequestIdentity identity{};
};

struct CreateRoleInput
{
    RequestIdentity identity{};
    CreateRoleInputWire request{};
};

struct CreateRoleV2Input
{
    RequestIdentity identity{};
    CreateRoleInputWire request{};
};

struct GetRoleInput
{
    RequestIdentity identity{};
    std::string role_id{};
};

struct GetParentInfoForRoleInput
{
    RequestIdentity identity{};
    std::string role_id{};
};

struct UpdateRoleInput
{
    RequestIdentity identity{};
    std::string role_id{};
    UpdateRoleInputWire request{};
};

struct UpdateUserRoleInput
{
    RequestIdentity identity{};
    ::app::dto::user_role::UpdateUserRoleRequest request{};
};

struct AcceptInvitationsV2Input
{
    RequestIdentity identity{};
    std::vector<AcceptEntityWire> request{};
};

struct AcceptInvitationsPreAuthInput
{
    RequestIdentity identity{};
    std::vector<AcceptEntityWire> request{};
};

struct DeleteUserRoleInput
{
    RequestIdentity identity{};
    ::app::dto::user_role::DeleteUserRoleRequest request{};
};

struct ListUsersInLineageInput
{
    RequestIdentity identity{};
    std::string entity_type{};
};

struct ListRolesWithInfoInput
{
    RequestIdentity identity{};
    std::string entity_type{};
    bool groups{false};
};

struct ListInvitableRolesAtEntityLevelInput
{
    RequestIdentity identity{};
    std::string entity_type{};
};

struct ListUpdatableRolesAtEntityLevelInput
{
    RequestIdentity identity{};
    std::string entity_type{};
};

struct ListInvitationsForUserInput
{
    RequestIdentity identity{};
};

} // namespace app::handlers::user_role
