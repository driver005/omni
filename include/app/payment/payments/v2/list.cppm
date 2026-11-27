export module app_payment:payments_handlers_v2_list;

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

// v2 list handlers. Owns its add_route registrations; the version facade calls
// install_route_NN in the original registration order.
class PaymentsV2ListHandlers
{
public:
    void bind_request_support(PaymentsRequestSupport& support) noexcept
    {
        m_request_support = &support;
    }

    PaymentsRequestSupport* m_request_support{nullptr};

    PaymentsV2ListHandlers(
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

    void handle_revenue_recovery_invoices_list(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};

        if (const auto auth_error = m_request_support->authorize(
                req,
                "revenue_recovery_invoices_list",
                spec_for("revenue_recovery_invoices_list")
            )) {
            auth_error->send(sink);
            return;
        }
        m_context.logger().record_flow("payments.revenue_recovery_invoices_list");
        m_context.logger().counter("payments.revenue_recovery_invoices_list").increment();

        const ::app::model::payments::PaymentsRequest dto{};
        m_runtime.revenue_recovery_invoices_list(dto, m_request_support->context(req)).handle(sink);
    }

    void handle_revenue_recovery_invoices_list_profile(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};

        if (const auto auth_error = m_request_support->authorize(
                req,
                "revenue_recovery_invoices_list_profile",
                spec_for("revenue_recovery_invoices_list_profile")
            )) {
            auth_error->send(sink);
            return;
        }
        m_context.logger().record_flow("payments.revenue_recovery_invoices_list_profile");
        m_context.logger().counter("payments.revenue_recovery_invoices_list_profile").increment();

        const ::app::model::payments::PaymentsRequest dto{};
        m_runtime.revenue_recovery_invoices_list_profile(dto, m_request_support->context(req))
            .handle(sink);
    }

    // original install() route #4
    void install_route_04(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "v2/payments"}.add_route(
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

    // original install() route #5
    void install_route_05(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "v2/payments"}.add_route(
            Route{"recovery-list"}
                .get(
                    [this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    )
                    {
                        interfaces::io::ResponseSink sink{res, std::move(send)};

                        handle_revenue_recovery_invoices_list(req, res, std::move(send));
                    }
                )
                .summary("Hyperswitch revenue_recovery_invoices_list")
        );
    }

    // original install() route #6
    void install_route_06(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "v2/payments"}.add_router(
            Router{router, "profile"}.add_route(
                Route{"recovery-list"}
                    .get(
                        [this](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
                            interfaces::io::ResponseSink sink{res, std::move(send)};

                            handle_revenue_recovery_invoices_list_profile(
                                req,
                                res,
                                std::move(send)
                            );
                        }
                    )
                    .summary("Hyperswitch revenue_recovery_invoices_list_profile")
            )
        );
    }

    void install(core::router::RouterContext<>& router)
    {
        install_route_04(router);
        install_route_05(router);
        install_route_06(router);
    }

private:
    ::app::context::AppContext& m_context;
    PaymentsScopeState& m_scope;
    PaymentsInputSupport& m_input_support;
    PaymentsRuntime& m_runtime;
};

} // namespace app::handlers::payments
