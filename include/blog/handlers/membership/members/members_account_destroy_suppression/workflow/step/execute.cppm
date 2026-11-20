export module blog_handlers:members_account_destroy_suppression_step;
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
import core_serde;
import interfaces;
import :members_operations;
export namespace blog::handlers::members::members_account::destroy_suppression::workflow {
class ExecuteStep {
public:
 using Input=interfaces::io::IRequest&; using Output=std::map<std::string, std::string>;
 static constexpr std::string_view name{"members_account_destroy_suppression_execute"};
 [[nodiscard]] core::workflow::StepResponse<Output> invoke(const Input& in, core::workflow::WorkflowContext&) const {
  auto& ctx=*s_ctx;
 const std::string id=req_member_id(in);
  if (id.empty())
   return core::workflow::StepResponse<Output>{{{"status","204"},{"body",""}}};
  const auto one=ctx.store().find_by_resource("members", id);
  if (!one)
   return core::workflow::StepResponse<Output>{{{"status","404"},{"body",blog::api::ApiError{}.json("Resource not found","Member not found")}}};
  std::string email;
  const auto parsed=core_serde::JsonCodec{}.decode<serde::Value>(*one);
  if (parsed) if (const auto* object=std::get_if<serde::Value::Object>(&parsed->get()))
   for (const auto& [name,value]:*object)
    if (name=="email") if (const auto* text=std::get_if<std::string>(&value.get())) email=*text;
  // Ghost allowEmail removes the suppression unconditionally: an already-absent
  // entry is not an error (account-service.ts:235-238).
  if (!email.empty()) {
   (void)s_ops->remove_suppression(email);
   blog::api::members::members_support::emit_event("SuppressionRemoved",
       std::format(R"({{"id":"{}","email":"{}"}}")", id, email), id);
  }
  const auto result=ctx.store().edit("members", id, R"({"email_disabled":false})");
  if (result.status()!=200)
   return core::workflow::StepResponse<Output>{{{"status",std::to_string(result.status())},{"body",result.body()}}};
  return core::workflow::StepResponse<Output>{{{"status","204"},{"body",""}}};
 }
 void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}
 static blog::BlogContext* s_ctx; static blog::api::members::MembersOperations* s_ops;

private:
static std::string req_member_id(const interfaces::io::IRequest& in) {
  return std::string{in.find_header("x-member-id")};
 }
};
blog::api::members::MembersOperations* ExecuteStep::s_ops{nullptr};
blog::BlogContext* ExecuteStep::s_ctx{nullptr};
}