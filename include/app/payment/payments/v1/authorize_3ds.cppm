export module app_payment:payments_handlers_v1_authorize_3ds;

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

// v1 authorize_3ds handlers. Owns its add_route registrations; the version facade calls
// install_route_NN in the original registration order.
class PaymentsV1Authorize3dsHandlers
{
public:
    void bind_request_support(PaymentsRequestSupport& support) noexcept
    {
        m_request_support = &support;
    }

    PaymentsRequestSupport* m_request_support{nullptr};

    PaymentsV1Authorize3dsHandlers(
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

    void handle_post_3ds_payments_authorize(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};

        if (const auto auth_error = m_request_support->authorize(
                req,
                "post_3ds_payments_authorize",
                spec_for("post_3ds_payments_authorize")
            )) {
            auth_error->send(sink);
            return;
        }
        m_context.metrics().record_flow("payments.post_3ds_payments_authorize");
        m_context.metrics().counter("payments.post_3ds_payments_authorize").increment();

        const auto body =
            m_input_support.path_body({{"connector", m_input_support.last_path_segment(req)}});
        if (body.empty()) {
            const auto err = m_input_support.api_error(
                core::error::AppErrorKind::EMPTY_REQUEST,
                "empty request"
            );
            err.send(sink);
            return;
        }
        auto parsed_dto = core::behavior::Serializer<::app::model::payments::PaymentsRequest>::pass(req, sink);
        if (!parsed_dto) return;
        const auto& dto = *parsed_dto;
        m_runtime.post_3ds_payments_authorize(dto, m_request_support->context(req)).handle(sink);
    }

    // original install() route #76
    void install_route_76(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "payments"}.add_router(
            Router{router, ":payment_id"}.add_router(
                Router{router, ":merchant_id"}.add_router(
                    Router{router, "redirect"}.add_router(
                        Router{router, "token"}.add_router(
                            Router{router, ":callback_token"}.add_router(
                                Router{router, "authorize"}.add_route(
                                    Route{":connector"}
                                        .get(
                                            [this](
                                                interfaces::io::IRequest& req,
                                                interfaces::io::IResponse& res,
                                                std::function<void()> send
                                            )
                                            {
        interfaces::io::ResponseSink sink{res, std::move(send)};

                                                handle_post_3ds_payments_authorize(
                                                    req,
                                                    res,
                                                    std::move(send)
                                                );
                                            }
                                        )
                                        .summary("Hyperswitch post_3ds_payments_authorize")
                                )
                            )
                        )
                    )
                )
            )
        );
    }

    // original install() route #77
    void install_route_77(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "payments"}.add_router(
            Router{router, ":payment_id"}.add_router(
                Router{router, ":merchant_id"}.add_router(
                    Router{router, "redirect"}.add_router(
                        Router{router, "token"}.add_router(
                            Router{router, ":callback_token"}.add_router(
                                Router{router, "authorize"}.add_route(
                                    Route{":connector"}
                                        .post(
                                            [this](
                                                interfaces::io::IRequest& req,
                                                interfaces::io::IResponse& res,
                                                std::function<void()> send
                                            )
                                            {
        interfaces::io::ResponseSink sink{res, std::move(send)};

                                                handle_post_3ds_payments_authorize(
                                                    req,
                                                    res,
                                                    std::move(send)
                                                );
                                            }
                                        )
                                        .summary("Hyperswitch post_3ds_payments_authorize")
                                )
                            )
                        )
                    )
                )
            )
        );
    }

    // original install() route #83
    void install_route_83(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "payments"}.add_router(
            Router{router, ":payment_id"}.add_router(
                Router{router, ":merchant_id"}.add_router(
                    Router{router, "authorize"}.add_route(
                        Route{":connector"}
                            .get(
                                [this](
                                    interfaces::io::IRequest& req,
                                    interfaces::io::IResponse& res,
                                    std::function<void()> send
                                )
                                {
        interfaces::io::ResponseSink sink{res, std::move(send)};

                                    handle_post_3ds_payments_authorize(req, res, std::move(send));
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

                                    handle_post_3ds_payments_authorize(req, res, std::move(send));
                                }
                            )
                            .summary("Hyperswitch payment_callback_authorize (legacy)")
                    )
                )
            )
        );
    }

    void install(core::router::RouterContext<>& router)
    {
        install_route_76(router);
        install_route_77(router);
        install_route_83(router);
    }

private:
    ::app::context::AppContext& m_context;
    PaymentsScopeState& m_scope;
    PaymentsInputSupport& m_input_support;
    PaymentsRuntime& m_runtime;
};

} // namespace app::handlers::payments
