export module blog_handlers:users_users_id_token_put_step;
import std;
import core_workflow;
import interfaces;
import blog_context;
import :mutation;

import blog_dto;
export namespace blog::handlers::users::users_id_token_put::workflow {
class ExecuteStep {
public:
 using Input=interfaces::io::IRequest&; using Output=std::string;
 static constexpr std::string_view name{"users_users_id_token_put_execute"};
 [[nodiscard]] core::workflow::StepResponse<Output> invoke(const Input& in, core::workflow::WorkflowContext&) const {
  auto& ctx=*s_ctx; blog::api::MutationStore mutations{ctx.store()};
  
  return core::workflow::StepResponse<Output>{mutations.edit("users", std::string{}, in.body_as_string().empty()?"{}":in.body_as_string()).body()};
 }
 void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}
 static blog::BlogContext* s_ctx;
};
blog::BlogContext* ExecuteStep::s_ctx{nullptr};
}