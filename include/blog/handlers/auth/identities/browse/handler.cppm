export module blog_handlers:identities_browse_handler;
import :domain_all;
import std;
import interfaces;
import core_workflow;
import memory_backends;
import :identities_browse_flow;

export namespace blog::handlers::identities::browse {
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
        const auto ex = m_workflow.run("identities_browse", req);
        const auto* r = ex.result_of("identities_browse_execute");
        auto body = r && r->typed_output.has_value() ? std::any_cast<std::string>(r->typed_output)
                                                     : std::string{"{}"};
        std::vector<std::byte> bytes;
        for (char c: body) {
            bytes.push_back(static_cast<std::byte>(c));
        }
        // Ghost admin app installs cacheControl('private') for every Admin API route
        // (admin/app.js:24, cache-control.ts:16-18).
        res.set_header(
            "Cache-Control",
            "no-cache, private, no-store, must-revalidate, max-stale=0, post-check=0, pre-check=0"
        );
        // headers.cacheInvalidate:false is falsy: headers.get emits no X-Cache-Invalidate
        // for this route (headers.ts:135-144). No stand-in marker is written.
        res.set_status(interfaces::io::types::Status::OK);
        res.set_body(std::move(bytes));
        send();
    }

private:
    core::workflow::Workflow m_workflow{};
};
} // namespace blog::handlers::identities::browse
