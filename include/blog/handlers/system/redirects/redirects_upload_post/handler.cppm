export module blog_handlers:redirects_redirects_upload_post_handler;
import :domain_all;
import std;
import interfaces;
import blog_handlers_middleware;
import core_workflow;
import memory_backends;
import :redirects_redirects_upload_post_flow;

export namespace blog::handlers::redirects::redirects_upload_post {
class RedirectsUploadPostHandler
{
public:
    RedirectsUploadPostHandler(blog::BlogContext& ctx) noexcept
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

        // Ghost: mw.authAdminApi + permissions:true with docName:'redirects' and the
        // custom action `upload` (routes.js:506-512, redirects.js:12,32-35).
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
            "redirects",
            "upload"
        );
        if (!allowed) {
            return;
        }

        const auto ex = m_workflow.run("redirects_redirects_upload_post", req);
        const auto* r = ex.result_of("redirects_redirects_upload_post_execute");
        const auto payload =
            r && r->typed_output.has_value()
                ? std::any_cast<std::map<std::string, std::string>>(r->typed_output)
                : std::map<std::string, std::string>{{"status", "200"}, {"body", "{}"}};
        const auto code = payload.contains("status") ? payload.at("status") : std::string{"200"};
        const auto body = payload.contains("body") ? payload.at("body") : std::string{"{}"};

        auto status = interfaces::io::types::Status::OK;
        if (code == "400") {
            status = interfaces::io::types::Status::BAD_REQUEST;
        } else if (code == "415") {
            status = interfaces::io::types::Status::UNSUPPORTED_MEDIA_TYPE;
        } else if (code == "422") {
            status = interfaces::io::types::Status::UNPROCESSABLE_CONTENT;
        }

        // headers.get() runs only on success (http.ts:135-137), so the invalidation
        // header is emitted on the 2xx acknowledgement only (redirects.js:35).
        if (code == "200") {
            res.set_header("X-Cache-Invalidate", "/*");
        }
        std::vector<std::byte> bytes;
        for (char c: body) {
            bytes.push_back(static_cast<std::byte>(c));
        }
        res.set_status(status);
        res.set_body(std::move(bytes));
        send();
    }

private:
    core::workflow::Workflow m_workflow{};
};
} // namespace blog::handlers::redirects::redirects_upload_post
