export module app_payment:payments_handlers_v1_list;

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

// v1 list handlers. Owns its add_route registrations; the version facade calls
// install_route_NN in the original registration order.
class PaymentsV1ListHandlers
{
public:
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
        if (const auto auth_error = m_input_support.authorize(req, "payments_list")) {
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
            res.set_status(err.get_status());
            res.set_body(serde::Ser::serialize("application/json", err));
            send();
            return;
        }
        PaymentsRequestData data = m_input_support.request_data(req);
        PaymentsListInput input{std::move(data)};
        auto result = m_runtime.payments_list(input);
        if (!result) {
            const auto err = m_input_support.api_error(result.error());
            res.set_status(err.get_status());
            res.set_body(serde::Ser::serialize("application/json", err));
            send();
            return;
        }
        res.set_status(interfaces::io::types::Status::OK);
        res.set_body(serde::Ser::serialize("application/json", *result));
        send();
    }

    void handle_payments_list_by_filter(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto auth_error = m_input_support.authorize(req, "payments_list_by_filter")) {
            res.set_status(auth_error->get_status());
            res.set_body(serde::Ser::serialize("application/json", *auth_error));
            send();
            return;
        }
        m_context.metrics().record_flow("payments.payments_list_by_filter");
        m_context.metrics().counter("payments.payments_list_by_filter").increment();

        const auto body = req.body_as_string();
        if (body.empty()) {
            const auto err = m_input_support.api_error(
                core::error::AppErrorKind::EMPTY_REQUEST,
                "empty request"
            );
            res.set_status(err.get_status());
            res.set_body(serde::Ser::serialize("application/json", err));
            send();
            return;
        }
        using Input = ::app::model::payments::PaymentsRequest;
        auto typed = serde::Ser::deserialize<Input>("application/json", body);
        auto generic = serde::Ser::decode_generic("application/json", body);
        if (!typed && !generic) {
            const auto err =
                m_input_support.api_error(core::error::AppErrorKind::INVALID_JSON, "invalid JSON");
            res.set_status(err.get_status());
            res.set_body(serde::Ser::serialize("application/json", err));
            send();
            return;
        }
        PaymentsRequestData data = m_input_support.request_data(req);
        data.body = body;
        PaymentsListByFilterInput input{std::move(data)};
        auto result = m_runtime.payments_list_by_filter(input);
        if (!result) {
            const auto err = m_input_support.api_error(result.error());
            res.set_status(err.get_status());
            res.set_body(serde::Ser::serialize("application/json", err));
            send();
            return;
        }
        res.set_status(interfaces::io::types::Status::OK);
        res.set_body(serde::Ser::serialize("application/json", *result));
        send();
    }

    void handle_payments_list_for_platform(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto auth_error = m_input_support.authorize(req, "payments_list_for_platform")) {
            res.set_status(auth_error->get_status());
            res.set_body(serde::Ser::serialize("application/json", *auth_error));
            send();
            return;
        }
        m_context.metrics().record_flow("payments.payments_list_for_platform");
        m_context.metrics().counter("payments.payments_list_for_platform").increment();

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
            res.set_status(err.get_status());
            res.set_body(serde::Ser::serialize("application/json", err));
            send();
            return;
        }
        PaymentsRequestData data = m_input_support.request_data(req);
        PaymentsListForPlatformInput input{std::move(data)};
        auto result = m_runtime.payments_list_for_platform(input);
        if (!result) {
            const auto err = m_input_support.api_error(result.error());
            res.set_status(err.get_status());
            res.set_body(serde::Ser::serialize("application/json", err));
            send();
            return;
        }
        res.set_status(interfaces::io::types::Status::OK);
        res.set_body(serde::Ser::serialize("application/json", *result));
        send();
    }

    void handle_payments_list_for_platform_filters(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto auth_error =
                m_input_support.authorize(req, "payments_list_for_platform_filters")) {
            res.set_status(auth_error->get_status());
            res.set_body(serde::Ser::serialize("application/json", *auth_error));
            send();
            return;
        }
        m_context.metrics().record_flow("payments.payments_list_for_platform_filters");
        m_context.metrics().counter("payments.payments_list_for_platform_filters").increment();

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
            res.set_status(err.get_status());
            res.set_body(serde::Ser::serialize("application/json", err));
            send();
            return;
        }
        PaymentsRequestData data = m_input_support.request_data(req);
        PaymentsListForPlatformFiltersInput input{std::move(data)};
        auto result = m_runtime.payments_list_for_platform_filters(input);
        if (!result) {
            const auto err = m_input_support.api_error(result.error());
            res.set_status(err.get_status());
            res.set_body(serde::Ser::serialize("application/json", err));
            send();
            return;
        }
        res.set_status(interfaces::io::types::Status::OK);
        res.set_body(serde::Ser::serialize("application/json", *result));
        send();
    }

    void handle_profile_payments_list(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto auth_error = m_input_support.authorize(req, "profile_payments_list")) {
            res.set_status(auth_error->get_status());
            res.set_body(serde::Ser::serialize("application/json", *auth_error));
            send();
            return;
        }
        m_context.metrics().record_flow("payments.profile_payments_list");
        m_context.metrics().counter("payments.profile_payments_list").increment();

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
            res.set_status(err.get_status());
            res.set_body(serde::Ser::serialize("application/json", err));
            send();
            return;
        }
        PaymentsRequestData data = m_input_support.request_data(req);
        ProfilePaymentsListInput input{std::move(data)};
        auto result = m_runtime.profile_payments_list(input);
        if (!result) {
            const auto err = m_input_support.api_error(result.error());
            res.set_status(err.get_status());
            res.set_body(serde::Ser::serialize("application/json", err));
            send();
            return;
        }
        res.set_status(interfaces::io::types::Status::OK);
        res.set_body(serde::Ser::serialize("application/json", *result));
        send();
    }

    void handle_profile_payments_list_by_filter(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto auth_error =
                m_input_support.authorize(req, "profile_payments_list_by_filter")) {
            res.set_status(auth_error->get_status());
            res.set_body(serde::Ser::serialize("application/json", *auth_error));
            send();
            return;
        }
        m_context.metrics().record_flow("payments.profile_payments_list_by_filter");
        m_context.metrics().counter("payments.profile_payments_list_by_filter").increment();

        const auto body = req.body_as_string();
        if (body.empty()) {
            const auto err = m_input_support.api_error(
                core::error::AppErrorKind::EMPTY_REQUEST,
                "empty request"
            );
            res.set_status(err.get_status());
            res.set_body(serde::Ser::serialize("application/json", err));
            send();
            return;
        }
        using Input = ::app::model::payments::PaymentsRequest;
        auto typed = serde::Ser::deserialize<Input>("application/json", body);
        auto generic = serde::Ser::decode_generic("application/json", body);
        if (!typed && !generic) {
            const auto err =
                m_input_support.api_error(core::error::AppErrorKind::INVALID_JSON, "invalid JSON");
            res.set_status(err.get_status());
            res.set_body(serde::Ser::serialize("application/json", err));
            send();
            return;
        }
        PaymentsRequestData data = m_input_support.request_data(req);
        data.body = body;
        ProfilePaymentsListByFilterInput input{std::move(data)};
        auto result = m_runtime.profile_payments_list_by_filter(input);
        if (!result) {
            const auto err = m_input_support.api_error(result.error());
            res.set_status(err.get_status());
            res.set_body(serde::Ser::serialize("application/json", err));
            send();
            return;
        }
        res.set_status(interfaces::io::types::Status::OK);
        res.set_body(serde::Ser::serialize("application/json", *result));
        send();
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
