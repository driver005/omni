export module app_handlers_user:runtime_totp_begin;

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

class TotpBeginRuntime
{
public:
    explicit TotpBeginRuntime(app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] std::expected<TotpBeginResult, core::error::AppError>
    totp_begin(const TotpBeginInput& input) const
    {
        std::string set_cookie{};


        const auto totp_token = input.request_context.token;

        // Port of core/user.rs:2322-2353 (begin_totp): when TOTP is already set
        // for the user, upstream returns BeginTotpResponse { secret: None }.
        bool already_setup{false};
        for (const auto& record: m_context.repository().list_authentication_records()) {
            if (record.get_user_id() == totp_token.value() && record.get_method() == "TOTP" &&
                record.get_status() == "active") {
                already_setup = true;
                break;
            }
        }
        if (already_setup) {
            m_context.metrics().record_flow("totp_begin");
            m_context.metrics().counter("user.totp_begin").increment();
            return TotpBeginResult{
                ResponseDocument::from(app::model::user::BeginTotpResponse{}),
                set_cookie
            };
        }

        const auto totp_secret_value = m_context.ids().uuid_v4();
        m_context.key_store().put(
            "totp-secret",
            totp_token.value(),
            "RAW",
            app::crypto::SecretString{totp_secret_value}
        );

        app::store::authentication_records::AuthenticationRecord record{};
        record.set_auth_id(m_context.ids().key_id());
        record.set_user_id(totp_token.value());
        record.set_merchant_id(input.request_context.merchant_id);
        record.set_method("TOTP");
        record.set_status("pending");
        record.set_created_at(m_support.now_string());
        (void)m_context.repository().create_authentication_record(record);

        const auto key =
            std::string{"user.totp_begin:"} + std::to_string(input.request_context.body.size());
        m_context.cache().put(
            key,
            core::cache::to_cache_value(std::string{input.request_context.body})
        );
        const auto cached = m_context.cache().get(key);
        (void)cached;

        m_context.metrics().record_flow("totp_begin");
        m_context.metrics().counter("user.totp_begin").increment();

        app::model::user::TotpSecret totp_secret{};
        totp_secret.set_secret(totp_secret_value);
        totp_secret.set_totp_url("otpauth://totp/Hyperswitch?secret=" + totp_secret_value);
        using Response = app::model::user::BeginTotpResponse;
        Response response{};
        response.set_secret(totp_secret);
        auto document = ResponseDocument::from(response);
        return TotpBeginResult{document, set_cookie};
    }

private:
    app::context::AppContext& m_context;
    UserRuntimeSupport m_support;
};

} // namespace app::handlers::user
