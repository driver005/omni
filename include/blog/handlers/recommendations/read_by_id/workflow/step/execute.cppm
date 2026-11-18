export module blog_handlers:recommendations_read_by_id_step;
import std;
import core_workflow;
import blog_context;
import :mutation;

import blog_dto;
import :recommendations_read_by_id_model;
import :recommendations_read_by_id_dto;
export namespace blog::handlers::recommendations::read_by_id::workflow {
class ExecuteStep {
public:
 using Input=model::Request; using Output=std::string;
 static constexpr std::string_view name{"recommendations_read_by_id_execute"};
 [[nodiscard]] core::workflow::StepResponse<Output> invoke(const Input& in, core::workflow::WorkflowContext&) const {
  auto& ctx=*s_ctx; blog::api::MutationStore mutations{ctx.store()};
  
  const auto one=ctx.store().find_by_resource("recommendations", in.id.empty()?"1":in.id);
  if(one) return core::workflow::StepResponse<Output>{"{\"recommendations\":[" + *one + "]}"};
  return core::workflow::StepResponse<Output>{R"({"recommendations":[]})"};
 }
 void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}
 static blog::BlogContext* s_ctx;
};
blog::BlogContext* ExecuteStep::s_ctx{nullptr};
}
