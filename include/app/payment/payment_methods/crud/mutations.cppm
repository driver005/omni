export module app_payment:payment_methods_handlers_crud_mutations;

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

class CrudMutationsHandlers
{
public:
    void bind_request_support(PaymentMethodsRequestSupport& support) noexcept
    {
        m_request_support = &support;
    }

    PaymentMethodsRequestSupport* m_request_support{nullptr};

    CrudMutationsHandlers(::app::context::AppContext& ctx, PaymentMethodsRuntime& runtime) noexcept
        :
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
                Router{router, ":payment_method_id"}.add_route(
                    Route{"update"}
                        .post(
                            [this](
                                interfaces::io::IRequest& req,
                                interfaces::io::IResponse& res,
                                std::function<void()> send
                            )
                            {
                                interfaces::io::ResponseSink sink{res, std::move(send)};

                                handle_payment_method_update_api(req, res, std::move(send));
                            }
                        )
                        .summary("Hyperswitch payment_method_update_api")
                )
            )
            .add_router(
                Router{router, ":payment_method_id"}.add_route(
                    Route{"save"}
                        .post(
                            [this](
                                interfaces::io::IRequest& req,
                                interfaces::io::IResponse& res,
                                std::function<void()> send
                            )
                            {
                                interfaces::io::ResponseSink sink{res, std::move(send)};

                                handle_save_payment_method_api(req, res, std::move(send));
                            }
                        )
                        .summary("Hyperswitch save_payment_method_api")
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

                                handle_payment_method_update_api(req, res, std::move(send));
                            }
                        )
                        .summary("Hyperswitch payment_method_update_api")
                )
            )
            .add_router(
                Router{router, "customers"}.add_router(
                    Router{router, ":customer_id"}.add_router(
                        Router{router, "payment-methods"}.add_router(
                            Router{router, ":payment_method_id"}.add_route(
                                Route{"default"}
                                    .post(
                                        [this](
                                            interfaces::io::IRequest& req,
                                            interfaces::io::IResponse& res,
                                            std::function<void()> send
                                        )
                                        {
                                            interfaces::io::ResponseSink sink{res, std::move(send)};

                                            handle_default_payment_method_set_api(
                                                req,
                                                res,
                                                std::move(send)
                                            );
                                        }
                                    )
                                    .summary("Hyperswitch default_payment_method_set_api")
                            )
                        )
                    )
                )
            );

        Router{router, ":pm_prefix/payment-methods"}
            .add_router(
                Router{router, ":id"}.add_route(
                    Route{"update"}
                        .post(
                            [this](
                                interfaces::io::IRequest& req,
                                interfaces::io::IResponse& res,
                                std::function<void()> send
                            )
                            {
                                interfaces::io::ResponseSink sink{res, std::move(send)};

                                handle_payment_method_update_api(req, res, std::move(send));
                            }
                        )
                        .summary("Hyperswitch payment_method_update_api (v2 prefix)")
                )
            )
            .add_router(
                Router{router, ":id"}.add_route(
                    Route{"save"}
                        .post(
                            [this](
                                interfaces::io::IRequest& req,
                                interfaces::io::IResponse& res,
                                std::function<void()> send
                            )
                            {
                                interfaces::io::ResponseSink sink{res, std::move(send)};

                                handle_save_payment_method_api(req, res, std::move(send));
                            }
                        )
                        .summary("Hyperswitch save_payment_method_api (v2 prefix)")
                )
            );
    }

    void handle_payment_method_update_api(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};

        const auto auth_error = m_request_support->authorize(
            req,
            "payment_method_update_api",
            spec_for("payment_method_update_api")
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
        const std::string payment_method_id = m_input_support.path_segment(req, 1);
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
        const auto body = req.body_as_string();
        if (body.empty()) {
            const auto error =
                m_input_support.error(core::error::AppErrorKind::EMPTY_REQUEST, "empty request");
            error.send(sink);
            return;
        }
        auto parsed =
            core::behavior::Serializer<::app::model::payment_methods::PaymentMethodUpdate>::pass(
                req,
                sink
            );
        if (!parsed) {
            return;
        }
        if (const auto secret_error = m_input_support.check_client_secret(
                payment_method_id,
                parsed->get_client_secret()
            )) {
            const auto& error = *secret_error;
            error.send(sink);
            return;
        }
        const auto& dto = *parsed;
        auto result = m_runtime.payment_method_update_api(dto, m_request_support->context(req));
        result.handle(sink);
    }

    void handle_save_payment_method_api(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};

        const auto auth_error = m_request_support->authorize(
            req,
            "save_payment_method_api",
            spec_for("save_payment_method_api")
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
                "save_payment_method_api",
                client_facing_spec_for("save_payment_method_api")
            )) {
            const auto& error = *sdk_error;
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
        const auto body = req.body_as_string();
        if (body.empty()) {
            const auto error =
                m_input_support.error(core::error::AppErrorKind::EMPTY_REQUEST, "empty request");
            error.send(sink);
            return;
        }
        auto parsed =
            core::behavior::Serializer<::app::model::payment_methods::PaymentMethodCreate>::pass(
                req,
                sink
            );
        if (!parsed) {
            return;
        }
        if (!parsed->get_client_secret()) {
            if (const auto secret = ::app::http::RequestQuery{}(req).get("client_secret")) {
                parsed->set_client_secret(*secret);
            }
        }
        if (const auto secret_error = m_input_support.check_client_secret(
                payment_method_id,
                parsed->get_client_secret()
            )) {
            const auto& error = *secret_error;
            error.send(sink);
            return;
        }
        const std::string merchant_id = ::app::http::merchant_id_from_header{}(req);
        const auto& dto = *parsed;
        auto result = m_runtime.save_payment_method_api(dto, m_request_support->context(req));
        result.handle(sink);
    }

    void handle_default_payment_method_set_api(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};

        const auto auth_error = m_request_support->authorize(
            req,
            "default_payment_method_set_api",
            spec_for("default_payment_method_set_api")
        );
        if (auth_error) {
            const auto& error = *auth_error;
            error.send(sink);
            return;
        }
        const std::string customer_id = m_input_support.path_segment(req, 3);
        const std::string path_payment_method_id = m_input_support.path_segment(req, 1);
        if (customer_id.empty() || !m_input_support.customer_exists(customer_id)) {
            const auto error = m_input_support.error(
                core::error::AppErrorKind::NOT_FOUND,
                "Customer does not exist in our records"
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
        auto parsed =
            core::behavior::Serializer<::app::model::payment_methods::DefaultPaymentMethod>::pass(
                req,
                sink
            );
        if (!parsed) {
            return;
        }
        const std::string payment_method_id = path_payment_method_id.empty()
                                                  ? parsed->get_payment_method_id()
                                                  : path_payment_method_id;
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
        const auto& dto = *parsed;
        auto result =
            m_runtime.default_payment_method_set_api(dto, m_request_support->context(req));
        result.handle(sink);
    }

private:
    ::app::context::AppContext& m_context;
    PaymentMethodsInputSupport m_input_support;
    PaymentMethodsRuntime& m_runtime;
};

} // namespace app::handlers::payment_methods
