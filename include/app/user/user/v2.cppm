export module app_user:user_handlers_v2;

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
class UserHandlersV2
{
public:
    UserHandlersV2(
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
                Router{router, "v2"}.add_route(
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
                        .summary("Hyperswitch user_signin (v2 alias)")
                )
            )
            .add_router(
                Router{router, "v2"}.add_route(
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
                        .summary("Hyperswitch verify_email (v2 alias)")
                )
            )
            .add_router(
                Router{router, "user"}.add_router(
                    Router{router, "user"}.add_route(
                        Route{"v2"}
                            .post(
                                [this](
                                    interfaces::io::IRequest& req,
                                    interfaces::io::IResponse& res,
                                    std::function<void()> send
                                )
                                {
                                    handle_list_user_roles_details(req, res, std::move(send));
                                }
                            )
                            .summary("Hyperswitch list_user_roles_details (v2 alias)")
                    )
                )
            );
        Router{router, "v2"}.add_router(
            Router{router, "user"}.add_router(
                Router{router, "list"}.add_route(
                    Route{"merchant"}
                        .get(
                            [this](
                                interfaces::io::IRequest& req,
                                interfaces::io::IResponse& res,
                                std::function<void()> send
                            )
                            {
                                handle_list_merchants_for_user_in_org(req, res, std::move(send));
                            }
                        )
                        .summary("Hyperswitch list_merchants_for_user_in_org (v2 alias)")
                )
            )
        );
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

    void handle_list_user_roles_details(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};
        if (const auto auth_error = m_support.authorize(req, "list_user_roles_details", {})) {
            sink.set_status((*auth_error).get_status());
            sink.set_body(serde::Ser::serialize("application/json", *auth_error));
            sink.send();
            return;
        }
        const auto body = req.body_as_string();
        auto parsed =
            core::behavior::Serializer<::app::model::user::GetUserRoleDetailsRequest>::pass(
                req,
                sink
            );
        if (!parsed) {
            return;
        }
        auto ctx = m_support.context(req);
        m_runtime.list_user_roles_details(*parsed, ctx).handle(sink);
    }

    void handle_list_merchants_for_user_in_org(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};
        if (const auto auth_error =
                m_support.authorize(req, "list_merchants_for_user_in_org", {})) {
            sink.set_status((*auth_error).get_status());
            sink.set_body(serde::Ser::serialize("application/json", *auth_error));
            sink.send();
            return;
        }
        const auto body = std::string{R"({"op":"list_merchants_for_user_in_org"})"};
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
        m_runtime.list_merchants_for_user_in_org(*parsed, ctx).handle(sink);
    }

private:
    ::app::context::AppContext& m_context;
    UserRuntime& m_runtime;
    UserInputSupport& m_input_support;
    UserRequestSupport& m_support;
};

} // namespace app::handlers::user
