export module blog_handlers:emails_analytics_put_step;
import std;
import core_workflow;
import blog_context;
import :mutation;
import :emails_state;
import blog_dto;
import :emails_analytics_put_model;
import :emails_analytics_put_dto;
export namespace blog::handlers::emails::analytics_put::workflow {
class ExecuteStep {
public:
 using Input=model::Request; using Output=std::map<std::string, std::string>;
 static constexpr std::string_view name{"emails_analytics_put_execute"};
 [[nodiscard]] core::workflow::StepResponse<Output> invoke(const Input& in, core::workflow::WorkflowContext&) const {
  auto& ctx=*s_ctx; blog::api::MutationStore mutations{ctx.store()};
  auto& state=*s_state;
  const auto r=state.emails_analytics_put("1", in.body);
  if(r.first!=0) return core::workflow::StepResponse<Output>{{{"status",std::to_string(r.first)},{"body",r.second}}};
  return core::workflow::StepResponse<Output>{{{"status","200"},{"body",mutations.edit("emails", in.id, in.body.empty()?"{}":in.body).body()}}};
 }
 void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}
 static blog::BlogContext* s_ctx; static blog::api::EmailsState* s_state;
};
blog::BlogContext* ExecuteStep::s_ctx{nullptr};
blog::api::EmailsState* ExecuteStep::s_state{nullptr};
}