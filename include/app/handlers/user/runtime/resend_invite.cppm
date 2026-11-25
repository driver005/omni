export module app_handlers_user:runtime_resend_invite;

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

import :input;
import :result;
import :runtime_common;

export namespace app::handlers::user {

class ResendInviteRuntime
{
public:
    explicit ResendInviteRuntime(app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] std::expected<ResendInviteResult, core::error::AppError>
    resend_invite(const ResendInviteInput& input) const
    {
        std::string set_cookie{};


        const auto email = input.request.get_email().get_value();
        const auto resend_payload = std::string{R"({"email":")"} + email + R"("})";
        const auto resend_token = m_support.user_signer().sign(resend_payload);
        m_context.cache().put(
            "user.invite_token:" + email,
            core::cache::to_cache_value(resend_token)
        );
        m_context.cache().put(
            "user.invite_token_email:" + resend_token,
            core::cache::to_cache_value(email)
        );
        m_support.set_user_status_by_email(email, "invited");
        m_support.publish_identity_event("user.invite_resent", email);

        const auto key =
            std::string{"user.resend_invite:"} + std::to_string(input.request_context.body.size());
        m_context.cache().put(
            key,
            core::cache::to_cache_value(std::string{input.request_context.body})
        );
        const auto cached = m_context.cache().get(key);
        (void)cached;

        m_context.metrics().record_flow("resend_invite");
        m_context.metrics().counter("user.resend_invite").increment();

        return ResendInviteResult{ResponseDocument::empty(), set_cookie};
    }

private:
    app::context::AppContext& m_context;
    UserRuntimeSupport m_support;
};

} // namespace app::handlers::user
