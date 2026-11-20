export module blog_handlers:offers_read_by_id_handler;
import :domain_all;
import :offers_browse_step;
import std;
import interfaces;
import blog_handlers_middleware;
import core_workflow;
import memory_backends;
import :offers_read_by_id_flow;
export namespace blog::handlers::offers::read_by_id {
class ReadByIdHandler {
public:
  ReadByIdHandler(blog::BlogContext& ctx) noexcept {
   workflow::ExecuteStep::s_ctx=&ctx;
   m_workflow.use_backend(std::make_unique<core::workflow::LocalWorkflowDriver>());
   m_workflow.register_workflow(workflow::MainFlow{}.build());
  }
  void handle(interfaces::io::IRequest& req, interfaces::io::IResponse& res, std::function<void()> send) {
   // Ghost mw.authAdminApi + permissions:true (admin/routes.js:166, offers.js:32).
   if (!support::authorize(req, res, send, "read")) return;

   const auto ex=m_workflow.run("offers_read_by_id", req);
   const auto* r=ex.result_of("offers_read_by_id_execute");
   const auto payload=r&&r->typed_output.has_value()
     ?std::any_cast<std::map<std::string,std::string>>(r->typed_output)
     :std::map<std::string,std::string>{{"status","200"},{"body","{}"}};
   const auto code=payload.contains("status")?payload.at("status"):std::string{"200"};
   const auto body=payload.contains("body")?payload.at("body"):std::string{"{}"};
   std::vector<std::byte> bytes; for(char c:body) bytes.push_back(static_cast<std::byte>(c));
   auto status=interfaces::io::types::Status::OK;
   if (code=="401") status=interfaces::io::types::Status::UNAUTHORIZED;
   else if (code=="403") status=interfaces::io::types::Status::FORBIDDEN;
   else if (code=="404") status=interfaces::io::types::Status::NOT_FOUND;
   else if (code=="422") status=interfaces::io::types::Status::UNPROCESSABLE_CONTENT;
   // Ghost headers.cacheInvalidate:false -> no X-Cache-Invalidate header.
   res.set_status(status); res.set_body(std::move(bytes)); send();
  }
private: core::workflow::Workflow m_workflow{};
};
}
