export module app_payment:payments_handlers_v1_complete_redirect;

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

// v1 complete_redirect handlers. Owns its add_route registrations; the version facade calls
// install_route_NN in the original registration order.
class PaymentsV1CompleteRedirectHandlers
{
public:
    PaymentsV1CompleteRedirectHandlers(
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

    void handle_payments_complete_authorize_redirect(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto auth_error =
                m_input_support.authorize(req, "payments_complete_authorize_redirect")) {
            res.set_status(auth_error->get_status());
            res.set_body(serde::Ser::serialize("application/json", *auth_error));
            send();
            return;
        }
        m_context.metrics().record_flow("payments.payments_complete_authorize_redirect");
        m_context.metrics().counter("payments.payments_complete_authorize_redirect").increment();

        const auto body =
            m_input_support.path_body({{"connector", m_input_support.last_path_segment(req)}});
        if (body.empty()) {
            const auto err = m_input_support.api_error(
                core::error::AppErrorKind::EMPTY_REQUEST,
                "empty request"
            );
            err.send(res, send);
            return;
        }
        using Input = ::app::model::payments::PaymentsRedirectRequest;
        auto typed = serde::Ser::deserialize<Input>("application/json", body);
        auto generic = serde::Ser::decode_generic("application/json", body);
        if (!typed && !generic) {
            const auto err =
                m_input_support.api_error(core::error::AppErrorKind::INVALID_JSON, "invalid JSON");
            err.send(res, send);
            return;
        }
        PaymentsRequestData data = m_input_support.request_data(req);
        data.body = body;
        PaymentsCompleteAuthorizeRedirectInput input{std::move(data)};
        auto result = m_runtime.payments_complete_authorize_redirect(input);
        if (!result) {
            const auto err = m_input_support.api_error(result.error());
            err.send(res, send);
            return;
        }
        res.set_status(interfaces::io::types::Status::OK);
        res.set_body(serde::Ser::serialize("application/json", *result));
        send();
    }

    void handle_payments_complete_authorize_redirect_with_creds_identifier(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto auth_error = m_input_support.authorize(
                req,
                "payments_complete_authorize_redirect_with_creds_identifier"
            )) {
            res.set_status(auth_error->get_status());
            res.set_body(serde::Ser::serialize("application/json", *auth_error));
            send();
            return;
        }
        m_context.metrics().record_flow(
            "payments.payments_complete_authorize_redirect_with_creds_identifier"
        );
        m_context.metrics()
            .counter("payments.payments_complete_authorize_redirect_with_creds_identifier")
            .increment();

        const auto body = m_input_support.path_body(
            {{"connector", m_input_support.last_path_segment(req, 1)},
             {"creds_identifier", m_input_support.last_path_segment(req)}}
        );
        if (body.empty()) {
            const auto err = m_input_support.api_error(
                core::error::AppErrorKind::EMPTY_REQUEST,
                "empty request"
            );
            err.send(res, send);
            return;
        }
        using Input = ::app::model::payments::PaymentsRedirectRequest;
        auto typed = serde::Ser::deserialize<Input>("application/json", body);
        auto generic = serde::Ser::decode_generic("application/json", body);
        if (!typed && !generic) {
            const auto err =
                m_input_support.api_error(core::error::AppErrorKind::INVALID_JSON, "invalid JSON");
            err.send(res, send);
            return;
        }
        PaymentsRequestData data = m_input_support.request_data(req);
        data.body = body;
        PaymentsCompleteAuthorizeRedirectWithCredsIdentifierInput input{std::move(data)};
        auto result = m_runtime.payments_complete_authorize_redirect_with_creds_identifier(input);
        if (!result) {
            const auto err = m_input_support.api_error(result.error());
            err.send(res, send);
            return;
        }
        res.set_status(interfaces::io::types::Status::OK);
        res.set_body(serde::Ser::serialize("application/json", *result));
        send();
    }

    // original install() route #72
    void install_route_72(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "payments"}.add_router(
            Router{router, ":payment_id"}.add_router(
                Router{router, ":merchant_id"}.add_router(
                    Router{router, "redirect"}.add_router(
                        Router{router, "token"}.add_router(
                            Router{router, ":callback_token"}.add_router(
                                Router{router, "complete"}.add_route(
                                    Route{":connector"}
                                        .get(
                                            [this](
                                                interfaces::io::IRequest& req,
                                                interfaces::io::IResponse& res,
                                                std::function<void()> send
                                            )
                                            {
                                                handle_payments_complete_authorize_redirect(
                                                    req,
                                                    res,
                                                    std::move(send)
                                                );
                                            }
                                        )
                                        .summary("Hyperswitch payments_complete_authorize_redirect")
                                )
                            )
                        )
                    )
                )
            )
        );
    }

    // original install() route #73
    void install_route_73(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "payments"}.add_router(
            Router{router, ":payment_id"}.add_router(
                Router{router, ":merchant_id"}.add_router(
                    Router{router, "redirect"}.add_router(
                        Router{router, "token"}.add_router(
                            Router{router, ":callback_token"}.add_router(
                                Router{router, "complete"}.add_route(
                                    Route{":connector"}
                                        .post(
                                            [this](
                                                interfaces::io::IRequest& req,
                                                interfaces::io::IResponse& res,
                                                std::function<void()> send
                                            )
                                            {
                                                handle_payments_complete_authorize_redirect(
                                                    req,
                                                    res,
                                                    std::move(send)
                                                );
                                            }
                                        )
                                        .summary("Hyperswitch payments_complete_authorize_redirect")
                                )
                            )
                        )
                    )
                )
            )
        );
    }

    // original install() route #74
    void install_route_74(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "payments"}.add_router(
            Router{router, ":payment_id"}.add_router(
                Router{router, ":merchant_id"}.add_router(
                    Router{router, "redirect"}.add_router(
                        Router{router, "token"}.add_router(
                            Router{router, ":callback_token"}.add_router(
                                Router{router, "complete"}.add_router(
                                    Router{router, ":connector"}.add_route(
                                        Route{":creds_identifier"}
                                            .get(
                                                [this](
                                                    interfaces::io::IRequest& req,
                                                    interfaces::io::IResponse& res,
                                                    std::function<void()> send
                                                )
                                                {
                                                    handle_payments_complete_authorize_redirect_with_creds_identifier(
                                                        req,
                                                        res,
                                                        std::move(send)
                                                    );
                                                }
                                            )
                                            .summary(
                                                "Hyperswitch "
                                                "payments_complete_authorize_redirect_with_creds_"
                                                "identifier"
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

    // original install() route #75
    void install_route_75(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "payments"}.add_router(
            Router{router, ":payment_id"}.add_router(
                Router{router, ":merchant_id"}.add_router(
                    Router{router, "redirect"}.add_router(
                        Router{router, "token"}.add_router(
                            Router{router, ":callback_token"}.add_router(
                                Router{router, "complete"}.add_router(
                                    Router{router, ":connector"}.add_route(
                                        Route{":creds_identifier"}
                                            .post(
                                                [this](
                                                    interfaces::io::IRequest& req,
                                                    interfaces::io::IResponse& res,
                                                    std::function<void()> send
                                                )
                                                {
                                                    handle_payments_complete_authorize_redirect_with_creds_identifier(
                                                        req,
                                                        res,
                                                        std::move(send)
                                                    );
                                                }
                                            )
                                            .summary(
                                                "Hyperswitch "
                                                "payments_complete_authorize_redirect_with_creds_"
                                                "identifier"
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

    // original install() route #81
    void install_route_81(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "payments"}.add_router(
            Router{router, ":payment_id"}.add_router(
                Router{router, ":merchant_id"}.add_router(
                    Router{router, "redirect"}.add_router(
                        Router{router, "complete"}.add_route(
                            Route{":connector"}
                                .get(
                                    [this](
                                        interfaces::io::IRequest& req,
                                        interfaces::io::IResponse& res,
                                        std::function<void()> send
                                    )
                                    {
                                        handle_payments_complete_authorize_redirect(
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
                                        handle_payments_complete_authorize_redirect(
                                            req,
                                            res,
                                            std::move(send)
                                        );
                                    }
                                )
                                .summary("Hyperswitch payment_callback_redirect_complete (legacy)")
                        )
                    )
                )
            )
        );
    }

    // original install() route #82
    void install_route_82(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "payments"}.add_router(
            Router{router, ":payment_id"}.add_router(
                Router{router, ":merchant_id"}.add_router(
                    Router{router, "redirect"}.add_router(
                        Router{router, "complete"}.add_router(
                            Router{router, ":connector"}.add_route(
                                Route{":creds_identifier"}
                                    .get(
                                        [this](
                                            interfaces::io::IRequest& req,
                                            interfaces::io::IResponse& res,
                                            std::function<void()> send
                                        )
                                        {
                                            handle_payments_complete_authorize_redirect_with_creds_identifier(
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
                                            handle_payments_complete_authorize_redirect_with_creds_identifier(
                                                req,
                                                res,
                                                std::move(send)
                                            );
                                        }
                                    )
                                    .summary(
                                        "Hyperswitch "
                                        "payment_callback_redirect_complete_with_credentials "
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
        install_route_72(router);
        install_route_73(router);
        install_route_74(router);
        install_route_75(router);
        install_route_81(router);
        install_route_82(router);
    }

private:
    ::app::context::AppContext& m_context;
    PaymentsScopeState& m_scope;
    PaymentsInputSupport& m_input_support;
    PaymentsRuntime& m_runtime;
};

} // namespace app::handlers::payments
