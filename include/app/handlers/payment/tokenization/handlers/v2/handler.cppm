export module app_payment:tokenization_handlers_v2;

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
        const auto body = req.body_as_string();
        if (body.empty()) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::EMPTY_REQUEST,
                "empty request"
            );
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }
        const auto request = m_input_support.parse_create_request(body);
        if (!request) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INVALID_JSON,
                "invalid JSON"
            );
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }

        // routes/tokenization.rs:42 — customer_id is read before api_or_client_auth.
        const std::string customer_id = request->get_customer_id();
        if (customer_id.empty()) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::VALIDATION_FAILED,
                "missing customer id"
            );
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }

        // api_or_client_auth(V2ApiKeyAuth{false,false},
        //   V2ClientAuth(ResourceId::Customer(customer_id))) — routes/tokenization.rs:56-65.
        const auto authorized =
            m_input_support.authorize_request(req, "create_token_vault_api", customer_id);
        if (authorized.error) {
            const auto& error = *authorized.error;
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }

        // Flow::TokenizationCreate, LockAction::NotApplicable — routes/tokenization.rs:40,66.
        m_context.metrics().record_flow("TokenizationCreate");
        m_context.metrics().counter("TOKENIZATION_CREATED").increment();

        CreateTokenVaultInput input{
            *request,
            m_input_support.resolve_merchant_id(req, authorized.context)
        };
        auto result = m_runtime.create_token_vault(input);
        if (!result) {
            const auto& error = result.error();
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }
        res.set_status(interfaces::io::types::Status::OK);
        res.set_body(serde::Ser::serialize("application/json", *result));
        send();
    }

    void handle_delete_tokenized_data_api(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto body = req.body_as_string();
        if (body.empty()) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::EMPTY_REQUEST,
                "empty request"
            );
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }
        const auto request = m_input_support.parse_delete_request(body);
        if (!request) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INVALID_JSON,
                "invalid JSON"
            );
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }

        // routes/tokenization.rs:81 — session_id is read before api_or_client_auth.
        if (request->get_session_id().empty()) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::VALIDATION_FAILED,
                "missing session id"
            );
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }

        // routes/tokenization.rs:82 — GlobalTokenId comes from web::Path, not the body.
        const auto token_id = m_input_support.token_id_from_path(req);
        if (!token_id) {
            const auto& error = token_id.error();
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }

        // api_or_client_auth(V2ApiKeyAuth{false,false},
        //   V2ClientAuth(ResourceId::PaymentMethodSession(session_id))) — routes:92-101.
        const auto authorized = m_input_support.authorize_request(
            req,
            "delete_tokenized_data_api",
            request->get_session_id()
        );
        if (authorized.error) {
            const auto& error = *authorized.error;
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }

        // Flow::TokenizationDelete, LockAction::NotApplicable — routes/tokenization.rs:79,102.
        m_context.metrics().record_flow("TokenizationDelete");
        m_context.metrics().counter("TOKENIZATION_DELETED").increment();

        DeleteTokenizedDataInput input{*token_id, request->get_customer_id()};
        auto result = m_runtime.delete_tokenized_data(input);
        if (!result) {
            const auto& error = result.error();
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }
        res.set_status(interfaces::io::types::Status::OK);
        res.set_body(serde::Ser::serialize("application/json", *result));
        send();
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
