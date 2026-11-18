export module blog_handlers:invites_browse_step;
import std;
import core_workflow;
import blog_context;
import :mutation;
import blog_handlers_middleware;

import blog_dto;
import :invites_browse_model;
import :invites_browse_dto;
export namespace blog::handlers::invites::browse::workflow {
class ExecuteStep {
public:
 using Input=model::Request; using Output=std::string;
 static constexpr std::string_view name{"invites_browse_execute"};
 [[nodiscard]] core::workflow::StepResponse<Output> invoke(const Input& in, core::workflow::WorkflowContext&) const {
  auto& ctx=*s_ctx; blog::api::MutationStore mutations{ctx.store()};
    const auto r=[&]()->std::pair<int,std::string>{
        blog::dto::InvitesResponse response;
        response.invites.push_back(blog::dto::InviteDto{});
        return {200, blog::handlers::middleware::encode(response)};
    }();
  if(r.first!=0) return core::workflow::StepResponse<Output>{r.second};
  const auto rows=ctx.store().json_for("invites", {});
  return core::workflow::StepResponse<Output>{"{\"invites\":" + rows + ",\"meta\":{\"pagination\":{\"limit\":15,\"page\":1,\"pages\":1,\"prev\":null,\"next\":null,\"total\":0}}}}"};
 }
 void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}
 static blog::BlogContext* s_ctx;
};
blog::BlogContext* ExecuteStep::s_ctx{nullptr};
}
