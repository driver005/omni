export module blog_handlers:members_members_stats_count_get_step;
import :domain_all;
import std;
import core_workflow;
import interfaces;
import blog_handlers_middleware;

export namespace blog::handlers::members::members_stats_count_get::workflow {
class ExecuteStep {
public:
 using Input=interfaces::io::IRequest&; using Output=std::string;
 static constexpr std::string_view name{"members_members_stats_count_get_execute"};
 [[nodiscard]] core::workflow::StepResponse<Output> invoke(const Input& in, core::workflow::WorkflowContext&) const {
  auto& ctx=*s_ctx; auto& mutations = ctx.store();
    const auto r=[&]()->std::pair<int,std::string>{
        blog::dto::StatsMemberResponse response;
        response.stats.push_back(blog::dto::StatsMemberEntry{});
        return {200, blog::handlers::middleware::encode(response)};
    }();
  if(r.first!=0) return core::workflow::StepResponse<Output>{r.second};
  const auto rows=ctx.store().json_for("members", {});
  return core::workflow::StepResponse<Output>{"{\"members\":" + rows + ",\"meta\":{\"pagination\":{\"limit\":15,\"page\":1,\"pages\":1,\"prev\":null,\"next\":null,\"total\":0}}}}"};
 }
 void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}
 static blog::BlogContext* s_ctx;
};
blog::BlogContext* ExecuteStep::s_ctx{nullptr};
}
