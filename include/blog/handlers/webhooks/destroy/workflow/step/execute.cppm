export module blog_handlers:webhooks_destroy_step;
import std;
import core_workflow;
import interfaces;
import blog_context;

import blog_dto;
export namespace blog::handlers::webhooks::destroy::workflow {
class ExecuteStep {
public:
 using Input=interfaces::io::IRequest&; using Output=std::string;
 static constexpr std::string_view name{"webhooks_destroy_execute"};
 [[nodiscard]] core::workflow::StepResponse<Output> invoke(const Input& in, core::workflow::WorkflowContext&) const {
  auto& ctx=*s_ctx; auto& mutations = ctx.store();
  
  const auto result = [&]{ const std::string p{in.get_path()}; const auto sl=p.find_last_of('/'); const auto tail=sl==std::string::npos?p:p.substr(sl+1); return (!tail.empty() && tail!="webhooks")?tail:std::string{}; }().empty()
      ? mutations.bulk_destroy("webhooks", in.body_as_string().empty() ? "{}" : in.body_as_string())
      : mutations.destroy("webhooks", [&]{ const std::string p{in.get_path()}; const auto sl=p.find_last_of('/'); const auto tail=sl==std::string::npos?p:p.substr(sl+1); return (!tail.empty() && tail!="webhooks")?tail:std::string{}; }());
  return core::workflow::StepResponse<Output>{result.body()};
 }
 void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}
 static blog::BlogContext* s_ctx;
};
blog::BlogContext* ExecuteStep::s_ctx{nullptr};
}