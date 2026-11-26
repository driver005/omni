export module app_payment:payments_handlers_v2_capture_cancel;

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

// v2 capture_cancel handlers. Owns its add_route registrations; the version facade calls
// install_route_NN in the original registration order.
class PaymentsV2CaptureCancelHandlers
{
public:
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
        if (const auto auth_error = m_input_support.authorize(req, "payments_capture")) {
            res.set_status(auth_error->get_status());
            res.set_body(serde::Ser::serialize("application/json", *auth_error));
            send();
            return;
        }
        m_context.metrics().record_flow("payments.payments_capture");
        m_context.metrics().counter("payments.payments_capture").increment();

        const auto body = req.body_as_string();
        if (body.empty()) {
            const auto err = m_input_support.api_error(
                core::error::AppErrorKind::EMPTY_REQUEST,
                "empty request"
            );
            res.set_status(err.get_status());
            res.set_body(serde::Ser::serialize("application/json", err));
            send();
            return;
        }
        using Input = ::app::model::payments::PaymentsCaptureRequest;
        auto typed = serde::Ser::deserialize<Input>("application/json", body);
        auto generic = serde::Ser::decode_generic("application/json", body);
        if (!typed && !generic) {
            const auto err =
                m_input_support.api_error(core::error::AppErrorKind::INVALID_JSON, "invalid JSON");
            res.set_status(err.get_status());
            res.set_body(serde::Ser::serialize("application/json", err));
            send();
            return;
        }
        PaymentsRequestData data = m_input_support.request_data(req);
        data.body = body;
        PaymentsCaptureInput input{std::move(data)};
        auto result = m_runtime.payments_capture(input);
        if (!result) {
            const auto err = m_input_support.api_error(result.error());
            res.set_status(err.get_status());
            res.set_body(serde::Ser::serialize("application/json", err));
            send();
            return;
        }
        res.set_status(interfaces::io::types::Status::OK);
        res.set_body(serde::Ser::serialize("application/json", *result));
        send();
    }

    void handle_payments_cancel(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto auth_error = m_input_support.authorize(req, "payments_cancel")) {
            res.set_status(auth_error->get_status());
            res.set_body(serde::Ser::serialize("application/json", *auth_error));
            send();
            return;
        }
        m_context.metrics().record_flow("payments.payments_cancel");
        m_context.metrics().counter("payments.payments_cancel").increment();

        const auto body = req.body_as_string();
        if (body.empty()) {
            const auto err = m_input_support.api_error(
                core::error::AppErrorKind::EMPTY_REQUEST,
                "empty request"
            );
            res.set_status(err.get_status());
            res.set_body(serde::Ser::serialize("application/json", err));
            send();
            return;
        }
        using Input = ::app::model::payments::PaymentsCancelRequest;
        auto typed = serde::Ser::deserialize<Input>("application/json", body);
        auto generic = serde::Ser::decode_generic("application/json", body);
        if (!typed && !generic) {
            const auto err =
                m_input_support.api_error(core::error::AppErrorKind::INVALID_JSON, "invalid JSON");
            res.set_status(err.get_status());
            res.set_body(serde::Ser::serialize("application/json", err));
            send();
            return;
        }
        PaymentsRequestData data = m_input_support.request_data(req);
        data.body = body;
        PaymentsCancelInput input{std::move(data)};
        auto result = m_runtime.payments_cancel(input);
        if (!result) {
            const auto err = m_input_support.api_error(result.error());
            res.set_status(err.get_status());
            res.set_body(serde::Ser::serialize("application/json", err));
            send();
            return;
        }
        res.set_status(interfaces::io::types::Status::OK);
        res.set_body(serde::Ser::serialize("application/json", *result));
        send();
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
