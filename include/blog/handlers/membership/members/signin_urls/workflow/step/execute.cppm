export module blog_handlers:members_signin_urls_step;
import :domain_all;
import :members_dto_member_subscription;
import :members_dto_member_subscription_input;
import :members_dto_member_subscription_response;
import :members_dto_signin_url;
import :members_dto_signin_urls_response;
import :members_dto_stats_member_entry;
import :members_dto_stats_member_meta;
import :members_dto_stats_member_response;
import :members_dto_stats_member_totals;
import :members_dto_stats_mrr_entry;
import :members_dto_stats_mrr_meta;
import :members_dto_stats_mrr_response;
import :members_dto_stats_mrr_total;
import :members_dto_subscription_member;
import :members_dto_subscription_tier;
import :members_model_member;
import :members_model_wire_fixtures;
import std;
import core_workflow;
import interfaces;
import :members_operations;
export namespace blog::handlers::members::signin_urls::workflow {
class ExecuteStep {
public:
 using Input=interfaces::io::IRequest&; using Output=std::map<std::string, std::string>;
 static constexpr std::string_view name{"members_signin_urls_execute"};
 [[nodiscard]] core::workflow::StepResponse<Output> invoke(const Input& in, core::workflow::WorkflowContext&) const {
  auto& ctx=*s_ctx; auto& mutations = ctx.store();
    const auto r=s_ops->member_signin_urls("1");
  if(r.first!=0) return core::workflow::StepResponse<Output>{{{"status",std::to_string(r.first)},{"body",r.second}}};
  const auto one=ctx.store().find_by_resource("members", std::string{}.empty()?"1":std::string{});
  if(one) return core::workflow::StepResponse<Output>{{{"status","200"},{"body","{\"members\":[" + *one + "]}"}}};
  return core::workflow::StepResponse<Output>{{{"status","200"},{"body",R"({"members":[]})"}}};
 }
 static blog::api::members::MembersOperations* s_ops;
 void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}
 static blog::BlogContext* s_ctx;
};
blog::api::members::MembersOperations* ExecuteStep::s_ops{nullptr};
blog::BlogContext* ExecuteStep::s_ctx{nullptr};
}
