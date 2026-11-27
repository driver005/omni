export module app_user:user_runtime_totp_verify;

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

class TotpVerifyRuntime
{
public:
    explicit TotpVerifyRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] std::expected<std::pair<serde::Value, std::string>, core::error::AppError>
    totp_verify(const TotpVerifyInput& input) const
    {
        std::string set_cookie{};


        std::uint64_t step{0};
        if (const auto step_param = input.request_context.query.get("step")) {
            std::from_chars(step_param->data(), step_param->data() + step_param->size(), step);
        }
        const auto verify_token = input.request_context.token;
        const auto secret_entry = m_context.key_store().get("totp-secret", verify_token.value());
        const auto secret = secret_entry ? secret_entry->value() : std::string{};
        ::app::crypto::TotpGenerator generator{m_context.one_time_store()};
        const auto now = std::chrono::duration_cast<std::chrono::seconds>(
                             std::chrono::system_clock::now().time_since_epoch()
        )
                             .count();
        if (!generator.verify(secret, step, input.request.get_totp(), now)) {
            return std::unexpected{m_support.wire_error(
                "UR_37",
                "Invalid TOTP",
                core::error::AppErrorKind::INVALID_REQUEST
            )};
        }
        for (auto& record: m_context.repository().list_authentication_records()) {
            if (record.get_user_id() == verify_token.value() && record.get_method() == "TOTP") {
                record.set_status("active");
                m_context.repository().update_authentication_record(record);
            }
        }

        const auto key =
            std::string{"user.totp_verify:"} + std::to_string(input.request_context.body.size());
        m_context.cache().put(
            key,
            core::cache::to_cache_value(std::string{input.request_context.body})
        );
        const auto cached = m_context.cache().get(key);
        (void)cached;

        m_context.metrics().record_flow("totp_verify");
        m_context.metrics().counter("user.totp_verify").increment();

        return std::pair{ResponseDocument::empty(), set_cookie};
    }

private:
    ::app::context::AppContext& m_context;
    UserRuntimeSupport m_support;
};

} // namespace app::handlers::user
