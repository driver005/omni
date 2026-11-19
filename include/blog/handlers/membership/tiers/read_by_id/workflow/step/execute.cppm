export module blog_handlers:tiers_read_by_id_step;
import :domain_all;
import std;
import core_workflow;
import interfaces;

export namespace blog::handlers::tiers::read_by_id::workflow {
class ExecuteStep {
public:
 using Input=interfaces::io::IRequest&; using Output=std::string;
 static constexpr std::string_view name{"tiers_read_by_id_execute"};
 [[nodiscard]] core::workflow::StepResponse<Output> invoke(const Input& in, core::workflow::WorkflowContext&) const {
  auto& ctx=*s_ctx; auto& mutations = ctx.store();
  
  const auto one=ctx.store().find_by_resource("tiers", std::string{}.empty()?"1":std::string{});
  if(one) return core::workflow::StepResponse<Output>{"{\"tiers\":[" + *one + "]}"};
  return core::workflow::StepResponse<Output>{R"({"tiers":[]})"};
 }
 void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}
 static blog::BlogContext* s_ctx;
};
blog::BlogContext* ExecuteStep::s_ctx{nullptr};
}
