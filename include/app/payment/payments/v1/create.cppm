export module app_payment:payments_handlers_v1_create;

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

// v1 create handlers. Owns its add_route registrations; the version facade calls
// install_route_NN in the original registration order.
class PaymentsV1CreateHandlers
{
public:
    void bind_request_support(PaymentsRequestSupport& support) noexcept
    {
        m_request_support = &support;
    }

    PaymentsRequestSupport* m_request_support{nullptr};

    PaymentsV1CreateHandlers(
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

    void handle_payments_create(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};

        if (const auto auth_error =
                m_request_support->authorize(req, "payments_create", spec_for("payments_create"))) {
            auth_error->send(sink);
            return;
        }
        m_context.metrics().record_flow("payments.payments_create");
        m_context.metrics().counter("payments.payments_create").increment();

        auto parsed_dto = core::behavior::Serializer<::app::model::payments::PaymentsRequest>::pass(req, sink);
        if (!parsed_dto) return;
        const auto& dto = *parsed_dto;
        m_runtime.payments_create(dto, m_request_support->context(req)).handle(sink);
    }

    void handle_payment_link_create(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};

        if (const auto auth_error =
                m_request_support
                    ->authorize(req, "payment_link_create", spec_for("payment_link_create"))) {
            auth_error->send(sink);
            return;
        }
        m_context.metrics().record_flow("payments.payment_link_create");
        m_context.metrics().counter("payments.payment_link_create").increment();

        auto parsed_dto = core::behavior::Serializer<::app::model::payments::PaymentsRequest>::pass(req, sink);
        if (!parsed_dto) return;
        const auto& dto = *parsed_dto;
        m_runtime.payment_link_create(dto, m_request_support->context(req)).handle(sink);
    }

    void handle_payments_update(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};

        if (const auto auth_error =
                m_request_support->authorize(req, "payments_update", spec_for("payments_update"))) {
            auth_error->send(sink);
            return;
        }
        m_context.metrics().record_flow("payments.payments_update");
        m_context.metrics().counter("payments.payments_update").increment();

        auto parsed_dto = core::behavior::Serializer<::app::model::payments::PaymentsRequest>::pass(req, sink);
        if (!parsed_dto) return;
        const auto& dto = *parsed_dto;
        m_runtime.payments_update(dto, m_request_support->context(req)).handle(sink);
    }

    void handle_payments_update_metadata(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};

        if (const auto auth_error = m_request_support->authorize(
                req,
                "payments_update_metadata",
                spec_for("payments_update_metadata")
            )) {
            auth_error->send(sink);
            return;
        }
        m_context.metrics().record_flow("payments.payments_update_metadata");
        m_context.metrics().counter("payments.payments_update_metadata").increment();

        auto parsed_dto = core::behavior::Serializer<::app::model::payments::PaymentsRequest>::pass(req, sink);
        if (!parsed_dto) return;
        const auto& dto = *parsed_dto;
        m_runtime.payments_update_metadata(dto, m_request_support->context(req)).handle(sink);
    }

    void handle_payments_manual_update(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};

        if (const auto auth_error = m_request_support->authorize(
                req,
                "payments_manual_update",
                spec_for("payments_manual_update")
            )) {
            auth_error->send(sink);
            return;
        }
        m_context.metrics().record_flow("payments.payments_manual_update");
        m_context.metrics().counter("payments.payments_manual_update").increment();

        auto parsed_dto = core::behavior::Serializer<::app::model::payments::PaymentsRequest>::pass(req, sink);
        if (!parsed_dto) return;
        const auto& dto = *parsed_dto;
        m_runtime.payments_manual_update(dto, m_request_support->context(req)).handle(sink);
    }

    // original install() route #42
    void install_route_42(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "payments"}.add_router(
            Router{router, ":payment_id"}.add_route(
                Route{"manual-update"}
                    .put(
                        [this](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
        interfaces::io::ResponseSink sink{res, std::move(send)};

                            handle_payments_manual_update(req, res, std::move(send));
                        }
                    )
                    .summary("Hyperswitch payments_manual_update")
            )
        );
    }

    // original install() route #45
    void install_route_45(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "payments"}.add_route(
            Route{""}
                .post(
                    [this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    )
                    {
        interfaces::io::ResponseSink sink{res, std::move(send)};

                        handle_payments_create(req, res, std::move(send));
                    }
                )
                .summary("Hyperswitch payments_create")
        );
    }

    // original install() route #46
    void install_route_46(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "payments"}.add_route(
            Route{"payment_link"}
                .post(
                    [this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    )
                    {
        interfaces::io::ResponseSink sink{res, std::move(send)};

                        handle_payment_link_create(req, res, std::move(send));
                    }
                )
                .summary("Hyperswitch payment_link_create")
        );
    }

    // original install() route #50
    void install_route_50(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "payments"}.add_route(
            Route{":payment_id"}
                .post(
                    [this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    )
                    {
        interfaces::io::ResponseSink sink{res, std::move(send)};

                        handle_payments_update(req, res, std::move(send));
                    }
                )
                .summary("Hyperswitch payments_update")
        );
    }

    // original install() route #67
    void install_route_67(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "payments"}.add_router(
            Router{router, ":payment_id"}.add_route(
                Route{"update_metadata"}
                    .post(
                        [this](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
        interfaces::io::ResponseSink sink{res, std::move(send)};

                            handle_payments_update_metadata(req, res, std::move(send));
                        }
                    )
                    .summary("Hyperswitch payments_update_metadata")
            )
        );
    }

    void install(core::router::RouterContext<>& router)
    {
        install_route_42(router);
        install_route_45(router);
        install_route_46(router);
        install_route_50(router);
        install_route_67(router);
    }

private:
    ::app::context::AppContext& m_context;
    PaymentsScopeState& m_scope;
    PaymentsInputSupport& m_input_support;
    PaymentsRuntime& m_runtime;
};

} // namespace app::handlers::payments
