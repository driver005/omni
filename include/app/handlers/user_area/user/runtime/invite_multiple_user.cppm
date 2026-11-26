export module app_user:user_runtime_invite_multiple_user;

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

class InviteMultipleUserRuntime
{
public:
    explicit InviteMultipleUserRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] std::expected<InviteMultipleUserResult, core::error::AppError>
    invite_multiple_user(const InviteMultipleUserInput& input) const
    {
        std::string set_cookie{};


        const auto key = std::string{"user.invite_multiple_user:"} +
                         std::to_string(input.request_context.body.size());
        m_context.cache().put(
            key,
            core::cache::to_cache_value(std::string{input.request_context.body})
        );
        const auto cached = m_context.cache().get(key);
        (void)cached;

        m_context.metrics().record_flow("invite_multiple_user");
        m_context.metrics().counter("user.invite_multiple_user").increment();

        using Response = std::vector<::app::model::user::InviteMultipleUserResponse>;
        Response response{};
        std::vector<::app::model::user::InviteMultipleUserResponse> responses{};
        responses.reserve(input.request.size());
        for (const auto& request: input.request) {
            const auto email_value = request.get_email().get_value();
            const auto invite_user_id = m_context.ids().uuid_v4();
            const auto invite_payload = std::string{R"({"sub":")"} + invite_user_id +
                                        R"(","email":")" + email_value + R"("})";
            const auto invite_token = m_support.user_signer().sign(invite_payload);
            m_context.cache().put(
                "user.invite_token:" + email_value,
                core::cache::to_cache_value(invite_token)
            );
            m_context.cache().put(
                "user.email_index:" + email_value,
                core::cache::to_cache_value(invite_user_id)
            );
            m_context.cache().put(
                "user.invite_token_email:" + invite_token,
                core::cache::to_cache_value(email_value)
            );
            m_support.persist_new_user(
                invite_user_id,
                email_value,
                email_value,
                "invited",
                input.request_context.merchant_id,
                "invited_user"
            );
            m_support.publish_identity_event("user.invited", invite_user_id);

            ::app::model::user::InviteMultipleUserResponse item{};
            item.set_email(request.get_email());
            item.set_is_email_sent(false);
            responses.push_back(std::move(item));
        }
        auto document = ResponseDocument::from(responses);
        return InviteMultipleUserResult{document, set_cookie};
    }

private:
    ::app::context::AppContext& m_context;
    UserRuntimeSupport m_support;
};

} // namespace app::handlers::user
