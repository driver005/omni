export module blog_handlers:actions_browse_step;
import std;
import core_workflow;
import blog_context;
import :mutation;
import :special;
import blog_dto;
import :actions_browse_model;
import :actions_browse_dto;
export namespace blog::handlers::actions::browse::workflow {
class ExecuteStep {
public:
 using Input=model::Request; using Output=std::string;
 static constexpr std::string_view name{"actions_browse_execute"};
 [[nodiscard]] core::workflow::StepResponse<Output> invoke(const Input& in, core::workflow::WorkflowContext&) const {
  auto& ctx=*s_ctx; blog::api::MutationStore mutations{ctx.store()};
  auto& special=*s_special;
  
  const auto rows=ctx.store().json_for("actions", {});
  return core::workflow::StepResponse<Output>{"{\"actions\":" + rows + ",\"meta\":{\"pagination\":{\"limit\":15,\"page\":1,\"pages\":1,\"prev\":null,\"next\":null,\"total\":0}}}}"};
 }
 void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}
 static blog::BlogContext* s_ctx; static blog::api::SpecialEndpoints* s_special;
};
blog::BlogContext* ExecuteStep::s_ctx{nullptr};
blog::api::SpecialEndpoints* ExecuteStep::s_special{nullptr};
}
