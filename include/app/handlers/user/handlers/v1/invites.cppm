export module app_handlers_user:handlers_v1_invites;

import std;
import interfaces;
import core_router;
import core_serde;
import utils_openapi;
import app_model;
import app_context;
import core_error;
import :input;
import :result;
import :runtime;
import :handlers_common;

export namespace app::handlers::user {
class UserHandlersV1Invites
{
public:
    UserHandlersV1Invites(
        app::context::AppContext& ctx,
        UserRuntime& runtime,
        UserInputSupport& input
    ) noexcept :
        m_context{ctx},
        m_runtime{runtime},
        m_input_support{input}
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
        if (const auto auth_error = m_input_support.authorize(req, "invite_multiple_user")) {
            res.set_status((*auth_error).get_status());
            res.set_body(serde::Ser::serialize("application/json", *auth_error));
            send();
            return;
        }
        const auto body = req.body_as_string();
        if (body.empty()) {
            auto deserialize_error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::VALIDATION_FAILED,
                "Failed to deserialize the JSON body into the target type"
            );
            deserialize_error.set_kind_name("IR_06");
            res.set_status(deserialize_error.get_status());
            res.set_body(serde::Ser::serialize("application/json", deserialize_error));
            send();
            return;
        }
        using Input = std::vector<app::model::user::InviteUserRequest>;
        auto parsed = serde::Ser::deserialize<Input>("application/json", body);
        if (!parsed) {
            auto deserialize_error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::VALIDATION_FAILED,
                "Failed to deserialize the JSON body into the target type"
            );
            deserialize_error.set_kind_name("IR_06");
            res.set_status(deserialize_error.get_status());
            res.set_body(serde::Ser::serialize("application/json", deserialize_error));
            send();
            return;
        }
        InviteMultipleUserInput input{
            m_input_support.request_data(req, req.body_as_string()),
            *parsed
        };
        auto result = m_runtime.invite_multiple_user(input);
        if (!result) {
            res.set_status((result.error()).get_status());
            res.set_body(serde::Ser::serialize("application/json", result.error()));
            send();
            return;
        }
        if (!result->get_set_cookie().empty()) {
            res.add_set_cookie(result->get_set_cookie());
        }
        res.set_status(interfaces::io::types::Status::OK);
        res.set_body(serde::Ser::serialize("application/json", result->get_document()));
        send();
    }

    void handle_resend_invite(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto auth_error = m_input_support.authorize(req, "resend_invite")) {
            res.set_status((*auth_error).get_status());
            res.set_body(serde::Ser::serialize("application/json", *auth_error));
            send();
            return;
        }
        const auto body = req.body_as_string();
        if (body.empty()) {
            auto deserialize_error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::VALIDATION_FAILED,
                "Failed to deserialize the JSON body into the target type"
            );
            deserialize_error.set_kind_name("IR_06");
            res.set_status(deserialize_error.get_status());
            res.set_body(serde::Ser::serialize("application/json", deserialize_error));
            send();
            return;
        }
        using Input = app::model::user::ReInviteUserRequest;
        auto parsed = serde::Ser::deserialize<Input>("application/json", body);
        if (!parsed) {
            auto deserialize_error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::VALIDATION_FAILED,
                "Failed to deserialize the JSON body into the target type"
            );
            deserialize_error.set_kind_name("IR_06");
            res.set_status(deserialize_error.get_status());
            res.set_body(serde::Ser::serialize("application/json", deserialize_error));
            send();
            return;
        }
        ResendInviteInput input{m_input_support.request_data(req, req.body_as_string()), *parsed};
        auto result = m_runtime.resend_invite(input);
        if (!result) {
            res.set_status((result.error()).get_status());
            res.set_body(serde::Ser::serialize("application/json", result.error()));
            send();
            return;
        }
        if (!result->get_set_cookie().empty()) {
            res.add_set_cookie(result->get_set_cookie());
        }
        res.set_status(interfaces::io::types::Status::OK);
        res.set_body(serde::Ser::serialize("application/json", result->get_document()));
        send();
    }

    void handle_accept_invite_from_email(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto auth_error = m_input_support.authorize(req, "accept_invite_from_email")) {
            res.set_status((*auth_error).get_status());
            res.set_body(serde::Ser::serialize("application/json", *auth_error));
            send();
            return;
        }
        const auto body = req.body_as_string();
        if (body.empty()) {
            auto deserialize_error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::VALIDATION_FAILED,
                "Failed to deserialize the JSON body into the target type"
            );
            deserialize_error.set_kind_name("IR_06");
            res.set_status(deserialize_error.get_status());
            res.set_body(serde::Ser::serialize("application/json", deserialize_error));
            send();
            return;
        }
        using Input = app::model::user::AcceptInviteFromEmailRequest;
        auto parsed = serde::Ser::deserialize<Input>("application/json", body);
        if (!parsed) {
            auto deserialize_error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::VALIDATION_FAILED,
                "Failed to deserialize the JSON body into the target type"
            );
            deserialize_error.set_kind_name("IR_06");
            res.set_status(deserialize_error.get_status());
            res.set_body(serde::Ser::serialize("application/json", deserialize_error));
            send();
            return;
        }
        AcceptInviteFromEmailInput input{
            m_input_support.request_data(req, req.body_as_string()),
            *parsed
        };
        auto result = m_runtime.accept_invite_from_email(input);
        if (!result) {
            res.set_status((result.error()).get_status());
            res.set_body(serde::Ser::serialize("application/json", result.error()));
            send();
            return;
        }
        if (!result->get_set_cookie().empty()) {
            res.add_set_cookie(result->get_set_cookie());
        }
        res.set_status(interfaces::io::types::Status::OK);
        res.set_body(serde::Ser::serialize("application/json", result->get_document()));
        send();
    }

    void handle_terminate_accept_invite(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto auth_error = m_input_support.authorize(req, "terminate_accept_invite")) {
            res.set_status((*auth_error).get_status());
            res.set_body(serde::Ser::serialize("application/json", *auth_error));
            send();
            return;
        }
        const auto body = req.body_as_string();
        if (body.empty()) {
            auto deserialize_error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::VALIDATION_FAILED,
                "Failed to deserialize the JSON body into the target type"
            );
            deserialize_error.set_kind_name("IR_06");
            res.set_status(deserialize_error.get_status());
            res.set_body(serde::Ser::serialize("application/json", deserialize_error));
            send();
            return;
        }
        auto parsed = serde::Ser::decode_generic("application/json", body);
        if (!parsed) {
            auto deserialize_error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::VALIDATION_FAILED,
                "Failed to deserialize the JSON body into the target type"
            );
            deserialize_error.set_kind_name("IR_06");
            res.set_status(deserialize_error.get_status());
            res.set_body(serde::Ser::serialize("application/json", deserialize_error));
            send();
            return;
        }
        TerminateAcceptInviteInput input{
            m_input_support.request_data(req, req.body_as_string()),
            *parsed
        };
        auto result = m_runtime.terminate_accept_invite(input);
        if (!result) {
            res.set_status((result.error()).get_status());
            res.set_body(serde::Ser::serialize("application/json", result.error()));
            send();
            return;
        }
        if (!result->get_set_cookie().empty()) {
            res.add_set_cookie(result->get_set_cookie());
        }
        res.set_status(interfaces::io::types::Status::OK);
        res.set_body(serde::Ser::serialize("application/json", result->get_document()));
        send();
    }

private:
    app::context::AppContext& m_context;
    UserRuntime& m_runtime;
    UserInputSupport& m_input_support;
};

} // namespace app::handlers::user
