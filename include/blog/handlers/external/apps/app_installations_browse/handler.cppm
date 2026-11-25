export module blog_handlers:apps_app_installations_browse_handler;
import :domain_all;
import std;
import interfaces;
import blog_handlers_middleware;
import core_workflow;
import memory_backends;
import :apps_app_installations_browse_flow;

export namespace blog::handlers::apps::app_installations_browse {
class AppInstallationsBrowseHandler
{
public:
    AppInstallationsBrowseHandler(blog::BlogContext& ctx) noexcept
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

        // authAdminApi resolves the admin token to a staff identity. No token store
        // exists in this folder, so a presented admin token maps to the Ghost
        // Administrator role (presence-only stand-in, as on the other admin routes).
        if (req.find_header(kGhostContextHeader).empty()) {
            req.set_header(kGhostContextHeader, "ADMIN_API");
        }
        if (!req.get_authorization().empty() && req.find_header(kGhostRoleHeader).empty()) {
            req.set_header(kGhostRoleHeader, "Administrator");
            req.set_header(kGhostPermissionsHeader, "*");
        }

        // permissions:true on browse (app-installations.ts:59): docName
        // 'app_installations', method 'browse'.
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
            "browse"
        );
        if (!allowed) {
            return;
        }

        const auto ex = m_workflow.run("apps_app_installations_browse", req);
        const auto* r = ex.result_of("apps_app_installations_browse_execute");
        auto payload = r && r->typed_output.has_value()
                           ? std::any_cast<std::map<std::string, std::string>>(r->typed_output)
                           : std::map<std::string, std::string>{
                                 {"status", "200"},
                                 {"body", "{\"app_installations\":[]}"}
                             };
        const std::string code =
            payload.contains("status") ? payload.at("status") : std::string{"200"};
        const std::string body = payload.contains("body") ? payload.at("body") : std::string{"{}"};
        std::vector<std::byte> bytes;
        for (char c: body) {
            bytes.push_back(static_cast<std::byte>(c));
        }
        res.set_status(static_cast<interfaces::io::types::Status>(std::stoi(code)));
        // headers.cacheInvalidate:false => framework emits no X-Cache-Invalidate header.
        res.set_body(std::move(bytes));
        send();
    }

private:
    core::workflow::Workflow m_workflow{};
};
} // namespace blog::handlers::apps::app_installations_browse
