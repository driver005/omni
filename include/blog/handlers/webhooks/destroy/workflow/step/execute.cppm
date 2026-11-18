export module blog_handlers:webhooks_destroy_step;
import std;
import core_workflow;
import blog_context;
import :mutation;

import blog_dto;
import :webhooks_destroy_model;
import :webhooks_destroy_dto;
export namespace blog::handlers::webhooks::destroy::workflow {
class ExecuteStep {
public:
 using Input=model::Request; using Output=std::string;
 static constexpr std::string_view name{"webhooks_destroy_execute"};
 [[nodiscard]] core::workflow::StepResponse<Output> invoke(const Input& in, core::workflow::WorkflowContext&) const {
  auto& ctx=*s_ctx; blog::api::MutationStore mutations{ctx.store()};
  
  const auto result = in.id.empty()
      ? mutations.bulk_destroy("webhooks", in.body.empty() ? "{}" : in.body)
      : mutations.destroy("webhooks", in.id);
  return core::workflow::StepResponse<Output>{result.body()};
 }
 void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}
 static blog::BlogContext* s_ctx;
};
blog::BlogContext* ExecuteStep::s_ctx{nullptr};
}