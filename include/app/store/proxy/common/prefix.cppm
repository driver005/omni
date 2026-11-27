export module app_store:proxy_handlers_prefix;

import std;
import interfaces;
import core_router;
import core_serde;
import core_behavior;
import utils_openapi;
import app_context;
import core_error;
import app_context_core;
import app_http;

import :proxy_input;
import :proxy_runtime;

export namespace app::handlers::proxy {

/**
 * Unversioned, config-driven alias POST /{payment_methods_prefix}/proxy.
 *
 * Upstream scope is "/{payment_methods_prefix}/proxy" (default "v2",
 * app.rs:1043-1052, configs.rs:85-89). The explicit /v2 and /v1 docs aliases
 * live in the version partitions; this family registers the configured prefix
 * only when it is non-empty and not "v2" (the v2 route already covers default).
 * The adapter is a verbatim copy; runtime and input helper are shared by ref.
 */
class ProxyHandlersPrefix
{
public:
    ProxyHandlersPrefix(
        ::app::context::AppContext& context,
        ProxyRuntime& runtime,
        ProxyInputSupport& input_support,
        core::behavior::RequestSupport<ProxyPolicy>& request_support
    ) noexcept :
        m_context{context},
        m_runtime{runtime},
        m_input_support{input_support},
        m_request_support{request_support}
    {
    }

    void handle_proxy(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto auth_error = m_request_support.authorize(req, "proxy", {});
        if (auth_error) {
            const auto& error = *auth_error;
            error.send(res, send);
            return;
        }
        const auto auth = ::app::auth::AuthContextBuilder{}(req);

        // Bind the forward to the authenticated identity, never to the request body.
        const auto merchant_id = m_request_support.scope(req).merchant_id;

        const auto body = req.body_as_string();
        auto parsed = m_input_support.parse_request(body);
        if (!parsed) {
            const auto& error = parsed.error();
            error.send(res, send);
            return;
        }
        ProxyInput input{*parsed, std::string{merchant_id}, auth};

        auto result = m_runtime.proxy(input);
        if (!result) {
            const auto& error = result.error();
            error.send(res, send);
            return;
        }

        res.set_status(interfaces::io::types::Status::OK);
        res.set_body(serde::Ser::serialize("application/json", *result));
        send();
    }

    void install(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;

        const auto prefix = m_context.config_store()
                                .get_value("micro_services", "payment_methods_prefix")
                                .value_or(std::string{"v2"});
        if (prefix.empty() || prefix == "v2") {
            return;
        }

        Router{router, "/" + prefix + "/proxy"}.add_route(
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
    ::app::context::AppContext& m_context;
    ProxyRuntime& m_runtime;
    ProxyInputSupport& m_input_support;
    core::behavior::RequestSupport<ProxyPolicy>& m_request_support;
};

} // namespace app::handlers::proxy
