export module app_auth:client_secret_runtime;

import std;
import core_error;
import app_context;

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

    [[nodiscard]] std::expected<ClientSecretCreateResult, core::error::AppError>
    create(const ClientSecretCreateInput& input) const
    {
        return m_create.create(input);
    }

    [[nodiscard]] std::expected<ClientSecretDeleteResult, core::error::AppError>
    delete_client_secret(const ClientSecretDeleteInput& input) const
    {
        return m_delete.delete_client_secret(input);
    }

private:
    ::app::context::AppContext& m_context;
    ClientSecretCreateRuntime m_create;
    ClientSecretDeleteRuntime m_delete;
};

} // namespace app::handlers::client_secret
