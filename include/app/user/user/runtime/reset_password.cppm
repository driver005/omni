export module app_user:user_runtime_reset_password;

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

class ResetPasswordRuntime
{
public:
    explicit ResetPasswordRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] std::expected<std::pair<serde::Value, std::string>, core::error::AppError>
    reset_password(const ResetPasswordInput& input) const
    {
        std::string set_cookie{};


        if (!m_support.user_signer().verify(input.request.get_token())) {
            return std::unexpected{m_support.wire_error(
                "UR_04",
                "Invalid or expired link",
                core::error::AppErrorKind::UNAUTHORIZED
            )};
        }
        const ::app::crypto::SecretString reset_secret{input.request.get_password()};
        const auto reset_hash =
            ::app::crypto::KeyedHasher{"user-password-pepper"}.hash(reset_secret.value());
        m_context.key_store().put(
            "user-password-reset",
            input.request.get_token(),
            "FNV1a",
            ::app::crypto::SecretString{reset_hash}
        );

        const auto key =
            std::string{"user.reset_password:"} + std::to_string(input.request_context.body.size());
        m_context.cache().put(
            key,
            core::cache::to_cache_value(std::string{input.request_context.body})
        );
        const auto cached = m_context.cache().get(key);
        (void)cached;

        m_context.metrics().record_flow("reset_password");
        m_context.metrics().counter("user.reset_password").increment();

        return std::pair{ResponseDocument::empty(), set_cookie};
    }

private:
    ::app::context::AppContext& m_context;
    UserRuntimeSupport m_support;
};

} // namespace app::handlers::user
