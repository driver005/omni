export module app_user:user_handlers_v1_two_factor;

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
class UserHandlersV1TwoFactor
{
public:
    UserHandlersV1TwoFactor(
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
                    Router{router, "2fa"}.add_router(
                        Router{router, "totp"}.add_route(
                            Route{"begin"}
                                .get(
                                    [this](
                                        interfaces::io::IRequest& req,
                                        interfaces::io::IResponse& res,
                                        std::function<void()> send
                                    )
                                    {
                                        handle_totp_begin(req, res, std::move(send));
                                    }
                                )
                                .summary("Hyperswitch totp_begin")
                        )
                    )
                )
            )
            .add_router(
                Router{router, "user"}.add_router(
                    Router{router, "2fa"}.add_router(
                        Router{router, "totp"}.add_route(
                            Route{"reset"}
                                .get(
                                    [this](
                                        interfaces::io::IRequest& req,
                                        interfaces::io::IResponse& res,
                                        std::function<void()> send
                                    )
                                    {
                                        handle_totp_reset(req, res, std::move(send));
                                    }
                                )
                                .summary("Hyperswitch totp_reset")
                        )
                    )
                )
            )
            .add_router(
                Router{router, "user"}.add_router(
                    Router{router, "2fa"}.add_router(
                        Router{router, "totp"}.add_route(
                            Route{"verify"}
                                .post(
                                    [this](
                                        interfaces::io::IRequest& req,
                                        interfaces::io::IResponse& res,
                                        std::function<void()> send
                                    )
                                    {
                                        handle_totp_verify(req, res, std::move(send));
                                    }
                                )
                                .summary("Hyperswitch totp_verify")
                        )
                    )
                )
            )
            .add_router(
                Router{router, "user"}.add_router(
                    Router{router, "2fa"}.add_router(
                        Router{router, "recovery_code"}.add_route(
                            Route{"verify"}
                                .post(
                                    [this](
                                        interfaces::io::IRequest& req,
                                        interfaces::io::IResponse& res,
                                        std::function<void()> send
                                    )
                                    {
                                        handle_verify_recovery_code(req, res, std::move(send));
                                    }
                                )
                                .summary("Hyperswitch verify_recovery_code")
                        )
                    )
                )
            )
            .add_router(
                Router{router, "user"}.add_router(
                    Router{router, "2fa"}.add_router(
                        Router{router, "totp"}.add_route(
                            Route{"verify"}
                                .put(
                                    [this](
                                        interfaces::io::IRequest& req,
                                        interfaces::io::IResponse& res,
                                        std::function<void()> send
                                    )
                                    {
                                        handle_totp_update(req, res, std::move(send));
                                    }
                                )
                                .summary("Hyperswitch totp_update")
                        )
                    )
                )
            )
            .add_router(
                Router{router, "user"}.add_router(
                    Router{router, "2fa"}.add_router(
                        Router{router, "recovery_code"}.add_route(
                            Route{"generate"}
                                .get(
                                    [this](
                                        interfaces::io::IRequest& req,
                                        interfaces::io::IResponse& res,
                                        std::function<void()> send
                                    )
                                    {
                                        handle_generate_recovery_codes(req, res, std::move(send));
                                    }
                                )
                                .summary("Hyperswitch generate_recovery_codes")
                        )
                    )
                )
            )
            .add_router(
                Router{router, "user"}.add_router(
                    Router{router, "2fa"}.add_route(
                        Route{"terminate"}
                            .get(
                                [this](
                                    interfaces::io::IRequest& req,
                                    interfaces::io::IResponse& res,
                                    std::function<void()> send
                                )
                                {
                                    handle_terminate_two_factor_auth(req, res, std::move(send));
                                }
                            )
                            .summary("Hyperswitch terminate_two_factor_auth")
                    )
                )
            )
            .add_router(
                Router{router, "user"}.add_route(
                    Route{"2fa"}
                        .get(
                            [this](
                                interfaces::io::IRequest& req,
                                interfaces::io::IResponse& res,
                                std::function<void()> send
                            )
                            {
                                handle_check_two_factor_auth_status(req, res, std::move(send));
                            }
                        )
                        .summary("Hyperswitch check_two_factor_auth_status")
                )
            )
            .add_router(
                Router{router, "user"}.add_router(
                    Router{router, "2fa"}.add_route(
                        Route{"v2"}
                            .get(
                                [this](
                                    interfaces::io::IRequest& req,
                                    interfaces::io::IResponse& res,
                                    std::function<void()> send
                                )
                                {
                                    handle_check_two_factor_auth_status_with_attempts(
                                        req,
                                        res,
                                        std::move(send)
                                    );
                                }
                            )
                            .summary("Hyperswitch check_two_factor_auth_status_with_attempts")
                    )
                )
            );
    }

    void handle_totp_begin(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};
        if (const auto auth_error = m_support.authorize(req, "totp_begin", {})) {
            sink.set_status((*auth_error).get_status());
            sink.set_body(serde::Ser::serialize("application/json", *auth_error));
            sink.send();
            return;
        }
        const auto body = std::string{R"({"op":"totp_begin"})"};
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
        m_runtime.totp_begin(*parsed, ctx).handle(sink);
    }

    void handle_totp_reset(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};
        if (const auto auth_error = m_support.authorize(req, "totp_reset", {})) {
            sink.set_status((*auth_error).get_status());
            sink.set_body(serde::Ser::serialize("application/json", *auth_error));
            sink.send();
            return;
        }
        const auto body = std::string{R"({"op":"totp_reset"})"};
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
        m_runtime.totp_reset(*parsed, ctx).handle(sink);
    }

    void handle_totp_verify(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};
        if (const auto auth_error = m_support.authorize(req, "totp_verify", {})) {
            sink.set_status((*auth_error).get_status());
            sink.set_body(serde::Ser::serialize("application/json", *auth_error));
            sink.send();
            return;
        }
        const auto body = req.body_as_string();
        auto parsed =
            core::behavior::Serializer<::app::model::user::VerifyTotpRequest>::pass(req, sink);
        if (!parsed) {
            return;
        }
        auto ctx = m_support.context(req);
        m_runtime.totp_verify(*parsed, ctx).handle(sink);
    }

    void handle_verify_recovery_code(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};
        if (const auto auth_error = m_support.authorize(req, "verify_recovery_code", {})) {
            sink.set_status((*auth_error).get_status());
            sink.set_body(serde::Ser::serialize("application/json", *auth_error));
            sink.send();
            return;
        }
        const auto body = req.body_as_string();
        auto parsed =
            core::behavior::Serializer<::app::model::user::VerifyRecoveryCodeRequest>::pass(
                req,
                sink
            );
        if (!parsed) {
            return;
        }
        auto ctx = m_support.context(req);
        m_runtime.verify_recovery_code(*parsed, ctx).handle(sink);
    }

    void handle_totp_update(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};
        if (const auto auth_error = m_support.authorize(req, "totp_update", {})) {
            sink.set_status((*auth_error).get_status());
            sink.set_body(serde::Ser::serialize("application/json", *auth_error));
            sink.send();
            return;
        }
        const auto body = req.body_as_string();
        auto parsed =
            core::behavior::Serializer<::app::model::user::VerifyTotpRequest>::pass(req, sink);
        if (!parsed) {
            return;
        }
        auto ctx = m_support.context(req);
        m_runtime.totp_update(*parsed, ctx).handle(sink);
    }

    void handle_generate_recovery_codes(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};
        if (const auto auth_error = m_support.authorize(req, "generate_recovery_codes", {})) {
            sink.set_status((*auth_error).get_status());
            sink.set_body(serde::Ser::serialize("application/json", *auth_error));
            sink.send();
            return;
        }
        const auto body = std::string{R"({"op":"generate_recovery_codes"})"};
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
        m_runtime.generate_recovery_codes(*parsed, ctx).handle(sink);
    }

    void handle_terminate_two_factor_auth(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};
        if (const auto auth_error = m_support.authorize(req, "terminate_two_factor_auth", {})) {
            sink.set_status((*auth_error).get_status());
            sink.set_body(serde::Ser::serialize("application/json", *auth_error));
            sink.send();
            return;
        }
        const auto body = std::string{R"({"op":"terminate_two_factor_auth"})"};
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
        m_runtime.terminate_two_factor_auth(*parsed, ctx).handle(sink);
    }

    void handle_check_two_factor_auth_status(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};
        if (const auto auth_error = m_support.authorize(req, "check_two_factor_auth_status", {})) {
            sink.set_status((*auth_error).get_status());
            sink.set_body(serde::Ser::serialize("application/json", *auth_error));
            sink.send();
            return;
        }
        const auto body = std::string{R"({"op":"check_two_factor_auth_status"})"};
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
        m_runtime.check_two_factor_auth_status(*parsed, ctx).handle(sink);
    }

    void handle_check_two_factor_auth_status_with_attempts(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};
        if (const auto auth_error =
                m_support.authorize(req, "check_two_factor_auth_status_with_attempts", {})) {
            sink.set_status((*auth_error).get_status());
            sink.set_body(serde::Ser::serialize("application/json", *auth_error));
            sink.send();
            return;
        }
        const auto body = std::string{R"({"op":"check_two_factor_auth_status_with_attempts"})"};
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
        m_runtime.check_two_factor_auth_status_with_attempts(*parsed, ctx).handle(sink);
    }

private:
    ::app::context::AppContext& m_context;
    UserRuntime& m_runtime;
    UserInputSupport& m_input_support;
    UserRequestSupport& m_support;
};

} // namespace app::handlers::user
