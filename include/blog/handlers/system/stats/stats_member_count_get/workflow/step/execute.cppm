export module blog_handlers:stats_stats_member_count_get_step;
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

export namespace blog::handlers::stats::stats_member_count_get::workflow {
class ExecuteStep {
public:
 using Input=interfaces::io::IRequest&; using Output=std::string;
 static constexpr std::string_view name{"stats_member_count_get_execute"};
 [[nodiscard]] core::workflow::StepResponse<Output> invoke(const Input& in, core::workflow::WorkflowContext&) const {
  auto& ctx=*s_ctx;
  namespace s=blog::handlers::stats::support;
  static constexpr std::string_view kOptions[]{"date_from"};
  const auto q=core::query::parse(in.get_path(), kOptions);
  (void)q;
  const auto rows=ctx.store().json_for("stats", s::store_params(q));
  const std::string body=s::wrap(rows, s::meta_member_totals(rows));
  return core::workflow::StepResponse<Output>{body};
 }
 void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}
 static blog::BlogContext* s_ctx;
};
blog::BlogContext* ExecuteStep::s_ctx{nullptr};
}
