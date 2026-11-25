export module blog_handlers:upgrades_create_handler;
import :domain_all;
import std;
import interfaces;
import blog_handlers_middleware;
import core_workflow;
import memory_backends;
import :upgrades_browse_step;
import :upgrades_create_flow;

export namespace blog::handlers::upgrades::create {

class CreateHandler
{
public:
    CreateHandler(blog::BlogContext& ctx) noexcept
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
        // Ghost: mw.authAdminApi + permissions:true; the step adds the staff-user
        // gate, SHA-256 idempotency scoping and the host-adapter error taxonomy.
        support::prepare_principal(req);
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
            support::kDocName,
            "add"
        );
        if (!allowed) {
            return;
        }

        const auto execution = m_workflow.run("upgrades_create", req);
        const auto* result = execution.result_of("upgrades_create_execute");
        const auto payload = result && result->typed_output.has_value()
                                 ? std::any_cast<support::Output>(result->typed_output)
                                 : support::Output{{"status", "202"}, {"body", "{}"}};
        const int status = payload.contains("status") ? std::stoi(payload.at("status")) : 202;
        const auto body = payload.contains("body") ? payload.at("body") : std::string{"{}"};

        std::vector<std::byte> bytes;
        for (const char c: body) {
            bytes.push_back(static_cast<std::byte>(c));
        }
        // headers: {cacheInvalidate:false, location:false} — no X-Cache-Invalidate,
        // no Location; only the admin app's private Cache-Control.
        res.set_header(
            "Cache-Control",
            "no-cache, private, no-store, must-revalidate, max-stale=0, post-check=0, pre-check=0"
        );
        res.set_status(static_cast<interfaces::io::types::Status>(status));
        res.set_body(std::move(bytes));
        send();
    }

private:
    core::workflow::Workflow m_workflow{};
};

} // namespace blog::handlers::upgrades::create
