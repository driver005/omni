export module app_user:user_handlers_v1_invites;

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
class UserHandlersV1Invites
{
public:
    UserHandlersV1Invites(
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
                Router{router, "user"}.add_router(
                    Router{router, "user"}.add_route(
                        Route{"invite_multiple"}
                            .post(
                                [this](
                                    interfaces::io::IRequest& req,
                                    interfaces::io::IResponse& res,
                                    std::function<void()> send
                                )
                                {
                                    handle_invite_multiple_user(req, res, std::move(send));
                                }
                            )
                            .summary("Hyperswitch invite_multiple_user")
                    )
                )
            )
            .add_router(
                Router{router, "user"}.add_router(
                    Router{router, "user"}.add_route(
                        Route{"resend_invite"}
                            .post(
                                [this](
                                    interfaces::io::IRequest& req,
                                    interfaces::io::IResponse& res,
                                    std::function<void()> send
                                )
                                {
                                    handle_resend_invite(req, res, std::move(send));
                                }
                            )
                            .summary("Hyperswitch resend_invite")
                    )
                )
            )
            .add_router(
                Router{router, "user"}.add_route(
                    Route{"accept_invite_from_email"}
                        .post(
                            [this](
                                interfaces::io::IRequest& req,
                                interfaces::io::IResponse& res,
                                std::function<void()> send
                            )
                            {
                                handle_accept_invite_from_email(req, res, std::move(send));
                            }
                        )
                        .summary("Hyperswitch accept_invite_from_email")
                )
            )
            .add_router(
                Router{router, "user"}.add_route(
                    Route{"terminate_accept_invite"}
                        .post(
                            [this](
                                interfaces::io::IRequest& req,
                                interfaces::io::IResponse& res,
                                std::function<void()> send
                            )
                            {
                                handle_terminate_accept_invite(req, res, std::move(send));
                            }
                        )
                        .summary("Hyperswitch terminate_accept_invite")
                )
            );
    }

    void handle_invite_multiple_user(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};
        if (const auto auth_error = m_support.authorize(req, "invite_multiple_user", {})) {
            sink.set_status((*auth_error).get_status());
            sink.set_body(serde::Ser::serialize("application/json", *auth_error));
            sink.send();
            return;
        }
        const auto body = req.body_as_string();
        if (body.empty()) {
            auto deserialize_error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::VALIDATION_FAILED,
                "Failed to deserialize the JSON body into the target type"
            );
            deserialize_error.set_kind_name("IR_06");
            deserialize_error.send(sink);
            return;
        }
        using Input = std::vector<::app::model::user::InviteUserRequest>;
        auto parsed = serde::Ser::deserialize<Input>("application/json", body);
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
        m_runtime.invite_multiple_user(*parsed, ctx).handle(sink);
    }

    void handle_resend_invite(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};
        if (const auto auth_error = m_support.authorize(req, "resend_invite", {})) {
            sink.set_status((*auth_error).get_status());
            sink.set_body(serde::Ser::serialize("application/json", *auth_error));
            sink.send();
            return;
        }
        const auto body = req.body_as_string();
        auto parsed =
            core::behavior::Serializer<::app::model::user::ReInviteUserRequest>::pass(req, sink);
        if (!parsed) {
            return;
        }
        auto ctx = m_support.context(req);
        m_runtime.resend_invite(*parsed, ctx).handle(sink);
    }

    void handle_accept_invite_from_email(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};
        if (const auto auth_error = m_support.authorize(req, "accept_invite_from_email", {})) {
            sink.set_status((*auth_error).get_status());
            sink.set_body(serde::Ser::serialize("application/json", *auth_error));
            sink.send();
            return;
        }
        const auto body = req.body_as_string();
        auto parsed =
            core::behavior::Serializer<::app::model::user::AcceptInviteFromEmailRequest>::pass(
                req,
                sink
            );
        if (!parsed) {
            return;
        }
        auto ctx = m_support.context(req);
        m_runtime.accept_invite_from_email(*parsed, ctx).handle(sink);
    }

    void handle_terminate_accept_invite(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};
        if (const auto auth_error = m_support.authorize(req, "terminate_accept_invite", {})) {
            sink.set_status((*auth_error).get_status());
            sink.set_body(serde::Ser::serialize("application/json", *auth_error));
            sink.send();
            return;
        }
        const auto body = req.body_as_string();
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
        m_runtime.terminate_accept_invite(*parsed, ctx).handle(sink);
    }

private:
    ::app::context::AppContext& m_context;
    UserRuntime& m_runtime;
    UserInputSupport& m_input_support;
    UserRequestSupport& m_support;
};

} // namespace app::handlers::user
