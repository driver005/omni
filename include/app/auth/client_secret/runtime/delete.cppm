export module app_auth:client_secret_runtime_delete;

import std;
import core_behavior;
import core_error;
import app_context;
import app_dto;
import app_metrics;

import :call_context;
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
    [[nodiscard]] core::behavior::Outcome<::app::dto::client_secret::ClientSecretResponse>
    delete_client_secret(const core::behavior::CallContext& ctx) const
    {
        m_context.logger().record_flow("ClientSecretDelete");
        m_context.logger().counter("client_secret_delete").increment();

        const auto& client_secret_id = detail::path_segment_after(ctx, "client-secret");
        ::app::dto::client_secret::ClientSecretResponse response{};
        if (auto err = m_support.delete_client_secret(client_secret_id, response)) {
            return std::unexpected{*err};
        }
        return std::move(response);
    }

private:
    ::app::context::AppContext& m_context;
    ClientSecretRuntimeSupport m_support;
};

} // namespace app::handlers::client_secret
