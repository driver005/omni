export module app_webhook:webhooks_handlers_common;

import std;
import interfaces;
import core_router;
import core_serde;
import core_behavior;
import core_error;
import utils_openapi;
import app_context;
import app_http;

import :webhooks_input;
import :webhook_policy;
import :webhooks_result;
import :webhooks_runtime;

export namespace app::handlers::webhooks {

using Status = interfaces::io::types::Status;

/** @brief Unversioned webhook route family (network token requestor).
 *
 *  Upstream POST/GET/PUT /webhooks/network_token_requestor/ref, NoAuth
 *  (routes/webhooks.rs:172-194, app.rs:2346-2351). Core still verifies the
 *  Authorization header against the network-tokenization service secret
 *  (incoming.rs:863-934, network_tokenization_incoming.rs:201-238).
 */
class WebhooksHandlersCommon
{
public:
    using RequestSupport = ::core::behavior::RequestSupport<::app::webhook::policy::WebhooksPolicy>;

    WebhooksHandlersCommon(
        ::app::context::AppContext& ctx,
        WebhooksRuntime& runtime,
        WebhooksInputSupport& input_support,
        RequestSupport& request_support
    ) noexcept :
        m_context{ctx},
        m_runtime{runtime},
        m_input_support{input_support},
        m_request_support{request_support}
    {
    }

    void handle_receive_network_token_requestor_incoming_webhook(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};
        if (const auto auth_error = m_request_support.authorize(
                req,
                "receive_network_token_requestor_incoming_webhook",
                ::app::webhook::policy::kWebhooksNoAuth
            )) {
            auth_error->send(sink);
            return;
        }

        const auto body = req.body_as_string();
        auto ctx = m_request_support.context(req);
        ctx.operation = "receive_network_token_requestor_incoming_webhook";
        ctx.merchant_id = ::app::http::merchant_id_from_header{}(req);
        m_runtime.receive_network_token(body, ctx).handle(sink);
    }

    void install(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "/webhooks"}
            .add_router(
                Router{router, "webhooks"}.add_router(
                    Router{router, "network_token_requestor"}.add_route(
                        Route{"ref"}
                            .post(
                                [this](
                                    interfaces::io::IRequest& req,
                                    interfaces::io::IResponse& res,
                                    std::function<void()> send
                                )
                                {
                                    handle_receive_network_token_requestor_incoming_webhook(
                                        req,
                                        res,
                                        std::move(send)
                                    );
                                }
                            )
                            .summary("Hyperswitch receive_network_token_requestor_incoming_webhook")

                    )
                )
            )
            .add_router(
                Router{router, "webhooks"}.add_router(
                    Router{router, "network_token_requestor"}.add_route(
                        Route{"ref"}
                            .get(
                                [this](
                                    interfaces::io::IRequest& req,
                                    interfaces::io::IResponse& res,
                                    std::function<void()> send
                                )
                                {
                                    handle_receive_network_token_requestor_incoming_webhook(
                                        req,
                                        res,
                                        std::move(send)
                                    );
                                }
                            )
                            .summary("Hyperswitch receive_network_token_requestor_incoming_webhook")

                    )
                )
            )
            .add_router(
                Router{router, "webhooks"}.add_router(
                    Router{router, "network_token_requestor"}.add_route(
                        Route{"ref"}
                            .put(
                                [this](
                                    interfaces::io::IRequest& req,
                                    interfaces::io::IResponse& res,
                                    std::function<void()> send
                                )
                                {
                                    handle_receive_network_token_requestor_incoming_webhook(
                                        req,
                                        res,
                                        std::move(send)
                                    );
                                }
                            )
                            .summary("Hyperswitch receive_network_token_requestor_incoming_webhook")

                    )
                )
            );
    }

private:
    ::app::context::AppContext& m_context;
    WebhooksRuntime& m_runtime;
    WebhooksInputSupport& m_input_support;
    RequestSupport& m_request_support;
};

} // namespace app::handlers::webhooks
