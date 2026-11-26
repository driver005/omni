export module app_routing:superposition_proxy_handlers_v1_audit;

import std;
import interfaces;
import core_router;
import core_serde;
import core_error;
import core_cache;
import utils_openapi;
import app_model;
import app_dto;
import app_context;
import app_auth;
import app_http;
import app_crypto;
import app_metrics;
import app_store;
import app_services;

import :superposition_proxy_input;
import :superposition_proxy_result;
import :superposition_proxy_runtime;

export namespace app::handlers::superposition_proxy {

/**
 * v1 audit family: `v1/superposition/audit` GET adapter. Body is verbatim
 * from the original combined handler; runtime and input support are owned
 * by the facade and shared by reference.
 */
class SuperpositionProxyHandlersV1Audit
{
public:
    SuperpositionProxyHandlersV1Audit(
        ::app::context::AppContext& context,
        SuperpositionProxyRuntime& runtime,
        SuperpositionProxyInputSupport& input_support
    ) noexcept :
        m_context{context},
        m_runtime{runtime},
        m_input_support{input_support}
    {
    }

    // ── GET /v1/superposition/audit ─────────────────────────────────────────────
    void handle_list_audit_logs(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto auth_error = m_input_support.authorize_request(req, "list_audit_logs");
        if (auth_error) {
            const auto& error = *auth_error;
            error.send(res, send);
            return;
        }
        auto scope = m_input_support.extract_proxy_headers(req);
        if (!scope) {
            const auto& error = scope.error();
            error.send(res, send);
            return;
        }
        m_input_support.cache_scope(req, *scope);

        ListAuditLogsHandlerInput input{
            m_input_support.list_audit_logs_query(m_input_support.query_pairs(req)),
            ::app::auth::AuthContextBuilder{}(req),
            scope->org_id,
            scope->workspace_id
        };
        auto result = m_runtime.list_audit_logs(input);
        if (!result) {
            const auto& error = result.error();
            error.send(res, send);
            return;
        }
        res.set_status(interfaces::io::types::Status::OK);
        res.set_body(serde::Ser::serialize("application/json", result->get_document()));
        send();
    }

private:
    ::app::context::AppContext& m_context;
    SuperpositionProxyRuntime& m_runtime;
    SuperpositionProxyInputSupport& m_input_support;
};

} // namespace app::handlers::superposition_proxy
