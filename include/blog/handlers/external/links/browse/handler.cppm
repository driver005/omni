export module blog_handlers:links_browse_handler;
import :domain_all;
import std;
import interfaces;
import blog_handlers_middleware;
import core_workflow;
import memory_backends;
import :links_browse_step;
import :links_browse_flow;

export namespace blog::handlers::links::browse {

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
        // Ghost: mw.authAdminApi + permissions:true (docName 'links', browse).
        if (!blog::handlers::links::support::permitted(req, res, send, "links", "browse")) {
            return;
        }

        const auto ex = m_workflow.run("links_browse", req);
        const auto* r = ex.result_of("links_browse_execute");
        auto body = r && r->typed_output.has_value() ? std::any_cast<std::string>(r->typed_output)
                                                     : std::string{"{}"};

        int status = static_cast<int>(interfaces::io::types::Status::OK);
        if (body.starts_with("{\"errors\"")) {
            status = static_cast<int>(interfaces::io::types::Status::UNPROCESSABLE_CONTENT);
        }
        std::vector<std::byte> bytes;
        for (const char c: body) {
            bytes.push_back(static_cast<std::byte>(c));
        }
        res.set_status(static_cast<interfaces::io::types::Status>(status));
        res.set_body(std::move(bytes));
        send();
    }

private:
    core::workflow::Workflow m_workflow{};
};

} // namespace blog::handlers::links::browse
