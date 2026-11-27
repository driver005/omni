export module app_payment:payment_methods_handlers_tokenize;

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
        auto parsed = core::behavior::Serializer<::app::model::payment_methods::TokenizeCardRequest>::pass(req, sink);
        if (!parsed) return;
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
        auto parsed = core::behavior::Serializer<::app::model::payment_methods::TokenizePaymentMethodRequest>::pass(req, sink);
        if (!parsed) return;
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

} // namespace app::handlers::payment_methods
