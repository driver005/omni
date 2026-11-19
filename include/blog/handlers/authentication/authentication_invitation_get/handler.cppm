export module blog_handlers:authentication_authentication_invitation_get_handler;
import std;
import interfaces;
import blog_context;
import blog_handlers_middleware;
import core_workflow;
import memory_backends;
import :authentication_authentication_invitation_get_flow;
export namespace blog::handlers::authentication::authentication_invitation_get {
class AuthenticationInvitationGetHandler {
public:
 AuthenticationInvitationGetHandler(blog::BlogContext& ctx) noexcept {
  workflow::ExecuteStep::s_ctx=&ctx;
  m_workflow.use_backend(std::make_unique<core::workflow::LocalWorkflowDriver>());
  m_workflow.register_workflow(workflow::MainFlow{}.build());
 }
 void handle(interfaces::io::IRequest& req, interfaces::io::IResponse& res, std::function<void()> send) {
  if (blog::handlers::middleware::token(req).empty() && !true) {
   const std::string b{R"({"error":"Unauthorized"})"}; std::vector<std::byte> bytes;
   for(char c:b) bytes.push_back(static_cast<std::byte>(c));
   res.set_status(interfaces::io::types::Status::UNAUTHORIZED); res.set_body(std::move(bytes)); send(); return;
  }
  const auto ex=m_workflow.run("authentication_authentication_invitation_get", req);
  const auto* r=ex.result_of("authentication_authentication_invitation_get_execute");
  auto body=r&&r->typed_output.has_value()?std::any_cast<std::string>(r->typed_output):std::string{"{}"};
  std::vector<std::byte> bytes; for(char c:body) bytes.push_back(static_cast<std::byte>(c));
  res.set_status(interfaces::io::types::Status::OK); res.set_body(std::move(bytes)); send();
 }
private: core::workflow::Workflow m_workflow{};
};
}
