export module app_webhook:webhook_events_handler;

import std;
import interfaces;
import core_router;
import core_serde;
import core_behavior;
import core_error;
import utils_openapi;
import app_model;
import app_dto;
import app_context;
import app_auth;
import app_http;
import app_crypto;
import app_metrics;
import app_store;
import app_connector;
import app_events;
import app_workflow;
import app_keystore;

import :webhook_events_input;
import :webhook_policy;
import :webhook_events_result;
import :webhook_events_runtime;

export namespace app::handlers::webhook_events {

class WebhookEventsRouteHandlers
{
public:
    // Upstream: POST /events/profile/list
    // routes/webhook_events.rs:57-103 -> JWTAuth(ProfileWebhookEventRead,
    // allow_connected, allow_platform) -> core list_initial_delivery_attempts.
    // The merchant AND profile are forced from UserFromToken into the constraints;
    // the body never supplies them (routes/webhook_events.rs:76-86).
    void handle_list_initial_webhook_delivery_attempts_with_jwtauth(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};
        if (const auto auth_error =
                m_request_support
                    .authorize(req, "list_initial_webhook_delivery_attempts_with_jwtauth", {})) {
            const auto& error = *auth_error;
            error.send(sink);
            return;
        }

        auto ctx = m_request_support.context(req);
        if (ctx.merchant_id.empty()) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INVALID_REQUEST,
                "missing merchant_id"
            );
            error.send(sink);
            return;
        }

        using Dto = ::app::dto::webhook_events::EventListConstraints;
        auto parsed = core::behavior::Serializer<Dto>::pass(req, sink);
        if (!parsed) {
            return;
        }

        // Upstream validates constraints (foreign_try_from) before the profile lookup.
        if (const auto profile_error =
                m_input_support.validate_profile(ctx.profile_id, ctx.merchant_id)) {
            const auto& error = *profile_error;
            error.send(sink);
            return;
        }

        // UserFromToken anchoring (routes/webhook_events.rs:80-85): override body profile.
        ::app::dto::webhook_events::ProfileIdValue anchored;
        anchored.set_value(ctx.profile_id);
        parsed->set_profile_id(std::move(anchored));

        m_runtime.list_initial_with_jwtauth(*parsed, ctx).handle(sink);
    }

    // Upstream: POST /events/{merchant_id}
    // routes/webhook_events.rs:18-55 -> JWTAuthMerchantFromRoute(merchant_id,
    // MerchantWebhookEventRead, allow_connected, allow_platform).
    void handle_list_initial_webhook_delivery_attempts(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};
        const auto ids = m_input_support.path_ids(req);
        if (ids.merchant_id.empty()) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INVALID_REQUEST,
                "missing merchant_id"
            );
            error.send(sink);
            return;
        }

        if (const auto auth_error =
                m_request_support.authorize(req, "list_initial_webhook_delivery_attempts", {})) {
            const auto& error = *auth_error;
            error.send(sink);
            return;
        }
        auto ctx = m_request_support.context(req);
        ctx.merchant_id = ids.merchant_id;

        using Dto = ::app::dto::webhook_events::EventListConstraints;
        auto parsed = core::behavior::Serializer<Dto>::pass(req, sink);
        if (!parsed) {
            return;
        }

        if (parsed->get_profile_id() && !parsed->get_profile_id()->get_value().empty()) {
            ctx.profile_id = parsed->get_profile_id()->get_value();
        }
        // Upstream validates constraints (foreign_try_from) before the profile lookup.
        if (const auto profile_error =
                m_input_support.validate_profile(ctx.profile_id, ctx.merchant_id)) {
            const auto& error = *profile_error;
            error.send(sink);
            return;
        }

        m_runtime.list_initial(*parsed, ctx).handle(sink);
    }

    // Upstream: GET /events/{merchant_id}/{initial_attempt_id}/attempts
    // routes/webhook_events.rs:105-140 -> core list_delivery_attempts.
    void handle_list_webhook_delivery_attempts(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};
        const auto ids = m_input_support.path_ids(req);
        if (ids.merchant_id.empty() || ids.event_id.empty()) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INVALID_REQUEST,
                "missing merchant_id or initial_attempt_id"
            );
            error.send(sink);
            return;
        }

        if (const auto auth_error =
                m_request_support.authorize(req, "list_webhook_delivery_attempts", {})) {
            const auto& error = *auth_error;
            error.send(sink);
            return;
        }
        auto ctx = m_request_support.context(req);
        ctx.merchant_id = ids.merchant_id;
        m_runtime.list_attempts(ctx).handle(sink);
    }

    // Upstream: POST /events/{merchant_id}/{event_id}/retry (v1 only)
    // routes/webhook_events.rs:142-178 -> core retry_delivery_attempt.
    void handle_retry_webhook_delivery_attempt(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};
        const auto ids = m_input_support.path_ids(req);
        if (ids.merchant_id.empty() || ids.event_id.empty()) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INVALID_REQUEST,
                "missing merchant_id or event_id"
            );
            error.send(sink);
            return;
        }

        if (const auto auth_error =
                m_request_support.authorize(req, "retry_webhook_delivery_attempt", {})) {
            const auto& error = *auth_error;
            error.send(sink);
            return;
        }
        auto ctx = m_request_support.context(req);
        ctx.merchant_id = ids.merchant_id;
        if (const auto profile_error =
                m_input_support.validate_profile(ctx.profile_id, ctx.merchant_id)) {
            const auto& error = *profile_error;
            error.send(sink);
            return;
        }

        m_runtime.retry(ctx).handle(sink);
    }

    void install_routes(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "events"}
            .add_router(
                Router{router, "profile"}.add_route(
                    Route{"list"}
                        .post(
                            [this](
                                interfaces::io::IRequest& req,
                                interfaces::io::IResponse& res,
                                std::function<void()> send
                            )
                            {
                                handle_list_initial_webhook_delivery_attempts_with_jwtauth(
                                    req,
                                    res,
                                    std::move(send)
                                );
                            }
                        )
                        .summary("Hyperswitch list_initial_webhook_delivery_attempts_with_jwtauth")
                )
            )
            .add_route(
                Route{":merchant_id"}
                    .post(
                        [this](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
                            handle_list_initial_webhook_delivery_attempts(
                                req,
                                res,
                                std::move(send)
                            );
                        }
                    )
                    .summary("Hyperswitch list_initial_webhook_delivery_attempts")
            )
            .add_router(
                Router{router, ":merchant_id"}.add_router(
                    Router{router, ":event_id"}.add_route(
                        Route{"attempts"}
                            .get(
                                [this](
                                    interfaces::io::IRequest& req,
                                    interfaces::io::IResponse& res,
                                    std::function<void()> send
                                )
                                {
                                    handle_list_webhook_delivery_attempts(
                                        req,
                                        res,
                                        std::move(send)
                                    );
                                }
                            )
                            .summary("Hyperswitch list_webhook_delivery_attempts")
                    )
                )
            )
            .add_router(
                Router{router, ":merchant_id"}.add_router(
                    Router{router, ":event_id"}.add_route(
                        Route{"retry"}
                            .post(
                                [this](
                                    interfaces::io::IRequest& req,
                                    interfaces::io::IResponse& res,
                                    std::function<void()> send
                                )
                                {
                                    handle_retry_webhook_delivery_attempt(
                                        req,
                                        res,
                                        std::move(send)
                                    );
                                }
                            )
                            .summary("Hyperswitch retry_webhook_delivery_attempt")
                    )
                )
            );
    }

    using RequestSupport =
        ::core::behavior::RequestSupport<::app::webhook::policy::WebhookEventsPolicy>;

    explicit WebhookEventsRouteHandlers(::app::context::AppContext& context) noexcept :
        m_runtime{context},
        m_input_support{context},
        m_policy{m_input_support},
        m_request_support{m_policy}
    {
    }

private:
    WebhookEventsRuntime m_runtime;
    WebhookEventsInputSupport m_input_support;
    ::app::webhook::policy::WebhookEventsPolicy m_policy;
    RequestSupport m_request_support;
};

} // namespace app::handlers::webhook_events
