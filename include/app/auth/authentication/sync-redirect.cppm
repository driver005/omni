export module app_auth:authentication_handlers_sync_redirect;

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
import app_context_core;
import app_connector;
import app_events;
import app_workflow;
import app_keystore;

import :authentication_input;
import :authentication_result;
import :authentication_runtime;

export namespace app::handlers::authentication {

class SyncRedirectHandlers
{
public:
    SyncRedirectHandlers(
        ::app::context::AppContext& ctx,
        AuthenticationRuntime& runtime,
        AuthenticationPolicy& policy,
        core::behavior::RequestSupport<AuthenticationPolicy>& request_support
    ) noexcept :
        m_context{ctx},
        m_runtime{runtime},
        m_policy{policy},
        m_request_support{request_support}
    {
    }

    void install(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "/authentication"}.add_router(
            Router{router, ":merchant_id"}.add_router(
                Router{router, ":authentication_id"}.add_route(
                    Route{"redirect"}
                        .post(
                            [this](
                                interfaces::io::IRequest& req,
                                interfaces::io::IResponse& res,
                                std::function<void()> send
                            )
                            {
                                handle_authentication_sync_post_update(req, res, std::move(send));
                            }
                        )
                        .summary("Hyperswitch authentication_sync_post_update")
                )
            )
        );
        Router{router, "/authentication"}.add_router(
            Router{router, ":merchant_id"}.add_router(
                Router{router, ":authentication_id"}.add_route(
                    Route{"redirect"}
                        .get(
                            [this](
                                interfaces::io::IRequest& req,
                                interfaces::io::IResponse& res,
                                std::function<void()> send
                            )
                            {
                                handle_authentication_sync_post_update(req, res, std::move(send));
                            }
                        )
                        .summary("Hyperswitch authentication_sync_post_update")
                )
            )
        );
        Router{router, "/authentication"}.add_router(
            Router{router, ":merchant_id"}.add_router(
                Router{router, ":authentication_id"}.add_route(
                    Route{"sync"}
                        .post(
                            [this](
                                interfaces::io::IRequest& req,
                                interfaces::io::IResponse& res,
                                std::function<void()> send
                            )
                            {
                                handle_authentication_sync(req, res, std::move(send));
                            }
                        )
                        .summary("Hyperswitch authentication_sync")
                )
            )
        );
    }

    // routes/authentication.rs:216-260 -> unified_authentication_service.rs:2070-2383
    void handle_authentication_sync(
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
            error.send(res, send);
            return;
        }
        using Input = ::app::dto::authentication::AuthenticationSyncRequest;
        auto parsed = serde::Ser::deserialize<Input>("application/json", body);
        if (!parsed) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INVALID_JSON,
                "invalid JSON"
            );
            error.send(res, send);
            return;
        }
        m_policy.set_body_secret(parsed->get_client_secret());
        if (const auto auth_error =
                m_request_support
                    .authorize(req, "authentication_sync", kAuthenticationSdkAuthSpec)) {
            auth_error->send(res, send);
            return;
        }
        const auto authorized = m_policy.authorized();
        const auto merchant_id = m_request_support.path(req, 1);
        const auto authentication_id = m_request_support.path(req, 2);
        if (merchant_id.empty() || authentication_id.empty()) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INVALID_REQUEST,
                "merchant id or authentication id missing in path"
            );
            error.send(res, send);
            return;
        }
        auto authentication_id_value = ::app::dto::authentication::AuthenticationIdValue{};
        authentication_id_value.set_value(authentication_id);
        parsed->set_authentication_id(authentication_id_value);
        AuthenticationSyncInput input{*parsed, merchant_id, authentication_id, authorized};
        auto result = m_runtime.sync(input);
        if (!result) {
            const auto& error = result.error();
            error.send(res, send);
            return;
        }
        auto encoded = result->serialize(sync_wire_fields());
        if (!encoded) {
            encoded.error().send(res, send);
            return;
        }
        res.set_status(interfaces::io::types::Status::OK);
        res.set_body(std::as_bytes(std::span{encoded->data(), encoded->size()}));
        send();
    }

    // routes/authentication.rs:264-288 -> unified_authentication_service.rs:2386-2496
    // Both POST and GET /redirect bind here (upstream route registration app.rs:3504-3506).
    void handle_authentication_sync_post_update(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto auth_error = m_request_support.authorize(
                req,
                "authentication_sync_post_update",
                kAuthenticationMerchantAuthSpec
            )) {
            auth_error->send(res, send);
            return;
        }
        const auto authorized = m_policy.authorized();
        const auto merchant_id = m_request_support.path(req, 1);
        const auto authentication_id = m_request_support.path(req, 2);
        if (merchant_id.empty() || authentication_id.empty()) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INVALID_REQUEST,
                "merchant id or authentication id missing in path"
            );
            error.send(res, send);
            return;
        }
        // MerchantIdAuth(merchant_id) authenticates the path merchant; a mismatch with the
        // authenticated merchant identity is FORBIDDEN.
        if (authorized.context.merchant_id() != merchant_id) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::FORBIDDEN,
                "merchant id does not match request path"
            );
            error.send(res, send);
            return;
        }
        AuthenticationSyncPostUpdateInput input{authentication_id, merchant_id, authorized};
        auto result = m_runtime.sync_post_update(input);
        if (!result) {
            const auto& error = result.error();
            error.send(res, send);
            return;
        }
        res.set_status(interfaces::io::types::Status::FOUND);
        res.set_header(interfaces::io::types::Token::LOCATION, result->second);
        res.set_body(serde::Ser::serialize("application/json", result->first));
        send();
    }

private:
    ::app::context::AppContext& m_context;
    AuthenticationRuntime& m_runtime;
    AuthenticationPolicy& m_policy;
    core::behavior::RequestSupport<AuthenticationPolicy>& m_request_support;
};

} // namespace app::handlers::authentication
