export module blog_handlers:redirects_redirects_download_get_step;
import std;
import core_workflow;
import blog_context;
import :mutation;

import blog_dto;
import :redirects_redirects_download_get_model;
import :redirects_redirects_download_get_dto;
export namespace blog::handlers::redirects::redirects_download_get::workflow {
class ExecuteStep {
public:
 using Input=model::Request; using Output=std::string;
 static constexpr std::string_view name{"redirects_redirects_download_get_execute"};
 [[nodiscard]] core::workflow::StepResponse<Output> invoke(const Input& in, core::workflow::WorkflowContext&) const {
  auto& ctx=*s_ctx; blog::api::MutationStore mutations{ctx.store()};
  
  const auto rows=ctx.store().json_for("redirects", {});
  return core::workflow::StepResponse<Output>{"{\"redirects\":" + rows + ",\"meta\":{\"pagination\":{\"limit\":15,\"page\":1,\"pages\":1,\"prev\":null,\"next\":null,\"total\":0}}}}"};
 }
 void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}
 static blog::BlogContext* s_ctx;
};
blog::BlogContext* ExecuteStep::s_ctx{nullptr};
}
