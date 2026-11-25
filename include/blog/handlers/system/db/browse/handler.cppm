export module blog_handlers:db_browse_handler;
import :domain_all;
import std;
import interfaces;
import core_workflow;
import memory_backends;
import :db_browse_flow;

export namespace blog::handlers::db::browse {
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
        // Ghost authorizes at the router chain (admin/routes.js:390 + db.js:60);
        // cacheInvalidate:false emits no X-Cache-Invalidate header (headers.ts:135).
        const auto ex = m_workflow.run("db_browse", req);
        const auto* r = ex.result_of("db_browse_execute");
        const auto payload =
            r && r->typed_output.has_value()
                ? std::any_cast<std::map<std::string, std::string>>(r->typed_output)
                : std::map<std::string, std::string>{
                      {"status", "200"},
                      {"body", "{\"db\":[]}"},
                      {"filename", "ghost.json"}
                  };
        const int status = payload.contains("status") ? std::stoi(payload.at("status")) : 200;
        const auto body =
            payload.contains("body") ? payload.at("body") : std::string{"{\"db\":[]}"};
        const auto filename =
            payload.contains("filename") ? payload.at("filename") : std::string{"ghost.json"};
        std::vector<std::byte> bytes;
        for (char c: body) {
            bytes.push_back(static_cast<std::byte>(c));
        }
        res.set_status(static_cast<interfaces::io::types::Status>(status));
        if (status == 200) {
            res.set_header("Content-Disposition", "Attachment; filename=\"" + filename + "\"");
        }
        res.set_body(std::move(bytes));
        send();
    }

private:
    core::workflow::Workflow m_workflow{};
};
} // namespace blog::handlers::db::browse
