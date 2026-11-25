export module app_handlers_user:runtime_check_two_factor_auth_status;

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

class CheckTwoFactorAuthStatusRuntime
{
public:
    explicit CheckTwoFactorAuthStatusRuntime(app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] std::expected<CheckTwoFactorAuthStatusResult, core::error::AppError>
    check_two_factor_auth_status(const CheckTwoFactorAuthStatusInput& input) const
    {
        std::string set_cookie{};


        const auto owner = input.request_context.token.value();
        bool totp_active{false};
        for (const auto& record: m_context.repository().list_authentication_records()) {
            if (record.get_user_id() == owner && record.get_method() == "TOTP" &&
                record.get_status() == "active") {
                totp_active = true;
                break;
            }
        }
        const auto recovery_enabled =
            m_context.repository().get_config("user.recovery_codes." + owner).has_value();

        m_context.metrics().record_flow("check_two_factor_auth_status");
        m_context.metrics().counter("user.check_two_factor_auth_status").increment();

        using Response = app::model::user::TwoFactorAuthStatusResponse;
        Response response{};
        response.set_totp(totp_active);
        response.set_recovery_code(recovery_enabled);
        auto document = ResponseDocument::from(response);
        return CheckTwoFactorAuthStatusResult{document, set_cookie};
    }

private:
    app::context::AppContext& m_context;
    UserRuntimeSupport m_support;
};

} // namespace app::handlers::user
