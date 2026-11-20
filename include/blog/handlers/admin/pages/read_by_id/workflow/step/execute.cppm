export module blog_handlers:pages_read_by_id_step;
import :domain_all;
import :pages_dto_links;
import std;
import core_workflow;
import core_query;
import core_auth;
import core_serde;
import interfaces;

export namespace blog::handlers::pages::read_by_id::workflow {

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
 static constexpr std::string_view name{"pages_read_by_id_execute"};
 [[nodiscard]] core::workflow::StepResponse<Output> invoke(const Input& in, core::workflow::WorkflowContext&) const {
  using namespace blog::handlers::pages::support;
  auto& ctx=*s_ctx;

  const auto principal=principal_for(in);
  core::auth::ghost::GhostAuthorizer authorizer;
  const auto authorized=authorizer.authorize(principal,true,"pages","read");
  if(!authorized.allowed)
   return core::workflow::StepResponse<Output>{{{"status",std::to_string(authorized.status)},{"body",authorized.error_json}}};

  // Path identifier (id, slug or uuid) is the last non-empty path segment.
  const std::string raw{in.get_path()};
  auto path=std::string_view{raw}.substr(0,raw.find('?'));
  while(path.size()>1 && path.back()=='/') path.remove_suffix(1);
  const auto slash=path.find_last_of('/');
  const std::string identifier{slash==std::string_view::npos?std::string{path}:std::string{path.substr(slash+1)}};

  const auto q=core::query::parse(in.get_path(),kReadOptions);
  const bool pub=is_public(principal);
  const auto& allowedIncludes=pub?std::span<const std::string_view>{kPublicIncludes}
                                 :std::span<const std::string_view>{kAdminIncludes};
  if(!includes_ok(q.include,allowedIncludes) ||
     !tokens_ok(q.options.contains("formats")?q.options.at("formats"):"",kFormats))
   return core::workflow::StepResponse<Output>{{{"status","400"},
     {"body",blog::api::ApiError{}.validation("Invalid include or formats","Validation Error","include")}}};
  if(identifier.empty())
   return core::workflow::StepResponse<Output>{{{"status","400"},
     {"body",blog::api::ApiError{}.validation("A page id, slug or uuid is required.","Bad Request","id")}}};

  // findOne(id) first; Ghost also accepts slug/uuid via the same data bag.
  std::string row;
  if(const auto one=ctx.store().find_by_resource("pages",identifier)) row=*one;
  if(row.empty()) {
   const auto by_slug=ctx.store().json_for("pages",{{"filter","slug:"+identifier}});
   row=first_row(by_slug);
  }
  if(row.empty()) {
   const auto by_uuid=ctx.store().json_for("pages",{{"filter","uuid:"+identifier}});
   row=first_row(by_uuid);
  }
  if(row.empty() || row=="null")
   return core::workflow::StepResponse<Output>{{{"status","404"},
     {"body",blog::api::ApiError{}.json("Page not found.","Page not found")}}};

  // The Content API only exposes published pages.
  if(pub && row_field(row,"status")!="published")
   return core::workflow::StepResponse<Output>{{{"status","404"},
     {"body",blog::api::ApiError{}.json("Page not found.","Page not found")}}};

  // fields/include via core::query projection on the single matched row.
  std::string projected="["+row+"]";
  try { projected=core::query::apply(projected,q,15); } catch (...) {}
  const auto single=first_row(projected);
  const auto cleaned=local_mapping::clean(single,"page");
  const auto body=local_mapping::wrap("pages",cleaned);
  return core::workflow::StepResponse<Output>{{{"status","200"},{"body",body}}};
 }
 void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}
 static blog::BlogContext* s_ctx;
};
blog::BlogContext* ExecuteStep::s_ctx{nullptr};
}
