export module blog_handlers:members_put_detail_step;
import std;
import core_workflow;

import blog_context;
import :mutation;
import :members_operations;
import blog_dto;
import :members_put_detail_model;
import :members_put_detail_dto;
export namespace blog::handlers::members::put_detail::workflow {
class ExecuteStep {
public:
 using Input=model::Request; using Output=std::map<std::string, std::string>;
 static constexpr std::string_view name{"members_put_detail_execute"};
 [[nodiscard]] core::workflow::StepResponse<Output> invoke(const Input& in, core::workflow::WorkflowContext&) const {
  auto& ctx=*s_ctx; blog::api::MutationStore mutations{ctx.store()};
    const auto r=s_ops->member_subscription_edit("1", "1", in.body);
  if(r.first!=0) return core::workflow::StepResponse<Output>{{{"status",std::to_string(r.first)},{"body",r.second}}};
  return core::workflow::StepResponse<Output>{{{"status","200"},{"body",mutations.edit("members", in.id, in.body.empty()?"{}":in.body).body()}}};
 }
 void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}
 static blog::BlogContext* s_ctx; static blog::api::members::MembersOperations* s_ops;
};
blog::api::members::MembersOperations* ExecuteStep::s_ops{nullptr};
blog::BlogContext* ExecuteStep::s_ctx{nullptr};
}