export module blog_handlers:authentication_authentication_setup_put_handler;
import :domain_all;
import std;
import interfaces;
import blog_handlers_middleware;
import core_workflow;
import memory_backends;
import :authentication_authentication_setup_put_flow;
export namespace blog::handlers::authentication::authentication_setup_put {
class AuthenticationSetupPutHandler {
public:
  AuthenticationSetupPutHandler(blog::BlogContext& ctx) noexcept {
   workflow::ExecuteStep::s_ctx=&ctx;
   m_workflow.use_backend(std::make_unique<core::workflow::LocalWorkflowDriver>());
   m_workflow.register_workflow(workflow::MainFlow{}.build());
  }
  void handle(interfaces::io::IRequest& req, interfaces::io::IResponse& res, std::function<void()> send) {
   // mw.authAdminApi is installed per-verb on PUT /authentication/setup in routes.cppm.
   const auto ex=m_workflow.run("authentication_authentication_setup_put", req);
   const auto* r=ex.result_of("authentication_authentication_setup_put_execute");
   const auto payload=r&&r->typed_output.has_value()
       ? std::any_cast<std::map<std::string, std::string>>(r->typed_output)
       : std::map<std::string, std::string>{{"status", "200"}, {"body", "{}"}};
   const int status=payload.contains("status") ? std::stoi(payload.at("status")) : 200;
   const auto body=payload.contains("body") ? payload.at("body") : std::string{"{}"};
   std::vector<std::byte> bytes; for(char c:body) bytes.push_back(static_cast<std::byte>(c));
   // Ghost headers.cacheInvalidate:true emits X-Cache-Invalidate: /*
   res.set_header("X-Cache-Invalidate", "/*");
   res.set_status(static_cast<interfaces::io::types::Status>(status)); res.set_body(std::move(bytes)); send();
  }
 private: core::workflow::Workflow m_workflow{};
};
}
