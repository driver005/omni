export module blog_handlers:invites_browse_handler;
import std;
import interfaces;
import blog_context;
import blog_handlers_middleware;
import core_workflow;
import memory_backends;
import :invites_browse_model; 
import :invites_browse_flow;
export namespace blog::handlers::invites::browse {
class BrowseHandler {
public:
 BrowseHandler(blog::BlogContext& ctx, blog::api::SpecialEndpoints& sp) noexcept {
  workflow::ExecuteStep::s_ctx=&ctx; workflow::ExecuteStep::s_special=&sp;
  m_workflow.use_backend(std::make_unique<core::workflow::LocalWorkflowDriver>());
  m_workflow.register_workflow(workflow::MainFlow{}.build());
 }
 void handle(interfaces::io::IRequest& req, interfaces::io::IResponse& res, std::function<void()> send) {
  if (blog::handlers::middleware::token(req).empty()) {
   const std::string b{R"({"error":"Unauthorized"})"}; std::vector<std::byte> bytes;
   for(char c:b) bytes.push_back(static_cast<std::byte>(c));
   res.set_status(interfaces::io::types::Status::UNAUTHORIZED); res.set_body(std::move(bytes)); send(); return;
  }
  model::Request in; in.path=std::string{req.get_path()};
  for(const std::byte byte:req.get_body()) in.body+=static_cast<char>(byte);
  const auto ex=m_workflow.run("invites_browse", in);
  const auto* r=ex.result_of("invites_browse_execute");
  auto body=r&&r->typed_output.has_value()?std::any_cast<std::string>(r->typed_output):std::string{"{}"};
  std::vector<std::byte> bytes; for(char c:body) bytes.push_back(static_cast<std::byte>(c));
  res.set_status(interfaces::io::types::Status::OK); res.set_body(std::move(bytes)); send();
 }
private: core::workflow::Workflow m_workflow{};
};
}
