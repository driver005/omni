export module blog_handlers:custom_theme_settings_bulk_edit_handler;
import :domain_all;
import std;
import interfaces;
import blog_handlers_middleware;
import core_workflow;
import memory_backends;
import :custom_theme_settings_bulk_edit_flow;
export namespace blog::handlers::custom_theme_settings::bulk_edit {
class BulkEditHandler {
public:
  BulkEditHandler(blog::BlogContext& ctx) noexcept {
   workflow::ExecuteStep::s_ctx=&ctx;
   m_workflow.use_backend(std::make_unique<core::workflow::LocalWorkflowDriver>());
   m_workflow.register_workflow(workflow::MainFlow{}.build());
  }
  void handle(interfaces::io::IRequest& req, interfaces::io::IResponse& res, std::function<void()> send) {
   using blog::handlers::middleware::kGhostContextHeader;
   using blog::handlers::middleware::kGhostPermissionsHeader;
   using blog::handlers::middleware::kGhostRoleHeader;
   // Ghost `permissions:true` / docName `custom_theme_settings` (custom-theme-settings.js:5,21)
   // -> canThis(context).edit.custom_theme_setting via the shared authorizer.
   if (req.find_header(kGhostContextHeader).empty()) req.set_header(kGhostContextHeader,"ADMIN_API");
   if (!req.get_authorization().empty() && req.find_header(kGhostRoleHeader).empty()) {
    req.set_header(kGhostRoleHeader,"Administrator");
    req.set_header(kGhostPermissionsHeader,"*");
   }
   bool allowed{false};
   blog::handlers::middleware::require_ghost_permission(
    req, res,
    [&allowed](interfaces::io::IRequest&, interfaces::io::IResponse&, std::function<void()>) noexcept { allowed=true; },
    send, "custom_theme_settings", "edit");
   if (!allowed) return;

   const auto ex=m_workflow.run("custom_theme_settings_bulk_edit", req);
   const auto* r=ex.result_of("custom_theme_settings_bulk_edit_execute");
   auto payload=r&&r->typed_output.has_value()
    ? std::any_cast<std::map<std::string,std::string>>(r->typed_output)
    : std::map<std::string,std::string>{{"status","200"},{"body","{}"}};
   const int status=payload.contains("status")?std::stoi(payload.at("status")):200;
   const auto body=payload.contains("body")?payload.at("body"):std::string{"{}"};
   std::vector<std::byte> bytes; for(char c:body) bytes.push_back(static_cast<std::byte>(c));
   res.set_status(static_cast<interfaces::io::types::Status>(status));
   // Ghost headers.cacheInvalidate:true -> X-Cache-Invalidate: /* on success only;
   // errors skip headers.get and carry no invalidation header (headers.ts:9,135-144).
   if (status>=200 && status<300) res.set_header("X-Cache-Invalidate","/*");
   res.set_body(std::move(bytes)); send();
  }
 private: core::workflow::Workflow m_workflow{};
};
}
