export module blog_handlers:members_members_id_commenting_disable_post_handler;
import :domain_all;
import std;
import interfaces;
import blog_handlers_middleware;
import core_workflow;
import memory_backends;
import :members_operations;
import :members_members_id_commenting_disable_post_flow;
export namespace blog::handlers::members::members_id_commenting_disable_post {
class MembersIdCommentingDisablePostHandler {
public:
 MembersIdCommentingDisablePostHandler(blog::BlogContext& ctx, blog::api::members::MembersOperations& ops) noexcept {
  workflow::ExecuteStep::s_ctx=&ctx; workflow::ExecuteStep::s_ops=&ops;
  m_workflow.use_backend(std::make_unique<core::workflow::LocalWorkflowDriver>());
  m_workflow.register_workflow(workflow::MainFlow{}.build());
 }
 void handle(interfaces::io::IRequest& req, interfaces::io::IResponse& res, std::function<void()> send) {
  namespace support=blog::api::members::members_support;
  // Ghost mw.authAdminApi + permissions {method:'edit',docName:'member'}.
  if (!support::enforce(req, res, send, "member", "edit")) return;
  const auto ex=m_workflow.run("members_members_id_commenting_disable_post", req);
  const auto* r=ex.result_of("members_members_id_commenting_disable_post_execute");
  const auto payload=r&&r->typed_output.has_value()
   ?std::any_cast<std::map<std::string,std::string>>(r->typed_output)
   :std::map<std::string,std::string>{{"status","200"}};
  const int status=payload.contains("status")?std::stoi(payload.at("status")):200;
  const auto body=payload.contains("body")?payload.at("body"):std::string{"{}"};
  std::vector<std::byte> bytes; for(char c:body) bytes.push_back(static_cast<std::byte>(c));
  res.set_header("X-Cache-Invalidate", "/members/");
  res.set_header("Cache-Control", "private");
  res.set_status(static_cast<interfaces::io::types::Status>(status));
  res.set_body(std::move(bytes)); send();
 }
private: core::workflow::Workflow m_workflow{};
};
}
