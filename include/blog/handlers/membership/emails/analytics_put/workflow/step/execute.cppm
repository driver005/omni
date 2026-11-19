export module blog_handlers:emails_analytics_put_step;
import :domain_all;
import std;
import core_workflow;
import interfaces;

import :emails_operations;
export namespace blog::handlers::emails::analytics_put::workflow {
class ExecuteStep {
public:
 using Input=interfaces::io::IRequest&; using Output=std::map<std::string, std::string>;
 static constexpr std::string_view name{"emails_analytics_put_execute"};
 [[nodiscard]] core::workflow::StepResponse<Output> invoke(const Input& in, core::workflow::WorkflowContext&) const {
  auto& ctx=*s_ctx; auto& mutations = ctx.store();
    const auto r=s_ops->emails_analytics_put(in.body_as_string());
  if(r.first!=0) return core::workflow::StepResponse<Output>{{{"status",std::to_string(r.first)},{"body",r.second}}};
  return core::workflow::StepResponse<Output>{{{"status","200"},{"body",mutations.edit("emails", std::string{}, in.body_as_string().empty()?"{}":in.body_as_string()).body()}}};
 }
 void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}
 static blog::BlogContext* s_ctx; static blog::api::emails::EmailsOperations* s_ops;
};
blog::api::emails::EmailsOperations* ExecuteStep::s_ops{nullptr};
blog::BlogContext* ExecuteStep::s_ctx{nullptr};
}