export module blog_handlers:pages_read_by_slug_step;
import :domain_all;
import :pages_dto_links;
import std;
import core_workflow;
import core_query;
import core_auth;
import core_serde;
import interfaces;

export namespace blog::handlers::pages::read_by_slug::workflow {

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
 static constexpr std::string_view name{"pages_read_by_slug_execute"};
 [[nodiscard]] core::workflow::StepResponse<Output> invoke(const Input& in, core::workflow::WorkflowContext&) const {
  using namespace blog::handlers::pages::support;
  auto& ctx=*s_ctx;

  const auto principal=principal_for(in);
  core::auth::ghost::GhostAuthorizer authorizer;
  const auto authorized=authorizer.authorize(principal,true,"pages","read");
  if(!authorized.allowed)
   return core::workflow::StepResponse<Output>{{{"status",std::to_string(authorized.status)},{"body",authorized.error_json}}};

  const std::string raw{in.get_path()};
  auto path=std::string_view{raw}.substr(0,raw.find('?'));
  while(path.size()>1 && path.back()=='/') path.remove_suffix(1);
  const auto slash=path.find_last_of('/');
  const std::string slug{slash==std::string_view::npos?std::string{path}:std::string{path.substr(slash+1)}};

  const auto q=core::query::parse(in.get_path(),kReadOptions);
  const bool pub=is_public(principal);
  const auto& allowedIncludes=pub?std::span<const std::string_view>{kPublicIncludes}
                                 :std::span<const std::string_view>{kAdminIncludes};
  if(!includes_ok(q.include,allowedIncludes) ||
     !tokens_ok(q.options.contains("formats")?q.options.at("formats"):"",kFormats))
   return core::workflow::StepResponse<Output>{{{"status","400"},
     {"body",blog::api::ApiError{}.validation("Invalid include or formats","Validation Error","include")}}};
  if(slug.empty())
   return core::workflow::StepResponse<Output>{{{"status","400"},
     {"body",blog::api::ApiError{}.validation("A page id, slug or uuid is required.","Bad Request","slug")}}};

  // Ghost: models.Post.findOne({slug}, options) with forced type/status filter.
  auto scoped=q;
  scoped.raw_filter="slug:"+slug;
  if(pub) scoped.filters.emplace_back("status","published");
  std::string applied;
  try { applied=core::query::apply(ctx.store().json_for("pages",{}),scoped,15); } catch (...) {}
  const auto first=first_row(applied);
  if(first.empty() || first=="null")
   return core::workflow::StepResponse<Output>{{{"status","404"},
     {"body",blog::api::ApiError{}.json("Page not found.","Page not found")}}};

  const auto cleaned=local_mapping::clean(first,"page");
  const auto body=local_mapping::wrap("pages",cleaned);
  return core::workflow::StepResponse<Output>{{{"status","200"},{"body",body}}};
 }
 void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}
 static blog::BlogContext* s_ctx;
};
blog::BlogContext* ExecuteStep::s_ctx{nullptr};
}
