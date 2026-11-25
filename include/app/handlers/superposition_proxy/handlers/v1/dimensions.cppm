export module app_handlers_superposition_proxy:handlers_v1_dimensions;

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

import :input;
import :result;
import :runtime;

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
        app::context::AppContext& context,
        SuperpositionProxyRuntime& runtime,
        SuperpositionProxyInputSupport& input_support
    ) noexcept :
        m_context{context},
        m_runtime{runtime},
        m_input_support{input_support}
    {
    }

    // ── GET /v1/superposition/dimension ─────────────────────────────────────────
    void handle_list_dimensions(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto auth_error = m_input_support.authorize_request(req, "list_dimensions");
        if (auth_error) {
            const auto& error = *auth_error;
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }
        auto scope = m_input_support.extract_proxy_headers(req);
        if (!scope) {
            const auto& error = scope.error();
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }
        m_input_support.cache_scope(req, *scope);

        ListDimensionsHandlerInput input{
            m_input_support.list_dimensions_query(m_input_support.query_pairs(req)),
            app::auth::AuthContextBuilder{}(req),
            scope->org_id,
            scope->workspace_id
        };
        auto result = m_runtime.list_dimensions(input);
        if (!result) {
            const auto& error = result.error();
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }
        res.set_status(interfaces::io::types::Status::OK);
        res.set_body(serde::Ser::serialize("application/json", result->get_document()));
        send();
    }

    // ── GET /v1/superposition/dimension/{dimension_name} ────────────────────────
    void handle_get_dimension(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto auth_error = m_input_support.authorize_request(req, "get_dimension");
        if (auth_error) {
            const auto& error = *auth_error;
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }
        auto scope = m_input_support.extract_proxy_headers(req);
        if (!scope) {
            const auto& error = scope.error();
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }
        m_input_support.cache_scope(req, *scope);

        GetDimensionHandlerInput input{
            GetDimensionRequest{m_input_support.last_path_segment(req)},
            app::auth::AuthContextBuilder{}(req),
            scope->org_id,
            scope->workspace_id
        };
        auto result = m_runtime.get_dimension(input);
        if (!result) {
            const auto& error = result.error();
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }
        res.set_status(interfaces::io::types::Status::OK);
        res.set_body(serde::Ser::serialize("application/json", result->get_document()));
        send();
    }

private:
    app::context::AppContext& m_context;
    SuperpositionProxyRuntime& m_runtime;
    SuperpositionProxyInputSupport& m_input_support;
};

} // namespace app::handlers::superposition_proxy
