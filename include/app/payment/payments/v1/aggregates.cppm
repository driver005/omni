export module app_payment:payments_handlers_v1_aggregates;

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

// v1 aggregates handlers. Owns its add_route registrations; the version facade calls
// install_route_NN in the original registration order.
class PaymentsV1AggregatesHandlers
{
public:
    void bind_request_support(PaymentsRequestSupport& support) noexcept
    {
        m_request_support = &support;
    }

    PaymentsRequestSupport* m_request_support{nullptr};

    PaymentsV1AggregatesHandlers(
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

    void handle_get_payment_list_from_opensearch(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};

        if (const auto auth_error = m_request_support->authorize(
                req,
                "payments_list",
                spec_for("get_payment_list_from_opensearch")
            )) {
            auth_error->send(sink);
            return;
        }
        m_context.logger().record_flow("payments.get_payment_list_from_opensearch");
        m_context.logger().counter("payments.get_payment_list_from_opensearch").increment();

        auto parsed_dto =
            core::behavior::Serializer<::app::model::payments::PaymentsRequest>::pass(req, sink);
        if (!parsed_dto) {
            return;
        }
        const auto& dto = *parsed_dto;
        m_runtime.get_payment_list_from_opensearch(dto, m_request_support->context(req))
            .handle(sink);
    }

    void handle_get_profile_payment_list_from_opensearch(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};

        if (const auto auth_error = m_request_support->authorize(
                req,
                "profile_payments_list",
                spec_for("get_profile_payment_list_from_opensearch")
            )) {
            auth_error->send(sink);
            return;
        }
        m_context.logger().record_flow("payments.get_profile_payment_list_from_opensearch");
        m_context.logger().counter("payments.get_profile_payment_list_from_opensearch").increment();

        auto parsed_dto =
            core::behavior::Serializer<::app::model::payments::PaymentsRequest>::pass(req, sink);
        if (!parsed_dto) {
            return;
        }
        const auto& dto = *parsed_dto;
        m_runtime.get_profile_payment_list_from_opensearch(dto, m_request_support->context(req))
            .handle(sink);
    }

    // original install() route #36
    void install_route_36(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "payments"}.add_route(
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

    // original install() route #37
    void install_route_37(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "payments"}.add_router(
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

    // original install() route #38
    void install_route_38(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "payments"}.add_router(
            Router{router, "advanced"}.add_route(
                Route{"list"}
                    .post(
                        [this](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
                            interfaces::io::ResponseSink sink{res, std::move(send)};

                            handle_get_payment_list_from_opensearch(req, res, std::move(send));
                        }
                    )
                    .summary("Hyperswitch get_payment_list_from_opensearch")
            )
        );
    }

    // original install() route #39
    void install_route_39(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "payments"}.add_router(
            Router{router, "profile"}.add_router(
                Router{router, "advanced"}.add_route(
                    Route{"list"}
                        .post(
                            [this](
                                interfaces::io::IRequest& req,
                                interfaces::io::IResponse& res,
                                std::function<void()> send
                            )
                            {
                                interfaces::io::ResponseSink sink{res, std::move(send)};

                                handle_get_profile_payment_list_from_opensearch(
                                    req,
                                    res,
                                    std::move(send)
                                );
                            }
                        )
                        .summary("Hyperswitch get_profile_payment_list_from_opensearch")
                )
            )
        );
    }

    void install(core::router::RouterContext<>& router)
    {
        install_route_36(router);
        install_route_37(router);
        install_route_38(router);
        install_route_39(router);
    }

private:
    ::app::context::AppContext& m_context;
    PaymentsScopeState& m_scope;
    PaymentsInputSupport& m_input_support;
    PaymentsRuntime& m_runtime;
};

} // namespace app::handlers::payments
