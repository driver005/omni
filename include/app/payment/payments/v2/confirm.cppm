export module app_payment:payments_handlers_v2_confirm;

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

// v2 confirm handlers. Owns its add_route registrations; the version facade calls
// install_route_NN in the original registration order.
class PaymentsV2ConfirmHandlers
{
public:
    void bind_request_support(PaymentsRequestSupport& support) noexcept
    {
        m_request_support = &support;
    }

    PaymentsRequestSupport* m_request_support{nullptr};

    PaymentsV2ConfirmHandlers(
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

    void handle_payment_confirm_intent(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};

        if (const auto auth_error = m_request_support->authorize(
                req,
                "payment_confirm_intent",
                spec_for("payment_confirm_intent")
            )) {
            auth_error->send(sink);
            return;
        }
        m_context.logger().record_flow("payments.payment_confirm_intent");
        m_context.logger().counter("payments.payment_confirm_intent").increment();

        auto parsed_dto =
            core::behavior::Serializer<::app::model::payments::PaymentsRequest>::pass(req, sink);
        if (!parsed_dto) {
            return;
        }
        const auto& dto = *parsed_dto;
        m_runtime.payment_confirm_intent(dto, m_request_support->context(req)).handle(sink);
    }

    void handle_proxy_confirm_intent(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};

        if (const auto auth_error =
                m_request_support
                    ->authorize(req, "proxy_confirm_intent", spec_for("proxy_confirm_intent"))) {
            auth_error->send(sink);
            return;
        }
        m_context.logger().record_flow("payments.proxy_confirm_intent");
        m_context.logger().counter("payments.proxy_confirm_intent").increment();

        auto parsed_dto =
            core::behavior::Serializer<::app::model::payments::PaymentsRequest>::pass(req, sink);
        if (!parsed_dto) {
            return;
        }
        const auto& dto = *parsed_dto;
        m_runtime.proxy_confirm_intent(dto, m_request_support->context(req)).handle(sink);
    }

    void handle_confirm_intent_with_external_vault_proxy(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};

        if (const auto auth_error = m_request_support->authorize(
                req,
                "confirm_intent_with_external_vault_proxy",
                spec_for("confirm_intent_with_external_vault_proxy")
            )) {
            auth_error->send(sink);
            return;
        }
        m_context.logger().record_flow("payments.confirm_intent_with_external_vault_proxy");
        m_context.logger().counter("payments.confirm_intent_with_external_vault_proxy").increment();

        auto parsed_dto =
            core::behavior::Serializer<::app::model::payments::PaymentsRequest>::pass(req, sink);
        if (!parsed_dto) {
            return;
        }
        const auto& dto = *parsed_dto;
        m_runtime.confirm_intent_with_external_vault_proxy(dto, m_request_support->context(req))
            .handle(sink);
    }

    // original install() route #11
    void install_route_11(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "v2/payments"}.add_router(
            Router{router, ":payment_id"}.add_route(
                Route{"confirm-intent"}
                    .post(
                        [this](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
                            interfaces::io::ResponseSink sink{res, std::move(send)};

                            handle_payment_confirm_intent(req, res, std::move(send));
                        }
                    )
                    .summary("Hyperswitch payment_confirm_intent")
            )
        );
    }

    // original install() route #14
    void install_route_14(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "v2/payments"}.add_router(
            Router{router, ":payment_id"}.add_route(
                Route{"proxy-confirm-intent"}
                    .post(
                        [this](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
                            interfaces::io::ResponseSink sink{res, std::move(send)};

                            handle_proxy_confirm_intent(req, res, std::move(send));
                        }
                    )
                    .summary("Hyperswitch proxy_confirm_intent")
            )
        );
    }

    // original install() route #15
    void install_route_15(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "v2/payments"}.add_router(
            Router{router, ":payment_id"}.add_router(
                Router{router, "confirm-intent"}.add_route(
                    Route{"external-vault-proxy"}
                        .post(
                            [this](
                                interfaces::io::IRequest& req,
                                interfaces::io::IResponse& res,
                                std::function<void()> send
                            )
                            {
                                interfaces::io::ResponseSink sink{res, std::move(send)};

                                handle_confirm_intent_with_external_vault_proxy(
                                    req,
                                    res,
                                    std::move(send)
                                );
                            }
                        )
                        .summary("Hyperswitch confirm_intent_with_external_vault_proxy")
                )
            )
        );
    }

    void install(core::router::RouterContext<>& router)
    {
        install_route_11(router);
        install_route_14(router);
        install_route_15(router);
    }

private:
    ::app::context::AppContext& m_context;
    PaymentsScopeState& m_scope;
    PaymentsInputSupport& m_input_support;
    PaymentsRuntime& m_runtime;
};

} // namespace app::handlers::payments
