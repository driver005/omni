export module app_auth:client_secret_runtime_delete;

import std;
import core_error;
import app_context;
import app_dto;
import app_metrics;

import :client_secret_input;
import :client_secret_result;
import :client_secret_runtime_common;

export namespace app::handlers::client_secret {

class ClientSecretDeleteRuntime
{
public:
    explicit ClientSecretDeleteRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    // ───────────────────────── Core: delete ─────────────────────────
    // api::server_wrap(flow=ClientSecretDelete) + helpers.rs:4761-4782.
    [[nodiscard]] std::
        expected<::app::dto::client_secret::ClientSecretResponse, core::error::AppError>
        delete_client_secret(const ClientSecretDeleteInput& input) const
    {
        m_context.metrics().record_flow("ClientSecretDelete");
        m_context.metrics().counter("client_secret_delete").increment();

        ::app::dto::client_secret::ClientSecretResponse response{};
        if (auto err = m_support.delete_client_secret(input.client_secret_id, response)) {
            return std::unexpected{*err};
        }
        return std::move(response);
    }

private:
    ::app::context::AppContext& m_context;
    ClientSecretRuntimeSupport m_support;
};

} // namespace app::handlers::client_secret
