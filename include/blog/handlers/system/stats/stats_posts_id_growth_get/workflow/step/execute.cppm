export module blog_handlers:stats_stats_posts_id_growth_get_step;
import :domain_all;
import :stats_dto_stats_empty_response;
import :stats_dto_stats_member_entry;
import :stats_dto_stats_member_meta;
import :stats_dto_stats_member_response;
import :stats_dto_stats_member_totals;
import :stats_dto_stats_mrr_entry;
import :stats_dto_stats_mrr_meta;
import :stats_dto_stats_mrr_response;
import :stats_dto_stats_mrr_total;
import std;
import core_workflow;
import core_serde;
import core_query;
import interfaces;
import blog_handlers_middleware;

export namespace blog::handlers::stats::stats_posts_id_growth_get::workflow {
class ExecuteStep {
public:
 using Input=interfaces::io::IRequest&; using Output=std::map<std::string,std::string>;
 static constexpr std::string_view name{"stats_posts_id_growth_get_execute"};
 [[nodiscard]] core::workflow::StepResponse<Output> invoke(const Input& in, core::workflow::WorkflowContext&) const {
  auto& ctx=*s_ctx;
  namespace s=blog::handlers::stats::support;
  static constexpr std::span<const std::string_view> kOptions{};
  const auto q=core::query::parse(in.get_path(), kOptions);
  (void)q;
  const std::string id=s::path_id(in.get_path());
  if (id.empty())
   return core::workflow::StepResponse<Output>{s::result("422",s::invalid("id required","id","path"))};
  const auto rows=ctx.store().json_for("stats", {{ "filter[id]", id }});
  const std::string body=s::wrap(rows, "");
  return core::workflow::StepResponse<Output>{s::result("200",body)};
 }
 void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}
 static blog::BlogContext* s_ctx;
};
blog::BlogContext* ExecuteStep::s_ctx{nullptr};
}
