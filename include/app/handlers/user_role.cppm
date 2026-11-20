module;
export module app_handlers:user_role;

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
    void set_group(std::string v) { group = std::move(v); }
    [[nodiscard]] const std::string& get_group() const noexcept { return group; }
    void set_description(std::string v) { description = std::move(v); }
    [[nodiscard]] const std::string& get_description() const noexcept { return description; }
    std::string group{};
    std::string description{};
};

class ParentInfoWire
{
public:
    void set_name(std::string v) { name = std::move(v); }
    [[nodiscard]] const std::string& get_name() const noexcept { return name; }
    void set_description(std::string v) { description = std::move(v); }
    [[nodiscard]] const std::string& get_description() const noexcept { return description; }
    void set_groups(std::vector<std::string> v) { groups = std::move(v); }
    [[nodiscard]] const std::vector<std::string>& get_groups() const noexcept { return groups; }
    std::string name{};
    std::string description{};
    std::vector<std::string> groups{};
};

class ParentGroupDescriptionWire
{
public:
    void set_name(std::string v) { name = std::move(v); }
    [[nodiscard]] const std::string& get_name() const noexcept { return name; }
    void set_description(std::string v) { description = std::move(v); }
    [[nodiscard]] const std::string& get_description() const noexcept { return description; }
    void set_scopes(std::vector<std::string> v) { scopes = std::move(v); }
    [[nodiscard]] const std::vector<std::string>& get_scopes() const noexcept { return scopes; }
    std::string name{};
    std::string description{};
    std::vector<std::string> scopes{};
};

class ParentGroupInfoWire
{
public:
    void set_name(std::string v) { name = std::move(v); }
    [[nodiscard]] const std::string& get_name() const noexcept { return name; }
    void set_resources(std::vector<std::string> v) { resources = std::move(v); }
    [[nodiscard]] const std::vector<std::string>& get_resources() const noexcept { return resources; }
    void set_scopes(std::vector<std::string> v) { scopes = std::move(v); }
    [[nodiscard]] const std::vector<std::string>& get_scopes() const noexcept { return scopes; }
    std::string name{};
    std::vector<std::string> resources{};
    std::vector<std::string> scopes{};
};

class ParentGroupInfoRequestWire
{
public:
    void set_name(std::string v) { name = std::move(v); }
    [[nodiscard]] const std::string& get_name() const noexcept { return name; }
    void set_scopes(std::vector<std::string> v) { scopes = std::move(v); }
    [[nodiscard]] const std::vector<std::string>& get_scopes() const noexcept { return scopes; }
    std::string name{};
    std::vector<std::string> scopes{};
};

class CreateRoleInputWire
{
public:
    void set_role_name(std::string v) { role_name = std::move(v); }
    [[nodiscard]] const std::string& get_role_name() const noexcept { return role_name; }
    void set_groups(std::vector<std::string> v) { groups = std::move(v); }
    [[nodiscard]] const std::vector<std::string>& get_groups() const noexcept { return groups; }
    void set_role_scope(std::string v) { role_scope = std::move(v); }
    [[nodiscard]] const std::string& get_role_scope() const noexcept { return role_scope; }
    void set_entity_type(std::string v) { entity_type = std::move(v); }
    [[nodiscard]] const std::string& get_entity_type() const noexcept { return entity_type; }
    void set_parent_groups(std::vector<ParentGroupInfoRequestWire> v) { parent_groups = std::move(v); }
    [[nodiscard]] const std::vector<ParentGroupInfoRequestWire>& get_parent_groups() const noexcept { return parent_groups; }
    std::string role_name{};
    std::vector<std::string> groups{};
    std::string role_scope{};
    std::string entity_type{};
    std::vector<ParentGroupInfoRequestWire> parent_groups{};
};

class UpdateRoleInputWire
{
public:
    void set_groups(std::optional<std::vector<std::string>> v) { groups = std::move(v); }
    [[nodiscard]] const std::optional<std::vector<std::string>>& get_groups() const noexcept { return groups; }
    void set_role_name(std::optional<std::string> v) { role_name = std::move(v); }
    [[nodiscard]] const std::optional<std::string>& get_role_name() const noexcept { return role_name; }
    std::optional<std::vector<std::string>> groups{};
    std::optional<std::string> role_name{};
};

class AcceptEntityWire
{
public:
    void set_entity_id(std::string v) { entity_id = std::move(v); }
    [[nodiscard]] const std::string& get_entity_id() const noexcept { return entity_id; }
    void set_entity_type(std::string v) { entity_type = std::move(v); }
    [[nodiscard]] const std::string& get_entity_type() const noexcept { return entity_type; }
    std::string entity_id{};
    std::string entity_type{};
};

class InvitationWire
{
public:
    void set_entity_id(std::string v) { entity_id = std::move(v); }
    [[nodiscard]] const std::string& get_entity_id() const noexcept { return entity_id; }
    void set_entity_type(std::string v) { entity_type = std::move(v); }
    [[nodiscard]] const std::string& get_entity_type() const noexcept { return entity_type; }
    void set_entity_name(std::optional<std::string> v) { entity_name = std::move(v); }
    [[nodiscard]] const std::optional<std::string>& get_entity_name() const noexcept { return entity_name; }
    void set_role_id(std::string v) { role_id = std::move(v); }
    [[nodiscard]] const std::string& get_role_id() const noexcept { return role_id; }
    std::string entity_id{};
    std::string entity_type{};
    std::optional<std::string> entity_name{};
    std::string role_id{};
};

class MinimalRoleInfoWire
{
public:
    void set_role_id(std::string v) { role_id = std::move(v); }
    [[nodiscard]] const std::string& get_role_id() const noexcept { return role_id; }
    void set_role_name(std::string v) { role_name = std::move(v); }
    [[nodiscard]] const std::string& get_role_name() const noexcept { return role_name; }
    std::string role_id{};
    std::string role_name{};
};

class ListUsersInEntityWire
{
public:
    void set_email(std::string v) { email = std::move(v); }
    [[nodiscard]] const std::string& get_email() const noexcept { return email; }
    void set_roles(std::vector<MinimalRoleInfoWire> v) { roles = std::move(v); }
    [[nodiscard]] const std::vector<MinimalRoleInfoWire>& get_roles() const noexcept { return roles; }
    std::string email{};
    std::vector<MinimalRoleInfoWire> roles{};
};

class RoleInfoWithGroupsWire
{
public:
    void set_role_id(std::string v) { role_id = std::move(v); }
    [[nodiscard]] const std::string& get_role_id() const noexcept { return role_id; }
    void set_groups(std::vector<std::string> v) { groups = std::move(v); }
    [[nodiscard]] const std::vector<std::string>& get_groups() const noexcept { return groups; }
    void set_role_name(std::string v) { role_name = std::move(v); }
    [[nodiscard]] const std::string& get_role_name() const noexcept { return role_name; }
    void set_role_scope(std::string v) { role_scope = std::move(v); }
    [[nodiscard]] const std::string& get_role_scope() const noexcept { return role_scope; }
    void set_entity_type(std::string v) { entity_type = std::move(v); }
    [[nodiscard]] const std::string& get_entity_type() const noexcept { return entity_type; }
    std::string role_id{};
    std::vector<std::string> groups{};
    std::string role_name{};
    std::string role_scope{};
    std::string entity_type{};
};

class RoleInfoWithParentsWire
{
public:
    void set_role_id(std::string v) { role_id = std::move(v); }
    [[nodiscard]] const std::string& get_role_id() const noexcept { return role_id; }
    void set_parent_groups(std::vector<ParentGroupDescriptionWire> v) { parent_groups = std::move(v); }
    [[nodiscard]] const std::vector<ParentGroupDescriptionWire>& get_parent_groups() const noexcept { return parent_groups; }
    void set_role_name(std::string v) { role_name = std::move(v); }
    [[nodiscard]] const std::string& get_role_name() const noexcept { return role_name; }
    void set_role_scope(std::string v) { role_scope = std::move(v); }
    [[nodiscard]] const std::string& get_role_scope() const noexcept { return role_scope; }
    std::string role_id{};
    std::vector<ParentGroupDescriptionWire> parent_groups{};
    std::string role_name{};
    std::string role_scope{};
};

class RoleInfoResponseNewWire
{
public:
    void set_role_id(std::string v) { role_id = std::move(v); }
    [[nodiscard]] const std::string& get_role_id() const noexcept { return role_id; }
    void set_role_name(std::string v) { role_name = std::move(v); }
    [[nodiscard]] const std::string& get_role_name() const noexcept { return role_name; }
    void set_entity_type(std::string v) { entity_type = std::move(v); }
    [[nodiscard]] const std::string& get_entity_type() const noexcept { return entity_type; }
    void set_groups(std::vector<std::string> v) { groups = std::move(v); }
    [[nodiscard]] const std::vector<std::string>& get_groups() const noexcept { return groups; }
    void set_scope(std::string v) { scope = std::move(v); }
    [[nodiscard]] const std::string& get_scope() const noexcept { return scope; }
    std::string role_id{};
    std::string role_name{};
    std::string entity_type{};
    std::vector<std::string> groups{};
    std::string scope{};
};

class GroupsAndResourcesWire
{
public:
    void set_groups(std::vector<std::string> v) { groups = std::move(v); }
    [[nodiscard]] const std::vector<std::string>& get_groups() const noexcept { return groups; }
    void set_resources(std::vector<std::string> v) { resources = std::move(v); }
    [[nodiscard]] const std::vector<std::string>& get_resources() const noexcept { return resources; }
    std::vector<std::string> groups{};
    std::vector<std::string> resources{};
};

class StatusOkWire
{
public:
    void set_status(std::string v) { status = std::move(v); }
    [[nodiscard]] const std::string& get_status() const noexcept { return status; }
    std::string status{};
};

class TokenResponseWire
{
public:
    void set_token(std::string v) { token = std::move(v); }
    [[nodiscard]] const std::string& get_token() const noexcept { return token; }
    void set_token_type(std::string v) { token_type = std::move(v); }
    [[nodiscard]] const std::string& get_token_type() const noexcept { return token_type; }
    std::string token{};
    std::string token_type{};
};

} // namespace app::handlers::user_role

template<>
struct serde::Serializable<app::handlers::user_role::GroupInfoWire>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"group", &app::handlers::user_role::GroupInfoWire::get_group, &app::handlers::user_role::GroupInfoWire::set_group, serde::FieldOptions{}>{},
            serde::FieldDesc<"description", &app::handlers::user_role::GroupInfoWire::get_description, &app::handlers::user_role::GroupInfoWire::set_description, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "group_info_wire"; }
};

template<>
struct serde::Serializable<app::handlers::user_role::ParentInfoWire>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"name", &app::handlers::user_role::ParentInfoWire::get_name, &app::handlers::user_role::ParentInfoWire::set_name, serde::FieldOptions{}>{},
            serde::FieldDesc<"description", &app::handlers::user_role::ParentInfoWire::get_description, &app::handlers::user_role::ParentInfoWire::set_description, serde::FieldOptions{}>{},
            serde::FieldDesc<"groups", &app::handlers::user_role::ParentInfoWire::get_groups, &app::handlers::user_role::ParentInfoWire::set_groups, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "parent_info_wire"; }
};

template<>
struct serde::Serializable<app::handlers::user_role::ParentGroupDescriptionWire>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"name", &app::handlers::user_role::ParentGroupDescriptionWire::get_name, &app::handlers::user_role::ParentGroupDescriptionWire::set_name, serde::FieldOptions{}>{},
            serde::FieldDesc<"description", &app::handlers::user_role::ParentGroupDescriptionWire::get_description, &app::handlers::user_role::ParentGroupDescriptionWire::set_description, serde::FieldOptions{}>{},
            serde::FieldDesc<"scopes", &app::handlers::user_role::ParentGroupDescriptionWire::get_scopes, &app::handlers::user_role::ParentGroupDescriptionWire::set_scopes, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "parent_group_description_wire"; }
};

template<>
struct serde::Serializable<app::handlers::user_role::ParentGroupInfoWire>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"name", &app::handlers::user_role::ParentGroupInfoWire::get_name, &app::handlers::user_role::ParentGroupInfoWire::set_name, serde::FieldOptions{}>{},
            serde::FieldDesc<"resources", &app::handlers::user_role::ParentGroupInfoWire::get_resources, &app::handlers::user_role::ParentGroupInfoWire::set_resources, serde::FieldOptions{}>{},
            serde::FieldDesc<"scopes", &app::handlers::user_role::ParentGroupInfoWire::get_scopes, &app::handlers::user_role::ParentGroupInfoWire::set_scopes, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "parent_group_info_wire"; }
};

template<>
struct serde::Serializable<app::handlers::user_role::ParentGroupInfoRequestWire>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"name", &app::handlers::user_role::ParentGroupInfoRequestWire::get_name, &app::handlers::user_role::ParentGroupInfoRequestWire::set_name, serde::FieldOptions{}>{},
            serde::FieldDesc<"scopes", &app::handlers::user_role::ParentGroupInfoRequestWire::get_scopes, &app::handlers::user_role::ParentGroupInfoRequestWire::set_scopes, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "parent_group_info_request_wire"; }
};

template<>
struct serde::Serializable<app::handlers::user_role::CreateRoleInputWire>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"role_name", &app::handlers::user_role::CreateRoleInputWire::get_role_name, &app::handlers::user_role::CreateRoleInputWire::set_role_name, serde::FieldOptions{}>{},
            serde::FieldDesc<"groups", &app::handlers::user_role::CreateRoleInputWire::get_groups, &app::handlers::user_role::CreateRoleInputWire::set_groups, serde::FieldOptions{}>{},
            serde::FieldDesc<"role_scope", &app::handlers::user_role::CreateRoleInputWire::get_role_scope, &app::handlers::user_role::CreateRoleInputWire::set_role_scope, serde::FieldOptions{}>{},
            serde::FieldDesc<"entity_type", &app::handlers::user_role::CreateRoleInputWire::get_entity_type, &app::handlers::user_role::CreateRoleInputWire::set_entity_type, serde::FieldOptions{}>{},
            serde::FieldDesc<"parent_groups", &app::handlers::user_role::CreateRoleInputWire::get_parent_groups, &app::handlers::user_role::CreateRoleInputWire::set_parent_groups, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "create_role_input_wire"; }
};

template<>
struct serde::Serializable<app::handlers::user_role::UpdateRoleInputWire>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"groups", &app::handlers::user_role::UpdateRoleInputWire::get_groups, &app::handlers::user_role::UpdateRoleInputWire::set_groups, serde::FieldOptions{}>{},
            serde::FieldDesc<"role_name", &app::handlers::user_role::UpdateRoleInputWire::get_role_name, &app::handlers::user_role::UpdateRoleInputWire::set_role_name, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "update_role_input_wire"; }
};

template<>
struct serde::Serializable<app::handlers::user_role::AcceptEntityWire>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"entity_id", &app::handlers::user_role::AcceptEntityWire::get_entity_id, &app::handlers::user_role::AcceptEntityWire::set_entity_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"entity_type", &app::handlers::user_role::AcceptEntityWire::get_entity_type, &app::handlers::user_role::AcceptEntityWire::set_entity_type, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "accept_entity_wire"; }
};

template<>
struct serde::Serializable<app::handlers::user_role::InvitationWire>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"entity_id", &app::handlers::user_role::InvitationWire::get_entity_id, &app::handlers::user_role::InvitationWire::set_entity_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"entity_type", &app::handlers::user_role::InvitationWire::get_entity_type, &app::handlers::user_role::InvitationWire::set_entity_type, serde::FieldOptions{}>{},
            serde::FieldDesc<"entity_name", &app::handlers::user_role::InvitationWire::get_entity_name, &app::handlers::user_role::InvitationWire::set_entity_name, serde::FieldOptions{}>{},
            serde::FieldDesc<"role_id", &app::handlers::user_role::InvitationWire::get_role_id, &app::handlers::user_role::InvitationWire::set_role_id, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "invitation_wire"; }
};

template<>
struct serde::Serializable<app::handlers::user_role::MinimalRoleInfoWire>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"role_id", &app::handlers::user_role::MinimalRoleInfoWire::get_role_id, &app::handlers::user_role::MinimalRoleInfoWire::set_role_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"role_name", &app::handlers::user_role::MinimalRoleInfoWire::get_role_name, &app::handlers::user_role::MinimalRoleInfoWire::set_role_name, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "minimal_role_info_wire"; }
};

template<>
struct serde::Serializable<app::handlers::user_role::ListUsersInEntityWire>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"email", &app::handlers::user_role::ListUsersInEntityWire::get_email, &app::handlers::user_role::ListUsersInEntityWire::set_email, serde::FieldOptions{}>{},
            serde::FieldDesc<"roles", &app::handlers::user_role::ListUsersInEntityWire::get_roles, &app::handlers::user_role::ListUsersInEntityWire::set_roles, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "list_users_in_entity_wire"; }
};

template<>
struct serde::Serializable<app::handlers::user_role::RoleInfoWithGroupsWire>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"role_id", &app::handlers::user_role::RoleInfoWithGroupsWire::get_role_id, &app::handlers::user_role::RoleInfoWithGroupsWire::set_role_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"groups", &app::handlers::user_role::RoleInfoWithGroupsWire::get_groups, &app::handlers::user_role::RoleInfoWithGroupsWire::set_groups, serde::FieldOptions{}>{},
            serde::FieldDesc<"role_name", &app::handlers::user_role::RoleInfoWithGroupsWire::get_role_name, &app::handlers::user_role::RoleInfoWithGroupsWire::set_role_name, serde::FieldOptions{}>{},
            serde::FieldDesc<"role_scope", &app::handlers::user_role::RoleInfoWithGroupsWire::get_role_scope, &app::handlers::user_role::RoleInfoWithGroupsWire::set_role_scope, serde::FieldOptions{}>{},
            serde::FieldDesc<"entity_type", &app::handlers::user_role::RoleInfoWithGroupsWire::get_entity_type, &app::handlers::user_role::RoleInfoWithGroupsWire::set_entity_type, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "role_info_with_groups_wire"; }
};

template<>
struct serde::Serializable<app::handlers::user_role::RoleInfoWithParentsWire>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"role_id", &app::handlers::user_role::RoleInfoWithParentsWire::get_role_id, &app::handlers::user_role::RoleInfoWithParentsWire::set_role_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"parent_groups", &app::handlers::user_role::RoleInfoWithParentsWire::get_parent_groups, &app::handlers::user_role::RoleInfoWithParentsWire::set_parent_groups, serde::FieldOptions{}>{},
            serde::FieldDesc<"role_name", &app::handlers::user_role::RoleInfoWithParentsWire::get_role_name, &app::handlers::user_role::RoleInfoWithParentsWire::set_role_name, serde::FieldOptions{}>{},
            serde::FieldDesc<"role_scope", &app::handlers::user_role::RoleInfoWithParentsWire::get_role_scope, &app::handlers::user_role::RoleInfoWithParentsWire::set_role_scope, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "role_info_with_parents_wire"; }
};

template<>
struct serde::Serializable<app::handlers::user_role::RoleInfoResponseNewWire>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"role_id", &app::handlers::user_role::RoleInfoResponseNewWire::get_role_id, &app::handlers::user_role::RoleInfoResponseNewWire::set_role_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"role_name", &app::handlers::user_role::RoleInfoResponseNewWire::get_role_name, &app::handlers::user_role::RoleInfoResponseNewWire::set_role_name, serde::FieldOptions{}>{},
            serde::FieldDesc<"entity_type", &app::handlers::user_role::RoleInfoResponseNewWire::get_entity_type, &app::handlers::user_role::RoleInfoResponseNewWire::set_entity_type, serde::FieldOptions{}>{},
            serde::FieldDesc<"groups", &app::handlers::user_role::RoleInfoResponseNewWire::get_groups, &app::handlers::user_role::RoleInfoResponseNewWire::set_groups, serde::FieldOptions{}>{},
            serde::FieldDesc<"scope", &app::handlers::user_role::RoleInfoResponseNewWire::get_scope, &app::handlers::user_role::RoleInfoResponseNewWire::set_scope, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "role_info_response_new_wire"; }
};

template<>
struct serde::Serializable<app::handlers::user_role::GroupsAndResourcesWire>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"groups", &app::handlers::user_role::GroupsAndResourcesWire::get_groups, &app::handlers::user_role::GroupsAndResourcesWire::set_groups, serde::FieldOptions{}>{},
            serde::FieldDesc<"resources", &app::handlers::user_role::GroupsAndResourcesWire::get_resources, &app::handlers::user_role::GroupsAndResourcesWire::set_resources, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "groups_and_resources_wire"; }
};

template<>
struct serde::Serializable<app::handlers::user_role::StatusOkWire>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"status", &app::handlers::user_role::StatusOkWire::get_status, &app::handlers::user_role::StatusOkWire::set_status, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "status_ok_wire"; }
};

template<>
struct serde::Serializable<app::handlers::user_role::TokenResponseWire>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"token", &app::handlers::user_role::TokenResponseWire::get_token, &app::handlers::user_role::TokenResponseWire::set_token, serde::FieldOptions{}>{},
            serde::FieldDesc<"token_type", &app::handlers::user_role::TokenResponseWire::get_token_type, &app::handlers::user_role::TokenResponseWire::set_token_type, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "token_response_wire"; }
};

export namespace app::handlers::user_role {

class UserRoleRouteHandlers
{
public:
    explicit UserRoleRouteHandlers(app::context::AppContext& context) noexcept
        : m_context{&context}
    {
    }

    void handle_get_authorization_info(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto auth_error = authorize(req, "get_authorization_info")) {
            send_error(res, send, *auth_error);
            return;
        }
        (void)identity_from(req);

        // Upstream: info::get_group_authorization_info() (static permission-group table).
        auto text = serde::Ser::encode_json(group_authorization_info());
        track_flow("get_authorization_info", "user_role.get_authorization_info");
        send_json(res, send, text, true);
    }

    void handle_get_role_from_token(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto auth_error = authorize(req, "get_role_from_token")) {
            send_error(res, send, *auth_error);
            return;
        }
        const auto identity = identity_from(req);
        const auto groups = groups_for_token(req, identity);

        auto text = serde::Ser::encode_json(groups);
        track_flow("get_role_from_token", "user_role.get_role_from_token");
        send_json(res, send, text, true);
    }

    void handle_get_groups_and_resources_for_role_from_token(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto auth_error = authorize(req, "get_groups_and_resources_for_role_from_token")) {
            send_error(res, send, *auth_error);
            return;
        }
        const auto identity = identity_from(req);
        GroupsAndResourcesWire response{};
        response.set_groups(groups_for_token(req, identity));
        std::vector<std::string> resources{};
        for (const auto& role : roles_in_scope(identity, "")) {
            resources.push_back(role.get_role_id());
        }
        response.set_resources(std::move(resources));

        auto text = serde::Ser::encode_json(response);
        track_flow("get_groups_and_resources_for_role_from_token", "user_role.get_groups_and_resources_for_role_from_token");
        send_json(res, send, text, true);
    }

    void handle_get_parent_groups_info_for_role_from_token(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto auth_error = authorize(req, "get_parent_groups_info_for_role_from_token")) {
            send_error(res, send, *auth_error);
            return;
        }
        const auto identity = identity_from(req);
        const auto groups = groups_for_token(req, identity);
        std::vector<ParentGroupInfoWire> response{};
        for (const auto& group : groups) {
            ParentGroupInfoWire info{};
            info.set_name(parent_of(group));
            info.set_scopes({group});
            info.set_resources({group});
            response.push_back(std::move(info));
        }

        auto text = serde::Ser::encode_json(response);
        track_flow("get_parent_groups_info_for_role_from_token", "user_role.get_parent_groups_info_for_role_from_token");
        send_json(res, send, text, true);
    }

    void handle_create_role(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto auth_error = authorize(req, "create_role")) {
            send_error(res, send, *auth_error);
            return;
        }
        const auto identity = identity_from(req);
        const auto body = req.body_as_string();
        if (body.empty()) {
            send_error(res, send, error(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }
        auto parsed = serde::Ser::deserialize<CreateRoleInputWire>("application/json", body);
        if (!parsed) {
            send_error(res, send, error(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }
        if (const auto validation = validate_role_input(*parsed, identity)) {
            send_error(res, send, *validation);
            return;
        }
        const auto merchant = m_context->repository().find_merchant_account_by_string(identity.merchant_id);
        if (!merchant) {
            send_error(res, send, error(core::error::AppErrorKind::NOT_FOUND, "merchant account not found"));
            return;
        }

        app::store::roles::RoleRecord record{};
        record.set_role_id(m_context->ids().payment_id("role_"));
        record.set_role_name(parsed->get_role_name());
        record.set_entity_type(normalize_entity(parsed->get_entity_type()));
        record.set_merchant_id(identity.merchant_id);
        record.set_organization_id(identity.org_id);
        record.set_profile_id(identity.profile_id);
        record.set_is_internal(0);
        record.set_status("active");
        record.set_created_at(now_string());
        const auto saved = m_context->repository().create_role(std::move(record));
        if (!saved) {
            send_error(res, send, saved.error());
            return;
        }

        RoleInfoWithGroupsWire response{};
        response.set_role_id(saved->get_role_id());
        response.set_role_name(saved->get_role_name());
        response.set_role_scope(parsed->get_role_scope().empty() ? std::string{"organization"} : parsed->get_role_scope());
        response.set_entity_type(saved->get_entity_type());
        response.set_groups(groups_for(*saved));

        auto text = serde::Ser::encode_json(response);
        track_flow("create_role", "user_role.create_role");
        send_json(res, send, text, true);
    }

    void handle_create_role_v2(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto auth_error = authorize(req, "create_role_v2")) {
            send_error(res, send, *auth_error);
            return;
        }
        const auto identity = identity_from(req);
        const auto body = req.body_as_string();
        if (body.empty()) {
            send_error(res, send, error(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }
        auto parsed = serde::Ser::deserialize<CreateRoleInputWire>("application/json", body);
        if (!parsed) {
            send_error(res, send, error(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }
        if (const auto validation = validate_role_input(*parsed, identity)) {
            send_error(res, send, *validation);
            return;
        }
        for (const auto& parent : parsed->get_parent_groups()) {
            if (parent.get_name().empty() || parent.get_scopes().empty()) {
                send_error(res, send, error(core::error::AppErrorKind::VALIDATION_FAILED, "invalid parent group"));
                return;
            }
        }
        if (!m_context->repository().find_merchant_account_by_string(identity.merchant_id)) {
            send_error(res, send, error(core::error::AppErrorKind::NOT_FOUND, "merchant account not found"));
            return;
        }

        app::store::roles::RoleRecord record{};
        record.set_role_id(m_context->ids().payment_id("role_"));
        record.set_role_name(parsed->get_role_name());
        record.set_entity_type(normalize_entity(parsed->get_entity_type()));
        record.set_merchant_id(identity.merchant_id);
        record.set_organization_id(identity.org_id);
        record.set_profile_id(identity.profile_id);
        record.set_is_internal(0);
        record.set_status("active");
        record.set_created_at(now_string());
        const auto saved = m_context->repository().create_role(std::move(record));
        if (!saved) {
            send_error(res, send, saved.error());
            return;
        }

        RoleInfoWithParentsWire response{};
        response.set_role_id(saved->get_role_id());
        response.set_role_name(saved->get_role_name());
        response.set_role_scope(parsed->get_role_scope().empty() ? std::string{"organization"} : parsed->get_role_scope());
        response.set_parent_groups(parent_group_descriptions(saved->get_entity_type(), ""));

        auto text = serde::Ser::encode_json(response);
        track_flow("create_role_v2", "user_role.create_role_v2");
        send_json(res, send, text, true);
    }

    void handle_get_role(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto auth_error = authorize(req, "get_role")) {
            send_error(res, send, *auth_error);
            return;
        }
        const auto identity = identity_from(req);
        const auto role_id = role_id_from_path(req);
        if (!role_id.has_value() || role_id->empty()) {
            send_error(res, send, error(core::error::AppErrorKind::NOT_FOUND, "role not found"));
            return;
        }
        const auto role = m_context->repository().find_role_by_string(*role_id);
        if (!role || role_is_internal(*role) || !role_in_scope(*role, identity)) {
            send_error(res, send, error(core::error::AppErrorKind::NOT_FOUND, "role not found"));
            return;
        }

        RoleInfoWithGroupsWire response{};
        response.set_role_id(role->get_role_id());
        response.set_role_name(role->get_role_name());
        response.set_role_scope(scope_for(*role));
        response.set_entity_type(role->get_entity_type());
        response.set_groups(groups_for(*role));

        auto text = serde::Ser::encode_json(response);
        track_flow("get_role", "user_role.get_role");
        send_json(res, send, text, true);
    }

    void handle_get_parent_info_for_role(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto auth_error = authorize(req, "get_parent_info_for_role")) {
            send_error(res, send, *auth_error);
            return;
        }
        const auto identity = identity_from(req);
        const auto role_id = role_id_from_path(req);
        if (!role_id.has_value() || role_id->empty()) {
            send_error(res, send, error(core::error::AppErrorKind::NOT_FOUND, "role not found"));
            return;
        }
        const auto role = m_context->repository().find_role_by_string(*role_id);
        if (!role || role_is_internal(*role) || !role_in_scope(*role, identity)) {
            send_error(res, send, error(core::error::AppErrorKind::NOT_FOUND, "role not found"));
            return;
        }

        RoleInfoWithParentsWire response{};
        response.set_role_id(role->get_role_id());
        response.set_role_name(role->get_role_name());
        response.set_role_scope(scope_for(*role));
        response.set_parent_groups(parent_group_descriptions(role->get_entity_type(), ""));

        auto text = serde::Ser::encode_json(response);
        track_flow("get_parent_info_for_role", "user_role.get_parent_info_for_role");
        send_json(res, send, text, true);
    }

    void handle_update_role(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto auth_error = authorize(req, "update_role")) {
            send_error(res, send, *auth_error);
            return;
        }
        const auto identity = identity_from(req);
        const auto role_id = role_id_from_path(req);
        if (!role_id.has_value() || role_id->empty()) {
            send_error(res, send, error(core::error::AppErrorKind::NOT_FOUND, "role not found"));
            return;
        }
        const auto body = req.body_as_string();
        if (body.empty()) {
            send_error(res, send, error(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }
        auto parsed = serde::Ser::deserialize<UpdateRoleInputWire>("application/json", body);
        if (!parsed) {
            send_error(res, send, error(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }
        auto role = m_context->repository().find_role_by_string(*role_id);
        if (!role || !role_in_scope(*role, identity)) {
            send_error(res, send, error(core::error::AppErrorKind::NOT_FOUND, "role not found"));
            return;
        }
        if (!role_is_updatable(*role)) {
            send_error(res, send, error(core::error::AppErrorKind::PRECONDITION_FAILED, "role is not updatable"));
            return;
        }
        if (const auto hierarchy = get_min_entity(requestor_entity(identity), role->get_entity_type())) {
            send_error(res, send, *hierarchy);
            return;
        }
        if (parsed->get_role_name().has_value()) {
            const auto name = *parsed->get_role_name();
            if (name.empty() || name.size() > 64) {
                send_error(res, send, error(core::error::AppErrorKind::VALIDATION_FAILED, "invalid role name"));
                return;
            }
            for (const auto& existing : m_context->repository().list_roles()) {
                if (existing.get_role_id() != role->get_role_id()
                    && existing.get_role_name() == name && role_in_scope(existing, identity)) {
                    send_error(res, send, error(core::error::AppErrorKind::ALREADY_EXISTS, "role name already exists"));
                    return;
                }
            }
            role->set_role_name(name);
        }
        if (parsed->get_groups().has_value()) {
            if (parsed->get_groups()->empty()) {
                send_error(res, send, error(core::error::AppErrorKind::VALIDATION_FAILED, "role requires at least one group"));
                return;
            }
        }
        m_context->repository().update_role(*role);
        blacklist("role", role->get_role_id() + ":" + identity.merchant_id);

        RoleInfoWithGroupsWire response{};
        response.set_role_id(role->get_role_id());
        response.set_role_name(role->get_role_name());
        response.set_role_scope(scope_for(*role));
        response.set_entity_type(role->get_entity_type());
        response.set_groups(parsed->get_groups().value_or(groups_for(*role)));

        auto text = serde::Ser::encode_json(response);
        track_flow("update_role", "user_role.update_role");
        send_json(res, send, text, true);
    }

    void handle_update_user_role(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto auth_error = authorize(req, "update_user_role")) {
            send_error(res, send, *auth_error);
            return;
        }
        const auto identity = identity_from(req);
        const auto body = req.body_as_string();
        if (body.empty()) {
            send_error(res, send, error(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }
        using Input = app::dto::user_role::UpdateUserRoleRequest;
        auto parsed = serde::Ser::deserialize<Input>("application/json", body);
        if (!parsed) {
            send_error(res, send, error(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }
        const app::crypto::SecretString email_secret{parsed->get_email().get_value()};
        const auto& role_id = parsed->get_role_id();
        if (email_secret.value().empty() || role_id.empty()) {
            send_error(res, send, error(core::error::AppErrorKind::PRECONDITION_FAILED, "invalid role assignment"));
            return;
        }
        const auto role = m_context->repository().find_role_by_string(role_id);
        if (!role || !role_in_scope(*role, identity)) {
            send_error(res, send, error(core::error::AppErrorKind::NOT_FOUND, "role not found"));
            return;
        }
        if (const auto hierarchy = get_min_entity(requestor_entity(identity), role->get_entity_type())) {
            send_error(res, send, *hierarchy);
            return;
        }
        const auto user = user_by_email(identity, email_secret.value());
        if (!user) {
            send_error(res, send, error(core::error::AppErrorKind::NOT_FOUND, "user not found"));
            return;
        }
        if (is_blacklisted("user-role", email_secret.value() + ":" + role_id)) {
            send_error(res, send, error(core::error::AppErrorKind::PRECONDITION_FAILED, "role assignment blacklisted"));
            return;
        }
        bool updated{false};
        for (auto assignment : m_context->repository().list_user_roles()) {
            if (assignment.get_user_id() == user->get_user_id()
                && assignment.get_role_id() == role_id && assignment_in_scope(assignment, identity)) {
                assignment.set_status("invitation_accepted");
                m_context->repository().update_user_role(assignment);
                updated = true;
            }
        }
        if (!updated) {
            app::store::user_roles::UserRoleRecord assignment{};
            assignment.set_user_role_id(m_context->ids().payment_id("ur_"));
            assignment.set_user_id(user->get_user_id());
            assignment.set_role_id(role_id);
            assignment.set_merchant_id(identity.merchant_id);
            assignment.set_organization_id(identity.org_id);
            assignment.set_profile_id(identity.profile_id);
            assignment.set_status("invitation_accepted");
            assignment.set_created_at(now_string());
            const auto created = m_context->repository().create_user_role(std::move(assignment));
            if (!created) {
                send_error(res, send, created.error());
                return;
            }
        }

        StatusOkWire response{};
        response.set_status("Ok");
        auto text = serde::Ser::encode_json(response);
        track_flow("update_user_role", "user_role.update_user_role");
        send_json(res, send, text, true);
    }

    void handle_accept_invitations_v2(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto auth_error = authorize(req, "accept_invitations_v2")) {
            send_error(res, send, *auth_error);
            return;
        }
        const auto identity = identity_from(req);
        const auto body = req.body_as_string();
        if (body.empty()) {
            send_error(res, send, error(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }
        auto parsed = serde::Ser::deserialize<std::vector<AcceptEntityWire>>("application/json", body);
        if (!parsed || parsed->empty()) {
            send_error(res, send, error(core::error::AppErrorKind::INVALID_JSON, "invalid invitation payload"));
            return;
        }
        const auto subject = token_subject(req);
        std::size_t accepted{0};
        for (const auto& entity : *parsed) {
            const auto role = find_role_for_entity(identity, entity.get_entity_type());
            if (!role) {
                continue;
            }
            for (auto assignment : m_context->repository().list_user_roles()) {
                if (assignment.get_user_id() == subject && assignment.get_role_id() == role->get_role_id()) {
                    assignment.set_status("active");
                    m_context->repository().update_user_role(assignment);
                    ++accepted;
                }
            }
        }
        if (accepted == 0) {
            send_error(res, send, error(core::error::AppErrorKind::NOT_FOUND, "no invitations to accept"));
            return;
        }

        StatusOkWire response{};
        response.set_status("Ok");
        auto text = serde::Ser::encode_json(response);
        track_flow("accept_invitations_v2", "user_role.accept_invitations_v2");
        send_json(res, send, text, true);
    }

    void handle_accept_invitations_pre_auth(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto auth_error = authorize(req, "accept_invitations_pre_auth")) {
            send_error(res, send, *auth_error);
            return;
        }
        const auto identity = identity_from(req);
        const auto body = req.body_as_string();
        if (body.empty()) {
            send_error(res, send, error(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }
        auto parsed = serde::Ser::deserialize<std::vector<AcceptEntityWire>>("application/json", body);
        if (!parsed || parsed->empty()) {
            send_error(res, send, error(core::error::AppErrorKind::INVALID_JSON, "invalid invitation payload"));
            return;
        }
        const auto subject = token_subject(req);
        std::vector<std::string> activated{};
        for (const auto& entity : *parsed) {
            const auto role = find_role_for_entity(identity, entity.get_entity_type());
            if (!role) {
                continue;
            }
            for (auto assignment : m_context->repository().list_user_roles()) {
                if (assignment.get_user_id() == subject && assignment.get_role_id() == role->get_role_id()) {
                    assignment.set_status("active");
                    m_context->repository().update_user_role(assignment);
                    activated.push_back(role->get_role_id());
                }
            }
        }
        if (activated.empty()) {
            send_error(res, send, error(core::error::AppErrorKind::NOT_FOUND, "no invitations to accept"));
            return;
        }

        // Upstream CurrentFlow::next: single-purpose token plus a cookie.
        const auto payload = std::format(R"({{"sub":"{}","flow":"AcceptInvite","roles":{}}})",
                                         subject, serde::Ser::encode_json(activated));
        const auto token = app::crypto::JwtSigner{invite_secret()}.sign(payload);
        const auto cookie = std::format("token={}; HttpOnly; Path=/; SameSite=Lax", token);
        res.set_header(std::string_view{"set-cookie"}, cookie);

        TokenResponseWire response{};
        response.set_token(token);
        response.set_token_type("SINGLE_PURPOSE");
        auto text = serde::Ser::encode_json(response);
        track_flow("accept_invitations_pre_auth", "user_role.accept_invitations_pre_auth");
        send_json(res, send, text, true);
    }

    void handle_delete_user_role(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto auth_error = authorize(req, "delete_user_role")) {
            send_error(res, send, *auth_error);
            return;
        }
        const auto identity = identity_from(req);
        const auto body = req.body_as_string();
        if (body.empty()) {
            send_error(res, send, error(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }
        using Input = app::dto::user_role::DeleteUserRoleRequest;
        auto parsed = serde::Ser::deserialize<Input>("application/json", body);
        if (!parsed) {
            send_error(res, send, error(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }
        const app::crypto::SecretString email_secret{parsed->get_email().get_value()};
        if (email_secret.value().empty()) {
            send_error(res, send, error(core::error::AppErrorKind::NOT_FOUND, "user not found"));
            return;
        }
        const auto user = user_by_email(identity, email_secret.value());
        if (!user) {
            send_error(res, send, error(core::error::AppErrorKind::NOT_FOUND, "user not found"));
            return;
        }
        std::vector<std::string> removed{};
        for (const auto& assignment : m_context->repository().list_user_roles()) {
            if (assignment.get_user_id() == user->get_user_id() && assignment_in_scope(assignment, identity)) {
                removed.push_back(assignment.get_role_id());
                m_context->repository().remove_user_role(assignment.get_id());
            }
        }
        if (removed.empty()) {
            send_error(res, send, error(core::error::AppErrorKind::NOT_FOUND, "no roles to delete"));
            return;
        }
        // Deactivate the user once no assignments remain in scope.
        bool remains{false};
        for (const auto& assignment : m_context->repository().list_user_roles()) {
            if (assignment.get_user_id() == user->get_user_id()) {
                remains = true;
                break;
            }
        }
        if (!remains) {
            auto deactivated = *user;
            deactivated.set_status("inactive");
            m_context->repository().update_user(deactivated);
        }
        for (const auto& role_id : removed) {
            blacklist("user-role", email_secret.value() + ":" + role_id);
        }

        // Deletion e-mail is a deterministic event until a mail provider is wired.
        const auto payload = std::format(R"({{"email":"{}","merchant_id":"{}"}})",
                                         email_secret.value(), identity.merchant_id);
        const auto event_id = m_context->events().publish("user_role.deleted", payload);

        app::dto::user_role::DeleteUserRoleResponse response{};
        response.set_is_email_sent(!event_id.empty());
        auto text = serde::Ser::encode_json(response);
        track_flow("delete_user_role", "user_role.delete_user_role");
        send_json(res, send, text, true);
    }

    void handle_get_role_information(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto auth_error = authorize(req, "get_role_information")) {
            send_error(res, send, *auth_error);
            return;
        }
        (void)identity_from(req);

        // Upstream: LazyLock<Vec<ParentInfo>> built from the permission-group table.
        auto text = serde::Ser::encode_json(parent_authorization_info());
        track_flow("get_role_information", "user_role.get_role_information");
        send_json(res, send, text, true);
    }

    void handle_get_parent_group_info(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto auth_error = authorize(req, "get_parent_group_info")) {
            send_error(res, send, *auth_error);
            return;
        }
        const auto identity = identity_from(req);
        const auto query = app::http::RequestQuery{}(req);
        const auto entity_type = normalize_entity(query.get("entity_type").value_or(""));
        const auto product_type = query.get("product_type").value_or("");
        if (const auto hierarchy = get_min_entity(requestor_entity(identity), entity_type)) {
            send_error(res, send, *hierarchy);
            return;
        }

        auto text = serde::Ser::encode_json(parent_group_descriptions(entity_type, product_type));
        track_flow("get_parent_group_info", "user_role.get_parent_group_info");
        send_json(res, send, text, true);
    }

    void handle_list_users_in_lineage(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto auth_error = authorize(req, "list_users_in_lineage")) {
            send_error(res, send, *auth_error);
            return;
        }
        const auto identity = identity_from(req);
        const auto query = app::http::RequestQuery{}(req);
        const auto entity_type = normalize_entity(query.get("entity_type").value_or(""));
        const auto limit = parse_limit(query.get("limit"));
        const auto offset = parse_limit(query.get("offset"));
        if (const auto hierarchy = get_min_entity(requestor_entity(identity), entity_type)) {
            send_error(res, send, *hierarchy);
            return;
        }

        std::map<std::string, std::vector<MinimalRoleInfoWire>> by_user{};
        for (const auto& assignment : m_context->repository().list_user_roles()) {
            if (!assignment_in_scope(assignment, identity)) {
                continue;
            }
            const auto role = m_context->repository().find_role_by_string(assignment.get_role_id());
            if (!role || (!entity_type.empty() && role->get_entity_type() != entity_type)) {
                continue;
            }
            MinimalRoleInfoWire info{};
            info.set_role_id(role->get_role_id());
            info.set_role_name(role->get_role_name());
            by_user[assignment.get_user_id()].push_back(std::move(info));
        }
        std::vector<ListUsersInEntityWire> response{};
        std::size_t index{0};
        for (const auto& [user_id, roles] : by_user) {
            if (index++ < offset) {
                continue;
            }
            if (response.size() >= limit) {
                break;
            }
            const auto user = m_context->repository().find_user_by_string(user_id);
            ListUsersInEntityWire item{};
            item.set_email(user ? user->get_email() : user_id);
            item.set_roles(roles);
            response.push_back(std::move(item));
        }

        auto text = serde::Ser::encode_json(response);
        track_flow("list_users_in_lineage", "user_role.list_users_in_lineage");
        send_json(res, send, text, true);
    }

    void handle_list_roles_with_info(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto auth_error = authorize(req, "list_roles_with_info")) {
            send_error(res, send, *auth_error);
            return;
        }
        const auto identity = identity_from(req);
        const auto query = app::http::RequestQuery{}(req);
        const auto entity_type = normalize_entity(query.get("entity_type").value_or(""));
        const auto limit = parse_limit(query.get("limit"));
        const auto offset = parse_limit(query.get("offset"));
        if (const auto hierarchy = get_min_entity(requestor_entity(identity), entity_type)) {
            send_error(res, send, *hierarchy);
            return;
        }

        std::vector<RoleInfoResponseNewWire> response{};
        std::size_t index{0};
        for (const auto& role : roles_in_scope(identity, entity_type)) {
            if (index++ < offset) {
                continue;
            }
            if (response.size() >= limit) {
                break;
            }
            RoleInfoResponseNewWire item{};
            item.set_role_id(role.get_role_id());
            item.set_role_name(role.get_role_name());
            item.set_entity_type(role.get_entity_type());
            item.set_groups(groups_for(role));
            item.set_scope(scope_for(role));
            response.push_back(std::move(item));
        }

        auto text = serde::Ser::encode_json(response);
        track_flow("list_roles_with_info", "user_role.list_roles_with_info");
        send_json(res, send, text, true);
    }

    void handle_list_invitable_roles_at_entity_level(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        list_minimal_roles(req, res, std::move(send), "list_invitable_roles_at_entity_level", false);
    }

    void handle_list_updatable_roles_at_entity_level(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        list_minimal_roles(req, res, std::move(send), "list_updatable_roles_at_entity_level", true);
    }

    void handle_list_invitations_for_user(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto auth_error = authorize(req, "list_invitations_for_user")) {
            send_error(res, send, *auth_error);
            return;
        }
        const auto identity = identity_from(req);
        const auto subject = token_subject(req);
        std::vector<InvitationWire> response{};
        for (const auto& assignment : m_context->repository().list_user_roles()) {
            if (assignment.get_user_id() != subject || !assignment_in_scope(assignment, identity)) {
                continue;
            }
            if (assignment.get_status() == "active" || assignment.get_status() == "invitation_accepted") {
                continue;
            }
            const auto role = m_context->repository().find_role_by_string(assignment.get_role_id());
            InvitationWire item{};
            item.set_entity_id(entity_id_for(assignment));
            item.set_entity_type(role ? role->get_entity_type() : std::string{"merchant"});
            item.set_entity_name(entity_name_for(assignment));
            item.set_role_id(assignment.get_role_id());
            response.push_back(std::move(item));
        }

        auto text = serde::Ser::encode_json(response);
        track_flow("list_invitations_for_user", "user_role.list_invitations_for_user");
        send_json(res, send, text, true);
    }

    void install(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "/user"}
            .add_route(
                Route{"/permission_info"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_get_authorization_info(req, res, std::move(send));
                    })
                    .summary("Hyperswitch get_authorization_info")
            )
            .add_route(
                Route{"/role"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_get_role_from_token(req, res, std::move(send));
                    })
                    .summary("Hyperswitch get_role_from_token")
            )
            .add_route(
                Route{"/role/v2"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_get_groups_and_resources_for_role_from_token(req, res, std::move(send));
                    })
                    .summary("Hyperswitch get_groups_and_resources_for_role_from_token")
            )
            .add_route(
                Route{"/role/v3"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_get_parent_groups_info_for_role_from_token(req, res, std::move(send));
                    })
                    .summary("Hyperswitch get_parent_groups_info_for_role_from_token")
            )
            .add_route(
                Route{"/role"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_create_role(req, res, std::move(send));
                    })
                    .summary("Hyperswitch create_role")
            )
            .add_route(
                Route{"/role/v2"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_create_role_v2(req, res, std::move(send));
                    })
                    .summary("Hyperswitch create_role_v2")
            )
            .add_route(
                Route{"/role/:role_id"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_get_role(req, res, std::move(send));
                    })
                    .summary("Hyperswitch get_role")
            )
            .add_route(
                Route{"/role/:role_id/v2"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_get_parent_info_for_role(req, res, std::move(send));
                    })
                    .summary("Hyperswitch get_parent_info_for_role")
            )
            .add_route(
                Route{"/role/:role_id"}
                    .put([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_update_role(req, res, std::move(send));
                    })
                    .summary("Hyperswitch update_role")
            )
            .add_route(
                Route{"/user/update_role"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_update_user_role(req, res, std::move(send));
                    })
                    .summary("Hyperswitch update_user_role")
            )
            .add_route(
                Route{"/user/invite/accept"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_accept_invitations_v2(req, res, std::move(send));
                    })
                    .summary("Hyperswitch accept_invitations_v2")
            )
            .add_route(
                Route{"/user/invite/accept/v2"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_accept_invitations_v2(req, res, std::move(send));
                    })
                    .summary("Hyperswitch accept_invitations_v2 (v2 alias)")
            )
            .add_route(
                Route{"/user/invite/accept/pre_auth"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_accept_invitations_pre_auth(req, res, std::move(send));
                    })
                    .summary("Hyperswitch accept_invitations_pre_auth")
            )
            .add_route(
                Route{"/user/invite/accept/v2/pre_auth"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_accept_invitations_pre_auth(req, res, std::move(send));
                    })
                    .summary("Hyperswitch accept_invitations_pre_auth (v2 alias)")
            )
            .add_route(
                Route{"/user/delete"}
                    .delt([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_delete_user_role(req, res, std::move(send));
                    })
                    .summary("Hyperswitch delete_user_role")
            )
            .add_route(
                Route{"/user/v2/list"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_list_users_in_lineage(req, res, std::move(send));
                    })
                    .summary("Hyperswitch list_users_in_lineage (v2 alias)")
            )
            .add_route(
                Route{"/module/list"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_get_role_information(req, res, std::move(send));
                    })
                    .summary("Hyperswitch get_role_information")
            )
            .add_route(
                Route{"/parent/list"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_get_parent_group_info(req, res, std::move(send));
                    })
                    .summary("Hyperswitch get_parent_group_info")
            )
            .add_route(
                Route{"/user/list"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_list_users_in_lineage(req, res, std::move(send));
                    })
                    .summary("Hyperswitch list_users_in_lineage")
            )
            .add_route(
                Route{"/role/list"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_list_roles_with_info(req, res, std::move(send));
                    })
                    .summary("Hyperswitch list_roles_with_info")
            )
            .add_route(
                Route{"/role/v2/list"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_list_roles_with_info(req, res, std::move(send));
                    })
                    .summary("Hyperswitch list_roles_with_info (v2 alias)")
            )
            .add_route(
                Route{"/role/list/invite"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_list_invitable_roles_at_entity_level(req, res, std::move(send));
                    })
                    .summary("Hyperswitch list_invitable_roles_at_entity_level")
            )
            .add_route(
                Route{"/role/list/update"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_list_updatable_roles_at_entity_level(req, res, std::move(send));
                    })
                    .summary("Hyperswitch list_updatable_roles_at_entity_level")
            )
            .add_route(
                Route{"/list/invitation"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_list_invitations_for_user(req, res, std::move(send));
                    })
                    .summary("Hyperswitch list_invitations_for_user")
            );
    }

private:
    struct RequestIdentity
    {
        std::string merchant_id{};
        std::string org_id{};
        std::string profile_id{};
    };

    [[nodiscard]] core::error::AppError error(
        core::error::AppErrorKind kind,
        std::string message
    ) const
    {
        return core::error::AppErrorBuilder{}.operator()(kind, std::move(message));
    }

    [[nodiscard]] RequestIdentity identity_from(interfaces::io::IRequest& req) const
    {
        const auto auth = app::auth::AuthContextBuilder{}(req);
        RequestIdentity out{};
        out.merchant_id = app::http::merchant_id_from_header{}(req);
        out.org_id = app::http::org_id_from_header{}(req);
        if (out.merchant_id.empty()) {
            out.merchant_id = auth.merchant_id();
        }
        if (out.org_id.empty()) {
            out.org_id = auth.org_id();
        }
        out.profile_id = auth.profile_id();
        return out;
    }

    [[nodiscard]] std::optional<core::error::AppError> authorize(
        interfaces::io::IRequest& req,
        std::string_view operation
    ) const
    {
        const auto rule = m_context->auth_rules().rule_for(operation);
        return m_context->authorizer().authorize_route(app::auth::AuthContextBuilder{}(req), rule);
    }

    void track_flow(std::string_view flow, std::string_view counter) const
    {
        m_context->metrics().record_flow(std::string{flow});
        m_context->metrics().counter(std::string{counter}).increment();
    }

    [[nodiscard]] std::string now_string() const noexcept
    {
        return "1970-01-01T00:00:00Z";
    }

    [[nodiscard]] std::size_t parse_limit(std::optional<std::string> value) const noexcept
    {
        if (!value.has_value() || value->empty()) {
            return 20;
        }
        try {
            return static_cast<std::size_t>(std::stoull(*value));
        } catch (...) {
            return 20;
        }
    }

    [[nodiscard]] int entity_rank(std::string_view entity_type) const noexcept
    {
        if (entity_type == "tenant") {
            return 3;
        }
        if (entity_type == "organization") {
            return 2;
        }
        if (entity_type == "merchant") {
            return 1;
        }
        return 0; // profile
    }

    [[nodiscard]] std::string normalize_entity(std::string_view value) const
    {
        if (value == "tenant" || value == "organization" || value == "merchant" || value == "profile") {
            return std::string{value};
        }
        return {};
    }

    [[nodiscard]] std::string requestor_entity(const RequestIdentity& identity) const
    {
        if (!identity.profile_id.empty()) {
            return "profile";
        }
        if (!identity.merchant_id.empty()) {
            return "merchant";
        }
        if (!identity.org_id.empty()) {
            return "organization";
        }
        return "tenant";
    }

    [[nodiscard]] std::optional<core::error::AppError> get_min_entity(
        const std::string& requestor,
        std::string_view filter
    ) const
    {
        if (filter.empty()) {
            return std::nullopt;
        }
        if (entity_rank(requestor) < entity_rank(filter)) {
            return error(core::error::AppErrorKind::PRECONDITION_FAILED,
                         "requestor entity level cannot access the requested entity level");
        }
        return std::nullopt;
    }

    [[nodiscard]] bool role_in_scope(
        const app::store::roles::RoleRecord& role,
        const RequestIdentity& identity
    ) const noexcept
    {
        if (!identity.merchant_id.empty() && !role.get_merchant_id().empty()
            && role.get_merchant_id() != identity.merchant_id) {
            return false;
        }
        if (!identity.org_id.empty() && !role.get_organization_id().empty()
            && role.get_organization_id() != identity.org_id) {
            return false;
        }
        return true;
    }

    [[nodiscard]] bool assignment_in_scope(
        const app::store::user_roles::UserRoleRecord& assignment,
        const RequestIdentity& identity
    ) const noexcept
    {
        if (!identity.merchant_id.empty() && !assignment.get_merchant_id().empty()
            && assignment.get_merchant_id() != identity.merchant_id) {
            return false;
        }
        if (!identity.org_id.empty() && !assignment.get_organization_id().empty()
            && assignment.get_organization_id() != identity.org_id) {
            return false;
        }
        return true;
    }

    [[nodiscard]] std::vector<app::store::roles::RoleRecord> roles_in_scope(
        const RequestIdentity& identity,
        std::string_view entity_type
    ) const
    {
        std::vector<app::store::roles::RoleRecord> out{};
        for (const auto& role : m_context->repository().list_roles()) {
            if (!role_in_scope(role, identity)) {
                continue;
            }
            if (!entity_type.empty() && role.get_entity_type() != entity_type) {
                continue;
            }
            out.push_back(role);
        }
        return out;
    }

    [[nodiscard]] std::optional<app::store::users::UserRecord> user_by_email(
        const RequestIdentity& identity,
        std::string_view email
    ) const
    {
        for (const auto& user : m_context->database().all<app::store::users::UserRecord>()) {
            if (user.get_email() == email
                && (identity.merchant_id.empty() || user.get_merchant_id().empty()
                    || user.get_merchant_id() == identity.merchant_id)) {
                return user;
            }
        }
        return std::nullopt;
    }

    [[nodiscard]] bool role_is_internal(const app::store::roles::RoleRecord& role) const noexcept
    {
        return role.get_is_internal() != 0;
    }

    [[nodiscard]] bool role_is_updatable(const app::store::roles::RoleRecord& role) const noexcept
    {
        return !role_is_internal(role) && role.get_status() != "deleted";
    }

    [[nodiscard]] bool role_is_deletable(const app::store::roles::RoleRecord& role) const noexcept
    {
        return !role_is_internal(role);
    }

    [[nodiscard]] std::string scope_for(const app::store::roles::RoleRecord& role) const
    {
        return role.get_entity_type().empty() ? std::string{"organization"} : role.get_entity_type();
    }

    [[nodiscard]] std::vector<std::string> groups_for(const app::store::roles::RoleRecord& role) const
    {
        std::vector<std::string> out{"operations_view"};
        const auto& entity = role.get_entity_type();
        if (entity == "merchant" || entity == "profile") {
            out.push_back("operations_manage");
            out.push_back("connectors_view");
        }
        if (!role_is_internal(role)) {
            out.push_back("users_view");
        }
        if (role.get_role_name().find("admin") != std::string::npos) {
            out.push_back("users_manage");
            out.push_back("connectors_manage");
        }
        return out;
    }

    [[nodiscard]] std::vector<std::string> groups_for_token(
        interfaces::io::IRequest& req,
        const RequestIdentity& identity
    ) const
    {
        const auto subject = token_subject(req);
        std::vector<std::string> out{};
        for (const auto& assignment : m_context->repository().list_user_roles()) {
            if (assignment.get_user_id() != subject || !assignment_in_scope(assignment, identity)) {
                continue;
            }
            const auto role = m_context->repository().find_role_by_string(assignment.get_role_id());
            if (!role || role_is_internal(*role)) {
                continue;
            }
            for (auto&& group : groups_for(*role)) {
                if (std::find(out.begin(), out.end(), group) == out.end()) {
                    out.push_back(std::move(group));
                }
            }
        }
        if (out.empty()) {
            for (const auto& role : roles_in_scope(identity, "")) {
                if (!role_is_internal(role)) {
                    return groups_for(role);
                }
            }
        }
        return out;
    }

    [[nodiscard]] std::optional<app::store::roles::RoleRecord> find_role_for_entity(
        const RequestIdentity& identity,
        std::string_view entity_type
    ) const
    {
        const auto normalized = normalize_entity(entity_type);
        for (const auto& role : roles_in_scope(identity, "")) {
            if (role_is_internal(role)) {
                continue;
            }
            if (normalized.empty() || role.get_entity_type() == normalized) {
                return role;
            }
        }
        return std::nullopt;
    }

    [[nodiscard]] std::string parent_of(std::string_view group) const
    {
        if (group.starts_with("operations")) {
            return "operations";
        }
        if (group.starts_with("connectors")) {
            return "connectors";
        }
        if (group.starts_with("workflows")) {
            return "workflows";
        }
        if (group.starts_with("users")) {
            return "users";
        }
        if (group.starts_with("account")) {
            return "account";
        }
        if (group.starts_with("analytics")) {
            return "analytics";
        }
        return "configurations";
    }

    [[nodiscard]] std::vector<GroupInfoWire> group_authorization_info() const
    {
        return {
            GroupInfoWire{"operations_view", "View Payments, Refunds, Payouts, Mandates, Disputes and Customers"},
            GroupInfoWire{"operations_manage", "Create, modify and delete Payments, Refunds, Payouts, Mandates, Disputes and Customers"},
            GroupInfoWire{"connectors_view", "View connected Payment Processors, Payout Processors and Fraud & Risk Manager details"},
            GroupInfoWire{"connectors_manage", "Create, modify and delete connectors like Payment Processors, Payout Processors and Fraud & Risk Manager"},
            GroupInfoWire{"workflows_view", "View Routing, 3DS Decision Manager, Surcharge Decision Manager"},
            GroupInfoWire{"workflows_manage", "Create, modify and delete Routing, 3DS Decision Manager, Surcharge Decision Manager"},
            GroupInfoWire{"analytics_view", "View Analytics"},
            GroupInfoWire{"users_view", "View Users"},
            GroupInfoWire{"users_manage", "Manage and invite Users to the Team"},
            GroupInfoWire{"account_view", "View Merchant Details"},
            GroupInfoWire{"account_manage", "Create, modify and delete Merchant Details like api keys, webhooks, etc"},
            GroupInfoWire{"theme_view", "View Themes"},
            GroupInfoWire{"theme_manage", "Manage Themes"},
            GroupInfoWire{"configurations_view", "View Configurations"},
            GroupInfoWire{"configurations_manage", "Create, modify and delete Configurations"},
        };
    }

    [[nodiscard]] std::vector<ParentInfoWire> parent_authorization_info() const
    {
        std::vector<ParentInfoWire> out{};
        for (const auto& group : group_authorization_info()) {
            const auto parent = parent_of(group.get_group());
            bool found{false};
            for (auto& item : out) {
                if (item.get_name() == parent) {
                    auto groups = item.get_groups();
                    groups.push_back(group.get_group());
                    item.set_groups(std::move(groups));
                    found = true;
                    break;
                }
            }
            if (!found) {
                ParentInfoWire item{};
                item.set_name(parent);
                item.set_description(parent_description(parent));
                item.set_groups({group.get_group()});
                out.push_back(std::move(item));
            }
        }
        return out;
    }

    [[nodiscard]] std::string parent_description(std::string_view parent) const
    {
        if (parent == "operations") {
            return "Payments, Refunds, Payouts, Mandates, Disputes and Customers";
        }
        if (parent == "connectors") {
            return "Create, modify and delete connectors like Payment Processors, Payout Processors and Fraud & Risk Manager";
        }
        if (parent == "workflows") {
            return "Create, modify and delete Routing, 3DS Decision Manager, Surcharge Decision Manager";
        }
        if (parent == "analytics") {
            return "View Analytics";
        }
        if (parent == "users") {
            return "Manage and invite Users to the Team";
        }
        if (parent == "account") {
            return "Create, modify and delete Merchant Details like api keys, webhooks, etc";
        }
        if (parent == "theme") {
            return "Manage and view themes for the organization";
        }
        return "Manage and view configurations";
    }

    [[nodiscard]] std::vector<ParentGroupDescriptionWire> parent_group_descriptions(
        std::string_view entity_type,
        std::string_view product_type
    ) const
    {
        std::vector<ParentGroupDescriptionWire> out{};
        for (const auto& group : group_authorization_info()) {
            if (!product_type.empty() && product_type == "orchestration"
                && (group.get_group().starts_with("recon") || group.get_group().starts_with("alerts"))) {
                continue;
            }
            const auto parent = parent_of(group.get_group());
            bool found{false};
            for (auto& item : out) {
                if (item.get_name() == parent) {
                    auto scopes = item.get_scopes();
                    scopes.push_back(group.get_group());
                    item.set_scopes(std::move(scopes));
                    found = true;
                    break;
                }
            }
            if (!found) {
                ParentGroupDescriptionWire item{};
                item.set_name(parent);
                item.set_description(parent_description(parent));
                item.set_scopes({group.get_group()});
                out.push_back(std::move(item));
            }
        }
        if (!entity_type.empty()) {
            // Deterministic product/entity filtering: organization scopes keep the
            // full set; narrower scopes drop the configuration-only descriptions.
            if (entity_type == "merchant" || entity_type == "profile") {
                std::erase_if(out, [](const ParentGroupDescriptionWire& item) {
                    return item.get_name() == "configurations";
                });
            }
        }
        return out;
    }

    [[nodiscard]] std::optional<core::error::AppError> validate_role_input(
        const CreateRoleInputWire& input,
        const RequestIdentity& identity
    ) const
    {
        if (input.get_role_name().empty() || input.get_role_name().size() > 64) {
            return error(core::error::AppErrorKind::VALIDATION_FAILED, "invalid role name");
        }
        if (input.get_groups().empty() && input.get_parent_groups().empty()) {
            return error(core::error::AppErrorKind::VALIDATION_FAILED, "role requires groups");
        }
        for (const auto& group : input.get_groups()) {
            if (group.empty()) {
                return error(core::error::AppErrorKind::VALIDATION_FAILED, "invalid permission group");
            }
        }
        if (const auto hierarchy = get_min_entity(requestor_entity(identity), normalize_entity(input.get_entity_type()))) {
            return hierarchy;
        }
        for (const auto& existing : m_context->repository().list_roles()) {
            if (existing.get_role_name() == input.get_role_name() && role_in_scope(existing, identity)) {
                return error(core::error::AppErrorKind::ALREADY_EXISTS, "role name already exists");
            }
        }
        return std::nullopt;
    }

    [[nodiscard]] std::string token_subject(interfaces::io::IRequest& req) const
    {
        const auto token = app::auth::TokenParser{}(req).value();
        return app::crypto::KeyedHasher{"user-role-token-pepper"}.hash(token);
    }

    [[nodiscard]] std::string invite_secret() const
    {
        const auto stored = m_context->key_store().get("jwt", "accept_invite");
        return stored ? stored->value() : std::string{"user-role-invite-secret"};
    }

    [[nodiscard]] std::string blacklist_key(std::string_view kind, std::string_view fingerprint) const
    {
        return std::string{"user_role.blacklist."} + std::string{kind} + ":"
               + app::crypto::KeyedHasher{"user-role-blacklist-pepper"}.hash(fingerprint);
    }

    [[nodiscard]] bool is_blacklisted(std::string_view kind, std::string_view fingerprint) const
    {
        return m_context->cache().get(blacklist_key(kind, fingerprint)).hit;
    }

    void blacklist(std::string_view kind, std::string_view fingerprint) const
    {
        m_context->cache().put(
            blacklist_key(kind, fingerprint),
            core::cache::to_cache_value(std::string{"1"}),
            core::cache::Ttl{}.seconds(3600)
        );
    }

    [[nodiscard]] std::string entity_id_for(
        const app::store::user_roles::UserRoleRecord& assignment
    ) const noexcept
    {
        if (!assignment.get_profile_id().empty()) {
            return assignment.get_profile_id();
        }
        if (!assignment.get_merchant_id().empty()) {
            return assignment.get_merchant_id();
        }
        return assignment.get_organization_id();
    }

    [[nodiscard]] std::optional<std::string> entity_name_for(
        const app::store::user_roles::UserRoleRecord& assignment
    ) const
    {
        if (!assignment.get_merchant_id().empty()) {
            const auto merchant = m_context->repository().find_merchant_account_by_string(
                assignment.get_merchant_id()
            );
            if (merchant) {
                return merchant->get_merchant_name();
            }
        }
        if (!assignment.get_organization_id().empty()) {
            const auto org = m_context->repository().find_organization_by_string(
                assignment.get_organization_id()
            );
            if (org) {
                return org->get_organization_name();
            }
        }
        return std::nullopt;
    }

    void list_minimal_roles(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send,
        std::string_view operation,
        bool updatable_only
    ) const
    {
        if (const auto auth_error = authorize(req, operation)) {
            send_error(res, send, *auth_error);
            return;
        }
        const auto identity = identity_from(req);
        const auto query = app::http::RequestQuery{}(req);
        const auto entity_type = normalize_entity(query.get("entity_type").value_or(""));
        if (const auto hierarchy = get_min_entity(requestor_entity(identity), entity_type)) {
            send_error(res, send, *hierarchy);
            return;
        }
        std::vector<MinimalRoleInfoWire> response{};
        for (const auto& role : roles_in_scope(identity, entity_type)) {
            if (role_is_internal(role)) {
                continue;
            }
            if (updatable_only && !role_is_updatable(role)) {
                continue;
            }
            if (!updatable_only && !role_is_deletable(role)) {
                continue;
            }
            MinimalRoleInfoWire item{};
            item.set_role_id(role.get_role_id());
            item.set_role_name(role.get_role_name());
            response.push_back(std::move(item));
        }
        auto text = serde::Ser::encode_json(response);
        track_flow(std::string{operation}, std::string{"user_role."} + std::string{operation});
        send_json(res, send, text, true);
    }

    [[nodiscard]] std::optional<std::string> role_id_from_path(interfaces::io::IRequest& req) const
    {
        const auto segments = app::http::RequestPath{}(req);
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

    void send_error(
        interfaces::io::IResponse& res,
        std::function<void()> send,
        core::error::AppError err
    ) const
    {
        auto text = serde::Ser::encode_json(err);
        auto bytes = serde::Ser::serialize_raw("application/json", text);
        res.set_status(core::error::AppErrorStatus{}(err.get_kind()));
        res.set_body(std::move(bytes));
        send();
    }

    void send_json(
        interfaces::io::IResponse& res,
        std::function<void()> send,
        std::string_view text,
        bool ok
    ) const
    {
        auto bytes = serde::Ser::serialize_raw("application/json", text);
        res.set_status(ok ? interfaces::io::types::Status::OK
                          : interfaces::io::types::Status::BAD_REQUEST);
        res.set_body(std::move(bytes));
        send();
    }

    app::context::AppContext* m_context{nullptr};
};

} // namespace app::handlers::user_role
