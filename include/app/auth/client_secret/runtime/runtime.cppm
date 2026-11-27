export module app_auth:client_secret_runtime;

import std;
import core_behavior;
import core_error;
import app_context;
import app_dto;

import :client_secret_input;
import :client_secret_result;
import :client_secret_runtime_common;
import :client_secret_runtime_create;
import :client_secret_runtime_delete;

export namespace app::handlers::client_secret {

class ClientSecretRuntime
{
public:
    explicit ClientSecretRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_create{context},
        m_delete{context}
    {
    }

    [[nodiscard]] core::behavior::Outcome<::app::dto::client_secret::ClientSecretResponse>
    create(
        const ::app::dto::client_secret::ClientSecretCreateRequest& request,
        const core::behavior::CallContext& ctx
    ) const
    {
        return m_create.create(request, ctx);
    }

    [[nodiscard]] core::behavior::Outcome<::app::dto::client_secret::ClientSecretResponse>
    delete_client_secret(const core::behavior::CallContext& ctx) const
    {
        return m_delete.delete_client_secret(ctx);
    }

private:
    ::app::context::AppContext& m_context;
    ClientSecretCreateRuntime m_create;
    ClientSecretDeleteRuntime m_delete;
};

} // namespace app::handlers::client_secret
