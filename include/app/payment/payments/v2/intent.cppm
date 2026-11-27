export module app_payment:payments_handlers_v2_intent;

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

// v2 intent handlers. Owns its add_route registrations; the version facade calls
// install_route_NN in the original registration order.
class PaymentsV2IntentHandlers
{
public:
    void bind_request_support(PaymentsRequestSupport& support) noexcept
    {
        m_request_support = &support;
    }

    PaymentsRequestSupport* m_request_support{nullptr};

    PaymentsV2IntentHandlers(
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

    void handle_payment_get_intent_using_merchant_reference_id(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};

        if (const auto auth_error = m_request_support->authorize(
                req,
                "payment_get_intent_using_merchant_reference_id",
                spec_for("payment_get_intent_using_merchant_reference_id")
            )) {
            auth_error->send(sink);
            return;
        }
        m_context.metrics().record_flow("payments.payment_get_intent_using_merchant_reference_id");
        m_context.metrics()
            .counter("payments.payment_get_intent_using_merchant_reference_id")
            .increment();

        const auto body = m_input_support.path_body(
            {{"merchant_reference_id", m_input_support.last_path_segment(req)}}
        );
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
        m_runtime.payment_get_intent_using_merchant_reference_id(dto, m_request_support->context(req)).handle(sink);
    }

    void handle_payments_get_intent(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};

        if (const auto auth_error =
                m_request_support
                    ->authorize(req, "payments_get_intent", spec_for("payments_get_intent"))) {
            auth_error->send(sink);
            return;
        }
        m_context.metrics().record_flow("payments.payments_get_intent");
        m_context.metrics().counter("payments.payments_get_intent").increment();

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
        m_runtime.payments_get_intent(dto, m_request_support->context(req)).handle(sink);
    }

    void handle_revenue_recovery_get_intent(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};

        if (const auto auth_error = m_request_support->authorize(
                req,
                "revenue_recovery_get_intent",
                spec_for("revenue_recovery_get_intent")
            )) {
            auth_error->send(sink);
            return;
        }
        m_context.metrics().record_flow("payments.revenue_recovery_get_intent");
        m_context.metrics().counter("payments.revenue_recovery_get_intent").increment();

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
        m_runtime.revenue_recovery_get_intent(dto, m_request_support->context(req)).handle(sink);
    }

    void handle_list_payment_attempts(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};

        if (const auto auth_error =
                m_request_support
                    ->authorize(req, "list_payment_attempts", spec_for("list_payment_attempts"))) {
            auth_error->send(sink);
            return;
        }
        m_context.metrics().record_flow("payments.list_payment_attempts");
        m_context.metrics().counter("payments.list_payment_attempts").increment();

        const ::app::model::payments::PaymentsRequest dto{};
        m_runtime.list_payment_attempts(dto, m_request_support->context(req)).handle(sink);
    }

    void handle_list_payment_methods(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};

        if (const auto auth_error =
                m_request_support
                    ->authorize(req, "list_payment_methods", spec_for("list_payment_methods"))) {
            auth_error->send(sink);
            return;
        }
        m_context.metrics().record_flow("payments.list_payment_methods");
        m_context.metrics().counter("payments.list_payment_methods").increment();

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
        m_runtime.list_payment_methods(dto, m_request_support->context(req)).handle(sink);
    }

    // original install() route #10
    void install_route_10(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "v2/payments"}.add_router(
            Router{router, "ref"}.add_route(
                Route{":merchant_reference_id"}
                    .get(
                        [this](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
        interfaces::io::ResponseSink sink{res, std::move(send)};

                            handle_payment_get_intent_using_merchant_reference_id(
                                req,
                                res,
                                std::move(send)
                            );
                        }
                    )
                    .summary("Hyperswitch payment_get_intent_using_merchant_reference_id")
            )
        );
    }

    // original install() route #12
    void install_route_12(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "v2/payments"}.add_router(
            Router{router, ":payment_id"}.add_route(
                Route{"list-attempts"}
                    .get(
                        [this](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
        interfaces::io::ResponseSink sink{res, std::move(send)};

                            handle_list_payment_attempts(req, res, std::move(send));
                        }
                    )
                    .summary("Hyperswitch list_payment_attempts")
            )
        );
    }

    // original install() route #13
    void install_route_13(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "v2/payments"}.add_router(
            Router{router, ":payment_id"}.add_route(
                Route{"list_attempts"}
                    .get(
                        [this](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
        interfaces::io::ResponseSink sink{res, std::move(send)};

                            handle_list_payment_attempts(req, res, std::move(send));
                        }
                    )
                    .summary("Hyperswitch list_payment_attempts (deprecated alias)")
            )
        );
    }

    // original install() route #16
    void install_route_16(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "v2/payments"}.add_router(
            Router{router, ":payment_id"}.add_route(
                Route{"get-intent"}
                    .get(
                        [this](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
        interfaces::io::ResponseSink sink{res, std::move(send)};

                            handle_payments_get_intent(req, res, std::move(send));
                        }
                    )
                    .summary("Hyperswitch payments_get_intent")
            )
        );
    }

    // original install() route #17
    void install_route_17(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "v2/payments"}.add_router(
            Router{router, ":payment_id"}.add_route(
                Route{"get-revenue-recovery-intent"}
                    .get(
                        [this](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
        interfaces::io::ResponseSink sink{res, std::move(send)};

                            handle_revenue_recovery_get_intent(req, res, std::move(send));
                        }
                    )
                    .summary("Hyperswitch revenue_recovery_get_intent")
            )
        );
    }

    // original install() route #23
    void install_route_23(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "v2/payments"}.add_router(
            Router{router, ":payment_id"}.add_route(
                Route{"payment-methods"}
                    .get(
                        [this](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
        interfaces::io::ResponseSink sink{res, std::move(send)};

                            handle_list_payment_methods(req, res, std::move(send));
                        }
                    )
                    .summary("Hyperswitch list_payment_methods")
            )
        );
    }

    void install(core::router::RouterContext<>& router)
    {
        install_route_10(router);
        install_route_12(router);
        install_route_13(router);
        install_route_16(router);
        install_route_17(router);
        install_route_23(router);
    }

private:
    ::app::context::AppContext& m_context;
    PaymentsScopeState& m_scope;
    PaymentsInputSupport& m_input_support;
    PaymentsRuntime& m_runtime;
};

} // namespace app::handlers::payments
