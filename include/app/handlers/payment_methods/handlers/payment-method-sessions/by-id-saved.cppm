export module app_handlers_payment_methods:handlers_payment_method_sessions_by_id_saved;

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
import app_http;
import app_crypto;
import app_metrics;
import app_store;
import app_connector;
import app_events;
import app_workflow;
import app_keystore;

import :input;
import :result;
import :runtime;

export namespace app::handlers::payment_methods {

class PaymentMethodSessionSavedHandlers
{
public:
    PaymentMethodSessionSavedHandlers(
        app::context::AppContext& ctx,
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
        const auto auth_error =
            m_input_support.authorize(req, "payment_method_session_delete_saved_payment_method");
        if (auth_error) {
            const auto& error = *auth_error;
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }
        const std::string session_id = m_input_support.path_segment(req);
        if (session_id.empty()) {
            const auto error = m_input_support.error(
                core::error::AppErrorKind::INVALID_REQUEST,
                "missing session id"
            );
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }
        const auto session = m_input_support.find_payment_method_session(session_id);
        if (!session) {
            const auto error = m_input_support.error(
                core::error::AppErrorKind::NOT_FOUND,
                "payment methods session does not exist or has expired"
            );
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }
        const auto body = req.body_as_string();
        using Input = app::model::payment_methods::PaymentMethodSessionDeleteSavedPaymentMethod;
        auto parsed = serde::Ser::deserialize<Input>("application/json", body);
        if (!parsed) {
            const auto error =
                m_input_support.error(core::error::AppErrorKind::INVALID_JSON, "invalid JSON");
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }
        const std::string token = parsed->get_payment_method_token();
        if (token.empty()) {
            const auto error = m_input_support.error(
                core::error::AppErrorKind::INVALID_REQUEST,
                "missing payment method token"
            );
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }
        const auto record = m_input_support.find_payment_method(token);
        if (!record) {
            const auto error = m_input_support.error(
                core::error::AppErrorKind::NOT_FOUND,
                "Payment method does not exist in our records"
            );
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }
        PaymentMethodSessionDeleteSavedPaymentMethodInput input{token, *session, *record};
        auto result = m_runtime.payment_method_session_delete_saved_payment_method(input);
        if (!result) {
            const auto& error = result.error();
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }
        res.set_status(interfaces::io::types::Status::OK);
        res.set_body(serde::Ser::serialize("application/json", *result));
        send();
    }

    void handle_payment_method_session_list_payment_methods(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto auth_error =
            m_input_support.authorize(req, "payment_method_session_list_payment_methods");
        if (auth_error) {
            const auto& error = *auth_error;
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }
        const std::string session_id = m_input_support.path_segment(req, 1);
        if (session_id.empty()) {
            const auto error = m_input_support.error(
                core::error::AppErrorKind::INVALID_REQUEST,
                "missing session id"
            );
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }
        const auto session = m_input_support.find_payment_method_session(session_id);
        if (!session) {
            const auto error = m_input_support.error(
                core::error::AppErrorKind::NOT_FOUND,
                "payment methods session does not exist or has expired"
            );
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }
        const std::string customer_id =
            app::http::RequestQuery{}(req).get("customer_id").value_or("");
        PaymentMethodSessionListPaymentMethodsInput input{*session, customer_id};
        auto result = m_runtime.payment_method_session_list_payment_methods(input);
        if (!result) {
            const auto& error = result.error();
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }
        res.set_status(interfaces::io::types::Status::OK);
        res.set_body(serde::Ser::serialize("application/json", *result));
        send();
    }

    void handle_payment_method_session_update_saved_payment_method(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto auth_error =
            m_input_support.authorize(req, "payment_method_session_update_saved_payment_method");
        if (auth_error) {
            const auto& error = *auth_error;
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }
        const std::string session_id = m_input_support.path_segment(req, 1);
        if (session_id.empty()) {
            const auto error = m_input_support.error(
                core::error::AppErrorKind::INVALID_REQUEST,
                "missing session id"
            );
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }
        const auto session = m_input_support.find_payment_method_session(session_id);
        if (!session) {
            const auto error = m_input_support.error(
                core::error::AppErrorKind::NOT_FOUND,
                "payment methods session does not exist or has expired"
            );
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }
        const auto body = req.body_as_string();
        if (body.empty()) {
            const auto error =
                m_input_support.error(core::error::AppErrorKind::EMPTY_REQUEST, "empty request");
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }
        using Input = app::model::payment_methods::PaymentMethodSessionUpdateSavedPaymentMethod;
        auto parsed = serde::Ser::deserialize<Input>("application/json", body);
        if (!parsed) {
            const auto error =
                m_input_support.error(core::error::AppErrorKind::INVALID_JSON, "invalid JSON");
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }
        const std::string token = parsed->get_payment_method_token().value_or("");
        if (token.empty()) {
            const auto error = m_input_support.error(
                core::error::AppErrorKind::INVALID_REQUEST,
                "missing payment method token"
            );
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
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
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }
        PaymentMethodSessionUpdateSavedPaymentMethodInput input{
            *parsed,
            *session,
            std::move(*record)
        };
        auto result = m_runtime.payment_method_session_update_saved_payment_method(input);
        if (!result) {
            const auto& error = result.error();
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }
        res.set_status(interfaces::io::types::Status::OK);
        res.set_body(serde::Ser::serialize("application/json", *result));
        send();
    }

private:
    app::context::AppContext& m_context;
    PaymentMethodsInputSupport m_input_support;
    PaymentMethodsRuntime& m_runtime;
};

} // namespace app::handlers::payment_methods
