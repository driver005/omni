export module blog_handlers:members_members_id_commenting_disable_post_step;
import std;
import core_workflow;
import blog_context;
import :mutation;
import :members_state;
import blog_dto;
import :members_members_id_commenting_disable_post_model;
import :members_members_id_commenting_disable_post_dto;
export namespace blog::handlers::members::members_id_commenting_disable_post::workflow {
class ExecuteStep {
public:
 using Input=model::Request; using Output=std::map<std::string, std::string>;
 static constexpr std::string_view name{"members_members_id_commenting_disable_post_execute"};
 [[nodiscard]] core::workflow::StepResponse<Output> invoke(const Input& in, core::workflow::WorkflowContext&) const {
  auto& ctx=*s_ctx; blog::api::MutationStore mutations{ctx.store()};
  auto& state=*s_state;
  const auto r=state.commenting_set("1", in.body, true);
  if(r.first!=0) return core::workflow::StepResponse<Output>{{{"status",std::to_string(r.first)},{"body",r.second}}};
  return core::workflow::StepResponse<Output>{{{"status","200"},{"body",mutations.create("members", in.body.empty()?"{}":in.body).body()}}};
 }
 void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}
 static blog::BlogContext* s_ctx; static blog::api::MembersState* s_state;
};
blog::BlogContext* ExecuteStep::s_ctx{nullptr};
blog::api::MembersState* ExecuteStep::s_state{nullptr};
}