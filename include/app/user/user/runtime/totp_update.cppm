export module app_user:user_runtime_totp_update;

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

class TotpUpdateRuntime
{
public:
    explicit TotpUpdateRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] std::expected<std::pair<serde::Value, std::string>, core::error::AppError>
    totp_update(const TotpUpdateInput& input) const
    {
        std::string set_cookie{};


        // Port of core/user.rs:2447-2518 (update_totp): the pending secret staged
        // by totp_begin/reset must validate the submitted code before it is
        // promoted to the durable per-user TOTP secret; then StatusOk.
        const auto owner = input.request_context.token.value();
        const auto pending = m_context.key_store().get("totp-secret", owner);
        if (!pending) {
            return std::unexpected{m_support.wire_error(
                "UR_42",
                "TOTP secret not found",
                core::error::AppErrorKind::INVALID_REQUEST
            )};
        }

        std::uint64_t step{0};
        if (const auto step_param = input.request_context.query.get("step")) {
            std::from_chars(step_param->data(), step_param->data() + step_param->size(), step);
        }
        ::app::crypto::TotpGenerator generator{m_context.one_time_store()};
        if (!generator.verify(
                pending->value(),
                step,
                input.request.get_totp(),
                m_support.now_seconds()
            )) {
            return std::unexpected{m_support.wire_error(
                "UR_37",
                "Invalid TOTP",
                core::error::AppErrorKind::INVALID_REQUEST
            )};
        }

        // Promote the staged secret to the durable user secret and mark the
        // authentication record active (mirrors the encrypted DB TotpUpdate).
        m_context.key_store()
            .put("totp-active-secret", owner, "RAW", ::app::crypto::SecretString{pending->value()});
        for (auto& record: m_context.repository().list_authentication_records()) {
            if (record.get_user_id() == owner && record.get_method() == "TOTP") {
                record.set_status("active");
                m_context.repository().update_authentication_record(record);
            }
        }
        // Delete the staged secret (upstream delete_totp_secret_from_redis).
        m_context.key_store()
            .rotate("totp-secret", owner, ::app::crypto::SecretString{std::string{}});

        m_context.cache().put(
            "user.totp_update:" + owner,
            core::cache::to_cache_value(std::string{"ok"})
        );

        m_context.metrics().record_flow("totp_update");
        m_context.metrics().counter("user.totp_update").increment();

        // Upstream ApplicationResponse::StatusOk -> HttpResponse::Ok().finish()
        // (services/api.rs:499,730-732): bare empty 200 body.
        return std::pair{ResponseDocument::empty(), set_cookie};
    }

private:
    ::app::context::AppContext& m_context;
    UserRuntimeSupport m_support;
};

} // namespace app::handlers::user
