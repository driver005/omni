export module app_payment:payments_handlers_v1_redirects;

import std;
import core_behavior;
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
import app_store;
import app_connector;
import app_http;
import app_crypto;
import app_metrics;
import app_locking;
import app_events;
import app_workflow;
import app_services;

import :payments_input;
import :payments_result;
import :payments_runtime;

export namespace app::handlers::payments {

// v1 redirects handlers. Owns its add_route registrations; the version facade calls
// install_route_NN in the original registration order.
class PaymentsV1RedirectsHandlers
{
public:
    void bind_request_support(PaymentsRequestSupport& support) noexcept
    {
        m_request_support = &support;
    }

    PaymentsRequestSupport* m_request_support{nullptr};

    PaymentsV1RedirectsHandlers(
        ::app::context::AppContext& ctx,
        PaymentsScopeState& scope,
        PaymentsInputSupport& input_support,
        PaymentsRuntime& runtime
    ) noexcept :
        m_context{ctx},
        m_scope{scope},
        m_input_support{input_support},
        m_runtime{runtime}
    {
    }

    void handle_payments_start(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};

        if (const auto auth_error =
                m_request_support->authorize(req, "payments_start", spec_for("payments_start"))) {
            auth_error->send(sink);
            return;
        }
        m_context.logger().record_flow("payments.payments_start");
        m_context.logger().counter("payments.payments_start").increment();

        const auto body = m_input_support.path_body(
            {{"payment_id", m_input_support.payment_id_from_path(req)},
             {"merchant_id", m_input_support.merchant_id_from_path(req)},
             {"attempt_id", m_input_support.last_path_segment(req)}}
        );
        if (body.empty()) {
            const auto err = m_input_support.api_error(
                core::error::AppErrorKind::EMPTY_REQUEST,
                "empty request"
            );
            err.send(sink);
            return;
        }
        auto parsed_dto =
            core::behavior::Serializer<::app::model::payments::PaymentsRequest>::pass(req, sink);
        if (!parsed_dto) {
            return;
        }
        const auto& dto = *parsed_dto;
        m_runtime.payments_start(dto, m_request_support->context(req)).handle(sink);
    }

    void handle_payments_redirect_response(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};

        if (const auto auth_error = m_request_support->authorize(
                req,
                "payments_redirect_response",
                spec_for("payments_redirect_response")
            )) {
            auth_error->send(sink);
            return;
        }
        m_context.logger().record_flow("payments.payments_redirect_response");
        m_context.logger().counter("payments.payments_redirect_response").increment();

        const auto body = m_input_support.path_body(
            {{"payment_id", m_input_support.payment_id_from_path(req)},
             {"connector", m_input_support.last_path_segment(req)}}
        );
        if (body.empty()) {
            const auto err = m_input_support.api_error(
                core::error::AppErrorKind::EMPTY_REQUEST,
                "empty request"
            );
            err.send(sink);
            return;
        }
        auto parsed_dto =
            core::behavior::Serializer<::app::model::payments::PaymentsRequest>::pass(req, sink);
        if (!parsed_dto) {
            return;
        }
        const auto& dto = *parsed_dto;
        m_runtime.payments_redirect_response(dto, m_request_support->context(req)).handle(sink);
    }

    void handle_payments_redirect_response_with_creds_identifier(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};

        if (const auto auth_error = m_request_support->authorize(
                req,
                "payments_redirect_response_with_creds_identifier",
                spec_for("payments_redirect_response_with_creds_identifier")
            )) {
            auth_error->send(sink);
            return;
        }
        m_context.logger().record_flow("payments.payments_redirect_response_with_creds_identifier");
        m_context.logger()
            .counter("payments.payments_redirect_response_with_creds_identifier")
            .increment();

        const auto body = m_input_support.path_body(
            {{"payment_id", m_input_support.payment_id_from_path(req)},
             {"connector", m_input_support.last_path_segment(req, 1)},
             {"creds_identifier", m_input_support.last_path_segment(req)}}
        );
        if (body.empty()) {
            const auto err = m_input_support.api_error(
                core::error::AppErrorKind::EMPTY_REQUEST,
                "empty request"
            );
            err.send(sink);
            return;
        }
        auto parsed_dto =
            core::behavior::Serializer<::app::model::payments::PaymentsRequest>::pass(req, sink);
        if (!parsed_dto) {
            return;
        }
        const auto& dto = *parsed_dto;
        m_runtime
            .payments_redirect_response_with_creds_identifier(dto, m_request_support->context(req))
            .handle(sink);
    }

    // original install() route #68
    void install_route_68(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "payments"}.add_router(
            Router{router, ":payment_id"}.add_router(
                Router{router, ":merchant_id"}.add_router(
                    Router{router, "redirect"}.add_router(
                        Router{router, "token"}.add_router(
                            Router{router, ":callback_token"}.add_router(
                                Router{router, "start"}.add_route(
                                    Route{":attempt_id"}
                                        .get(
                                            [this](
                                                interfaces::io::IRequest& req,
                                                interfaces::io::IResponse& res,
                                                std::function<void()> send
                                            )
                                            {
                                                interfaces::io::ResponseSink sink{
                                                    res,
                                                    std::move(send)
                                                };

                                                handle_payments_start(req, res, std::move(send));
                                            }
                                        )
                                        .summary("Hyperswitch payments_start")
                                )
                            )
                        )
                    )
                )
            )
        );
    }

    // original install() route #69
    void install_route_69(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "payments"}.add_router(
            Router{router, ":payment_id"}.add_router(
                Router{router, ":merchant_id"}.add_router(
                    Router{router, "redirect"}.add_router(
                        Router{router, "token"}.add_router(
                            Router{router, ":callback_token"}.add_router(
                                Router{router, "response"}.add_route(
                                    Route{":connector"}
                                        .get(
                                            [this](
                                                interfaces::io::IRequest& req,
                                                interfaces::io::IResponse& res,
                                                std::function<void()> send
                                            )
                                            {
                                                interfaces::io::ResponseSink sink{
                                                    res,
                                                    std::move(send)
                                                };

                                                handle_payments_redirect_response(
                                                    req,
                                                    res,
                                                    std::move(send)
                                                );
                                            }
                                        )
                                        .summary("Hyperswitch payments_redirect_response")
                                )
                            )
                        )
                    )
                )
            )
        );
    }

    // original install() route #70
    void install_route_70(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "payments"}.add_router(
            Router{router, ":payment_id"}.add_router(
                Router{router, ":merchant_id"}.add_router(
                    Router{router, "redirect"}.add_router(
                        Router{router, "token"}.add_router(
                            Router{router, ":callback_token"}.add_router(
                                Router{router, "response"}.add_route(
                                    Route{":connector"}
                                        .post(
                                            [this](
                                                interfaces::io::IRequest& req,
                                                interfaces::io::IResponse& res,
                                                std::function<void()> send
                                            )
                                            {
                                                interfaces::io::ResponseSink sink{
                                                    res,
                                                    std::move(send)
                                                };

                                                handle_payments_redirect_response(
                                                    req,
                                                    res,
                                                    std::move(send)
                                                );
                                            }
                                        )
                                        .summary("Hyperswitch payments_redirect_response")
                                )
                            )
                        )
                    )
                )
            )
        );
    }

    // original install() route #71
    void install_route_71(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "payments"}.add_router(
            Router{router, ":payment_id"}.add_router(
                Router{router, ":merchant_id"}.add_router(
                    Router{router, "redirect"}.add_router(
                        Router{router, "token"}.add_router(
                            Router{router, ":callback_token"}.add_router(
                                Router{router, "response"}.add_router(
                                    Router{router, ":connector"}.add_route(
                                        Route{":creds_identifier"}
                                            .get(
                                                [this](
                                                    interfaces::io::IRequest& req,
                                                    interfaces::io::IResponse& res,
                                                    std::function<void()> send
                                                )
                                                {
                                                    interfaces::io::ResponseSink sink{
                                                        res,
                                                        std::move(send)
                                                    };

                                                    handle_payments_redirect_response_with_creds_identifier(
                                                        req,
                                                        res,
                                                        std::move(send)
                                                    );
                                                }
                                            )
                                            .summary(
                                                "Hyperswitch "
                                                "payments_redirect_response_with_creds_identifier"
                                            )
                                    )
                                )
                            )
                        )
                    )
                )
            )
        );
    }

    // original install() route #78
    // Legacy callback resources (upstream app.rs:1196-1201);
    // callback_routes.rs:76-121,181-209). Same handlers, original tokenless
    // paths, kept for the compatibility rollout.
    void install_route_78(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "payments"}.add_router(
            Router{router, "redirect"}.add_router(
                Router{router, ":payment_id"}.add_router(
                    Router{router, ":merchant_id"}.add_route(
                        Route{":attempt_id"}
                            .get(
                                [this](
                                    interfaces::io::IRequest& req,
                                    interfaces::io::IResponse& res,
                                    std::function<void()> send
                                )
                                {
                                    interfaces::io::ResponseSink sink{res, std::move(send)};

                                    handle_payments_start(req, res, std::move(send));
                                }
                            )
                            .summary("Hyperswitch payment_callback_start (legacy)")
                    )
                )
            )
        );
    }

    // original install() route #79
    void install_route_79(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "payments"}.add_router(
            Router{router, ":payment_id"}.add_router(
                Router{router, ":merchant_id"}.add_router(
                    Router{router, "redirect"}.add_router(
                        Router{router, "response"}.add_route(
                            Route{":connector"}
                                .get(
                                    [this](
                                        interfaces::io::IRequest& req,
                                        interfaces::io::IResponse& res,
                                        std::function<void()> send
                                    )
                                    {
                                        interfaces::io::ResponseSink sink{res, std::move(send)};

                                        handle_payments_redirect_response(
                                            req,
                                            res,
                                            std::move(send)
                                        );
                                    }
                                )
                                .post(
                                    [this](
                                        interfaces::io::IRequest& req,
                                        interfaces::io::IResponse& res,
                                        std::function<void()> send
                                    )
                                    {
                                        interfaces::io::ResponseSink sink{res, std::move(send)};

                                        handle_payments_redirect_response(
                                            req,
                                            res,
                                            std::move(send)
                                        );
                                    }
                                )
                                .summary("Hyperswitch payment_callback_redirect_response (legacy)")
                        )
                    )
                )
            )
        );
    }

    // original install() route #80
    void install_route_80(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "payments"}.add_router(
            Router{router, ":payment_id"}.add_router(
                Router{router, ":merchant_id"}.add_router(
                    Router{router, "redirect"}.add_router(
                        Router{router, "response"}.add_router(
                            Router{router, ":connector"}.add_route(
                                Route{":creds_identifier"}
                                    .get(
                                        [this](
                                            interfaces::io::IRequest& req,
                                            interfaces::io::IResponse& res,
                                            std::function<void()> send
                                        )
                                        {
                                            interfaces::io::ResponseSink sink{res, std::move(send)};

                                            handle_payments_redirect_response_with_creds_identifier(
                                                req,
                                                res,
                                                std::move(send)
                                            );
                                        }
                                    )
                                    .summary(
                                        "Hyperswitch "
                                        "payment_callback_redirect_response_with_credentials "
                                        "(legacy)"
                                    )
                            )
                        )
                    )
                )
            )
        );
    }

    void install(core::router::RouterContext<>& router)
    {
        install_route_68(router);
        install_route_69(router);
        install_route_70(router);
        install_route_71(router);
        install_route_78(router);
        install_route_79(router);
        install_route_80(router);
    }

private:
    ::app::context::AppContext& m_context;
    PaymentsScopeState& m_scope;
    PaymentsInputSupport& m_input_support;
    PaymentsRuntime& m_runtime;
};

} // namespace app::handlers::payments
