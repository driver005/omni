export module blog_handlers:members_destroy_step;
import :domain_all;
import std;
import core_workflow;
import interfaces;
import :members_operations;

// Strict-parity DELETE /members/:id (destroy). Ghost statusCode 204 (M:291);
// members.destroy({id},{require:true,cancelStripeSubscriptions:cancel}) emits
// member.deleted webhook and _MemberPaidSubscriptionEvent rows (MR:1225-1291).
export namespace blog::handlers::members::destroy::workflow {
namespace support=blog::api::members::members_support;
class ExecuteStep {
public:
 using Input=interfaces::io::IRequest&;
 using Output=std::map<std::string,std::string>;
 static constexpr std::string_view name{"members_destroy_execute"};
 [[nodiscard]] core::workflow::StepResponse<Output> invoke(const Input& in, core::workflow::WorkflowContext&) const {
  auto& ctx=*s_ctx;
  const std::string id=support::last_segment(in.get_path());
  if (id.empty())
   return core::workflow::StepResponse<Output>{{{"status","422"},
     {"body",support::invalid("Validation error, cannot destroy member.", "", "id")}}};
  const auto result=ctx.store().destroy("members", id);
  if (result.status()>=400)
   return core::workflow::StepResponse<Output>{{{"status",std::to_string(result.status())},
     {"body",result.body()}}};
  const auto p=support::params(in.get_path());
  const bool cancel=p.contains("cancel") && p.at("cancel")!="false";
  support::emit_webhook("member.deleted", std::format(R"({{"id":"{}"}})", id), id);
  support::emit_event("MemberDeletedEvent",
      std::format(R"({{"id":"{}","cancel_stripe_subscriptions":{}}})",
                  id, cancel ? "true" : "false"), id);
  return core::workflow::StepResponse<Output>{{{"status","204"},{"body",""}}};
 }
 void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}
 static blog::BlogContext* s_ctx;
};
blog::BlogContext* ExecuteStep::s_ctx{nullptr};
}
