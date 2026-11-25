export module blog_handlers:session_bulk_destroy_handler;

import :domain_all;
import std;
import interfaces;
import core_auth;
import blog_handlers_middleware;
import core_workflow;
import memory_backends;
import :session_bulk_destroy_flow;

export namespace blog::handlers::session::bulk_destroy {

class BulkDestroyHandler
{
public:
    BulkDestroyHandler(blog::BlogContext& ctx) noexcept
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
        using interfaces::io::types::Status;

        // Ghost route: mw.authAdminApi (routes.js:424). Use the shared Ghost
        // authorizer rather than a bare token-presence check.
        if (blog::handlers::middleware::token(req).empty()) {
            blog::handlers::middleware::write_error(
                res,
                Status::UNAUTHORIZED,
                blog::handlers::middleware::ErrorEnvelope{
                    std::vector{blog::handlers::middleware::ErrorItem{
                        .message = "Unauthorized",
                        .type = "UnauthorizedError"
                    }}
                },
                std::move(send)
            );
            return;
        }
        auto principal = blog::handlers::middleware::principal_from_request(req);
        if (principal.context == core::auth::ghost::ApiContext::ANONYMOUS) {
            principal.context = core::auth::ghost::ApiContext::ADMIN_API;
        }
        if (principal.role.empty()) {
            principal.role = "Administrator";
        }
        principal.id = "bearer";
        // authAdminApi authenticates the caller; it does not apply a
        // resource-specific permission check to logout (session.ts:88-96).
        principal.permissions.insert("session:destroy");
        const auto decision =
            core::auth::ghost::GhostAuthorizer{}.authorize(principal, "session", "destroy");
        if (!decision.allowed) {
            blog::handlers::middleware::write_error(
                res,
                decision.status == 401 ? Status::UNAUTHORIZED : Status::FORBIDDEN,
                blog::handlers::middleware::ErrorEnvelope{
                    std::vector{blog::handlers::middleware::ErrorItem{
                        .message = decision.status == 401
                                       ? "Unauthorized"
                                       : "You do not have permission to delete session",
                        .type = decision.status == 401 ? "UnauthorizedError" : "NoPermissionError"
                    }}
                },
                std::move(send)
            );
            return;
        }

        const auto ex = m_workflow.run("session_bulk_destroy", req);
        const auto* r = ex.result_of("session_bulk_destroy_execute");
        const auto payload =
            r && r->typed_output.has_value()
                ? std::any_cast<std::map<std::string, std::string>>(r->typed_output)
                : std::map<std::string, std::string>{{"status", "204"}, {"body", ""}};
        const int code = payload.contains("status") ? std::stoi(payload.at("status")) : 204;
        const auto body = payload.contains("body") ? payload.at("body") : std::string{};

        // Ghost's logout issues no explicit Set-Cookie; the server-side session
        // is invalidated in place.  Status 204 with an empty body.
        std::vector<std::byte> bytes;
        for (char c: body) {
            bytes.push_back(static_cast<std::byte>(c));
        }
        res.set_status(static_cast<Status>(code));
        res.set_body(std::move(bytes));
        send();
    }

private:
    core::workflow::Workflow m_workflow{};
};

} // namespace blog::handlers::session::bulk_destroy
