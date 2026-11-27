export module app_payment:tokenization_handlers_v2;

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
import app_keystore;

import :tokenization_input;
import :tokenization_result;
import :tokenization_runtime;

export namespace app::handlers::tokenization {

/**
 * @brief C++ port of the upstream v2 tokenization resource.
 *
 * Both routes live under `web::scope("/v2/tokenize")` in the original
 * `routes/tokenization.rs`. Core logic stays in the shared `TokenizationRuntime`;
 * this class only holds the thin request/response adapters and route table.
 */
class TokenizationHandlersV2
{
public:
    void bind_request_support(TokenizationRequestSupport& support) noexcept
    {
        m_request_support = &support;
    }

    TokenizationRequestSupport* m_request_support{nullptr};

    void bind_policy(TokenizationPolicy& p) noexcept
    {
        m_policy = &p;
    }

    TokenizationPolicy* m_policy{nullptr};

    TokenizationHandlersV2(
        ::app::context::AppContext& context,
        TokenizationRuntime& runtime
    ) noexcept :
        m_context{context},
        m_runtime{runtime},
        m_input_support{context}
    {
    }

    void handle_create_token_vault_api(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};

        const auto body = req.body_as_string();
        if (body.empty()) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::EMPTY_REQUEST,
                "empty request"
            );
            error.send(sink);
            return;
        }
        const auto request = m_input_support.parse_create_request(body);
        if (!request) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INVALID_JSON,
                "invalid JSON"
            );
            error.send(sink);
            return;
        }

        // routes/tokenization.rs:42 — customer_id is read before api_or_client_auth.
        const std::string customer_id = request->get_customer_id();
        if (customer_id.empty()) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::VALIDATION_FAILED,
                "missing customer id"
            );
            error.send(sink);
            return;
        }

        // api_or_client_auth(V2ApiKeyAuth{false,false},
        //   V2ClientAuth(ResourceId::Customer(customer_id))) — routes/tokenization.rs:56-65.
        const auto auth_error =
            m_request_support->authorize(req, "create_token_vault_api", spec_for(customer_id));
        if (auth_error) {
            const auto& error = *auth_error;
            error.send(sink);
            return;
        }

        // Flow::TokenizationCreate, LockAction::NotApplicable — routes/tokenization.rs:40,66.
        m_context.metrics().record_flow("TokenizationCreate");
        m_context.metrics().counter("TOKENIZATION_CREATED").increment();

        const auto& dto = *request;
        auto result = m_runtime.create_token_vault(dto, m_request_support->context(req));
        result.handle(sink);
    }

    void handle_delete_tokenized_data_api(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};

        const auto body = req.body_as_string();
        if (body.empty()) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::EMPTY_REQUEST,
                "empty request"
            );
            error.send(sink);
            return;
        }
        const auto request = m_input_support.parse_delete_request(body);
        if (!request) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INVALID_JSON,
                "invalid JSON"
            );
            error.send(sink);
            return;
        }

        // routes/tokenization.rs:81 — session_id is read before api_or_client_auth.
        if (request->get_session_id().empty()) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::VALIDATION_FAILED,
                "missing session id"
            );
            error.send(sink);
            return;
        }

        // routes/tokenization.rs:82 — GlobalTokenId comes from web::Path, not the body.
        const auto token_id = m_input_support.token_id_from_path(req);
        if (!token_id) {
            const auto& error = token_id.error();
            error.send(sink);
            return;
        }

        // api_or_client_auth(V2ApiKeyAuth{false,false},
        //   V2ClientAuth(ResourceId::PaymentMethodSession(session_id))) — routes:92-101.
        const auto auth_error = m_request_support->authorize(
            req,
            "delete_tokenized_data_api",
            spec_for(request->get_session_id())
        );
        if (auth_error) {
            const auto& error = *auth_error;
            error.send(sink);
            return;
        }

        // Flow::TokenizationDelete, LockAction::NotApplicable — routes/tokenization.rs:79,102.
        m_context.metrics().record_flow("TokenizationDelete");
        m_context.metrics().counter("TOKENIZATION_DELETED").increment();

        auto result = m_runtime.delete_tokenized_data(m_request_support->context(req));
        result.handle(sink);
    }

    void install(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "v2/tokenize"}
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

                            handle_create_token_vault_api(req, res, std::move(send));
                        }
                    )
                    .summary("Hyperswitch create_token_vault_api")
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

                            handle_delete_tokenized_data_api(req, res, std::move(send));
                        }
                    )
                    .summary("Hyperswitch delete_tokenized_data_api")
            );
    }

private:
    ::app::context::AppContext& m_context;
    TokenizationRuntime& m_runtime;
    TokenizationInputSupport m_input_support;
};

} // namespace app::handlers::tokenization
