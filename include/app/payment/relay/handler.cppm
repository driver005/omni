export module app_payment:relay_handler;

import std;
import interfaces;
import core_router;
import core_serde;
import utils_openapi;
import app_model;
import app_dto;
import app_context;
import core_cache;
import core_error;
import app_auth;
import app_http;
import app_crypto;
import app_metrics;
import app_store;
import app_connector;
import app_events;
import app_workflow;
import app_keystore;
import app_locking;
import app_services;

import :relay_input;
import :relay_result;
import :relay_runtime;

export namespace app::handlers::relay {

class RelayRouteHandlers
{
public:
    void handle_unreferenced_refund(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        // Upstream route: ApiKeyAuth with a required profile (relay.rs:14-47).
        if (const auto auth =
                m_request_support
                    .authorize(req, "unreferenced_refund", spec_for("unreferenced_refund"));
            auth.has_value()) {
            const auto& error = *auth;
            error.send(res, send);
            return;
        }

        const auto body = req.body_as_string();
        if (body.empty()) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::EMPTY_REQUEST,
                "empty request"
            );
            error.send(res, send);
            return;
        }
        using Input = ::app::dto::unreferenced_refund::UnreferencedRefundRequest;
        auto parsed = serde::Ser::deserialize<Input>("application/json", body);
        if (!parsed) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INVALID_JSON,
                "invalid JSON"
            );
            error.send(res, send);
            return;
        }

        RelayUnreferencedRefundInput input{
            *parsed,
            std::string{body},
            m_input_support.resolve_scope(req)
        };
        auto result = m_runtime.unreferenced_refund(input);
        if (!result) {
            const auto& error = result.error();
            error.send(res, send);
            return;
        }

        res.set_status(interfaces::io::types::Status::OK);
        res.set_body(serialize_wire(*result));
        send();
    }

    void handle_relay(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        // Upstream route: ApiKeyAuth with connected scope optional (relay.rs:51-81).
        if (const auto auth = m_request_support.authorize(req, "relay", spec_for("relay"));
            auth.has_value()) {
            const auto& error = *auth;
            error.send(res, send);
            return;
        }

        const auto body = req.body_as_string();
        if (body.empty()) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::EMPTY_REQUEST,
                "empty request"
            );
            error.send(res, send);
            return;
        }

        // The wire shape is a tagged union normalized through the generic JSON tree,
        // so the runtime receives the raw body rather than a typed DTO.
        RelayCreateInput input{std::string{body}, m_input_support.resolve_scope(req)};
        auto result = m_runtime.relay(input);
        if (!result) {
            const auto& error = result.error();
            error.send(res, send);
            return;
        }

        res.set_status(interfaces::io::types::Status::OK);
        res.set_body(serialize_wire(*result));
        send();
    }

    void handle_relay_retrieve(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        // Upstream route: ApiKeyAuth with connected scope (relay.rs:85-119).
        if (const auto auth =
                m_request_support.authorize(req, "relay_retrieve", spec_for("relay_retrieve"));
            auth.has_value()) {
            const auto& error = *auth;
            error.send(res, send);
            return;
        }

        auto relay_id = m_input_support.relay_id_from_path(req);
        if (!relay_id.has_value() || relay_id->empty()) {
            using Input = ::app::dto::relay::RelayRetrieveRequest;
            auto parsed = serde::Ser::deserialize<Input>("application/json", req.body_as_string());
            if (parsed && !parsed->get_id().get_value().empty()) {
                relay_id = parsed->get_id().get_value();
            }
        }
        if (!relay_id.has_value() || relay_id->empty()) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INVALID_REQUEST,
                "missing relay id in path"
            );
            error.send(res, send);
            return;
        }
        RelayRetrieveInput input{
            *relay_id,
            m_input_support.force_sync_from_query(req),
            m_input_support.resolve_scope(req)
        };
        auto result = m_runtime.retrieve(input);
        if (!result) {
            const auto& error = result.error();
            error.send(res, send);
            return;
        }

        res.set_status(interfaces::io::types::Status::OK);
        res.set_body(serialize_wire(*result));
        send();
    }

    /**
     * @brief Upstream POST /webhooks/relay/{merchant_id}/{connector_id}.
     *
     * Port of `receive_incoming_relay_webhook` (routes/webhooks.rs:48-83,
     * routes/app.rs:2374-2382). Path identity, MerchantIdAuth and the verbatim
     * body/signature are collected here; the webhook pipeline itself lives in
     * the incoming-webhook runtime.
     */
    void handle_incoming_relay_webhook(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto ids = m_input_support.webhook_path_ids(req);
        if (ids.merchant_id.empty() || ids.connector_id.empty()) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INVALID_REQUEST,
                "missing merchant_id or connector_id"
            );
            error.send(res, send);
            return;
        }

        // Upstream: MerchantIdAuth(merchant_id) with Flow::IncomingWebhookReceive
        // and LockAction::NotApplicable (webhooks.rs:57-80). The path id is the
        // authoritative merchant identity for this auth scheme.
        if (const auto auth = m_request_support.authorize(
                req,
                "receive_incoming_relay_webhook",
                spec_for("receive_incoming_relay_webhook")
            );
            auth.has_value()) {
            const auto& error = *auth;
            error.send(res, send);
            return;
        }

        // Upstream consumes `web::Bytes` verbatim; there is no JSON requirement
        // on the wire, so the raw body is kept for verification and decoding.
        RelayIncomingWebhookInput input{
            ids.merchant_id,
            ids.connector_id,
            req.body_as_string(),
            m_input_support.webhook_signature(req)
        };
        auto result = m_runtime.incoming_webhook(input);
        if (!result) {
            const auto& error = result.error();
            error.send(res, send);
            return;
        }

        // Upstream WebhookResponse::StatusOk -> HTTP 200; empty typed result DTO.
        res.set_status(interfaces::io::types::Status::OK);
        res.set_body(serialize_wire(*result));
        send();
    }

    void install_routes(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "relay"}
            .add_route(
                Route{""}
                    .post(
                        [this](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
                            handle_relay(req, res, std::move(send));
                        }
                    )
                    .summary("Hyperswitch relay")
            )
            .add_route(
                Route{"unreferenced_refund"}
                    .post(
                        [this](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
                            handle_unreferenced_refund(req, res, std::move(send));
                        }
                    )
                    .summary("Hyperswitch unreferenced_refund")
            )
            .add_route(
                Route{":relay_id"}
                    .get(
                        [this](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
                            handle_relay_retrieve(req, res, std::move(send));
                        }
                    )
                    .summary("Hyperswitch relay_retrieve")
            );

        // Upstream: POST /webhooks/relay/{merchant_id}/{connector_id}
        // (routes/app.rs:2374-2382, routes/webhooks.rs:48-83). Nested
        // single-segment routers build the exact /webhooks/relay/{m}/{c} trie.
        Router{router, "webhooks"}.add_router(
            Router{router, "relay"}.add_router(
                Router{router, ":merchant_id"}.add_route(
                    Route{":connector_id"}
                        .post(
                            [this](
                                interfaces::io::IRequest& req,
                                interfaces::io::IResponse& res,
                                std::function<void()> send
                            )
                            {
                                handle_incoming_relay_webhook(req, res, std::move(send));
                            }
                        )
                        .summary("Hyperswitch receive_incoming_relay_webhook")
                )
            )
        );
    }

    explicit RelayRouteHandlers(::app::context::AppContext& context) noexcept :
        m_runtime{context},
        m_input_support{context},
        m_policy{context, m_input_support},
        m_request_support{m_policy}
    {
    }

private:
    RelayRuntime m_runtime;
    RelayInputSupport m_input_support;
    RelayPolicy m_policy;
    RelayRequestSupport m_request_support;
};

} // namespace app::handlers::relay
