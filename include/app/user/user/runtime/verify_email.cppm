export module app_user:user_runtime_verify_email;

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

class VerifyEmailRuntime
{
public:
    explicit VerifyEmailRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] std::expected<std::pair<serde::Value, std::string>, core::error::AppError>
    verify_email(const VerifyEmailInput& input) const
    {
        std::string set_cookie{};


        if (!m_support.user_signer().verify(input.request.get_token())) {
            return std::unexpected{m_support.wire_error(
                "UR_04",
                "Invalid or expired link",
                core::error::AppErrorKind::UNAUTHORIZED
            )};
        }

        // Port of core/user.rs:2157-2180: a verified single-purpose token is
        // exchanged for a LOGIN token with a cookie.
        const auto subject = m_support.jwt_claim(input.request.get_token(), "sub");
        const auto email = m_support.jwt_claim(input.request.get_token(), "email");
        const auto login_token = m_support.user_signer().sign(
            std::format(R"({{"sub":"{}","email":"{}"}})", subject, email)
        );
        set_cookie = "user_token=" + login_token + "; HttpOnly; Path=/";
        m_context.cache().put(
            "user.token_email:" + login_token,
            core::cache::to_cache_value(email)
        );

        m_context.metrics().record_flow("verify_email");
        m_context.metrics().counter("user.verify_email").increment();

        using Response = ::app::model::user::TokenResponse;
        Response response{};
        ::app::model::user::TokenPurposeValue purpose{};
        purpose.set_value("LOGIN");
        response.set_token(login_token);
        response.set_token_type(purpose);
        return std::pair{ResponseDocument::from(response), set_cookie};
    }

private:
    ::app::context::AppContext& m_context;
    UserRuntimeSupport m_support;
};

} // namespace app::handlers::user
