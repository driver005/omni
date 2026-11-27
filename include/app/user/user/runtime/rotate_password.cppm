export module app_user:user_runtime_rotate_password;

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

class RotatePasswordRuntime
{
public:
    explicit RotatePasswordRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] core::behavior::Outcome<std::pair<serde::Value, std::string>>
    rotate_password(const ::app::model::user::RotatePasswordRequest& request,
        const ::core::behavior::CallContext& ctx) const
    {
        std::string set_cookie{};


        if (request.get_password().empty()) {
            return std::unexpected{m_support.wire_error(
                "UR_09",
                "Invalid Password",
                core::error::AppErrorKind::INVALID_REQUEST
            )};
        }
        const ::app::crypto::SecretString rotate_secret{request.get_password()};
        const auto rotate_hash =
            ::app::crypto::KeyedHasher{"user-password-pepper"}.hash(rotate_secret.value());
        const auto rotate_token = ctx_token(ctx);
        m_context.key_store().put(
            "user-password-rotate",
            rotate_token.value(),
            "FNV1a",
            ::app::crypto::SecretString{rotate_hash}
        );

        const auto key = std::string{"user.rotate_password:"} +
                         std::to_string(ctx_string(ctx,"body").size());
        m_context.cache().put(
            key,
            core::cache::to_cache_value(std::string{ctx_string(ctx,"body")})
        );
        const auto cached = m_context.cache().get(key);
        (void)cached;

        m_context.metrics().record_flow("rotate_password");
        m_context.metrics().counter("user.rotate_password").increment();

        return std::pair{ResponseDocument::empty(), set_cookie};
    }

private:
    ::app::context::AppContext& m_context;
    UserRuntimeSupport m_support;
};

} // namespace app::handlers::user
