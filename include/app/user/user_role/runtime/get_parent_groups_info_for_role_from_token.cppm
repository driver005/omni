export module app_user:user_role_runtime_get_parent_groups_info_for_role_from_token;

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

class GetParentGroupsInfoForRoleFromTokenRuntime
{
public:
    explicit GetParentGroupsInfoForRoleFromTokenRuntime(
        ::app::context::AppContext& context
    ) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] core::behavior::Outcome<serde::Value>
    get_parent_groups_info_for_role_from_token(
        const ::core::behavior::CallContext& ctx
    ) const
    {
        const auto role = m_support.role_info_from_id(identity_from_context(ctx), identity_from_context(ctx).role_id);
        if (!role) {
            return std::unexpected{
                m_support.error(core::error::AppErrorKind::NOT_FOUND, "InvalidRoleId")
            };
        }
        auto parents = role_tables::permission_groups_to_parent_group_info(
            role->permission_groups(),
            role->entity_type
        );
        m_support.track_flow(
            "get_parent_groups_info_for_role_from_token",
            "user_role.get_parent_groups_info_for_role_from_token"
        );
        return to_document_array(parents, kParentGroupInfoWireFields);
    }

private:
    ::app::context::AppContext& m_context;
    UserRoleRuntimeSupport m_support;
};

} // namespace app::handlers::user_role
