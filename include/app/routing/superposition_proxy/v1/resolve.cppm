export module app_routing:superposition_proxy_handlers_v1_resolve;

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
 * v1 resolve family: `v1/superposition/config/resolve/detailed` POST and
 * `v1/superposition/config/resolve/explain/:key` POST adapters. Bodies are
 * verbatim from the original combined handler; runtime and input support
 * are owned by the facade and shared by reference.
 */
class SuperpositionProxyHandlersV1Resolve
{
public:
    SuperpositionProxyHandlersV1Resolve(
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

    // ── POST /v1/superposition/config/resolve/detailed ──────────────────────────
    void handle_resolve_detailed_config(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto auth_error = m_request_support.authorize(req, "resolve_detailed_config", {});
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

        const auto body = req.body_as_string();
        if (body.empty()) {
            const auto error =
                m_input_support.make_api_error(m_input_support.bad_request("empty request"));
            error.send(res, send);
            return;
        }
        auto parsed = serde::Ser::decode_generic("application/json", body);
        if (!parsed) {
            const auto error =
                m_input_support.make_api_error(m_input_support.bad_request("invalid JSON"));
            error.send(res, send);
            return;
        }
        ResolveDetailedConfigHandlerInput input{
            ResolveDetailedConfigRequest{m_input_support.context_entries(*parsed)},
            ::app::auth::AuthContextBuilder{}(req),
            scope->org_id,
            scope->workspace_id
        };
        auto result = m_runtime.resolve_detailed_config(input);
        if (!result) {
            const auto& error = result.error();
            error.send(res, send);
            return;
        }
        res.set_status(interfaces::io::types::Status::OK);
        res.set_body(serde::Ser::serialize("application/json", *result));
        send();
    }

    // ── POST /v1/superposition/config/resolve/explain/{key} ─────────────────────
    void handle_resolve_config_explanation(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto auth_error = m_request_support.authorize(req, "resolve_config_explanation", {});
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

        const auto key = m_request_support.path(req, ::app::routing::policy::last_path_index(req));
        const auto body = req.body_as_string();
        if (body.empty()) {
            const auto error =
                m_input_support.make_api_error(m_input_support.bad_request("empty request"));
            error.send(res, send);
            return;
        }
        auto parsed = serde::Ser::decode_generic("application/json", body);
        if (!parsed) {
            const auto error =
                m_input_support.make_api_error(m_input_support.bad_request("invalid JSON"));
            error.send(res, send);
            return;
        }
        ResolveConfigExplanationHandlerInput input{
            ResolveConfigExplanationRequest{key, m_input_support.context_entries(*parsed)},
            ::app::auth::AuthContextBuilder{}(req),
            scope->org_id,
            scope->workspace_id
        };
        auto result = m_runtime.resolve_config_explanation(input);
        if (!result) {
            const auto& error = result.error();
            error.send(res, send);
            return;
        }
        res.set_status(interfaces::io::types::Status::OK);
        res.set_body(serde::Ser::serialize("application/json", *result));
        send();
    }

private:
    ::app::context::AppContext& m_context;
    SuperpositionProxyRuntime& m_runtime;
    SuperpositionProxyInputSupport& m_input_support;
    SuperpositionProxyRequestSupport& m_request_support;
};

} // namespace app::handlers::superposition_proxy
