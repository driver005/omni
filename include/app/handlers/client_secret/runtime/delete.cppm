export module app_handlers_client_secret:runtime_delete;

import std;
import core_error;
import app_context;
import app_dto;
import app_metrics;

import :input;
import :result;
import :runtime_common;

export namespace app::handlers::client_secret {

class ClientSecretDeleteRuntime
{
public:
    explicit ClientSecretDeleteRuntime(app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    // ───────────────────────── Core: delete ─────────────────────────
    // api::server_wrap(flow=ClientSecretDelete) + helpers.rs:4761-4782.
    [[nodiscard]] std::expected<ClientSecretDeleteResult, core::error::AppError>
    delete_client_secret(const ClientSecretDeleteInput& input) const
    {
        m_context.metrics().record_flow("ClientSecretDelete");
        m_context.metrics().counter("client_secret_delete").increment();

        app::dto::client_secret::ClientSecretResponse response{};
        if (auto err = m_support.delete_client_secret(input.client_secret_id, response)) {
            return std::unexpected{*err};
        }
        return ClientSecretDeleteResult{std::move(response)};
    }

private:
    app::context::AppContext& m_context;
    ClientSecretRuntimeSupport m_support;
};

} // namespace app::handlers::client_secret
