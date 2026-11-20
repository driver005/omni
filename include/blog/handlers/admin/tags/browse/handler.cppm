export module blog_handlers:tags_browse_handler;
import :domain_all;
import std;
import interfaces;
import blog_handlers_middleware;
import core_workflow;
import memory_backends;
import :tags_browse_flow;
export namespace blog::handlers::tags::browse {
class BrowseHandler {
public:
 BrowseHandler(blog::BlogContext& ctx) noexcept {
  workflow::ExecuteStep::s_ctx=&ctx;
  m_workflow.use_backend(std::make_unique<core::workflow::LocalWorkflowDriver>());
  m_workflow.register_workflow(workflow::MainFlow{}.build());
 }
 void handle(interfaces::io::IRequest& req, interfaces::io::IResponse& res, std::function<void()> send) {
  // Ghost authAdminApi (admin) / authenticatePublic (content) + permissions:true.
  // One flat C++ router serves both contexts; infer the missing context from the
  // request path so anonymous content reads pass and admin reads require a token.
  if (req.find_header(blog::handlers::middleware::kGhostContextHeader).empty()) {
   const std::string path{req.get_path()};
   req.set_header(blog::handlers::middleware::kGhostContextHeader,
                  path.find("/content/") != std::string::npos ? "CONTENT_API" : "ADMIN_API");
  }
  bool allowed=false;
  blog::handlers::middleware::require_ghost_permission(
      req, res,
      [&allowed](interfaces::io::IRequest&, interfaces::io::IResponse&, std::function<void()>) noexcept { allowed=true; },
      send, "tags", "browse");
  if(!allowed) return;

  const auto ex=m_workflow.run("tags_browse", req);
  const auto* r=ex.result_of("tags_browse_execute");
  auto payload=r&&r->typed_output.has_value()
    ?std::any_cast<std::map<std::string,std::string>>(r->typed_output)
    :std::map<std::string,std::string>{{"status","200"},{"body","{}"}};
  const std::string code=payload.contains("status")?payload.at("status"):"200";
  const std::string body=payload.contains("body")?payload.at("body"):"{}";
  std::vector<std::byte> bytes; for(char c:body) bytes.push_back(static_cast<std::byte>(c));
  // cacheInvalidate:false -> Ghost emits no X-Cache-Invalidate header.
  auto status=interfaces::io::types::Status::OK;
  if(code=="400") status=interfaces::io::types::Status::BAD_REQUEST;
  else if(code=="422") status=interfaces::io::types::Status::UNPROCESSABLE_CONTENT;
  res.set_status(status); res.set_body(std::move(bytes)); send();
 }
private: core::workflow::Workflow m_workflow{};
};
}
