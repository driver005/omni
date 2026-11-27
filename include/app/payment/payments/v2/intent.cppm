export module app_payment:payments_handlers_v2_intent;

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
        if (const auto auth_error = m_request_support->authorize(
                req,
                "payment_get_intent_using_merchant_reference_id",
                spec_for("payment_get_intent_using_merchant_reference_id")
            )) {
            res.set_status(auth_error->get_status());
            res.set_body(serde::Ser::serialize("application/json", *auth_error));
            send();
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
            err.send(res, send);
            return;
        }
        using Input = ::app::model::payments::PaymentsGetIntentRequest;
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
        PaymentGetIntentUsingMerchantReferenceIdInput input{std::move(data)};
        auto result = m_runtime.payment_get_intent_using_merchant_reference_id(input);
        if (!result) {
            const auto err = m_input_support.api_error(result.error());
            err.send(res, send);
            return;
        }
        res.set_status(interfaces::io::types::Status::OK);
        res.set_body(serialize_wire(*result));
        send();
    }

    void handle_payments_get_intent(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto auth_error =
                m_request_support
                    ->authorize(req, "payments_get_intent", spec_for("payments_get_intent"))) {
            res.set_status(auth_error->get_status());
            res.set_body(serde::Ser::serialize("application/json", *auth_error));
            send();
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
            err.send(res, send);
            return;
        }
        using Input = ::app::model::payments::PaymentsGetIntentRequest;
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
        PaymentsGetIntentInput input{std::move(data)};
        auto result = m_runtime.payments_get_intent(input);
        if (!result) {
            const auto err = m_input_support.api_error(result.error());
            err.send(res, send);
            return;
        }
        res.set_status(interfaces::io::types::Status::OK);
        res.set_body(serialize_wire(*result));
        send();
    }

    void handle_revenue_recovery_get_intent(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto auth_error = m_request_support->authorize(
                req,
                "revenue_recovery_get_intent",
                spec_for("revenue_recovery_get_intent")
            )) {
            res.set_status(auth_error->get_status());
            res.set_body(serde::Ser::serialize("application/json", *auth_error));
            send();
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
            err.send(res, send);
            return;
        }
        using Input = ::app::model::payments::PaymentsGetIntentRequest;
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
        RevenueRecoveryGetIntentInput input{std::move(data)};
        auto result = m_runtime.revenue_recovery_get_intent(input);
        if (!result) {
            const auto err = m_input_support.api_error(result.error());
            err.send(res, send);
            return;
        }
        res.set_status(interfaces::io::types::Status::OK);
        res.set_body(serialize_wire(*result));
        send();
    }

    void handle_list_payment_attempts(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto auth_error =
                m_request_support
                    ->authorize(req, "list_payment_attempts", spec_for("list_payment_attempts"))) {
            res.set_status(auth_error->get_status());
            res.set_body(serde::Ser::serialize("application/json", *auth_error));
            send();
            return;
        }
        m_context.metrics().record_flow("payments.list_payment_attempts");
        m_context.metrics().counter("payments.list_payment_attempts").increment();

        using Input = ::app::model::payments::PaymentAttemptListFilterRequest;
        auto typed = m_input_support.parse_query_as<Input>(
            req,
            {"payment_id", "status", "connector", "offset", "limit"}
        );
        if (!typed) {
            const auto err = m_input_support.api_error(
                core::error::AppErrorKind::INVALID_REQUEST,
                "invalid query parameters"
            );
            err.send(res, send);
            return;
        }
        PaymentsRequestData data = m_input_support.request_data(req);
        ListPaymentAttemptsInput input{std::move(data)};
        auto result = m_runtime.list_payment_attempts(input);
        if (!result) {
            const auto err = m_input_support.api_error(result.error());
            err.send(res, send);
            return;
        }
        res.set_status(interfaces::io::types::Status::OK);
        res.set_body(serialize_wire(*result));
        send();
    }

    void handle_list_payment_methods(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto auth_error =
                m_request_support
                    ->authorize(req, "list_payment_methods", spec_for("list_payment_methods"))) {
            res.set_status(auth_error->get_status());
            res.set_body(serde::Ser::serialize("application/json", *auth_error));
            send();
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
            err.send(res, send);
            return;
        }
        using Input = ::app::model::payments::ListMethodsForPaymentsRequest;
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
        ListPaymentMethodsInput input{std::move(data)};
        auto result = m_runtime.list_payment_methods(input);
        if (!result) {
            const auto err = m_input_support.api_error(result.error());
            err.send(res, send);
            return;
        }
        res.set_status(interfaces::io::types::Status::OK);
        res.set_body(serialize_wire(*result));
        send();
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
