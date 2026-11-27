export module app_user:user_runtime_user_signin;

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

class UserSigninRuntime
{
public:
    explicit UserSigninRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] core::behavior::Outcome<std::pair<serde::Value, std::string>>
    user_signin(const ::app::model::user::SignInRequest& request,
        const ::core::behavior::CallContext& ctx) const
    {
        std::string set_cookie{};

        const auto email = request.get_email().get_value();
        const auto password = request.get_password();
        if (email.empty() || password.empty()) {
            return std::unexpected{m_support.wire_error(
                "UR_07",
                "Invalid Email",
                core::error::AppErrorKind::INVALID_REQUEST
            )};
        }
        const auto stored_hash = m_context.key_store().get("user-password", email);
        if (!stored_hash || !::app::crypto::KeyedHasher{"user-password-pepper"}
                                 .verify(password, stored_hash->value())) {
            return std::unexpected{m_support.wire_error(
                "UR_01",
                "Incorrect email or password",
                core::error::AppErrorKind::UNAUTHORIZED
            )};
        }

        const auto auth_ctx = ctx_auth(ctx);
        // Upstream UserFromToken::user_id is the persisted user id, not the email.
        // Role-scoped handlers resolve role_id from this claim, so keep sub aligned
        // with the id stored by persist_new_user().
        std::string user_id{email};
        if (const auto entry = m_context.cache().get("user.email_index:" + std::string{email});
            entry.hit) {
            user_id = core::cache::value_as<std::string>(entry.value, email);
        }
        const auto signin_payload = std::string{R"({"sub":")"} + user_id + R"(","email":")" +
                                    email + R"(","merchant_id":")" + auth_ctx.merchant_id() +
                                    R"(","org_id":")" + auth_ctx.org_id() + R"("})";
        const auto signin_token = m_support.user_signer().sign(signin_payload);
        m_context.cache().put(
            "user.token_email:" + signin_token,
            core::cache::to_cache_value(email)
        );
        set_cookie = "user_token=" + signin_token + "; HttpOnly; Path=/";

        m_context.metrics().record_flow("user_signin");
        m_context.metrics().counter("user.user_signin").increment();

        using Response = ::app::model::user::TokenResponse;
        Response response{};
        ::app::model::user::TokenPurposeValue purpose{};
        purpose.set_value("user_info");
        response.set_token(signin_token);
        response.set_token_type(purpose);
        auto document = ResponseDocument::from(response);
        return std::pair{document, set_cookie};
    }

private:
    ::app::context::AppContext& m_context;
    UserRuntimeSupport m_support;
};

} // namespace app::handlers::user
