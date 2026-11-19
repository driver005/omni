export module blog_handlers:automations_automations_id_email_test_post_step;
import std;
import core_workflow;
import interfaces;

import blog_context;
import :mutation;
import :automations_operations;
import blog_dto;
export namespace blog::handlers::automations::automations_id_email_test_post::workflow {
class ExecuteStep {
public:
 using Input=interfaces::io::IRequest&; using Output=std::map<std::string, std::string>;
 static constexpr std::string_view name{"automations_automations_id_email_test_post_execute"};
 [[nodiscard]] core::workflow::StepResponse<Output> invoke(const Input& in, core::workflow::WorkflowContext&) const {
  auto& ctx=*s_ctx; blog::api::MutationStore mutations{ctx.store()};
    const auto r=s_ops->automation_email_test("1", in.body_as_string());
  if(r.first!=0) return core::workflow::StepResponse<Output>{{{"status",std::to_string(r.first)},{"body",r.second}}};
  return core::workflow::StepResponse<Output>{{{"status","200"},{"body",mutations.create("automations", in.body_as_string().empty()?"{}":in.body_as_string()).body()}}};
 }
 void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}
 static blog::BlogContext* s_ctx; static blog::api::automations::AutomationsOperations* s_ops;
};
blog::api::automations::AutomationsOperations* ExecuteStep::s_ops{nullptr};
blog::BlogContext* ExecuteStep::s_ctx{nullptr};
}