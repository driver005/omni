export module blog_handlers:members_members_stats_mrr_get_handler;
import :domain_all;
import std;
import interfaces;
import blog_handlers_middleware;
import core_workflow;
import memory_backends;
import :members_operations;
import :members_members_stats_mrr_get_flow;

export namespace blog::handlers::members::members_stats_mrr_get {
class MembersStatsMrrGetHandler
{
public:
    MembersStatsMrrGetHandler(blog::BlogContext& ctx) noexcept
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
        namespace support = blog::api::members::members_support;
        // Ghost mw.authAdminApi + permissions {method:'browse',docName:'members'}.
        if (!support::enforce(req, res, send, support::kDocName, "browse")) {
            return;
        }
        const auto ex = m_workflow.run("members_members_stats_mrr_get", req);
        const auto* r = ex.result_of("members_members_stats_mrr_get_execute");
        const auto body = r && r->typed_output.has_value()
                              ? std::any_cast<std::string>(r->typed_output)
                              : std::string{"{}"};
        std::vector<std::byte> bytes;
        for (char c: body) {
            bytes.push_back(static_cast<std::byte>(c));
        }
        // Ghost cacheInvalidate:false -> no X-Cache-Invalidate header.
        res.set_status(interfaces::io::types::Status::OK);
        res.set_body(std::move(bytes));
        send();
    }

private:
    core::workflow::Workflow m_workflow{};
};
} // namespace blog::handlers::members::members_stats_mrr_get
