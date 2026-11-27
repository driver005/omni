export module app_auth:client_secret_handlers_v2;

import std;
import interfaces;
import core_router;
import core_behavior;
import core_serde;
import utils_openapi;
import app_model;
import app_dto;
import app_context;
import core_cache;
import core_error;
import app_context_core;
import app_http;
import app_crypto;
import app_metrics;
import app_keystore;
import app_locking;
import app_services;

import :client_secret_input;
import :client_secret_result;
import :client_secret_runtime;

export namespace app::handlers::client_secret {

/**
 * @brief C++ port of the upstream v2 client-secret resource.
 *
 * The two route functions are `routes/client_secret.rs:12-39` (create) and `:43-63`
 * (delete). Their core logic lives in `core/payments/helpers.rs:4694-4782`
 * (`make_client_secret`, `create_client_secret`, `delete_client_secret`) and the Redis
 * hash store in `db/client_secret.rs:49-151`. The runtime ports those functions using
 * the in-process facilities (cache = Redis hash, key store = platform key store,
 * repository = DB).
 */
class ClientSecretHandlersV2
{
public:
    ClientSecretHandlersV2(
        ::app::context::AppContext& context,
        ClientSecretRuntime& runtime,
        core::behavior::RequestSupport<ClientSecretPolicy>& request_support
    ) noexcept :
        m_context{context},
        m_runtime{runtime},
        m_request_support{request_support},
        m_input_support{}
    {
    }

    void handle_client_secret_create(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto auth_error =
                m_request_support.authorize(req, "client_secret_create", kClientSecretApiKeySpec)) {
            auth_error->send(res, send);
            return;
        }

        using Input = ::app::dto::client_secret::ClientSecretCreateRequest;
        const auto body = req.body_as_string();
        auto parsed = serde::Ser::deserialize<Input>("application/json", body);
        if (!parsed || !parsed->get_resource_id().is_customer()) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INVALID_JSON,
                "invalid JSON"
            );
            error.send(res, send);
            return;
        }

        ClientSecretCreateInput input{*parsed, m_request_support.scope(req).merchant_id, {}};
        auto result = m_runtime.create(input);
        if (!result) {
            const auto& error = result.error();
            error.send(res, send);
            return;
        }
        auto encoded = result->serialize(wire_fields());
        if (!encoded) {
            encoded.error().send(res, send);
            return;
        }
        res.set_status(interfaces::io::types::Status::OK);
        res.set_body(std::as_bytes(std::span{encoded->data(), encoded->size()}));
        send();
    }

    void handle_client_secret_delete(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto auth_error =
                m_request_support.authorize(req, "client_secret_delete", kClientSecretApiKeySpec)) {
            auth_error->send(res, send);
            return;
        }

        // web::Path<String> id — routes/client_secret.rs:46,49.
        const auto client_secret_id = m_input_support.client_secret_id_from_request(req);
        if (client_secret_id.empty()) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INVALID_REQUEST,
                "missing client secret id"
            );
            error.send(res, send);
            return;
        }

        ClientSecretDeleteInput input{client_secret_id, {}};
        auto result = m_runtime.delete_client_secret(input);
        if (!result) {
            const auto& error = result.error();
            error.send(res, send);
            return;
        }
        auto encoded = result->serialize(wire_fields());
        if (!encoded) {
            encoded.error().send(res, send);
            return;
        }
        res.set_status(interfaces::io::types::Status::OK);
        res.set_body(std::as_bytes(std::span{encoded->data(), encoded->size()}));
        send();
    }

    void install(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "v2/client-secret"}
            .add_route(
                Route{""}
                    .post(
                        [this](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
                            handle_client_secret_create(req, res, std::move(send));
                        }
                    )
                    .summary("Hyperswitch client_secret_create")
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
                            handle_client_secret_delete(req, res, std::move(send));
                        }
                    )
                    .summary("Hyperswitch client_secret_delete")
            );
    }

private:
    ::app::context::AppContext& m_context;
    ClientSecretRuntime& m_runtime;
    core::behavior::RequestSupport<ClientSecretPolicy>& m_request_support;
    ClientSecretInputSupport m_input_support;
};

} // namespace app::handlers::client_secret
