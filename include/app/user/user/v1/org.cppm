export module app_user:user_handlers_v1_org;

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
class UserHandlersV1Org
{
public:
    UserHandlersV1Org(
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
                    Route{"create_org"}
                        .post(
                            [this](
                                interfaces::io::IRequest& req,
                                interfaces::io::IResponse& res,
                                std::function<void()> send
                            )
                            {
                                handle_user_org_create(req, res, std::move(send));
                            }
                        )
                        .summary("Hyperswitch user_org_create")
                )
            )
            .add_route(
                Route{"create_merchant"}
                    .post(
                        [this](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
                            handle_user_merchant_account_create(req, res, std::move(send));
                        }
                    )
                    .summary("Hyperswitch user_merchant_account_create")
            )
            .add_router(
                Router{router, "list"}.add_route(
                    Route{"org"}
                        .get(
                            [this](
                                interfaces::io::IRequest& req,
                                interfaces::io::IResponse& res,
                                std::function<void()> send
                            )
                            {
                                handle_list_orgs_for_user(req, res, std::move(send));
                            }
                        )
                        .summary("Hyperswitch list_orgs_for_user")
                )
            )
            .add_router(
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
                        .summary("Hyperswitch list_merchants_for_user_in_org")
                )
            )
            .add_router(
                Router{router, "list"}.add_route(
                    Route{"profile"}
                        .get(
                            [this](
                                interfaces::io::IRequest& req,
                                interfaces::io::IResponse& res,
                                std::function<void()> send
                            )
                            {
                                handle_list_profiles_for_user_in_org_and_merchant(
                                    req,
                                    res,
                                    std::move(send)
                                );
                            }
                        )
                        .summary("Hyperswitch list_profiles_for_user_in_org_and_merchant")
                )
            )
            .add_router(
                Router{router, "switch"}.add_route(
                    Route{"org"}
                        .post(
                            [this](
                                interfaces::io::IRequest& req,
                                interfaces::io::IResponse& res,
                                std::function<void()> send
                            )
                            {
                                handle_switch_org_for_user(req, res, std::move(send));
                            }
                        )
                        .summary("Hyperswitch switch_org_for_user")
                )
            )
            .add_router(
                Router{router, "switch"}.add_route(
                    Route{"merchant"}
                        .post(
                            [this](
                                interfaces::io::IRequest& req,
                                interfaces::io::IResponse& res,
                                std::function<void()> send
                            )
                            {
                                handle_switch_merchant_for_user_in_org(req, res, std::move(send));
                            }
                        )
                        .summary("Hyperswitch switch_merchant_for_user_in_org")
                )
            )
            .add_router(
                Router{router, "switch"}.add_route(
                    Route{"profile"}
                        .post(
                            [this](
                                interfaces::io::IRequest& req,
                                interfaces::io::IResponse& res,
                                std::function<void()> send
                            )
                            {
                                handle_switch_profile_for_user_in_org_and_merchant(
                                    req,
                                    res,
                                    std::move(send)
                                );
                            }
                        )
                        .summary("Hyperswitch switch_profile_for_user_in_org_and_merchant")
                )
            )
            .add_route(
                Route{"merchant_details"}
                    .get(
                        [this](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
                            handle_get_user_merchant_details(req, res, std::move(send));
                        }
                    )
                    .summary("Hyperswitch get_user_merchant_details")
            );
    }

    void handle_user_org_create(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};
        if (const auto auth_error = m_support.authorize(req, "user_org_create", {})) {
            sink.set_status((*auth_error).get_status());
            sink.set_body(serde::Ser::serialize("application/json", *auth_error));
            sink.send();
            return;
        }
        const auto body = req.body_as_string();
        auto parsed =
            core::behavior::Serializer<::app::model::user::UserOrgMerchantCreateRequest>::pass(
                req,
                sink
            );
        if (!parsed) {
            return;
        }
        auto ctx = m_support.context(req);
        m_runtime.user_org_create(*parsed, ctx).handle(sink);
    }

    void handle_user_merchant_account_create(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};
        if (const auto auth_error = m_support.authorize(req, "user_merchant_account_create", {})) {
            sink.set_status((*auth_error).get_status());
            sink.set_body(serde::Ser::serialize("application/json", *auth_error));
            sink.send();
            return;
        }
        const auto body = req.body_as_string();
        auto parsed =
            core::behavior::Serializer<::app::model::user::UserMerchantCreate>::pass(req, sink);
        if (!parsed) {
            return;
        }
        auto ctx = m_support.context(req);
        m_runtime.user_merchant_account_create(*parsed, ctx).handle(sink);
    }

    void handle_list_orgs_for_user(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};
        if (const auto auth_error = m_support.authorize(req, "list_orgs_for_user", {})) {
            sink.set_status((*auth_error).get_status());
            sink.set_body(serde::Ser::serialize("application/json", *auth_error));
            sink.send();
            return;
        }
        const auto body = std::string{R"({"op":"list_orgs_for_user"})"};
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
        m_runtime.list_orgs_for_user(*parsed, ctx).handle(sink);
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

    void handle_list_profiles_for_user_in_org_and_merchant(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};
        if (const auto auth_error =
                m_support.authorize(req, "list_profiles_for_user_in_org_and_merchant", {})) {
            sink.set_status((*auth_error).get_status());
            sink.set_body(serde::Ser::serialize("application/json", *auth_error));
            sink.send();
            return;
        }
        const auto body = std::string{R"({"op":"list_profiles_for_user_in_org_and_merchant"})"};
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
        m_runtime.list_profiles_for_user_in_org_and_merchant(*parsed, ctx).handle(sink);
    }

    void handle_switch_org_for_user(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};
        if (const auto auth_error = m_support.authorize(req, "switch_org_for_user", {})) {
            sink.set_status((*auth_error).get_status());
            sink.set_body(serde::Ser::serialize("application/json", *auth_error));
            sink.send();
            return;
        }
        const auto body = req.body_as_string();
        auto parsed =
            core::behavior::Serializer<::app::model::user::SwitchOrganizationRequest>::pass(
                req,
                sink
            );
        if (!parsed) {
            return;
        }
        auto ctx = m_support.context(req);
        m_runtime.switch_org_for_user(*parsed, ctx).handle(sink);
    }

    void handle_switch_merchant_for_user_in_org(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};
        if (const auto auth_error =
                m_support.authorize(req, "switch_merchant_for_user_in_org", {})) {
            sink.set_status((*auth_error).get_status());
            sink.set_body(serde::Ser::serialize("application/json", *auth_error));
            sink.send();
            return;
        }
        const auto body = req.body_as_string();
        auto parsed =
            core::behavior::Serializer<::app::model::user::SwitchMerchantRequest>::pass(req, sink);
        if (!parsed) {
            return;
        }
        auto ctx = m_support.context(req);
        m_runtime.switch_merchant_for_user_in_org(*parsed, ctx).handle(sink);
    }

    void handle_switch_profile_for_user_in_org_and_merchant(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};
        if (const auto auth_error =
                m_support.authorize(req, "switch_profile_for_user_in_org_and_merchant", {})) {
            sink.set_status((*auth_error).get_status());
            sink.set_body(serde::Ser::serialize("application/json", *auth_error));
            sink.send();
            return;
        }
        const auto body = req.body_as_string();
        auto parsed =
            core::behavior::Serializer<::app::model::user::SwitchProfileRequest>::pass(req, sink);
        if (!parsed) {
            return;
        }
        auto ctx = m_support.context(req);
        m_runtime.switch_profile_for_user_in_org_and_merchant(*parsed, ctx).handle(sink);
    }

    void handle_get_user_merchant_details(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};
        if (const auto auth_error = m_support.authorize(req, "get_user_merchant_details", {})) {
            sink.set_status((*auth_error).get_status());
            sink.set_body(serde::Ser::serialize("application/json", *auth_error));
            sink.send();
            return;
        }
        const auto body = std::string{R"({"op":"get_user_merchant_details"})"};
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
        m_runtime.get_user_merchant_details(*parsed, ctx).handle(sink);
    }

private:
    ::app::context::AppContext& m_context;
    UserRuntime& m_runtime;
    UserInputSupport& m_input_support;
    UserRequestSupport& m_support;
};

} // namespace app::handlers::user
