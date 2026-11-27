export module app_user:user_runtime_terminate_two_factor_auth;

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

class TerminateTwoFactorAuthRuntime
{
public:
    explicit TerminateTwoFactorAuthRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] core::behavior::Outcome<std::pair<serde::Value, std::string>>
    terminate_two_factor_auth(const interfaces::Value& generic,
        const ::core::behavior::CallContext& ctx) const
    {
        std::string set_cookie{};


        const auto owner = ctx_token(ctx).value();
        for (auto& record: m_context.repository().list_authentication_records()) {
            if (record.get_user_id() == owner && record.get_method() == "TOTP") {
                record.set_status("disabled");
                m_context.repository().update_authentication_record(record);
            }
        }
        m_context.repository().set_config("user.recovery_codes." + owner, "0");

        // Port of core/user.rs:2637-2695: successful termination reissues a
        // LOGIN token with the TOTP step marked complete and a cookie.
        const auto token = m_support.user_signer().sign(
            std::format(R"({{"sub":"{}","purpose":"TOTP","totp":"complete"}})", owner)
        );
        set_cookie = "user_token=" + token + "; HttpOnly; Path=/";
        m_context.cache().put("user.token_email:" + token, core::cache::to_cache_value(owner));

        m_context.metrics().record_flow("terminate_two_factor_auth");
        m_context.metrics().counter("user.terminate_two_factor_auth").increment();

        using Response = ::app::model::user::TokenResponse;
        Response response{};
        ::app::model::user::TokenPurposeValue purpose{};
        purpose.set_value("LOGIN");
        response.set_token(token);
        response.set_token_type(purpose);
        return std::pair{ResponseDocument::from(response), set_cookie};
    }

private:
    ::app::context::AppContext& m_context;
    UserRuntimeSupport m_support;
};

} // namespace app::handlers::user
