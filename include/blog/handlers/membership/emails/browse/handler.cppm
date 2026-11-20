export module blog_handlers:emails_browse_handler;
import :domain_all;
import :emails_dto_analytics_job;
import :emails_dto_analytics_status;
import :emails_dto_analytics_status_input;
import :emails_dto_scheduled_analytics_job;
import :emails_model_member;
import :emails_model_wire_fixtures;
import :emails_operations;
import std;
import interfaces;
import blog_handlers_middleware;
import core_workflow;
import memory_backends;
import :emails_browse_flow;
export namespace blog::handlers::emails::browse {
class BrowseHandler {
public:
  BrowseHandler(blog::BlogContext& ctx) noexcept {
   workflow::ExecuteStep::s_ctx=&ctx;
   m_workflow.use_backend(std::make_unique<core::workflow::LocalWorkflowDriver>());
   m_workflow.register_workflow(workflow::MainFlow{}.build());
  }
  void handle(interfaces::io::IRequest& req, interfaces::io::IResponse& res, std::function<void()> send) {
   // Ghost mw.authAdminApi + controller permissions (docName emails).
   // Router-level require_auth stays as the 401 gate; GhostAuthorizer
   // supplies the 403 NoPermissionError gate for the declared method.
   if (!blog::api::emails::permitted(req, res, send, "emails", "browse")) return;
   // Auth moved to the router chain: content_type_middleware + require_auth.
   // Ghost browse declares cacheInvalidate:false -> no X-Cache-Invalidate header.
   const auto ex=m_workflow.run("emails_browse", req);
   const auto* r=ex.result_of("emails_browse_execute");
   auto body=r&&r->typed_output.has_value()?std::any_cast<std::string>(r->typed_output):std::string{"{}"};
   std::vector<std::byte> bytes; for(char c:body) bytes.push_back(static_cast<std::byte>(c));
   res.set_status(interfaces::io::types::Status::OK); res.set_body(std::move(bytes)); send();
  }
private: core::workflow::Workflow m_workflow{};
};
}
