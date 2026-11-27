export module app_payment:payment_methods_handlers_crud_mutations;

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
        const auto auth_error = m_request_support->authorize(
            req,
            "payment_method_update_api",
            spec_for("payment_method_update_api")
        );
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
        const std::string payment_method_id = m_input_support.path_segment(req, 1);
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
        const auto body = req.body_as_string();
        if (body.empty()) {
            const auto error =
                m_input_support.error(core::error::AppErrorKind::EMPTY_REQUEST, "empty request");
            error.send(res, send);
            return;
        }
        using Input = ::app::model::payment_methods::PaymentMethodUpdate;
        auto parsed = serde::Ser::deserialize<Input>("application/json", body);
        if (!parsed) {
            const auto error =
                m_input_support.error(core::error::AppErrorKind::INVALID_JSON, "invalid JSON");
            error.send(res, send);
            return;
        }
        if (const auto secret_error = m_input_support.check_client_secret(
                payment_method_id,
                parsed->get_client_secret()
            )) {
            const auto& error = *secret_error;
            error.send(res, send);
            return;
        }
        PaymentMethodUpdateApiInput input{*parsed, payment_method_id, std::move(*record)};
        auto result = m_runtime.payment_method_update_api(input);
        if (!result) {
            const auto& error = result.error();
            error.send(res, send);
            return;
        }
        res.set_status(interfaces::io::types::Status::OK);
        res.set_body(serialize_wire(*result));
        send();
    }

    void handle_save_payment_method_api(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto auth_error = m_request_support->authorize(
            req,
            "save_payment_method_api",
            spec_for("save_payment_method_api")
        );
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
        if (const auto sdk_error = m_request_support->authorize(
                req,
                "save_payment_method_api",
                client_facing_spec_for("save_payment_method_api")
            )) {
            const auto& error = *sdk_error;
            error.send(res, send);
            return;
        }
        const std::string payment_method_id = m_input_support.path_segment(req, 1);
        if (payment_method_id.empty()) {
            const auto error = m_input_support.error(
                core::error::AppErrorKind::INVALID_REQUEST,
                "missing payment method id"
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
        if (const auto secret_error = m_input_support.check_client_secret(
                payment_method_id,
                parsed->get_client_secret()
            )) {
            const auto& error = *secret_error;
            error.send(res, send);
            return;
        }
        const std::string merchant_id = ::app::http::merchant_id_from_header{}(req);
        SavePaymentMethodApiInput input{*parsed, payment_method_id, merchant_id};
        auto result = m_runtime.save_payment_method_api(input);
        if (!result) {
            const auto& error = result.error();
            error.send(res, send);
            return;
        }
        res.set_status(interfaces::io::types::Status::OK);
        res.set_body(serialize_wire(*result));
        send();
    }

    void handle_default_payment_method_set_api(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto auth_error = m_request_support->authorize(
            req,
            "default_payment_method_set_api",
            spec_for("default_payment_method_set_api")
        );
        if (auth_error) {
            const auto& error = *auth_error;
            error.send(res, send);
            return;
        }
        const std::string customer_id = m_input_support.path_segment(req, 3);
        const std::string path_payment_method_id = m_input_support.path_segment(req, 1);
        if (customer_id.empty() || !m_input_support.customer_exists(customer_id)) {
            const auto error = m_input_support.error(
                core::error::AppErrorKind::NOT_FOUND,
                "Customer does not exist in our records"
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
        using Input = ::app::model::payment_methods::DefaultPaymentMethod;
        auto parsed = serde::Ser::deserialize<Input>("application/json", body);
        if (!parsed) {
            const auto error =
                m_input_support.error(core::error::AppErrorKind::INVALID_JSON, "invalid JSON");
            error.send(res, send);
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
        DefaultPaymentMethodSetApiInput input{*parsed, customer_id, payment_method_id, *record};
        auto result = m_runtime.default_payment_method_set_api(input);
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
};

} // namespace app::handlers::payment_methods
