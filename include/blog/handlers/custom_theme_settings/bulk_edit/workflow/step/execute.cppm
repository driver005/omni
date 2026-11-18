export module blog_handlers:custom_theme_settings_bulk_edit_step;
import std;
import core_workflow;
import blog_context;
import :mutation;
import :special;
import blog_dto;
import :custom_theme_settings_bulk_edit_model;
import :custom_theme_settings_bulk_edit_dto;
export namespace blog::handlers::custom_theme_settings::bulk_edit::workflow {
class ExecuteStep {
public:
 using Input=model::Request; using Output=std::string;
 static constexpr std::string_view name{"custom_theme_settings_bulk_edit_execute"};
 [[nodiscard]] core::workflow::StepResponse<Output> invoke(const Input& in, core::workflow::WorkflowContext&) const {
  auto& ctx=*s_ctx; blog::api::MutationStore mutations{ctx.store()};
  auto& special=*s_special;
  const auto r=special.custom_theme_settings_put();
  if(r.status!=0) return core::workflow::StepResponse<Output>{r.body};
  
  return core::workflow::StepResponse<Output>{mutations.bulk_edit("custom_theme_settings", in.body.empty()?"{}":in.body).body()};
 }
 void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}
 static blog::BlogContext* s_ctx; static blog::api::SpecialEndpoints* s_special;
};
blog::BlogContext* ExecuteStep::s_ctx{nullptr};
blog::api::SpecialEndpoints* ExecuteStep::s_special{nullptr};
}