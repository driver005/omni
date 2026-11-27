export module app_user:user_role_runtime_list_users_in_lineage;

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

class ListUsersInLineageRuntime
{
public:
    explicit ListUsersInLineageRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] core::behavior::Outcome<serde::Value>
    list_users_in_lineage(const ::core::behavior::CallContext& ctx) const
    {
        const auto requestor = m_support.role_info_from_id(identity_from_context(ctx), identity_from_context(ctx).role_id);
        if (!requestor) {
            return std::unexpected{
                m_support.error(core::error::AppErrorKind::INTERNAL, "InvalidRoleId")
            };
        }
        const auto entity_type = m_support.normalize_entity(ctx_string(ctx,"entity_type"));
        if (const auto err = m_support.get_min_entity(requestor->entity_type, entity_type)) {
            return std::unexpected{*err};
        }
        const auto min_entity = entity_type.empty() ? requestor->entity_type
                                                    : std::min(requestor->entity_type, entity_type);
        std::map<std::string, std::vector<MinimalRoleInfoWire>> by_user{};
        for (const auto& assignment: m_context.repository().list_user_roles()) {
            if (!m_support.assignment_in_scope(assignment, identity_from_context(ctx))) {
                continue;
            }
            if (!m_support.lineage_matches_entity(assignment, min_entity)) {
                continue;
            }
            const auto role = m_support.role_info_from_id(identity_from_context(ctx), assignment.get_role_id());
            if (!role) {
                continue;
            }
            if (role_tables::entity_rank(role->entity_type) >
                role_tables::entity_rank(requestor->entity_type)) {
                continue;
            }
            MinimalRoleInfoWire info{};
            info.set_role_id(role->role_id);
            info.set_role_name(role->role_name);
            by_user[assignment.get_user_id()].push_back(std::move(info));
        }
        std::vector<ListUsersInEntityWire> response{};
        for (const auto& [user_id, roles]: by_user) {
            const auto user = m_context.repository().find_user_by_string(user_id);
            if (!user || user->get_status() != "active") {
                continue;
            }
            ListUsersInEntityWire item{};
            item.set_email(user->get_email());
            item.set_roles(roles);
            response.push_back(std::move(item));
        }
        m_support.track_flow("list_users_in_lineage", "user_role.list_users_in_lineage");
        return to_document_array(response, kListUsersInEntityWireFields);
    }

private:
    ::app::context::AppContext& m_context;
    UserRoleRuntimeSupport m_support;
};

} // namespace app::handlers::user_role
