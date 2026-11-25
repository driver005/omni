export module app_handlers_user:runtime_rotate_password;

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

class RotatePasswordRuntime
{
public:
    explicit RotatePasswordRuntime(app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] std::expected<RotatePasswordResult, core::error::AppError>
    rotate_password(const RotatePasswordInput& input) const
    {
        std::string set_cookie{};


        if (input.request.get_password().empty()) {
            return std::unexpected{m_support.wire_error(
                "UR_09",
                "Invalid Password",
                core::error::AppErrorKind::INVALID_REQUEST
            )};
        }
        const app::crypto::SecretString rotate_secret{input.request.get_password()};
        const auto rotate_hash =
            app::crypto::KeyedHasher{"user-password-pepper"}.hash(rotate_secret.value());
        const auto rotate_token = input.request_context.token;
        m_context.key_store().put(
            "user-password-rotate",
            rotate_token.value(),
            "FNV1a",
            app::crypto::SecretString{rotate_hash}
        );

        const auto key = std::string{"user.rotate_password:"} +
                         std::to_string(input.request_context.body.size());
        m_context.cache().put(
            key,
            core::cache::to_cache_value(std::string{input.request_context.body})
        );
        const auto cached = m_context.cache().get(key);
        (void)cached;

        m_context.metrics().record_flow("rotate_password");
        m_context.metrics().counter("user.rotate_password").increment();

        return RotatePasswordResult{ResponseDocument::empty(), set_cookie};
    }

private:
    app::context::AppContext& m_context;
    UserRuntimeSupport m_support;
};

} // namespace app::handlers::user
