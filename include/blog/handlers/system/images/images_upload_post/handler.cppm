export module blog_handlers:images_images_upload_post_handler;
import :domain_all;
import std;
import interfaces;
import core_workflow;
import memory_backends;
import :images_images_upload_post_flow;
export namespace blog::handlers::images::images_upload_post {
class ImagesUploadPostHandler {
public:
  ImagesUploadPostHandler(blog::BlogContext& ctx) noexcept {
   workflow::ExecuteStep::s_ctx=&ctx;
   m_workflow.use_backend(std::make_unique<core::workflow::LocalWorkflowDriver>());
   m_workflow.register_workflow(workflow::MainFlow{}.build());
  }
  void handle(interfaces::io::IRequest& req, interfaces::io::IResponse& res, std::function<void()> send) {
   // Auth is enforced by the router chain (bearer_middleware + require_auth);
   // Ghost controller declares permissions:false, so no docName gate here.
   const auto ex=m_workflow.run("images_images_upload_post", req);
   const auto* r=ex.result_of("images_images_upload_post_execute");
   auto payload=r&&r->typed_output.has_value()
     ? std::any_cast<std::map<std::string,std::string>>(r->typed_output)
     : std::map<std::string,std::string>{{"status","422"},
       {"body",blog::api::ApiError{}.validation("Validation error","Upload failed","file")}};
   const int status=payload.contains("status")?std::stoi(payload.at("status")):422;
   const auto body=payload.contains("body")?payload.at("body"):std::string{"{}"};

   // Ghost admin app forces `Cache-Control: private` (admin/app.js:24). The controller
   // sets headers.cacheInvalidate:false, which SUPPRESSES X-Cache-Invalidate entirely
   // (images.js:14-16, headers.ts:135-144) — no stand-in header is emitted.
   res.set_header("Cache-Control","private");
   res.set_status(static_cast<interfaces::io::types::Status>(status));
   std::vector<std::byte> bytes; for(char c:body) bytes.push_back(static_cast<std::byte>(c));
   res.set_body(std::move(bytes)); send();
  }
 private: core::workflow::Workflow m_workflow{};
};
}
