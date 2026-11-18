export module blog_handlers:gift_links_gift_links_remove_all_put_step;
import std;
import core_workflow;
import blog_context;
import :mutation;
import blog_handlers_middleware;

import blog_dto;
import :gift_links_gift_links_remove_all_put_model;
import :gift_links_gift_links_remove_all_put_dto;
export namespace blog::handlers::gift_links::gift_links_remove_all_put::workflow {
class ExecuteStep {
public:
 using Input=model::Request; using Output=std::string;
 static constexpr std::string_view name{"gift_links_gift_links_remove_all_put_execute"};
 [[nodiscard]] core::workflow::StepResponse<Output> invoke(const Input& in, core::workflow::WorkflowContext&) const {
  auto& ctx=*s_ctx; blog::api::MutationStore mutations{ctx.store()};
    const auto r=[&]()->std::pair<int,std::string>{
        blog::dto::GiftLinksResponse response;
        response.gift_links.push_back(blog::dto::LinkRecord{});
        return {200, blog::handlers::middleware::encode(response)};
    }();
  if(r.first!=0) return core::workflow::StepResponse<Output>{r.second};
  return core::workflow::StepResponse<Output>{mutations.edit("gift_links", in.id, in.body.empty()?"{}":in.body).body()};
 }
 void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}
 static blog::BlogContext* s_ctx;
};
blog::BlogContext* ExecuteStep::s_ctx{nullptr};
}