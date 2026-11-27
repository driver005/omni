export module app_payment:payments_handlers_v1_status;

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

// v1 status handlers. Owns its add_route registrations; the version facade calls
// install_route_NN in the original registration order.
class PaymentsV1StatusHandlers
{
public:
    void bind_request_support(PaymentsRequestSupport& support) noexcept
    {
        m_request_support = &support;
    }

    PaymentsRequestSupport* m_request_support{nullptr};

    PaymentsV1StatusHandlers(
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

    void handle_payments_retrieve(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};

        if (const auto auth_error =
                m_request_support
                    ->authorize(req, "payments_retrieve", spec_for("payments_retrieve"))) {
            auth_error->send(sink);
            return;
        }
        m_context.logger().record_flow("payments.payments_retrieve");
        m_context.logger().counter("payments.payments_retrieve").increment();

        const auto body =
            m_input_support.path_body({{"payment_id", m_input_support.payment_id_from_path(req)}});
        if (body.empty()) {
            const auto err = m_input_support.api_error(
                core::error::AppErrorKind::EMPTY_REQUEST,
                "empty request"
            );
            err.send(sink);
            return;
        }
        auto parsed_dto =
            core::behavior::Serializer<::app::model::payments::PaymentsRequest>::pass(req, sink);
        if (!parsed_dto) {
            return;
        }
        const auto& dto = *parsed_dto;
        m_runtime.payments_retrieve(dto, m_request_support->context(req)).handle(sink);
    }

    void handle_payments_retrieve_with_gateway_creds(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};

        if (const auto auth_error = m_request_support->authorize(
                req,
                "payments_retrieve_with_gateway_creds",
                spec_for("payments_retrieve_with_gateway_creds")
            )) {
            auth_error->send(sink);
            return;
        }
        m_context.logger().record_flow("payments.payments_retrieve_with_gateway_creds");
        m_context.logger().counter("payments.payments_retrieve_with_gateway_creds").increment();

        auto parsed_dto =
            core::behavior::Serializer<::app::model::payments::PaymentsRequest>::pass(req, sink);
        if (!parsed_dto) {
            return;
        }
        const auto& dto = *parsed_dto;
        m_runtime.payments_retrieve_with_gateway_creds(dto, m_request_support->context(req))
            .handle(sink);
    }

    void handle_retrieve_extended_card_info(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};

        if (const auto auth_error = m_request_support->authorize(
                req,
                "retrieve_extended_card_info",
                spec_for("retrieve_extended_card_info")
            )) {
            auth_error->send(sink);
            return;
        }
        m_context.logger().record_flow("payments.retrieve_extended_card_info");
        m_context.logger().counter("payments.retrieve_extended_card_info").increment();

        const auto body =
            m_input_support.path_body({{"payment_id", m_input_support.payment_id_from_path(req)}});
        if (body.empty()) {
            const auto err = m_input_support.api_error(
                core::error::AppErrorKind::EMPTY_REQUEST,
                "empty request"
            );
            err.send(sink);
            return;
        }
        auto parsed_dto =
            core::behavior::Serializer<::app::model::payments::PaymentsRequest>::pass(req, sink);
        if (!parsed_dto) {
            return;
        }
        const auto& dto = *parsed_dto;
        m_runtime.retrieve_extended_card_info(dto, m_request_support->context(req)).handle(sink);
    }

    void handle_payments_manual_status_update(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};

        if (const auto auth_error = m_request_support->authorize(
                req,
                "payments_manual_status_update",
                spec_for("payments_manual_status_update")
            )) {
            auth_error->send(sink);
            return;
        }
        m_context.logger().record_flow("payments.payments_manual_status_update");
        m_context.logger().counter("payments.payments_manual_status_update").increment();

        auto parsed_dto =
            core::behavior::Serializer<::app::model::payments::PaymentsRequest>::pass(req, sink);
        if (!parsed_dto) {
            return;
        }
        const auto& dto = *parsed_dto;
        m_runtime.payments_manual_status_update(dto, m_request_support->context(req)).handle(sink);
    }

    void handle_payments_manual_status_update_eligible_statuses(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};

        if (const auto auth_error = m_request_support->authorize(
                req,
                "payments_manual_status_update_eligible_statuses",
                spec_for("payments_manual_status_update_eligible_statuses")
            )) {
            auth_error->send(sink);
            return;
        }
        m_context.logger().record_flow("payments.payments_manual_status_update_eligible_statuses");
        m_context.logger()
            .counter("payments.payments_manual_status_update_eligible_statuses")
            .increment();

        const auto body =
            m_input_support.path_body({{"payment_id", m_input_support.payment_id_from_path(req)}});
        if (body.empty()) {
            const auto err = m_input_support.api_error(
                core::error::AppErrorKind::EMPTY_REQUEST,
                "empty request"
            );
            err.send(sink);
            return;
        }
        auto parsed_dto =
            core::behavior::Serializer<::app::model::payments::PaymentsRequest>::pass(req, sink);
        if (!parsed_dto) {
            return;
        }
        const auto& dto = *parsed_dto;
        m_runtime
            .payments_manual_status_update_eligible_statuses(dto, m_request_support->context(req))
            .handle(sink);
    }

    // original install() route #43
    void install_route_43(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "payments"}.add_router(
            Router{router, ":payment_id"}.add_route(
                Route{"manual-status-update"}
                    .post(
                        [this](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
                            interfaces::io::ResponseSink sink{res, std::move(send)};

                            handle_payments_manual_status_update(req, res, std::move(send));
                        }
                    )
                    .summary("Hyperswitch payments_manual_status_update")
            )
        );
    }

    // original install() route #44
    void install_route_44(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "payments"}.add_router(
            Router{router, ":payment_id"}.add_route(
                Route{"manual-status-update"}
                    .get(
                        [this](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
                            interfaces::io::ResponseSink sink{res, std::move(send)};

                            handle_payments_manual_status_update_eligible_statuses(
                                req,
                                res,
                                std::move(send)
                            );
                        }
                    )
                    .summary("Hyperswitch payments_manual_status_update_eligible_statuses")
            )
        );
    }

    // original install() route #48
    void install_route_48(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "payments"}.add_route(
            Route{"sync"}
                .post(
                    [this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    )
                    {
                        interfaces::io::ResponseSink sink{res, std::move(send)};

                        handle_payments_retrieve_with_gateway_creds(req, res, std::move(send));
                    }
                )
                .summary("Hyperswitch payments_retrieve_with_gateway_creds")
        );
    }

    // original install() route #49
    void install_route_49(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "payments"}.add_route(
            Route{":payment_id"}
                .get(
                    [this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    )
                    {
                        interfaces::io::ResponseSink sink{res, std::move(send)};

                        handle_payments_retrieve(req, res, std::move(send));
                    }
                )
                .summary("Hyperswitch payments_retrieve")
        );
    }

    // original install() route #65
    void install_route_65(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "payments"}.add_router(
            Router{router, ":payment_id"}.add_route(
                Route{"extended_card_info"}
                    .get(
                        [this](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
                            interfaces::io::ResponseSink sink{res, std::move(send)};

                            handle_retrieve_extended_card_info(req, res, std::move(send));
                        }
                    )
                    .summary("Hyperswitch retrieve_extended_card_info")
            )
        );
    }

    void install(core::router::RouterContext<>& router)
    {
        install_route_43(router);
        install_route_44(router);
        install_route_48(router);
        install_route_49(router);
        install_route_65(router);
    }

private:
    ::app::context::AppContext& m_context;
    PaymentsScopeState& m_scope;
    PaymentsInputSupport& m_input_support;
    PaymentsRuntime& m_runtime;
};

} // namespace app::handlers::payments
