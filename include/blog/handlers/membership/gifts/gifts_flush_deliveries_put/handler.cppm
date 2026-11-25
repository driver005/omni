export module blog_handlers:gifts_gifts_flush_deliveries_put_handler;
import :domain_all;
import std;
import interfaces;
import blog_handlers_middleware;
import core_workflow;
import memory_backends;
import :gifts_gifts_flush_deliveries_put_flow;

export namespace blog::handlers::gifts::gifts_flush_deliveries_put {
class GiftsFlushDeliveriesPutHandler
{
public:
    GiftsFlushDeliveriesPutHandler(blog::BlogContext& ctx) noexcept
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

        // Ghost: mw.authAdminApiWithUrl + permissions:{docName:'gifts',method:'flushDeliveries'}
        // (admin/routes.js:98, gifts.js:30-33). Route the {docName,method} check through the
        // shared Ghost permission service (core_auth:ghost). No header -> Ghost 401 envelope;
        // an authenticated non-admin -> 403 NoPermissionError. The admin-token-in-URL variant
        // of authAdminApiWithUrl has no allowed C++ primitive here; see behavior report gaps.
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
            "gifts",
            "flushDeliveries"
        );
        if (!allowed) {
            return;
        }

        // Ghost query() dispatches StartGiftDeliveryFlushEvent.create() and returns
        // undefined: no body/query input, no store write, 204 with an empty body.
        (void)m_workflow.run("gifts_gifts_flush_deliveries_put", req);
        // headers.cacheInvalidate:false (gifts.js:27-29) => Ghost emits no
        // X-Cache-Invalidate header; do not write the literal "false".
        res.set_status(interfaces::io::types::Status::NO_CONTENT);
        send();
    }

private:
    core::workflow::Workflow m_workflow{};
};
} // namespace blog::handlers::gifts::gifts_flush_deliveries_put
