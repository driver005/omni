export module app_auth:client_secret_runtime_create;

import std;
import core_behavior;
import core_error;
import app_context;
import app_dto;
import app_metrics;

import :client_secret_input;
import :client_secret_result;
import :client_secret_runtime_common;

export namespace app::handlers::client_secret {

class ClientSecretCreateRuntime
{
public:
    explicit ClientSecretCreateRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    // ───────────────────────── Core: create ─────────────────────────
    // api::server_wrap(flow=ClientSecretCreate) + helpers.rs:4694-4758.
    [[nodiscard]] core::behavior::Outcome<::app::dto::client_secret::ClientSecretResponse>
    create(
        const ::app::dto::client_secret::ClientSecretCreateRequest& request,
        const core::behavior::CallContext& ctx
    ) const
    {
        m_context.metrics().record_flow("ClientSecretCreate");
        m_context.metrics().counter("client_secret_create").increment();
        // No API lock: upstream declares LockAction::NotApplicable.

        ::app::dto::client_secret::ClientSecretResponse response{};
        if (auto err = m_support.make_client_secret(
                ctx.merchant_id,
                request.get_resource_id(),
                response
            )) {
            return std::unexpected{*err};
        }
        return std::move(response);
    }

private:
    ::app::context::AppContext& m_context;
    ClientSecretRuntimeSupport m_support;
};

} // namespace app::handlers::client_secret
