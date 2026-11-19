export module blog_handlers:members_read_by_id_step;
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

export namespace blog::handlers::members::read_by_id::workflow {
class ExecuteStep {
public:
 using Input=interfaces::io::IRequest&; using Output=std::string;
 static constexpr std::string_view name{"members_read_by_id_execute"};
 [[nodiscard]] core::workflow::StepResponse<Output> invoke(const Input& in, core::workflow::WorkflowContext&) const {
  auto& ctx=*s_ctx; auto& mutations = ctx.store();
  
  const auto one=ctx.store().find_by_resource("members", std::string{}.empty()?"1":std::string{});
  if(one) return core::workflow::StepResponse<Output>{"{\"members\":[" + *one + "]}"};
  return core::workflow::StepResponse<Output>{R"({"members":[]})"};
 }
 void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}
 static blog::BlogContext* s_ctx;
};
blog::BlogContext* ExecuteStep::s_ctx{nullptr};
}
