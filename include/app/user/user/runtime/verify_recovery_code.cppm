export module app_user:user_runtime_verify_recovery_code;

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

class VerifyRecoveryCodeRuntime
{
public:
    explicit VerifyRecoveryCodeRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] core::behavior::Outcome<std::pair<serde::Value, std::string>>
    verify_recovery_code(const ::app::model::user::VerifyRecoveryCodeRequest& request,
        const ::core::behavior::CallContext& ctx) const
    {
        std::string set_cookie{};


        std::uint64_t seed{0};
        if (const auto seed_param = ctx_query(ctx).get("seed")) {
            std::from_chars(seed_param->data(), seed_param->data() + seed_param->size(), seed);
        }
        const auto owner = ctx_token(ctx).value();
        ::app::crypto::RecoveryCodeGenerator generator{seed, owner, m_context.one_time_store()};
        if (!generator.verify(request.get_recovery_code())) {
            return std::unexpected{m_support.wire_error(
                "UR_39",
                "Invalid Recovery Code",
                core::error::AppErrorKind::INVALID_REQUEST
            )};
        }

        const auto key = std::string{"user.verify_recovery_code:"} +
                         std::to_string(ctx_string(ctx,"body").size());
        m_context.cache().put(
            key,
            core::cache::to_cache_value(std::string{ctx_string(ctx,"body")})
        );
        const auto cached = m_context.cache().get(key);
        (void)cached;

        m_context.metrics().record_flow("verify_recovery_code");
        m_context.metrics().counter("user.verify_recovery_code").increment();

        return std::pair{ResponseDocument::empty(), set_cookie};
    }

private:
    ::app::context::AppContext& m_context;
    UserRuntimeSupport m_support;
};

} // namespace app::handlers::user
