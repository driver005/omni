export module blog_handlers:stats_stats_posts_visitor_counts_post_step;
import std;
import core_workflow;
import blog_context;
import :mutation;
import :special;
import blog_dto;
import :stats_stats_posts_visitor_counts_post_model;
import :stats_stats_posts_visitor_counts_post_dto;
export namespace blog::handlers::stats::stats_posts_visitor_counts_post::workflow {
class ExecuteStep {
public:
 using Input=model::Request; using Output=std::string;
 static constexpr std::string_view name{"stats_stats_posts_visitor_counts_post_execute"};
 [[nodiscard]] core::workflow::StepResponse<Output> invoke(const Input& in, core::workflow::WorkflowContext&) const {
  auto& ctx=*s_ctx; blog::api::MutationStore mutations{ctx.store()};
  auto& special=*s_special;
  const auto r=special.stats_empty();
  if(r.status!=0) return core::workflow::StepResponse<Output>{r.body};
  return core::workflow::StepResponse<Output>{mutations.create("stats", in.body.empty()?"{}":in.body).body()};
 }
 void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}
 static blog::BlogContext* s_ctx; static blog::api::SpecialEndpoints* s_special;
};
blog::BlogContext* ExecuteStep::s_ctx{nullptr};
blog::api::SpecialEndpoints* ExecuteStep::s_special{nullptr};
}