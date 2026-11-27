export module app_payment:payments_handlers_v1_list;

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

// v1 list handlers. Owns its add_route registrations; the version facade calls
// install_route_NN in the original registration order.
class PaymentsV1ListHandlers
{
public:
    void bind_request_support(PaymentsRequestSupport& support) noexcept
    {
        m_request_support = &support;
    }

    PaymentsRequestSupport* m_request_support{nullptr};

    PaymentsV1ListHandlers(
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

    void handle_payments_list(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};

        if (const auto auth_error =
                m_request_support->authorize(req, "payments_list", spec_for("payments_list"))) {
            auth_error->send(sink);
            return;
        }
        m_context.logger().record_flow("payments.payments_list");
        m_context.logger().counter("payments.payments_list").increment();

        const ::app::model::payments::PaymentsRequest dto{};
        m_runtime.payments_list(dto, m_request_support->context(req)).handle(sink);
    }

    void handle_payments_list_by_filter(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};

        if (const auto auth_error = m_request_support->authorize(
                req,
                "payments_list_by_filter",
                spec_for("payments_list_by_filter")
            )) {
            auth_error->send(sink);
            return;
        }
        m_context.logger().record_flow("payments.payments_list_by_filter");
        m_context.logger().counter("payments.payments_list_by_filter").increment();

        auto parsed_dto =
            core::behavior::Serializer<::app::model::payments::PaymentsRequest>::pass(req, sink);
        if (!parsed_dto) {
            return;
        }
        const auto& dto = *parsed_dto;
        m_runtime.payments_list_by_filter(dto, m_request_support->context(req)).handle(sink);
    }

    void handle_payments_list_for_platform(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};

        if (const auto auth_error = m_request_support->authorize(
                req,
                "payments_list_for_platform",
                spec_for("payments_list_for_platform")
            )) {
            auth_error->send(sink);
            return;
        }
        m_context.logger().record_flow("payments.payments_list_for_platform");
        m_context.logger().counter("payments.payments_list_for_platform").increment();

        const ::app::model::payments::PaymentsRequest dto{};
        m_runtime.payments_list_for_platform(dto, m_request_support->context(req)).handle(sink);
    }

    void handle_payments_list_for_platform_filters(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};

        if (const auto auth_error = m_request_support->authorize(
                req,
                "payments_list_for_platform_filters",
                spec_for("payments_list_for_platform_filters")
            )) {
            auth_error->send(sink);
            return;
        }
        m_context.logger().record_flow("payments.payments_list_for_platform_filters");
        m_context.logger().counter("payments.payments_list_for_platform_filters").increment();

        const ::app::model::payments::PaymentsRequest dto{};
        m_runtime.payments_list_for_platform_filters(dto, m_request_support->context(req))
            .handle(sink);
    }

    void handle_profile_payments_list(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};

        if (const auto auth_error =
                m_request_support
                    ->authorize(req, "profile_payments_list", spec_for("profile_payments_list"))) {
            auth_error->send(sink);
            return;
        }
        m_context.logger().record_flow("payments.profile_payments_list");
        m_context.logger().counter("payments.profile_payments_list").increment();

        const ::app::model::payments::PaymentsRequest dto{};
        m_runtime.profile_payments_list(dto, m_request_support->context(req)).handle(sink);
    }

    void handle_profile_payments_list_by_filter(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};

        if (const auto auth_error = m_request_support->authorize(
                req,
                "profile_payments_list_by_filter",
                spec_for("profile_payments_list_by_filter")
            )) {
            auth_error->send(sink);
            return;
        }
        m_context.logger().record_flow("payments.profile_payments_list_by_filter");
        m_context.logger().counter("payments.profile_payments_list_by_filter").increment();

        auto parsed_dto =
            core::behavior::Serializer<::app::model::payments::PaymentsRequest>::pass(req, sink);
        if (!parsed_dto) {
            return;
        }
        const auto& dto = *parsed_dto;
        m_runtime.profile_payments_list_by_filter(dto, m_request_support->context(req))
            .handle(sink);
    }

    // original install() route #28
    void install_route_28(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "payments"}.add_route(
            Route{"list"}
                .get(
                    [this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    )
                    {
                        interfaces::io::ResponseSink sink{res, std::move(send)};

                        handle_payments_list(req, res, std::move(send));
                    }
                )
                .summary("Hyperswitch payments_list")
        );
    }

    // original install() route #29
    void install_route_29(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "payments"}.add_route(
            Route{"list"}
                .post(
                    [this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    )
                    {
                        interfaces::io::ResponseSink sink{res, std::move(send)};

                        handle_payments_list_by_filter(req, res, std::move(send));
                    }
                )
                .summary("Hyperswitch payments_list_by_filter")
        );
    }

    // original install() route #30
    void install_route_30(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "payments"}.add_router(
            Router{router, "platform"}.add_route(
                Route{"list"}
                    .get(
                        [this](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
                            interfaces::io::ResponseSink sink{res, std::move(send)};

                            handle_payments_list_for_platform(req, res, std::move(send));
                        }
                    )
                    .summary("Hyperswitch payments_list_for_platform")
            )
        );
    }

    // original install() route #31
    void install_route_31(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "payments"}.add_router(
            Router{router, "platform"}.add_route(
                Route{"filter"}
                    .get(
                        [this](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
                            interfaces::io::ResponseSink sink{res, std::move(send)};

                            handle_payments_list_for_platform_filters(req, res, std::move(send));
                        }
                    )
                    .summary("Hyperswitch payments_list_for_platform_filters")
            )
        );
    }

    // original install() route #32
    void install_route_32(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "payments"}.add_router(
            Router{router, "profile"}.add_route(
                Route{"list"}
                    .get(
                        [this](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
                            interfaces::io::ResponseSink sink{res, std::move(send)};

                            handle_profile_payments_list(req, res, std::move(send));
                        }
                    )
                    .summary("Hyperswitch profile_payments_list")
            )
        );
    }

    // original install() route #33
    void install_route_33(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "payments"}.add_router(
            Router{router, "profile"}.add_route(
                Route{"list"}
                    .post(
                        [this](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
                            interfaces::io::ResponseSink sink{res, std::move(send)};

                            handle_profile_payments_list_by_filter(req, res, std::move(send));
                        }
                    )
                    .summary("Hyperswitch profile_payments_list_by_filter")
            )
        );
    }

    void install(core::router::RouterContext<>& router)
    {
        install_route_28(router);
        install_route_29(router);
        install_route_30(router);
        install_route_31(router);
        install_route_32(router);
        install_route_33(router);
    }

private:
    ::app::context::AppContext& m_context;
    PaymentsScopeState& m_scope;
    PaymentsInputSupport& m_input_support;
    PaymentsRuntime& m_runtime;
};

} // namespace app::handlers::payments
