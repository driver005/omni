export module blog_handlers:settings_browse_handler;
import :domain_all;
import std;
import interfaces;
import blog_handlers_middleware;
import core_workflow;
import memory_backends;
import :settings_browse_step;
import :settings_browse_flow;

export namespace blog::handlers::settings::browse {
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
        // Ghost `permissions:true` under docName 'settings'; the public projection
        // is content-negotiated in the step (x-blog-context: CONTENT_API).
        if (!support::authorize(req, res, send, support::kDocName, "browse")) {
            return;
        }

        const auto ex = m_workflow.run("settings_browse", req);
        const auto* r = ex.result_of("settings_browse_execute");
        auto payload = r && r->typed_output.has_value()
                           ? std::any_cast<support::Result>(r->typed_output)
                           : support::Result{{"status", "200"}, {"body", "{}"}};
        const int status = payload.contains("status") ? std::stoi(payload.at("status")) : 200;
        const auto body = payload.contains("body") ? payload.at("body") : std::string{"{}"};
        std::vector<std::byte> bytes;
        for (char c: body) {
            bytes.push_back(static_cast<std::byte>(c));
        }
        // Ghost headers.cacheInvalidate:false -> no X-Cache-Invalidate header.
        res.set_status(static_cast<interfaces::io::types::Status>(status));
        res.set_body(std::move(bytes));
        send();
    }

private:
    core::workflow::Workflow m_workflow{};
};
} // namespace blog::handlers::settings::browse
