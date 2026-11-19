export module blog_handlers:members_put_detail_step;
import :domain_all;
import std;
import core_workflow;
import interfaces;

import :members_operations;
export namespace blog::handlers::members::put_detail::workflow {
class ExecuteStep {
public:
 using Input=interfaces::io::IRequest&; using Output=std::map<std::string, std::string>;
 static constexpr std::string_view name{"members_put_detail_execute"};
 [[nodiscard]] core::workflow::StepResponse<Output> invoke(const Input& in, core::workflow::WorkflowContext&) const {
  auto& ctx=*s_ctx; auto& mutations = ctx.store();
    const auto r=s_ops->member_subscription_edit("1", "1", in.body_as_string());
  if(r.first!=0) return core::workflow::StepResponse<Output>{{{"status",std::to_string(r.first)},{"body",r.second}}};
  return core::workflow::StepResponse<Output>{{{"status","200"},{"body",mutations.edit("members", std::string{}, in.body_as_string().empty()?"{}":in.body_as_string()).body()}}};
 }
 void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}
 static blog::BlogContext* s_ctx; static blog::api::members::MembersOperations* s_ops;
};
blog::api::members::MembersOperations* ExecuteStep::s_ops{nullptr};
blog::BlogContext* ExecuteStep::s_ctx{nullptr};
}