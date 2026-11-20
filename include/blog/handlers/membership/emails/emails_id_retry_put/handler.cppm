export module blog_handlers:emails_emails_id_retry_put_handler;
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
import :emails_emails_id_retry_put_flow;
export namespace blog::handlers::emails::emails_id_retry_put {
class EmailsIdRetryPutHandler {
public:
  EmailsIdRetryPutHandler(blog::BlogContext& ctx) noexcept {
   workflow::ExecuteStep::s_ctx=&ctx;
   m_workflow.use_backend(std::make_unique<core::workflow::LocalWorkflowDriver>());
   m_workflow.register_workflow(workflow::MainFlow{}.build());
  }
  void handle(interfaces::io::IRequest& req, interfaces::io::IResponse& res, std::function<void()> send) {
   // Ghost mw.authAdminApi + controller permissions (docName emails).
   // Router-level require_auth stays as the 401 gate; GhostAuthorizer
   // supplies the 403 NoPermissionError gate for the declared method.
   if (!blog::api::emails::permitted(req, res, send, "emails", "retry")) return;
   // Auth moved to the router chain: content_type_middleware + require_auth.
   const auto ex=m_workflow.run("emails_emails_id_retry_put", req);
   const auto* r=ex.result_of("emails_emails_id_retry_put_execute");
   const auto payload=r&&r->typed_output.has_value()?std::any_cast<std::map<std::string,std::string>>(r->typed_output):std::map<std::string,std::string>{{"status","200"},{"body","{}"}};
   const int status=payload.contains("status")?std::stoi(payload.at("status")):200;
   const auto body=payload.contains("body")?payload.at("body"):std::string{"{}"};
   std::vector<std::byte> bytes; for(char c:body) bytes.push_back(static_cast<std::byte>(c));
   res.set_status(static_cast<interfaces::io::types::Status>(status)); res.set_body(std::move(bytes)); send();
  }
private: core::workflow::Workflow m_workflow{};
};
}
