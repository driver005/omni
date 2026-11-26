export module app_user:user_runtime_check_two_factor_auth_status_with_attempts;

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

class CheckTwoFactorAuthStatusWithAttemptsRuntime
{
public:
    explicit CheckTwoFactorAuthStatusWithAttemptsRuntime(
        ::app::context::AppContext& context
    ) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] std::expected<CheckTwoFactorAuthStatusWithAttemptsResult, core::error::AppError>
    check_two_factor_auth_status_with_attempts(
        const CheckTwoFactorAuthStatusWithAttemptsInput& input
    ) const
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

        m_context.metrics().record_flow("check_two_factor_auth_status_with_attempts");
        m_context.metrics().counter("user.check_two_factor_auth_status_with_attempts").increment();

        using Response = ::app::model::user::TwoFactorStatus;
        Response response{};
        ::app::model::user::TwoFactorAuthStatusResponseWithAttempts status{};
        ::app::model::user::TwoFactorAuthAttempts totp{};
        totp.set_is_completed(totp_active);
        totp.set_remaining_attempts(totp_active ? std::uint8_t{0} : std::uint8_t{3});
        status.set_totp(totp);
        response.set_status(status);
        const auto skip_flag = input.request_context.query.get("skip_two_factor_auth");
        response.set_is_skippable(
            !totp_active || (skip_flag && (*skip_flag == "true" || *skip_flag == "1"))
        );
        auto document = ResponseDocument::from(response);
        return CheckTwoFactorAuthStatusWithAttemptsResult{document, set_cookie};
    }

private:
    ::app::context::AppContext& m_context;
    UserRuntimeSupport m_support;
};

} // namespace app::handlers::user
