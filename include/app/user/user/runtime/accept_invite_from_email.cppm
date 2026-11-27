export module app_user:user_runtime_accept_invite_from_email;

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
import app_connector;
import app_events;
import app_workflow;
import app_keystore;
import app_locking;
import app_services;

import :user_input;
import :user_result;
import :user_runtime_common;

export namespace app::handlers::user {

class AcceptInviteFromEmailRuntime
{
public:
    explicit AcceptInviteFromEmailRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] core::behavior::Outcome<std::pair<serde::Value, std::string>>
    accept_invite_from_email(const ::app::model::user::AcceptInviteFromEmailRequest& request,
        const ::core::behavior::CallContext& ctx) const
    {
        std::string set_cookie{};


        if (!m_support.user_signer().verify(request.get_token())) {
            return std::unexpected{m_support.wire_error(
                "UR_04",
                "Invalid or expired link",
                core::error::AppErrorKind::UNAUTHORIZED
            )};
        }

        const auto email_entry =
            m_context.cache().get("user.invite_token_email:" + request.get_token());
        if (email_entry.hit) {
            const auto email = core::cache::value_as<std::string>(email_entry.value, std::string{});
            m_support.set_user_status_by_email(email, "active");
            m_support.publish_identity_event("user.invite_accepted", email);
        }

        m_context.metrics().record_flow("accept_invite_from_email");
        m_context.metrics().counter("user.accept_invite_from_email").increment();

        using Response = ::app::model::user::TokenResponse;
        Response response{};
        ::app::model::user::TokenPurposeValue purpose{};
        purpose.set_value("LOGIN");
        response.set_token(request.get_token());
        response.set_token_type(purpose);
        auto document = ResponseDocument::from(response);
        return std::pair{document, set_cookie};
    }

private:
    ::app::context::AppContext& m_context;
    UserRuntimeSupport m_support;
};

} // namespace app::handlers::user
