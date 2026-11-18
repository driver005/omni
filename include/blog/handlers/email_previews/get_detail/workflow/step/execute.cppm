export module blog_handlers:email_previews_get_detail_step;
import std;
import core_workflow;
import blog_context;
import :mutation;
import blog_handlers_middleware;

import blog_dto;
import :email_previews_get_detail_model;
import :email_previews_get_detail_dto;
export namespace blog::handlers::email_previews::get_detail::workflow {
class ExecuteStep {
public:
 using Input=model::Request; using Output=std::string;
 static constexpr std::string_view name{"email_previews_get_detail_execute"};
 [[nodiscard]] core::workflow::StepResponse<Output> invoke(const Input& in, core::workflow::WorkflowContext&) const {
  auto& ctx=*s_ctx; blog::api::MutationStore mutations{ctx.store()};
    const auto r=[&]()->std::pair<int,std::string>{
        blog::dto::EmailPreviewsResponse response;
        response.email_previews.push_back(blog::dto::EmailPreview{});
        return {200, blog::handlers::middleware::encode(response)};
    }();
  if(r.first!=0) return core::workflow::StepResponse<Output>{r.second};
  const auto rows=ctx.store().json_for("email_previews", {});
  return core::workflow::StepResponse<Output>{"{\"email_previews\":" + rows + ",\"meta\":{\"pagination\":{\"limit\":15,\"page\":1,\"pages\":1,\"prev\":null,\"next\":null,\"total\":0}}}}"};
 }
 void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}
 static blog::BlogContext* s_ctx;
};
blog::BlogContext* ExecuteStep::s_ctx{nullptr};
}
