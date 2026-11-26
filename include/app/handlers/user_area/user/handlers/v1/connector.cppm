export module app_user:user_handlers_v1_connector;

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
class UserHandlersV1Connector
{
public:
    UserHandlersV1Connector(
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
                Router{router, "user"}.add_route(
                    Route{"clone_connector"}
                        .post(
                            [this](
                                interfaces::io::IRequest& req,
                                interfaces::io::IResponse& res,
                                std::function<void()> send
                            )
                            {
                                handle_clone_connector(req, res, std::move(send));
                            }
                        )
                        .summary("Hyperswitch clone_connector")
                )
            )
            .add_router(
                Router{router, "embedded"}.add_route(
                    Route{"token"}
                        .get(
                            [this](
                                interfaces::io::IRequest& req,
                                interfaces::io::IResponse& res,
                                std::function<void()> send
                            )
                            {
                                handle_issue_embedded_token(req, res, std::move(send));
                            }
                        )
                        .summary("Hyperswitch issue_embedded_token")
                )
            )
            .add_route(
                Route{"embedded"}
                    .get(
                        [this](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
                            handle_embedded_token_info(req, res, std::move(send));
                        }
                    )
                    .summary("Hyperswitch embedded_token_info")
            );
    }

    void handle_clone_connector(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto auth_error = m_input_support.authorize(req, "clone_connector")) {
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
        using Input = ::app::model::user::CloneConnectorRequest;
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
        CloneConnectorInput input{m_input_support.request_data(req, req.body_as_string()), *parsed};
        auto result = m_runtime.clone_connector(input);
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

    void handle_issue_embedded_token(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto auth_error = m_input_support.authorize(req, "issue_embedded_token")) {
            res.set_status((*auth_error).get_status());
            res.set_body(serde::Ser::serialize("application/json", *auth_error));
            send();
            return;
        }
        const auto body = std::string{R"({"op":"issue_embedded_token"})"};
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
        IssueEmbeddedTokenInput input{
            m_input_support.request_data(req, std::string{R"({"op":"issue_embedded_token"})"}),
            *parsed
        };
        auto result = m_runtime.issue_embedded_token(input);
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

    void handle_embedded_token_info(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto auth_error = m_input_support.authorize(req, "embedded_token_info")) {
            res.set_status((*auth_error).get_status());
            res.set_body(serde::Ser::serialize("application/json", *auth_error));
            send();
            return;
        }
        const auto body = std::string{R"({"op":"embedded_token_info"})"};
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
        EmbeddedTokenInfoInput input{
            m_input_support.request_data(req, std::string{R"({"op":"embedded_token_info"})"}),
            *parsed
        };
        auto result = m_runtime.embedded_token_info(input);
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
