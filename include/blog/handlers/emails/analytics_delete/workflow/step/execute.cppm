export module blog_handlers:emails_analytics_delete_step;
import std;
import core_workflow;

import blog_context;
import :mutation;
import :emails_operations;
import blog_dto;
import :emails_analytics_delete_model;
import :emails_analytics_delete_dto;
export namespace blog::handlers::emails::analytics_delete::workflow {
class ExecuteStep {
public:
 using Input=model::Request; using Output=std::map<std::string, std::string>;
 static constexpr std::string_view name{"emails_analytics_delete_execute"};
 [[nodiscard]] core::workflow::StepResponse<Output> invoke(const Input& in, core::workflow::WorkflowContext&) const {
  auto& ctx=*s_ctx; blog::api::MutationStore mutations{ctx.store()};
    const auto r=s_ops->emails_analytics_delete();
  if(r.first!=0) return core::workflow::StepResponse<Output>{{{"status",std::to_string(r.first)},{"body",r.second}}};
  return core::workflow::StepResponse<Output>{{{"status","200"},{"body",mutations.bulk_destroy("emails", in.body.empty()?"{}":in.body).body()}}};
 }
 void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}
 static blog::BlogContext* s_ctx; static blog::api::emails::EmailsOperations* s_ops;
};
blog::api::emails::EmailsOperations* ExecuteStep::s_ops{nullptr};
blog::BlogContext* ExecuteStep::s_ctx{nullptr};
}