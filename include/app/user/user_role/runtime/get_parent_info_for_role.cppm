export module app_user:user_role_runtime_get_parent_info_for_role;

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

import :user_role_input;
import :user_role_result;
import :user_role_runtime_common;

export namespace app::handlers::user_role {

class GetParentInfoForRoleRuntime
{
public:
    explicit GetParentInfoForRoleRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] std::expected<GetParentInfoForRoleResult, core::error::AppError>
    get_parent_info_for_role(const GetParentInfoForRoleInput& input) const
    {
        if (input.role_id.empty()) {
            return std::unexpected{
                m_support.error(core::error::AppErrorKind::NOT_FOUND, "InvalidRoleId")
            };
        }
        const auto role = m_support.role_info_from_id(input.identity, input.role_id);
        if (!role || role->is_internal) {
            return std::unexpected{
                m_support.error(core::error::AppErrorKind::NOT_FOUND, "InvalidRoleId")
            };
        }
        RoleInfoWithParentsWire response{};
        response.set_role_id(role->role_id);
        response.set_parent_groups(
            role_tables::descriptions_for_groups(role->entity_type, role->permission_groups())
        );
        response.set_role_name(role->role_name);
        response.set_role_scope(role->scope);
        m_support.track_flow("get_parent_info_for_role", "user_role.get_parent_info_for_role");
        return GetParentInfoForRoleResult{std::move(response)};
    }

private:
    ::app::context::AppContext& m_context;
    UserRoleRuntimeSupport m_support;
};

} // namespace app::handlers::user_role
