export module app_user:api_keys_handlers_v1;

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
import core_database;

import :api_keys_input;
import :api_keys_result;
import :api_keys_runtime;

export namespace app::handlers::api_keys {

class ApiKeysHandlersV1
{
public:
    ApiKeysHandlersV1(
        ::app::context::AppContext& context,
        ApiKeysRuntime& runtime,
        ApiKeysInputSupport& input_support
    ) noexcept :
        m_context{context},
        m_runtime{runtime},
        m_input_support{input_support}
    {
    }

    void handle_api_key_create(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto auth_error = m_input_support.authorize_operation(
                req,
                "api_key_create",
                m_input_support.permission_for("api_key_create", req)
            )) {
            const auto& error = *auth_error;
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }

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
        using Input = ::app::dto::api_keys::CreateApiKeyRequest;
        auto parsed = serde::Ser::deserialize<Input>("application/json", body);
        if (!parsed) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INVALID_JSON,
                "invalid JSON"
            );
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }

        const std::string merchant_id = m_input_support.merchant_from_request(req);
        if (merchant_id.empty()) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::UNAUTHORIZED,
                "missing merchant id"
            );
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }

        ApiKeyCreateInput input{*parsed, merchant_id};
        auto result = m_runtime.create(input);
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

    void handle_api_key_retrieve(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto auth_error = m_input_support.authorize_operation(
                req,
                "api_key_retrieve",
                m_input_support.permission_for("api_key_retrieve", req)
            )) {
            const auto& error = *auth_error;
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }

        const std::string merchant_id = m_input_support.merchant_from_request(req);
        const std::string key_id = m_input_support.key_id_from_request(req);
        if (key_id.empty()) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INVALID_REQUEST,
                "missing key id"
            );
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }

        ApiKeyRetrieveInput input{merchant_id, key_id};
        auto result = m_runtime.retrieve(input);
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

    void handle_api_key_update(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto auth_error = m_input_support.authorize_operation(
                req,
                "api_key_update",
                m_input_support.permission_for("api_key_update", req)
            )) {
            const auto& error = *auth_error;
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }

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
        using Input = ::app::dto::api_keys::UpdateApiKeyRequest;
        auto parsed = serde::Ser::deserialize<Input>("application/json", body);
        if (!parsed) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INVALID_JSON,
                "invalid JSON"
            );
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }

        const std::string merchant_id = m_input_support.merchant_from_request(req);
        const std::string key_id = m_input_support.key_id_from_request(req);
        if (key_id.empty()) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INVALID_REQUEST,
                "missing key id"
            );
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }

        ApiKeyUpdateInput input{*parsed, merchant_id, key_id};
        auto result = m_runtime.update(input);
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

    void handle_api_key_revoke(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto auth_error = m_input_support.authorize_operation(
                req,
                "api_key_revoke",
                m_input_support.permission_for("api_key_revoke", req)
            )) {
            const auto& error = *auth_error;
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }

        const std::string merchant_id = m_input_support.merchant_from_request(req);
        const std::string key_id = m_input_support.key_id_from_request(req);
        if (key_id.empty()) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INVALID_REQUEST,
                "missing key id"
            );
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }

        ApiKeyRevokeInput input{merchant_id, key_id};
        auto result = m_runtime.revoke(input);
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

    void handle_api_key_list(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto auth_error = m_input_support.authorize_operation(
                req,
                "api_key_list",
                m_input_support.permission_for("api_key_list", req)
            )) {
            const auto& error = *auth_error;
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }

        const std::string merchant_id = m_input_support.merchant_from_request(req);
        ApiKeyListInput input{merchant_id, m_input_support.parse_constraints(req)};
        auto result = m_runtime.list(input);
        if (!result) {
            const auto& error = result.error();
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }
        res.set_status(interfaces::io::types::Status::OK);
        res.set_body(serde::Ser::serialize("application/json", result->get_keys()));
        send();
    }

    void install(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;

        if constexpr (kV1) {
            // app.rs:2504-2513 — web::scope("/api_keys/{merchant_id}")
            Router{router, "api_keys"}
                .add_route(
                    Route{":merchant_id"}
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
                        .summary("Hyperswitch api_key_create")
                )
                .add_router(
                    Router{router, ":merchant_id"}.add_route(
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
                            .summary("Hyperswitch api_key_list")
                    )
                )
                .add_router(
                    Router{router, ":merchant_id"}.add_route(
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
                            .summary("Hyperswitch api_key_retrieve")
                    )
                )
                .add_router(
                    Router{router, ":merchant_id"}.add_route(
                        Route{":key_id"}
                            .post(
                                [this](
                                    interfaces::io::IRequest& req,
                                    interfaces::io::IResponse& res,
                                    std::function<void()> send
                                )
                                {
                                    handle_api_key_update(req, res, std::move(send));
                                }
                            )
                            .summary("Hyperswitch api_key_update")
                    )
                )
                .add_router(
                    Router{router, ":merchant_id"}.add_route(
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
                            .summary("Hyperswitch api_key_revoke")
                    )
                );
        }
    }

private:
    ::app::context::AppContext& m_context;
    ApiKeysRuntime& m_runtime;
    ApiKeysInputSupport& m_input_support;
};

} // namespace app::handlers::api_keys
