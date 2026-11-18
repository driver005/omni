export module blog_handlers:stats_stats_posts_visitor_counts_post_step;
import std;
import core_workflow;
import blog_context;
import :mutation;
import blog_handlers_middleware;

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
    const auto r=[&]()->std::pair<int,std::string>{
        return {200, blog::handlers::middleware::encode(blog::dto::StatsEmptyResponse{})};
    }();
  if(r.first!=0) return core::workflow::StepResponse<Output>{r.second};
  return core::workflow::StepResponse<Output>{mutations.create("stats", in.body.empty()?"{}":in.body).body()};
 }
 void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}
 static blog::BlogContext* s_ctx;
};
blog::BlogContext* ExecuteStep::s_ctx{nullptr};
}