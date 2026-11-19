export module blog_handlers:incoming_recommendations_browse_step;
import :domain_all;
import std;
import core_workflow;
import interfaces;

export namespace blog::handlers::incoming_recommendations::browse::workflow {
class ExecuteStep {
public:
 using Input=interfaces::io::IRequest&; using Output=std::string;
 static constexpr std::string_view name{"incoming_recommendations_browse_execute"};
 [[nodiscard]] core::workflow::StepResponse<Output> invoke(const Input& in, core::workflow::WorkflowContext&) const {
  auto& ctx=*s_ctx; auto& mutations = ctx.store();
  
  const auto rows=ctx.store().json_for("incoming_recommendations", {});
  return core::workflow::StepResponse<Output>{"{\"incoming_recommendations\":" + rows + ",\"meta\":{\"pagination\":{\"limit\":15,\"page\":1,\"pages\":1,\"prev\":null,\"next\":null,\"total\":0}}}}"};
 }
 void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}
 static blog::BlogContext* s_ctx;
};
blog::BlogContext* ExecuteStep::s_ctx{nullptr};
}
