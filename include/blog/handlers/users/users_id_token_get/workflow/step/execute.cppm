export module blog_handlers:users_users_id_token_get_step;
import std;
import core_workflow;
import blog_context;
import :mutation;
import :special;
import blog_dto;
import :users_users_id_token_get_model;
import :users_users_id_token_get_dto;
export namespace blog::handlers::users::users_id_token_get::workflow {
class ExecuteStep {
public:
 using Input=model::Request; using Output=std::string;
 static constexpr std::string_view name{"users_users_id_token_get_execute"};
 [[nodiscard]] core::workflow::StepResponse<Output> invoke(const Input& in, core::workflow::WorkflowContext&) const {
  auto& ctx=*s_ctx; blog::api::MutationStore mutations{ctx.store()};
  auto& special=*s_special;
  
  const auto one=ctx.store().find_by_resource("users", in.id.empty()?"1":in.id);
  if(one) return core::workflow::StepResponse<Output>{"{\"users\":[" + *one + "]}"};
  return core::workflow::StepResponse<Output>{R"({"users":[]})"};
 }
 void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}
 static blog::BlogContext* s_ctx; static blog::api::SpecialEndpoints* s_special;
};
blog::BlogContext* ExecuteStep::s_ctx{nullptr};
blog::api::SpecialEndpoints* ExecuteStep::s_special{nullptr};
}
