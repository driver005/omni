export module blog_handlers:member_metafields_members_metafields_browse_handler;
import :domain_all;
import std;
import interfaces;
import blog_handlers_middleware;
import core_workflow;
import memory_backends;
import :member_metafields_members_metafields_browse_flow;
export namespace blog::handlers::member_metafields_members::metafields_browse {
class MemberMetafieldsMembersMetafieldsBrowseHandler {
public:
  MemberMetafieldsMembersMetafieldsBrowseHandler(blog::BlogContext& ctx) noexcept {
   workflow::ExecuteStep::s_ctx=&ctx;
   m_workflow.use_backend(std::make_unique<core::workflow::LocalWorkflowDriver>());
   m_workflow.register_workflow(workflow::MainFlow{}.build());
  }
  void handle(interfaces::io::IRequest& req, interfaces::io::IResponse& res, std::function<void()> send) {
   const auto ex=m_workflow.run("member_metafields_members_metafields_browse", req);
   const auto* r=ex.result_of("member_metafields_members_metafields_browse_execute");
   const auto payload=r&&r->typed_output.has_value()
     ?std::any_cast<std::map<std::string,std::string>>(r->typed_output)
     :std::map<std::string,std::string>{{"status","200"},{"body","{}"}};
   const int status=payload.contains("status")?std::stoi(payload.at("status")):200;
   const auto body=payload.contains("body")?payload.at("body"):std::string{"{}"};
   std::vector<std::byte> bytes; for(char c:body) bytes.push_back(static_cast<std::byte>(c));
   res.set_status(static_cast<interfaces::io::types::Status>(status));
   res.set_body(std::move(bytes));

   // Ghost: headers.cacheInvalidate:false (member-metafields-members.ts:31) emits
   // NO X-Cache-Invalidate header (api-framework headers.ts:135-144). The members
   // app installs cacheControl('private') (web/members/app.js:27), which is the full
   // private/no-store directive string (web/shared/middleware/cache-control.ts:16-19).
   res.set_header(
       interfaces::io::types::Token::CACHE_CONTROL,
       "no-cache, private, no-store, must-revalidate, max-stale=0, post-check=0, pre-check=0");
   send();
  }
 private: core::workflow::Workflow m_workflow{};
};
}
