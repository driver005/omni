export module app_payment:payments_handlers_v1_authorize;

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

// v1 authorize handlers. Owns its add_route registrations; the version facade calls
// install_route_NN in the original registration order.
class PaymentsV1AuthorizeHandlers
{
public:
    void bind_request_support(PaymentsRequestSupport& support) noexcept
    {
        m_request_support = &support;
    }

    PaymentsRequestSupport* m_request_support{nullptr};

    PaymentsV1AuthorizeHandlers(
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

    void handle_payments_complete_authorize(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};

        if (const auto auth_error = m_request_support->authorize(
                req,
                "payments_complete_authorize",
                spec_for("payments_complete_authorize")
            )) {
            auth_error->send(sink);
            return;
        }
        m_context.metrics().record_flow("payments.payments_complete_authorize");
        m_context.metrics().counter("payments.payments_complete_authorize").increment();

        auto parsed_dto = core::behavior::Serializer<::app::model::payments::PaymentsRequest>::pass(req, sink);
        if (!parsed_dto) return;
        const auto& dto = *parsed_dto;
        m_runtime.payments_complete_authorize(dto, m_request_support->context(req)).handle(sink);
    }

    void handle_payments_incremental_authorization(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};

        if (const auto auth_error = m_request_support->authorize(
                req,
                "payments_incremental_authorization",
                spec_for("payments_incremental_authorization")
            )) {
            auth_error->send(sink);
            return;
        }
        m_context.metrics().record_flow("payments.payments_incremental_authorization");
        m_context.metrics().counter("payments.payments_incremental_authorization").increment();

        auto parsed_dto = core::behavior::Serializer<::app::model::payments::PaymentsRequest>::pass(req, sink);
        if (!parsed_dto) return;
        const auto& dto = *parsed_dto;
        m_runtime.payments_incremental_authorization(dto, m_request_support->context(req)).handle(sink);
    }

    void handle_payments_extend_authorization(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};

        if (const auto auth_error = m_request_support->authorize(
                req,
                "payments_extend_authorization",
                spec_for("payments_extend_authorization")
            )) {
            auth_error->send(sink);
            return;
        }
        m_context.metrics().record_flow("payments.payments_extend_authorization");
        m_context.metrics().counter("payments.payments_extend_authorization").increment();

        const auto body =
            m_input_support.path_body({{"payment_id", m_input_support.payment_id_from_path(req)}});
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
        m_runtime.payments_extend_authorization(dto, m_request_support->context(req)).handle(sink);
    }

    void handle_payments_external_authentication(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};

        if (const auto auth_error = m_request_support->authorize(
                req,
                "payments_external_authentication",
                spec_for("payments_external_authentication")
            )) {
            auth_error->send(sink);
            return;
        }
        m_context.metrics().record_flow("payments.payments_external_authentication");
        m_context.metrics().counter("payments.payments_external_authentication").increment();

        auto parsed_dto = core::behavior::Serializer<::app::model::payments::PaymentsRequest>::pass(req, sink);
        if (!parsed_dto) return;
        const auto& dto = *parsed_dto;
        m_runtime.payments_external_authentication(dto, m_request_support->context(req)).handle(sink);
    }

    void handle_payments_submit_eligibility(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};

        if (const auto auth_error = m_request_support->authorize(
                req,
                "payments_submit_eligibility",
                spec_for("payments_submit_eligibility")
            )) {
            auth_error->send(sink);
            return;
        }
        m_context.metrics().record_flow("payments.payments_submit_eligibility");
        m_context.metrics().counter("payments.payments_submit_eligibility").increment();

        auto parsed_dto = core::behavior::Serializer<::app::model::payments::PaymentsRequest>::pass(req, sink);
        if (!parsed_dto) return;
        const auto& dto = *parsed_dto;
        m_runtime.payments_submit_eligibility(dto, m_request_support->context(req)).handle(sink);
    }

    void handle_payments_submit_eligibility_check(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};

        if (const auto auth_error = m_request_support->authorize(
                req,
                "payments_submit_eligibility_check",
                spec_for("payments_submit_eligibility_check")
            )) {
            auth_error->send(sink);
            return;
        }
        m_context.metrics().record_flow("payments.payments_submit_eligibility_check");
        m_context.metrics().counter("payments.payments_submit_eligibility_check").increment();

        auto parsed_dto = core::behavior::Serializer<::app::model::payments::PaymentsRequest>::pass(req, sink);
        if (!parsed_dto) return;
        const auto& dto = *parsed_dto;
        m_runtime.payments_submit_eligibility_check(dto, m_request_support->context(req)).handle(sink);
    }

    // original install() route #59
    void install_route_59(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "payments"}.add_router(
            Router{router, ":payment_id"}.add_route(
                Route{"eligibility_check"}
                    .post(
                        [this](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
        interfaces::io::ResponseSink sink{res, std::move(send)};

                            handle_payments_submit_eligibility_check(req, res, std::move(send));
                        }
                    )
                    .summary("Hyperswitch payments_submit_eligibility_check")
            )
        );
    }

    // original install() route #60
    void install_route_60(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "payments"}.add_router(
            Router{router, ":payment_id"}.add_route(
                Route{"eligibility"}
                    .post(
                        [this](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
        interfaces::io::ResponseSink sink{res, std::move(send)};

                            handle_payments_submit_eligibility(req, res, std::move(send));
                        }
                    )
                    .summary("Hyperswitch payments_submit_eligibility")
            )
        );
    }

    // original install() route #61
    void install_route_61(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "payments"}.add_router(
            Router{router, ":payment_id"}.add_router(
                Router{router, "3ds"}.add_route(
                    Route{"authentication"}
                        .post(
                            [this](
                                interfaces::io::IRequest& req,
                                interfaces::io::IResponse& res,
                                std::function<void()> send
                            )
                            {
        interfaces::io::ResponseSink sink{res, std::move(send)};

                                handle_payments_external_authentication(req, res, std::move(send));
                            }
                        )
                        .summary("Hyperswitch payments_external_authentication")
                )
            )
        );
    }

    // original install() route #62
    void install_route_62(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "payments"}.add_router(
            Router{router, ":payment_id"}.add_route(
                Route{"complete_authorize"}
                    .post(
                        [this](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
        interfaces::io::ResponseSink sink{res, std::move(send)};

                            handle_payments_complete_authorize(req, res, std::move(send));
                        }
                    )
                    .summary("Hyperswitch payments_complete_authorize")
            )
        );
    }

    // original install() route #63
    void install_route_63(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "payments"}.add_router(
            Router{router, ":payment_id"}.add_route(
                Route{"incremental_authorization"}
                    .post(
                        [this](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
        interfaces::io::ResponseSink sink{res, std::move(send)};

                            handle_payments_incremental_authorization(req, res, std::move(send));
                        }
                    )
                    .summary("Hyperswitch payments_incremental_authorization")
            )
        );
    }

    // original install() route #64
    void install_route_64(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "payments"}.add_router(
            Router{router, ":payment_id"}.add_route(
                Route{"extend_authorization"}
                    .post(
                        [this](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
        interfaces::io::ResponseSink sink{res, std::move(send)};

                            handle_payments_extend_authorization(req, res, std::move(send));
                        }
                    )
                    .summary("Hyperswitch payments_extend_authorization")
            )
        );
    }

    void install(core::router::RouterContext<>& router)
    {
        install_route_59(router);
        install_route_60(router);
        install_route_61(router);
        install_route_62(router);
        install_route_63(router);
        install_route_64(router);
    }

private:
    ::app::context::AppContext& m_context;
    PaymentsScopeState& m_scope;
    PaymentsInputSupport& m_input_support;
    PaymentsRuntime& m_runtime;
};

} // namespace app::handlers::payments
