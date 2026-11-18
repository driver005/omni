export module blog_handlers:custom_theme_settings_browse_step;
import std;
import core_workflow;
import blog_context;
import :mutation;
import :special;
import blog_dto;
import :custom_theme_settings_browse_model;
import :custom_theme_settings_browse_dto;
export namespace blog::handlers::custom_theme_settings::browse::workflow {
class ExecuteStep {
public:
 using Input=model::Request; using Output=std::string;
 static constexpr std::string_view name{"custom_theme_settings_browse_execute"};
 [[nodiscard]] core::workflow::StepResponse<Output> invoke(const Input& in, core::workflow::WorkflowContext&) const {
  auto& ctx=*s_ctx; blog::api::MutationStore mutations{ctx.store()};
  auto& special=*s_special;
  const auto r=special.dispatch(std::string_view{in.path}.substr(0, in.path.find('?')), "GET", in.id, in.body);
  if(r.status()!=0) return core::workflow::StepResponse<Output>{r.body()};
  const auto rows=ctx.store().json_for("custom_theme_settings", {});
  return core::workflow::StepResponse<Output>{"{\"custom_theme_settings\":" + rows + ",\"meta\":{\"pagination\":{\"limit\":15,\"page\":1,\"pages\":1,\"prev\":null,\"next\":null,\"total\":0}}}}"};
 }
 void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}
 static blog::BlogContext* s_ctx; static blog::api::SpecialEndpoints* s_special;
};
blog::BlogContext* ExecuteStep::s_ctx{nullptr};
blog::api::SpecialEndpoints* ExecuteStep::s_special{nullptr};
}
