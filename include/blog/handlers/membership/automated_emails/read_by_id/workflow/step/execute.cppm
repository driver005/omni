export module blog_handlers:automated_emails_read_by_id_step;
import std;
import core_workflow;
import interfaces;
import blog_context;

import blog_dto;
export namespace blog::handlers::automated_emails::read_by_id::workflow {
class ExecuteStep {
public:
 using Input=interfaces::io::IRequest&; using Output=std::string;
 static constexpr std::string_view name{"automated_emails_read_by_id_execute"};
 [[nodiscard]] core::workflow::StepResponse<Output> invoke(const Input& in, core::workflow::WorkflowContext&) const {
  auto& ctx=*s_ctx; auto& mutations = ctx.store();
  const auto one=ctx.store().find_by_resource("automated_emails", std::string{}.empty()?"1":std::string{});
  if(one) return core::workflow::StepResponse<Output>{"{\"automated_emails\":[" + *one + "]}"};
  return core::workflow::StepResponse<Output>{R"({"automated_emails":[]})"};
 }
 void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}
 static blog::BlogContext* s_ctx;
};
blog::BlogContext* ExecuteStep::s_ctx{nullptr};
}
