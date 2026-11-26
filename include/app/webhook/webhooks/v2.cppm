export module app_webhook:webhooks_handlers_v2;

import std;
import interfaces;
import core_router;
import core_serde;
import utils_openapi;
import app_context;
import core_error;

import :webhooks_input;
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
    WebhooksHandlersV2(
        ::app::context::AppContext& ctx,
        WebhooksRuntime& runtime,
        WebhooksInputSupport& input_support
    ) noexcept :
        m_context{ctx},
        m_runtime{runtime},
        m_input_support{input_support}
    {
    }

    void handle_receive_incoming_webhook_v2(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        // 1. request -> Input.
        const auto ids = m_input_support.path_ids(req, true);
        if (const auto auth_error =
                m_input_support.authorize(req, AuthVariant::MERCHANT_AND_PROFILE_ID, ids)) {
            res.set_status((*auth_error).get_status());
            res.set_body(serde::Ser::serialize("application/json", *auth_error));
            send();
            return;
        }
        ReceiveIncomingWebhookV2Input input{
            ids,
            req.body_as_string(),
            m_input_support.header_reader(req),
            "receive_incoming_webhook",
            "IncomingWebhookReceive",
            "WEBHOOK_V2_RECEIVED"
        };

        // 2. Input -> Result.
        auto result = m_runtime.receive_incoming_webhook_v2(input);
        if (!result) {
            res.set_status((result.error()).get_status());
            res.set_body(serde::Ser::serialize("application/json", result.error()));
            send();
            return;
        }

        // 3. Result -> response.
        const auto& ack = result->get_ack();
        if (ack.kind() == AckKind::STATUS_OK) {
            res.set_status(interfaces::io::types::Status::OK);
            res.set_body(std::vector<std::byte>{});
            send();
        } else if (ack.kind() == AckKind::FILE_DATA) {
            for (const auto& [name, value]: ack.headers()) {
                res.add_header(name, value);
            }
            res.add_header("content-type", ack.mime());
            res.set_status(interfaces::io::types::Status::OK);
            res.set_body(serde::Ser::serialize("application/json", result->get_body()));
            send();
        } else if (ack.kind() == AckKind::JSON_WITH_HEADERS) {
            for (const auto& [name, value]: ack.headers()) {
                res.add_header(name, value);
            }
            res.set_status(interfaces::io::types::Status::OK);
            res.set_body(serde::Ser::serialize("application/json", result->get_body()));
            send();
        } else {
            res.set_status(interfaces::io::types::Status::OK);
            res.set_body(serde::Ser::serialize("application/json", result->get_body()));
            send();
        }
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
};

} // namespace app::handlers::webhooks
