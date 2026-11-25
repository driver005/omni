export module app_handlers_user:runtime_change_password;

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

class ChangePasswordRuntime
{
public:
    explicit ChangePasswordRuntime(app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] std::expected<ChangePasswordResult, core::error::AppError>
    change_password(const ChangePasswordInput& input) const
    {
        std::string set_cookie{};


        if (input.request.get_old_password() == input.request.get_new_password()) {
            return std::unexpected{m_support.wire_error(
                "UR_29",
                "Old and new password cannot be same",
                core::error::AppErrorKind::INVALID_REQUEST
            )};
        }
        const auto token = input.request_context.token;
        const auto email_entry = m_context.cache().get("user.token_email:" + token.value());
        const auto email =
            email_entry.hit ? core::cache::value_as<std::string>(email_entry.value, std::string{})
                            : std::string{};
        if (email.empty()) {
            return std::unexpected{m_support.wire_error(
                "IR_17",
                "Access forbidden, invalid JWT token was used",
                core::error::AppErrorKind::UNAUTHORIZED
            )};
        }
        const auto old_hash = m_context.key_store().get("user-password", email);
        const app::crypto::SecretString old_secret{input.request.get_old_password()};
        if (!old_hash || !app::crypto::KeyedHasher{"user-password-pepper"}
                              .verify(old_secret.value(), old_hash->value())) {
            return std::unexpected{m_support.wire_error(
                "UR_06",
                "Old password incorrect. Please enter the correct password",
                core::error::AppErrorKind::INVALID_REQUEST
            )};
        }
        const app::crypto::SecretString new_secret{input.request.get_new_password()};
        m_support.store_password(
            email,
            app::crypto::KeyedHasher{"user-password-pepper"}.hash(new_secret.value())
        );

        m_context.metrics().record_flow("change_password");
        m_context.metrics().counter("user.change_password").increment();

        return ChangePasswordResult{ResponseDocument::empty(), set_cookie};
    }

private:
    app::context::AppContext& m_context;
    UserRuntimeSupport m_support;
};

} // namespace app::handlers::user
