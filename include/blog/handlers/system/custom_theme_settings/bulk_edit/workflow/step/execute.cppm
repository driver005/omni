export module blog_handlers:custom_theme_settings_bulk_edit_step;
import std;
import core_workflow;
import interfaces;
import blog_context;

import blog_dto;
export namespace blog::handlers::custom_theme_settings::bulk_edit::workflow {
class ExecuteStep {
public:
 using Input=interfaces::io::IRequest&; using Output=std::string;
 static constexpr std::string_view name{"custom_theme_settings_bulk_edit_execute"};
 [[nodiscard]] core::workflow::StepResponse<Output> invoke(const Input& in, core::workflow::WorkflowContext&) const {
  auto& ctx=*s_ctx; auto& mutations = ctx.store();
    const auto r=[&]()->std::pair<int,std::string>{
        return {200, R"({"custom_theme_settings":[],"meta":{}})"};
    }();
  if(r.first!=0) return core::workflow::StepResponse<Output>{r.second};
  
  return core::workflow::StepResponse<Output>{mutations.bulk_edit("custom_theme_settings", in.body_as_string().empty()?"{}":in.body_as_string()).body()};
 }
 void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}
 static blog::BlogContext* s_ctx;
};
blog::BlogContext* ExecuteStep::s_ctx{nullptr};
}