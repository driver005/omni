export module blog_handlers:mentions_receive_handler;
import :domain_all;
import std;
import interfaces;
import core_workflow;
import :mentions_receive_flow;
export namespace blog::handlers::mentions::receive {
class ReceiveHandler {
public:
  ReceiveHandler(blog::BlogContext& ctx) noexcept {
   workflow::ExecuteStep::s_ctx=&ctx;
   m_workflow.use_backend(std::make_unique<core::workflow::LocalWorkflowDriver>());
   m_workflow.register_workflow(workflow::MainFlow{}.build());
  }
  void handle(interfaces::io::IRequest& req, interfaces::io::IResponse& res, std::function<void()> send) {
   // permissions:false — no require_auth / ghost permission gate on this route.
   const auto ex=m_workflow.run("mentions_receive", req);
   const auto* r=ex.result_of("mentions_receive_execute");
   const auto payload=r&&r->typed_output.has_value()
     ? std::any_cast<std::map<std::string, std::string>>(r->typed_output)
     : std::map<std::string, std::string>{{"status", "202"}, {"body", ""}};
   const int status=payload.contains("status") ? std::stoi(payload.at("status")) : 202;
   const auto body=payload.contains("body") ? payload.at("body") : std::string{};
   std::vector<std::byte> bytes; for(char c:body) bytes.push_back(static_cast<std::byte>(c));
   res.set_status(static_cast<interfaces::io::types::Status>(status));
   res.set_header("X-Cache-Invalidate", "false");
   res.set_header(interfaces::io::types::Token::CACHE_CONTROL, "private");
   res.set_body(std::move(bytes)); send();
  }
private: core::workflow::Workflow m_workflow{};
};
}
