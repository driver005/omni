export module blog_handlers:members_account_update_step;
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
export namespace blog::handlers::members::members_account::update::workflow {
class ExecuteStep {
public:
 using Input=interfaces::io::IRequest&; using Output=std::map<std::string, std::string>;
 static constexpr std::string_view name{"members_account_update_execute"};
 [[nodiscard]] core::workflow::StepResponse<Output> invoke(const Input& in, core::workflow::WorkflowContext&) const {
  auto& ctx=*s_ctx;
 const std::string id=req_member_id(in);
  if (id.empty())
   return core::workflow::StepResponse<Output>{{{"status","204"},{"body",""}}};
  const auto one=ctx.store().find_by_resource("members", id);
  if (!one)
   return core::workflow::StepResponse<Output>{{{"status","404"},{"body",blog::api::ApiError{}.json("Resource not found","Member not found")}}};
  const std::string raw{in.body_as_string()};
  const auto picked=pick(raw);
  if (picked.empty())
   return core::workflow::StepResponse<Output>{{{"status","422"},
     {"body",blog::api::ApiError{}.validation("Validation error, cannot save member.","Invalid body","data")}}};
  const auto result=ctx.store().edit("members", id, picked);
  if (result.status()>=400)
   return core::workflow::StepResponse<Output>{{{"status",std::to_string(result.status())},
     {"body",result.body()}}};
  s_ops->account_fields_set(id, raw);
  blog::api::members::members_support::emit_event("member.edited",
      std::format(R"({{"id":"{}","source":"portal"}}")", id), id);
  const auto after=ctx.store().find_by_resource("members", id).value_or(*one);
  return core::workflow::StepResponse<Output>{{{"status","200"},
    {"body",blog::api::members::MembersOperations::account_projection(after, raw)}}};
 }
 void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}
 static blog::BlogContext* s_ctx; static blog::api::members::MembersOperations* s_ops;

private:
static std::string req_member_id(const interfaces::io::IRequest& in) {
  return std::string{in.find_header("x-member-id")};
 }
static std::string pick(std::string_view raw) {
  const auto parsed=core_serde::JsonCodec{}.decode<serde::Value>(raw);
  if (!parsed) return {};
  const auto* object=std::get_if<serde::Value::Object>(&parsed->get());
  if (object==nullptr) return {};
  static const std::vector<std::string_view> writable{
    "name","expertise","subscribed","newsletters",
    "enable_comment_notifications","enable_updates_and_announcements"};
  std::string out="{";
  bool first=true;
  for (const auto& [name,value]:*object) {
   if (std::ranges::find(writable,name)==writable.end()) continue;
   if (!first) out+=",";
   first=false;
   out+="\""+name+"\":"+core_serde::JsonCodec{}.encode(value);
  }
  return out+"}";
 }
};
blog::api::members::MembersOperations* ExecuteStep::s_ops{nullptr};
blog::BlogContext* ExecuteStep::s_ctx{nullptr};
}