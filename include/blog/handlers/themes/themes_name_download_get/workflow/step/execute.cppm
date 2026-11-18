export module blog_handlers:themes_themes_name_download_get_step;
import std;
import core_workflow;
import blog_context;
import :mutation;
import :special;
import blog_dto;
import :themes_themes_name_download_get_model;
import :themes_themes_name_download_get_dto;
export namespace blog::handlers::themes::themes_name_download_get::workflow {
class ExecuteStep {
public:
 using Input=model::Request; using Output=std::string;
 static constexpr std::string_view name{"themes_themes_name_download_get_execute"};
 [[nodiscard]] core::workflow::StepResponse<Output> invoke(const Input& in, core::workflow::WorkflowContext&) const {
  auto& ctx=*s_ctx; blog::api::MutationStore mutations{ctx.store()};
  auto& special=*s_special;
  
  const auto one=ctx.store().find_by_resource("themes", in.id.empty()?"1":in.id);
  if(one) return core::workflow::StepResponse<Output>{"{\"themes\":[" + *one + "]}"};
  return core::workflow::StepResponse<Output>{R"({"themes":[]})"};
 }
 void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}
 static blog::BlogContext* s_ctx; static blog::api::SpecialEndpoints* s_special;
};
blog::BlogContext* ExecuteStep::s_ctx{nullptr};
blog::api::SpecialEndpoints* ExecuteStep::s_special{nullptr};
}
