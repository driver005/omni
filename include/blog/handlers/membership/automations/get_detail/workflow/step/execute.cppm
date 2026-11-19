export module blog_handlers:automations_get_detail_step;
import :domain_all;
import std;
import core_workflow;
import interfaces;

export namespace blog::handlers::automations::get_detail::workflow {
class ExecuteStep {
public:
 using Input=interfaces::io::IRequest&; using Output=std::string;
 static constexpr std::string_view name{"automations_get_detail_execute"};
 [[nodiscard]] core::workflow::StepResponse<Output> invoke(const Input& in, core::workflow::WorkflowContext&) const {
  auto& ctx=*s_ctx; auto& mutations = ctx.store();
  
  const auto one=ctx.store().find_by_resource("automations", std::string{}.empty()?"1":std::string{});
  if(one) return core::workflow::StepResponse<Output>{"{\"automations\":[" + *one + "]}"};
  return core::workflow::StepResponse<Output>{R"({"automations":[]})"};
 }
 void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}
 static blog::BlogContext* s_ctx;
};
blog::BlogContext* ExecuteStep::s_ctx{nullptr};
}
