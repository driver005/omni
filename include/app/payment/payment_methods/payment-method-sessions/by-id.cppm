export module app_payment:payment_methods_handlers_payment_method_sessions_by_id;

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

import :payment_methods_input;
import :payment_methods_result;
import :payment_methods_runtime;

export namespace app::handlers::payment_methods {

class PaymentMethodSessionByIdHandlers
{
public:
    PaymentMethodSessionByIdHandlers(
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
                        .get(
                            [this](
                                interfaces::io::IRequest& req,
                                interfaces::io::IResponse& res,
                                std::function<void()> send
                            )
                            {
                                handle_payment_methods_session_retrieve(req, res, std::move(send));
                            }
                        )
                        .summary("Hyperswitch payment_methods_session_retrieve")
                )
            )
            .add_router(
                Router{router, "payment-method-sessions"}.add_route(
                    Route{":payment_method_session_id"}
                        .put(
                            [this](
                                interfaces::io::IRequest& req,
                                interfaces::io::IResponse& res,
                                std::function<void()> send
                            )
                            {
                                handle_payment_methods_session_update(req, res, std::move(send));
                            }
                        )
                        .summary("Hyperswitch payment_methods_session_update")
                )
            )
            .add_router(
                Router{router, "payment-method-sessions"}.add_router(
                    Router{router, ":payment_method_session_id"}.add_route(
                        Route{"confirm"}
                            .post(
                                [this](
                                    interfaces::io::IRequest& req,
                                    interfaces::io::IResponse& res,
                                    std::function<void()> send
                                )
                                {
                                    handle_payment_method_session_confirm(
                                        req,
                                        res,
                                        std::move(send)
                                    );
                                }
                            )
                            .summary("Hyperswitch payment_method_session_confirm")
                    )
                )
            );

        Router{router, ":pm_prefix/payment-method-sessions"}
            .add_route(
                Route{":id"}
                    .get(
                        [this](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
                            handle_payment_methods_session_retrieve(req, res, std::move(send));
                        }
                    )
                    .summary("Hyperswitch payment_methods_session_retrieve (v2 prefix)")
            )
            .add_route(
                Route{":id"}
                    .put(
                        [this](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
                            handle_payment_methods_session_update(req, res, std::move(send));
                        }
                    )
                    .summary("Hyperswitch payment_methods_session_update (v2 prefix)")
            )
            .add_router(
                Router{router, ":id"}.add_route(
                    Route{"confirm"}
                        .post(
                            [this](
                                interfaces::io::IRequest& req,
                                interfaces::io::IResponse& res,
                                std::function<void()> send
                            )
                            {
                                handle_payment_method_session_confirm(req, res, std::move(send));
                            }
                        )
                        .summary("Hyperswitch payment_method_session_confirm (v2 prefix)")
                )
            );
    }

    void handle_payment_methods_session_retrieve(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto auth_error = m_input_support.authorize(req, "payment_methods_session_retrieve");
        if (auth_error) {
            const auto& error = *auth_error;
            error.send(res, send);
            return;
        }
        const std::string session_id = m_input_support.path_segment(req);
        if (session_id.empty()) {
            const auto error = m_input_support.error(
                core::error::AppErrorKind::INVALID_REQUEST,
                "missing session id"
            );
            error.send(res, send);
            return;
        }
        const auto record = m_input_support.find_payment_method_session(session_id);
        if (!record) {
            const auto error = m_input_support.error(
                core::error::AppErrorKind::NOT_FOUND,
                "payment methods session does not exist or has expired"
            );
            error.send(res, send);
            return;
        }
        const auto presented = ::app::http::RequestQuery{}(req).get("client_secret");
        if (presented) {
            if (const auto e = m_input_support.check_client_secret(session_id, *presented)) {
                const auto& error = *e;
                error.send(res, send);
                return;
            }
        }
        PaymentMethodsSessionRetrieveInput input{session_id, *record, presented};
        auto result = m_runtime.payment_methods_session_retrieve(input);
        if (!result) {
            const auto& error = result.error();
            error.send(res, send);
            return;
        }
        res.set_status(interfaces::io::types::Status::OK);
        res.set_body(serde::Ser::serialize("application/json", *result));
        send();
    }

    void handle_payment_methods_session_update(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto auth_error = m_input_support.authorize(req, "payment_methods_session_update");
        if (auth_error) {
            const auto& error = *auth_error;
            error.send(res, send);
            return;
        }
        const std::string session_id = m_input_support.path_segment(req);
        if (session_id.empty()) {
            const auto error = m_input_support.error(
                core::error::AppErrorKind::INVALID_REQUEST,
                "missing session id"
            );
            error.send(res, send);
            return;
        }
        auto record = m_input_support.find_payment_method_session(session_id);
        if (!record) {
            const auto error = m_input_support.error(
                core::error::AppErrorKind::NOT_FOUND,
                "payment methods session does not exist or has expired"
            );
            error.send(res, send);
            return;
        }
        const auto body = req.body_as_string();
        if (body.empty()) {
            const auto error =
                m_input_support.error(core::error::AppErrorKind::EMPTY_REQUEST, "empty request");
            error.send(res, send);
            return;
        }
        using Input = ::app::model::payment_methods::PaymentMethodsSessionUpdateRequest;
        auto parsed = serde::Ser::deserialize<Input>("application/json", body);
        if (!parsed) {
            const auto error =
                m_input_support.error(core::error::AppErrorKind::INVALID_JSON, "invalid JSON");
            error.send(res, send);
            return;
        }
        PaymentMethodsSessionUpdateInput input{*parsed, session_id, std::move(*record)};
        auto result = m_runtime.payment_methods_session_update(input);
        if (!result) {
            const auto& error = result.error();
            error.send(res, send);
            return;
        }
        res.set_status(interfaces::io::types::Status::OK);
        res.set_body(serde::Ser::serialize("application/json", *result));
        send();
    }

    void handle_payment_method_session_confirm(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto auth_error = m_input_support.authorize(req, "payment_method_session_confirm");
        if (auth_error) {
            const auto& error = *auth_error;
            error.send(res, send);
            return;
        }
        const std::string session_id = m_input_support.path_segment(req, 1);
        if (session_id.empty()) {
            const auto error = m_input_support.error(
                core::error::AppErrorKind::INVALID_REQUEST,
                "missing session id"
            );
            error.send(res, send);
            return;
        }
        auto session = m_input_support.find_payment_method_session(session_id);
        if (!session) {
            const auto error = m_input_support.error(
                core::error::AppErrorKind::NOT_FOUND,
                "payment methods session does not exist or has expired"
            );
            error.send(res, send);
            return;
        }
        if (session->get_status() != "active") {
            const auto error = m_input_support.error(
                core::error::AppErrorKind::PRECONDITION_FAILED,
                "session is not active"
            );
            error.send(res, send);
            return;
        }
        const auto body = req.body_as_string();
        if (body.empty()) {
            const auto error =
                m_input_support.error(core::error::AppErrorKind::EMPTY_REQUEST, "empty request");
            error.send(res, send);
            return;
        }
        using Input = ::app::model::payment_methods::PaymentMethodSessionConfirmRequest;
        auto parsed = serde::Ser::deserialize<Input>("application/json", body);
        if (!parsed) {
            const auto error =
                m_input_support.error(core::error::AppErrorKind::INVALID_JSON, "invalid JSON");
            error.send(res, send);
            return;
        }
        const auto presented = ::app::http::RequestQuery{}(req).get("client_secret");
        if (presented) {
            if (const auto e = m_input_support.check_client_secret(session_id, *presented)) {
                const auto& error = *e;
                error.send(res, send);
                return;
            }
        }
        const auto payload = serde::Ser::encode_json(parsed->get_payment_method_data());
        if (payload.empty() || payload == "null") {
            const auto error = m_input_support.error(
                core::error::AppErrorKind::INVALID_REQUEST,
                "Missing required field: payment_method_data"
            );
            error.send(res, send);
            return;
        }
        PaymentMethodSessionConfirmInput
            input{*parsed, std::move(*session), session_id, body, payload, presented};
        auto result = m_runtime.payment_method_session_confirm(input);
        if (!result) {
            const auto& error = result.error();
            error.send(res, send);
            return;
        }
        res.set_status(interfaces::io::types::Status::OK);
        res.set_body(serde::Ser::serialize("application/json", *result));
        send();
    }

private:
    ::app::context::AppContext& m_context;
    PaymentMethodsInputSupport m_input_support;
    PaymentMethodsRuntime& m_runtime;
};

} // namespace app::handlers::payment_methods
