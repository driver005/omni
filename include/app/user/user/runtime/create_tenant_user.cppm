export module app_user:user_runtime_create_tenant_user;

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

class CreateTenantUserRuntime
{
public:
    explicit CreateTenantUserRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] core::behavior::Outcome<std::pair<serde::Value, std::string>> create_tenant_user(
        const ::app::model::user::CreateTenantUserRequest& request,
        const ::core::behavior::CallContext& ctx
    ) const
    {
        std::string set_cookie{};


        const auto tenant_user_id = m_context.ids().uuid_v4();
        const auto tenant_email = request.get_email().get_value();
        const ::app::crypto::SecretString tenant_secret{request.get_password()};
        const auto tenant_hash =
            ::app::crypto::KeyedHasher{"user-password-pepper"}.hash(tenant_secret.value());
        m_support.store_password(tenant_email, tenant_hash);
        m_support.persist_new_user(
            tenant_user_id,
            tenant_email,
            tenant_email,
            "active",
            ctx.merchant_id,
            "tenant"
        );
        m_support.publish_identity_event("user.tenant_created", tenant_user_id);

        const auto key = std::string{"user.create_tenant_user:"} +
                         std::to_string(ctx_string(ctx, "body").size());
        m_context.cache().put(
            key,
            core::cache::to_cache_value(std::string{ctx_string(ctx, "body")})
        );
        const auto cached = m_context.cache().get(key);
        (void)cached;

        m_context.logger().record_flow("create_tenant_user");
        m_context.logger().counter("user.create_tenant_user").increment();

        return std::pair{ResponseDocument::empty(), set_cookie};
    }

private:
    ::app::context::AppContext& m_context;
    UserRuntimeSupport m_support;
};

} // namespace app::handlers::user
