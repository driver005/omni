export module app_user:user_runtime_totp_reset;

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

class TotpResetRuntime
{
public:
    explicit TotpResetRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] core::behavior::Outcome<std::pair<serde::Value, std::string>>
    totp_reset(const interfaces::Value& generic, const ::core::behavior::CallContext& ctx) const
    {
        std::string set_cookie{};


        // Port of core/user.rs:2355-2391 (reset_totp): stage a fresh secret and
        // return BeginTotpResponse with the secret + otpauth URL.
        const auto reset_totp_token = ctx_token(ctx).value();
        const auto new_secret = m_context.ids().uuid_v4();
        m_context.key_store()
            .rotate("totp-secret", reset_totp_token, ::app::crypto::SecretString{new_secret});
        for (auto& record: m_context.repository().list_authentication_records()) {
            if (record.get_user_id() == reset_totp_token && record.get_method() == "TOTP") {
                record.set_status("pending");
                m_context.repository().update_authentication_record(record);
            }
        }

        m_context.logger().record_flow("totp_reset");
        m_context.logger().counter("user.totp_reset").increment();

        ::app::model::user::TotpSecret totp_secret{};
        totp_secret.set_secret(new_secret);
        totp_secret.set_totp_url("otpauth://totp/Hyperswitch?secret=" + new_secret);
        ::app::model::user::BeginTotpResponse response{};
        response.set_secret(totp_secret);
        return std::pair{ResponseDocument::from(response), set_cookie};
    }

private:
    ::app::context::AppContext& m_context;
    UserRuntimeSupport m_support;
};

} // namespace app::handlers::user
