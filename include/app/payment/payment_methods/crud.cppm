export module app_payment:payment_methods_handlers_crud;

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
import :payment_methods_handlers_crud_mutations;

export namespace app::handlers::payment_methods {

class CrudHandlers
{
public:
    void bind_request_support(PaymentMethodsRequestSupport& support) noexcept
    {
        m_request_support = &support;
        m_mutations.bind_request_support(support);
    }

    PaymentMethodsRequestSupport* m_request_support{nullptr};

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
        interfaces::io::ResponseSink sink{res, std::move(send)};

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
        interfaces::io::ResponseSink sink{res, std::move(send)};

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
        interfaces::io::ResponseSink sink{res, std::move(send)};

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
        interfaces::io::ResponseSink sink{res, std::move(send)};

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
        interfaces::io::ResponseSink sink{res, std::move(send)};

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
        interfaces::io::ResponseSink sink{res, std::move(send)};

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
        interfaces::io::ResponseSink sink{res, std::move(send)};

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
        interfaces::io::ResponseSink sink{res, std::move(send)};

        const auto auth_error = m_request_support->authorize(
            req,
            "create_payment_method_api",
            spec_for("create_payment_method_api")
        );
        if (auth_error) {
            const auto& error = *auth_error;
            error.send(sink);
            return;
        }
        if (!m_input_support.legacy_access_allowed(req)) {
            const auto error = m_input_support.error(
                core::error::AppErrorKind::FORBIDDEN,
                "legacy endpoint access denied"
            );
            error.send(sink);
            return;
        }
        if (const auto sdk_error = m_request_support->authorize(
                req,
                "create_payment_method_api",
                client_facing_spec_for("create_payment_method_api")
            )) {
            const auto& error = *sdk_error;
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
        auto parsed = core::behavior::Serializer<::app::model::payment_methods::PaymentMethodCreate>::pass(req, sink);
        if (!parsed) return;
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
            error.send(sink);
            return;
        }
        const std::string profile_id = ::app::auth::AuthContextBuilder{}(req).profile_id();
        const auto& dto = *parsed;
        auto result = m_runtime.create_payment_method_api(dto, m_request_support->context(req));
        result.handle(sink);
    }

    void handle_create_payment_method_intent_api(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};

        const auto auth_error = m_request_support->authorize(
            req,
            "create_payment_method_intent_api",
            spec_for("create_payment_method_intent_api")
        );
        if (auth_error) {
            const auto& error = *auth_error;
            error.send(sink);
            return;
        }
        if (!m_input_support.legacy_access_allowed(req)) {
            const auto error = m_input_support.error(
                core::error::AppErrorKind::FORBIDDEN,
                "legacy endpoint access denied"
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
        auto parsed = core::behavior::Serializer<::app::model::payment_methods::PaymentMethodIntentCreate>::pass(req, sink);
        if (!parsed) return;
        const std::string merchant_id = ::app::http::merchant_id_from_header{}(req);
        if (merchant_id.empty()) {
            const auto error = m_input_support.error(
                core::error::AppErrorKind::INVALID_REQUEST,
                "missing merchant id"
            );
            error.send(sink);
            return;
        }
        const auto& dto = *parsed;
        auto result = m_runtime.create_payment_method_intent_api(dto, m_request_support->context(req));
        result.handle(sink);
    }

    void handle_payment_method_retrieve_api(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};

        const auto auth_error = m_request_support->authorize(
            req,
            "payment_method_retrieve_api",
            spec_for("payment_method_retrieve_api")
        );
        if (auth_error) {
            const auto& error = *auth_error;
            error.send(sink);
            return;
        }
        if (!m_input_support.legacy_access_allowed(req)) {
            const auto error = m_input_support.error(
                core::error::AppErrorKind::FORBIDDEN,
                "legacy endpoint access denied"
            );
            error.send(sink);
            return;
        }
        const std::string payment_method_id = m_input_support.path_segment(req);
        if (payment_method_id.empty()) {
            const auto error = m_input_support.error(
                core::error::AppErrorKind::INVALID_REQUEST,
                "missing payment method id"
            );
            error.send(sink);
            return;
        }
        auto record = m_input_support.find_payment_method(payment_method_id);
        if (!record) {
            const auto error = m_input_support.error(
                core::error::AppErrorKind::NOT_FOUND,
                "Payment method does not exist in our records"
            );
            error.send(sink);
            return;
        }
        const std::string merchant_id = ::app::http::merchant_id_from_header{}(req);
        if (!merchant_id.empty() && record->get_merchant_id() != merchant_id) {
            const auto error = m_input_support.error(
                core::error::AppErrorKind::FORBIDDEN,
                "payment method belongs to another merchant"
            );
            error.send(sink);
            return;
        }
        const bool force_sync = ::app::http::RequestQuery{}(req).get("force_sync").has_value();
        const auto& dto = *record;
        auto result = m_runtime.payment_method_retrieve_api(dto, m_request_support->context(req));
        result.handle(sink);
    }

    void handle_payment_method_delete_api(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};

        const auto auth_error = m_request_support->authorize(
            req,
            "payment_method_delete_api",
            spec_for("payment_method_delete_api")
        );
        if (auth_error) {
            const auto& error = *auth_error;
            error.send(sink);
            return;
        }
        if (!m_input_support.legacy_access_allowed(req)) {
            const auto error = m_input_support.error(
                core::error::AppErrorKind::FORBIDDEN,
                "legacy endpoint access denied"
            );
            error.send(sink);
            return;
        }
        const std::string payment_method_id = m_input_support.path_segment(req);
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
        auto result = m_runtime.payment_method_delete_api(dto, m_request_support->context(req));
        result.handle(sink);
    }

private:
    ::app::context::AppContext& m_context;
    PaymentMethodsInputSupport m_input_support;
    PaymentMethodsRuntime& m_runtime;
    CrudMutationsHandlers m_mutations;
};

} // namespace app::handlers::payment_methods
