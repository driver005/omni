export module app_webhook:webhooks_handlers_common;

import std;
import interfaces;
import core_router;
import core_serde;
import core_error;
import utils_openapi;
import app_context;
import app_http;

import :webhooks_input;
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
    WebhooksHandlersCommon(
        ::app::context::AppContext& ctx,
        WebhooksRuntime& runtime,
        WebhooksInputSupport& input_support
    ) noexcept :
        m_context{ctx},
        m_runtime{runtime},
        m_input_support{input_support}
    {
    }

    void handle_receive_network_token_requestor_incoming_webhook(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        // 1. request -> Input.
        PathIds ids;
        ids.merchant_id = ::app::http::merchant_id_from_header{}(req);
        if (const auto auth_error = m_input_support.authorize(req, AuthVariant::NO_AUTH, ids)) {
            res.set_status((*auth_error).get_status());
            res.set_body(serde::Ser::serialize("application/json", *auth_error));
            send();
            return;
        }
        ReceiveNetworkTokenInput input{
            ids,
            req.body_as_string(),
            m_input_support.header_reader(req),
            "receive_network_token_requestor_incoming_webhook",
            "IncomingNetworkTokenWebhookReceive",
            "WEBHOOK_NETWORK_TOKEN_RECEIVED"
        };

        // 2. Input -> Result.
        auto result = m_runtime.receive_network_token(input);
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
};

} // namespace app::handlers::webhooks
