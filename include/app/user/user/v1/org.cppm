export module app_user:user_handlers_v1_org;

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
class UserHandlersV1Org
{
public:
    UserHandlersV1Org(
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
        if (const auto auth_error = m_input_support.authorize(req, "user_org_create")) {
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
            deserialize_error.send(res, send);
            return;
        }
        using Input = ::app::model::user::UserOrgMerchantCreateRequest;
        auto parsed = serde::Ser::deserialize<Input>("application/json", body);
        if (!parsed) {
            auto deserialize_error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::VALIDATION_FAILED,
                "Failed to deserialize the JSON body into the target type"
            );
            deserialize_error.set_kind_name("IR_06");
            deserialize_error.send(res, send);
            return;
        }
        UserOrgCreateInput input{m_input_support.request_data(req, req.body_as_string()), *parsed};
        auto result = m_runtime.user_org_create(input);
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

    void handle_user_merchant_account_create(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto auth_error =
                m_input_support.authorize(req, "user_merchant_account_create")) {
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
            deserialize_error.send(res, send);
            return;
        }
        using Input = ::app::model::user::UserMerchantCreate;
        auto parsed = serde::Ser::deserialize<Input>("application/json", body);
        if (!parsed) {
            auto deserialize_error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::VALIDATION_FAILED,
                "Failed to deserialize the JSON body into the target type"
            );
            deserialize_error.set_kind_name("IR_06");
            deserialize_error.send(res, send);
            return;
        }
        UserMerchantAccountCreateInput input{
            m_input_support.request_data(req, req.body_as_string()),
            *parsed
        };
        auto result = m_runtime.user_merchant_account_create(input);
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

    void handle_list_orgs_for_user(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto auth_error = m_input_support.authorize(req, "list_orgs_for_user")) {
            res.set_status((*auth_error).get_status());
            res.set_body(serde::Ser::serialize("application/json", *auth_error));
            send();
            return;
        }
        const auto body = std::string{R"({"op":"list_orgs_for_user"})"};
        if (body.empty()) {
            auto deserialize_error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::VALIDATION_FAILED,
                "Failed to deserialize the JSON body into the target type"
            );
            deserialize_error.set_kind_name("IR_06");
            deserialize_error.send(res, send);
            return;
        }
        auto parsed = serde::Ser::decode_generic("application/json", body);
        if (!parsed) {
            auto deserialize_error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::VALIDATION_FAILED,
                "Failed to deserialize the JSON body into the target type"
            );
            deserialize_error.set_kind_name("IR_06");
            deserialize_error.send(res, send);
            return;
        }
        ListOrgsForUserInput input{
            m_input_support.request_data(req, std::string{R"({"op":"list_orgs_for_user"})"}),
            *parsed
        };
        auto result = m_runtime.list_orgs_for_user(input);
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
            deserialize_error.send(res, send);
            return;
        }
        auto parsed = serde::Ser::decode_generic("application/json", body);
        if (!parsed) {
            auto deserialize_error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::VALIDATION_FAILED,
                "Failed to deserialize the JSON body into the target type"
            );
            deserialize_error.set_kind_name("IR_06");
            deserialize_error.send(res, send);
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

    void handle_list_profiles_for_user_in_org_and_merchant(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto auth_error =
                m_input_support.authorize(req, "list_profiles_for_user_in_org_and_merchant")) {
            res.set_status((*auth_error).get_status());
            res.set_body(serde::Ser::serialize("application/json", *auth_error));
            send();
            return;
        }
        const auto body = std::string{R"({"op":"list_profiles_for_user_in_org_and_merchant"})"};
        if (body.empty()) {
            auto deserialize_error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::VALIDATION_FAILED,
                "Failed to deserialize the JSON body into the target type"
            );
            deserialize_error.set_kind_name("IR_06");
            deserialize_error.send(res, send);
            return;
        }
        auto parsed = serde::Ser::decode_generic("application/json", body);
        if (!parsed) {
            auto deserialize_error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::VALIDATION_FAILED,
                "Failed to deserialize the JSON body into the target type"
            );
            deserialize_error.set_kind_name("IR_06");
            deserialize_error.send(res, send);
            return;
        }
        ListProfilesForUserInOrgAndMerchantInput input{
            m_input_support.request_data(
                req,
                std::string{R"({"op":"list_profiles_for_user_in_org_and_merchant"})"}
            ),
            *parsed
        };
        auto result = m_runtime.list_profiles_for_user_in_org_and_merchant(input);
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

    void handle_switch_org_for_user(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto auth_error = m_input_support.authorize(req, "switch_org_for_user")) {
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
            deserialize_error.send(res, send);
            return;
        }
        using Input = ::app::model::user::SwitchOrganizationRequest;
        auto parsed = serde::Ser::deserialize<Input>("application/json", body);
        if (!parsed) {
            auto deserialize_error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::VALIDATION_FAILED,
                "Failed to deserialize the JSON body into the target type"
            );
            deserialize_error.set_kind_name("IR_06");
            deserialize_error.send(res, send);
            return;
        }
        SwitchOrgForUserInput input{
            m_input_support.request_data(req, req.body_as_string()),
            *parsed
        };
        auto result = m_runtime.switch_org_for_user(input);
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

    void handle_switch_merchant_for_user_in_org(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto auth_error =
                m_input_support.authorize(req, "switch_merchant_for_user_in_org")) {
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
            deserialize_error.send(res, send);
            return;
        }
        using Input = ::app::model::user::SwitchMerchantRequest;
        auto parsed = serde::Ser::deserialize<Input>("application/json", body);
        if (!parsed) {
            auto deserialize_error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::VALIDATION_FAILED,
                "Failed to deserialize the JSON body into the target type"
            );
            deserialize_error.set_kind_name("IR_06");
            deserialize_error.send(res, send);
            return;
        }
        SwitchMerchantForUserInOrgInput input{
            m_input_support.request_data(req, req.body_as_string()),
            *parsed
        };
        auto result = m_runtime.switch_merchant_for_user_in_org(input);
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

    void handle_switch_profile_for_user_in_org_and_merchant(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto auth_error =
                m_input_support.authorize(req, "switch_profile_for_user_in_org_and_merchant")) {
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
            deserialize_error.send(res, send);
            return;
        }
        using Input = ::app::model::user::SwitchProfileRequest;
        auto parsed = serde::Ser::deserialize<Input>("application/json", body);
        if (!parsed) {
            auto deserialize_error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::VALIDATION_FAILED,
                "Failed to deserialize the JSON body into the target type"
            );
            deserialize_error.set_kind_name("IR_06");
            deserialize_error.send(res, send);
            return;
        }
        SwitchProfileForUserInOrgAndMerchantInput input{
            m_input_support.request_data(req, req.body_as_string()),
            *parsed
        };
        auto result = m_runtime.switch_profile_for_user_in_org_and_merchant(input);
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

    void handle_get_user_merchant_details(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto auth_error = m_input_support.authorize(req, "get_user_merchant_details")) {
            res.set_status((*auth_error).get_status());
            res.set_body(serde::Ser::serialize("application/json", *auth_error));
            send();
            return;
        }
        const auto body = std::string{R"({"op":"get_user_merchant_details"})"};
        if (body.empty()) {
            auto deserialize_error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::VALIDATION_FAILED,
                "Failed to deserialize the JSON body into the target type"
            );
            deserialize_error.set_kind_name("IR_06");
            deserialize_error.send(res, send);
            return;
        }
        auto parsed = serde::Ser::decode_generic("application/json", body);
        if (!parsed) {
            auto deserialize_error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::VALIDATION_FAILED,
                "Failed to deserialize the JSON body into the target type"
            );
            deserialize_error.set_kind_name("IR_06");
            deserialize_error.send(res, send);
            return;
        }
        GetUserMerchantDetailsInput input{
            m_input_support.request_data(req, std::string{R"({"op":"get_user_merchant_details"})"}),
            *parsed
        };
        auto result = m_runtime.get_user_merchant_details(input);
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
