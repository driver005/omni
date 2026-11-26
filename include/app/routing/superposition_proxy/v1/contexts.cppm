export module app_routing:superposition_proxy_handlers_v1_contexts;

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
 * v1 context family: `v1/superposition/context` GET/PUT adapters. Bodies are
 * verbatim from the original combined handler; runtime and input support are
 * owned by the facade and shared by reference.
 */
class SuperpositionProxyHandlersV1Contexts
{
public:
    SuperpositionProxyHandlersV1Contexts(
        ::app::context::AppContext& context,
        SuperpositionProxyRuntime& runtime,
        SuperpositionProxyInputSupport& input_support
    ) noexcept :
        m_context{context},
        m_runtime{runtime},
        m_input_support{input_support}
    {
    }

    // ── GET /v1/superposition/context ──────────────────────────────────────────
    void handle_list_contexts(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto auth_error = m_input_support.authorize_request(req, "list_contexts");
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

        ListContextsHandlerInput input{
            m_input_support.list_contexts_query(m_input_support.query_pairs(req)),
            ::app::auth::AuthContextBuilder{}(req),
            scope->org_id,
            scope->workspace_id
        };
        auto result = m_runtime.list_contexts(input);
        if (!result) {
            const auto& error = result.error();
            error.send(res, send);
            return;
        }
        res.set_status(interfaces::io::types::Status::OK);
        res.set_body(serde::Ser::serialize("application/json", result->get_document()));
        send();
    }

    // ── PUT /v1/superposition/context ──────────────────────────────────────────
    void handle_create_context(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto auth_error = m_input_support.authorize_request(req, "create_context");
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
        CreateContextHandlerInput input{
            m_input_support.context_put_request_from(*parsed),
            ::app::auth::AuthContextBuilder{}(req),
            scope->org_id,
            scope->workspace_id
        };
        auto result = m_runtime.create_context(input);
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
