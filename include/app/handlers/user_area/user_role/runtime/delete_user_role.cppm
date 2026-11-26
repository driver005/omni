export module app_user:user_role_runtime_delete_user_role;

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

class DeleteUserRoleRuntime
{
public:
    explicit DeleteUserRoleRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] std::expected<DeleteUserRoleResult, core::error::AppError>
    delete_user_role(const DeleteUserRoleInput& input) const
    {
        const ::app::crypto::SecretString email_secret{input.request.get_email().get_value()};
        const auto user = m_support.active_user_by_email(email_secret.value());
        if (!user) {
            return std::unexpected{
                m_support.error(core::error::AppErrorKind::NOT_FOUND, "InvalidRoleOperation")
            };
        }
        if (input.identity.user_id == user->get_user_id()) {
            return std::unexpected{m_support.error(
                core::error::AppErrorKind::PRECONDITION_FAILED,
                "User deleting himself"
            )};
        }
        const auto requestor = m_support.role_info_from_id(input.identity, input.identity.role_id);
        if (!requestor) {
            return std::unexpected{
                m_support.error(core::error::AppErrorKind::INTERNAL, "InvalidRoleId")
            };
        }
        std::optional<role_tables::RoleInfo> deleted_role{};
        std::vector<std::string> removed{};
        for (const auto& assignment: m_context.repository().list_user_roles()) {
            if (assignment.get_user_id() != user->get_user_id() ||
                !m_support.assignment_in_scope(assignment, input.identity)) {
                continue;
            }
            const auto target =
                m_support.role_info_from_id(input.identity, assignment.get_role_id());
            if (!target || target->is_internal || !target->is_deletable) {
                return std::unexpected{m_support.error(
                    core::error::AppErrorKind::PRECONDITION_FAILED,
                    "InvalidDeleteOperation"
                )};
            }
            if (role_tables::entity_rank(requestor->entity_type) <
                role_tables::entity_rank(target->entity_type)) {
                return std::unexpected{m_support.error(
                    core::error::AppErrorKind::PRECONDITION_FAILED,
                    "InvalidDeleteOperation"
                )};
            }
            if (!deleted_role) {
                deleted_role = *target;
            }
            removed.push_back(assignment.get_role_id());
            m_context.repository().remove_user_role(assignment.get_id());
        }
        if (removed.empty()) {
            return std::unexpected{m_support.error(
                core::error::AppErrorKind::PRECONDITION_FAILED,
                "User is not associated with the merchant"
            )};
        }
        const auto payload = std::format(
            R"({{"email":"{}","merchant_id":"{}","role_id":"{}"}})",
            email_secret.value(),
            input.identity.merchant_id,
            deleted_role->role_id
        );
        const auto event_id = m_context.events().publish("user_role.deleted", payload);
        bool is_email_sent = !event_id.empty();
        bool remains{false};
        for (const auto& assignment: m_context.repository().list_user_roles()) {
            if (assignment.get_user_id() == user->get_user_id()) {
                remains = true;
                break;
            }
        }
        if (!remains) {
            auto deactivated = *user;
            deactivated.set_status("inactive");
            m_context.repository().update_user(deactivated);
            m_context.events().publish(
                "user.metadata.deleted",
                std::format(R"({{"user_id":"{}"}})", user->get_user_id())
            );
        }
        m_support.blacklist("user", user->get_user_id());
        ::app::dto::user_role::DeleteUserRoleResponse response{};
        response.set_is_email_sent(is_email_sent);
        m_support.track_flow("delete_user_role", "user_role.delete_user_role");
        return DeleteUserRoleResult{std::move(response)};
    }

private:
    ::app::context::AppContext& m_context;
    UserRoleRuntimeSupport m_support;
};

} // namespace app::handlers::user_role
