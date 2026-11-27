module;
export module app_dto:user_role;

import core_serde;
import core_behavior;

import std;
import core_serde;
import core_behavior;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace app::dto::user_role {

enum class AuthorizationInfo
{
    Group,
    GroupWithTag,
};

class DeleteUserRoleResponse : public core::behavior::Serializer<DeleteUserRoleResponse>
{
public:
    void set_is_email_sent(bool value)
    {
        is_email_sent = std::move(value);
    }

    [[nodiscard]] const bool& get_is_email_sent() const noexcept
    {
        return is_email_sent;
    }

    bool is_email_sent{};
};

class EmailValue : public core::behavior::Serializer<EmailValue>
{
public:
    void set_value(std::string v)
    {
        value = std::move(v);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

class EntityTypeValue : public core::behavior::Serializer<EntityTypeValue>
{
public:
    void set_value(std::string v)
    {
        value = std::move(v);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

class MinimalRoleInfoValue : public core::behavior::Serializer<MinimalRoleInfoValue>
{
public:
    void set_value(std::string v)
    {
        value = std::move(v);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

class ParentGroupValue : public core::behavior::Serializer<ParentGroupValue>
{
public:
    void set_value(std::string v)
    {
        value = std::move(v);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

class PermissionGroupValue : public core::behavior::Serializer<PermissionGroupValue>
{
public:
    void set_value(std::string v)
    {
        value = std::move(v);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

class SecretStringValue : public core::behavior::Serializer<SecretStringValue>
{
public:
    void set_value(std::string v)
    {
        value = std::move(v);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

enum class UserStatus
{
    Active,
    InvitationSent,
};

class AuthorizationInfoResponse : public core::behavior::Serializer<AuthorizationInfoResponse>
{
public:
    void set_value(std::vector<AuthorizationInfo> v)
    {
        value = std::move(v);
    }

    [[nodiscard]] const std::vector<AuthorizationInfo>& get_value() const noexcept
    {
        return value;
    }

    std::vector<AuthorizationInfo> value{};
};

class DeleteUserRoleRequest : public core::behavior::Serializer<DeleteUserRoleRequest>
{
public:
    void set_email(EmailValue value)
    {
        email = std::move(value);
    }

    [[nodiscard]] const EmailValue& get_email() const noexcept
    {
        return email;
    }

    EmailValue email{};
};

class Entity : public core::behavior::Serializer<Entity>
{
public:
    void set_entity_id(std::string value)
    {
        entity_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_entity_id() const noexcept
    {
        return entity_id;
    }

    void set_entity_type(EntityTypeValue value)
    {
        entity_type = std::move(value);
    }

    [[nodiscard]] const EntityTypeValue& get_entity_type() const noexcept
    {
        return entity_type;
    }

    std::string entity_id{};
    EntityTypeValue entity_type{};
};

class GroupInfo : public core::behavior::Serializer<GroupInfo>
{
public:
    void set_group(PermissionGroupValue value)
    {
        group = std::move(value);
    }

    [[nodiscard]] const PermissionGroupValue& get_group() const noexcept
    {
        return group;
    }

    void set_description(std::string value)
    {
        description = std::move(value);
    }

    [[nodiscard]] const std::string& get_description() const noexcept
    {
        return description;
    }

    PermissionGroupValue group{};
    std::string description{};
};

class ListInvitationForUserResponse :
    public core::behavior::Serializer<ListInvitationForUserResponse>
{
public:
    void set_entity_id(std::string value)
    {
        entity_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_entity_id() const noexcept
    {
        return entity_id;
    }

    void set_entity_type(EntityTypeValue value)
    {
        entity_type = std::move(value);
    }

    [[nodiscard]] const EntityTypeValue& get_entity_type() const noexcept
    {
        return entity_type;
    }

    void set_entity_name(std::optional<SecretStringValue> value)
    {
        entity_name = std::move(value);
    }

    [[nodiscard]] const std::optional<SecretStringValue>& get_entity_name() const noexcept
    {
        return entity_name;
    }

    void set_role_id(std::string value)
    {
        role_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_role_id() const noexcept
    {
        return role_id;
    }

    std::string entity_id{};
    EntityTypeValue entity_type{};
    std::optional<SecretStringValue> entity_name{};
    std::string role_id{};
};

class ListUsersInEntityRequest : public core::behavior::Serializer<ListUsersInEntityRequest>
{
public:
    void set_entity_type(std::optional<EntityTypeValue> value)
    {
        entity_type = std::move(value);
    }

    [[nodiscard]] const std::optional<EntityTypeValue>& get_entity_type() const noexcept
    {
        return entity_type;
    }

    std::optional<EntityTypeValue> entity_type{};
};

class ListUsersInEntityResponse : public core::behavior::Serializer<ListUsersInEntityResponse>
{
public:
    void set_email(EmailValue value)
    {
        email = std::move(value);
    }

    [[nodiscard]] const EmailValue& get_email() const noexcept
    {
        return email;
    }

    void set_roles(std::vector<MinimalRoleInfoValue> value)
    {
        roles = std::move(value);
    }

    [[nodiscard]] const std::vector<MinimalRoleInfoValue>& get_roles() const noexcept
    {
        return roles;
    }

    EmailValue email{};
    std::vector<MinimalRoleInfoValue> roles{};
};

class ParentInfo : public core::behavior::Serializer<ParentInfo>
{
public:
    void set_name(ParentGroupValue value)
    {
        name = std::move(value);
    }

    [[nodiscard]] const ParentGroupValue& get_name() const noexcept
    {
        return name;
    }

    void set_description(std::string value)
    {
        description = std::move(value);
    }

    [[nodiscard]] const std::string& get_description() const noexcept
    {
        return description;
    }

    void set_groups(std::vector<PermissionGroupValue> value)
    {
        groups = std::move(value);
    }

    [[nodiscard]] const std::vector<PermissionGroupValue>& get_groups() const noexcept
    {
        return groups;
    }

    ParentGroupValue name{};
    std::string description{};
    std::vector<PermissionGroupValue> groups{};
};

class UpdateUserRoleRequest : public core::behavior::Serializer<UpdateUserRoleRequest>
{
public:
    void set_email(EmailValue value)
    {
        email = std::move(value);
    }

    [[nodiscard]] const EmailValue& get_email() const noexcept
    {
        return email;
    }

    void set_role_id(std::string value)
    {
        role_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_role_id() const noexcept
    {
        return role_id;
    }

    EmailValue email{};
    std::string role_id{};
};
} // namespace app::dto::user_role

template<>
struct serde::Serializable<app::dto::user_role::PermissionGroupValue>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<
            "value",
            &app::dto::user_role::PermissionGroupValue::get_value,
            &app::dto::user_role::PermissionGroupValue::set_value,
            serde::FieldOptions{}>{}};
    }

    static constexpr std::string_view table_name()
    {
        return "permission_group_value";
    }
};

template<>
struct serde::Serializable<app::dto::user_role::ParentGroupValue>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<
            "value",
            &app::dto::user_role::ParentGroupValue::get_value,
            &app::dto::user_role::ParentGroupValue::set_value,
            serde::FieldOptions{}>{}};
    }

    static constexpr std::string_view table_name()
    {
        return "parent_group_value";
    }
};

template<>
struct serde::Serializable<app::dto::user_role::EmailValue>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<
            "value",
            &app::dto::user_role::EmailValue::get_value,
            &app::dto::user_role::EmailValue::set_value,
            serde::FieldOptions{}>{}};
    }

    static constexpr std::string_view table_name()
    {
        return "email_value";
    }
};

template<>
struct serde::Serializable<app::dto::user_role::EntityTypeValue>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<
            "value",
            &app::dto::user_role::EntityTypeValue::get_value,
            &app::dto::user_role::EntityTypeValue::set_value,
            serde::FieldOptions{}>{}};
    }

    static constexpr std::string_view table_name()
    {
        return "entity_type_value";
    }
};

template<>
struct serde::Serializable<app::dto::user_role::SecretStringValue>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<
            "value",
            &app::dto::user_role::SecretStringValue::get_value,
            &app::dto::user_role::SecretStringValue::set_value,
            serde::FieldOptions{}>{}};
    }

    static constexpr std::string_view table_name()
    {
        return "secret_string_value";
    }
};

template<>
struct serde::Serializable<app::dto::user_role::MinimalRoleInfoValue>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<
            "value",
            &app::dto::user_role::MinimalRoleInfoValue::get_value,
            &app::dto::user_role::MinimalRoleInfoValue::set_value,
            serde::FieldOptions{}>{}};
    }

    static constexpr std::string_view table_name()
    {
        return "minimal_role_info_value";
    }
};

template<>
struct serde::Serializable<app::dto::user_role::AuthorizationInfoResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<
            "value",
            &app::dto::user_role::AuthorizationInfoResponse::get_value,
            &app::dto::user_role::AuthorizationInfoResponse::set_value,
            serde::FieldOptions{}>{}};
    }

    static constexpr std::string_view table_name()
    {
        return "authorization_info_response";
    }
};

template<>
struct serde::Serializable<app::dto::user_role::GroupInfo>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "group",
                &app::dto::user_role::GroupInfo::get_group,
                &app::dto::user_role::GroupInfo::set_group,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "description",
                &app::dto::user_role::GroupInfo::get_description,
                &app::dto::user_role::GroupInfo::set_description,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "group_info";
    }
};

template<>
struct serde::Serializable<app::dto::user_role::ParentInfo>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "name",
                &app::dto::user_role::ParentInfo::get_name,
                &app::dto::user_role::ParentInfo::set_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "description",
                &app::dto::user_role::ParentInfo::get_description,
                &app::dto::user_role::ParentInfo::set_description,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "groups",
                &app::dto::user_role::ParentInfo::get_groups,
                &app::dto::user_role::ParentInfo::set_groups,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "parent_info";
    }
};

template<>
struct serde::Serializable<app::dto::user_role::UpdateUserRoleRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "email",
                &app::dto::user_role::UpdateUserRoleRequest::get_email,
                &app::dto::user_role::UpdateUserRoleRequest::set_email,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "role_id",
                &app::dto::user_role::UpdateUserRoleRequest::get_role_id,
                &app::dto::user_role::UpdateUserRoleRequest::set_role_id,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "update_user_role_request";
    }
};

template<>
struct serde::Serializable<app::dto::user_role::DeleteUserRoleRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<
            "email",
            &app::dto::user_role::DeleteUserRoleRequest::get_email,
            &app::dto::user_role::DeleteUserRoleRequest::set_email,
            serde::FieldOptions{}>{}};
    }

    static constexpr std::string_view table_name()
    {
        return "delete_user_role_request";
    }
};

template<>
struct serde::Serializable<app::dto::user_role::DeleteUserRoleResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<
            "is_email_sent",
            &app::dto::user_role::DeleteUserRoleResponse::get_is_email_sent,
            &app::dto::user_role::DeleteUserRoleResponse::set_is_email_sent,
            serde::FieldOptions{}>{}};
    }

    static constexpr std::string_view table_name()
    {
        return "delete_user_role_response";
    }
};

template<>
struct serde::Serializable<app::dto::user_role::ListUsersInEntityResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "email",
                &app::dto::user_role::ListUsersInEntityResponse::get_email,
                &app::dto::user_role::ListUsersInEntityResponse::set_email,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "roles",
                &app::dto::user_role::ListUsersInEntityResponse::get_roles,
                &app::dto::user_role::ListUsersInEntityResponse::set_roles,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "list_users_in_entity_response";
    }
};

template<>
struct serde::Serializable<app::dto::user_role::ListInvitationForUserResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "entity_id",
                &app::dto::user_role::ListInvitationForUserResponse::get_entity_id,
                &app::dto::user_role::ListInvitationForUserResponse::set_entity_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "entity_type",
                &app::dto::user_role::ListInvitationForUserResponse::get_entity_type,
                &app::dto::user_role::ListInvitationForUserResponse::set_entity_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "entity_name",
                &app::dto::user_role::ListInvitationForUserResponse::get_entity_name,
                &app::dto::user_role::ListInvitationForUserResponse::set_entity_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "role_id",
                &app::dto::user_role::ListInvitationForUserResponse::get_role_id,
                &app::dto::user_role::ListInvitationForUserResponse::set_role_id,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "list_invitation_for_user_response";
    }
};

template<>
struct serde::Serializable<app::dto::user_role::Entity>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "entity_id",
                &app::dto::user_role::Entity::get_entity_id,
                &app::dto::user_role::Entity::set_entity_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "entity_type",
                &app::dto::user_role::Entity::get_entity_type,
                &app::dto::user_role::Entity::set_entity_type,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "entity";
    }
};

template<>
struct serde::Serializable<app::dto::user_role::ListUsersInEntityRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<
            "entity_type",
            &app::dto::user_role::ListUsersInEntityRequest::get_entity_type,
            &app::dto::user_role::ListUsersInEntityRequest::set_entity_type,
            serde::FieldOptions{}>{}};
    }

    static constexpr std::string_view table_name()
    {
        return "list_users_in_entity_request";
    }
};

#ifdef CONGELADO_TEST
namespace app::dto::user_role::tests {
using namespace boost::ut;
suite<"app_dto_user_role"> app_dto_user_role_suite = []
{
    "reflects converted types"_test = []
    {
        expect(
            std::tuple_size_v<decltype(serde::Serializable<
                                       app::dto::user_role::AuthorizationInfoResponse>::fields())> >
            0
        );
    };
};
} // namespace app::dto::user_role::tests
#endif
