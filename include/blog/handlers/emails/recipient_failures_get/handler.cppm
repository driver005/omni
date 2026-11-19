export module blog_handlers:emails_recipient_failures_get_handler;
import std;
import interfaces;
import blog_context;
import blog_handlers_middleware;
import core_workflow;
import memory_backends;
import :emails_recipient_failures_get_model; 
import :emails_recipient_failures_get_flow;
export namespace blog::handlers::emails::recipient_failures_get {
class RecipientFailuresGetHandler {
public:
 RecipientFailuresGetHandler(blog::BlogContext& ctx, blog::api::emails::EmailsOperations& ops) noexcept {
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
  model::Request in; in.path=std::string{req.get_path()};
  for(const std::byte byte:req.get_body()) in.body+=static_cast<char>(byte);
  const auto ex=m_workflow.run("emails_recipient_failures_get", in);
  const auto* r=ex.result_of("emails_recipient_failures_get_execute");
  const auto payload=r&&r->typed_output.has_value()?std::any_cast<std::map<std::string, std::string>>(r->typed_output):std::map<std::string, std::string>{{"status","200"},{"body","{}"}};
  const int status=payload.contains("status")?std::stoi(payload.at("status")):200;
  const auto body=payload.contains("body")?payload.at("body"):std::string{"{}"};
  std::vector<std::byte> bytes; for(char c:body) bytes.push_back(static_cast<std::byte>(c));
  res.set_status(static_cast<interfaces::io::types::Status>(status)); res.set_body(std::move(bytes)); send();
 }
private: core::workflow::Workflow m_workflow{};
};
}
