export module app_payment:payments_handlers_v1_misc;

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

// v1 misc handlers. Owns its add_route registrations; the version facade calls
// install_route_NN in the original registration order.
class PaymentsV1MiscHandlers
{
public:
    void bind_request_support(PaymentsRequestSupport& support) noexcept
    {
        m_request_support = &support;
    }

    PaymentsRequestSupport* m_request_support{nullptr};

    PaymentsV1MiscHandlers(
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

    void handle_list_payment_methods_for_payments_client(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};

        if (const auto auth_error = m_request_support->authorize(
                req,
                "list_payment_methods_for_payments_client",
                extractor_only_spec_for("list_payment_methods_for_payments_client")
            )) {
            auth_error->send(sink);
            return;
        }
        m_context.logger().record_flow("payments.list_payment_methods_for_payments_client");
        m_context.logger().counter("payments.list_payment_methods_for_payments_client").increment();

        auto parsed_dto =
            core::behavior::Serializer<::app::model::payments::PaymentsRequest>::pass(req, sink);
        if (!parsed_dto) {
            return;
        }
        const auto& dto = *parsed_dto;
        m_runtime.list_payment_methods_for_payments_client(dto, m_request_support->context(req))
            .handle(sink);
    }

    void handle_payments_dynamic_tax_calculation(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};

        if (const auto auth_error = m_request_support->authorize(
                req,
                "payments_dynamic_tax_calculation",
                spec_for("payments_dynamic_tax_calculation")
            )) {
            auth_error->send(sink);
            return;
        }
        m_context.logger().record_flow("payments.payments_dynamic_tax_calculation");
        m_context.logger().counter("payments.payments_dynamic_tax_calculation").increment();

        auto parsed_dto =
            core::behavior::Serializer<::app::model::payments::PaymentsRequest>::pass(req, sink);
        if (!parsed_dto) {
            return;
        }
        const auto& dto = *parsed_dto;
        m_runtime.payments_dynamic_tax_calculation(dto, m_request_support->context(req))
            .handle(sink);
    }

    // original install() route #40
    void install_route_40(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "payments"}.add_router(
            Router{router, ":payment_id"}.add_route(
                Route{"client"}
                    .get(
                        [this](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
                            interfaces::io::ResponseSink sink{res, std::move(send)};

                            handle_list_payment_methods_for_payments_client(
                                req,
                                res,
                                std::move(send)
                            );
                        }
                    )
                    .summary("Hyperswitch list_payment_methods_for_payments_client")
            )
        );
    }

    // original install() route #66
    void install_route_66(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "payments"}.add_router(
            Router{router, ":payment_id"}.add_route(
                Route{"calculate_tax"}
                    .post(
                        [this](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
                            interfaces::io::ResponseSink sink{res, std::move(send)};

                            handle_payments_dynamic_tax_calculation(req, res, std::move(send));
                        }
                    )
                    .summary("Hyperswitch payments_dynamic_tax_calculation")
            )
        );
    }

    void install(core::router::RouterContext<>& router)
    {
        install_route_40(router);
        install_route_66(router);
    }

private:
    ::app::context::AppContext& m_context;
    PaymentsScopeState& m_scope;
    PaymentsInputSupport& m_input_support;
    PaymentsRuntime& m_runtime;
};

} // namespace app::handlers::payments
