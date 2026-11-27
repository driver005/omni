export module app_user:user_role_runtime_list_invitations_for_user;

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

class ListInvitationsForUserRuntime
{
public:
    explicit ListInvitationsForUserRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] std::expected<std::vector<InvitationWire>, core::error::AppError>
    list_invitations_for_user(const ListInvitationsForUserInput& input) const
    {
        std::vector<InvitationWire> response{};
        std::set<std::string> seen{};
        for (const auto& assignment: m_context.repository().list_user_roles()) {
            if (assignment.get_user_id() != input.identity.user_id ||
                assignment.get_status() != "invitation_sent") {
                continue;
            }
            if (seen.contains(assignment.get_user_role_id())) {
                continue;
            }
            seen.insert(assignment.get_user_role_id());
            const auto [entity_id_value, entity_type] = m_support.entity_id_and_type(assignment);
            if (entity_id_value.empty() || entity_type.empty()) {
                continue;
            }
            if (entity_type == "tenant") {
                return std::unexpected{m_support.error(
                    core::error::AppErrorKind::INTERNAL,
                    "Tenant roles are not allowed"
                )};
            }
            InvitationWire item{};
            item.set_entity_id(entity_id_value);
            item.set_entity_type(entity_type);
            item.set_entity_name(m_support.entity_name_for(assignment, entity_type));
            item.set_role_id(assignment.get_role_id());
            response.push_back(std::move(item));
        }
        m_support.track_flow("list_invitations_for_user", "user_role.list_invitations_for_user");
        return std::move(response);
    }

private:
    ::app::context::AppContext& m_context;
    UserRoleRuntimeSupport m_support;
};

} // namespace app::handlers::user_role
