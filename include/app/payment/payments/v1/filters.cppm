export module app_payment:payments_handlers_v1_filters;

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

// v1 filters handlers. Owns its add_route registrations; the version facade calls
// install_route_NN in the original registration order.
class PaymentsV1FiltersHandlers
{
public:
    void bind_request_support(PaymentsRequestSupport& support) noexcept
    {
        m_request_support = &support;
    }

    PaymentsRequestSupport* m_request_support{nullptr};

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
        interfaces::io::ResponseSink sink{res, std::move(send)};

        if (const auto auth_error = m_request_support->authorize(
                req,
                "get_filters_for_payments",
                spec_for("get_filters_for_payments")
            )) {
            auth_error->send(sink);
            return;
        }
        m_context.logger().record_flow("payments.get_filters_for_payments");
        m_context.logger().counter("payments.get_filters_for_payments").increment();

        auto parsed_dto =
            core::behavior::Serializer<::app::model::payments::PaymentsRequest>::pass(req, sink);
        if (!parsed_dto) {
            return;
        }
        const auto& dto = *parsed_dto;
        m_runtime.get_filters_for_payments(dto, m_request_support->context(req)).handle(sink);
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
                        interfaces::io::ResponseSink sink{res, std::move(send)};

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
                            interfaces::io::ResponseSink sink{res, std::move(send)};

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
                                interfaces::io::ResponseSink sink{res, std::move(send)};

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
