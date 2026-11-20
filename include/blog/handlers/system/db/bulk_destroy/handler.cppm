export module blog_handlers:db_bulk_destroy_handler;
import :domain_all;
import std;
import interfaces;
import core_workflow;
import memory_backends;
import :db_bulk_destroy_flow;
export namespace blog::handlers::db::bulk_destroy {
class BulkDestroyHandler {
public:
  BulkDestroyHandler(blog::BlogContext& ctx) noexcept {
   workflow::ExecuteStep::s_ctx=&ctx;
   m_workflow.use_backend(std::make_unique<core::workflow::LocalWorkflowDriver>());
   m_workflow.register_workflow(workflow::MainFlow{}.build());
  }
  void handle(interfaces::io::IRequest& req, interfaces::io::IResponse& res, std::function<void()> send) {
   // Ghost deleteAllContent: 204, no body, cacheInvalidate:true -> /* (db.js:137-142).
   const auto ex=m_workflow.run("db_bulk_destroy", req);
   const auto* r=ex.result_of("db_bulk_destroy_execute");
   const auto payload=r&&r->typed_output.has_value()
     ?std::any_cast<std::map<std::string,std::string>>(r->typed_output)
     :std::map<std::string,std::string>{{"status","204"},{"body",""}};
   const int status=payload.contains("status")?std::stoi(payload.at("status")):204;
   const auto body=payload.contains("body")?payload.at("body"):std::string{""};
   std::vector<std::byte> bytes; for(char c:body) bytes.push_back(static_cast<std::byte>(c));
   res.set_status(static_cast<interfaces::io::types::Status>(status));
   res.set_header("X-Cache-Invalidate", "/*");
   res.set_body(std::move(bytes)); send();
  }
private: core::workflow::Workflow m_workflow{};
};
}
