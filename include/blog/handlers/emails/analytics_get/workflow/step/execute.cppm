export module blog_handlers:emails_analytics_get_step;
import std;
import core_workflow;
import blog_context;
import :mutation;
import :special;
import blog_dto;
import :emails_analytics_get_model;
import :emails_analytics_get_dto;
export namespace blog::handlers::emails::analytics_get::workflow {
class ExecuteStep {
public:
 using Input=model::Request; using Output=std::string;
 static constexpr std::string_view name{"emails_analytics_get_execute"};
 [[nodiscard]] core::workflow::StepResponse<Output> invoke(const Input& in, core::workflow::WorkflowContext&) const {
  auto& ctx=*s_ctx; blog::api::MutationStore mutations{ctx.store()};
  auto& special=*s_special;
  const auto r=special.emails_analytics_get("1");
  if(r.status!=0) return core::workflow::StepResponse<Output>{r.body};
  const auto one=ctx.store().find_by_resource("emails", in.id.empty()?"1":in.id);
  if(one) return core::workflow::StepResponse<Output>{"{\"emails\":[" + *one + "]}"};
  return core::workflow::StepResponse<Output>{R"({"emails":[]})"};
 }
 void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}
 static blog::BlogContext* s_ctx; static blog::api::SpecialEndpoints* s_special;
};
blog::BlogContext* ExecuteStep::s_ctx{nullptr};
blog::api::SpecialEndpoints* ExecuteStep::s_special{nullptr};
}
