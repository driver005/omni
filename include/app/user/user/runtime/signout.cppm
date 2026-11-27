export module app_user:user_runtime_signout;

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

class SignoutRuntime
{
public:
    explicit SignoutRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] core::behavior::Outcome<std::pair<serde::Value, std::string>>
    signout(const interfaces::Value& generic, const ::core::behavior::CallContext& ctx) const
    {
        std::string set_cookie{};


        const auto token = ctx_token(ctx);
        if (!token.value().empty()) {
            m_context.cache().put(
                "user.blacklist:" + token.value(),
                core::cache::to_cache_value(std::string{"blacklisted"})
            );
        }

        const auto key =
            std::string{"user.signout:"} + std::to_string(ctx_string(ctx, "body").size());
        m_context.cache().put(
            key,
            core::cache::to_cache_value(std::string{ctx_string(ctx, "body")})
        );
        const auto cached = m_context.cache().get(key);
        (void)cached;

        m_context.logger().record_flow("signout");
        m_context.logger().counter("user.signout").increment();

        return std::pair{ResponseDocument::empty(), set_cookie};
    }

private:
    ::app::context::AppContext& m_context;
    UserRuntimeSupport m_support;
};

} // namespace app::handlers::user
