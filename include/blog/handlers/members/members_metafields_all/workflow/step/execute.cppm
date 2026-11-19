export module blog_handlers:members_members_metafields_all_step;
import std;
import core_workflow;
import blog_context;
import :mutation;
import :members_operations;
import blog_dto;
import :members_members_metafields_all_model;
import :members_members_metafields_all_dto;
export namespace blog::handlers::members::members_metafields_all::workflow {
class ExecuteStep {
public:
 using Input=model::Request; using Output=std::map<std::string, std::string>;
 static constexpr std::string_view name{"members_members_metafields_all_execute"};
 [[nodiscard]] core::workflow::StepResponse<Output> invoke(const Input& in, core::workflow::WorkflowContext&) const {
  auto& ctx=*s_ctx; blog::api::MutationStore mutations{ctx.store()};
    
  const auto r=blog::api::members::metafields_browse();
  if(r.first!=0) return core::workflow::StepResponse<Output>{{{"status",std::to_string(r.first)},{"body",r.second}}};
 }
 void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}
 static blog::BlogContext* s_ctx; };
blog::BlogContext* ExecuteStep::s_ctx{nullptr};
}