export module app_user:user_handlers_v1_auth;

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
class UserHandlersV1Auth
{
public:
    UserHandlersV1Auth(
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
                Router{router, "user"}.add_route(
                    Route{"signup_with_merchant_id"}
                        .post(
                            [this](
                                interfaces::io::IRequest& req,
                                interfaces::io::IResponse& res,
                                std::function<void()> send
                            )
                            {
                                handle_user_signup_with_merchant_id(req, res, std::move(send));
                            }
                        )
                        .summary("Hyperswitch user_signup_with_merchant_id")
                )
            )
            .add_route(
                Route{"signup"}
                    .post(
                        [this](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
                            handle_user_signup(req, res, std::move(send));
                        }
                    )
                    .summary("Hyperswitch user_signup")
            )
            .add_route(
                Route{"signin"}
                    .post(
                        [this](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
                            handle_user_signin(req, res, std::move(send));
                        }
                    )
                    .summary("Hyperswitch user_signin")
            )
            .add_router(
                Router{router, "user"}.add_route(
                    Route{"connect_account"}
                        .post(
                            [this](
                                interfaces::io::IRequest& req,
                                interfaces::io::IResponse& res,
                                std::function<void()> send
                            )
                            {
                                handle_user_connect_account(req, res, std::move(send));
                            }
                        )
                        .summary("Hyperswitch user_connect_account")
                )
            )
            .add_router(
                Router{router, "user"}.add_route(
                    Route{"signout"}
                        .post(
                            [this](
                                interfaces::io::IRequest& req,
                                interfaces::io::IResponse& res,
                                std::function<void()> send
                            )
                            {
                                handle_signout(req, res, std::move(send));
                            }
                        )
                        .summary("Hyperswitch signout")
                )
            )
            .add_router(
                Router{router, "user"}.add_route(
                    Route{"change_password"}
                        .post(
                            [this](
                                interfaces::io::IRequest& req,
                                interfaces::io::IResponse& res,
                                std::function<void()> send
                            )
                            {
                                handle_change_password(req, res, std::move(send));
                            }
                        )
                        .summary("Hyperswitch change_password")
                )
            );
    }

    void handle_user_signup_with_merchant_id(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};
        if (const auto auth_error = m_support.authorize(req, "user_signup_with_merchant_id", {})) {
            sink.set_status((*auth_error).get_status());
            sink.set_body(serde::Ser::serialize("application/json", *auth_error));
            sink.send();
            return;
        }
        const auto body = req.body_as_string();
        auto parsed =
            core::behavior::Serializer<::app::model::user::SignUpWithMerchantIdRequest>::pass(
                req,
                sink
            );
        if (!parsed) {
            return;
        }
        auto ctx = m_support.context(req);
        m_runtime.user_signup_with_merchant_id(*parsed, ctx).handle(sink);
    }

    void handle_user_signup(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};
        if (const auto auth_error = m_support.authorize(req, "user_signup", {})) {
            sink.set_status((*auth_error).get_status());
            sink.set_body(serde::Ser::serialize("application/json", *auth_error));
            sink.send();
            return;
        }
        const auto body = req.body_as_string();
        auto parsed =
            core::behavior::Serializer<::app::model::user::SignUpRequest>::pass(req, sink);
        if (!parsed) {
            return;
        }
        auto ctx = m_support.context(req);
        m_runtime.user_signup(*parsed, ctx).handle(sink);
    }

    void handle_user_signin(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};
        if (const auto auth_error = m_support.authorize(req, "user_signin", {})) {
            sink.set_status((*auth_error).get_status());
            sink.set_body(serde::Ser::serialize("application/json", *auth_error));
            sink.send();
            return;
        }
        const auto body = req.body_as_string();
        auto parsed =
            core::behavior::Serializer<::app::model::user::SignInRequest>::pass(req, sink);
        if (!parsed) {
            return;
        }
        auto ctx = m_support.context(req);
        m_runtime.user_signin(*parsed, ctx).handle(sink);
    }

    void handle_user_connect_account(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};
        if (const auto auth_error = m_support.authorize(req, "user_connect_account", {})) {
            sink.set_status((*auth_error).get_status());
            sink.set_body(serde::Ser::serialize("application/json", *auth_error));
            sink.send();
            return;
        }
        const auto body = req.body_as_string();
        auto parsed =
            core::behavior::Serializer<::app::model::user::ConnectAccountRequest>::pass(req, sink);
        if (!parsed) {
            return;
        }
        auto ctx = m_support.context(req);
        m_runtime.user_connect_account(*parsed, ctx).handle(sink);
    }

    void handle_signout(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};
        if (const auto auth_error = m_support.authorize(req, "signout", {})) {
            sink.set_status((*auth_error).get_status());
            sink.set_body(serde::Ser::serialize("application/json", *auth_error));
            sink.send();
            return;
        }
        const auto body = std::string{R"({"op":"signout"})"};
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
        m_runtime.signout(*parsed, ctx).handle(sink);
    }

    void handle_change_password(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};
        if (const auto auth_error = m_support.authorize(req, "change_password", {})) {
            sink.set_status((*auth_error).get_status());
            sink.set_body(serde::Ser::serialize("application/json", *auth_error));
            sink.send();
            return;
        }
        auto parsed =
            core::behavior::Serializer<::app::model::user::ChangePasswordRequest>::pass(req, sink);
        if (!parsed) {
            return;
        }
        auto ctx = m_support.context(req);
        m_runtime.change_password(*parsed, ctx).handle(sink);
    }

private:
    ::app::context::AppContext& m_context;
    UserRuntime& m_runtime;
    UserInputSupport& m_input_support;
    UserRequestSupport& m_support;
};

} // namespace app::handlers::user
