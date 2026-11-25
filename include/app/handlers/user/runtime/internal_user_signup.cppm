export module app_handlers_user:runtime_internal_user_signup;

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

class InternalUserSignupRuntime
{
public:
    explicit InternalUserSignupRuntime(app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] std::expected<InternalUserSignupResult, core::error::AppError>
    internal_user_signup(const InternalUserSignupInput& input) const
    {
        std::string set_cookie{};


        const auto internal_user_id = m_context.ids().uuid_v4();
        const auto internal_email = input.request.get_email().get_value();
        const app::crypto::SecretString internal_secret{input.request.get_password()};
        const auto internal_hash =
            app::crypto::KeyedHasher{"user-password-pepper"}.hash(internal_secret.value());
        m_support.store_password(internal_email, internal_hash);
        m_support.persist_new_user(
            internal_user_id,
            internal_email,
            internal_email,
            "active",
            input.request_context.merchant_id,
            "admin"
        );
        m_support.publish_identity_event("user.internal_created", internal_user_id);

        const auto key = std::string{"user.internal_user_signup:"} +
                         std::to_string(input.request_context.body.size());
        m_context.cache().put(
            key,
            core::cache::to_cache_value(std::string{input.request_context.body})
        );
        const auto cached = m_context.cache().get(key);
        (void)cached;

        m_context.metrics().record_flow("internal_user_signup");
        m_context.metrics().counter("user.internal_user_signup").increment();

        return InternalUserSignupResult{ResponseDocument::empty(), set_cookie};
    }

private:
    app::context::AppContext& m_context;
    UserRuntimeSupport m_support;
};

} // namespace app::handlers::user
