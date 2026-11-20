export module blog_handlers:invites_destroy_handler;
import :domain_all;
import :invites_dto_admin_resources;
import std;
import interfaces;
import blog_handlers_middleware;
import core_workflow;
import memory_backends;
import :invites_destroy_flow;
export namespace blog::handlers::invites::destroy {
class DestroyHandler {
public:
  DestroyHandler(blog::BlogContext& ctx) noexcept {
   workflow::ExecuteStep::s_ctx=&ctx;
   m_workflow.use_backend(std::make_unique<core::workflow::LocalWorkflowDriver>());
   m_workflow.register_workflow(workflow::MainFlow{}.build());
  }
  void handle(interfaces::io::IRequest& req, interfaces::io::IResponse& res, std::function<void()> send) {
   using blog::handlers::middleware::kGhostContextHeader;
   using blog::handlers::middleware::kGhostPermissionsHeader;
   using blog::handlers::middleware::kGhostRoleHeader;
   if (req.find_header(kGhostContextHeader).empty()) req.set_header(kGhostContextHeader,"ADMIN_API");
   if (!req.get_authorization().empty() && req.find_header(kGhostRoleHeader).empty()) {
    req.set_header(kGhostRoleHeader,"Administrator");
    req.set_header(kGhostPermissionsHeader,"*");
   }
   bool allowed{false};
   blog::handlers::middleware::require_ghost_permission(
    req, res,
    [&allowed](interfaces::io::IRequest&, interfaces::io::IResponse&, std::function<void()>) noexcept { allowed=true; },
    send, support::kDocName, "destroy");
   if (!allowed) return;

   const auto ex=m_workflow.run("invites_destroy", req);
   const auto* r=ex.result_of("invites_destroy_execute");
   auto payload=r&&r->typed_output.has_value()
    ?std::any_cast<std::map<std::string,std::string>>(r->typed_output)
    :std::map<std::string,std::string>{{"status","404"},{"body",blog::api::ApiError{}.json("Invite not found.","Not Found")}};
   const auto code=payload.contains("status")?payload.at("status"):std::string{"404"};
   auto status=interfaces::io::types::Status::NO_CONTENT;
   if (code=="404") status=interfaces::io::types::Status::NOT_FOUND;
   else if (code=="422") status=interfaces::io::types::Status::UNPROCESSABLE_CONTENT;
   else if (code!="204") status=interfaces::io::types::Status::OK;
   // Ghost headers.cacheInvalidate:false -> no X-Cache-Invalidate header.
   res.set_status(status);
   if (code!="204") {
    const auto body=payload.contains("body")?payload.at("body"):std::string{};
    std::vector<std::byte> bytes; for(char c:body) bytes.push_back(static_cast<std::byte>(c));
    res.set_body(std::move(bytes));
   }
   send();
  }
private: core::workflow::Workflow m_workflow{};
};
}
