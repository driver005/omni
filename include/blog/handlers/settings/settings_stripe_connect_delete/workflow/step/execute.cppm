export module blog_handlers:settings_settings_stripe_connect_delete_step;
import std;
import core_workflow;
import blog_context;
import :mutation;

import blog_dto;
import :settings_settings_stripe_connect_delete_model;
import :settings_settings_stripe_connect_delete_dto;
export namespace blog::handlers::settings::settings_stripe_connect_delete::workflow {
class ExecuteStep {
public:
 using Input=model::Request; using Output=std::string;
 static constexpr std::string_view name{"settings_settings_stripe_connect_delete_execute"};
 [[nodiscard]] core::workflow::StepResponse<Output> invoke(const Input& in, core::workflow::WorkflowContext&) const {
  auto& ctx=*s_ctx; blog::api::MutationStore mutations{ctx.store()};
  
  return core::workflow::StepResponse<Output>{mutations.bulk_destroy("settings", in.body.empty()?"{}":in.body).body()};
 }
 void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}
 static blog::BlogContext* s_ctx;
};
blog::BlogContext* ExecuteStep::s_ctx{nullptr};
}