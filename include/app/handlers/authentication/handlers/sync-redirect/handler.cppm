export module app_handlers_authentication:handlers_sync_redirect;

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

import :input;
import :result;
import :runtime;

export namespace app::handlers::authentication {

class SyncRedirectHandlers
{
public:
    SyncRedirectHandlers(app::context::AppContext& ctx, AuthenticationRuntime& runtime) noexcept :
        m_context{ctx},
        m_input_support{ctx},
        m_runtime{runtime}
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
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }
        using Input = app::dto::authentication::AuthenticationSyncRequest;
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
        const auto authorized =
            m_input_support
                .authorize_sdk(req, "authentication_sync", body, parsed->get_client_secret());
        if (authorized.error) {
            const auto& error = *authorized.error;
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }
        const auto path = m_input_support.merchant_and_auth_id_from_path(req);
        if (!path.has_value()) {
            const auto& error = path.error();
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }
        auto authentication_id_value = app::dto::authentication::AuthenticationIdValue{};
        authentication_id_value.set_value(path->second);
        parsed->set_authentication_id(authentication_id_value);
        AuthenticationSyncInput input{*parsed, path->first, path->second, authorized};
        auto result = m_runtime.sync(input);
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

    // routes/authentication.rs:264-288 -> unified_authentication_service.rs:2386-2496
    // Both POST and GET /redirect bind here (upstream route registration app.rs:3504-3506).
    void handle_authentication_sync_post_update(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto authorized =
            m_input_support.authorize_merchant(req, "authentication_sync_post_update");
        if (authorized.error) {
            const auto& error = *authorized.error;
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }
        const auto path = m_input_support.merchant_and_auth_id_from_path(req);
        if (!path.has_value()) {
            const auto& error = path.error();
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }
        // MerchantIdAuth(merchant_id) authenticates the path merchant; a mismatch with the
        // authenticated merchant identity is FORBIDDEN.
        if (authorized.context.merchant_id() != path->first) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::FORBIDDEN,
                "merchant id does not match request path"
            );
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }
        AuthenticationSyncPostUpdateInput input{path->second, path->first, authorized};
        auto result = m_runtime.sync_post_update(input);
        if (!result) {
            const auto& error = result.error();
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }
        res.set_status(interfaces::io::types::Status::FOUND);
        res.set_header(interfaces::io::types::Token::LOCATION, result->get_location());
        res.set_body(serde::Ser::serialize("application/json", result->get_document()));
        send();
    }

private:
    app::context::AppContext& m_context;
    AuthenticationInputSupport m_input_support;
    AuthenticationRuntime& m_runtime;
};

} // namespace app::handlers::authentication
