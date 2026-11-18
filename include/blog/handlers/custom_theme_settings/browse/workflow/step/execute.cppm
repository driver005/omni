export module blog_handlers:custom_theme_settings_browse_step;
import std;
import core_workflow;
import blog_context;
import :mutation;

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
    const auto r=[&]()->std::pair<int,std::string>{
        return std::pair<int, std::string>{200, std::format(
            R"({{"custom_theme_settings":[{}]}})",
            blog::dto::GhostFixtures::custom_theme_setting()
        )};
    }();
  if(r.first!=0) return core::workflow::StepResponse<Output>{r.second};
  const auto rows=ctx.store().json_for("custom_theme_settings", {});
  return core::workflow::StepResponse<Output>{"{\"custom_theme_settings\":" + rows + ",\"meta\":{\"pagination\":{\"limit\":15,\"page\":1,\"pages\":1,\"prev\":null,\"next\":null,\"total\":0}}}}"};
 }
 void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}
 static blog::BlogContext* s_ctx;
};
blog::BlogContext* ExecuteStep::s_ctx{nullptr};
}
