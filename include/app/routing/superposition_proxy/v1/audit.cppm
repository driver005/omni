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
        SuperpositionProxyInputSupport& input_support,
        SuperpositionProxyRequestSupport& request_support
    ) noexcept :
        m_context{context},
        m_runtime{runtime},
        m_input_support{input_support},
        m_request_support{request_support}
    {
    }

    // ── GET /v1/superposition/audit ─────────────────────────────────────────────
    void handle_list_audit_logs(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};
        const auto auth_error = m_request_support.authorize(req, "list_audit_logs", {});
        if (auth_error) {
            const auto& error = *auth_error;
            error.send(sink);
            return;
        }
        auto scope = m_input_support.extract_proxy_headers(req);
        if (!scope) {
            const auto& error = scope.error();
            error.send(sink);
            return;
        }
        m_input_support.cache_scope(req, *scope);
        auto ctx = m_request_support.context(req);
        ctx.org_id = scope->org_id;
        ctx.headers["x-workspace"] = scope->workspace_id;

        auto request = m_input_support.list_audit_logs_query(m_input_support.query_pairs(req));
        m_runtime.list_audit_logs(request, ctx).handle(sink);
    }

private:
    ::app::context::AppContext& m_context;
    SuperpositionProxyRuntime& m_runtime;
    SuperpositionProxyInputSupport& m_input_support;
    SuperpositionProxyRequestSupport& m_request_support;
};

} // namespace app::handlers::superposition_proxy
