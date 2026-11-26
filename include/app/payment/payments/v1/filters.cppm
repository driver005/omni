export module app_payment:payments_handlers_v1_filters;

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

// v1 filters handlers. Owns its add_route registrations; the version facade calls
// install_route_NN in the original registration order.
class PaymentsV1FiltersHandlers
{
public:
    PaymentsV1FiltersHandlers(
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

    void handle_get_filters_for_payments(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto auth_error = m_input_support.authorize(req, "get_filters_for_payments")) {
            res.set_status(auth_error->get_status());
            res.set_body(serde::Ser::serialize("application/json", *auth_error));
            send();
            return;
        }
        m_context.metrics().record_flow("payments.get_filters_for_payments");
        m_context.metrics().counter("payments.get_filters_for_payments").increment();

        const auto body = req.body_as_string();
        if (body.empty()) {
            const auto err = m_input_support.api_error(
                core::error::AppErrorKind::EMPTY_REQUEST,
                "empty request"
            );
            err.send(res, send);
            return;
        }
        using Input = ::app::model::payments::PaymentsRequest;
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
        GetFiltersForPaymentsInput input{std::move(data)};
        auto result = m_runtime.get_filters_for_payments(input);
        if (!result) {
            const auto err = m_input_support.api_error(result.error());
            err.send(res, send);
            return;
        }
        res.set_status(interfaces::io::types::Status::OK);
        res.set_body(serde::Ser::serialize("application/json", *result));
        send();
    }

    void handle_get_payment_filters(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto auth_error = m_input_support.authorize(req, "get_payment_filters")) {
            res.set_status(auth_error->get_status());
            res.set_body(serde::Ser::serialize("application/json", *auth_error));
            send();
            return;
        }
        m_context.metrics().record_flow("payments.get_payment_filters");
        m_context.metrics().counter("payments.get_payment_filters").increment();

        using Input = ::app::model::payments::PaymentsFilterRequest;
        auto typed = m_input_support.parse_query_as<Input>(
            req,
            {"status", "connector", "payment_method", "payment_method_type", "offset", "limit"}
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
        GetPaymentFiltersInput input{std::move(data)};
        auto result = m_runtime.get_payment_filters(input);
        if (!result) {
            const auto err = m_input_support.api_error(result.error());
            err.send(res, send);
            return;
        }
        res.set_status(interfaces::io::types::Status::OK);
        res.set_body(serde::Ser::serialize("application/json", *result));
        send();
    }

    void handle_get_payment_filters_profile(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto auth_error = m_input_support.authorize(req, "get_payment_filters_profile")) {
            res.set_status(auth_error->get_status());
            res.set_body(serde::Ser::serialize("application/json", *auth_error));
            send();
            return;
        }
        m_context.metrics().record_flow("payments.get_payment_filters_profile");
        m_context.metrics().counter("payments.get_payment_filters_profile").increment();

        using Input = ::app::model::payments::PaymentsFilterRequest;
        auto typed = m_input_support.parse_query_as<Input>(
            req,
            {"status", "connector", "payment_method", "payment_method_type", "offset", "limit"}
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
        GetPaymentFiltersProfileInput input{std::move(data)};
        auto result = m_runtime.get_payment_filters_profile(input);
        if (!result) {
            const auto err = m_input_support.api_error(result.error());
            err.send(res, send);
            return;
        }
        res.set_status(interfaces::io::types::Status::OK);
        res.set_body(serde::Ser::serialize("application/json", *result));
        send();
    }

    // original install() route #34
    void install_route_34(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "payments"}.add_route(
            Route{"filter"}
                .post(
                    [this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    )
                    {
                        handle_get_filters_for_payments(req, res, std::move(send));
                    }
                )
                .summary("Hyperswitch get_filters_for_payments")
        );
    }

    // original install() route #35
    void install_route_35(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "payments"}.add_router(
            Router{router, "v2"}.add_route(
                Route{"filter"}
                    .get(
                        [this](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
                            handle_get_payment_filters(req, res, std::move(send));
                        }
                    )
                    .summary("Hyperswitch get_payment_filters")
            )
        );
    }

    // original install() route #41
    void install_route_41(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "payments"}.add_router(
            Router{router, "v2"}.add_router(
                Router{router, "profile"}.add_route(
                    Route{"filter"}
                        .get(
                            [this](
                                interfaces::io::IRequest& req,
                                interfaces::io::IResponse& res,
                                std::function<void()> send
                            )
                            {
                                handle_get_payment_filters_profile(req, res, std::move(send));
                            }
                        )
                        .summary("Hyperswitch get_payment_filters_profile")
                )
            )
        );
    }

    void install(core::router::RouterContext<>& router)
    {
        install_route_34(router);
        install_route_35(router);
        install_route_41(router);
    }

private:
    ::app::context::AppContext& m_context;
    PaymentsScopeState& m_scope;
    PaymentsInputSupport& m_input_support;
    PaymentsRuntime& m_runtime;
};

} // namespace app::handlers::payments
