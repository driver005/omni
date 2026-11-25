export module blog_handlers:config_browse_handler;
import :domain_all;
import std;
import interfaces;
import core_workflow;
import memory_backends;
import :config_browse_flow;

export namespace blog::handlers::config::browse {
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
        // Auth is enforced by require_auth in ConfigRoutes::install — Ghost's
        // mw.authAdminApi (config.read sets permissions:false, so no permission stage).
        const auto ex = m_workflow.run("config_browse", req);
        const auto* r = ex.result_of("config_browse_execute");
        auto body = r && r->typed_output.has_value() ? std::any_cast<std::string>(r->typed_output)
                                                     : std::string{"{}"};
        std::vector<std::byte> bytes;
        for (char c: body) {
            bytes.push_back(static_cast<std::byte>(c));
        }
        // Ghost sets headers.cacheInvalidate:false, so headers.get emits no
        // X-Cache-Invalidate (headers.ts:135-144). Only the admin app's
        // cacheControl('private') header is written (admin/app.js:24).
        res.set_header(
            interfaces::io::types::Token::CACHE_CONTROL,
            "no-cache, private, no-store, must-revalidate, max-stale=0, post-check=0, pre-check=0"
        );
        res.set_status(interfaces::io::types::Status::OK);
        res.set_body(std::move(bytes));
        send();
    }

private:
    core::workflow::Workflow m_workflow{};
};
} // namespace blog::handlers::config::browse
