export module blog_handlers:stats_stats_mrr_get_step;
import std;
import core_workflow;
import interfaces;
import blog_context;
import :mutation;
import blog_handlers_middleware;

import blog_dto;
export namespace blog::handlers::stats::stats_mrr_get::workflow {
class ExecuteStep {
public:
 using Input=interfaces::io::IRequest&; using Output=std::string;
 static constexpr std::string_view name{"stats_stats_mrr_get_execute"};
 [[nodiscard]] core::workflow::StepResponse<Output> invoke(const Input& in, core::workflow::WorkflowContext&) const {
  auto& ctx=*s_ctx; blog::api::MutationStore mutations{ctx.store()};
    const auto r=[&]()->std::pair<int,std::string>{
        blog::dto::StatsMrrResponse response;
        blog::dto::StatsMrrEntry first;
        first.set_currency("usd");
        first.set_date("1970-01-01");
        first.set_mrr(0);
        blog::dto::StatsMrrEntry second;
        second.set_currency("usd");
        second.set_date("1970-01-01");
        second.set_mrr(1000);
        response.stats.push_back(std::move(first));
        response.stats.push_back(std::move(second));
        response.meta.totals.push_back(blog::dto::StatsMrrTotal{});
        return {200, blog::handlers::middleware::encode(response)};
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
