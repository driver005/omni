export module blog_handlers:members_signin_urls_step;
import std;
import core_workflow;
import blog_context;
import :mutation;
import :members_operations;
import blog_dto;
import :members_signin_urls_model;
import :members_signin_urls_dto;
export namespace blog::handlers::members::signin_urls::workflow {
class ExecuteStep {
public:
 using Input=model::Request; using Output=std::map<std::string, std::string>;
 static constexpr std::string_view name{"members_signin_urls_execute"};
 [[nodiscard]] core::workflow::StepResponse<Output> invoke(const Input& in, core::workflow::WorkflowContext&) const {
  auto& ctx=*s_ctx; blog::api::MutationStore mutations{ctx.store()};
    const auto r=blog::api::members::member_signin_urls("1");
  if(r.first!=0) return core::workflow::StepResponse<Output>{{{"status",std::to_string(r.first)},{"body",r.second}}};
  const auto one=ctx.store().find_by_resource("members", in.id.empty()?"1":in.id);
  if(one) return core::workflow::StepResponse<Output>{{{"status","200"},{"body","{\"members\":[" + *one + "]}"}}};
  return core::workflow::StepResponse<Output>{{{"status","200"},{"body",R"({"members":[]})"}}};
 }
 void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}
 static blog::BlogContext* s_ctx; };
blog::BlogContext* ExecuteStep::s_ctx{nullptr};
}
