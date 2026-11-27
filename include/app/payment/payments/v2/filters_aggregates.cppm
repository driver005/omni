export module app_payment:payments_handlers_v2_filters_aggregates;

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

// v2 filters_aggregates handlers. Owns its add_route registrations; the version facade calls
// install_route_NN in the original registration order.
class PaymentsV2FiltersAggregatesHandlers
{
public:
    void bind_request_support(PaymentsRequestSupport& support) noexcept
    {
        m_request_support = &support;
    }

    PaymentsRequestSupport* m_request_support{nullptr};

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
        interfaces::io::ResponseSink sink{res, std::move(send)};

        if (const auto auth_error =
                m_request_support
                    ->authorize(req, "get_payment_filters", spec_for("get_payment_filters"))) {
            auth_error->send(sink);
            return;
        }
        m_context.logger().record_flow("payments.get_payment_filters");
        m_context.logger().counter("payments.get_payment_filters").increment();

        const ::app::model::payments::PaymentsRequest dto{};
        m_runtime.get_payment_filters(dto, m_request_support->context(req)).handle(sink);
    }

    void handle_get_payment_filters_profile(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};

        if (const auto auth_error = m_request_support->authorize(
                req,
                "get_payment_filters_profile",
                spec_for("get_payment_filters_profile")
            )) {
            auth_error->send(sink);
            return;
        }
        m_context.logger().record_flow("payments.get_payment_filters_profile");
        m_context.logger().counter("payments.get_payment_filters_profile").increment();

        const ::app::model::payments::PaymentsRequest dto{};
        m_runtime.get_payment_filters_profile(dto, m_request_support->context(req)).handle(sink);
    }

    void handle_get_payments_aggregates(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};

        if (const auto auth_error = m_request_support->authorize(
                req,
                "get_payments_aggregates",
                spec_for("get_payments_aggregates")
            )) {
            auth_error->send(sink);
            return;
        }
        m_context.logger().record_flow("payments.get_payments_aggregates");
        m_context.logger().counter("payments.get_payments_aggregates").increment();

        const ::app::model::payments::PaymentsRequest dto{};
        m_runtime.get_payments_aggregates(dto, m_request_support->context(req)).handle(sink);
    }

    void handle_get_payments_aggregates_profile(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};

        if (const auto auth_error = m_request_support->authorize(
                req,
                "get_payments_aggregates_profile",
                spec_for("get_payments_aggregates_profile")
            )) {
            auth_error->send(sink);
            return;
        }
        m_context.logger().record_flow("payments.get_payments_aggregates_profile");
        m_context.logger().counter("payments.get_payments_aggregates_profile").increment();

        const ::app::model::payments::PaymentsRequest dto{};
        m_runtime.get_payments_aggregates_profile(dto, m_request_support->context(req))
            .handle(sink);
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
                        interfaces::io::ResponseSink sink{res, std::move(send)};

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
                            interfaces::io::ResponseSink sink{res, std::move(send)};

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
                        interfaces::io::ResponseSink sink{res, std::move(send)};

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
                            interfaces::io::ResponseSink sink{res, std::move(send)};

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
