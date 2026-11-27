export module app_webhook:webhooks_handlers_v1;

import std;
import interfaces;
import core_router;
import core_serde;
import core_behavior;
import utils_openapi;
import app_context;
import core_error;
import app_http;

import :webhooks_input;
import :webhook_policy;
import :webhooks_result;
import :webhooks_runtime;
import :webhooks_handlers_common;

export namespace app::handlers::webhooks {

/** @brief v1 webhook route family.
 *
 *  Upstream POST/GET/PUT /webhooks/{merchant_id}/{connector_id_or_name},
 *  MerchantIdAuth(path) (routes/webhooks.rs:11-44, app.rs:2352-2357).
 */
class WebhooksHandlersV1
{
public:
    using RequestSupport = ::core::behavior::RequestSupport<::app::webhook::policy::WebhooksPolicy>;

    WebhooksHandlersV1(
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

    void handle_receive_incoming_webhook(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};
        if (const auto auth_error = m_request_support.authorize(
                req,
                "receive_incoming_webhook",
                ::app::webhook::policy::kWebhooksMerchantAuth
            )) {
            auth_error->send(sink);
            return;
        }

        const auto body = req.body_as_string();
        auto ctx = m_request_support.context(req);
        ctx.operation = "receive_incoming_webhook";
        ctx.profile_id.clear(); // v1 route is merchant-only
        m_runtime.receive_incoming_webhook(body, ctx).handle(sink);
    }

    void install(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "/webhooks"}
            .add_router(
                Router{router, "webhooks"}.add_router(
                    Router{router, ":merchant_id"}.add_route(
                        Route{":connector_id_or_name"}
                            .post(
                                [this](
                                    interfaces::io::IRequest& req,
                                    interfaces::io::IResponse& res,
                                    std::function<void()> send
                                )
                                {
                                    handle_receive_incoming_webhook(req, res, std::move(send));
                                }
                            )
                            .summary("Hyperswitch receive_incoming_webhook")

                    )
                )
            )
            .add_router(
                Router{router, "webhooks"}.add_router(
                    Router{router, ":merchant_id"}.add_route(
                        Route{":connector_id_or_name"}
                            .get(
                                [this](
                                    interfaces::io::IRequest& req,
                                    interfaces::io::IResponse& res,
                                    std::function<void()> send
                                )
                                {
                                    handle_receive_incoming_webhook(req, res, std::move(send));
                                }
                            )
                            .summary("Hyperswitch receive_incoming_webhook")

                    )
                )
            )
            .add_router(
                Router{router, "webhooks"}.add_router(
                    Router{router, ":merchant_id"}.add_route(
                        Route{":connector_id_or_name"}
                            .put(
                                [this](
                                    interfaces::io::IRequest& req,
                                    interfaces::io::IResponse& res,
                                    std::function<void()> send
                                )
                                {
                                    handle_receive_incoming_webhook(req, res, std::move(send));
                                }
                            )
                            .summary("Hyperswitch receive_incoming_webhook")

                    )
                )
            );
        // POST /webhooks/relay/:merchant_id/:connector_id is registered only by
        // relay.cppm:781 (cross-module ownership); registering it here as well would
        // throw std::runtime_error on duplicate POST in the core router.
    }

private:
    ::app::context::AppContext& m_context;
    WebhooksRuntime& m_runtime;
    WebhooksInputSupport& m_input_support;
    RequestSupport& m_request_support;
};

} // namespace app::handlers::webhooks
