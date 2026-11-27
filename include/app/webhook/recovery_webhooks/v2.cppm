export module app_webhook:recovery_webhooks_handlers_v2;

import std;
import interfaces;
import core_router;
import core_serde;
import core_behavior;
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

import :recovery_webhooks_input;
import :webhook_policy;
import :recovery_webhooks_result;
import :recovery_webhooks_runtime;

export namespace app::handlers::recovery_webhooks {

class RecoveryWebhooksHandlersV2
{
public:
    using RequestSupport =
        ::core::behavior::RequestSupport<::app::webhook::policy::RecoveryWebhooksPolicy>;

    RecoveryWebhooksHandlersV2(
        ::app::context::AppContext& context,
        RecoveryWebhooksRuntime& runtime,
        RecoveryWebhooksInputSupport& input_support,
        RequestSupport& request_support
    ) noexcept :
        m_context{context},
        m_runtime{runtime},
        m_input_support{input_support},
        m_request_support{request_support}
    {
    }

    /**
     * Upstream route `recovery_receive_incoming_webhook`
     * (routes/recovery_webhooks.rs:11-49). Binds the URL merchant/profile into
     * `MerchantIdAndProfileIdAuth` and wraps the core with `server_wrap` using
     * `LockAction::NotApplicable` and `Flow::RecoveryIncomingWebhookReceive`.
     */
    void handle_recovery_receive_incoming_webhook(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};
        // 1. request -> Input.
        const auto ids = m_input_support.recovery_ids(req);
        if (ids.merchant_id.empty() || ids.profile_id.empty() || ids.connector_id.empty()) {
            const auto error = m_input_support.invalid_request_data(
                "missing merchant_id, profile_id or connector_id"
            );
            error.send(sink);
            return;
        }

        // MerchantIdAndProfileIdAuth (recovery_webhooks.rs:42-45): bind URL ids.
        if (const auto auth =
                m_request_support.authorize(req, "recovery_receive_incoming_webhook", {})) {
            const auto& error = *auth;
            error.send(sink);
            return;
        }

        // web::Bytes body (recovery_webhooks.rs:15, incoming_v2.rs:133).
        const auto body = req.body_as_string();
        if (body.empty()) {
            const auto error = m_input_support.invalid_request_data("empty webhook body");
            error.send(sink);
            return;
        }

        const auto request_details = m_input_support.build_request_details(req, body);
        auto ctx = m_request_support.context(req);
        ctx.org_id = ::app::http::org_id_from_header{}(req);
        m_runtime.recovery_receive_incoming_webhook(request_details, ctx).handle(sink);
    }

    void install(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "v2/webhooks"}.add_router(
            Router{router, "recovery"}.add_router(
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
                                    handle_recovery_receive_incoming_webhook(
                                        req,
                                        res,
                                        std::move(send)
                                    );
                                }
                            )
                            .summary("Hyperswitch recovery_receive_incoming_webhook")
                            .description(
                                "POST "
                                "/v2/webhooks/recovery/{merchant_id}/{profile_id}/{connector_id} "
                                "(upstream routes/recovery_webhooks.rs:11-49)"
                            )
                    )
                )
            )
        );
    }

private:
    ::app::context::AppContext& m_context;
    RecoveryWebhooksRuntime& m_runtime;
    RecoveryWebhooksInputSupport& m_input_support;
    RequestSupport& m_request_support;
};

} // namespace app::handlers::recovery_webhooks
