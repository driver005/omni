export module blog_handlers:slugs_get_detail_handler;
import :domain_all;
import std;
import interfaces;
import blog_handlers_middleware;
import core_workflow;
import memory_backends;
import :slugs_get_detail_flow;

export namespace blog::handlers::slugs::get_detail {
/**
 * @brief HTTP shell for Ghost `slugs.generate` (slugs.js:15-54).
 *
 * Strict-parity rewrite (group 58): input validation, slug generation and
 * dedup live in the step; this handler only performs the Ghost admin
 * permission gate, maps the step's status/body payload to IResponse and writes
 * the admin-wide Cache-Control header. No X-Cache-Invalidate is emitted
 * (slugs.js:19-21).
 */
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
        using blog::handlers::middleware::kGhostContextHeader;
        using blog::handlers::middleware::kGhostPermissionsHeader;
        using blog::handlers::middleware::kGhostRoleHeader;

        // Ghost route middleware mw.authAdminApi + controller permissions:true ->
        // canThis.generate.slug(id). A bare admin bearer is mapped to the default
        // Administrator principal; explicit x-blog-* headers stay authoritative.
        if (req.find_header(kGhostContextHeader).empty()) {
            req.set_header(kGhostContextHeader, "ADMIN_API");
        }
        if (!req.get_authorization().empty()) {
            if (req.find_header(kGhostRoleHeader).empty()) {
                req.set_header(kGhostRoleHeader, "Administrator");
            }
            if (req.find_header(kGhostPermissionsHeader).empty()) {
                req.set_header(kGhostPermissionsHeader, "*");
            }
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
            "slugs",
            "generate"
        );
        if (!allowed) {
            return;
        }

        const auto ex = m_workflow.run("slugs_generate", req);
        const auto* r = ex.result_of("slugs_generate_execute");
        const auto payload =
            r && r->typed_output.has_value()
                ? std::any_cast<std::map<std::string, std::string>>(r->typed_output)
                : std::map<std::string, std::string>{
                      {"status", "500"},
                      {"body",
                       blog::api::ApiError{}
                           .validation("Could not generate slug.", "", "", "InternalServerError")}
                  };
        const int status = payload.contains("status") ? std::stoi(payload.at("status")) : 200;
        const auto body = payload.contains("body") ? payload.at("body") : std::string{};
        std::vector<std::byte> bytes;
        for (char c: body) {
            bytes.push_back(static_cast<std::byte>(c));
        }

        // Admin app sets Cache-Control on every response (web/shared/middleware/
        // cache-control.ts:16-18); cacheInvalidate:false suppresses X-Cache-Invalidate.
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
} // namespace blog::handlers::slugs::get_detail
