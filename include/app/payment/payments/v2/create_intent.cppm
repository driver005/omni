export module app_payment:payments_handlers_v2_create_intent;

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

// v2 create_intent handlers. Owns its add_route registrations; the version facade calls
// install_route_NN in the original registration order.
class PaymentsV2CreateIntentHandlers
{
public:
    void bind_request_support(PaymentsRequestSupport& support) noexcept
    {
        m_request_support = &support;
    }

    PaymentsRequestSupport* m_request_support{nullptr};

    PaymentsV2CreateIntentHandlers(
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

    void handle_payments_create_intent(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};

        if (const auto auth_error = m_request_support->authorize(
                req,
                "payments_create_intent",
                spec_for("payments_create_intent")
            )) {
            auth_error->send(sink);
            return;
        }
        m_context.logger().record_flow("payments.payments_create_intent");
        m_context.logger().counter("payments.payments_create_intent").increment();

        auto parsed_dto =
            core::behavior::Serializer<::app::model::payments::PaymentsRequest>::pass(req, sink);
        if (!parsed_dto) {
            return;
        }
        const auto& dto = *parsed_dto;
        m_runtime.payments_create_intent(dto, m_request_support->context(req)).handle(sink);
    }

    void handle_payments_create_and_confirm_intent(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};

        if (const auto auth_error = m_request_support->authorize(
                req,
                "payments_create_and_confirm_intent",
                spec_for("payments_create_and_confirm_intent")
            )) {
            auth_error->send(sink);
            return;
        }
        m_context.logger().record_flow("payments.payments_create_and_confirm_intent");
        m_context.logger().counter("payments.payments_create_and_confirm_intent").increment();

        auto parsed_dto =
            core::behavior::Serializer<::app::model::payments::PaymentsRequest>::pass(req, sink);
        if (!parsed_dto) {
            return;
        }
        const auto& dto = *parsed_dto;
        m_runtime.payments_create_and_confirm_intent(dto, m_request_support->context(req))
            .handle(sink);
    }

    void handle_payments_update_intent(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};

        if (const auto auth_error = m_request_support->authorize(
                req,
                "payments_update_intent",
                spec_for("payments_update_intent")
            )) {
            auth_error->send(sink);
            return;
        }
        m_context.logger().record_flow("payments.payments_update_intent");
        m_context.logger().counter("payments.payments_update_intent").increment();

        auto parsed_dto =
            core::behavior::Serializer<::app::model::payments::PaymentsRequest>::pass(req, sink);
        if (!parsed_dto) {
            return;
        }
        const auto& dto = *parsed_dto;
        m_runtime.payments_update_intent(dto, m_request_support->context(req)).handle(sink);
    }

    // original install() route #0
    void install_route_00(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "v2/payments"}.add_route(
            Route{"create-intent"}
                .post(
                    [this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    )
                    {
                        interfaces::io::ResponseSink sink{res, std::move(send)};

                        handle_payments_create_intent(req, res, std::move(send));
                    }
                )
                .summary("Hyperswitch payments_create_intent")
        );
    }

    // original install() route #3
    void install_route_03(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "v2/payments"}.add_route(
            Route{""}
                .post(
                    [this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    )
                    {
                        interfaces::io::ResponseSink sink{res, std::move(send)};

                        handle_payments_create_and_confirm_intent(req, res, std::move(send));
                    }
                )
                .summary("Hyperswitch payments_create_and_confirm_intent")
        );
    }

    // original install() route #18
    void install_route_18(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "v2/payments"}.add_router(
            Router{router, ":payment_id"}.add_route(
                Route{"update-intent"}
                    .put(
                        [this](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
                            interfaces::io::ResponseSink sink{res, std::move(send)};

                            handle_payments_update_intent(req, res, std::move(send));
                        }
                    )
                    .summary("Hyperswitch payments_update_intent")
            )
        );
    }

    void install(core::router::RouterContext<>& router)
    {
        install_route_00(router);
        install_route_03(router);
        install_route_18(router);
    }

private:
    ::app::context::AppContext& m_context;
    PaymentsScopeState& m_scope;
    PaymentsInputSupport& m_input_support;
    PaymentsRuntime& m_runtime;
};

} // namespace app::handlers::payments
