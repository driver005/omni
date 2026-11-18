export module blog_handlers:authentication_authentication_password_reset_put_step;
import std;
import core_workflow;
import blog_context;
import :mutation;

import blog_dto;
import :authentication_authentication_password_reset_put_model;
import :authentication_authentication_password_reset_put_dto;
export namespace blog::handlers::authentication::authentication_password_reset_put::workflow {
class ExecuteStep {
public:
 using Input=model::Request; using Output=std::string;
 static constexpr std::string_view name{"authentication_authentication_password_reset_put_execute"};
 [[nodiscard]] core::workflow::StepResponse<Output> invoke(const Input& in, core::workflow::WorkflowContext&) const {
  auto& ctx=*s_ctx; blog::api::MutationStore mutations{ctx.store()};
  
  return core::workflow::StepResponse<Output>{mutations.edit("authentication", in.id, in.body.empty()?"{}":in.body).body()};
 }
 void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}
 static blog::BlogContext* s_ctx;
};
blog::BlogContext* ExecuteStep::s_ctx{nullptr};
}