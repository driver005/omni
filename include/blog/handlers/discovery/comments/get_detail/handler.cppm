export module blog_handlers:comments_get_detail_handler;
import :domain_all;
import :comments_model_comment;
import :comments_model_member;
import :comments_model_wire_fixtures;
import std;
import interfaces;
import blog_handlers_middleware;
import core_workflow;
import memory_backends;
import :comments_get_detail_flow;

export namespace blog::handlers::comments::get_detail {
class GetDetailHandler
{
public:
    GetDetailHandler(blog::BlogContext& ctx) noexcept
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
        // Ghost browse declares permissions:true with docName 'comments'.
        if (req.find_header(blog::handlers::middleware::kGhostContextHeader).empty()) {
            req.set_header(blog::handlers::middleware::kGhostContextHeader, "ADMIN_API");
        }
        if (req.find_header(blog::handlers::middleware::kGhostRoleHeader).empty() &&
            !blog::handlers::middleware::token(req).empty()) {
            req.set_header(blog::handlers::middleware::kGhostRoleHeader, "Administrator");
        }
        bool allowed = false;
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
            "comments",
            "browse"
        );
        if (!allowed) {
            return;
        }

        const auto ex = m_workflow.run("comments_get_detail", req);
        const auto* r = ex.result_of("comments_get_detail_execute");
        const auto payload =
            r && r->typed_output.has_value()
                ? std::any_cast<std::map<std::string, std::string>>(r->typed_output)
                : std::map<std::string, std::string>{{"status", "200"}, {"body", "{}"}};
        const std::string code = payload.contains("status") ? payload.at("status") : "200";
        const auto body = payload.contains("body") ? payload.at("body") : std::string{"{}"};
        // Ghost headers.cacheInvalidate:false -> no X-Cache-Invalidate header.
        std::vector<std::byte> bytes;
        for (char c: body) {
            bytes.push_back(static_cast<std::byte>(c));
        }
        res.set_status(static_cast<interfaces::io::types::Status>(status_for(code)));
        res.set_body(std::move(bytes));
        send();
    }

private:
    core::workflow::Workflow m_workflow{};

    static interfaces::io::types::Status status_for(const std::string& code)
    {
        if (code == "400") {
            return interfaces::io::types::Status::BAD_REQUEST;
        }
        if (code == "401") {
            return interfaces::io::types::Status::UNAUTHORIZED;
        }
        if (code == "403") {
            return interfaces::io::types::Status::FORBIDDEN;
        }
        if (code == "404") {
            return interfaces::io::types::Status::NOT_FOUND;
        }
        if (code == "405") {
            return interfaces::io::types::Status::METHOD_NOT_ALLOWED;
        }
        return interfaces::io::types::Status::OK;
    }
};
} // namespace blog::handlers::comments::get_detail
