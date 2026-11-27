export module app_store:files_handler;

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
        // Upstream: files.rs:44-64 — server_wrap(auth_type(ApiKeyAuth{...},
        // DashboardNoPermissionAuth{...})) around files_create_core.
        if (const auto denied = m_request_support.authorize(req, "files_create", {})) {
            const auto& error = *denied;
            error.send(res, send);
            return;
        }
        const auto auth = ::app::auth::AuthContextBuilder{}(req);
        const auto processor_merchant_id = m_request_support.scope(req).merchant_id;
        const auto provider_merchant_id =
            auth.merchant_id().empty() ? processor_merchant_id : auth.merchant_id();

        // Upstream: files.rs:39, transformers.rs:13-82. Multipart is decoded from the
        // raw request body because the local HTTP layer exposes bytes, not Actix fields.
        const auto body = req.body_as_string();
        if (body.empty()) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::EMPTY_REQUEST,
                "empty request"
            );
            error.send(res, send);
            return;
        }
        auto parsed = m_input_support.get_create_file_request(req, body);
        if (!parsed) {
            const auto& error = parsed.error();
            error.send(res, send);
            return;
        }

        FilesCreateInput input{*parsed, processor_merchant_id, provider_merchant_id};
        auto result = m_runtime.create(input);
        if (!result) {
            const auto& error = result.error();
            error.send(res, send);
            return;
        }

        auto encoded = result->serialize(files_create_wire_fields());
        if (!encoded) {
            encoded.error().send(res, send);
            return;
        }
        res.set_status(interfaces::io::types::Status::OK);
        std::vector<std::byte> body_bytes{};
        body_bytes.reserve(encoded->size());
        for (const char ch: *encoded) {
            body_bytes.push_back(static_cast<std::byte>(ch));
        }
        res.set_body(std::move(body_bytes));
        send();
    }

    void handle_files_delete(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        // Upstream: files.rs:96-117.
        if (const auto denied = m_request_support.authorize(req, "files_delete", {})) {
            const auto& error = *denied;
            error.send(res, send);
            return;
        }
        // Upstream web::Path<String> for /files/{file_id}: find the "files"
        // segment and take the next non-empty segment as the id.
        std::optional<std::string> file_id{};
        for (std::size_t index{0};; ++index) {
            const auto segment = m_request_support.path(req, index);
            if (segment.empty()) {
                break;
            }
            if (segment != "files") {
                continue;
            }
            for (std::size_t next{index + 1};; ++next) {
                const auto candidate = m_request_support.path(req, next);
                if (candidate.empty()) {
                    break;
                }
                file_id = candidate;
                break;
            }
            break;
        }
        if (!file_id.has_value()) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INVALID_REQUEST,
                "missing file id in path"
            );
            error.send(res, send);
            return;
        }
        const auto processor_merchant_id = m_request_support.scope(req).merchant_id;

        FilesDeleteInput input{processor_merchant_id, *file_id};
        auto result = m_runtime.delete_file(input);
        if (!result) {
            const auto& error = result.error();
            error.send(res, send);
            return;
        }

        res.set_status(interfaces::io::types::Status::OK);
        res.set_body(serde::Ser::serialize("application/json", *result));
        send();
    }

    void handle_files_retrieve(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        // Upstream: files.rs:150-171.
        if (const auto denied = m_request_support.authorize(req, "files_retrieve", {})) {
            const auto& error = *denied;
            error.send(res, send);
            return;
        }
        // Upstream web::Path<String> for /files/{file_id}: find the "files"
        // segment and take the next non-empty segment as the id.
        std::optional<std::string> file_id{};
        for (std::size_t index{0};; ++index) {
            const auto segment = m_request_support.path(req, index);
            if (segment.empty()) {
                break;
            }
            if (segment != "files") {
                continue;
            }
            for (std::size_t next{index + 1};; ++next) {
                const auto candidate = m_request_support.path(req, next);
                if (candidate.empty()) {
                    break;
                }
                file_id = candidate;
                break;
            }
            break;
        }
        if (!file_id.has_value()) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INVALID_REQUEST,
                "missing file id in path"
            );
            error.send(res, send);
            return;
        }
        const auto dispute_id = m_request_support.query(req, "dispute_id");
        const auto processor_merchant_id = m_request_support.scope(req).merchant_id;

        FilesRetrieveInput input{processor_merchant_id, *file_id, dispute_id};
        auto result = m_runtime.retrieve(input);
        if (!result) {
            const auto& error = result.error();
            error.send(res, send);
            return;
        }

        // Upstream: core/files.rs:139-145 — ApplicationResponse::FileData((bytes, mime)).
        // Payload is a typed document; mime stays on the Content-Type header.
        std::string content_type{};
        if (const auto* document = std::get_if<serde::Value::Object>(&result->get())) {
            const auto field = document->get(std::string{files_retrieve_wire_fields()[1]});
            if (field && std::holds_alternative<std::string>(field.value().get())) {
                content_type = std::get<std::string>(field.value().get());
            }
        }
        res.set_status(interfaces::io::types::Status::OK);
        res.add_content_type(content_type);
        res.set_body(serde::Ser::serialize("application/json", *result));
        send();
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
