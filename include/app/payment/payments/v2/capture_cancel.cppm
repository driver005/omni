export module app_payment:payments_handlers_v2_capture_cancel;

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

// v2 capture_cancel handlers. Owns its add_route registrations; the version facade calls
// install_route_NN in the original registration order.
class PaymentsV2CaptureCancelHandlers
{
public:
    void bind_request_support(PaymentsRequestSupport& support) noexcept
    {
        m_request_support = &support;
    }

    PaymentsRequestSupport* m_request_support{nullptr};

    PaymentsV2CaptureCancelHandlers(
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

    void handle_payments_capture(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};

        if (const auto auth_error =
                m_request_support
                    ->authorize(req, "payments_capture", spec_for("payments_capture"))) {
            auth_error->send(sink);
            return;
        }
        m_context.logger().record_flow("payments.payments_capture");
        m_context.logger().counter("payments.payments_capture").increment();

        auto parsed_dto =
            core::behavior::Serializer<::app::model::payments::PaymentsRequest>::pass(req, sink);
        if (!parsed_dto) {
            return;
        }
        const auto& dto = *parsed_dto;
        m_runtime.payments_capture(dto, m_request_support->context(req)).handle(sink);
    }

    void handle_payments_cancel(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};

        if (const auto auth_error =
                m_request_support->authorize(req, "payments_cancel", spec_for("payments_cancel"))) {
            auth_error->send(sink);
            return;
        }
        m_context.logger().record_flow("payments.payments_cancel");
        m_context.logger().counter("payments.payments_cancel").increment();

        auto parsed_dto =
            core::behavior::Serializer<::app::model::payments::PaymentsRequest>::pass(req, sink);
        if (!parsed_dto) {
            return;
        }
        const auto& dto = *parsed_dto;
        m_runtime.payments_cancel(dto, m_request_support->context(req)).handle(sink);
    }

    // original install() route #26
    void install_route_26(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "v2/payments"}.add_router(
            Router{router, ":payment_id"}.add_route(
                Route{"capture"}
                    .post(
                        [this](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
                            interfaces::io::ResponseSink sink{res, std::move(send)};

                            handle_payments_capture(req, res, std::move(send));
                        }
                    )
                    .summary("Hyperswitch payments_capture")
            )
        );
    }

    // original install() route #27
    void install_route_27(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "v2/payments"}.add_router(
            Router{router, ":payment_id"}.add_route(
                Route{"cancel"}
                    .post(
                        [this](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
                            interfaces::io::ResponseSink sink{res, std::move(send)};

                            handle_payments_cancel(req, res, std::move(send));
                        }
                    )
                    .summary("Hyperswitch payments_cancel")
            )
        );
    }

    void install(core::router::RouterContext<>& router)
    {
        install_route_26(router);
        install_route_27(router);
    }

private:
    ::app::context::AppContext& m_context;
    PaymentsScopeState& m_scope;
    PaymentsInputSupport& m_input_support;
    PaymentsRuntime& m_runtime;
};

} // namespace app::handlers::payments
