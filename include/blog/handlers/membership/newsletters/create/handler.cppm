export module blog_handlers:newsletters_create_handler;
import :domain_all;
import std;
import interfaces;
import blog_handlers_middleware;
import core_workflow;
import memory_backends;
import :newsletters_create_flow;
export namespace blog::handlers::newsletters::create {
class CreateHandler {
public:
  CreateHandler(blog::BlogContext& ctx) noexcept {
   workflow::ExecuteStep::s_ctx=&ctx;
   m_workflow.use_backend(std::make_unique<core::workflow::LocalWorkflowDriver>());
   m_workflow.register_workflow(workflow::MainFlow{}.build());
  }
  void handle(interfaces::io::IRequest& req, interfaces::io::IResponse& res, std::function<void()> send) {
   // Ghost route-level mw.authAdminApi + controller `permissions:true`.
   using blog::handlers::middleware::kGhostContextHeader;
   using blog::handlers::middleware::kGhostPermissionsHeader;
   using blog::handlers::middleware::kGhostRoleHeader;
   const bool authenticated = !req.get_authorization().empty();
   if (authenticated && req.find_header(kGhostContextHeader).empty())
    req.set_header(kGhostContextHeader, "ADMIN_API");
   if (authenticated && req.find_header(kGhostRoleHeader).empty()) {
    req.set_header(kGhostRoleHeader, "Administrator");
    req.set_header(kGhostPermissionsHeader, "*");
   }
   bool allowed{false};
   blog::handlers::middleware::require_ghost_permission(
    req, res,
    [&allowed](interfaces::io::IRequest&, interfaces::io::IResponse&, std::function<void()>) noexcept { allowed = true; },
    send, "newsletters", "add");
   if (!allowed) return;

   const auto ex=m_workflow.run("newsletters_create", req);
   const auto* r=ex.result_of("newsletters_create_execute");
   auto payload=r&&r->typed_output.has_value()
    ?std::any_cast<std::map<std::string,std::string>>(r->typed_output)
    :std::map<std::string,std::string>{{"status","200"},{"body","{}"}};
   const int status=payload.contains("status")?std::stoi(payload.at("status")):200;
   const auto body=payload.contains("body")?payload.at("body"):std::string{"{}"};
   std::vector<std::byte> bytes; for(char c:body) bytes.push_back(static_cast<std::byte>(c));
   // Ghost headers.cacheInvalidate:true -> X-Cache-Invalidate: /*
   res.set_header("X-Cache-Invalidate", "/*");
   res.set_status(static_cast<interfaces::io::types::Status>(status));
   res.set_body(std::move(bytes)); send();
  }
 private: core::workflow::Workflow m_workflow{};
};
}
