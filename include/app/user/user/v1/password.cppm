export module app_user:user_handlers_v1_password;

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
class UserHandlersV1Password
{
public:
    UserHandlersV1Password(
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
                    Route{"rotate_password"}
                        .post(
                            [this](
                                interfaces::io::IRequest& req,
                                interfaces::io::IResponse& res,
                                std::function<void()> send
                            )
                            {
                                handle_rotate_password(req, res, std::move(send));
                            }
                        )
                        .summary("Hyperswitch rotate_password")
                )
            )
            .add_router(
                Router{router, "user"}.add_route(
                    Route{"forgot_password"}
                        .post(
                            [this](
                                interfaces::io::IRequest& req,
                                interfaces::io::IResponse& res,
                                std::function<void()> send
                            )
                            {
                                handle_forgot_password(req, res, std::move(send));
                            }
                        )
                        .summary("Hyperswitch forgot_password")
                )
            )
            .add_router(
                Router{router, "user"}.add_route(
                    Route{"reset_password"}
                        .post(
                            [this](
                                interfaces::io::IRequest& req,
                                interfaces::io::IResponse& res,
                                std::function<void()> send
                            )
                            {
                                handle_reset_password(req, res, std::move(send));
                            }
                        )
                        .summary("Hyperswitch reset_password")
                )
            )
            .add_route(
                Route{"verify_email"}
                    .post(
                        [this](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
                            handle_verify_email(req, res, std::move(send));
                        }
                    )
                    .summary("Hyperswitch verify_email")
            )
            .add_router(
                Router{router, "user"}.add_route(
                    Route{"verify_email_request"}
                        .post(
                            [this](
                                interfaces::io::IRequest& req,
                                interfaces::io::IResponse& res,
                                std::function<void()> send
                            )
                            {
                                handle_verify_email_request(req, res, std::move(send));
                            }
                        )
                        .summary("Hyperswitch verify_email_request")
                )
            );
    }

    void handle_rotate_password(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};
        if (const auto auth_error = m_support.authorize(req, "rotate_password", {})) {
            sink.set_status((*auth_error).get_status());
            sink.set_body(serde::Ser::serialize("application/json", *auth_error));
            sink.send();
            return;
        }
        const auto body = req.body_as_string();
        auto parsed =
            core::behavior::Serializer<::app::model::user::RotatePasswordRequest>::pass(req, sink);
        if (!parsed) {
            return;
        }
        auto ctx = m_support.context(req);
        m_runtime.rotate_password(*parsed, ctx).handle(sink);
    }

    void handle_forgot_password(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};
        if (const auto auth_error = m_support.authorize(req, "forgot_password", {})) {
            sink.set_status((*auth_error).get_status());
            sink.set_body(serde::Ser::serialize("application/json", *auth_error));
            sink.send();
            return;
        }
        const auto body = req.body_as_string();
        auto parsed =
            core::behavior::Serializer<::app::model::user::ForgotPasswordRequest>::pass(req, sink);
        if (!parsed) {
            return;
        }
        auto ctx = m_support.context(req);
        m_runtime.forgot_password(*parsed, ctx).handle(sink);
    }

    void handle_reset_password(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};
        if (const auto auth_error = m_support.authorize(req, "reset_password", {})) {
            sink.set_status((*auth_error).get_status());
            sink.set_body(serde::Ser::serialize("application/json", *auth_error));
            sink.send();
            return;
        }
        const auto body = req.body_as_string();
        auto parsed =
            core::behavior::Serializer<::app::model::user::ResetPasswordRequest>::pass(req, sink);
        if (!parsed) {
            return;
        }
        auto ctx = m_support.context(req);
        m_runtime.reset_password(*parsed, ctx).handle(sink);
    }

    void handle_verify_email(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};
        if (const auto auth_error = m_support.authorize(req, "verify_email", {})) {
            sink.set_status((*auth_error).get_status());
            sink.set_body(serde::Ser::serialize("application/json", *auth_error));
            sink.send();
            return;
        }
        const auto body = req.body_as_string();
        auto parsed =
            core::behavior::Serializer<::app::model::user::VerifyEmailRequest>::pass(req, sink);
        if (!parsed) {
            return;
        }
        auto ctx = m_support.context(req);
        m_runtime.verify_email(*parsed, ctx).handle(sink);
    }

    void handle_verify_email_request(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};
        if (const auto auth_error = m_support.authorize(req, "verify_email_request", {})) {
            sink.set_status((*auth_error).get_status());
            sink.set_body(serde::Ser::serialize("application/json", *auth_error));
            sink.send();
            return;
        }
        auto parsed =
            core::behavior::Serializer<::app::model::user::SendVerifyEmailRequest>::pass(req, sink);
        if (!parsed) {
            return;
        }
        auto ctx = m_support.context(req);
        m_runtime.verify_email_request(*parsed, ctx).handle(sink);
    }

private:
    ::app::context::AppContext& m_context;
    UserRuntime& m_runtime;
    UserInputSupport& m_input_support;
    UserRequestSupport& m_support;
};

} // namespace app::handlers::user
