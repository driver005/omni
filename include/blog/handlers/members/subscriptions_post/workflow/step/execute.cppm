export module blog_handlers:members_subscriptions_post_step;
import std;
import core_workflow;
import core_cache;
import blog_context;
import :mutation;
import :members_operations;
import blog_dto;
import :members_subscriptions_post_model;
import :members_subscriptions_post_dto;
export namespace blog::handlers::members::subscriptions_post::workflow {
class ExecuteStep {
public:
 using Input=model::Request; using Output=std::map<std::string, std::string>;
 static constexpr std::string_view name{"members_subscriptions_post_execute"};
 [[nodiscard]] core::workflow::StepResponse<Output> invoke(const Input& in, core::workflow::WorkflowContext&) const {
  auto& ctx=*s_ctx; blog::api::MutationStore mutations{ctx.store()};
    const auto r=blog::api::members::member_subscription_create(*s_cache, "1", in.body);
  if(r.first!=0) return core::workflow::StepResponse<Output>{{{"status",std::to_string(r.first)},{"body",r.second}}};
  return core::workflow::StepResponse<Output>{{{"status","200"},{"body",mutations.create("members", in.body.empty()?"{}":in.body).body()}}};
 }
 void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}
 static blog::BlogContext* s_ctx; static core::cache::Cache* s_cache;
};
blog::BlogContext* ExecuteStep::s_ctx{nullptr};
core::cache::Cache* ExecuteStep::s_cache{nullptr};
}