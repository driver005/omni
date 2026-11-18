export module blog_handlers:users_users_password_put_step;
import std;
import core_workflow;
import blog_context;
import :mutation;
import blog_handlers_middleware;

import blog_dto;
import :users_users_password_put_model;
import :users_users_password_put_dto;
export namespace blog::handlers::users::users_password_put::workflow {
class ExecuteStep {
public:
 using Input=model::Request; using Output=std::string;
 static constexpr std::string_view name{"users_users_password_put_execute"};
 [[nodiscard]] core::workflow::StepResponse<Output> invoke(const Input& in, core::workflow::WorkflowContext&) const {
  auto& ctx=*s_ctx; blog::api::MutationStore mutations{ctx.store()};
    const auto r=[&]()->std::pair<int,std::string>{
        return {200, blog::handlers::middleware::encode(blog::dto::PasswordResponse{})};
    }();
  if(r.first!=0) return core::workflow::StepResponse<Output>{r.second};
  return core::workflow::StepResponse<Output>{mutations.edit("users", in.id, in.body.empty()?"{}":in.body).body()};
 }
 void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}
 static blog::BlogContext* s_ctx;
};
blog::BlogContext* ExecuteStep::s_ctx{nullptr};
}