export module blog_handlers:pages_gift_links_get_step;
import std;
import core_workflow;
import interfaces;
import blog_context;
import blog_handlers_middleware;

import blog_dto;
export namespace blog::handlers::pages::gift_links_get::workflow {
class ExecuteStep {
public:
 using Input=interfaces::io::IRequest&; using Output=std::string;
 static constexpr std::string_view name{"pages_gift_links_get_execute"};
 [[nodiscard]] core::workflow::StepResponse<Output> invoke(const Input& in, core::workflow::WorkflowContext&) const {
  auto& ctx=*s_ctx; auto& mutations = ctx.store();
    const auto r=[&]()->std::pair<int,std::string>{
        blog::dto::GiftLinksResponse response;
        response.gift_links.push_back(blog::dto::LinkRecord{});
        return {200, blog::handlers::middleware::encode(response)};
    }();
  if(r.first!=0) return core::workflow::StepResponse<Output>{r.second};
  const auto one=ctx.store().find_by_resource("pages", std::string{}.empty()?"1":std::string{});
  if(one) return core::workflow::StepResponse<Output>{"{\"pages\":[" + *one + "]}"};
  return core::workflow::StepResponse<Output>{R"({"pages":[]})"};
 }
 void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}
 static blog::BlogContext* s_ctx;
};
blog::BlogContext* ExecuteStep::s_ctx{nullptr};
}
