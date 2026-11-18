export module blog_handlers:stats_subscriptions_get_step;
import std;
import core_workflow;
import blog_context;
import :mutation;
import blog_handlers_middleware;

import blog_dto;
import :stats_subscriptions_get_model;
import :stats_subscriptions_get_dto;
export namespace blog::handlers::stats::subscriptions_get::workflow {
class ExecuteStep {
public:
 using Input=model::Request; using Output=std::string;
 static constexpr std::string_view name{"stats_subscriptions_get_execute"};
 [[nodiscard]] core::workflow::StepResponse<Output> invoke(const Input& in, core::workflow::WorkflowContext&) const {
  auto& ctx=*s_ctx; blog::api::MutationStore mutations{ctx.store()};
    const auto r=[&]()->std::pair<int,std::string>{
        return {200, blog::handlers::middleware::encode(blog::dto::StatsEmptyResponse{})};
    }();
  if(r.first!=0) return core::workflow::StepResponse<Output>{r.second};
  const auto rows=ctx.store().json_for("stats", {});
  return core::workflow::StepResponse<Output>{"{\"stats\":" + rows + ",\"meta\":{\"pagination\":{\"limit\":15,\"page\":1,\"pages\":1,\"prev\":null,\"next\":null,\"total\":0}}}}"};
 }
 void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}
 static blog::BlogContext* s_ctx;
};
blog::BlogContext* ExecuteStep::s_ctx{nullptr};
}
