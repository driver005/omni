export module app_payment:payment_methods_handlers_client_list;

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

class ClientListHandlers
{
public:
    void bind_request_support(PaymentMethodsRequestSupport& support) noexcept
    {
        m_request_support = &support;
    }

    PaymentMethodsRequestSupport* m_request_support{nullptr};

    ClientListHandlers(::app::context::AppContext& ctx, PaymentMethodsRuntime& runtime) noexcept :
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
                Route{""}
                    .get(
                        [this](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
                            interfaces::io::ResponseSink sink{res, std::move(send)};

                            handle_list_payment_method_api(req, res, std::move(send));
                        }
                    )
                    .summary("Hyperswitch list_payment_method_api")
            )
            .add_route(
                Route{"filter"}
                    .get(
                        [this](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
                            interfaces::io::ResponseSink sink{res, std::move(send)};

                            handle_list_countries_currencies_for_connector_payment_method(
                                req,
                                res,
                                std::move(send)
                            );
                        }
                    )
                    .summary("Hyperswitch list_countries_currencies_for_connector_payment_method")
            )
            .add_route(
                Route{"batch"}
                    .get(
                        [this](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
                            interfaces::io::ResponseSink sink{res, std::move(send)};

                            handle_payment_methods_batch_retrieve_api(req, res, std::move(send));
                        }
                    )
                    .summary("Hyperswitch payment_methods_batch_retrieve_api")
            )
            .add_router(
                Router{router, "customers"}.add_route(
                    Route{"total-payment-methods"}
                        .get(
                            [this](
                                interfaces::io::IRequest& req,
                                interfaces::io::IResponse& res,
                                std::function<void()> send
                            )
                            {
                                interfaces::io::ResponseSink sink{res, std::move(send)};

                                handle_get_total_payment_method_count(req, res, std::move(send));
                            }
                        )
                        .summary("Hyperswitch get_total_payment_method_count")
                )
            )
            .add_router(
                Router{router, "customers"}.add_route(
                    Route{"payment_methods"}
                        .get(
                            [this](
                                interfaces::io::IRequest& req,
                                interfaces::io::IResponse& res,
                                std::function<void()> send
                            )
                            {
                                interfaces::io::ResponseSink sink{res, std::move(send)};

                                handle_list_customer_payment_method_api_client(
                                    req,
                                    res,
                                    std::move(send)
                                );
                            }
                        )
                        .summary("Hyperswitch list_customer_payment_method_api_client")
                )
            )
            .add_router(
                Router{router, "customers"}.add_router(
                    Router{router, ":id"}.add_route(
                        Route{"saved-payment-methods"}
                            .get(
                                [this](
                                    interfaces::io::IRequest& req,
                                    interfaces::io::IResponse& res,
                                    std::function<void()> send
                                )
                                {
                                    interfaces::io::ResponseSink sink{res, std::move(send)};

                                    handle_list_customer_payment_method_api(
                                        req,
                                        res,
                                        std::move(send)
                                    );
                                }
                            )
                            .summary("Hyperswitch list_customer_payment_method_api")
                    )
                )
            )
            .add_router(
                Router{router, "customers"}.add_router(
                    Router{router, ":customer_id"}.add_route(
                        Route{"payment_methods"}
                            .get(
                                [this](
                                    interfaces::io::IRequest& req,
                                    interfaces::io::IResponse& res,
                                    std::function<void()> send
                                )
                                {
                                    interfaces::io::ResponseSink sink{res, std::move(send)};

                                    handle_list_customer_payment_method_api(
                                        req,
                                        res,
                                        std::move(send)
                                    );
                                }
                            )
                            .summary("Hyperswitch list_customer_payment_method_api")
                    )
                )
            )
            .add_router(
                Router{router, "payments"}.add_router(
                    Router{router, ":payment_id"}.add_route(
                        Route{"client"}
                            .get(
                                [this](
                                    interfaces::io::IRequest& req,
                                    interfaces::io::IResponse& res,
                                    std::function<void()> send
                                )
                                {
                                    interfaces::io::ResponseSink sink{res, std::move(send)};

                                    handle_list_payment_methods_for_payments_client(
                                        req,
                                        res,
                                        std::move(send)
                                    );
                                }
                            )
                            .summary("Hyperswitch list_payment_methods_for_payments_client")
                    )
                )
            );

        Router{router, ":pm_prefix/payment-methods"}
            .add_route(
                Route{""}
                    .get(
                        [this](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
                            interfaces::io::ResponseSink sink{res, std::move(send)};

                            handle_list_payment_method_api(req, res, std::move(send));
                        }
                    )
                    .summary("Hyperswitch list_payment_method_api (v2 prefix)")
            )
            .add_route(
                Route{"filter"}
                    .get(
                        [this](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
                            interfaces::io::ResponseSink sink{res, std::move(send)};

                            handle_list_countries_currencies_for_connector_payment_method(
                                req,
                                res,
                                std::move(send)
                            );
                        }
                    )
                    .summary(
                        "Hyperswitch list_countries_currencies_for_connector_payment_method (v2 "
                        "prefix)"
                    )
            );
    }

    void handle_list_payment_method_api(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};

        const auto auth_error = m_request_support->authorize(
            req,
            "list_payment_method_api",
            spec_for("list_payment_method_api")
        );
        if (auth_error) {
            const auto& error = *auth_error;
            error.send(sink);
            return;
        }
        if (const auto sdk_error = m_request_support->authorize(
                req,
                "list_payment_method_api",
                client_facing_spec_for("list_payment_method_api")
            )) {
            const auto& error = *sdk_error;
            error.send(sink);
            return;
        }
        const auto query = ::app::http::RequestQuery{}(req);
        std::optional<std::string> client_secret{};
        if (const auto value = query.get("client_secret")) {
            client_secret = *value;
        }
        std::optional<std::string> amount{};
        if (const auto value = query.get("amount")) {
            amount = *value;
        }
        std::size_t limit{20};
        if (const auto value = query.get("limit")) {
            std::int64_t parsed_limit{};
            const auto converted =
                std::from_chars(value->data(), value->data() + value->size(), parsed_limit);
            if (converted.ec == std::errc{} && parsed_limit > 0) {
                limit = static_cast<std::size_t>(parsed_limit);
            }
        }
        const std::string merchant_id = ::app::http::merchant_id_from_header{}(req);
        if (const auto secret_error = m_input_support.check_client_secret("list", client_secret)) {
            const auto& error = *secret_error;
            error.send(sink);
            return;
        }
        auto result = m_runtime.list_payment_method_api(m_request_support->context(req));
        result.handle(sink);
    }

    void handle_list_customer_payment_method_api(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};

        const auto auth_error = m_request_support->authorize(
            req,
            "list_customer_payment_method_api",
            spec_for("list_customer_payment_method_api")
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
        const std::string customer_id = m_input_support.path_segment(req, 1);
        if (customer_id.empty()) {
            const auto error = m_input_support.error(
                core::error::AppErrorKind::INVALID_REQUEST,
                "missing customer id"
            );
            error.send(sink);
            return;
        }
        if (!m_input_support.customer_exists(customer_id)) {
            const auto error = m_input_support.error(
                core::error::AppErrorKind::NOT_FOUND,
                "Customer does not exist in our records"
            );
            error.send(sink);
            return;
        }
        const std::string merchant_id = ::app::http::merchant_id_from_header{}(req);
        auto result = m_runtime.list_customer_payment_method_api(m_request_support->context(req));
        result.handle(sink);
    }

    void handle_list_customer_payment_method_api_client(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};

        const auto auth_error = m_request_support->authorize(
            req,
            "list_customer_payment_method_api_client",
            spec_for("list_customer_payment_method_api_client")
        );
        if (auth_error) {
            const auto& error = *auth_error;
            error.send(sink);
            return;
        }
        if (const auto sdk_error = m_request_support->authorize(
                req,
                "list_customer_payment_method_api_client",
                client_facing_spec_for("list_customer_payment_method_api_client")
            )) {
            const auto& error = *sdk_error;
            error.send(sink);
            return;
        }
        const auto query = ::app::http::RequestQuery{}(req);
        const std::string customer_id = query.get("customer_id").value_or("");
        const std::string client_secret = query.get("client_secret").value_or("");
        if (!customer_id.empty() && !m_input_support.customer_exists(customer_id)) {
            const auto error = m_input_support.error(
                core::error::AppErrorKind::NOT_FOUND,
                "Customer does not exist in our records"
            );
            error.send(sink);
            return;
        }
        const auto presented =
            client_secret.empty() ? std::nullopt : std::optional<std::string>{client_secret};
        if (const auto secret_error = m_input_support.check_client_secret(customer_id, presented)) {
            const auto& error = *secret_error;
            error.send(sink);
            return;
        }
        auto result =
            m_runtime.list_customer_payment_method_api_client(m_request_support->context(req));
        result.handle(sink);
    }

    void handle_get_total_payment_method_count(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};

        const auto auth_error = m_request_support->authorize(
            req,
            "get_total_payment_method_count",
            spec_for("get_total_payment_method_count")
        );
        if (auth_error) {
            const auto& error = *auth_error;
            error.send(sink);
            return;
        }
        const auto query = ::app::http::RequestQuery{}(req);
        const std::string customer_id = query.get("customer_id").value_or("");
        const std::string merchant_id = ::app::http::merchant_id_from_header{}(req);
        if (!customer_id.empty() && !m_input_support.customer_exists(customer_id)) {
            const auto error = m_input_support.error(
                core::error::AppErrorKind::NOT_FOUND,
                "Customer does not exist in our records"
            );
            error.send(sink);
            return;
        }
        auto result = m_runtime.get_total_payment_method_count(m_request_support->context(req));
        result.handle(sink);
    }

    void handle_list_countries_currencies_for_connector_payment_method(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};

        const auto auth_error = m_request_support->authorize(
            req,
            "list_countries_currencies_for_connector_payment_method",
            spec_for("list_countries_currencies_for_connector_payment_method")
        );
        if (auth_error) {
            const auto& error = *auth_error;
            error.send(sink);
            return;
        }
        const auto query = ::app::http::RequestQuery{}(req);
        const std::string connector = query.get("connector").value_or("");
        const std::string payment_method_type = query.get("payment_method_type").value_or("");
        auto result = m_runtime.list_countries_currencies_for_connector_payment_method(
            m_request_support->context(req)
        );
        result.handle(sink);
    }

    void handle_payment_methods_batch_retrieve_api(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};

        const auto auth_error = m_request_support->authorize(
            req,
            "payment_methods_batch_retrieve_api",
            spec_for("payment_methods_batch_retrieve_api")
        );
        if (auth_error) {
            const auto& error = *auth_error;
            error.send(sink);
            return;
        }
        const auto body = req.body_as_string();
        const auto rows = m_input_support.batch_retrieve_rows(body);
        if (rows.empty()) {
            const auto error = m_input_support.error(
                core::error::AppErrorKind::INVALID_REQUEST,
                "No payment_method_ids provided"
            );
            error.send(sink);
            return;
        }
        if (rows.size() > 200) {
            const auto error = m_input_support.error(
                core::error::AppErrorKind::INVALID_REQUEST,
                "A maximum of 200 payment_method_ids are allowed per request"
            );
            error.send(sink);
            return;
        }
        auto result = m_runtime.payment_methods_batch_retrieve_api(m_request_support->context(req));
        result.handle(sink);
    }

    void handle_list_payment_methods_for_payments_client(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};

        const auto auth_error = m_request_support->authorize(
            req,
            "list_payment_methods_for_payments_client",
            spec_for("list_payment_methods_for_payments_client")
        );
        if (auth_error) {
            const auto& error = *auth_error;
            error.send(sink);
            return;
        }
        if (const auto sdk_error = m_request_support->authorize(
                req,
                "list_payment_methods_for_payments_client",
                client_facing_spec_for("list_payment_methods_for_payments_client")
            )) {
            const auto& error = *sdk_error;
            error.send(sink);
            return;
        }
        const std::string payment_id = m_input_support.path_segment(req, 1);
        if (payment_id.empty()) {
            const auto error = m_input_support.error(
                core::error::AppErrorKind::INVALID_REQUEST,
                "missing payment id"
            );
            error.send(sink);
            return;
        }
        const std::string merchant_id = ::app::http::merchant_id_from_header{}(req);
        if (const auto access_error =
                m_input_support.payment_access_error(payment_id, merchant_id)) {
            const auto& error = *access_error;
            error.send(sink);
            return;
        }
        const std::string client_secret =
            ::app::http::RequestQuery{}(req).get("client_secret").value_or("");
        const auto presented =
            client_secret.empty() ? std::nullopt : std::optional<std::string>{client_secret};
        if (const auto secret_error = m_input_support.check_client_secret(payment_id, presented)) {
            const auto& error = *secret_error;
            error.send(sink);
            return;
        }
        auto result =
            m_runtime.list_payment_methods_for_payments_client(m_request_support->context(req));
        result.handle(sink);
    }

private:
    ::app::context::AppContext& m_context;
    PaymentMethodsInputSupport m_input_support;
    PaymentMethodsRuntime& m_runtime;
};

} // namespace app::handlers::payment_methods
