export module app_webhook:webhooks_handlers_v1;

import std;
import interfaces;
import core_router;
import core_serde;
import utils_openapi;
import app_context;
import core_error;
import app_http;

import :webhooks_input;
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
    WebhooksHandlersV1(
        ::app::context::AppContext& ctx,
        WebhooksRuntime& runtime,
        WebhooksInputSupport& input_support
    ) noexcept :
        m_context{ctx},
        m_runtime{runtime},
        m_input_support{input_support}
    {
    }

    void handle_receive_incoming_webhook(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        // 1. request -> Input.
        const auto ids = m_input_support.path_ids(req, false);
        if (const auto auth_error = m_input_support.authorize(req, AuthVariant::MERCHANT_ID, ids)) {
            res.set_status((*auth_error).get_status());
            res.set_body(serde::Ser::serialize("application/json", *auth_error));
            send();
            return;
        }
        ReceiveIncomingWebhookInput input{
            ids,
            req.body_as_string(),
            m_input_support.header_reader(req),
            "receive_incoming_webhook",
            "IncomingWebhookReceive",
            "WEBHOOK_INCOMING_RECEIVED"
        };

        // 2. Input -> Result.
        auto result = m_runtime.receive_incoming_webhook(input);
        if (!result) {
            res.set_status((result.error()).get_status());
            res.set_body(serde::Ser::serialize("application/json", result.error()));
            send();
            return;
        }

        // 3. Result -> response (direct writes, original send_ack shape).
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
};

} // namespace app::handlers::webhooks
