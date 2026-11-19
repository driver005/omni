export module blog_handlers:automations_automations_id_email_preview_post_step;
import std;
import core_workflow;
import blog_context;
import :mutation;
import :automations_operations;
import blog_dto;
import :automations_automations_id_email_preview_post_model;
import :automations_automations_id_email_preview_post_dto;
export namespace blog::handlers::automations::automations_id_email_preview_post::workflow {
class ExecuteStep {
public:
 using Input=model::Request; using Output=std::map<std::string, std::string>;
 static constexpr std::string_view name{"automations_automations_id_email_preview_post_execute"};
 [[nodiscard]] core::workflow::StepResponse<Output> invoke(const Input& in, core::workflow::WorkflowContext&) const {
  auto& ctx=*s_ctx; blog::api::MutationStore mutations{ctx.store()};
    const auto r=s_ops->automation_email_preview("1", in.body);
  if(r.first!=0) return core::workflow::StepResponse<Output>{{{"status",std::to_string(r.first)},{"body",r.second}}};
  return core::workflow::StepResponse<Output>{{{"status","200"},{"body",mutations.create("automations", in.body.empty()?"{}":in.body).body()}}};
 }
 static blog::api::automations::AutomationsOperations* s_ops;
 void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}
 static blog::BlogContext* s_ctx;
};
blog::api::automations::AutomationsOperations* ExecuteStep::s_ops{nullptr};
blog::BlogContext* ExecuteStep::s_ctx{nullptr};
}