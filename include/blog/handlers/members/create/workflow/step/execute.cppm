export module blog_handlers:members_create_step;
import std;
import core_workflow;
import blog_context;
import :mutation;

import blog_dto;
import :members_create_model;
import :members_create_dto;
export namespace blog::handlers::members::create::workflow {
class ExecuteStep {
public:
 using Input=model::Request; using Output=std::string;
 static constexpr std::string_view name{"members_create_execute"};
 [[nodiscard]] core::workflow::StepResponse<Output> invoke(const Input& in, core::workflow::WorkflowContext&) const {
  auto& ctx=*s_ctx; blog::api::MutationStore mutations{ctx.store()};
  
  return core::workflow::StepResponse<Output>{mutations.create("members", in.body.empty()?"{}":in.body).body()};
 }
 void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}
 static blog::BlogContext* s_ctx;
};
blog::BlogContext* ExecuteStep::s_ctx{nullptr};
}