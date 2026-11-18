export module blog_handlers:integrations_read_by_id_step;
import std;
import core_workflow;
import blog_context;
import :mutation;

import blog_dto;
import :integrations_read_by_id_model;
import :integrations_read_by_id_dto;
export namespace blog::handlers::integrations::read_by_id::workflow {
class ExecuteStep {
public:
 using Input=model::Request; using Output=std::string;
 static constexpr std::string_view name{"integrations_read_by_id_execute"};
 [[nodiscard]] core::workflow::StepResponse<Output> invoke(const Input& in, core::workflow::WorkflowContext&) const {
  auto& ctx=*s_ctx; blog::api::MutationStore mutations{ctx.store()};
  
  const auto one=ctx.store().find_by_resource("integrations", in.id.empty()?"1":in.id);
  if(one) return core::workflow::StepResponse<Output>{"{\"integrations\":[" + *one + "]}"};
  return core::workflow::StepResponse<Output>{R"({"integrations":[]})"};
 }
 void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}
 static blog::BlogContext* s_ctx;
};
blog::BlogContext* ExecuteStep::s_ctx{nullptr};
}
