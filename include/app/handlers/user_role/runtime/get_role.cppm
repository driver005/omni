export module app_handlers_user_role:runtime_get_role;

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

import :input;
import :result;
import :runtime_common;

export namespace app::handlers::user_role {

class GetRoleRuntime
{
public:
    explicit GetRoleRuntime(app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] std::expected<GetRoleResult, core::error::AppError>
    get_role(const GetRoleInput& input) const
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
        RoleInfoWithGroupsWire response{};
        response.set_role_id(role->role_id);
        response.set_groups(role->permission_groups());
        response.set_role_name(role->role_name);
        response.set_role_scope(role->scope);
        response.set_entity_type(role->entity_type);
        m_support.track_flow("get_role", "user_role.get_role");
        return GetRoleResult{std::move(response)};
    }

private:
    app::context::AppContext& m_context;
    UserRoleRuntimeSupport m_support;
};

} // namespace app::handlers::user_role
