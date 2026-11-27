export module app_payment:payments_handlers_v2_sessions;

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

// v2 sessions handlers. Owns its add_route registrations; the version facade calls
// install_route_NN in the original registration order.
class PaymentsV2SessionsHandlers
{
public:
    void bind_request_support(PaymentsRequestSupport& support) noexcept
    {
        m_request_support = &support;
    }

    PaymentsRequestSupport* m_request_support{nullptr};

    PaymentsV2SessionsHandlers(
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

    void handle_payments_connector_session(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};

        if (const auto auth_error = m_request_support->authorize(
                req,
                "payments_connector_session",
                spec_for("payments_connector_session")
            )) {
            auth_error->send(sink);
            return;
        }
        m_context.logger().record_flow("payments.payments_connector_session");
        m_context.logger().counter("payments.payments_connector_session").increment();

        auto parsed_dto =
            core::behavior::Serializer<::app::model::payments::PaymentsRequest>::pass(req, sink);
        if (!parsed_dto) {
            return;
        }
        const auto& dto = *parsed_dto;
        m_runtime.payments_connector_session(dto, m_request_support->context(req)).handle(sink);
    }

    void handle_payments_apply_pm_data(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};

        if (const auto auth_error = m_request_support->authorize(
                req,
                "payments_apply_pm_data",
                spec_for("payments_apply_pm_data")
            )) {
            auth_error->send(sink);
            return;
        }
        m_context.logger().record_flow("payments.payments_apply_pm_data");
        m_context.logger().counter("payments.payments_apply_pm_data").increment();

        auto parsed_dto =
            core::behavior::Serializer<::app::model::payments::PaymentsRequest>::pass(req, sink);
        if (!parsed_dto) {
            return;
        }
        const auto& dto = *parsed_dto;
        m_runtime.payments_apply_pm_data(dto, m_request_support->context(req)).handle(sink);
    }

    void handle_payments_finish_redirection(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};

        if (const auto auth_error = m_request_support->authorize(
                req,
                "payments_finish_redirection",
                spec_for("payments_finish_redirection")
            )) {
            auth_error->send(sink);
            return;
        }
        m_context.logger().record_flow("payments.payments_finish_redirection");
        m_context.logger().counter("payments.payments_finish_redirection").increment();

        const auto body = m_input_support.path_body(
            {{"payment_id", m_input_support.payment_id_from_path(req)},
             {"profile_id", m_input_support.last_path_segment(req)}}
        );
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
        m_runtime.payments_finish_redirection(dto, m_request_support->context(req)).handle(sink);
    }

    void handle_payments_start_redirection(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};

        if (const auto auth_error = m_request_support->authorize(
                req,
                "payments_start_redirection",
                spec_for("payments_start_redirection")
            )) {
            auth_error->send(sink);
            return;
        }
        m_context.logger().record_flow("payments.payments_start_redirection");
        m_context.logger().counter("payments.payments_start_redirection").increment();

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
        m_runtime.payments_start_redirection(dto, m_request_support->context(req)).handle(sink);
    }

    // original install() route #19
    void install_route_19(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "v2/payments"}.add_router(
            Router{router, ":payment_id"}.add_route(
                Route{"create-external-sdk-tokens"}
                    .post(
                        [this](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
                            interfaces::io::ResponseSink sink{res, std::move(send)};

                            handle_payments_connector_session(req, res, std::move(send));
                        }
                    )
                    .summary("Hyperswitch payments_connector_session")
            )
        );
    }

    // original install() route #22
    void install_route_22(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "v2/payments"}.add_router(
            Router{router, ":payment_id"}.add_route(
                Route{"start-redirection"}
                    .get(
                        [this](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
                            interfaces::io::ResponseSink sink{res, std::move(send)};

                            handle_payments_start_redirection(req, res, std::move(send));
                        }
                    )
                    .summary("Hyperswitch payments_start_redirection")
            )
        );
    }

    // original install() route #24
    void install_route_24(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "v2/payments"}.add_router(
            Router{router, ":payment_id"}.add_router(
                Router{router, "eligibility"}.add_route(
                    Route{"check-balance-and-apply-pm-data"}
                        .post(
                            [this](
                                interfaces::io::IRequest& req,
                                interfaces::io::IResponse& res,
                                std::function<void()> send
                            )
                            {
                                interfaces::io::ResponseSink sink{res, std::move(send)};

                                handle_payments_apply_pm_data(req, res, std::move(send));
                            }
                        )
                        .summary("Hyperswitch payments_apply_pm_data")
                )
            )
        );
    }

    // original install() route #25
    void install_route_25(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "v2/payments"}.add_router(
            Router{router, ":payment_id"}.add_router(
                Router{router, "finish-redirection"}.add_router(
                    Router{router, ":publishable_key"}.add_route(
                        Route{":profile_id"}
                            .get(
                                [this](
                                    interfaces::io::IRequest& req,
                                    interfaces::io::IResponse& res,
                                    std::function<void()> send
                                )
                                {
                                    interfaces::io::ResponseSink sink{res, std::move(send)};

                                    handle_payments_finish_redirection(req, res, std::move(send));
                                }
                            )
                            .summary("Hyperswitch payments_finish_redirection")
                    )
                )
            )
        );
    }

    void install(core::router::RouterContext<>& router)
    {
        install_route_19(router);
        install_route_22(router);
        install_route_24(router);
        install_route_25(router);
    }

private:
    ::app::context::AppContext& m_context;
    PaymentsScopeState& m_scope;
    PaymentsInputSupport& m_input_support;
    PaymentsRuntime& m_runtime;
};

} // namespace app::handlers::payments
