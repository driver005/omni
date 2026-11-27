export module app_payment:payment_methods_handlers_migrate;

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

class MigrateHandlers
{
public:
    void bind_request_support(PaymentMethodsRequestSupport& support) noexcept
    {
        m_request_support = &support;
    }

    PaymentMethodsRequestSupport* m_request_support{nullptr};

    MigrateHandlers(::app::context::AppContext& ctx, PaymentMethodsRuntime& runtime) noexcept :
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
                Route{"migrate"}
                    .post(
                        [this](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
        interfaces::io::ResponseSink sink{res, std::move(send)};

                            handle_migrate_payment_method_api(req, res, std::move(send));
                        }
                    )
                    .summary("Hyperswitch migrate_payment_method_api")
            )
            .add_route(
                Route{"migrate-batch"}
                    .post(
                        [this](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
        interfaces::io::ResponseSink sink{res, std::move(send)};

                            handle_migrate_payment_methods(req, res, std::move(send));
                        }
                    )
                    .summary("Hyperswitch migrate_payment_methods")
            )
            .add_route(
                Route{"update-batch"}
                    .post(
                        [this](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
        interfaces::io::ResponseSink sink{res, std::move(send)};

                            handle_update_payment_methods(req, res, std::move(send));
                        }
                    )
                    .summary("Hyperswitch update_payment_methods")
            )
            .add_router(
                Router{router, "fingerprint"}.add_route(
                    Route{"migrate-batch"}
                        .post(
                            [this](
                                interfaces::io::IRequest& req,
                                interfaces::io::IResponse& res,
                                std::function<void()> send
                            )
                            {
        interfaces::io::ResponseSink sink{res, std::move(send)};

                                handle_modular_migrate_payment_methods(req, res, std::move(send));
                            }
                        )
                        .summary("Hyperswitch modular_migrate_payment_methods")
                )
            );

        Router{router, ":pm_prefix/payment-methods"}.add_router(
            Router{router, "fingerprint"}.add_route(
                Route{"migrate-batch"}
                    .post(
                        [this](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
        interfaces::io::ResponseSink sink{res, std::move(send)};

                            handle_modular_migrate_payment_methods(req, res, std::move(send));
                        }
                    )
                    .summary("Hyperswitch modular_migrate_payment_methods (v2 prefix)")
            )
        );
    }

    void handle_migrate_payment_method_api(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};

        const auto auth_error = m_request_support->authorize(
            req,
            "migrate_payment_method_api",
            spec_for("migrate_payment_method_api")
        );
        if (auth_error) {
            const auto& error = *auth_error;
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
        auto parsed = core::behavior::Serializer<::app::model::payment_methods::PaymentMethodMigrate>::pass(req, sink);
        if (!parsed) return;
        std::string merchant_id = ::app::http::merchant_id_from_header{}(req);
        if (merchant_id.empty()) {
            merchant_id = parsed->get_merchant_id().get_value();
        }
        if (merchant_id.empty()) {
            const auto error = m_input_support.error(
                core::error::AppErrorKind::INVALID_REQUEST,
                "missing merchant id"
            );
            error.send(sink);
            return;
        }
        const auto& dto = *parsed;
        auto result = m_runtime.migrate_payment_method_api(dto, m_request_support->context(req));
        result.handle(sink);
    }

    void handle_migrate_payment_methods(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};

        const auto auth_error = m_request_support->authorize(
            req,
            "migrate_payment_methods",
            spec_for("migrate_payment_methods")
        );
        if (auth_error) {
            const auto& error = *auth_error;
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
        auto parsed = core::behavior::Serializer<std::vector<::app::model::payment_methods::PaymentMethodMigrate>>::pass(req, sink);
        if (!parsed) return;
            "application/json",
            m_input_support.parse_batch_rows(body)
        );
        if (!parsed) {
            const auto error =
                m_input_support.error(core::error::AppErrorKind::INVALID_JSON, "invalid JSON");
            error.send(sink);
            return;
        }
        if (parsed->empty()) {
            const auto error =
                m_input_support.error(core::error::AppErrorKind::INVALID_REQUEST, "no rows");
            error.send(sink);
            return;
        }
        if (parsed->size() > 200) {
            const auto error = m_input_support.error(
                core::error::AppErrorKind::VALIDATION_FAILED,
                "batch limit is 200"
            );
            error.send(sink);
            return;
        }
        auto result = m_runtime.migrate_payment_methods(m_request_support->context(req));
        result.handle(sink);
    }

    void handle_update_payment_methods(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};

        const auto auth_error = m_request_support->authorize(
            req,
            "update_payment_methods",
            spec_for("update_payment_methods")
        );
        if (auth_error) {
            const auto& error = *auth_error;
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
        auto parsed = core::behavior::Serializer<std::vector<::app::model::payment_methods::UpdatePaymentMethodRecord>>::pass(req, sink);
        if (!parsed) return;
            "application/json",
            m_input_support.parse_batch_rows(body)
        );
        if (!parsed) {
            const auto error =
                m_input_support.error(core::error::AppErrorKind::INVALID_JSON, "invalid JSON");
            error.send(sink);
            return;
        }
        if (parsed->empty()) {
            const auto error =
                m_input_support.error(core::error::AppErrorKind::INVALID_REQUEST, "no records");
            error.send(sink);
            return;
        }
        if (parsed->size() > 200) {
            const auto error = m_input_support.error(
                core::error::AppErrorKind::VALIDATION_FAILED,
                "batch limit is 200"
            );
            error.send(sink);
            return;
        }
        auto result = m_runtime.update_payment_methods(m_request_support->context(req));
        result.handle(sink);
    }

    void handle_modular_migrate_payment_methods(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};

        const auto auth_error = m_request_support->authorize(
            req,
            "modular_migrate_payment_methods",
            spec_for("modular_migrate_payment_methods")
        );
        if (auth_error) {
            const auto& error = *auth_error;
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
        auto parsed = core::behavior::Serializer<std::vector<::app::model::payment_methods::PaymentMethodMigrate>>::pass(req, sink);
        if (!parsed) return;
            "application/json",
            m_input_support.parse_batch_rows(body)
        );
        if (!parsed) {
            const auto error =
                m_input_support.error(core::error::AppErrorKind::INVALID_JSON, "invalid JSON");
            error.send(sink);
            return;
        }
        if (parsed->empty()) {
            const auto error =
                m_input_support.error(core::error::AppErrorKind::INVALID_REQUEST, "no rows");
            error.send(sink);
            return;
        }
        if (parsed->size() > 200) {
            const auto error = m_input_support.error(
                core::error::AppErrorKind::VALIDATION_FAILED,
                "batch limit is 200"
            );
            error.send(sink);
            return;
        }
        auto result = m_runtime.modular_migrate_payment_methods(m_request_support->context(req));
        result.handle(sink);
    }

private:
    ::app::context::AppContext& m_context;
    PaymentMethodsInputSupport m_input_support;
    PaymentMethodsRuntime& m_runtime;
};

} // namespace app::handlers::payment_methods
