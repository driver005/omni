export module blog_handlers:apps_app_installation_previews_add_handler;
import :domain_all;
import std;
import interfaces;
import blog_handlers_middleware;
import core_workflow;
import memory_backends;
import :apps_app_installation_previews_add_flow;

export namespace blog::handlers::apps::app_installation_previews_add {
class AppInstallationPreviewsAddHandler
{
public:
    AppInstallationPreviewsAddHandler(blog::BlogContext& ctx) noexcept
    {
        workflow::ExecuteStep::s_ctx = &ctx;
        m_workflow.use_backend(std::make_unique<core::workflow::LocalWorkflowDriver>());
        m_workflow.register_workflow(workflow::MainFlow{}.build());
    }

    void handle(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        using blog::handlers::middleware::kGhostContextHeader;
        using blog::handlers::middleware::kGhostPermissionsHeader;
        using blog::handlers::middleware::kGhostRoleHeader;

        if (req.find_header(kGhostContextHeader).empty()) {
            req.set_header(kGhostContextHeader, "ADMIN_API");
        }
        if (!req.get_authorization().empty() && req.find_header(kGhostRoleHeader).empty()) {
            req.set_header(kGhostRoleHeader, "Administrator");
            req.set_header(kGhostPermissionsHeader, "*");
        }
        // Ghost requires the install permission because preview fetches a URL
        // (permissions:{docName:'app_installations',method:'add'},
        // app-installation-previews.ts:20).
        bool allowed{false};
        blog::handlers::middleware::require_ghost_permission(
            req,
            res,
            [&allowed](
                interfaces::io::IRequest&,
                interfaces::io::IResponse&,
                std::function<void()>
            ) noexcept
            {
                allowed = true;
            },
            send,
            "app_installations",
            "add"
        );
        if (!allowed) {
            return;
        }

        const auto ex = m_workflow.run("apps_app_installation_previews_add", req);
        const auto* r = ex.result_of("apps_app_installation_previews_add_execute");
        auto payload = r && r->typed_output.has_value()
                           ? std::any_cast<std::map<std::string, std::string>>(r->typed_output)
                           : std::map<std::string, std::string>{
                                 {"status", "422"},
                                 {"body",
                                  blog::api::ApiError{}.validation(
                                      "Validation error",
                                      "Expected the manifest_url to preview.",
                                      "manifest_url"
                                  )}
                             };
        const std::string code =
            payload.contains("status") ? payload.at("status") : std::string{"422"};
        const std::string body = payload.contains("body") ? payload.at("body") : std::string{"{}"};
        std::vector<std::byte> bytes;
        for (char c: body) {
            bytes.push_back(static_cast<std::byte>(c));
        }
        auto status = interfaces::io::types::Status::OK;
        if (code == "400") {
            status = interfaces::io::types::Status::BAD_REQUEST;
        } else if (code == "404") {
            status = interfaces::io::types::Status::NOT_FOUND;
        } else if (code == "409") {
            status = interfaces::io::types::Status::CONFLICT;
        } else if (code == "422") {
            status = interfaces::io::types::Status::UNPROCESSABLE_CONTENT;
        } else if (code == "500") {
            status = interfaces::io::types::Status::INTERNAL_SERVER_ERROR;
        }
        // headers.cacheInvalidate:false => no X-Cache-Invalidate header.
        res.set_status(status);
        res.set_body(std::move(bytes));
        send();
    }

private:
    core::workflow::Workflow m_workflow{};
};
} // namespace blog::handlers::apps::app_installation_previews_add
