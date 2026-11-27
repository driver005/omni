module;
#include <rfl/to_generic.hpp>

export module app_user:user_role_result;

import std;
import core_serde;
import app_dto;

import :user_role_input;

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

// Wire field lists captured from the former Result Serializable specializations.
// Each Result reused the full FieldDesc tuple of its underlying model, so these
// lists hold the exact declared field names in Serializable<T>::fields() order.

inline constexpr std::array<std::string_view, 2> kGroupsAndResourcesWireFields{
    "groups",
    "resources",
};

inline constexpr std::array<std::string_view, 5> kRoleInfoWithGroupsWireFields{
    "role_id",
    "groups",
    "role_name",
    "role_scope",
    "entity_type",
};

inline constexpr std::array<std::string_view, 4> kRoleInfoWithParentsWireFields{
    "role_id",
    "parent_groups",
    "role_name",
    "role_scope",
};

inline constexpr std::array<std::string_view, 1> kStatusOkWireFields{
    "status",
};

inline constexpr std::array<std::string_view, 2> kTokenResponseWireFields{
    "token",
    "token_type",
};

inline constexpr std::array<std::string_view, 1> kDeleteUserRoleResponseWireFields{
    "is_email_sent",
};

inline constexpr std::array<std::string_view, 2> kGroupInfoWireFields{
    "group",
    "description",
};

inline constexpr std::array<std::string_view, 3> kParentInfoWireFields{
    "name",
    "description",
    "groups",
};

inline constexpr std::array<std::string_view, 3> kParentGroupDescriptionWireFields{
    "name",
    "description",
    "scopes",
};

inline constexpr std::array<std::string_view, 3> kParentGroupInfoWireFields{
    "name",
    "resources",
    "scopes",
};

inline constexpr std::array<std::string_view, 2> kListUsersInEntityWireFields{
    "email",
    "roles",
};

inline constexpr std::array<std::string_view, 2> kMinimalRoleInfoWireFields{
    "role_id",
    "role_name",
};

inline constexpr std::array<std::string_view, 4> kInvitationWireFields{
    "entity_id",
    "entity_type",
    "entity_name",
    "role_id",
};

} // namespace app::handlers::user_role
