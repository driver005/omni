export module blog_handlers:automations_automations_id_email_test_post_handler;
import :domain_all;
import :automations_dto_automation_email_preview;
import :automations_dto_automation_email_preview_input;
import :automations_dto_automation_email_preview_response;
import :automations_dto_automation_email_test_input;
import :automations_dto_links;
import :automations_dto_stats_empty_response;
import std;
import interfaces;
import blog_handlers_middleware;
import core_workflow;

import memory_backends;
import :automations_automations_id_email_test_post_flow;
export namespace blog::handlers::automations::automations_id_email_test_post {
class AutomationsIdEmailTestPostHandler {
public:
 AutomationsIdEmailTestPostHandler(blog::BlogContext& ctx, blog::api::automations::AutomationsOperations& ops) noexcept {
  workflow::ExecuteStep::s_ctx=&ctx; workflow::ExecuteStep::s_ops=&ops;
  m_workflow.use_backend(std::make_unique<core::workflow::LocalWorkflowDriver>());
  m_workflow.register_workflow(workflow::MainFlow{}.build());
 }
 void handle(interfaces::io::IRequest& req, interfaces::io::IResponse& res, std::function<void()> send) {
  if (blog::handlers::middleware::token(req).empty()) {
   const std::string b{R"({"error":"Unauthorized"})"}; std::vector<std::byte> bytes;
   for(char c:b) bytes.push_back(static_cast<std::byte>(c));
   res.set_status(interfaces::io::types::Status::UNAUTHORIZED); res.set_body(std::move(bytes)); send(); return;
  }
  const auto ex=m_workflow.run("automations_automations_id_email_test_post", req);
  const auto* r=ex.result_of("automations_automations_id_email_test_post_execute");
  const auto payload=r&&r->typed_output.has_value()?std::any_cast<std::map<std::string, std::string>>(r->typed_output):std::map<std::string, std::string>{{"status","200"},{"body","{}"}};
  const int status=payload.contains("status")?std::stoi(payload.at("status")):200;
  const auto body=payload.contains("body")?payload.at("body"):std::string{"{}"};
  std::vector<std::byte> bytes; for(char c:body) bytes.push_back(static_cast<std::byte>(c));
  res.set_status(static_cast<interfaces::io::types::Status>(status)); res.set_body(std::move(bytes)); send();
 }
private: core::workflow::Workflow m_workflow{};
};
}
