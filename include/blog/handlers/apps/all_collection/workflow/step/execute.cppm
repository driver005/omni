export module blog_handlers:apps_all_collection_step;
import std;
import core_workflow;
import blog_context;
import :mutation;

import blog_dto;
import :apps_all_collection_model;
import :apps_all_collection_dto;
export namespace blog::handlers::apps::all_collection::workflow {
class ExecuteStep {
public:
 using Input=model::Request; using Output=std::string;
 static constexpr std::string_view name{"apps_all_collection_execute"};
 [[nodiscard]] core::workflow::StepResponse<Output> invoke(const Input& in, core::workflow::WorkflowContext&) const {
  auto& ctx=*s_ctx; blog::api::MutationStore mutations{ctx.store()};
  const auto rows=ctx.store().json_for("apps", {});
  return core::workflow::StepResponse<Output>{"{\"apps\":" + rows + ",\"meta\":{\"pagination\":{\"limit\":15,\"page\":1,\"pages\":1,\"prev\":null,\"next\":null,\"total\":0}}}"};
  return core::workflow::StepResponse<Output>{"{}"};
 }
 void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}
 static blog::BlogContext* s_ctx;
};
blog::BlogContext* ExecuteStep::s_ctx{nullptr};
}