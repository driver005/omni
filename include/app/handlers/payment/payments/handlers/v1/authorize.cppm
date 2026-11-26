export module app_payment:payments_handlers_v1_authorize;

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

// v1 authorize handlers. Owns its add_route registrations; the version facade calls
// install_route_NN in the original registration order.
class PaymentsV1AuthorizeHandlers
{
public:
    PaymentsV1AuthorizeHandlers(
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

    void handle_payments_complete_authorize(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto auth_error = m_input_support.authorize(req, "payments_complete_authorize")) {
            res.set_status(auth_error->get_status());
            res.set_body(serde::Ser::serialize("application/json", *auth_error));
            send();
            return;
        }
        m_context.metrics().record_flow("payments.payments_complete_authorize");
        m_context.metrics().counter("payments.payments_complete_authorize").increment();

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
        using Input = ::app::model::payments::PaymentsCompleteAuthorizeRequest;
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
        PaymentsCompleteAuthorizeInput input{std::move(data)};
        auto result = m_runtime.payments_complete_authorize(input);
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

    void handle_payments_incremental_authorization(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto auth_error =
                m_input_support.authorize(req, "payments_incremental_authorization")) {
            res.set_status(auth_error->get_status());
            res.set_body(serde::Ser::serialize("application/json", *auth_error));
            send();
            return;
        }
        m_context.metrics().record_flow("payments.payments_incremental_authorization");
        m_context.metrics().counter("payments.payments_incremental_authorization").increment();

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
        using Input = ::app::model::payments::PaymentsIncrementalAuthorizationRequest;
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
        PaymentsIncrementalAuthorizationInput input{std::move(data)};
        auto result = m_runtime.payments_incremental_authorization(input);
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

    void handle_payments_extend_authorization(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto auth_error =
                m_input_support.authorize(req, "payments_extend_authorization")) {
            res.set_status(auth_error->get_status());
            res.set_body(serde::Ser::serialize("application/json", *auth_error));
            send();
            return;
        }
        m_context.metrics().record_flow("payments.payments_extend_authorization");
        m_context.metrics().counter("payments.payments_extend_authorization").increment();

        const auto body =
            m_input_support.path_body({{"payment_id", m_input_support.payment_id_from_path(req)}});
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
        using Input = ::app::model::payments::PaymentsExtendAuthorizationRequest;
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
        PaymentsExtendAuthorizationInput input{std::move(data)};
        auto result = m_runtime.payments_extend_authorization(input);
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

    void handle_payments_external_authentication(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto auth_error =
                m_input_support.authorize(req, "payments_external_authentication")) {
            res.set_status(auth_error->get_status());
            res.set_body(serde::Ser::serialize("application/json", *auth_error));
            send();
            return;
        }
        m_context.metrics().record_flow("payments.payments_external_authentication");
        m_context.metrics().counter("payments.payments_external_authentication").increment();

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
        using Input = ::app::model::payments::PaymentsExternalAuthenticationRequest;
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
        PaymentsExternalAuthenticationInput input{std::move(data)};
        auto result = m_runtime.payments_external_authentication(input);
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

    void handle_payments_submit_eligibility(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto auth_error = m_input_support.authorize(req, "payments_submit_eligibility")) {
            res.set_status(auth_error->get_status());
            res.set_body(serde::Ser::serialize("application/json", *auth_error));
            send();
            return;
        }
        m_context.metrics().record_flow("payments.payments_submit_eligibility");
        m_context.metrics().counter("payments.payments_submit_eligibility").increment();

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
        using Input = ::app::model::payments::PaymentsEligibilityRequest;
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
        PaymentsSubmitEligibilityInput input{std::move(data)};
        auto result = m_runtime.payments_submit_eligibility(input);
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

    void handle_payments_submit_eligibility_check(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto auth_error =
                m_input_support.authorize(req, "payments_submit_eligibility_check")) {
            res.set_status(auth_error->get_status());
            res.set_body(serde::Ser::serialize("application/json", *auth_error));
            send();
            return;
        }
        m_context.metrics().record_flow("payments.payments_submit_eligibility_check");
        m_context.metrics().counter("payments.payments_submit_eligibility_check").increment();

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
        using Input = ::app::model::payments::PaymentsEligibilityCheckRequest;
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
        PaymentsSubmitEligibilityCheckInput input{std::move(data)};
        auto result = m_runtime.payments_submit_eligibility_check(input);
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

    // original install() route #59
    void install_route_59(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "payments"}.add_router(
            Router{router, ":payment_id"}.add_route(
                Route{"eligibility_check"}
                    .post(
                        [this](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
                            handle_payments_submit_eligibility_check(req, res, std::move(send));
                        }
                    )
                    .summary("Hyperswitch payments_submit_eligibility_check")
            )
        );
    }

    // original install() route #60
    void install_route_60(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "payments"}.add_router(
            Router{router, ":payment_id"}.add_route(
                Route{"eligibility"}
                    .post(
                        [this](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
                            handle_payments_submit_eligibility(req, res, std::move(send));
                        }
                    )
                    .summary("Hyperswitch payments_submit_eligibility")
            )
        );
    }

    // original install() route #61
    void install_route_61(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "payments"}.add_router(
            Router{router, ":payment_id"}.add_router(
                Router{router, "3ds"}.add_route(
                    Route{"authentication"}
                        .post(
                            [this](
                                interfaces::io::IRequest& req,
                                interfaces::io::IResponse& res,
                                std::function<void()> send
                            )
                            {
                                handle_payments_external_authentication(req, res, std::move(send));
                            }
                        )
                        .summary("Hyperswitch payments_external_authentication")
                )
            )
        );
    }

    // original install() route #62
    void install_route_62(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "payments"}.add_router(
            Router{router, ":payment_id"}.add_route(
                Route{"complete_authorize"}
                    .post(
                        [this](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
                            handle_payments_complete_authorize(req, res, std::move(send));
                        }
                    )
                    .summary("Hyperswitch payments_complete_authorize")
            )
        );
    }

    // original install() route #63
    void install_route_63(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "payments"}.add_router(
            Router{router, ":payment_id"}.add_route(
                Route{"incremental_authorization"}
                    .post(
                        [this](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
                            handle_payments_incremental_authorization(req, res, std::move(send));
                        }
                    )
                    .summary("Hyperswitch payments_incremental_authorization")
            )
        );
    }

    // original install() route #64
    void install_route_64(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "payments"}.add_router(
            Router{router, ":payment_id"}.add_route(
                Route{"extend_authorization"}
                    .post(
                        [this](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
                            handle_payments_extend_authorization(req, res, std::move(send));
                        }
                    )
                    .summary("Hyperswitch payments_extend_authorization")
            )
        );
    }

    void install(core::router::RouterContext<>& router)
    {
        install_route_59(router);
        install_route_60(router);
        install_route_61(router);
        install_route_62(router);
        install_route_63(router);
        install_route_64(router);
    }

private:
    ::app::context::AppContext& m_context;
    PaymentsScopeState& m_scope;
    PaymentsInputSupport& m_input_support;
    PaymentsRuntime& m_runtime;
};

} // namespace app::handlers::payments
