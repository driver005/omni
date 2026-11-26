export module app_user:user_handlers_v1_dashboard;

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
class UserHandlersV1Dashboard
{
public:
    UserHandlersV1Dashboard(
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
            .add_route(
                Route{""}
                    .get(
                        [this](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
                            handle_get_active_user_details(req, res, std::move(send));
                        }
                    )
                    .summary("Hyperswitch get_active_user_details")
            )
            .add_router(
                Router{router, "user"}.add_route(
                    Route{"launch_sage"}
                        .post(
                            [this](
                                interfaces::io::IRequest& req,
                                interfaces::io::IResponse& res,
                                std::function<void()> send
                            )
                            {
                                handle_launch_sage(req, res, std::move(send));
                            }
                        )
                        .summary("Hyperswitch launch_sage")
                )
            )
            .add_router(
                Router{router, "user"}.add_route(
                    Route{"data"}
                        .post(
                            [this](
                                interfaces::io::IRequest& req,
                                interfaces::io::IResponse& res,
                                std::function<void()> send
                            )
                            {
                                handle_set_dashboard_metadata(req, res, std::move(send));
                            }
                        )
                        .summary("Hyperswitch set_dashboard_metadata")
                )
            )
            .add_route(
                Route{"data"}
                    .get(
                        [this](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
                            handle_get_multiple_dashboard_metadata(req, res, std::move(send));
                        }
                    )
                    .summary("Hyperswitch get_multiple_dashboard_metadata")
            );
    }

    void handle_get_active_user_details(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto auth_error = m_input_support.authorize(req, "get_active_user_details")) {
            res.set_status((*auth_error).get_status());
            res.set_body(serde::Ser::serialize("application/json", *auth_error));
            send();
            return;
        }
        const auto body = std::string{R"({"op":"get_active_user_details"})"};
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
        GetActiveUserDetailsInput input{
            m_input_support.request_data(req, std::string{R"({"op":"get_active_user_details"})"}),
            *parsed
        };
        auto result = m_runtime.get_active_user_details(input);
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

    void handle_launch_sage(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto auth_error = m_input_support.authorize(req, "launch_sage")) {
            res.set_status((*auth_error).get_status());
            res.set_body(serde::Ser::serialize("application/json", *auth_error));
            send();
            return;
        }
        const auto body = std::string{R"({"op":"launch_sage"})"};
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
        LaunchSageInput input{
            m_input_support.request_data(req, std::string{R"({"op":"launch_sage"})"}),
            *parsed
        };
        auto result = m_runtime.launch_sage(input);
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

    void handle_set_dashboard_metadata(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto auth_error = m_input_support.authorize(req, "set_dashboard_metadata")) {
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
        SetDashboardMetadataInput input{
            m_input_support.request_data(req, req.body_as_string()),
            *parsed
        };
        auto result = m_runtime.set_dashboard_metadata(input);
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

    void handle_get_multiple_dashboard_metadata(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto auth_error =
                m_input_support.authorize(req, "get_multiple_dashboard_metadata")) {
            res.set_status((*auth_error).get_status());
            res.set_body(serde::Ser::serialize("application/json", *auth_error));
            send();
            return;
        }
        const auto body = std::string{R"({"op":"get_multiple_dashboard_metadata"})"};
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
        GetMultipleDashboardMetadataInput input{
            m_input_support
                .request_data(req, std::string{R"({"op":"get_multiple_dashboard_metadata"})"}),
            *parsed
        };
        auto result = m_runtime.get_multiple_dashboard_metadata(input);
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
