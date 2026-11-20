export module blog_handlers:session_create_handler;
import :domain_all;
import std;
import interfaces;
import blog_handlers_middleware;
import core_workflow;
import memory_backends;
import :session_create_flow;
export namespace blog::handlers::session::create {
class CreateHandler {
public:
  CreateHandler(blog::BlogContext& ctx) noexcept {
   workflow::ExecuteStep::s_ctx=&ctx;
   m_workflow.use_backend(std::make_unique<core::workflow::LocalWorkflowDriver>());
   m_workflow.register_workflow(workflow::MainFlow{}.build());
  }
  void handle(interfaces::io::IRequest& req, interfaces::io::IResponse& res, std::function<void()> send) {
   // Ghost has no admin auth on this route; rate limiting has no allowed primitive.
   const auto ex=m_workflow.run("session_create", req);
   const auto* r=ex.result_of("session_create_execute");
   const auto payload=r&&r->typed_output.has_value()
    ?std::any_cast<std::map<std::string,std::string>>(r->typed_output)
    :std::map<std::string,std::string>{{"status","401"},{"body",blog::api::ApiError{}.json("Access Denied.","Access Denied.")}};
   const int code=payload.contains("status")?std::stoi(payload.at("status")):401;
   const auto body=payload.contains("body")?payload.at("body"):std::string{"{}"};
   if (const auto it=payload.find("set_cookie"); it!=payload.end() && !it->second.empty())
    res.add_set_cookie(it->second);
   std::vector<std::byte> bytes; for(char c:body) bytes.push_back(static_cast<std::byte>(c));
   res.set_status(static_cast<interfaces::io::types::Status>(code)); res.set_body(std::move(bytes)); send();
  }
private: core::workflow::Workflow m_workflow{};
};
}
