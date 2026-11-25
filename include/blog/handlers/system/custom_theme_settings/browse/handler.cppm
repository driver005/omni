export module blog_handlers:custom_theme_settings_browse_handler;
import :domain_all;
import std;
import interfaces;
import blog_handlers_middleware;
import core_workflow;
import memory_backends;
import :custom_theme_settings_browse_flow;

export namespace blog::handlers::custom_theme_settings::browse {
class BrowseHandler
{
public:
    BrowseHandler(blog::BlogContext& ctx) noexcept
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
        // Ghost `permissions:true` / docName `custom_theme_settings`
        // (custom-theme-settings.js:5,11) routes through
        // canThis(context).browse.custom_theme_setting. The shared authorizer needs the admin
        // principal; mirror the api-framework http(context) wiring.
        if (req.find_header(kGhostContextHeader).empty()) {
            req.set_header(kGhostContextHeader, "ADMIN_API");
        }
        if (!req.get_authorization().empty() && req.find_header(kGhostRoleHeader).empty()) {
            req.set_header(kGhostRoleHeader, "Administrator");
            req.set_header(kGhostPermissionsHeader, "*");
        }
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
            "custom_theme_settings",
            "browse"
        );
        if (!allowed) {
            return;
        }

        const auto ex = m_workflow.run("custom_theme_settings_browse", req);
        const auto* r = ex.result_of("custom_theme_settings_browse_execute");
        auto body = r && r->typed_output.has_value() ? std::any_cast<std::string>(r->typed_output)
                                                     : std::string{"{}"};
        std::vector<std::byte> bytes;
        for (char c: body) {
            bytes.push_back(static_cast<std::byte>(c));
        }
        res.set_status(interfaces::io::types::Status::OK);
        // Ghost headers.cacheInvalidate:false -> no X-Cache-Invalidate header (headers.ts:135-144).
        res.set_body(std::move(bytes));
        send();
    }

private:
    core::workflow::Workflow m_workflow{};
};
} // namespace blog::handlers::custom_theme_settings::browse
