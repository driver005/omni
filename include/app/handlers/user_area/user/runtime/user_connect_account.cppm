export module app_user:user_runtime_user_connect_account;

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

class UserConnectAccountRuntime
{
public:
    explicit UserConnectAccountRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] std::expected<UserConnectAccountResult, core::error::AppError>
    user_connect_account(const UserConnectAccountInput& input) const
    {
        std::string set_cookie{};


        const auto user_id = m_context.ids().uuid_v4();
        const auto temp_password = m_context.ids().uuid_v4();
        const auto email = input.request.get_email().get_value();
        const ::app::crypto::SecretString password_secret{temp_password};
        const auto password_hash =
            ::app::crypto::KeyedHasher{"user-password-pepper"}.hash(password_secret.value());

        m_support.store_password(email, password_hash);
        m_support.persist_new_user(
            user_id,
            email,
            email,
            "active",
            input.request_context.merchant_id,
            "admin"
        );
        m_support.publish_identity_event("user.account_connected", user_id);

        m_context.metrics().record_flow("user_connect_account");
        m_context.metrics().counter("user.user_connect_account").increment();

        using Response = ::app::model::user::AuthorizeResponse;
        Response response{};
        response.set_is_email_sent(false);
        response.set_user_id(user_id);
        auto document = ResponseDocument::from(response);
        return UserConnectAccountResult{document, set_cookie};
    }

private:
    ::app::context::AppContext& m_context;
    UserRuntimeSupport m_support;
};

} // namespace app::handlers::user
