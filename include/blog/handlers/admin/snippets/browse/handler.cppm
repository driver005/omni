export module blog_handlers:snippets_browse_handler;
import :domain_all;
import std;
import interfaces;
import core_workflow;
import memory_backends;
import blog_handlers_middleware;
import :snippets_browse_flow;

export namespace blog::handlers::snippets::browse {
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
        // Ghost permissions:true hook (contract two-arg shape). Only a request that
        // explicitly carries a Ghost identity context activates it; the steps apply
        // the same admin-API principal for ordinary bearer-token calls.
        if (!req.find_header(blog::handlers::middleware::kGhostContextHeader).empty()) {
            bool denied = false;
            blog::handlers::middleware::GhostPermissionRegistry::set_active(
                &req,
                &res,
                [&]
                {
                    denied = true;
                    send();
                },
                nullptr
            );
            blog::handlers::middleware::require_ghost_permission("snippets", "browse");
            blog::handlers::middleware::GhostPermissionRegistry::clear_active();
            if (denied) {
                return;
            }
        }
        const auto ex = m_workflow.run("snippets_browse", req);
        const auto* r = ex.result_of("snippets_browse_execute");
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
        res.set_status(static_cast<interfaces::io::types::Status>(status));
        res.set_body(std::move(bytes));
        send();
    }

private:
    core::workflow::Workflow m_workflow{};
};
} // namespace blog::handlers::snippets::browse
