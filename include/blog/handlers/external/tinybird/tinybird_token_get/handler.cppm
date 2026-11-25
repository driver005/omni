export module blog_handlers:tinybird_tinybird_token_get_handler;
import :domain_all;
import std;
import interfaces;
import blog_handlers_middleware;
import core_workflow;
import memory_backends;
import :tinybird_tinybird_token_get_flow;

export namespace blog::handlers::tinybird::tinybird_token_get {
class TinybirdTokenGetHandler
{
public:
    TinybirdTokenGetHandler(blog::BlogContext& ctx) noexcept
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

        // Ghost: route mw.authAdminApi (router chain keeps require_auth) plus the
        // controller's `permissions:{docName:'members', method:'browse'}`
        // (tinybird.js:11-14). The mock has no token-claims decoder, so a valid admin
        // bearer maps to Ghost's Administrator principal; explicit x-blog-* headers
        // are preserved so restricted roles stay testable.
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
            "members",
            "browse"
        );
        if (!allowed) {
            return;
        }

        const auto ex = m_workflow.run("tinybird_tinybird_token_get", req);
        const auto* r = ex.result_of("tinybird_tinybird_token_get_execute");
        auto body = r && r->typed_output.has_value() ? std::any_cast<std::string>(r->typed_output)
                                                     : std::string{"{\"tinybird\":null}"};
        std::vector<std::byte> bytes;
        for (char c: body) {
            bytes.push_back(static_cast<std::byte>(c));
        }
        // Ghost op sets headers.cacheInvalidate:false -> no X-Cache-Invalidate;
        // admin app installs cacheControl('private') on every route (admin/app.js:24,
        // cache-control.ts:16-19).
        res.set_header(
            interfaces::io::types::Token::CACHE_CONTROL,
            "no-cache, private, no-store, must-revalidate, max-stale=0, post-check=0, pre-check=0"
        );
        res.set_status(interfaces::io::types::Status::OK);
        res.set_body(std::move(bytes));
        send();
    }

private:
    core::workflow::Workflow m_workflow{};
};
} // namespace blog::handlers::tinybird::tinybird_token_get
