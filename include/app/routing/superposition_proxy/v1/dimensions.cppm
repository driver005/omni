export module app_routing:superposition_proxy_handlers_v1_dimensions;

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
import :policy;
import :superposition_proxy_runtime;

export namespace app::handlers::superposition_proxy {

/**
 * v1 dimension family: `v1/superposition/dimension` GET and
 * `v1/superposition/dimension/:dimension_name` GET adapters. Bodies are
 * verbatim from the original combined handler; runtime and input support
 * are owned by the facade and shared by reference.
 */
class SuperpositionProxyHandlersV1Dimensions
{
public:
    SuperpositionProxyHandlersV1Dimensions(
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

    // ── GET /v1/superposition/dimension ─────────────────────────────────────────
    void handle_list_dimensions(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};
        const auto auth_error = m_request_support.authorize(req, "list_dimensions", {});
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

        auto request = m_input_support.list_dimensions_query(m_input_support.query_pairs(req));
        m_runtime.list_dimensions(request, ctx).handle(sink);
    }

    // ── GET /v1/superposition/dimension/{dimension_name} ────────────────────────
    void handle_get_dimension(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};
        const auto auth_error = m_request_support.authorize(req, "get_dimension", {});
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

        auto request = GetDimensionRequest{
            m_request_support.path(req, ::app::routing::policy::last_path_index(req))
        };
        m_runtime.get_dimension(request, ctx).handle(sink);
    }

private:
    ::app::context::AppContext& m_context;
    SuperpositionProxyRuntime& m_runtime;
    SuperpositionProxyInputSupport& m_input_support;
    SuperpositionProxyRequestSupport& m_request_support;
};

} // namespace app::handlers::superposition_proxy
