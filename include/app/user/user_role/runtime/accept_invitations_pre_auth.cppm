export module app_user:user_role_runtime_accept_invitations_pre_auth;

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

class AcceptInvitationsPreAuthRuntime
{
public:
    explicit AcceptInvitationsPreAuthRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] core::behavior::Outcome<std::pair<TokenResponseWire, std::string>>
    accept_invitations_pre_auth(
        const std::vector<AcceptEntityWire>& request,
        const ::core::behavior::CallContext& ctx
    ) const
    {
        const auto lineages = m_support.lineages_for_accept(identity_from_context(ctx), request);
        std::vector<std::string> activated{};
        for (auto assignment: m_context.repository().list_user_roles()) {
            if (assignment.get_user_id() != identity_from_context(ctx).user_id ||
                assignment.get_status() != "invitation_sent") {
                continue;
            }
            if (!m_support.lineage_matches(assignment, lineages)) {
                continue;
            }
            assignment.set_status("active");
            m_context.repository().update_user_role(assignment);
            activated.push_back(assignment.get_role_id());
        }
        if (activated.empty()) {
            return std::unexpected{
                m_support.error(core::error::AppErrorKind::NOT_FOUND, "MerchantIdNotFound")
            };
        }
        const auto user = m_support.active_user_by_id(identity_from_context(ctx).user_id);
        if (!user) {
            return std::unexpected{
                m_support.error(core::error::AppErrorKind::NOT_FOUND, "UserNotFound")
            };
        }
        const auto token = m_support.issue_flow_token(
            identity_from_context(ctx).user_id,
            "MerchantSelect",
            activated
        );
        const auto cookie = std::format("token={}; HttpOnly; Path=/; SameSite=Lax", token);
        TokenResponseWire response{};
        response.set_token(token);
        response.set_token_type("MerchantSelect");
        m_support.track_flow(
            "accept_invitations_pre_auth",
            "user_role.accept_invitations_pre_auth"
        );
        return std::pair{std::move(response), std::move(cookie)};
    }

private:
    ::app::context::AppContext& m_context;
    UserRoleRuntimeSupport m_support;
};

} // namespace app::handlers::user_role
