export module app_store:files_router;

import std;
import interfaces;
import core_router;
import core_serde;
import core_behavior;
import utils_openapi;
import app_model;
import app_dto;
import app_context;
import core_error;
import app_context_core;
import app_http;
import app_metrics;
import app_context_core;
import app_connector;
import app_services;

import :files_input;
import :files_result;
import :files_runtime;

export namespace app::handlers::files {

class FilesRouteHandlers
{
public:
    explicit FilesRouteHandlers(::app::context::AppContext& context) noexcept :
        m_runtime{context},
        m_input_support{context},
        m_policy{m_input_support},
        m_request_support{m_policy}
    {
    }

    // ── Route handlers: authorize, parse the upstream input, then dispatch to the
    //    local ports of upstream core/files.rs and core/files/helpers.rs. ────────────

    void handle_files_create(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};
        // Upstream: files.rs:44-64 — server_wrap(auth_type(ApiKeyAuth{...},
        // DashboardNoPermissionAuth{...})) around files_create_core.
        if (const auto denied = m_request_support.authorize(req, "files_create", {})) {
            const auto& error = *denied;
            error.send(sink);
            return;
        }

        // Upstream: files.rs:39, transformers.rs:13-82. Multipart is decoded from the
        // raw request body because the local HTTP layer exposes bytes, not Actix fields.
        const auto body = req.body_as_string();
        if (body.empty()) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::EMPTY_REQUEST,
                "empty request"
            );
            error.send(sink);
            return;
        }
        auto parsed = m_input_support.get_create_file_request(req, body);
        if (!parsed) {
            const auto& error = parsed.error();
            error.send(sink);
            return;
        }

        // Upstream: core/files.rs:87-89 — Json(CreateFileResponse { file_id }).
        m_runtime.create(*parsed, m_request_support.context(req)).handle(sink);
    }

    void handle_files_delete(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};
        // Upstream: files.rs:96-117.
        if (const auto denied = m_request_support.authorize(req, "files_delete", {})) {
            const auto& error = *denied;
            error.send(sink);
            return;
        }
        // web::Path<String> /files/{file_id} is read from the context by the runtime.
        m_runtime.delete_file(m_request_support.context(req)).handle(sink);
    }

    void handle_files_retrieve(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};
        // Upstream: files.rs:150-171.
        if (const auto denied = m_request_support.authorize(req, "files_retrieve", {})) {
            const auto& error = *denied;
            error.send(sink);
            return;
        }
        // Upstream: core/files.rs:139-145 — ApplicationResponse::FileData((bytes, mime)).
        // Path id and dispute_id query are read from the context by the runtime.
        m_runtime.retrieve(m_request_support.context(req)).handle(sink);
    }

    void install_routes(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        // Upstream: routes/app.rs:2592-2599 — scope("/files"), resource("") POST,
        // resource("/{file_id}") DELETE + GET. An empty child path keeps POST at
        // exactly "/files" (no trailing slash).
        Router{router, "files"}
            .add_route(
                Route{""}
                    .post(
                        [this](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
                            handle_files_create(req, res, std::move(send));
                        }
                    )
                    .summary("Hyperswitch files_create")
            )
            .add_route(
                Route{":file_id"}
                    .delt(
                        [this](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
                            handle_files_delete(req, res, std::move(send));
                        }
                    )
                    .summary("Hyperswitch files_delete")
            )
            .add_route(
                Route{":file_id"}
                    .get(
                        [this](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
                            handle_files_retrieve(req, res, std::move(send));
                        }
                    )
                    .summary("Hyperswitch files_retrieve")
            );
    }

private:
    FilesRuntime m_runtime;
    FilesInputSupport m_input_support;
    FilesPolicy m_policy;
    core::behavior::RequestSupport<FilesPolicy> m_request_support;
};

} // namespace app::handlers::files
