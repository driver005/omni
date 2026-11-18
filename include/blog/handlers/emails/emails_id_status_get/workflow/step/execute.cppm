export module blog_handlers:emails_emails_id_status_get_step;
import std;
import core_workflow;
import blog_context;
import :mutation;

import blog_dto;
import :emails_emails_id_status_get_model;
import :emails_emails_id_status_get_dto;
export namespace blog::handlers::emails::emails_id_status_get::workflow {
class ExecuteStep {
public:
 using Input=model::Request; using Output=std::string;
 static constexpr std::string_view name{"emails_emails_id_status_get_execute"};
 [[nodiscard]] core::workflow::StepResponse<Output> invoke(const Input& in, core::workflow::WorkflowContext&) const {
  auto& ctx=*s_ctx; blog::api::MutationStore mutations{ctx.store()};
  
  const auto one=ctx.store().find_by_resource("emails", in.id.empty()?"1":in.id);
  if(one) return core::workflow::StepResponse<Output>{"{\"emails\":[" + *one + "]}"};
  return core::workflow::StepResponse<Output>{R"({"emails":[]})"};
 }
 void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}
 static blog::BlogContext* s_ctx;
};
blog::BlogContext* ExecuteStep::s_ctx{nullptr};
}
