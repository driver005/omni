export module blog_handlers:automations_browse_handler;
import :domain_all;
import :automations_dto_automation_email_preview;
import :automations_dto_automation_email_preview_input;
import :automations_dto_automation_email_preview_response;
import :automations_dto_automation_email_test_input;
import :automations_dto_links;
import :automations_dto_stats_empty_response;
import std;
import interfaces;
import blog_handlers_middleware;
import core_workflow;
import memory_backends;
import :automations_operations;
import :automations_browse_flow;

export namespace blog::handlers::automations::browse {
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
        // Ghost: router-level mw.authAdminApi (401 via require_auth) + permissions:true
        // (403 NoPermissionError via the shared GhostAuthorizer).
        if (!blog::api::automations::authorize(req, res, send, "automations", "browse")) {
            return;
        }
        const auto ex = m_workflow.run("automations_browse", req);
        const auto* r = ex.result_of("automations_browse_execute");
        auto body = r && r->typed_output.has_value() ? std::any_cast<std::string>(r->typed_output)
                                                     : std::string{"{}"};
        std::vector<std::byte> bytes;
        for (char c: body) {
            bytes.push_back(static_cast<std::byte>(c));
        }
        // Ghost headers.cacheInvalidate:false -> api-framework emits no X-Cache-Invalidate header.
        res.set_status(interfaces::io::types::Status::OK);
        res.set_body(std::move(bytes));
        send();
    }

private:
    core::workflow::Workflow m_workflow{};
};
} // namespace blog::handlers::automations::browse
