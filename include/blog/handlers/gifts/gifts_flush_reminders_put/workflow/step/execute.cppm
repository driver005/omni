export module blog_handlers:gifts_gifts_flush_reminders_put_step;
import std;
import core_workflow;
import blog_context;
import :mutation;

import blog_dto;
import :gifts_gifts_flush_reminders_put_model;
import :gifts_gifts_flush_reminders_put_dto;
export namespace blog::handlers::gifts::gifts_flush_reminders_put::workflow {
class ExecuteStep {
public:
 using Input=model::Request; using Output=std::string;
 static constexpr std::string_view name{"gifts_gifts_flush_reminders_put_execute"};
 [[nodiscard]] core::workflow::StepResponse<Output> invoke(const Input& in, core::workflow::WorkflowContext&) const {
  auto& ctx=*s_ctx; blog::api::MutationStore mutations{ctx.store()};
  
  return core::workflow::StepResponse<Output>{mutations.edit("gifts", in.id, in.body.empty()?"{}":in.body).body()};
 }
 void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}
 static blog::BlogContext* s_ctx;
};
blog::BlogContext* ExecuteStep::s_ctx{nullptr};
}