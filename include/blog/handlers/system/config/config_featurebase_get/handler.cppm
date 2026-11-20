export module blog_handlers:config_config_featurebase_get_handler;
import :domain_all;
import std;
import interfaces;
import core_workflow;
import memory_backends;
import :config_config_featurebase_get_flow;
export namespace blog::handlers::config::config_featurebase_get {
class ConfigFeaturebaseGetHandler {
public:
  ConfigFeaturebaseGetHandler(blog::BlogContext& ctx) noexcept {
   workflow::ExecuteStep::s_ctx=&ctx;
   m_workflow.use_backend(std::make_unique<core::workflow::LocalWorkflowDriver>());
   m_workflow.register_workflow(workflow::MainFlow{}.build());
  }
  void handle(interfaces::io::IRequest& req, interfaces::io::IResponse& res, std::function<void()> send) {
   // Auth is on the router chain: require_auth (middleware/auth.cppm). The former
   // local token check emitted {"error":"Unauthorized"} instead of the Ghost
   // UnauthorizedError envelope; it is removed in favour of the shared middleware.
   const auto ex=m_workflow.run("config_config_featurebase_get", req);
   const auto* r=ex.result_of("config_config_featurebase_get_execute");
   auto body=r&&r->typed_output.has_value()?std::any_cast<std::string>(r->typed_output):std::string{"{}"};
   std::vector<std::byte> bytes; for(char c:body) bytes.push_back(static_cast<std::byte>(c));
   // Admin app cacheControl('private') only; no X-Cache-Invalidate stand-in.
   res.set_header(interfaces::io::types::Token::CACHE_CONTROL,
       "no-cache, private, no-store, must-revalidate, max-stale=0, post-check=0, pre-check=0");
   res.set_status(interfaces::io::types::Status::OK); res.set_body(std::move(bytes)); send();
  }
private: core::workflow::Workflow m_workflow{};
};
}
