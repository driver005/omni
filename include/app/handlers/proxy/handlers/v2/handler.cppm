export module app_handlers_proxy:handlers_v2;

import std;
import interfaces;
import core_router;
import core_serde;
import utils_openapi;
import app_context;
import core_error;
import app_auth;
import app_http;

import :input;
import :result;
import :runtime;

export namespace app::handlers::proxy {

/**
 * v2 surface: docs alias POST /v2/proxy (openapi/routes/proxy.rs:7).
 *
 * handle_proxy is a distinct copy of the thin adapter — v2 does not share a
 * method with v1 or the prefix alias. Runtime and input helper are owned once
 * by the facade and shared by reference.
 */
class ProxyHandlersV2
{
public:
    ProxyHandlersV2(
        app::context::AppContext& context,
        ProxyRuntime& runtime,
        ProxyInputSupport& input_support
    ) noexcept :
        m_context{context},
        m_runtime{runtime},
        m_input_support{input_support}
    {
    }

    void handle_proxy(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto authorized = m_input_support.authorize(req);
        if (authorized.error) {
            const auto& error = *authorized.error;
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }

        // Bind the forward to the authenticated identity, never to the request body.
        const auto header_merchant_id = app::http::merchant_id_from_header{}(req);
        const auto merchant_id =
            header_merchant_id.empty() ? authorized.context.merchant_id() : header_merchant_id;

        const auto body = req.body_as_string();
        auto parsed = m_input_support.parse_request(body);
        if (!parsed) {
            const auto& error = parsed.error();
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }
        ProxyInput input{*parsed, std::string{merchant_id}, authorized.context};

        auto result = m_runtime.proxy(input);
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

    void install(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;

        Router{router, "/v2/proxy"}.add_route(
            Route{""}
                .post(
                    [this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    )
                    {
                        handle_proxy(req, res, std::move(send));
                    }
                )
                .summary("Hyperswitch proxy")
        );
    }

private:
    app::context::AppContext& m_context;
    ProxyRuntime& m_runtime;
    ProxyInputSupport& m_input_support;
};

} // namespace app::handlers::proxy
