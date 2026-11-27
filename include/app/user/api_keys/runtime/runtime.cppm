export module app_user:api_keys_runtime;

import std;
import core_behavior;
import core_error;
import app_context;

import :api_keys_input;
import :api_keys_result;
import :api_keys_runtime_common;
import :api_keys_runtime_create;
import :api_keys_runtime_retrieve;
import :api_keys_runtime_update;
import :api_keys_runtime_revoke;
import :api_keys_runtime_list;

export namespace app::handlers::api_keys {

class ApiKeysRuntime
{
public:
    explicit ApiKeysRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_create{context},
        m_retrieve{context},
        m_update{context},
        m_revoke{context},
        m_list{context}
    {
    }

    [[nodiscard]] core::behavior::Outcome<::app::dto::api_keys::CreateApiKeyResponse> create(
        const ::app::dto::api_keys::CreateApiKeyRequest& request,
        const ::core::behavior::CallContext& ctx
    ) const
    {
        return m_create.create(request, ctx);
    }

    [[nodiscard]] core::behavior::Outcome<::app::dto::api_keys::RetrieveApiKeyResponse>
    retrieve(const ::core::behavior::CallContext& ctx) const
    {
        return m_retrieve.retrieve(ctx);
    }

    [[nodiscard]] core::behavior::Outcome<::app::dto::api_keys::RetrieveApiKeyResponse> update(
        const ::app::dto::api_keys::UpdateApiKeyRequest& request,
        const ::core::behavior::CallContext& ctx
    ) const
    {
        return m_update.update(request, ctx);
    }

    [[nodiscard]] core::behavior::Outcome<::app::dto::api_keys::RevokeApiKeyResponse>
    revoke(const ::core::behavior::CallContext& ctx) const
    {
        return m_revoke.revoke(ctx);
    }

    [[nodiscard]] core::behavior::Outcome<serde::Value> list(
        const ::app::dto::api_keys::ListApiKeyConstraints& constraints,
        const ::core::behavior::CallContext& ctx
    ) const
    {
        return m_list.list(constraints, ctx);
    }

private:
    ::app::context::AppContext& m_context;
    ApiKeyCreateRuntime m_create;
    ApiKeyRetrieveRuntime m_retrieve;
    ApiKeyUpdateRuntime m_update;
    ApiKeyRevokeRuntime m_revoke;
    ApiKeyListRuntime m_list;
};

} // namespace app::handlers::api_keys
