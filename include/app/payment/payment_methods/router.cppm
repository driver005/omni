export module app_payment:payment_methods_handler;

import std;
import interfaces;
import core_router;
import app_context;

import :payment_methods_input;
import :context;
import :payment_methods_result;
import :payment_methods_runtime;
import :payment_methods_handlers_crud;
import :payment_methods_handlers_client_list;
import :payment_methods_handlers_olap_support;
import core_behavior;
import core_serde;
import utils_openapi;
import app_model;
import app_dto;
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
import :payment_methods_handlers_payment_method_sessions_by_id;
import :payment_methods_handlers_payment_method_sessions_by_id_saved;

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
        auto parsed =
            core::behavior::Serializer<::app::model::payment_methods::PaymentMethodMigrate>::pass(
                req,
                sink
            );
        if (!parsed) {
            return;
        }
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
        auto parsed = core::behavior::Serializer<
            std::vector<::app::model::payment_methods::PaymentMethodMigrate>>::pass(req, sink);
        if (!parsed) {
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
        auto parsed = core::behavior::Serializer<
            std::vector<::app::model::payment_methods::UpdatePaymentMethodRecord>>::pass(req, sink);
        if (!parsed) {
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
        auto parsed = core::behavior::Serializer<
            std::vector<::app::model::payment_methods::PaymentMethodMigrate>>::pass(req, sink);
        if (!parsed) {
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
                        interfaces::io::ResponseSink sink{res, std::move(send)};

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
                        interfaces::io::ResponseSink sink{res, std::move(send)};

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
        interfaces::io::ResponseSink sink{res, std::move(send)};

        const auto auth_error = m_request_support->authorize(
            req,
            "payment_methods_session_create",
            spec_for("payment_methods_session_create")
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
        auto parsed = core::behavior::Serializer<
            ::app::model::payment_methods::PaymentMethodSessionRequest>::pass(req, sink);
        if (!parsed) {
            return;
        }
        const std::string merchant_id = ::app::http::merchant_id_from_header{}(req);
        const std::string org_id = ::app::http::org_id_from_header{}(req);
        if (merchant_id.empty() && org_id.empty()) {
            const auto error = m_input_support.error(
                core::error::AppErrorKind::INVALID_REQUEST,
                "missing merchant or organization id"
            );
            error.send(sink);
            return;
        }
        if (parsed->get_customer_id() &&
            !m_input_support.customer_exists(parsed->get_customer_id()->get_value())) {
            const auto error = m_input_support.error(
                core::error::AppErrorKind::NOT_FOUND,
                "Customer does not exist in our records"
            );
            error.send(sink);
            return;
        }
        const auto& dto = *parsed;
        auto result =
            m_runtime.payment_methods_session_create(dto, m_request_support->context(req));
        result.handle(sink);
    }

private:
    ::app::context::AppContext& m_context;
    PaymentMethodsInputSupport m_input_support;
    PaymentMethodsRuntime& m_runtime;
    PaymentMethodSessionByIdHandlers m_by_id;
    PaymentMethodSessionSavedHandlers m_by_id_saved;
};

class TokenizeHandlers
{
public:
    void bind_request_support(PaymentMethodsRequestSupport& support) noexcept
    {
        m_request_support = &support;
    }

    PaymentMethodsRequestSupport* m_request_support{nullptr};

    TokenizeHandlers(::app::context::AppContext& ctx, PaymentMethodsRuntime& runtime) noexcept :
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
                Route{"tokenize-card"}
                    .post(
                        [this](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
                            interfaces::io::ResponseSink sink{res, std::move(send)};

                            handle_tokenize_card_api(req, res, std::move(send));
                        }
                    )
                    .summary("Hyperswitch tokenize_card_api")
            )
            .add_route(
                Route{"tokenize-card-batch"}
                    .post(
                        [this](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
                            interfaces::io::ResponseSink sink{res, std::move(send)};

                            handle_tokenize_card_batch_api(req, res, std::move(send));
                        }
                    )
                    .summary("Hyperswitch tokenize_card_batch_api")
            )
            .add_router(
                Router{router, ":payment_method_id"}.add_route(
                    Route{"tokenize-card"}
                        .post(
                            [this](
                                interfaces::io::IRequest& req,
                                interfaces::io::IResponse& res,
                                std::function<void()> send
                            )
                            {
                                interfaces::io::ResponseSink sink{res, std::move(send)};

                                handle_tokenize_card_using_pm_api(req, res, std::move(send));
                            }
                        )
                        .summary("Hyperswitch tokenize_card_using_pm_api")
                )
            );
    }

    void handle_tokenize_card_api(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};

        const auto auth_error =
            m_request_support->authorize(req, "tokenize_card_api", spec_for("tokenize_card_api"));
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
        auto parsed =
            core::behavior::Serializer<::app::model::payment_methods::TokenizeCardRequest>::pass(
                req,
                sink
            );
        if (!parsed) {
            return;
        }
        const std::string merchant_id = ::app::http::merchant_id_from_header{}(req);
        const auto& dto = *parsed;
        auto result = m_runtime.tokenize_card_api(dto, m_request_support->context(req));
        result.handle(sink);
    }

    void handle_tokenize_card_using_pm_api(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};

        const auto auth_error = m_request_support->authorize(
            req,
            "tokenize_card_using_pm_api",
            spec_for("tokenize_card_using_pm_api")
        );
        if (auth_error) {
            const auto& error = *auth_error;
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
        auto parsed = core::behavior::Serializer<
            ::app::model::payment_methods::TokenizePaymentMethodRequest>::pass(req, sink);
        if (!parsed) {
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
        auto result = m_runtime.tokenize_card_using_pm_api(dto, m_request_support->context(req));
        result.handle(sink);
    }

    void handle_tokenize_card_batch_api(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};

        const auto auth_error = m_request_support->authorize(
            req,
            "tokenize_card_batch_api",
            spec_for("tokenize_card_batch_api")
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
        auto parsed =
            serde::Ser::decode_generic("application/json", m_input_support.parse_batch_rows(body));
        if (!parsed) {
            const auto error =
                m_input_support.error(core::error::AppErrorKind::INVALID_JSON, "invalid JSON");
            error.send(sink);
            return;
        }
        const auto array = std::get_if<interfaces::Value::Array>(&parsed->get());
        if (!array) {
            const auto error = m_input_support.error(
                core::error::AppErrorKind::INVALID_REQUEST,
                "expected an array of cards"
            );
            error.send(sink);
            return;
        }
        if (array->size() > 200) {
            const auto error = m_input_support.error(
                core::error::AppErrorKind::VALIDATION_FAILED,
                "batch limit is 200"
            );
            error.send(sink);
            return;
        }
        const std::string merchant_id = ::app::http::merchant_id_from_header{}(req);
        std::vector<std::string> payloads{};
        for (const auto& element: *array) {
            payloads.push_back(serde::Ser::encode_json(element));
        }
        auto result = m_runtime.tokenize_card_batch_api(m_request_support->context(req));
        result.handle(sink);
    }

private:
    ::app::context::AppContext& m_context;
    PaymentMethodsInputSupport m_input_support;
    PaymentMethodsRuntime& m_runtime;
};

class NetworkTokenHandlers
{
public:
    void bind_request_support(PaymentMethodsRequestSupport& support) noexcept
    {
        m_request_support = &support;
    }

    PaymentMethodsRequestSupport* m_request_support{nullptr};

    NetworkTokenHandlers(::app::context::AppContext& ctx, PaymentMethodsRuntime& runtime) noexcept :
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
                Route{"get-network-token-eligibility"}
                    .get(
                        [this](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
                            interfaces::io::ResponseSink sink{res, std::move(send)};

                            handle_get_pm_nt_eligibility_api(req, res, std::move(send));
                        }
                    )
                    .summary("Hyperswitch get_pm_nt_eligibility_api")
            )
            .add_router(
                Router{router, "token"}.add_router(
                    Router{router, ":payment_method_temporary_token"}.add_route(
                        Route{"details"}
                            .get(
                                [this](
                                    interfaces::io::IRequest& req,
                                    interfaces::io::IResponse& res,
                                    std::function<void()> send
                                )
                                {
                                    interfaces::io::ResponseSink sink{res, std::move(send)};

                                    handle_payment_method_get_token_details_api(
                                        req,
                                        res,
                                        std::move(send)
                                    );
                                }
                            )
                            .summary("Hyperswitch payment_method_get_token_details_api")
                    )
                )
            )
            .add_router(
                Router{router, ":payment_method_id"}.add_route(
                    Route{"check-network-token-status"}
                        .get(
                            [this](
                                interfaces::io::IRequest& req,
                                interfaces::io::IResponse& res,
                                std::function<void()> send
                            )
                            {
                                interfaces::io::ResponseSink sink{res, std::move(send)};

                                handle_network_token_status_check_api(req, res, std::move(send));
                            }
                        )
                        .summary("Hyperswitch network_token_status_check_api")
                )
            )
            .add_router(
                Router{router, ":id"}.add_route(
                    Route{"get-token"}
                        .get(
                            [this](
                                interfaces::io::IRequest& req,
                                interfaces::io::IResponse& res,
                                std::function<void()> send
                            )
                            {
                                interfaces::io::ResponseSink sink{res, std::move(send)};

                                handle_get_payment_method_token_data(req, res, std::move(send));
                            }
                        )
                        .summary("Hyperswitch get_payment_method_token_data")
                )
            );

        Router{router, ":pm_prefix/payment-methods"}
            .add_router(
                Router{router, ":id"}.add_route(
                    Route{"get-token"}
                        .get(
                            [this](
                                interfaces::io::IRequest& req,
                                interfaces::io::IResponse& res,
                                std::function<void()> send
                            )
                            {
                                interfaces::io::ResponseSink sink{res, std::move(send)};

                                handle_get_payment_method_token_data(req, res, std::move(send));
                            }
                        )
                        .summary("Hyperswitch get_payment_method_token_data (v2 prefix)")
                )
            )
            .add_router(
                Router{router, ":id"}.add_route(
                    Route{"check-network-token-status"}
                        .get(
                            [this](
                                interfaces::io::IRequest& req,
                                interfaces::io::IResponse& res,
                                std::function<void()> send
                            )
                            {
                                interfaces::io::ResponseSink sink{res, std::move(send)};

                                handle_network_token_status_check_api(req, res, std::move(send));
                            }
                        )
                        .summary("Hyperswitch network_token_status_check_api (v2 prefix)")
                )
            );
    }

    void handle_get_pm_nt_eligibility_api(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};

        const auto auth_error = m_request_support->authorize(
            req,
            "get_pm_nt_eligibility_api",
            spec_for("get_pm_nt_eligibility_api")
        );
        if (auth_error) {
            const auto& error = *auth_error;
            error.send(sink);
            return;
        }
        const auto query = ::app::http::RequestQuery{}(req);
        const auto card_bin = query.get("card_bin").value_or("");
        const std::string org_id = ::app::http::org_id_from_header{}(req);
        if (card_bin.empty()) {
            const auto error = m_input_support.error(
                core::error::AppErrorKind::INVALID_REQUEST,
                "missing card_bin"
            );
            error.send(sink);
            return;
        }
        auto result = m_runtime.get_pm_nt_eligibility_api(m_request_support->context(req));
        result.handle(sink);
    }

    void handle_network_token_status_check_api(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};

        const auto auth_error = m_request_support->authorize(
            req,
            "network_token_status_check_api",
            spec_for("network_token_status_check_api")
        );
        if (auth_error) {
            const auto& error = *auth_error;
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
        auto result =
            m_runtime.network_token_status_check_api(dto, m_request_support->context(req));
        result.handle(sink);
    }

    void handle_payment_method_get_token_details_api(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};

        const auto auth_error = m_request_support->authorize(
            req,
            "payment_method_get_token_details_api",
            spec_for("payment_method_get_token_details_api")
        );
        if (auth_error) {
            const auto& error = *auth_error;
            error.send(sink);
            return;
        }
        const std::string temporary_token = m_input_support.path_segment(req, 1);
        if (temporary_token.empty()) {
            const auto error =
                m_input_support.error(core::error::AppErrorKind::INVALID_REQUEST, "missing token");
            error.send(sink);
            return;
        }
        auto payment_method_id = m_input_support.token_payment_method(temporary_token);
        if (!payment_method_id) {
            const auto& error = payment_method_id.error();
            error.send(sink);
            return;
        }
        auto result =
            m_runtime.payment_method_get_token_details_api(m_request_support->context(req));
        result.handle(sink);
    }

    void handle_get_payment_method_token_data(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};

        const auto auth_error = m_request_support->authorize(
            req,
            "get_payment_method_token_data",
            spec_for("get_payment_method_token_data")
        );
        if (auth_error) {
            const auto& error = *auth_error;
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
        const auto record = m_input_support.find_payment_method(payment_method_id);
        if (!record) {
            const auto error = m_input_support.error(
                core::error::AppErrorKind::NOT_FOUND,
                "Payment method does not exist in our records"
            );
            error.send(sink);
            return;
        }
        const std::string token_type =
            ::app::http::RequestQuery{}(req).get("token_type").value_or("network_token");
        const auto& dto = *record;
        auto result = m_runtime.get_payment_method_token_data(dto, m_request_support->context(req));
        result.handle(sink);
    }

private:
    ::app::context::AppContext& m_context;
    PaymentMethodsInputSupport m_input_support;
    PaymentMethodsRuntime& m_runtime;
};

class PaymentMethodsRouteHandlers
{
public:
    explicit PaymentMethodsRouteHandlers(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_runtime{context},
        m_crud{context, m_runtime},
        m_migrate{context, m_runtime},
        m_sessions{context, m_runtime},
        m_tokenize{context, m_runtime},
        m_network{context, m_runtime},
        m_client_list{context, m_runtime},
        m_olap_support{context, m_runtime},
        m_input_support{context},
        m_policy{context, m_input_support},
        m_request_support{m_policy}
    {
        m_crud.bind_request_support(m_request_support);
        m_migrate.bind_request_support(m_request_support);
        m_sessions.bind_request_support(m_request_support);
        m_tokenize.bind_request_support(m_request_support);
        m_network.bind_request_support(m_request_support);
        m_client_list.bind_request_support(m_request_support);
        m_olap_support.bind_request_support(m_request_support);
    }

    void install_routes(core::router::RouterContext<>& router)
    {
        m_crud.install(router);
        m_client_list.install(router);
        m_migrate.install(router);
        m_tokenize.install(router);
        m_olap_support.install(router);
        m_network.install(router);
        m_sessions.install(router);
    }

private:
    ::app::context::AppContext& m_context;
    PaymentMethodsRuntime m_runtime;
    CrudHandlers m_crud;
    MigrateHandlers m_migrate;
    PaymentMethodSessionsHandlers m_sessions;
    TokenizeHandlers m_tokenize;
    NetworkTokenHandlers m_network;
    ClientListHandlers m_client_list;
    OlapSupportHandlers m_olap_support;
    PaymentMethodsInputSupport m_input_support;
    PaymentMethodsPolicy m_policy;
    PaymentMethodsRequestSupport m_request_support;
};

} // namespace app::handlers::payment_methods
