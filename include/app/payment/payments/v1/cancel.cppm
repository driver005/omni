export module app_payment:payments_handlers_v1_cancel;

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

// v1 cancel handlers. Owns its add_route registrations; the version facade calls
// install_route_NN in the original registration order.
class PaymentsV1CancelHandlers
{
public:
    void bind_request_support(PaymentsRequestSupport& support) noexcept
    {
        m_request_support = &support;
    }

    PaymentsRequestSupport* m_request_support{nullptr};

    PaymentsV1CancelHandlers(
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
        m_context.metrics().record_flow("payments.payments_cancel");
        m_context.metrics().counter("payments.payments_cancel").increment();

        auto parsed_dto = core::behavior::Serializer<::app::model::payments::PaymentsRequest>::pass(req, sink);
        if (!parsed_dto) return;
        const auto& dto = *parsed_dto;
        m_runtime.payments_cancel(dto, m_request_support->context(req)).handle(sink);
    }

    void handle_payments_cancel_post_capture(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};

        if (const auto auth_error = m_request_support->authorize(
                req,
                "payments_cancel_post_capture",
                spec_for("payments_cancel_post_capture")
            )) {
            auth_error->send(sink);
            return;
        }
        m_context.metrics().record_flow("payments.payments_cancel_post_capture");
        m_context.metrics().counter("payments.payments_cancel_post_capture").increment();

        auto parsed_dto = core::behavior::Serializer<::app::model::payments::PaymentsRequest>::pass(req, sink);
        if (!parsed_dto) return;
        const auto& dto = *parsed_dto;
        m_runtime.payments_cancel_post_capture(dto, m_request_support->context(req)).handle(sink);
    }

    void handle_payments_cancel_post_capture_retrieve(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};

        if (const auto auth_error = m_request_support->authorize(
                req,
                "payments_cancel_post_capture_retrieve",
                spec_for("payments_cancel_post_capture_retrieve")
            )) {
            auth_error->send(sink);
            return;
        }
        m_context.metrics().record_flow("payments.payments_cancel_post_capture_retrieve");
        m_context.metrics().counter("payments.payments_cancel_post_capture_retrieve").increment();

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
        m_runtime.payments_cancel_post_capture_retrieve(dto, m_request_support->context(req)).handle(sink);
    }

    // original install() route #53
    void install_route_53(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "payments"}.add_router(
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

    // original install() route #54
    void install_route_54(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "payments"}.add_router(
            Router{router, ":payment_id"}.add_route(
                Route{"cancel_post_capture"}
                    .post(
                        [this](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
        interfaces::io::ResponseSink sink{res, std::move(send)};

                            handle_payments_cancel_post_capture(req, res, std::move(send));
                        }
                    )
                    .summary("Hyperswitch payments_cancel_post_capture")
            )
        );
    }

    // original install() route #55
    void install_route_55(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "payments"}.add_router(
            Router{router, ":payment_id"}.add_route(
                Route{"cancel_post_capture"}
                    .get(
                        [this](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
        interfaces::io::ResponseSink sink{res, std::move(send)};

                            handle_payments_cancel_post_capture_retrieve(req, res, std::move(send));
                        }
                    )
                    .summary("Hyperswitch payments_cancel_post_capture_retrieve")
            )
        );
    }

    void install(core::router::RouterContext<>& router)
    {
        install_route_53(router);
        install_route_54(router);
        install_route_55(router);
    }

private:
    ::app::context::AppContext& m_context;
    PaymentsScopeState& m_scope;
    PaymentsInputSupport& m_input_support;
    PaymentsRuntime& m_runtime;
};

} // namespace app::handlers::payments
