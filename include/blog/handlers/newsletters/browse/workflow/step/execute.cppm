export module blog_handlers:newsletters_browse_step;
import std;
import core_workflow;
import blog_context;
import :mutation;

import blog_dto;
import :newsletters_browse_model;
import :newsletters_browse_dto;
export namespace blog::handlers::newsletters::browse::workflow {
class ExecuteStep {
public:
 using Input=model::Request; using Output=std::string;
 static constexpr std::string_view name{"newsletters_browse_execute"};
 [[nodiscard]] core::workflow::StepResponse<Output> invoke(const Input& in, core::workflow::WorkflowContext&) const {
  auto& ctx=*s_ctx; blog::api::MutationStore mutations{ctx.store()};
  
  const auto rows=ctx.store().json_for("newsletters", {});
  return core::workflow::StepResponse<Output>{"{\"newsletters\":" + rows + ",\"meta\":{\"pagination\":{\"limit\":15,\"page\":1,\"pages\":1,\"prev\":null,\"next\":null,\"total\":0}}}}"};
 }
 void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}
 static blog::BlogContext* s_ctx;
};
blog::BlogContext* ExecuteStep::s_ctx{nullptr};
}
