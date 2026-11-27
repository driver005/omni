export module app_payment:payment_methods_handlers_payment_method_sessions_by_id_saved;

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

class PaymentMethodSessionSavedHandlers
{
public:
    void bind_request_support(PaymentMethodsRequestSupport& support) noexcept
    {
        m_request_support = &support;
    }

    PaymentMethodsRequestSupport* m_request_support{nullptr};

    PaymentMethodSessionSavedHandlers(
        ::app::context::AppContext& ctx,
        PaymentMethodsRuntime& runtime
    ) noexcept :
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
            .add_router(
                Router{router, "payment-method-sessions"}.add_route(
                    Route{":payment_method_session_id"}
                        .delt(
                            [this](
                                interfaces::io::IRequest& req,
                                interfaces::io::IResponse& res,
                                std::function<void()> send
                            )
                            {
        interfaces::io::ResponseSink sink{res, std::move(send)};

                                handle_payment_method_session_delete_saved_payment_method(
                                    req,
                                    res,
                                    std::move(send)
                                );
                            }
                        )
                        .summary("Hyperswitch payment_method_session_delete_saved_payment_method")
                )
            )
            .add_router(
                Router{router, "payment-method-sessions"}.add_router(
                    Router{router, ":payment_method_session_id"}.add_route(
                        Route{"list-payment-methods"}
                            .get(
                                [this](
                                    interfaces::io::IRequest& req,
                                    interfaces::io::IResponse& res,
                                    std::function<void()> send
                                )
                                {
        interfaces::io::ResponseSink sink{res, std::move(send)};

                                    handle_payment_method_session_list_payment_methods(
                                        req,
                                        res,
                                        std::move(send)
                                    );
                                }
                            )
                            .summary("Hyperswitch payment_method_session_list_payment_methods")
                    )
                )
            )
            .add_router(
                Router{router, "payment-method-sessions"}.add_router(
                    Router{router, ":payment_method_session_id"}.add_route(
                        Route{"update-saved-payment-method"}
                            .put(
                                [this](
                                    interfaces::io::IRequest& req,
                                    interfaces::io::IResponse& res,
                                    std::function<void()> send
                                )
                                {
        interfaces::io::ResponseSink sink{res, std::move(send)};

                                    handle_payment_method_session_update_saved_payment_method(
                                        req,
                                        res,
                                        std::move(send)
                                    );
                                }
                            )
                            .summary(
                                "Hyperswitch payment_method_session_update_saved_payment_method"
                            )
                    )
                )
            )
            .add_router(
                Router{router, ":id"}.add_route(
                    Route{"list-enabled-payment-methods"}
                        .get(
                            [this](
                                interfaces::io::IRequest& req,
                                interfaces::io::IResponse& res,
                                std::function<void()> send
                            )
                            {
        interfaces::io::ResponseSink sink{res, std::move(send)};

                                handle_payment_method_session_list_payment_methods(
                                    req,
                                    res,
                                    std::move(send)
                                );
                            }
                        )
                        .summary("Hyperswitch payment_method_session_list_payment_methods")
                )
            );

        Router{router, ":pm_prefix/payment-method-sessions"}
            .add_route(
                Route{":id"}
                    .delt(
                        [this](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
        interfaces::io::ResponseSink sink{res, std::move(send)};

                            handle_payment_method_session_delete_saved_payment_method(
                                req,
                                res,
                                std::move(send)
                            );
                        }
                    )
                    .summary(
                        "Hyperswitch payment_method_session_delete_saved_payment_method (v2 prefix)"
                    )
            )
            .add_router(
                Router{router, ":id"}.add_route(
                    Route{"list-payment-methods"}
                        .get(
                            [this](
                                interfaces::io::IRequest& req,
                                interfaces::io::IResponse& res,
                                std::function<void()> send
                            )
                            {
        interfaces::io::ResponseSink sink{res, std::move(send)};

                                handle_payment_method_session_list_payment_methods(
                                    req,
                                    res,
                                    std::move(send)
                                );
                            }
                        )
                        .summary(
                            "Hyperswitch payment_method_session_list_payment_methods (v2 prefix)"
                        )
                )
            )
            .add_router(
                Router{router, ":id"}.add_route(
                    Route{"update-saved-payment-method"}
                        .put(
                            [this](
                                interfaces::io::IRequest& req,
                                interfaces::io::IResponse& res,
                                std::function<void()> send
                            )
                            {
        interfaces::io::ResponseSink sink{res, std::move(send)};

                                handle_payment_method_session_update_saved_payment_method(
                                    req,
                                    res,
                                    std::move(send)
                                );
                            }
                        )
                        .summary(
                            "Hyperswitch payment_method_session_update_saved_payment_method (v2 "
                            "prefix)"
                        )
                )
            );

        Router{router, ":pm_prefix/payment-methods"}.add_router(
            Router{router, ":id"}.add_route(
                Route{"list-enabled-payment-methods"}
                    .get(
                        [this](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
        interfaces::io::ResponseSink sink{res, std::move(send)};

                            handle_payment_method_session_list_payment_methods(
                                req,
                                res,
                                std::move(send)
                            );
                        }
                    )
                    .summary("Hyperswitch payment_method_session_list_payment_methods (v2 prefix)")
            )
        );
    }

    void handle_payment_method_session_delete_saved_payment_method(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};

        const auto auth_error = m_request_support->authorize(
            req,
            "payment_method_session_delete_saved_payment_method",
            spec_for("payment_method_session_delete_saved_payment_method")
        );
        if (auth_error) {
            const auto& error = *auth_error;
            error.send(sink);
            return;
        }
        const std::string session_id = m_input_support.path_segment(req);
        if (session_id.empty()) {
            const auto error = m_input_support.error(
                core::error::AppErrorKind::INVALID_REQUEST,
                "missing session id"
            );
            error.send(sink);
            return;
        }
        const auto session = m_input_support.find_payment_method_session(session_id);
        if (!session) {
            const auto error = m_input_support.error(
                core::error::AppErrorKind::NOT_FOUND,
                "payment methods session does not exist or has expired"
            );
            error.send(sink);
            return;
        }
        const auto body = req.body_as_string();
        auto parsed = core::behavior::Serializer<::app::model::payment_methods::PaymentMethodSessionDeleteSavedPaymentMethod>::pass(req, sink);
        if (!parsed) return;
        const std::string token = parsed->get_payment_method_token();
        if (token.empty()) {
            const auto error = m_input_support.error(
                core::error::AppErrorKind::INVALID_REQUEST,
                "missing payment method token"
            );
            error.send(sink);
            return;
        }
        const auto record = m_input_support.find_payment_method(token);
        if (!record) {
            const auto error = m_input_support.error(
                core::error::AppErrorKind::NOT_FOUND,
                "Payment method does not exist in our records"
            );
            error.send(sink);
            return;
        }
        const auto& dto = token;
        auto result = m_runtime.payment_method_session_delete_saved_payment_method(dto, m_request_support->context(req));
        result.handle(sink);
    }

    void handle_payment_method_session_list_payment_methods(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};

        const auto auth_error = m_request_support->authorize(
            req,
            "payment_method_session_list_payment_methods",
            spec_for("payment_method_session_list_payment_methods")
        );
        if (auth_error) {
            const auto& error = *auth_error;
            error.send(sink);
            return;
        }
        const std::string session_id = m_input_support.path_segment(req, 1);
        if (session_id.empty()) {
            const auto error = m_input_support.error(
                core::error::AppErrorKind::INVALID_REQUEST,
                "missing session id"
            );
            error.send(sink);
            return;
        }
        const auto session = m_input_support.find_payment_method_session(session_id);
        if (!session) {
            const auto error = m_input_support.error(
                core::error::AppErrorKind::NOT_FOUND,
                "payment methods session does not exist or has expired"
            );
            error.send(sink);
            return;
        }
        const std::string customer_id =
            ::app::http::RequestQuery{}(req).get("customer_id").value_or("");
        const auto& dto = *session;
        auto result = m_runtime.payment_method_session_list_payment_methods(dto, m_request_support->context(req));
        result.handle(sink);
    }

    void handle_payment_method_session_update_saved_payment_method(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};

        const auto auth_error = m_request_support->authorize(
            req,
            "payment_method_session_update_saved_payment_method",
            spec_for("payment_method_session_update_saved_payment_method")
        );
        if (auth_error) {
            const auto& error = *auth_error;
            error.send(sink);
            return;
        }
        const std::string session_id = m_input_support.path_segment(req, 1);
        if (session_id.empty()) {
            const auto error = m_input_support.error(
                core::error::AppErrorKind::INVALID_REQUEST,
                "missing session id"
            );
            error.send(sink);
            return;
        }
        const auto session = m_input_support.find_payment_method_session(session_id);
        if (!session) {
            const auto error = m_input_support.error(
                core::error::AppErrorKind::NOT_FOUND,
                "payment methods session does not exist or has expired"
            );
            error.send(sink);
            return;
        }
        const auto body = req.body_as_string();
        if (body.empty()) {
            const auto error =
                m_input_support.error(core::error::AppErrorKind::EMPTY_REQUEST, "empty request");
            error.send(sink);
            return;
        }
        auto parsed = core::behavior::Serializer<::app::model::payment_methods::PaymentMethodSessionUpdateSavedPaymentMethod>::pass(req, sink);
        if (!parsed) return;
        const std::string token = parsed->get_payment_method_token().value_or("");
        if (token.empty()) {
            const auto error = m_input_support.error(
                core::error::AppErrorKind::INVALID_REQUEST,
                "missing payment method token"
            );
            error.send(sink);
            return;
        }
        auto record = m_input_support.find_payment_method(token);
        if (!record) {
            for (auto& candidate: m_context.repository().list_payment_methods()) {
                if (candidate.get_vault_id() == token) {
                    record = candidate;
                    break;
                }
            }
        }
        if (!record) {
            const auto error = m_input_support.error(
                core::error::AppErrorKind::NOT_FOUND,
                "Payment method does not exist in our records"
            );
            error.send(sink);
            return;
        }
        const auto& dto = *parsed;
        auto result = m_runtime.payment_method_session_update_saved_payment_method(dto, m_request_support->context(req));
        result.handle(sink);
    }

private:
    ::app::context::AppContext& m_context;
    PaymentMethodsInputSupport m_input_support;
    PaymentMethodsRuntime& m_runtime;
};

} // namespace app::handlers::payment_methods
