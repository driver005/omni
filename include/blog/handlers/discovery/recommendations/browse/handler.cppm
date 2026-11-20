export module blog_handlers:recommendations_browse_handler;
import :domain_all;
import std;
import interfaces;
import blog_handlers_middleware;
import core_workflow;
import memory_backends;
import :recommendations_browse_step;
import :recommendations_browse_flow;
export namespace blog::handlers::recommendations::browse {
class BrowseHandler {
public:
  BrowseHandler(blog::BlogContext& ctx) noexcept {
   workflow::ExecuteStep::s_ctx=&ctx;
   blog::handlers::recommendations::track_clicked::workflow::ExecuteStep::s_ctx=&ctx;
   blog::handlers::recommendations::track_subscribed::workflow::ExecuteStep::s_ctx=&ctx;
   m_workflow.use_backend(std::make_unique<core::workflow::LocalWorkflowDriver>());
   m_workflow.register_workflow(workflow::MainFlow{}.build());
   m_workflow.register_workflow(
       blog::handlers::recommendations::track_clicked::workflow::MainFlow{}.build());
   m_workflow.register_workflow(
       blog::handlers::recommendations::track_subscribed::workflow::MainFlow{}.build());
  }

  /** @brief Ghost `permissions:true` contract hook (two-arg shape). */
  static bool gated(interfaces::io::IRequest& req, interfaces::io::IResponse& res,
                    std::function<void()>& send, std::string_view method)
  {
   if (req.find_header(blog::handlers::middleware::kGhostContextHeader).empty()) return false;
   bool denied=false;
   blog::handlers::middleware::GhostPermissionRegistry::set_active(
       &req,&res,[&]{ denied=true; send(); },nullptr);
   blog::handlers::middleware::require_ghost_permission("recommendations",method);
   blog::handlers::middleware::GhostPermissionRegistry::clear_active();
   return denied;
  }

  void handle(interfaces::io::IRequest& req, interfaces::io::IResponse& res, std::function<void()> send) {
   if (gated(req,res,send,"browse")) return;
   const auto ex=m_workflow.run("recommendations_browse", req);
   const auto* r=ex.result_of("recommendations_browse_execute");
   auto body=r&&r->typed_output.has_value()?std::any_cast<std::string>(r->typed_output):std::string{"{}"};
   // Ghost browse has cacheInvalidate:false -> emit no X-Cache-Invalidate header.
   const auto status=body.starts_with(R"({"errors":)")
       ? interfaces::io::types::Status::BAD_REQUEST : interfaces::io::types::Status::OK;
   std::vector<std::byte> bytes; for(char c:body) bytes.push_back(static_cast<std::byte>(c));
   res.set_status(status); res.set_body(std::move(bytes)); send();
  }

  void handle_clicked(interfaces::io::IRequest& req, interfaces::io::IResponse& res,
                      std::function<void()> send) {
   if (gated(req,res,send,"browse")) return;
   dispatch("recommendations_track_clicked","recommendations_track_clicked_execute",
            req,res,send);
  }

  void handle_subscribed(interfaces::io::IRequest& req, interfaces::io::IResponse& res,
                         std::function<void()> send) {
   if (gated(req,res,send,"browse")) return;
   dispatch("recommendations_track_subscribed","recommendations_track_subscribed_execute",
            req,res,send);
  }
private:
  void dispatch(std::string_view flow, std::string_view step,
                interfaces::io::IRequest& req, interfaces::io::IResponse& res,
                std::function<void()> send)
  {
   const auto ex=m_workflow.run(flow, req);
   const auto* r=ex.result_of(step);
   const auto payload=r&&r->typed_output.has_value()
      ?std::any_cast<std::map<std::string,std::string>>(r->typed_output)
      :std::map<std::string,std::string>{{"status","204"},{"body",""}};
   const int status=payload.contains("status")?std::stoi(payload.at("status")):204;
   const auto body=payload.contains("body")?payload.at("body"):std::string{};
   std::vector<std::byte> bytes; for(char c:body) bytes.push_back(static_cast<std::byte>(c));
   // Ghost tracking has cacheInvalidate:false -> emit no X-Cache-Invalidate header.
   res.set_status(static_cast<interfaces::io::types::Status>(status));
   res.set_body(std::move(bytes)); send();
  }
  core::workflow::Workflow m_workflow{};
};
}
