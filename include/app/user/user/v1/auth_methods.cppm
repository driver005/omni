export module app_user:user_handlers_v1_auth_methods;

import std;
import interfaces;
import core_router;
import core_serde;
import core_behavior;
import utils_openapi;
import app_model;
import app_context;
import core_error;
import :user_input;
import :user_result;
import :user_runtime;
import :user_handlers_common;

export namespace app::handlers::user {
class UserHandlersV1AuthMethods
{
public:
    UserHandlersV1AuthMethods(
        ::app::context::AppContext& ctx,
        UserRuntime& runtime,
        UserInputSupport& input,
        UserRequestSupport& request_support
    ) noexcept :
        m_context{ctx},
        m_runtime{runtime},
        m_input_support{input},
        m_support{request_support}
    {
    }

    void install(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "user"}
            .add_router(
                Router{router, "auth"}.add_route(
                    Route{"url"}
                        .get(
                            [this](
                                interfaces::io::IRequest& req,
                                interfaces::io::IResponse& res,
                                std::function<void()> send
                            )
                            {
                                handle_get_sso_auth_url(req, res, std::move(send));
                            }
                        )
                        .summary("Hyperswitch get_sso_auth_url")
                )
            )
            .add_route(
                Route{"oidc"}
                    .post(
                        [this](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
                            handle_sso_sign(req, res, std::move(send));
                        }
                    )
                    .summary("Hyperswitch sso_sign")
            )
            .add_route(
                Route{"auth"}
                    .post(
                        [this](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
                            handle_create_user_authentication_method(req, res, std::move(send));
                        }
                    )
                    .summary("Hyperswitch create_user_authentication_method")
            )
            .add_route(
                Route{"auth"}
                    .put(
                        [this](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
                            handle_update_user_authentication_method(req, res, std::move(send));
                        }
                    )
                    .summary("Hyperswitch update_user_authentication_method")
            )
            .add_router(
                Router{router, "auth"}.add_route(
                    Route{"list"}
                        .get(
                            [this](
                                interfaces::io::IRequest& req,
                                interfaces::io::IResponse& res,
                                std::function<void()> send
                            )
                            {
                                handle_list_user_authentication_methods(req, res, std::move(send));
                            }
                        )
                        .summary("Hyperswitch list_user_authentication_methods")
                )
            )
            .add_router(
                Router{router, "auth"}.add_route(
                    Route{"select"}
                        .post(
                            [this](
                                interfaces::io::IRequest& req,
                                interfaces::io::IResponse& res,
                                std::function<void()> send
                            )
                            {
                                handle_terminate_auth_select(req, res, std::move(send));
                            }
                        )
                        .summary("Hyperswitch terminate_auth_select")
                )
            )
            .add_router(
                Router{router, "user"}.add_router(
                    Router{router, "key"}.add_route(
                        Route{"transfer"}
                            .post(
                                [this](
                                    interfaces::io::IRequest& req,
                                    interfaces::io::IResponse& res,
                                    std::function<void()> send
                                )
                                {
                                    handle_transfer_user_key(req, res, std::move(send));
                                }
                            )
                            .summary("Hyperswitch transfer_user_key")
                    )
                )
            );
    }

    void handle_get_sso_auth_url(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};
        if (const auto auth_error = m_support.authorize(req, "get_sso_auth_url", {})) {
            sink.set_status((*auth_error).get_status());
            sink.set_body(serde::Ser::serialize("application/json", *auth_error));
            sink.send();
            return;
        }
        const auto body = std::string{R"({"op":"get_sso_auth_url"})"};
        if (body.empty()) {
            auto deserialize_error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::VALIDATION_FAILED,
                "Failed to deserialize the JSON body into the target type"
            );
            deserialize_error.set_kind_name("IR_06");
            deserialize_error.send(sink);
            return;
        }
        auto parsed = serde::Ser::decode_generic("application/json", body);
        if (!parsed) {
            auto deserialize_error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::VALIDATION_FAILED,
                "Failed to deserialize the JSON body into the target type"
            );
            deserialize_error.set_kind_name("IR_06");
            deserialize_error.send(sink);
            return;
        }
        auto ctx = m_support.context(req);
        m_runtime.get_sso_auth_url(*parsed, ctx).handle(sink);
    }

    void handle_sso_sign(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};
        if (const auto auth_error = m_support.authorize(req, "sso_sign", {})) {
            sink.set_status((*auth_error).get_status());
            sink.set_body(serde::Ser::serialize("application/json", *auth_error));
            sink.send();
            return;
        }
        const auto body = req.body_as_string();
        auto parsed = core::behavior::Serializer<::app::model::user::SsoSignInRequest>::pass(req, sink);
        if (!parsed) return;
        auto ctx = m_support.context(req);
        m_runtime.sso_sign(*parsed, ctx).handle(sink);
    }

    void handle_create_user_authentication_method(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};
        if (const auto auth_error =
                m_support.authorize(req, "create_user_authentication_method", {})) {
            sink.set_status((*auth_error).get_status());
            sink.set_body(serde::Ser::serialize("application/json", *auth_error));
            sink.send();
            return;
        }
        const auto body = req.body_as_string();
        auto parsed = core::behavior::Serializer<::app::model::user::CreateUserAuthenticationMethodRequest>::pass(req, sink);
        if (!parsed) return;
        auto ctx = m_support.context(req);
        m_runtime.create_user_authentication_method(*parsed, ctx).handle(sink);
    }

    void handle_update_user_authentication_method(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};
        if (const auto auth_error =
                m_support.authorize(req, "update_user_authentication_method", {})) {
            sink.set_status((*auth_error).get_status());
            sink.set_body(serde::Ser::serialize("application/json", *auth_error));
            sink.send();
            return;
        }
        const auto body = req.body_as_string();
        auto parsed = core::behavior::Serializer<::app::model::user::AuthSelectRequest>::pass(req, sink);
        if (!parsed) return;
        auto ctx = m_support.context(req);
        m_runtime.terminate_auth_select(*parsed, ctx).handle(sink);
    }

    void handle_transfer_user_key(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};
        if (const auto auth_error = m_support.authorize(req, "transfer_user_key", {})) {
            sink.set_status((*auth_error).get_status());
            sink.set_body(serde::Ser::serialize("application/json", *auth_error));
            sink.send();
            return;
        }
        auto parsed = core::behavior::Serializer<::app::model::user::UserKeyTransferRequest>::pass(req, sink);
        if (!parsed) return;
        auto ctx = m_support.context(req);
        m_runtime.transfer_user_key(*parsed, ctx).handle(sink);
    }

private:
    ::app::context::AppContext& m_context;
    UserRuntime& m_runtime;
    UserInputSupport& m_input_support;
    UserRequestSupport& m_support;
};

} // namespace app::handlers::user
