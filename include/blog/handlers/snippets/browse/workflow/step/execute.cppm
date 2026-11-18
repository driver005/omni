export module blog_handlers:snippets_browse_step;
import std;
import core_workflow;
import blog_context;
import :mutation;
import :special;
import blog_dto;
import :snippets_browse_model;
import :snippets_browse_dto;
export namespace blog::handlers::snippets::browse::workflow {
class ExecuteStep {
public:
 using Input=model::Request; using Output=std::string;
 static constexpr std::string_view name{"snippets_browse_execute"};
 [[nodiscard]] core::workflow::StepResponse<Output> invoke(const Input& in, core::workflow::WorkflowContext&) const {
  auto& ctx=*s_ctx; blog::api::MutationStore mutations{ctx.store()};
  auto& special=*s_special;
  const auto rows=ctx.store().json_for("snippets", {});
  return core::workflow::StepResponse<Output>{"{\"snippets\":" + rows + ",\"meta\":{\"pagination\":{\"limit\":15,\"page\":1,\"pages\":1,\"prev\":null,\"next\":null,\"total\":0}}}}"};
 }
 void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}
 static blog::BlogContext* s_ctx; static blog::api::SpecialEndpoints* s_special;
};
blog::BlogContext* ExecuteStep::s_ctx{nullptr};
blog::api::SpecialEndpoints* ExecuteStep::s_special{nullptr};
}
