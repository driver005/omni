export module app_payment:payments_handlers_v2_filters_aggregates;

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

// v2 filters_aggregates handlers. Owns its add_route registrations; the version facade calls
// install_route_NN in the original registration order.
class PaymentsV2FiltersAggregatesHandlers
{
public:
    PaymentsV2FiltersAggregatesHandlers(
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

    void handle_get_payments_aggregates(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto auth_error = m_input_support.authorize(req, "get_payments_aggregates")) {
            res.set_status(auth_error->get_status());
            res.set_body(serde::Ser::serialize("application/json", *auth_error));
            send();
            return;
        }
        m_context.metrics().record_flow("payments.get_payments_aggregates");
        m_context.metrics().counter("payments.get_payments_aggregates").increment();

        using Input = ::app::model::payments::PaymentsAggregateRequest;
        auto typed =
            m_input_support.parse_query_as<Input>(req, {"start", "end", "status", "profile_id"});
        if (!typed) {
            const auto err = m_input_support.api_error(
                core::error::AppErrorKind::INVALID_REQUEST,
                "invalid query parameters"
            );
            err.send(res, send);
            return;
        }
        PaymentsRequestData data = m_input_support.request_data(req);
        GetPaymentsAggregatesInput input{std::move(data)};
        auto result = m_runtime.get_payments_aggregates(input);
        if (!result) {
            const auto err = m_input_support.api_error(result.error());
            err.send(res, send);
            return;
        }
        res.set_status(interfaces::io::types::Status::OK);
        res.set_body(serde::Ser::serialize("application/json", *result));
        send();
    }

    void handle_get_payments_aggregates_profile(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto auth_error =
                m_input_support.authorize(req, "get_payments_aggregates_profile")) {
            res.set_status(auth_error->get_status());
            res.set_body(serde::Ser::serialize("application/json", *auth_error));
            send();
            return;
        }
        m_context.metrics().record_flow("payments.get_payments_aggregates_profile");
        m_context.metrics().counter("payments.get_payments_aggregates_profile").increment();

        using Input = ::app::model::payments::PaymentsAggregateRequest;
        auto typed =
            m_input_support.parse_query_as<Input>(req, {"start", "end", "status", "profile_id"});
        if (!typed) {
            const auto err = m_input_support.api_error(
                core::error::AppErrorKind::INVALID_REQUEST,
                "invalid query parameters"
            );
            err.send(res, send);
            return;
        }
        PaymentsRequestData data = m_input_support.request_data(req);
        GetPaymentsAggregatesProfileInput input{std::move(data)};
        auto result = m_runtime.get_payments_aggregates_profile(input);
        if (!result) {
            const auto err = m_input_support.api_error(result.error());
            err.send(res, send);
            return;
        }
        res.set_status(interfaces::io::types::Status::OK);
        res.set_body(serde::Ser::serialize("application/json", *result));
        send();
    }

    // original install() route #1
    void install_route_01(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "v2/payments"}.add_route(
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
        );
    }

    // original install() route #2
    void install_route_02(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "v2/payments"}.add_router(
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
        );
    }

    // original install() route #7
    void install_route_07(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "v2/payments"}.add_route(
            Route{"aggregate"}
                .get(
                    [this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    )
                    {
                        handle_get_payments_aggregates(req, res, std::move(send));
                    }
                )
                .summary("Hyperswitch get_payments_aggregates")
        );
    }

    // original install() route #9
    void install_route_09(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "v2/payments"}.add_router(
            Router{router, "profile"}.add_route(
                Route{"aggregate"}
                    .get(
                        [this](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
                            handle_get_payments_aggregates_profile(req, res, std::move(send));
                        }
                    )
                    .summary("Hyperswitch get_payments_aggregates_profile")
            )
        );
    }

    void install(core::router::RouterContext<>& router)
    {
        install_route_01(router);
        install_route_02(router);
        install_route_07(router);
        install_route_09(router);
    }

private:
    ::app::context::AppContext& m_context;
    PaymentsScopeState& m_scope;
    PaymentsInputSupport& m_input_support;
    PaymentsRuntime& m_runtime;
};

} // namespace app::handlers::payments
