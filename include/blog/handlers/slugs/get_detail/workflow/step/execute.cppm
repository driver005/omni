export module blog_handlers:slugs_get_detail_step;
import std;
import core_workflow;
import blog_context;
import :mutation;

import blog_dto;
import :slugs_get_detail_model;
import :slugs_get_detail_dto;
export namespace blog::handlers::slugs::get_detail::workflow {
class ExecuteStep {
public:
 using Input=model::Request; using Output=std::string;
 static constexpr std::string_view name{"slugs_get_detail_execute"};
 [[nodiscard]] core::workflow::StepResponse<Output> invoke(const Input& in, core::workflow::WorkflowContext&) const {
  auto& ctx=*s_ctx; blog::api::MutationStore mutations{ctx.store()};
  
  const auto one=ctx.store().find_by_resource("slugs", in.id.empty()?"1":in.id);
  if(one) return core::workflow::StepResponse<Output>{"{\"slugs\":[" + *one + "]}"};
  return core::workflow::StepResponse<Output>{R"({"slugs":[]})"};
 }
 void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}
 static blog::BlogContext* s_ctx;
};
blog::BlogContext* ExecuteStep::s_ctx{nullptr};
}
