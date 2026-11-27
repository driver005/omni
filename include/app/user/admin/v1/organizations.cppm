export module app_user:admin_handlers_v1_organizations;

import std;
import interfaces;
import core_router;
import core_serde;
import utils_openapi;
import app_model;
import app_dto;
import app_context;
import core_behavior;
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
        m_support{request_support}
    {
    }

    void handle_organization_create(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};
        if (const auto error = m_support.authorize(req, "organization_create", {})) {
            sink.set_status(error->get_status());
            sink.set_body(serde::Ser::serialize("application/json", *error));
            sink.send();
            return;
        }
        const auto body = req.body_as_string();
        auto parsed =
            core::behavior::Serializer<::app::model::organization::OrganizationCreateRequest>::pass(
                req,
                sink
            );
        if (!parsed) {
            return;
        }
        auto ctx = m_support.context(req);
        m_runtime.create_organization(*parsed, ctx).handle(sink);
    }

    void handle_organization_retrieve(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};
        if (const auto error = m_support.authorize(req, "organization_retrieve", {})) {
            sink.set_status(error->get_status());
            sink.set_body(serde::Ser::serialize("application/json", *error));
            sink.send();
            return;
        }
        const std::string organization_id{m_support.path(req, AdminPolicy::path_index(req, 1))};
        if (organization_id.empty()) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INVALID_REQUEST,
                "missing organization id"
            );
            error.send(sink);
            return;
        }
        auto ctx = m_support.context(req);
        ctx.claims.insert_or_assign("organization_id", std::string{organization_id});
        m_runtime.retrieve_organization(ctx).handle(sink);
    }

    void handle_organization_update(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};
        if (const auto error = m_support.authorize(req, "organization_update", {})) {
            sink.set_status(error->get_status());
            sink.set_body(serde::Ser::serialize("application/json", *error));
            sink.send();
            return;
        }
        const std::string organization_id{m_support.path(req, AdminPolicy::path_index(req, 1))};
        if (organization_id.empty()) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INVALID_REQUEST,
                "missing organization id"
            );
            error.send(sink);
            return;
        }
        const auto body = req.body_as_string();
        auto parsed =
            core::behavior::Serializer<::app::model::organization::OrganizationUpdateRequest>::pass(
                req,
                sink
            );
        if (!parsed) {
            return;
        }
        auto ctx = m_support.context(req);
        ctx.claims.insert_or_assign("organization_id", std::string{organization_id});
        m_runtime.update_organization(*parsed, ctx).handle(sink);
    }

    void handle_convert_organization_to_platform(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};
        if (const auto error = m_support.authorize(req, "convert_organization_to_platform", {})) {
            sink.set_status(error->get_status());
            sink.set_body(serde::Ser::serialize("application/json", *error));
            sink.send();
            return;
        }
        const std::string organization_id{m_support.path(req, AdminPolicy::path_index(req, 2))};
        if (organization_id.empty()) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INVALID_REQUEST,
                "missing organization id"
            );
            error.send(sink);
            return;
        }
        auto parsed = core::behavior::Serializer<
            ::app::model::organization::ConvertOrganizationToPlatformRequest>::pass(req, sink);
        if (!parsed) {
            return;
        }
        auto ctx = m_support.context(req);
        ctx.claims.insert_or_assign("organization_id", std::string{organization_id});
        m_runtime.convert_organization_to_platform(*parsed, ctx).handle(sink);
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
    AdminRequestSupport& m_support;
};

} // namespace app::handlers::admin
