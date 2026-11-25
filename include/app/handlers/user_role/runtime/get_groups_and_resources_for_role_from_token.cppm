export module app_handlers_user_role:runtime_get_groups_and_resources_for_role_from_token;

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

class GetGroupsAndResourcesForRoleFromTokenRuntime
{
public:
    explicit GetGroupsAndResourcesForRoleFromTokenRuntime(
        app::context::AppContext& context
    ) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] std::expected<GetGroupsAndResourcesForRoleFromTokenResult, core::error::AppError>
    get_groups_and_resources_for_role_from_token(
        const GetGroupsAndResourcesForRoleFromTokenInput& input
    ) const
    {
        const auto role = m_support.role_info_from_id(input.identity, input.identity.role_id);
        if (!role) {
            return std::unexpected{
                m_support.error(core::error::AppErrorKind::NOT_FOUND, "InvalidRoleId")
            };
        }
        const auto groups = role->permission_groups();
        std::vector<std::string> resources{};
        for (const auto& group: groups) {
            const auto* def = role_tables::perm_group(group);
            if (def == nullptr) {
                continue;
            }
            for (const auto res_name: role_tables::parent_resources(def->parent)) {
                const std::string value{res_name};
                if (std::find(resources.begin(), resources.end(), value) == resources.end()) {
                    resources.push_back(value);
                }
            }
        }
        GroupsAndResourcesWire response{};
        response.set_groups(groups);
        response.set_resources(std::move(resources));
        m_support.track_flow(
            "get_groups_and_resources_for_role_from_token",
            "user_role.get_groups_and_resources_for_role_from_token"
        );
        return GetGroupsAndResourcesForRoleFromTokenResult{std::move(response)};
    }

private:
    app::context::AppContext& m_context;
    UserRoleRuntimeSupport m_support;
};

} // namespace app::handlers::user_role
