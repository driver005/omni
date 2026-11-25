export module app_handlers_user:handlers_v1_internal;

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
class UserHandlersV1Internal
{
public:
    UserHandlersV1Internal(
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
                    Router{router, "internal"}.add_router(
                        Router{router, "user"}.add_route(
                            Route{":user_id"}
                                .get(
                                    [this](
                                        interfaces::io::IRequest& req,
                                        interfaces::io::IResponse& res,
                                        std::function<void()> send
                                    )
                                    {
                                        handle_get_user_details_internal(req, res, std::move(send));
                                    }
                                )
                                .summary("Hyperswitch get_user_details_internal")
                        )
                    )
                )
            )
            .add_router(
                Router{router, "user"}.add_router(
                    Router{router, "internal"}.add_router(
                        Router{router, "user"}.add_route(
                            Route{"list"}
                                .post(
                                    [this](
                                        interfaces::io::IRequest& req,
                                        interfaces::io::IResponse& res,
                                        std::function<void()> send
                                    )
                                    {
                                        handle_list_users_internal(req, res, std::move(send));
                                    }
                                )
                                .summary("Hyperswitch list_users_internal")
                        )
                    )
                )
            )
            .add_router(
                Router{router, "user"}.add_router(
                    Router{router, "internal"}.add_route(
                        Route{"members"}
                            .get(
                                [this](
                                    interfaces::io::IRequest& req,
                                    interfaces::io::IResponse& res,
                                    std::function<void()> send
                                )
                                {
                                    handle_list_members_for_entity(req, res, std::move(send));
                                }
                            )
                            .summary("Hyperswitch list_members_for_entity")
                    )
                )
            )
            .add_router(
                Router{router, "user"}.add_router(
                    Router{router, "internal"}.add_route(
                        Route{"authorize"}
                            .post(
                                [this](
                                    interfaces::io::IRequest& req,
                                    interfaces::io::IResponse& res,
                                    std::function<void()> send
                                )
                                {
                                    handle_authorize_token(req, res, std::move(send));
                                }
                            )
                            .summary("Hyperswitch authorize_token")
                    )
                )
            );
    }

    void handle_get_user_details_internal(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto auth_error = m_input_support.authorize(req, "get_user_details_internal")) {
            res.set_status((*auth_error).get_status());
            res.set_body(serde::Ser::serialize("application/json", *auth_error));
            send();
            return;
        }
        const auto body = std::string{R"({"op":"get_user_details_internal"})"};
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
        GetUserDetailsInternalInput input{
            m_input_support.request_data(req, std::string{R"({"op":"get_user_details_internal"})"}),
            *parsed
        };
        auto result = m_runtime.get_user_details_internal(input);
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

    void handle_list_users_internal(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto auth_error = m_input_support.authorize(req, "list_users_internal")) {
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
        using Input = app::model::user::ListUsersInternalRequest;
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
        ListUsersInternalInput input{
            m_input_support.request_data(req, req.body_as_string()),
            *parsed
        };
        auto result = m_runtime.list_users_internal(input);
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

    void handle_list_members_for_entity(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto auth_error = m_input_support.authorize(req, "list_members_for_entity")) {
            res.set_status((*auth_error).get_status());
            res.set_body(serde::Ser::serialize("application/json", *auth_error));
            send();
            return;
        }
        const auto body = std::string{R"({"op":"list_members_for_entity"})"};
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
        ListMembersForEntityInput input{
            m_input_support.request_data(req, std::string{R"({"op":"list_members_for_entity"})"}),
            *parsed
        };
        auto result = m_runtime.list_members_for_entity(input);
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

    void handle_authorize_token(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto auth_error = m_input_support.authorize(req, "authorize_token")) {
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
        using Input = app::model::user::AuthorizeTokenRequest;
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
        AuthorizeTokenInput input{m_input_support.request_data(req, req.body_as_string()), *parsed};
        auto result = m_runtime.authorize_token(input);
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
