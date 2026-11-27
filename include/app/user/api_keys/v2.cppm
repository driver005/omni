export module app_user:api_keys_handlers_v2;

import std;
import interfaces;
import core_router;
import core_serde;
import core_behavior;
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
import core_database;

import :api_keys_input;
import :api_keys_result;
import :api_keys_runtime;

export namespace app::handlers::api_keys {

class ApiKeysHandlersV2
{
public:
    ApiKeysHandlersV2(
        ::app::context::AppContext& context,
        ApiKeysRuntime& runtime,
        ApiKeysInputSupport& input_support,
        ApiKeysRequestSupport& request_support
    ) noexcept :
        m_context{context},
        m_runtime{runtime},
        m_input_support{input_support},
        m_support{request_support}
    {
    }

    void handle_api_key_create(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};
        if (const auto auth_error = m_support.authorize(req, "api_key_create", {})) {
            const auto& error = *auth_error;
            error.send(sink);
            return;
        }

        auto parsed =
            core::behavior::Serializer<::app::dto::api_keys::CreateApiKeyRequest>::pass(req, sink);
        if (!parsed) {
            return;
        }
        m_runtime.create(*parsed, m_support.context(req)).handle(sink);
    }

    void handle_api_key_retrieve(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};
        if (const auto auth_error = m_support.authorize(req, "api_key_retrieve", {})) {
            const auto& error = *auth_error;
            error.send(sink);
            return;
        }

        m_runtime.retrieve(m_support.context(req)).handle(sink);
    }

    void handle_api_key_update(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};
        if (const auto auth_error = m_support.authorize(req, "api_key_update", {})) {
            const auto& error = *auth_error;
            error.send(sink);
            return;
        }

        auto parsed =
            core::behavior::Serializer<::app::dto::api_keys::UpdateApiKeyRequest>::pass(req, sink);
        if (!parsed) {
            return;
        }
        m_runtime.update(*parsed, m_support.context(req)).handle(sink);
    }

    void handle_api_key_revoke(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};
        if (const auto auth_error = m_support.authorize(req, "api_key_revoke", {})) {
            const auto& error = *auth_error;
            error.send(sink);
            return;
        }

        m_runtime.revoke(m_support.context(req)).handle(sink);
    }

    void handle_api_key_list(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};
        if (const auto auth_error = m_support.authorize(req, "api_key_list", {})) {
            const auto& error = *auth_error;
            error.send(sink);
            return;
        }

        auto constraints = m_input_support.parse_constraints(req);
        m_runtime.list(constraints, m_support.context(req)).handle(sink);
    }

    void install(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;

        if constexpr (kV2) {
            // app.rs:2488-2497 — web::scope("/v2/api-keys"), resource("") etc.
            Router{router, "v2/api-keys"}
                .add_route(
                    Route{""}
                        .post(
                            [this](
                                interfaces::io::IRequest& req,
                                interfaces::io::IResponse& res,
                                std::function<void()> send
                            )
                            {
                                handle_api_key_create(req, res, std::move(send));
                            }
                        )
                        .summary("Hyperswitch api_key_create (v2)")
                )
                .add_route(
                    Route{"list"}
                        .get(
                            [this](
                                interfaces::io::IRequest& req,
                                interfaces::io::IResponse& res,
                                std::function<void()> send
                            )
                            {
                                handle_api_key_list(req, res, std::move(send));
                            }
                        )
                        .summary("Hyperswitch api_key_list (v2)")
                )
                .add_route(
                    Route{":key_id"}
                        .get(
                            [this](
                                interfaces::io::IRequest& req,
                                interfaces::io::IResponse& res,
                                std::function<void()> send
                            )
                            {
                                handle_api_key_retrieve(req, res, std::move(send));
                            }
                        )
                        .summary("Hyperswitch api_key_retrieve (v2)")
                )
                .add_route(
                    Route{":key_id"}
                        .put(
                            [this](
                                interfaces::io::IRequest& req,
                                interfaces::io::IResponse& res,
                                std::function<void()> send
                            )
                            {
                                handle_api_key_update(req, res, std::move(send));
                            }
                        )
                        .summary("Hyperswitch api_key_update (v2)")
                )
                .add_route(
                    Route{":key_id"}
                        .delt(
                            [this](
                                interfaces::io::IRequest& req,
                                interfaces::io::IResponse& res,
                                std::function<void()> send
                            )
                            {
                                handle_api_key_revoke(req, res, std::move(send));
                            }
                        )
                        .summary("Hyperswitch api_key_revoke (v2)")
                );
        }
    }

private:
    ::app::context::AppContext& m_context;
    ApiKeysRuntime& m_runtime;
    ApiKeysInputSupport& m_input_support;
    ApiKeysRequestSupport& m_support;
};

} // namespace app::handlers::api_keys
