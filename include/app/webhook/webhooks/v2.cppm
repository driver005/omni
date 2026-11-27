export module app_webhook:webhooks_handlers_v2;

import std;
import interfaces;
import core_router;
import core_serde;
import core_behavior;
import utils_openapi;
import app_context;
import core_error;

import :webhooks_input;
import :webhook_policy;
import :webhooks_result;
import :webhooks_runtime;
import :webhooks_handlers_common;

export namespace app::handlers::webhooks {

/** @brief v2 webhook route family.
 *
 *  Upstream POST/GET/PUT /v2/webhooks/{merchant_id}/{profile_id}/{connector_id},
 *  MerchantIdAndProfileIdAuth(path) (routes/webhooks.rs:129-170, app.rs:2391-2395).
 */
class WebhooksHandlersV2
{
public:
    using RequestSupport = ::core::behavior::RequestSupport<::app::webhook::policy::WebhooksPolicy>;

    WebhooksHandlersV2(
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

    void handle_receive_incoming_webhook_v2(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};
        if (const auto auth_error = m_request_support.authorize(
                req,
                "receive_incoming_webhook_v2",
                ::app::webhook::policy::kWebhooksMerchantProfileAuth
            )) {
            auth_error->send(sink);
            return;
        }

        const auto body = req.body_as_string();
        auto ctx = m_request_support.context(req);
        ctx.operation = "receive_incoming_webhook_v2";
        m_runtime.receive_incoming_webhook_v2(body, ctx).handle(sink);
    }

    void install(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "/webhooks"}
            .add_router(
                Router{router, "v2"}.add_router(
                    Router{router, "webhooks"}.add_router(
                        Router{router, ":merchant_id"}.add_router(
                            Router{router, ":profile_id"}.add_route(
                                Route{":connector_id"}
                                    .post(
                                        [this](
                                            interfaces::io::IRequest& req,
                                            interfaces::io::IResponse& res,
                                            std::function<void()> send
                                        )
                                        {
                                            handle_receive_incoming_webhook_v2(
                                                req,
                                                res,
                                                std::move(send)
                                            );
                                        }
                                    )
                                    .summary("Hyperswitch receive_incoming_webhook")

                            )
                        )
                    )
                )
            )
            .add_router(
                Router{router, "v2"}.add_router(
                    Router{router, "webhooks"}.add_router(
                        Router{router, ":merchant_id"}.add_router(
                            Router{router, ":profile_id"}.add_route(
                                Route{":connector_id"}
                                    .get(
                                        [this](
                                            interfaces::io::IRequest& req,
                                            interfaces::io::IResponse& res,
                                            std::function<void()> send
                                        )
                                        {
                                            handle_receive_incoming_webhook_v2(
                                                req,
                                                res,
                                                std::move(send)
                                            );
                                        }
                                    )
                                    .summary("Hyperswitch receive_incoming_webhook")

                            )
                        )
                    )
                )
            )
            .add_router(
                Router{router, "v2"}.add_router(
                    Router{router, "webhooks"}.add_router(
                        Router{router, ":merchant_id"}.add_router(
                            Router{router, ":profile_id"}.add_route(
                                Route{":connector_id"}
                                    .put(
                                        [this](
                                            interfaces::io::IRequest& req,
                                            interfaces::io::IResponse& res,
                                            std::function<void()> send
                                        )
                                        {
                                            handle_receive_incoming_webhook_v2(
                                                req,
                                                res,
                                                std::move(send)
                                            );
                                        }
                                    )
                                    .summary("Hyperswitch receive_incoming_webhook")

                            )
                        )
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
