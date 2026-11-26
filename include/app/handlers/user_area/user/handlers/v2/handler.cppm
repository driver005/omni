export module app_user:user_handlers_v2;

import std;
import interfaces;
import core_router;
import core_serde;
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
        if (const auto auth_error = m_input_support.authorize(req, "user_signin")) {
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
        using Input = ::app::model::user::SignInRequest;
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
        UserSigninInput input{m_input_support.request_data(req, req.body_as_string()), *parsed};
        auto result = m_runtime.user_signin(input);
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

    void handle_verify_email(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto auth_error = m_input_support.authorize(req, "verify_email")) {
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
        using Input = ::app::model::user::VerifyEmailRequest;
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
        VerifyEmailInput input{m_input_support.request_data(req, req.body_as_string()), *parsed};
        auto result = m_runtime.verify_email(input);
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

    void handle_list_user_roles_details(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto auth_error = m_input_support.authorize(req, "list_user_roles_details")) {
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
        using Input = ::app::model::user::GetUserRoleDetailsRequest;
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
        ListUserRolesDetailsInput input{
            m_input_support.request_data(req, req.body_as_string()),
            *parsed
        };
        auto result = m_runtime.list_user_roles_details(input);
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

    void handle_list_merchants_for_user_in_org(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto auth_error =
                m_input_support.authorize(req, "list_merchants_for_user_in_org")) {
            res.set_status((*auth_error).get_status());
            res.set_body(serde::Ser::serialize("application/json", *auth_error));
            send();
            return;
        }
        const auto body = std::string{R"({"op":"list_merchants_for_user_in_org"})"};
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
        ListMerchantsForUserInOrgInput input{
            m_input_support
                .request_data(req, std::string{R"({"op":"list_merchants_for_user_in_org"})"}),
            *parsed
        };
        auto result = m_runtime.list_merchants_for_user_in_org(input);
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
    ::app::context::AppContext& m_context;
    UserRuntime& m_runtime;
    UserInputSupport& m_input_support;
};

} // namespace app::handlers::user
