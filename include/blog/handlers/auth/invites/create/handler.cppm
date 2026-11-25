export module blog_handlers:invites_create_handler;
import :domain_all;
import :invites_dto_admin_resources;
import std;
import interfaces;
import blog_handlers_middleware;
import core_workflow;
import memory_backends;
import :invites_create_flow;

export namespace blog::handlers::invites::create {
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
        using blog::handlers::middleware::kGhostContextHeader;
        using blog::handlers::middleware::kGhostPermissionsHeader;
        using blog::handlers::middleware::kGhostRoleHeader;
        if (req.find_header(kGhostContextHeader).empty()) {
            req.set_header(kGhostContextHeader, "ADMIN_API");
        }
        if (!req.get_authorization().empty() && req.find_header(kGhostRoleHeader).empty()) {
            req.set_header(kGhostRoleHeader, "Administrator");
            req.set_header(kGhostPermissionsHeader, "*");
        }
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

        const auto ex = m_workflow.run("invites_create", req);
        const auto* r = ex.result_of("invites_create_execute");
        auto payload = r && r->typed_output.has_value()
                           ? std::any_cast<std::map<std::string, std::string>>(r->typed_output)
                           : std::map<std::string, std::string>{{"status", "200"}, {"body", "{}"}};
        const auto code = payload.contains("status") ? payload.at("status") : std::string{"200"};
        const auto body = payload.contains("body") ? payload.at("body") : std::string{"{}"};
        std::vector<std::byte> bytes;
        for (char c: body) {
            bytes.push_back(static_cast<std::byte>(c));
        }
        auto status = interfaces::io::types::Status::OK;
        if (code == "201") {
            status = interfaces::io::types::Status::CREATED;
        } else if (code == "400") {
            status = interfaces::io::types::Status::BAD_REQUEST;
        } else if (code == "401") {
            status = interfaces::io::types::Status::UNAUTHORIZED;
        } else if (code == "403") {
            status = interfaces::io::types::Status::FORBIDDEN;
        } else if (code == "404") {
            status = interfaces::io::types::Status::NOT_FOUND;
        } else if (code == "422") {
            status = interfaces::io::types::Status::UNPROCESSABLE_CONTENT;
        }
        // Ghost headers.cacheInvalidate:false -> no X-Cache-Invalidate header.
        if (code == "201" && payload.contains("location")) {
            res.set_header("Location", payload.at("location"));
        }
        res.set_status(status);
        res.set_body(std::move(bytes));
        send();
    }

private:
    core::workflow::Workflow m_workflow{};
};
} // namespace blog::handlers::invites::create
