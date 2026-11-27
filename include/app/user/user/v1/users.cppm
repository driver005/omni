export module app_user:user_handlers_v1_users;

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
class UserHandlersV1Users
{
public:
    UserHandlersV1Users(
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
                    Route{"internal_signup"}
                        .post(
                            [this](
                                interfaces::io::IRequest& req,
                                interfaces::io::IResponse& res,
                                std::function<void()> send
                            )
                            {
                                handle_internal_user_signup(req, res, std::move(send));
                            }
                        )
                        .summary("Hyperswitch internal_user_signup")
                )
            )
            .add_router(
                Router{router, "user"}.add_route(
                    Route{"tenant_signup"}
                        .post(
                            [this](
                                interfaces::io::IRequest& req,
                                interfaces::io::IResponse& res,
                                std::function<void()> send
                            )
                            {
                                handle_create_tenant_user(req, res, std::move(send));
                            }
                        )
                        .summary("Hyperswitch create_tenant_user")
                )
            )
            .add_router(
                Router{router, "user"}.add_route(
                    Route{"create_platform"}
                        .post(
                            [this](
                                interfaces::io::IRequest& req,
                                interfaces::io::IResponse& res,
                                std::function<void()> send
                            )
                            {
                                handle_create_platform(req, res, std::move(send));
                            }
                        )
                        .summary("Hyperswitch create_platform")
                )
            )
            .add_router(
                Router{router, "user"}.add_route(
                    Route{"sample_data"}
                        .post(
                            [this](
                                interfaces::io::IRequest& req,
                                interfaces::io::IResponse& res,
                                std::function<void()> send
                            )
                            {
                                handle_generate_sample_data(req, res, std::move(send));
                            }
                        )
                        .summary("Hyperswitch generate_sample_data")
                )
            )
            .add_router(
                Router{router, "user"}.add_route(
                    Route{"sample_data"}
                        .delt(
                            [this](
                                interfaces::io::IRequest& req,
                                interfaces::io::IResponse& res,
                                std::function<void()> send
                            )
                            {
                                handle_delete_sample_data(req, res, std::move(send));
                            }
                        )
                        .summary("Hyperswitch delete_sample_data")
                )
            )
            .add_router(
                Router{router, "user"}.add_route(
                    Route{"user"}
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
                        .summary("Hyperswitch list_user_roles_details")
                )
            )
            .add_router(
                Router{router, "user"}.add_route(
                    Route{"update"}
                        .post(
                            [this](
                                interfaces::io::IRequest& req,
                                interfaces::io::IResponse& res,
                                std::function<void()> send
                            )
                            {
                                handle_update_user_account_details(req, res, std::move(send));
                            }
                        )
                        .summary("Hyperswitch update_user_account_details")
                )
            )
            .add_router(
                Router{router, "user"}.add_route(
                    Route{"from_email"}
                        .post(
                            [this](
                                interfaces::io::IRequest& req,
                                interfaces::io::IResponse& res,
                                std::function<void()> send
                            )
                            {
                                handle_user_from_email(req, res, std::move(send));
                            }
                        )
                        .summary("Hyperswitch user_from_email")
                )
            );
    }

    void handle_internal_user_signup(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};
        if (const auto auth_error = m_support.authorize(req, "internal_user_signup", {})) {
            sink.set_status((*auth_error).get_status());
            sink.set_body(serde::Ser::serialize("application/json", *auth_error));
            sink.send();
            return;
        }
        const auto body = req.body_as_string();
        auto parsed = core::behavior::Serializer<::app::model::user::CreateInternalUserRequest>::pass(req, sink);
        if (!parsed) return;
        auto ctx = m_support.context(req);
        m_runtime.internal_user_signup(*parsed, ctx).handle(sink);
    }

    void handle_create_tenant_user(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};
        if (const auto auth_error = m_support.authorize(req, "create_tenant_user", {})) {
            sink.set_status((*auth_error).get_status());
            sink.set_body(serde::Ser::serialize("application/json", *auth_error));
            sink.send();
            return;
        }
        const auto body = req.body_as_string();
        auto parsed = core::behavior::Serializer<::app::model::user::CreateTenantUserRequest>::pass(req, sink);
        if (!parsed) return;
        auto ctx = m_support.context(req);
        m_runtime.create_tenant_user(*parsed, ctx).handle(sink);
    }

    void handle_create_platform(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};
        if (const auto auth_error = m_support.authorize(req, "create_platform", {})) {
            sink.set_status((*auth_error).get_status());
            sink.set_body(serde::Ser::serialize("application/json", *auth_error));
            sink.send();
            return;
        }
        const auto body = req.body_as_string();
        auto parsed = core::behavior::Serializer<::app::model::user::PlatformAccountCreateRequest>::pass(req, sink);
        if (!parsed) return;
        auto ctx = m_support.context(req);
        m_runtime.create_platform(*parsed, ctx).handle(sink);
    }

    void handle_generate_sample_data(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};
        if (const auto auth_error = m_support.authorize(req, "generate_sample_data", {})) {
            sink.set_status((*auth_error).get_status());
            sink.set_body(serde::Ser::serialize("application/json", *auth_error));
            sink.send();
            return;
        }
        const auto body = req.body_as_string();
        auto parsed = core::behavior::Serializer<::app::model::user::SampleDataRequest>::pass(req, sink);
        if (!parsed) return;
        auto ctx = m_support.context(req);
        m_runtime.generate_sample_data(*parsed, ctx).handle(sink);
    }

    void handle_delete_sample_data(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};
        if (const auto auth_error = m_support.authorize(req, "delete_sample_data", {})) {
            sink.set_status((*auth_error).get_status());
            sink.set_body(serde::Ser::serialize("application/json", *auth_error));
            sink.send();
            return;
        }
        const auto body = req.body_as_string();
        auto parsed = core::behavior::Serializer<::app::model::user::SampleDataRequest>::pass(req, sink);
        if (!parsed) return;
        auto ctx = m_support.context(req);
        m_runtime.delete_sample_data(*parsed, ctx).handle(sink);
    }

    void handle_list_user_roles_details(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};
        if (const auto auth_error =
                m_support.authorize(req, "list_user_roles_details", {})) {
            sink.set_status((*auth_error).get_status());
            sink.set_body(serde::Ser::serialize("application/json", *auth_error));
            sink.send();
            return;
        }
        const auto body = req.body_as_string();
        auto parsed = core::behavior::Serializer<::app::model::user::GetUserRoleDetailsRequest>::pass(req, sink);
        if (!parsed) return;
        auto ctx = m_support.context(req);
        m_runtime.list_user_roles_details(*parsed, ctx).handle(sink);
    }

    void handle_update_user_account_details(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};
        if (const auto auth_error =
                m_support.authorize(req, "update_user_account_details", {})) {
            sink.set_status((*auth_error).get_status());
            sink.set_body(serde::Ser::serialize("application/json", *auth_error));
            sink.send();
            return;
        }
        const auto body = req.body_as_string();
        auto parsed = core::behavior::Serializer<::app::model::user::UpdateUserAccountDetailsRequest>::pass(req, sink);
        if (!parsed) return;
        auto ctx = m_support.context(req);
        m_runtime.update_user_account_details(*parsed, ctx).handle(sink);
    }

    void handle_user_from_email(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};
        if (const auto auth_error = m_support.authorize(req, "user_from_email", {})) {
            sink.set_status((*auth_error).get_status());
            sink.set_body(serde::Ser::serialize("application/json", *auth_error));
            sink.send();
            return;
        }
        auto parsed = core::behavior::Serializer<::app::model::user::UserFromEmailRequest>::pass(req, sink);
        if (!parsed) return;
        auto ctx = m_support.context(req);
        m_runtime.user_from_email(*parsed, ctx).handle(sink);
    }

private:
    ::app::context::AppContext& m_context;
    UserRuntime& m_runtime;
    UserInputSupport& m_input_support;
    UserRequestSupport& m_support;
};

} // namespace app::handlers::user
