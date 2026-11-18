export module blog_handlers:webhooks_edit_step;
import std;
import core_workflow;
import blog_context;
import :mutation;

import blog_dto;
import :webhooks_edit_model;
import :webhooks_edit_dto;
export namespace blog::handlers::webhooks::edit::workflow {
class ExecuteStep {
public:
 using Input=model::Request; using Output=std::string;
 static constexpr std::string_view name{"webhooks_edit_execute"};
 [[nodiscard]] core::workflow::StepResponse<Output> invoke(const Input& in, core::workflow::WorkflowContext&) const {
  auto& ctx=*s_ctx; blog::api::MutationStore mutations{ctx.store()};
  
  const auto body = in.body.empty() ? "{}" : in.body;
  const auto result = in.id == "bulk" ? mutations.bulk_edit("webhooks", body)
                                      : mutations.edit("webhooks", in.id, body);
  return core::workflow::StepResponse<Output>{result.body()};
 }
 void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}
 static blog::BlogContext* s_ctx;
};
blog::BlogContext* ExecuteStep::s_ctx{nullptr};
}