export module blog_handlers:notifications_destroy_handler;
import :domain_all;
import std;
import interfaces;
import core_workflow;
import memory_backends;
import :notifications_destroy_flow;
export namespace blog::handlers::notifications::destroy {
class DestroyHandler {
public:
  DestroyHandler(blog::BlogContext& ctx) noexcept {
   workflow::ExecuteStep::s_ctx=&ctx;
   m_workflow.use_backend(std::make_unique<core::workflow::LocalWorkflowDriver>());
   m_workflow.register_workflow(workflow::MainFlow{}.build());
  }
  void handle(interfaces::io::IRequest& req, interfaces::io::IResponse& res, std::function<void()> send) {
   const auto ex=m_workflow.run("notifications_destroy", req);
   const auto* r=ex.result_of("notifications_destroy_execute");
   auto payload=r&&r->typed_output.has_value()
     ?std::any_cast<std::map<std::string,std::string>>(r->typed_output)
     :std::map<std::string,std::string>{{"status","404"},{"body",R"({"errors":[]})"}};
   const int status=payload.contains("status")?std::stoi(payload.at("status")):404;
   const auto body=payload.contains("body")?payload.at("body"):std::string{};
   std::vector<std::byte> bytes; for(char c:body) bytes.push_back(static_cast<std::byte>(c));
   res.set_status(static_cast<interfaces::io::types::Status>(status));
   res.set_header("X-Cache-Invalidate", "false");
   res.set_body(std::move(bytes)); send();
  }
private: core::workflow::Workflow m_workflow{};
};
}
