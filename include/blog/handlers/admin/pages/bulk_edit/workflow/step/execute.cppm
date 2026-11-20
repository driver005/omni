export module blog_handlers:pages_bulk_edit_step;
import :domain_all;
import :pages_dto_links;
import std;
import core_workflow;
import core_query;
import core_auth;
import core_serde;
import interfaces;

export namespace blog::handlers::pages::bulk_edit::workflow {

class ExecuteStep {
public:
 using Input=interfaces::io::IRequest&; using Output=std::map<std::string,std::string>;
 static constexpr std::string_view name{"pages_bulk_edit_execute"};
 [[nodiscard]] core::workflow::StepResponse<Output> invoke(const Input& in, core::workflow::WorkflowContext&) const {
  using namespace blog::handlers::pages::support;
  auto& ctx=*s_ctx; auto& mutations=ctx.store();

  const auto principal=principal_for(in);
  core::auth::ghost::GhostAuthorizer authorizer;
  const auto authorized=authorizer.authorize(principal,true,"posts","edit");
  if(!authorized.allowed)
   return core::workflow::StepResponse<Output>{{{"status",std::to_string(authorized.status)},{"body",authorized.error_json}}};

  const auto q=core::query::parse(in.get_path(),std::array<std::string_view,1>{"filter"});
  const auto filter=q.options.contains("filter")?q.options.at("filter"):"";
  if(filter.empty())
   return core::workflow::StepResponse<Output>{{{"status","400"},
     {"body",blog::api::ApiError{}.validation("filter is required","Validation Error","filter")}}};
  if(core::query::has_restricted_field(filter))
   return core::workflow::StepResponse<Output>{{{"status","400"},
     {"body",blog::api::ApiError{}.validation("Restricted field used in bulk filter","Validation Error","filter")}}};

  std::string action, meta{"{}"};
  const std::string raw=in.body_as_string().empty()?"{}":in.body_as_string();
  try {
   if(auto value=core_serde::JsonCodec{}.decode<serde::Value>(raw)) {
    action=field_string(*value,"action");
    const auto m=field_raw(*value,"meta");
    if(!m.empty() && m!="null") meta=m;
   }
  } catch (...) {}
  if(action.empty())
   return core::workflow::StepResponse<Output>{{{"status","400"},
     {"body",blog::api::ApiError{}.validation("action is required","Validation Error","action")}}};

  // Ghost postsService.bulkEdit dispatches exactly six actions.
  static constexpr std::array<std::string_view,6> kActions{
    "unpublish","unschedule","feature","unfeature","access","addTag"};
  if(!in_list(action,kActions))
   return core::workflow::StepResponse<Output>{{{"status","400"},
     {"body",blog::api::ApiError{}.json("Unsupported bulk action","IncorrectUsageError")}}};

  const auto visibility=row_field(meta,"visibility");
  if(action=="access" && !in_list(visibility,std::array<std::string_view,4>{"public","members","paid","tiers"}))
   return core::workflow::StepResponse<Output>{{{"status","400"},
     {"body",blog::api::ApiError{}.validation("Invalid visibility","Validation Error","visibility")}}};

  auto scoped=q;
  scoped.limit=std::numeric_limits<std::size_t>::max();
  std::string matched;
  try { matched=core::query::apply(ctx.store().json_for("pages",{}),scoped,15); } catch (...) {}
  const auto ids=ids_from_rows(matched);

  std::string patch;
  if(action=="unpublish" || action=="unschedule") patch=R"({"status":"draft"})";
  else if(action=="feature") patch=R"({"featured":true})";
  else if(action=="unfeature") patch=R"({"featured":false})";
  else if(action=="access") patch=R"({"visibility":")"+visibility+R"("})";
  else patch=R"({"tags":[]})"; // addTag; Page wire has no tags column to merge.

  blog::handlers::outbox::global_outbox().begin_transaction();
  std::size_t successful=0, unsuccessful=0;
  for(const auto& id:ids) {
   const auto result=mutations.edit("pages",id,patch);
   if(result.ok()) ++successful; else ++unsuccessful;
  }
  std::string event_type;
  if(action=="unpublish") event_type="PostsBulkUnpublishedEvent";
  else if(action=="unschedule") event_type="PostsBulkUnscheduledEvent";
  else if(action=="feature") event_type="PostsBulkFeaturedEvent";
  else if(action=="unfeature") event_type="PostsBulkUnfeaturedEvent";
  else if(action=="access") event_type="PostsBulkAccessEvent";
  else event_type="PostsBulkAddTagsEvent";
  emit_event(event_type,"{\"ids\":"+json_ids(ids)+",\"action\":\""+action+"\"}","");
  blog::handlers::outbox::global_outbox().commit();

  // pages.bulkEdit serializer.
  const auto body=std::string{"{\"bulk\":{\"action\":\""}+action
    +"\",\"meta\":{\"stats\":{\"successful\":"+std::to_string(successful)
    +",\"unsuccessful\":"+std::to_string(unsuccessful)
    +"},\"errors\":[],\"unsuccessfulData\":[]}}}";
  return core::workflow::StepResponse<Output>{{{"status","200"},{"body",body},
    {"x-cache-invalidate","/*"}}};
 }
 void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}
 static blog::BlogContext* s_ctx;
};
blog::BlogContext* ExecuteStep::s_ctx{nullptr};
}
