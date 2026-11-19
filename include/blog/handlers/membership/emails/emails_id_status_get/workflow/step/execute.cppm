export module blog_handlers:emails_emails_id_status_get_step;
import :domain_all;
import std;
import core_workflow;
import interfaces;

export namespace blog::handlers::emails::emails_id_status_get::workflow {
class ExecuteStep {
public:
 using Input=interfaces::io::IRequest&; using Output=std::string;
 static constexpr std::string_view name{"emails_emails_id_status_get_execute"};
 [[nodiscard]] core::workflow::StepResponse<Output> invoke(const Input& in, core::workflow::WorkflowContext&) const {
  auto& ctx=*s_ctx; auto& mutations = ctx.store();
  
  const auto one=ctx.store().find_by_resource("emails", std::string{}.empty()?"1":std::string{});
  if(one) return core::workflow::StepResponse<Output>{"{\"emails\":[" + *one + "]}"};
  return core::workflow::StepResponse<Output>{R"({"emails":[]})"};
 }
 void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}
 static blog::BlogContext* s_ctx;
};
blog::BlogContext* ExecuteStep::s_ctx{nullptr};
}
