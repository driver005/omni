export module app_payment:payments_handlers_v2_list;

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
        if (const auto auth_error =
                m_request_support->authorize(req, "payments_list", spec_for("payments_list"))) {
            res.set_status(auth_error->get_status());
            res.set_body(serde::Ser::serialize("application/json", *auth_error));
            send();
            return;
        }
        m_context.metrics().record_flow("payments.payments_list");
        m_context.metrics().counter("payments.payments_list").increment();

        using Input = ::app::model::payments::PaymentsListRequest;
        auto typed = m_input_support.parse_query_as<Input>(
            req,
            {"merchant_id",
             "profile_id",
             "status",
             "connector",
             "payment_method",
             "payment_method_type",
             "offset",
             "limit",
             "force_sync",
             "expand_attempts"}
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
        PaymentsListInput input{std::move(data)};
        auto result = m_runtime.payments_list(input);
        if (!result) {
            const auto err = m_input_support.api_error(result.error());
            err.send(res, send);
            return;
        }
        res.set_status(interfaces::io::types::Status::OK);
        res.set_body(serialize_wire(*result));
        send();
    }

    void handle_revenue_recovery_invoices_list(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto auth_error = m_request_support->authorize(
                req,
                "revenue_recovery_invoices_list",
                spec_for("revenue_recovery_invoices_list")
            )) {
            res.set_status(auth_error->get_status());
            res.set_body(serde::Ser::serialize("application/json", *auth_error));
            send();
            return;
        }
        m_context.metrics().record_flow("payments.revenue_recovery_invoices_list");
        m_context.metrics().counter("payments.revenue_recovery_invoices_list").increment();

        using Input = ::app::model::payments::PaymentsListRequest;
        auto typed = m_input_support.parse_query_as<Input>(
            req,
            {"merchant_id",
             "profile_id",
             "status",
             "connector",
             "payment_method",
             "payment_method_type",
             "offset",
             "limit",
             "force_sync",
             "expand_attempts"}
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
        RevenueRecoveryInvoicesListInput input{std::move(data)};
        auto result = m_runtime.revenue_recovery_invoices_list(input);
        if (!result) {
            const auto err = m_input_support.api_error(result.error());
            err.send(res, send);
            return;
        }
        res.set_status(interfaces::io::types::Status::OK);
        res.set_body(serialize_wire(*result));
        send();
    }

    void handle_revenue_recovery_invoices_list_profile(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto auth_error = m_request_support->authorize(
                req,
                "revenue_recovery_invoices_list_profile",
                spec_for("revenue_recovery_invoices_list_profile")
            )) {
            res.set_status(auth_error->get_status());
            res.set_body(serde::Ser::serialize("application/json", *auth_error));
            send();
            return;
        }
        m_context.metrics().record_flow("payments.revenue_recovery_invoices_list_profile");
        m_context.metrics().counter("payments.revenue_recovery_invoices_list_profile").increment();

        using Input = ::app::model::payments::PaymentsListRequest;
        auto typed = m_input_support.parse_query_as<Input>(
            req,
            {"merchant_id",
             "profile_id",
             "status",
             "connector",
             "payment_method",
             "payment_method_type",
             "offset",
             "limit",
             "force_sync",
             "expand_attempts"}
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
        RevenueRecoveryInvoicesListProfileInput input{std::move(data)};
        auto result = m_runtime.revenue_recovery_invoices_list_profile(input);
        if (!result) {
            const auto err = m_input_support.api_error(result.error());
            err.send(res, send);
            return;
        }
        res.set_status(interfaces::io::types::Status::OK);
        res.set_body(serialize_wire(*result));
        send();
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
