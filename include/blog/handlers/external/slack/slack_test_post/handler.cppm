export module blog_handlers:slack_slack_test_post_handler;
import :domain_all;
import std;
import interfaces;
import blog_handlers_middleware;
import core_workflow;
import memory_backends;
import :slack_slack_test_post_flow;

export namespace blog::handlers::slack::slack_test_post {

class SlackTestPostHandler {
public:
    explicit SlackTestPostHandler(blog::BlogContext& ctx) noexcept
    {
        workflow::ExecuteStep::s_ctx = &ctx;
        m_workflow.use_backend(std::make_unique<core::workflow::LocalWorkflowDriver>());
        m_workflow.register_workflow(workflow::MainFlow{}.build());
    }

    void handle(interfaces::io::IRequest& req, interfaces::io::IResponse& res,
                std::function<void()> send)
    {
        using blog::handlers::middleware::kGhostContextHeader;
        using blog::handlers::middleware::kGhostPermissionsHeader;
        using blog::handlers::middleware::kGhostRoleHeader;

        // Ghost: router-level mw.authAdminApi, controller permissions:false
        // (slack.js:11) — any authenticated admin/user token is accepted, an
        // unauthenticated request is 401. Route through the shared Ghost
        // permission service instead of a bare token-presence check.
        const bool authenticated = !req.get_authorization().empty();
        if (authenticated && req.find_header(kGhostContextHeader).empty()) {
            req.set_header(kGhostContextHeader, "ADMIN_API");
        }
        if (authenticated && req.find_header(kGhostRoleHeader).empty()) {
            req.set_header(kGhostRoleHeader, "Administrator");
            req.set_header(kGhostPermissionsHeader, "*");
        }
        bool allowed{false};
        blog::handlers::middleware::require_ghost_permission(
            req, res,
            [&allowed](interfaces::io::IRequest&, interfaces::io::IResponse&,
                       std::function<void()>) noexcept { allowed = true; },
            send, "slack", "add");
        if (!allowed) return;

        // sendTest consumes no options/data/body (slack.js:7-15). The workflow
        // step only emits the shared 'slack.test' event through the outbox.
        const auto execution = m_workflow.run("slack_slack_test_post", req);
        const auto* result = execution.result_of("slack_slack_test_post_execute");
        const auto payload = result && result->typed_output.has_value()
            ? std::any_cast<std::map<std::string, std::string>>(result->typed_output)
            : std::map<std::string, std::string>{{"status", "200"}, {"body", "{}"}};
        const int status = payload.contains("status") ? std::stoi(payload.at("status")) : 200;
        const std::string body = payload.contains("body") ? payload.at("body") : std::string{"{}"};

        std::vector<std::byte> bytes;
        for (const char c : body) bytes.push_back(static_cast<std::byte>(c));

        // Ghost admin app app-level cacheControl('private') (admin/app.js:24).
        // headers.cacheInvalidate:false (slack.js:8-10) => no X-Cache-Invalidate.
        res.set_header(
            "Cache-Control",
            "no-cache, private, no-store, must-revalidate, max-stale=0, post-check=0, pre-check=0");
        res.set_status(static_cast<interfaces::io::types::Status>(status));
        res.set_body(std::move(bytes));
        send();
    }

private:
    core::workflow::Workflow m_workflow{};
};

} // namespace blog::handlers::slack::slack_test_post
