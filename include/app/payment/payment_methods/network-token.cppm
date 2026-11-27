export module app_payment:payment_methods_handlers_network_token;

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
import app_http;
import app_crypto;
import app_metrics;
import app_store;
import app_connector;
import app_events;
import app_workflow;
import app_keystore;

import :payment_methods_input;
import :payment_methods_result;
import :payment_methods_runtime;

export namespace app::handlers::payment_methods {

class NetworkTokenHandlers
{
public:
    void bind_request_support(PaymentMethodsRequestSupport& support) noexcept
    {
        m_request_support = &support;
    }

    PaymentMethodsRequestSupport* m_request_support{nullptr};

    NetworkTokenHandlers(::app::context::AppContext& ctx, PaymentMethodsRuntime& runtime) noexcept :
        m_context{ctx},
        m_input_support{ctx},
        m_runtime{runtime}
    {
    }

    void install(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "payment_methods"}
            .add_route(
                Route{"get-network-token-eligibility"}
                    .get(
                        [this](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
        interfaces::io::ResponseSink sink{res, std::move(send)};

                            handle_get_pm_nt_eligibility_api(req, res, std::move(send));
                        }
                    )
                    .summary("Hyperswitch get_pm_nt_eligibility_api")
            )
            .add_router(
                Router{router, "token"}.add_router(
                    Router{router, ":payment_method_temporary_token"}.add_route(
                        Route{"details"}
                            .get(
                                [this](
                                    interfaces::io::IRequest& req,
                                    interfaces::io::IResponse& res,
                                    std::function<void()> send
                                )
                                {
        interfaces::io::ResponseSink sink{res, std::move(send)};

                                    handle_payment_method_get_token_details_api(
                                        req,
                                        res,
                                        std::move(send)
                                    );
                                }
                            )
                            .summary("Hyperswitch payment_method_get_token_details_api")
                    )
                )
            )
            .add_router(
                Router{router, ":payment_method_id"}.add_route(
                    Route{"check-network-token-status"}
                        .get(
                            [this](
                                interfaces::io::IRequest& req,
                                interfaces::io::IResponse& res,
                                std::function<void()> send
                            )
                            {
        interfaces::io::ResponseSink sink{res, std::move(send)};

                                handle_network_token_status_check_api(req, res, std::move(send));
                            }
                        )
                        .summary("Hyperswitch network_token_status_check_api")
                )
            )
            .add_router(
                Router{router, ":id"}.add_route(
                    Route{"get-token"}
                        .get(
                            [this](
                                interfaces::io::IRequest& req,
                                interfaces::io::IResponse& res,
                                std::function<void()> send
                            )
                            {
        interfaces::io::ResponseSink sink{res, std::move(send)};

                                handle_get_payment_method_token_data(req, res, std::move(send));
                            }
                        )
                        .summary("Hyperswitch get_payment_method_token_data")
                )
            );

        Router{router, ":pm_prefix/payment-methods"}
            .add_router(
                Router{router, ":id"}.add_route(
                    Route{"get-token"}
                        .get(
                            [this](
                                interfaces::io::IRequest& req,
                                interfaces::io::IResponse& res,
                                std::function<void()> send
                            )
                            {
        interfaces::io::ResponseSink sink{res, std::move(send)};

                                handle_get_payment_method_token_data(req, res, std::move(send));
                            }
                        )
                        .summary("Hyperswitch get_payment_method_token_data (v2 prefix)")
                )
            )
            .add_router(
                Router{router, ":id"}.add_route(
                    Route{"check-network-token-status"}
                        .get(
                            [this](
                                interfaces::io::IRequest& req,
                                interfaces::io::IResponse& res,
                                std::function<void()> send
                            )
                            {
        interfaces::io::ResponseSink sink{res, std::move(send)};

                                handle_network_token_status_check_api(req, res, std::move(send));
                            }
                        )
                        .summary("Hyperswitch network_token_status_check_api (v2 prefix)")
                )
            );
    }

    void handle_get_pm_nt_eligibility_api(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};

        const auto auth_error = m_request_support->authorize(
            req,
            "get_pm_nt_eligibility_api",
            spec_for("get_pm_nt_eligibility_api")
        );
        if (auth_error) {
            const auto& error = *auth_error;
            error.send(sink);
            return;
        }
        const auto query = ::app::http::RequestQuery{}(req);
        const auto card_bin = query.get("card_bin").value_or("");
        const std::string org_id = ::app::http::org_id_from_header{}(req);
        if (card_bin.empty()) {
            const auto error = m_input_support.error(
                core::error::AppErrorKind::INVALID_REQUEST,
                "missing card_bin"
            );
            error.send(sink);
            return;
        }
        auto result = m_runtime.get_pm_nt_eligibility_api(m_request_support->context(req));
        result.handle(sink);
    }

    void handle_network_token_status_check_api(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};

        const auto auth_error = m_request_support->authorize(
            req,
            "network_token_status_check_api",
            spec_for("network_token_status_check_api")
        );
        if (auth_error) {
            const auto& error = *auth_error;
            error.send(sink);
            return;
        }
        const std::string payment_method_id = m_input_support.path_segment(req, 1);
        if (payment_method_id.empty()) {
            const auto error = m_input_support.error(
                core::error::AppErrorKind::INVALID_REQUEST,
                "missing payment method id"
            );
            error.send(sink);
            return;
        }
        const auto record = m_input_support.find_payment_method(payment_method_id);
        if (!record) {
            const auto error = m_input_support.error(
                core::error::AppErrorKind::NOT_FOUND,
                "Payment method does not exist in our records"
            );
            error.send(sink);
            return;
        }
        const auto& dto = *record;
        auto result = m_runtime.network_token_status_check_api(dto, m_request_support->context(req));
        result.handle(sink);
    }

    void handle_payment_method_get_token_details_api(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};

        const auto auth_error = m_request_support->authorize(
            req,
            "payment_method_get_token_details_api",
            spec_for("payment_method_get_token_details_api")
        );
        if (auth_error) {
            const auto& error = *auth_error;
            error.send(sink);
            return;
        }
        const std::string temporary_token = m_input_support.path_segment(req, 1);
        if (temporary_token.empty()) {
            const auto error =
                m_input_support.error(core::error::AppErrorKind::INVALID_REQUEST, "missing token");
            error.send(sink);
            return;
        }
        auto payment_method_id = m_input_support.token_payment_method(temporary_token);
        if (!payment_method_id) {
            const auto& error = payment_method_id.error();
            error.send(sink);
            return;
        }
        auto result = m_runtime.payment_method_get_token_details_api(m_request_support->context(req));
        result.handle(sink);
    }

    void handle_get_payment_method_token_data(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};

        const auto auth_error = m_request_support->authorize(
            req,
            "get_payment_method_token_data",
            spec_for("get_payment_method_token_data")
        );
        if (auth_error) {
            const auto& error = *auth_error;
            error.send(sink);
            return;
        }
        const std::string payment_method_id = m_input_support.path_segment(req, 1);
        if (payment_method_id.empty()) {
            const auto error = m_input_support.error(
                core::error::AppErrorKind::INVALID_REQUEST,
                "missing payment method id"
            );
            error.send(sink);
            return;
        }
        const auto record = m_input_support.find_payment_method(payment_method_id);
        if (!record) {
            const auto error = m_input_support.error(
                core::error::AppErrorKind::NOT_FOUND,
                "Payment method does not exist in our records"
            );
            error.send(sink);
            return;
        }
        const std::string token_type =
            ::app::http::RequestQuery{}(req).get("token_type").value_or("network_token");
        const auto& dto = *record;
        auto result = m_runtime.get_payment_method_token_data(dto, m_request_support->context(req));
        result.handle(sink);
    }

private:
    ::app::context::AppContext& m_context;
    PaymentMethodsInputSupport m_input_support;
    PaymentMethodsRuntime& m_runtime;
};

} // namespace app::handlers::payment_methods
