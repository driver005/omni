export module app_payment:payment_methods_handlers_crud;

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
import :payment_methods_handlers_crud_mutations;

export namespace app::handlers::payment_methods {

class CrudHandlers
{
public:
    CrudHandlers(::app::context::AppContext& ctx, PaymentMethodsRuntime& runtime) noexcept :
        m_context{ctx},
        m_input_support{ctx},
        m_runtime{runtime},
        m_mutations{ctx, runtime}
    {
    }

    void install(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "payment_methods"}
            .add_route(
                Route{""}
                    .post(
                        [this](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
                            handle_create_payment_method_api(req, res, std::move(send));
                        }
                    )
                    .summary("Hyperswitch create_payment_method_api")
            )
            .add_route(
                Route{"create-intent"}
                    .post(
                        [this](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
                            handle_create_payment_method_intent_api(req, res, std::move(send));
                        }
                    )
                    .summary("Hyperswitch create_payment_method_intent_api")
            )
            .add_route(
                Route{":payment_method_id"}
                    .get(
                        [this](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
                            handle_payment_method_retrieve_api(req, res, std::move(send));
                        }
                    )
                    .summary("Hyperswitch payment_method_retrieve_api")
            )
            .add_route(
                Route{":payment_method_id"}
                    .delt(
                        [this](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
                            handle_payment_method_delete_api(req, res, std::move(send));
                        }
                    )
                    .summary("Hyperswitch payment_method_delete_api")
            );

        Router{router, ":pm_prefix/payment-methods"}
            .add_route(
                Route{""}
                    .post(
                        [this](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
                            handle_create_payment_method_api(req, res, std::move(send));
                        }
                    )
                    .summary("Hyperswitch create_payment_method_api (v2 prefix)")
            )
            .add_route(
                Route{":id"}
                    .get(
                        [this](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
                            handle_payment_method_retrieve_api(req, res, std::move(send));
                        }
                    )
                    .summary("Hyperswitch payment_method_retrieve_api (v2 prefix)")
            )
            .add_route(
                Route{":id"}
                    .delt(
                        [this](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
                            handle_payment_method_delete_api(req, res, std::move(send));
                        }
                    )
                    .summary("Hyperswitch payment_method_delete_api (v2 prefix)")
            );

        m_mutations.install(router);
    }

    void handle_create_payment_method_api(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto auth_error = m_input_support.authorize(req, "create_payment_method_api");
        if (auth_error) {
            const auto& error = *auth_error;
            error.send(res, send);
            return;
        }
        if (!m_input_support.legacy_access_allowed(req)) {
            const auto error = m_input_support.error(
                core::error::AppErrorKind::FORBIDDEN,
                "legacy endpoint access denied"
            );
            error.send(res, send);
            return;
        }
        if (const auto sdk_error =
                m_input_support.authorize_client_facing(req, "create_payment_method_api")) {
            const auto& error = *sdk_error;
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
        using Input = ::app::model::payment_methods::PaymentMethodCreate;
        auto parsed = serde::Ser::deserialize<Input>("application/json", body);
        if (!parsed) {
            const auto error =
                m_input_support.error(core::error::AppErrorKind::INVALID_JSON, "invalid JSON");
            error.send(res, send);
            return;
        }
        if (!parsed->get_client_secret()) {
            if (const auto secret = ::app::http::RequestQuery{}(req).get("client_secret")) {
                parsed->set_client_secret(*secret);
            }
        }
        const std::string merchant_id = ::app::http::merchant_id_from_header{}(req);
        if (merchant_id.empty()) {
            const auto error = m_input_support.error(
                core::error::AppErrorKind::INVALID_REQUEST,
                "missing merchant id"
            );
            error.send(res, send);
            return;
        }
        const std::string profile_id = ::app::auth::AuthContextBuilder{}(req).profile_id();
        CreatePaymentMethodApiInput input{*parsed, merchant_id, profile_id};
        auto result = m_runtime.create_payment_method_api(input);
        if (!result) {
            const auto& error = result.error();
            error.send(res, send);
            return;
        }
        res.set_status(interfaces::io::types::Status::OK);
        res.set_body(serde::Ser::serialize("application/json", *result));
        send();
    }

    void handle_create_payment_method_intent_api(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto auth_error = m_input_support.authorize(req, "create_payment_method_intent_api");
        if (auth_error) {
            const auto& error = *auth_error;
            error.send(res, send);
            return;
        }
        if (!m_input_support.legacy_access_allowed(req)) {
            const auto error = m_input_support.error(
                core::error::AppErrorKind::FORBIDDEN,
                "legacy endpoint access denied"
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
        using Input = ::app::model::payment_methods::PaymentMethodIntentCreate;
        auto parsed = serde::Ser::deserialize<Input>("application/json", body);
        if (!parsed) {
            const auto error =
                m_input_support.error(core::error::AppErrorKind::INVALID_JSON, "invalid JSON");
            error.send(res, send);
            return;
        }
        const std::string merchant_id = ::app::http::merchant_id_from_header{}(req);
        if (merchant_id.empty()) {
            const auto error = m_input_support.error(
                core::error::AppErrorKind::INVALID_REQUEST,
                "missing merchant id"
            );
            error.send(res, send);
            return;
        }
        CreatePaymentMethodIntentApiInput input{*parsed, merchant_id};
        auto result = m_runtime.create_payment_method_intent_api(input);
        if (!result) {
            const auto& error = result.error();
            error.send(res, send);
            return;
        }
        res.set_status(interfaces::io::types::Status::OK);
        res.set_body(serde::Ser::serialize("application/json", *result));
        send();
    }

    void handle_payment_method_retrieve_api(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto auth_error = m_input_support.authorize(req, "payment_method_retrieve_api");
        if (auth_error) {
            const auto& error = *auth_error;
            error.send(res, send);
            return;
        }
        if (!m_input_support.legacy_access_allowed(req)) {
            const auto error = m_input_support.error(
                core::error::AppErrorKind::FORBIDDEN,
                "legacy endpoint access denied"
            );
            error.send(res, send);
            return;
        }
        const std::string payment_method_id = m_input_support.path_segment(req);
        if (payment_method_id.empty()) {
            const auto error = m_input_support.error(
                core::error::AppErrorKind::INVALID_REQUEST,
                "missing payment method id"
            );
            error.send(res, send);
            return;
        }
        auto record = m_input_support.find_payment_method(payment_method_id);
        if (!record) {
            const auto error = m_input_support.error(
                core::error::AppErrorKind::NOT_FOUND,
                "Payment method does not exist in our records"
            );
            error.send(res, send);
            return;
        }
        const std::string merchant_id = ::app::http::merchant_id_from_header{}(req);
        if (!merchant_id.empty() && record->get_merchant_id() != merchant_id) {
            const auto error = m_input_support.error(
                core::error::AppErrorKind::FORBIDDEN,
                "payment method belongs to another merchant"
            );
            error.send(res, send);
            return;
        }
        const bool force_sync = ::app::http::RequestQuery{}(req).get("force_sync").has_value();
        PaymentMethodRetrieveApiInput
            input{std::move(*record), payment_method_id, merchant_id, force_sync};
        auto result = m_runtime.payment_method_retrieve_api(input);
        if (!result) {
            const auto& error = result.error();
            error.send(res, send);
            return;
        }
        res.set_status(interfaces::io::types::Status::OK);
        res.set_body(serde::Ser::serialize("application/json", *result));
        send();
    }

    void handle_payment_method_delete_api(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto auth_error = m_input_support.authorize(req, "payment_method_delete_api");
        if (auth_error) {
            const auto& error = *auth_error;
            error.send(res, send);
            return;
        }
        if (!m_input_support.legacy_access_allowed(req)) {
            const auto error = m_input_support.error(
                core::error::AppErrorKind::FORBIDDEN,
                "legacy endpoint access denied"
            );
            error.send(res, send);
            return;
        }
        const std::string payment_method_id = m_input_support.path_segment(req);
        if (payment_method_id.empty()) {
            const auto error = m_input_support.error(
                core::error::AppErrorKind::INVALID_REQUEST,
                "missing payment method id"
            );
            error.send(res, send);
            return;
        }
        const auto record = m_input_support.find_payment_method(payment_method_id);
        if (!record) {
            const auto error = m_input_support.error(
                core::error::AppErrorKind::NOT_FOUND,
                "Payment method does not exist in our records"
            );
            error.send(res, send);
            return;
        }
        PaymentMethodDeleteApiInput input{*record, payment_method_id};
        auto result = m_runtime.payment_method_delete_api(input);
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
    CrudMutationsHandlers m_mutations;
};

} // namespace app::handlers::payment_methods
