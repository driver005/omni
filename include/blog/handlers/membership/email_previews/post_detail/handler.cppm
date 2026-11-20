export module blog_handlers:email_previews_post_detail_handler;
import :domain_all;
import :email_previews_dto_subresources;
import std;
import interfaces;
import blog_handlers_middleware;
import core_workflow;
import memory_backends;
import :email_previews_post_detail_flow;
export namespace blog::handlers::email_previews::post_detail {
class PostDetailHandler {
public:
  PostDetailHandler(blog::BlogContext& ctx) noexcept {
   workflow::ExecuteStep::s_ctx=&ctx;
   m_workflow.use_backend(std::make_unique<core::workflow::LocalWorkflowDriver>());
   m_workflow.register_workflow(workflow::MainFlow{}.build());
  }
  void handle(interfaces::io::IRequest& req, interfaces::io::IResponse& res, std::function<void()> send) {
   // Ghost route: rate limiter -> mw.authAdminApi -> permissions:true
   // (docName email_previews, method sendTestEmail). require_auth is the 401
   // gate; GhostAuthorizer supplies the 403 NoPermissionError gate. The
   // previewEmailLimiter 429 gate is a router-level residual (no primitive).
   using blog::handlers::middleware::kGhostContextHeader;
   using blog::handlers::middleware::kGhostPermissionsHeader;
   using blog::handlers::middleware::kGhostRoleHeader;
   const bool authenticated=!req.get_authorization().empty();
   if (authenticated && req.find_header(kGhostContextHeader).empty())
    req.set_header(kGhostContextHeader, "ADMIN_API");
   if (authenticated && req.find_header(kGhostRoleHeader).empty()) {
    req.set_header(kGhostRoleHeader, "Administrator");
    req.set_header(kGhostPermissionsHeader, "*");
   }
   bool allowed{false};
   blog::handlers::middleware::require_ghost_permission(
    req, res,
    [&allowed](interfaces::io::IRequest&, interfaces::io::IResponse&, std::function<void()>) noexcept { allowed=true; },
    send, "email_previews", "sendTestEmail");
   if (!allowed) return;

   const auto ex=m_workflow.run("email_previews_post_detail", req);
   const auto* r=ex.result_of("email_previews_post_detail_execute");
   auto payload=r&&r->typed_output.has_value()
    ?std::any_cast<std::map<std::string,std::string>>(r->typed_output)
    :std::map<std::string,std::string>{{"status","204"},{"body",""}};
   const int status=payload.contains("status")?std::stoi(payload.at("status")):204;
   const auto body=payload.contains("body")?payload.at("body"):std::string{};
   // Ghost statusCode:204 with an empty body; no X-Cache-Invalidate header.
   res.set_status(static_cast<interfaces::io::types::Status>(status));
   if (!body.empty()) {
    std::vector<std::byte> bytes; for(char c:body) bytes.push_back(static_cast<std::byte>(c));
    res.set_body(std::move(bytes));
   }
   send();
  }
 private: core::workflow::Workflow m_workflow{};
};
}
