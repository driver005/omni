export module blog_handlers:previews_preview_get_handler;
import :domain_all;
import std;
import interfaces;
import core_workflow;
import memory_backends;
import :previews_preview_get_flow;

export namespace blog::handlers::previews::preview_get {
class PreviewGetHandler
{
public:
    PreviewGetHandler(blog::BlogContext& ctx) noexcept
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
        const auto ex = m_workflow.run("previews_preview_get", req);
        const auto* r = ex.result_of("previews_preview_get_execute");
        const auto payload =
            r && r->typed_output.has_value()
                ? std::any_cast<std::map<std::string, std::string>>(r->typed_output)
                : std::map<std::string, std::string>{{"status", "200"}, {"body", "{}"}};
        const int status = payload.contains("status") ? std::stoi(payload.at("status")) : 200;
        const auto body = payload.contains("body") ? payload.at("body") : std::string{"{}"};
        std::vector<std::byte> bytes;
        for (char c: body) {
            bytes.push_back(static_cast<std::byte>(c));
        }
        // Ghost previews.js:59-61 sets headers.cacheInvalidate = false, which is a
        // no-header instruction: X-Cache-Invalidate must be omitted, not sent as
        // "false". No other cache headers are written on this route.
        res.set_status(static_cast<interfaces::io::types::Status>(status));
        res.set_body(std::move(bytes));
        send();
    }

private:
    core::workflow::Workflow m_workflow{};
};
} // namespace blog::handlers::previews::preview_get
