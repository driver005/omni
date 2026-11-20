export module blog_handlers:labels_edit_handler;
import :domain_all;
import std;
import interfaces;
import blog_handlers_middleware;
import core_workflow;
import memory_backends;
import :labels_edit_flow;

export namespace blog::handlers::labels::edit {

class EditHandler
{
public:
    explicit EditHandler(blog::BlogContext& ctx) noexcept
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
        if (req.find_header(kGhostContextHeader).empty()) {
            req.set_header(kGhostContextHeader, "ADMIN_API");
        }
        if (!req.get_authorization().empty() && req.find_header(kGhostRoleHeader).empty()) {
            req.set_header(kGhostRoleHeader, "Administrator");
            req.set_header(kGhostPermissionsHeader, "*");
        }
        bool allowed{false};
        blog::handlers::middleware::require_ghost_permission(
            req, res,
            [&allowed](interfaces::io::IRequest&, interfaces::io::IResponse&,
                       std::function<void()>) noexcept { allowed = true; },
            send, "labels", "edit");
        if (!allowed) return;

        const auto execution = m_workflow.run("labels_edit", req);
        const auto* result = execution.result_of("labels_edit_execute");
        auto payload = result && result->typed_output.has_value()
                           ? std::any_cast<std::map<std::string, std::string>>(result->typed_output)
                           : std::map<std::string, std::string>{{"status", "404"},
                                                                {"body", "{}"}};
        const std::string code = payload.contains("status") ? payload.at("status")
                                                            : std::string{"404"};
        const std::string body = payload.contains("body") ? payload.at("body")
                                                          : std::string{"{}"};
        const bool changed = payload.contains("changed") && payload.at("changed") == "true";
        std::vector<std::byte> bytes;
        for (const char c : body) bytes.push_back(static_cast<std::byte>(c));
        auto status = interfaces::io::types::Status::OK;
        if (code == "400") status = interfaces::io::types::Status::BAD_REQUEST;
        else if (code == "404") status = interfaces::io::types::Status::NOT_FOUND;
        else if (code == "422") status = interfaces::io::types::Status::UNPROCESSABLE_CONTENT;
        // model.wasChanged() -> frame.setHeader('X-Cache-Invalidate', '/*').
        if (changed) res.set_header("X-Cache-Invalidate", "/*");
        res.set_status(status);
        res.set_body(std::move(bytes));
        send();
    }

private:
    core::workflow::Workflow m_workflow{};
};

} // namespace blog::handlers::labels::edit
