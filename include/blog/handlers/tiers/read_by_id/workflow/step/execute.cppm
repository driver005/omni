export module blog_handlers:tiers_read_by_id_step;
import std;
import core_workflow;
import blog_context;
import :mutation;

import blog_dto;
import :tiers_read_by_id_model;
import :tiers_read_by_id_dto;
export namespace blog::handlers::tiers::read_by_id::workflow {
class ExecuteStep {
public:
 using Input=model::Request; using Output=std::string;
 static constexpr std::string_view name{"tiers_read_by_id_execute"};
 [[nodiscard]] core::workflow::StepResponse<Output> invoke(const Input& in, core::workflow::WorkflowContext&) const {
  auto& ctx=*s_ctx; blog::api::MutationStore mutations{ctx.store()};
  
  const auto one=ctx.store().find_by_resource("tiers", in.id.empty()?"1":in.id);
  if(one) return core::workflow::StepResponse<Output>{"{\"tiers\":[" + *one + "]}"};
  return core::workflow::StepResponse<Output>{R"({"tiers":[]})"};
 }
 void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}
 static blog::BlogContext* s_ctx;
};
blog::BlogContext* ExecuteStep::s_ctx{nullptr};
}
