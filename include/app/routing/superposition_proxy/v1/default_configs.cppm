export module app_routing:superposition_proxy_handlers_v1_default_configs;

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
 * v1 default-config family: `v1/superposition/default-config` GET and
 * `v1/superposition/default-config/:key` GET adapters. Bodies are verbatim
 * from the original combined handler; runtime and input support are shared.
 */
class SuperpositionProxyHandlersV1DefaultConfigs
{
public:
    SuperpositionProxyHandlersV1DefaultConfigs(
        ::app::context::AppContext& context,
        SuperpositionProxyRuntime& runtime,
        SuperpositionProxyInputSupport& input_support
    ) noexcept :
        m_context{context},
        m_runtime{runtime},
        m_input_support{input_support}
    {
    }

    // ── GET /v1/superposition/default-config ───────────────────────────────────
    void handle_list_default_configs(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto auth_error = m_input_support.authorize_request(req, "list_default_configs");
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

        ListDefaultConfigsHandlerInput input{
            m_input_support.list_default_configs_query(m_input_support.query_pairs(req)),
            ::app::auth::AuthContextBuilder{}(req),
            scope->org_id,
            scope->workspace_id
        };
        auto result = m_runtime.list_default_configs(input);
        if (!result) {
            const auto& error = result.error();
            error.send(res, send);
            return;
        }
        res.set_status(interfaces::io::types::Status::OK);
        res.set_body(serde::Ser::serialize("application/json", result->get_document()));
        send();
    }

    // ── GET /v1/superposition/default-config/{key} ──────────────────────────────
    void handle_get_default_config(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto auth_error = m_input_support.authorize_request(req, "get_default_config");
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

        GetDefaultConfigHandlerInput input{
            GetDefaultConfigRequest{m_input_support.last_path_segment(req)},
            ::app::auth::AuthContextBuilder{}(req),
            scope->org_id,
            scope->workspace_id
        };
        auto result = m_runtime.get_default_config(input);
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
