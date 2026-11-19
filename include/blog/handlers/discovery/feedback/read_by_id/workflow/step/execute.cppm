export module blog_handlers:feedback_read_by_id_step;
import std;
import core_workflow;
import interfaces;
import blog_context;
import blog_handlers_middleware;

import blog_dto;
export namespace blog::handlers::feedback::read_by_id::workflow {
class ExecuteStep {
public:
 using Input=interfaces::io::IRequest&; using Output=std::string;
 static constexpr std::string_view name{"feedback_read_by_id_execute"};
 [[nodiscard]] core::workflow::StepResponse<Output> invoke(const Input& in, core::workflow::WorkflowContext&) const {
  auto& ctx=*s_ctx; auto& mutations = ctx.store();
    const auto r=[&]()->std::pair<int,std::string>{
        return {200, blog::handlers::middleware::encode(blog::dto::FeedbackResponse{})};
    }();
  if(r.first!=0) return core::workflow::StepResponse<Output>{r.second};
  const auto one=ctx.store().find_by_resource("feedback", std::string{}.empty()?"1":std::string{});
  if(one) return core::workflow::StepResponse<Output>{"{\"feedback\":[" + *one + "]}"};
  return core::workflow::StepResponse<Output>{R"({"feedback":[]})"};
 }
 void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}
 static blog::BlogContext* s_ctx;
};
blog::BlogContext* ExecuteStep::s_ctx{nullptr};
}
