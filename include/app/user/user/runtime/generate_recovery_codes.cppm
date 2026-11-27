export module app_user:user_runtime_generate_recovery_codes;

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

class GenerateRecoveryCodesRuntime
{
public:
    explicit GenerateRecoveryCodesRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] core::behavior::Outcome<std::pair<serde::Value, std::string>>
    generate_recovery_codes(const interfaces::Value& generic,
        const ::core::behavior::CallContext& ctx) const
    {
        std::string set_cookie{};


        std::uint64_t seed{0};
        if (const auto seed_param = ctx_query(ctx).get("seed")) {
            std::from_chars(seed_param->data(), seed_param->data() + seed_param->size(), seed);
        }
        const auto codes_token = ctx_token(ctx);
        ::app::crypto::RecoveryCodeGenerator generator{
            seed,
            codes_token.value(),
            m_context.one_time_store()
        };
        const auto recovery_codes = generator.generate_for(8);
        m_context.repository().set_config(
            "user.recovery_codes." + codes_token.value(),
            std::to_string(recovery_codes.size())
        );

        const auto key = std::string{"user.generate_recovery_codes:"} +
                         std::to_string(ctx_string(ctx,"body").size());
        m_context.cache().put(
            key,
            core::cache::to_cache_value(std::string{ctx_string(ctx,"body")})
        );
        const auto cached = m_context.cache().get(key);
        (void)cached;

        m_context.metrics().record_flow("generate_recovery_codes");
        m_context.metrics().counter("user.generate_recovery_codes").increment();

        using Response = ::app::model::user::RecoveryCodes;
        Response response{};
        response.set_recovery_codes(recovery_codes);
        auto document = ResponseDocument::from(response);
        return std::pair{document, set_cookie};
    }

private:
    ::app::context::AppContext& m_context;
    UserRuntimeSupport m_support;
};

} // namespace app::handlers::user
