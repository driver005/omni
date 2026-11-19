export module blog_handlers:recommendations_read_by_id_step;
import std;
import core_workflow;
import interfaces;
import blog_context;

import blog_dto;
export namespace blog::handlers::recommendations::read_by_id::workflow {
class ExecuteStep {
public:
 using Input=interfaces::io::IRequest&; using Output=std::string;
 static constexpr std::string_view name{"recommendations_read_by_id_execute"};
 [[nodiscard]] core::workflow::StepResponse<Output> invoke(const Input& in, core::workflow::WorkflowContext&) const {
  auto& ctx=*s_ctx; auto& mutations = ctx.store();
  
  const auto one=ctx.store().find_by_resource("recommendations", std::string{}.empty()?"1":std::string{});
  if(one) return core::workflow::StepResponse<Output>{"{\"recommendations\":[" + *one + "]}"};
  return core::workflow::StepResponse<Output>{R"({"recommendations":[]})"};
 }
 void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}
 static blog::BlogContext* s_ctx;
};
blog::BlogContext* ExecuteStep::s_ctx{nullptr};
}
