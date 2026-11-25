export module blog_handlers:upgrades_read_by_id_handler;
import :domain_all;
import std;
import interfaces;
import blog_handlers_middleware;
import core_workflow;
import memory_backends;
import :upgrades_browse_step;
import :upgrades_read_by_id_flow;

export namespace blog::handlers::upgrades::read_by_id {

class ReadByIdHandler
{
public:
    ReadByIdHandler(blog::BlogContext& ctx) noexcept
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
        // Ghost: mw.authAdminApi + permissions:true; GET has no upgradeLimiter
        // and no staff-only check beyond the admin middleware.
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
            "read"
        );
        if (!allowed) {
            return;
        }

        const auto execution = m_workflow.run("upgrades_read_by_id", req);
        const auto* result = execution.result_of("upgrades_read_by_id_execute");
        const auto payload = result && result->typed_output.has_value()
                                 ? std::any_cast<support::Output>(result->typed_output)
                                 : support::Output{{"status", "200"}, {"body", "{}"}};
        const int status = payload.contains("status") ? std::stoi(payload.at("status")) : 200;
        const auto body = payload.contains("body") ? payload.at("body") : std::string{"{}"};

        std::vector<std::byte> bytes;
        for (const char c: body) {
            bytes.push_back(static_cast<std::byte>(c));
        }
        // cacheInvalidate:false => no X-Cache-Invalidate.
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

} // namespace blog::handlers::upgrades::read_by_id
