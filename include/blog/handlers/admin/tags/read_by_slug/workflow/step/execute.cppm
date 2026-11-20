export module blog_handlers:tags_read_by_slug_step;
import :domain_all;
import std;
import core_workflow;
import core_query;
import core_serde;
import interfaces;

export namespace blog::handlers::tags::read_by_slug::workflow {

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
 static constexpr std::string_view name{"tags_read_by_slug_execute"};
 [[nodiscard]] core::workflow::StepResponse<Output> invoke(const Input& in, core::workflow::WorkflowContext&) const {
  auto& ctx=*s_ctx;
  const std::string path{in.get_path()};
  const bool content=path.find("/content/")!=std::string::npos;

  constexpr std::array<std::string_view,4> kOptions{"include","filter","fields","debug"};
  const auto q=core::query::parse(path, kOptions);
  for(const auto& inc : q.include) {
   if(inc!="count.posts")
    return core::workflow::StepResponse<Output>{{{"status","422"},
      {"body",blog::api::ApiError{}.validation("Validation error","include invalid","include")}}};
  }
  if(core::query::has_restricted_field(q.raw_filter) || core::query::has_restricted_field(q.order))
   return core::workflow::StepResponse<Output>{{{"status","400"},
     {"body",blog::api::ApiError{}.json("Restricted field used in Ghost query","Bad Request")}}};

  const auto base=path.substr(0,path.find('?'));
  const auto slug=base.substr(base.rfind('/')+1);
  if(slug.empty()) {
   const std::string code=content?"400":"422";
   return core::workflow::StepResponse<Output>{{{"status",code},
     {"body",blog::api::ApiError{}.validation("A tag id or slug is required.","Bad Request","slug")}}};
  }

  // TagPublic.findOne({slug}) expressed through the shared NQL filter parser
  // (no manual "slug:"+slug concatenation; filter values are validated/escaped
  // by core::query::parse before the guard).
  constexpr std::array<std::string_view,1> kSlugOption{"filter"};
  const auto sq=core::query::parse("?filter[slug]="+slug, kSlugOption);
  const auto rows=ctx.store().json_for("tags", {});
  const auto cleaned=local_mapping::clean(rows, "tag");
  std::string matched;
  try { matched=core::query::apply(cleaned, sq); }
  catch(const std::runtime_error&) {
   return core::workflow::StepResponse<Output>{{{"status","400"},
     {"body",blog::api::ApiError{}.json("Restricted field used in Ghost query","Bad Request")}}};
  }
  const auto parsed=core_serde::JsonCodec{}.decode<serde::Value>(matched);
  const auto* array=parsed?std::get_if<serde::Value::Array>(&parsed->get()):nullptr;
  if(array==nullptr || array->empty())
   return core::workflow::StepResponse<Output>{{{"status","404"},
     {"body",blog::api::ApiError{}.json("Tag not found.","Not Found")}}};
  const std::string body=local_mapping::wrap("tags",
      local_mapping::clean(core_serde::JsonCodec{}.encode((*array)[0]), "tag"));
  return core::workflow::StepResponse<Output>{{{"status","200"},{"body",body}}};
 }
 void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}
 static blog::BlogContext* s_ctx;
};
blog::BlogContext* ExecuteStep::s_ctx{nullptr};
}
