export module blog_handlers:tags_read_by_id_step;
import :domain_all;
import std;
import core_workflow;
import core_query;
import interfaces;

export namespace blog::handlers::tags::read_by_id::workflow {

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
 static constexpr std::string_view name{"tags_read_by_id_execute"};
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
  // restrictAdminApiQueryOptions / mongoTransformer restricted-field guard.
  if(core::query::has_restricted_field(q.raw_filter) || core::query::has_restricted_field(q.order))
   return core::workflow::StepResponse<Output>{{{"status","400"},
     {"body",blog::api::ApiError{}.json("Restricted field used in Ghost query","Bad Request")}}};
  for(const auto& [field,value] : q.filters) {
   if(core::query::has_restricted_field(field) || core::query::has_restricted_field(value))
    return core::workflow::StepResponse<Output>{{{"status","400"},
      {"body",blog::api::ApiError{}.json("Restricted field used in Ghost query","Bad Request")}}};
  }

  const auto base=path.substr(0,path.find('?'));
  const auto id=base.substr(base.rfind('/')+1);
  if(id.empty()) {
   const std::string code=content?"400":"422";
   return core::workflow::StepResponse<Output>{{{"status",code},
     {"body",blog::api::ApiError{}.validation("A tag id or slug is required.","Bad Request","id")}}};
  }

  const auto one=ctx.store().find_by_resource("tags", id);
  if(!one)
   return core::workflow::StepResponse<Output>{{{"status","404"},
     {"body",blog::api::ApiError{}.json("Tag not found.","Not Found")}}};
  const std::string body=local_mapping::wrap("tags", local_mapping::clean(*one, "tag"));
  return core::workflow::StepResponse<Output>{{{"status","200"},{"body",body}}};
 }
 void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}
 static blog::BlogContext* s_ctx;
};
blog::BlogContext* ExecuteStep::s_ctx{nullptr};
}
