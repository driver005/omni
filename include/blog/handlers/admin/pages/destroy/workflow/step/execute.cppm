export module blog_handlers:pages_destroy_step;
import :domain_all;
import :pages_dto_links;
import std;
import core_workflow;
import core_query;
import core_auth;
import interfaces;

export namespace blog::handlers::pages::destroy::workflow {

class ExecuteStep {
public:
 using Input=interfaces::io::IRequest&; using Output=std::map<std::string,std::string>;
 static constexpr std::string_view name{"pages_destroy_execute"};
 [[nodiscard]] core::workflow::StepResponse<Output> invoke(const Input& in, core::workflow::WorkflowContext&) const {
  using namespace blog::handlers::pages::support;
  auto& ctx=*s_ctx; auto& mutations=ctx.store();

  const auto principal=principal_for(in);
  core::auth::ghost::GhostAuthorizer authorizer;
  const auto authorized=authorizer.authorize(principal,true,"posts","destroy");
  if(!authorized.allowed)
   return core::workflow::StepResponse<Output>{{{"status",std::to_string(authorized.status)},{"body",authorized.error_json}}};
  for(const char* attr:{"status","authors","visibility"}) {
   const auto unsafe=authorizer.unsafe_attrs(principal,attr,"");
   if(!unsafe.allowed)
    return core::workflow::StepResponse<Output>{{{"status",std::to_string(unsafe.status)},{"body",unsafe.error_json}}};
  }

  const std::string raw{in.get_path()};
  auto path=std::string_view{raw}.substr(0,raw.find('?'));
  while(path.size()>1 && path.back()=='/') path.remove_suffix(1);
  const auto slash=path.find_last_of('/');
  const std::string id{slash==std::string_view::npos?std::string{path}:std::string{path.substr(slash+1)}};
  if(id.empty())
   return core::workflow::StepResponse<Output>{{{"status","400"},
     {"body",blog::api::ApiError{}.validation("An id is required.","Validation Error","id")}}};

  // Ghost: findOne({id, type:'page', status:'all'}) then destroy(require:true).
  const auto page=ctx.store().find_by_resource("pages",id);
  if(!page)
   return core::workflow::StepResponse<Output>{{{"status","404"},
     {"body",blog::api::ApiError{}.json("Page not found.","Page not found")}}};
  const bool draft=row_field(*page,"status")=="draft";

  blog::handlers::outbox::global_outbox().begin_transaction();
  const auto result=mutations.destroy("pages",id);
  if(!result.ok()) {
   blog::handlers::outbox::global_outbox().rollback();
   return core::workflow::StepResponse<Output>{{{"status",std::to_string(result.status())},{"body",result.body()}}};
  }
  emit_event("post.destroyed",R"({"resource":"page"})",id);
  blog::handlers::outbox::global_outbox().commit();

  // statusCode:204, body empty; framework auto "/*", overridden with "" when draft.
  return core::workflow::StepResponse<Output>{{{"status","204"},{"body",""},
    {"x-cache-invalidate",draft?"":"/*"}}};
 }
 void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}
 static blog::BlogContext* s_ctx;
};
blog::BlogContext* ExecuteStep::s_ctx{nullptr};
}
