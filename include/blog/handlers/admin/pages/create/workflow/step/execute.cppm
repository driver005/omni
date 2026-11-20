export module blog_handlers:pages_create_step;
import :domain_all;
import :pages_dto_links;
import std;
import core_workflow;
import core_query;
import core_auth;
import core_serde;
import interfaces;

export namespace blog::handlers::pages::create::workflow {

namespace local_mapping {
[[nodiscard]] inline std::string wrap(std::string_view resource, std::string_view rows,
                                      std::string_view meta = {})
{
    return "{\"" + std::string{resource} + "\":" + std::string{rows} +
           (meta.empty() ? std::string{} : ",\"meta\":" + std::string{meta}) + "}";
}
[[nodiscard]] inline std::string clean(std::string_view json, std::string_view) { return std::string{json}; }
[[nodiscard]] inline std::string pagination(std::size_t total, std::size_t limit, std::size_t page,
                                            std::size_t pages, std::string_view prev,
                                            std::string_view next)
{
    return std::format(R"("total":{},"limit":{},"page":{},"pages":{},"prev":{},"next":{})",
                       total, limit, page, pages,
                       prev.empty() ? std::string{"null"} : std::string{prev},
                       next.empty() ? std::string{"null"} : std::string{next});
}
} // namespace local_mapping

class ExecuteStep {
public:
 using Input=interfaces::io::IRequest&; using Output=std::map<std::string,std::string>;
 static constexpr std::string_view name{"pages_create_execute"};
 [[nodiscard]] core::workflow::StepResponse<Output> invoke(const Input& in, core::workflow::WorkflowContext&) const {
  using namespace blog::handlers::pages::support;
  auto& ctx=*s_ctx; auto& mutations=ctx.store();

  const auto principal=principal_for(in);
  core::auth::ghost::GhostAuthorizer authorizer;
  const auto authorized=authorizer.authorize(principal,true,"posts","add");
  if(!authorized.allowed)
   return core::workflow::StepResponse<Output>{{{"status",std::to_string(authorized.status)},{"body",authorized.error_json}}};

  // Ghost input validation: source in {html}, include/formats allow-lists.
  const auto q=core::query::parse(in.get_path(),kAddOptions);
  const auto source=q.options.contains("source")?q.options.at("source"):"";
  if((!source.empty() && source!="html") ||
     !tokens_ok(q.options.contains("include")?q.options.at("include"):"",kAdminIncludes) ||
     !tokens_ok(q.options.contains("formats")?q.options.at("formats"):"",kFormats))
   return core::workflow::StepResponse<Output>{{{"status","400"},
     {"body",blog::api::ApiError{}.validation("Invalid source/include/formats","Validation Error","source")}}};

  std::string payload=in.body_as_string().empty()?"{}":in.body_as_string();
  try {
   if(auto value=core_serde::JsonCodec{}.decode<serde::Value>(payload)) {
    const auto pages=field_raw(*value,"pages");
    if(!pages.empty() && pages!="null") {
     const auto first=first_row(pages);
     if(!first.empty() && first!="null") payload=first;
    }
   }
  } catch (...) {}

  // unsafeAttrs: only staff/admin roles may set status, authors or visibility.
  for(const char* attr:{"status","authors","visibility"}) {
   if(payload.find(std::string{"\""}+attr+"\"")!=std::string::npos) {
    const auto unsafe=authorizer.unsafe_attrs(principal,attr,"");
    if(!unsafe.allowed)
     return core::workflow::StepResponse<Output>{{{"status",std::to_string(unsafe.status)},{"body",unsafe.error_json}}};
   }
  }

  // Ghost forces type:page and normalizes html->lexical (placeholder lexical is
  // already emitted by model::Page); source=html has no extra wire representation.
  payload=set_string_field(std::move(payload),"type","page");

  const auto result=mutations.create("pages",payload);
  if(!result.ok())
   return core::workflow::StepResponse<Output>{{{"status",std::to_string(result.status())},{"body",result.body()}}};

  // pages.all serializer: clean + {pages:[page]}.
  std::string rows;
  try {
   if(auto value=core_serde::JsonCodec{}.decode<serde::Value>(result.body())) {
    if(auto object=value->to_object()) {
     if(const auto pages=object->get("pages")) rows=core_serde::JsonCodec{}.encode(*pages);
    }
   }
  } catch (...) {}
  const auto body=local_mapping::wrap("pages",local_mapping::clean(rows,"page"));
  const auto status=row_field(first_row(rows),"status");
  const auto id=row_field(first_row(rows),"id");

  std::map<std::string,std::string> out{{"status",std::to_string(result.status())},{"body",body}};
  if(status=="published") out["x-cache-invalidate"]="/*";
  out["location"]="/pages/"+id+"/";
  return core::workflow::StepResponse<Output>{std::move(out)};
 }
 void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}
 static blog::BlogContext* s_ctx;
};
blog::BlogContext* ExecuteStep::s_ctx{nullptr};
}
