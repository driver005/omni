export module blog_handlers:tags_edit_handler;
import :domain_all;
import std;
import interfaces;
import blog_handlers_middleware;
import core_workflow;
import memory_backends;
import :tags_edit_flow;
export namespace blog::handlers::tags::edit {
class EditHandler {
public:
 EditHandler(blog::BlogContext& ctx) noexcept {
  workflow::ExecuteStep::s_ctx=&ctx;
  m_workflow.use_backend(std::make_unique<core::workflow::LocalWorkflowDriver>());
  m_workflow.register_workflow(workflow::MainFlow{}.build());
 }
 void handle(interfaces::io::IRequest& req, interfaces::io::IResponse& res, std::function<void()> send) {
  if (req.find_header(blog::handlers::middleware::kGhostContextHeader).empty())
   req.set_header(blog::handlers::middleware::kGhostContextHeader, "ADMIN_API");
  bool allowed=false;
  blog::handlers::middleware::require_ghost_permission(
      req, res,
      [&allowed](interfaces::io::IRequest&, interfaces::io::IResponse&, std::function<void()>) noexcept { allowed=true; },
      send, "tags", "edit");
  if(!allowed) return;

  const auto ex=m_workflow.run("tags_edit", req);
  const auto* r=ex.result_of("tags_edit_execute");
  auto payload=r&&r->typed_output.has_value()
    ?std::any_cast<std::map<std::string,std::string>>(r->typed_output)
    :std::map<std::string,std::string>{{"status","200"},{"body","{}"}};
  const std::string code=payload.contains("status")?payload.at("status"):"200";
  const std::string body=payload.contains("body")?payload.at("body"):"{}";
  const bool changed=payload.contains("changed") && payload.at("changed")=="true";
  std::vector<std::byte> bytes; for(char c:body) bytes.push_back(static_cast<std::byte>(c));
  auto status=interfaces::io::types::Status::OK;
  if(code=="201") status=interfaces::io::types::Status::OK;  // store wrap() -> 201; Ghost edit -> 200
  else if(code=="400") status=interfaces::io::types::Status::BAD_REQUEST;
  else if(code=="404") status=interfaces::io::types::Status::NOT_FOUND;
  else if(code=="422") status=interfaces::io::types::Status::UNPROCESSABLE_CONTENT;
  // model.wasChanged() -> X-Cache-Invalidate: /* ONLY on an actual change.
  if(changed) res.set_header("X-Cache-Invalidate", "/*");
  res.set_status(status); res.set_body(std::move(bytes)); send();
 }
private: core::workflow::Workflow m_workflow{};
};
}
