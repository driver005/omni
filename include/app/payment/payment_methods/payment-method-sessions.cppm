export module app_payment:payment_methods_handlers_payment_method_sessions;

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
import :payment_methods_handlers_payment_method_sessions_by_id;
import :payment_methods_handlers_payment_method_sessions_by_id_saved;

export namespace app::handlers::payment_methods {

class PaymentMethodSessionsHandlers
{
public:
    void bind_request_support(PaymentMethodsRequestSupport& support) noexcept
    {
        m_request_support = &support;
        m_by_id.bind_request_support(support);
        m_by_id_saved.bind_request_support(support);
    }

    PaymentMethodsRequestSupport* m_request_support{nullptr};

    PaymentMethodSessionsHandlers(
        ::app::context::AppContext& ctx,
        PaymentMethodsRuntime& runtime
    ) noexcept :
        m_context{ctx},
        m_input_support{ctx},
        m_runtime{runtime},
        m_by_id{ctx, runtime},
        m_by_id_saved{ctx, runtime}
    {
    }

    void install(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "payment_methods"}.add_route(
            Route{"payment-method-sessions"}
                .post(
                    [this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    )
                    {
                        handle_payment_methods_session_create(req, res, std::move(send));
                    }
                )
                .summary("Hyperswitch payment_methods_session_create")
        );

        Router{router, ":pm_prefix/payment-method-sessions"}.add_route(
            Route{""}
                .post(
                    [this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    )
                    {
                        handle_payment_methods_session_create(req, res, std::move(send));
                    }
                )
                .summary("Hyperswitch payment_methods_session_create (v2 prefix)")
        );

        m_by_id.install(router);

        m_by_id_saved.install(router);
    }

    void handle_payment_methods_session_create(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto auth_error = m_request_support->authorize(
            req,
            "payment_methods_session_create",
            spec_for("payment_methods_session_create")
        );
        if (auth_error) {
            const auto& error = *auth_error;
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
        using Input = ::app::model::payment_methods::PaymentMethodSessionRequest;
        auto parsed = serde::Ser::deserialize<Input>("application/json", body);
        if (!parsed) {
            const auto error =
                m_input_support.error(core::error::AppErrorKind::INVALID_JSON, "invalid JSON");
            error.send(res, send);
            return;
        }
        const std::string merchant_id = ::app::http::merchant_id_from_header{}(req);
        const std::string org_id = ::app::http::org_id_from_header{}(req);
        if (merchant_id.empty() && org_id.empty()) {
            const auto error = m_input_support.error(
                core::error::AppErrorKind::INVALID_REQUEST,
                "missing merchant or organization id"
            );
            error.send(res, send);
            return;
        }
        if (parsed->get_customer_id() &&
            !m_input_support.customer_exists(parsed->get_customer_id()->get_value())) {
            const auto error = m_input_support.error(
                core::error::AppErrorKind::NOT_FOUND,
                "Customer does not exist in our records"
            );
            error.send(res, send);
            return;
        }
        PaymentMethodsSessionCreateInput input{*parsed, merchant_id, org_id};
        auto result = m_runtime.payment_methods_session_create(input);
        if (!result) {
            const auto& error = result.error();
            error.send(res, send);
            return;
        }
        res.set_status(interfaces::io::types::Status::OK);
        res.set_body(serialize_wire(*result));
        send();
    }

private:
    ::app::context::AppContext& m_context;
    PaymentMethodsInputSupport m_input_support;
    PaymentMethodsRuntime& m_runtime;
    PaymentMethodSessionByIdHandlers m_by_id;
    PaymentMethodSessionSavedHandlers m_by_id_saved;
};

} // namespace app::handlers::payment_methods
