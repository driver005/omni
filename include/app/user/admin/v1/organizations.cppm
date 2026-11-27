export module app_user:admin_handlers_v1_organizations;

import std;
import interfaces;
import core_router;
import core_serde;
import utils_openapi;
import app_model;
import app_dto;
import app_context;
import core_error;
import app_http;
import :admin_input;
import :admin_result;
import :admin_runtime;

export namespace app::handlers::admin {

class AdminHandlersV1Organizations
{
public:
    AdminHandlersV1Organizations(
        ::app::context::AppContext& context,
        AdminRuntime& runtime,
        AdminInputSupport& input_support,
        AdminRequestSupport& request_support
    ) noexcept :
        m_context{context},
        m_runtime{runtime},
        m_input_support{input_support},
        m_request_support{request_support}
    {
    }

    void handle_organization_create(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto error = m_request_support.authorize(req, "organization_create", {})) {
            res.set_status(error->get_status());
            res.set_body(serde::Ser::serialize("application/json", *error));
            send();
            return;
        }
        const auto body = req.body_as_string();
        if (body.empty()) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::EMPTY_REQUEST,
                "empty request"
            );
            error.send(res, send);
            return;
        }
        auto parsed =
            serde::Ser::deserialize<::app::model::organization::OrganizationCreateRequest>(
                "application/json",
                body
            );
        if (!parsed) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INVALID_JSON,
                "invalid JSON"
            );
            error.send(res, send);
            return;
        }
        OrganizationCreateInput input{*parsed};
        auto result = m_runtime.create_organization(input);
        if (!result) {
            const auto& error = result.error();
            error.send(res, send);
            return;
        }
        res.set_status(interfaces::io::types::Status::OK);
        auto encoded = result->serialize(kOrganizationResponseWireFields);
        if (!encoded) {
            encoded.error().send(res, send);
            return;
        }
        res.set_body(serde::Ser::serialize_raw("application/json", *encoded));
        send();
    }

    void handle_organization_retrieve(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto error = m_request_support.authorize(req, "organization_retrieve", {})) {
            res.set_status(error->get_status());
            res.set_body(serde::Ser::serialize("application/json", *error));
            send();
            return;
        }
        const std::string organization_id{
            m_request_support.path(req, AdminPolicy::path_index(req, 1))
        };
        if (organization_id.empty()) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INVALID_REQUEST,
                "missing organization id"
            );
            error.send(res, send);
            return;
        }
        OrganizationRetrieveInput input{organization_id};
        auto result = m_runtime.retrieve_organization(input);
        if (!result) {
            const auto& error = result.error();
            error.send(res, send);
            return;
        }
        res.set_status(interfaces::io::types::Status::OK);
        auto encoded = result->serialize(kOrganizationResponseWireFields);
        if (!encoded) {
            encoded.error().send(res, send);
            return;
        }
        res.set_body(serde::Ser::serialize_raw("application/json", *encoded));
        send();
    }

    void handle_organization_update(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto error = m_request_support.authorize(req, "organization_update", {})) {
            res.set_status(error->get_status());
            res.set_body(serde::Ser::serialize("application/json", *error));
            send();
            return;
        }
        const std::string organization_id{
            m_request_support.path(req, AdminPolicy::path_index(req, 1))
        };
        if (organization_id.empty()) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INVALID_REQUEST,
                "missing organization id"
            );
            error.send(res, send);
            return;
        }
        const auto body = req.body_as_string();
        if (body.empty()) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::EMPTY_REQUEST,
                "empty request"
            );
            error.send(res, send);
            return;
        }
        auto parsed =
            serde::Ser::deserialize<::app::model::organization::OrganizationUpdateRequest>(
                "application/json",
                body
            );
        if (!parsed) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INVALID_JSON,
                "invalid JSON"
            );
            error.send(res, send);
            return;
        }
        OrganizationUpdateInput input{organization_id, *parsed};
        auto result = m_runtime.update_organization(input);
        if (!result) {
            const auto& error = result.error();
            error.send(res, send);
            return;
        }
        res.set_status(interfaces::io::types::Status::OK);
        auto encoded = result->serialize(kOrganizationResponseWireFields);
        if (!encoded) {
            encoded.error().send(res, send);
            return;
        }
        res.set_body(serde::Ser::serialize_raw("application/json", *encoded));
        send();
    }

    void handle_convert_organization_to_platform(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto error =
                m_request_support.authorize(req, "convert_organization_to_platform", {})) {
            res.set_status(error->get_status());
            res.set_body(serde::Ser::serialize("application/json", *error));
            send();
            return;
        }
        const std::string organization_id{
            m_request_support.path(req, AdminPolicy::path_index(req, 2))
        };
        if (organization_id.empty()) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INVALID_REQUEST,
                "missing organization id"
            );
            error.send(res, send);
            return;
        }
        const auto body = req.body_as_string();
        if (body.empty()) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::EMPTY_REQUEST,
                "empty request"
            );
            error.send(res, send);
            return;
        }
        auto parsed = serde::Ser::deserialize<
            ::app::model::organization::ConvertOrganizationToPlatformRequest>(
            "application/json",
            body
        );
        if (!parsed) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INVALID_JSON,
                "invalid JSON"
            );
            error.send(res, send);
            return;
        }
        ConvertOrganizationToPlatformInput input{organization_id, *parsed};
        auto result = m_runtime.convert_organization_to_platform(input);
        if (!result) {
            const auto& error = result.error();
            error.send(res, send);
            return;
        }
        res.set_status(interfaces::io::types::Status::OK);
        auto encoded = result->serialize(kConvertOrganizationToPlatformResponseWireFields);
        if (!encoded) {
            encoded.error().send(res, send);
            return;
        }
        res.set_body(serde::Ser::serialize_raw("application/json", *encoded));
        send();
    }

    void install(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;

        // routes/app.rs:2112-2131 (Organization::server, v1).
        Router{router, "organization"}
            .add_route(
                Route{""}
                    .post(
                        [this](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
                            handle_organization_create(req, res, std::move(send));
                        }
                    )
                    .summary("Hyperswitch organization_create")
            )
            .add_route(
                Route{":id"}
                    .get(
                        [this](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
                            handle_organization_retrieve(req, res, std::move(send));
                        }
                    )
                    .summary("Hyperswitch organization_retrieve")
            )
            .add_route(
                Route{":id"}
                    .put(
                        [this](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
                            handle_organization_update(req, res, std::move(send));
                        }
                    )
                    .summary("Hyperswitch organization_update")
            )
            .add_router(
                Router{router, ":id"}.add_route(
                    Route{"convert_to_platform"}
                        .post(
                            [this](
                                interfaces::io::IRequest& req,
                                interfaces::io::IResponse& res,
                                std::function<void()> send
                            )
                            {
                                handle_convert_organization_to_platform(req, res, std::move(send));
                            }
                        )
                        .summary("Hyperswitch convert_organization_to_platform")
                )
            );
    }

private:
    ::app::context::AppContext& m_context;
    AdminRuntime& m_runtime;
    AdminInputSupport& m_input_support;
    AdminRequestSupport& m_request_support;
};

} // namespace app::handlers::admin
