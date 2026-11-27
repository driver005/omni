export module app_user:user_role_runtime_update_user_role;

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

class UpdateUserRoleRuntime
{
public:
    explicit UpdateUserRoleRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] std::expected<StatusOkWire, core::error::AppError>
    update_user_role(const UpdateUserRoleInput& input) const
    {
        const ::app::crypto::SecretString email_secret{input.request.get_email().get_value()};
        const auto& role_id = input.request.get_role_id();
        if (email_secret.value().empty() || role_id.empty()) {
            return std::unexpected{m_support.error(
                core::error::AppErrorKind::PRECONDITION_FAILED,
                "InvalidRoleOperation"
            )};
        }
        const auto target_role = m_support.role_info_from_id(input.identity, role_id);
        if (!target_role) {
            return std::unexpected{
                m_support.error(core::error::AppErrorKind::NOT_FOUND, "InvalidRoleId")
            };
        }
        if (!target_role->is_updatable) {
            return std::unexpected{m_support.error(
                core::error::AppErrorKind::PRECONDITION_FAILED,
                "InvalidRoleOperation"
            )};
        }
        const auto user = m_support.active_user_by_email(email_secret.value());
        if (!user) {
            return std::unexpected{
                m_support.error(core::error::AppErrorKind::NOT_FOUND, "InvalidRoleOperation")
            };
        }
        if (input.identity.user_id == user->get_user_id()) {
            return std::unexpected{m_support.error(
                core::error::AppErrorKind::PRECONDITION_FAILED,
                "User Changing their own role"
            )};
        }
        const auto updator = m_support.role_info_from_id(input.identity, input.identity.role_id);
        if (!updator) {
            return std::unexpected{
                m_support.error(core::error::AppErrorKind::INTERNAL, "InvalidRoleId")
            };
        }
        bool updated{false};
        for (auto assignment: m_context.repository().list_user_roles()) {
            if (assignment.get_user_id() != user->get_user_id() ||
                !m_support.assignment_in_scope(assignment, input.identity)) {
                continue;
            }
            const auto current =
                m_support.role_info_from_id(input.identity, assignment.get_role_id());
            if (!current || !current->is_updatable) {
                return std::unexpected{m_support.error(
                    core::error::AppErrorKind::PRECONDITION_FAILED,
                    "InvalidRoleOperation"
                )};
            }
            if (current->entity_type != target_role->entity_type) {
                return std::unexpected{m_support.error(
                    core::error::AppErrorKind::PRECONDITION_FAILED,
                    "Upgrade and downgrade of roles is not allowed"
                )};
            }
            if (role_tables::entity_rank(updator->entity_type) <
                role_tables::entity_rank(current->entity_type)) {
                return std::unexpected{m_support.error(
                    core::error::AppErrorKind::PRECONDITION_FAILED,
                    "InvalidRoleOperation"
                )};
            }
            assignment.set_role_id(role_id);
            m_context.repository().update_user_role(assignment);
            updated = true;
        }
        if (!updated) {
            return std::unexpected{m_support.error(
                core::error::AppErrorKind::NOT_FOUND,
                "User with given email is not found in the organization"
            )};
        }
        m_support.blacklist("user", user->get_user_id());
        StatusOkWire response{};
        response.set_status("Ok");
        m_support.track_flow("update_user_role", "user_role.update_user_role");
        return std::move(response);
    }

private:
    ::app::context::AppContext& m_context;
    UserRoleRuntimeSupport m_support;
};

} // namespace app::handlers::user_role
