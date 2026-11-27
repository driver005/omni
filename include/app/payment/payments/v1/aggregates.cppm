export module app_payment:payments_handlers_v1_aggregates;

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
        if (const auto auth_error = m_request_support->authorize(
                req,
                "get_payments_aggregates",
                spec_for("get_payments_aggregates")
            )) {
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
        res.set_body(serialize_wire(*result));
        send();
    }

    void handle_get_payments_aggregates_profile(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto auth_error = m_request_support->authorize(
                req,
                "get_payments_aggregates_profile",
                spec_for("get_payments_aggregates_profile")
            )) {
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
        res.set_body(serialize_wire(*result));
        send();
    }

    void handle_get_payment_list_from_opensearch(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto auth_error = m_request_support->authorize(
                req,
                "payments_list",
                spec_for("get_payment_list_from_opensearch")
            )) {
            res.set_status(auth_error->get_status());
            res.set_body(serde::Ser::serialize("application/json", *auth_error));
            send();
            return;
        }
        m_context.metrics().record_flow("payments.get_payment_list_from_opensearch");
        m_context.metrics().counter("payments.get_payment_list_from_opensearch").increment();

        PaymentsRequestData data = m_input_support.request_data(req);
        GetPaymentListFromOpensearchInput input{std::move(data)};
        auto result = m_runtime.get_payment_list_from_opensearch(input);
        if (!result) {
            const auto err = m_input_support.api_error(result.error());
            err.send(res, send);
            return;
        }
        res.set_status(interfaces::io::types::Status::OK);
        res.set_body(serialize_wire(*result));
        send();
    }

    void handle_get_profile_payment_list_from_opensearch(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto auth_error = m_request_support->authorize(
                req,
                "profile_payments_list",
                spec_for("get_profile_payment_list_from_opensearch")
            )) {
            res.set_status(auth_error->get_status());
            res.set_body(serde::Ser::serialize("application/json", *auth_error));
            send();
            return;
        }
        m_context.metrics().record_flow("payments.get_profile_payment_list_from_opensearch");
        m_context.metrics()
            .counter("payments.get_profile_payment_list_from_opensearch")
            .increment();

        PaymentsRequestData data = m_input_support.request_data(req);
        GetProfilePaymentListFromOpensearchInput input{std::move(data)};
        auto result = m_runtime.get_profile_payment_list_from_opensearch(input);
        if (!result) {
            const auto err = m_input_support.api_error(result.error());
            err.send(res, send);
            return;
        }
        res.set_status(interfaces::io::types::Status::OK);
        res.set_body(serialize_wire(*result));
        send();
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
