export module app_user:user_runtime_terminate_accept_invite;

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

class TerminateAcceptInviteRuntime
{
public:
    explicit TerminateAcceptInviteRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] std::expected<std::pair<serde::Value, std::string>, core::error::AppError>
    terminate_accept_invite(const TerminateAcceptInviteInput& input) const
    {
        std::string set_cookie{};


        std::string invite_token{};
        if (const auto* object = std::get_if<interfaces::Value::Object>(&input.generic.get())) {
            if (const auto field = object->get("token")) {
                if (const auto* text = std::get_if<std::string>(&field.value().get())) {
                    invite_token = *text;
                }
            }
        }
        const auto email_entry = m_context.cache().get("user.invite_token_email:" + invite_token);
        if (email_entry.hit) {
            const auto email = core::cache::value_as<std::string>(email_entry.value, std::string{});
            m_support.set_user_status_by_email(email, "invite_terminated");
            m_support.publish_identity_event("user.invite_terminated", email);
        }

        m_context.metrics().record_flow("terminate_accept_invite");
        m_context.metrics().counter("user.terminate_accept_invite").increment();

        auto document = ResponseDocument::from(input.generic);
        return std::pair{document, set_cookie};
    }

private:
    ::app::context::AppContext& m_context;
    UserRuntimeSupport m_support;
};

} // namespace app::handlers::user
