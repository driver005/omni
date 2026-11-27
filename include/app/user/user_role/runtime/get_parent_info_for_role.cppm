export module app_user:user_role_runtime_get_parent_info_for_role;

import std;
import core_behavior;
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

    [[nodiscard]] core::behavior::Outcome<RoleInfoWithParentsWire>
    get_parent_info_for_role(const ::core::behavior::CallContext& ctx) const
    {
        if (ctx_string(ctx, "role_id").empty()) {
            return std::unexpected{
                m_support.error(core::error::AppErrorKind::NOT_FOUND, "InvalidRoleId")
            };
        }
        const auto role =
            m_support.role_info_from_id(identity_from_context(ctx), ctx_string(ctx, "role_id"));
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
        return std::move(response);
    }

private:
    ::app::context::AppContext& m_context;
    UserRoleRuntimeSupport m_support;
};

} // namespace app::handlers::user_role
