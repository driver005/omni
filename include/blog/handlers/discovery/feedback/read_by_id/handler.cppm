export module blog_handlers:feedback_read_by_id_handler;
import :domain_all;
import std;
import interfaces;
import blog_handlers_middleware;
import core_auth;
import core_workflow;
import memory_backends;
import :feedback_read_by_id_step;
import :feedback_read_by_id_flow;

export namespace blog::handlers::feedback::read_by_id {

class ReadByIdHandler
{
public:
    explicit ReadByIdHandler(blog::BlogContext& ctx) noexcept
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
        using blog::handlers::middleware::kGhostStaffHeader;

        // authAdminApi resolves the admin token to a staff identity. No token
        // store exists here, so a presented admin token is mapped to Ghost's
        // Administrator role (the same convention used by the admin handlers).
        if (req.find_header(kGhostContextHeader).empty()) {
            req.set_header(kGhostContextHeader, "ADMIN_API");
        }
        if (!req.get_authorization().empty() && req.find_header(kGhostRoleHeader).empty()) {
            req.set_header(kGhostRoleHeader, "Administrator");
            req.set_header(kGhostPermissionsHeader, "*");
            req.set_header(kGhostStaffHeader, "true");
        }

        // Declared controller permission: {docName:'posts', method:'browse'}.
        bool allowed{false};
        blog::handlers::middleware::require_ghost_permission(
            req, res,
            [&allowed](interfaces::io::IRequest&, interfaces::io::IResponse&,
                       std::function<void()>) noexcept { allowed = true; },
            send, "posts", "browse");
        if (!allowed) return;

        // In-service downgrade gate: permissions.canThis(context).browse.member()
        // succeeds only for an identity allowed to browse members; any throw is
        // swallowed by Ghost and simply omits the `member` relation.
        const auto principal = blog::handlers::middleware::principal_from_request(req);
        const auto member_right =
            core::auth::ghost::GhostAuthorizer{}.authorize(principal, "members", "browse");
        req.set_header(support::kWithMemberHeader, member_right.allowed ? "true" : "false");

        const auto execution = m_workflow.run("feedback_read_by_id", req);
        const auto* result = execution.result_of("feedback_read_by_id_execute");
        const auto payload = result && result->typed_output.has_value()
                                 ? std::any_cast<std::map<std::string, std::string>>(
                                       result->typed_output)
                                 : std::map<std::string, std::string>{{"status", "200"},
                                                                      {"body", "{}"}};
        const std::string code = payload.contains("status") ? payload.at("status")
                                                            : std::string{"200"};
        const std::string body = payload.contains("body") ? payload.at("body")
                                                          : std::string{"{}"};
        std::vector<std::byte> bytes;
        for (const char c : body) bytes.push_back(static_cast<std::byte>(c));

        auto status = interfaces::io::types::Status::OK;
        if (code == "400") status = interfaces::io::types::Status::BAD_REQUEST;
        else if (code == "401") status = interfaces::io::types::Status::UNAUTHORIZED;
        else if (code == "403") status = interfaces::io::types::Status::FORBIDDEN;
        else if (code == "404") status = interfaces::io::types::Status::NOT_FOUND;
        else if (code == "422") status = interfaces::io::types::Status::UNPROCESSABLE_CONTENT;

        // controller headers.cacheInvalidate:false -> X-Cache-Invalidate:false.
        res.set_header("X-Cache-Invalidate", "false");
        res.set_status(status);
        res.set_body(std::move(bytes));
        send();
    }

private:
    core::workflow::Workflow m_workflow{};
};

} // namespace blog::handlers::feedback::read_by_id
