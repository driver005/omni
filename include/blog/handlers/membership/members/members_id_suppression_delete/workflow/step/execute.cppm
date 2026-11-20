export module blog_handlers:members_members_id_suppression_delete_step;
import :domain_all;
import std;
import core_workflow;
import core_serde;
import interfaces;
import :members_operations;

// Strict-parity DELETE /members/:id/suppression (deleteEmailSuppression).
// Ghost: read member 404 -> emailSuppressionList.removeEmail(email) (false =>
// 500 InternalServerError) -> edit {email_disabled:false} -> 204 (M:512-543).
export namespace blog::handlers::members::members_id_suppression_delete::workflow {
namespace support=blog::api::members::members_support;
class ExecuteStep {
public:
 using Input=interfaces::io::IRequest&;
 using Output=std::map<std::string,std::string>;
 static constexpr std::string_view name{"members_members_id_suppression_delete_execute"};
 [[nodiscard]] core::workflow::StepResponse<Output> invoke(const Input& in, core::workflow::WorkflowContext&) const {
  auto& ctx=*s_ctx;
  const std::string id=support::segment_before(in.get_path(), "/suppression");
  if (id.empty())
   return core::workflow::StepResponse<Output>{{{"status","422"},
     {"body",support::invalid("Validation error, cannot delete suppression.", "", "id")}}};
  const auto member=support::member_row(ctx, id);
  if (!member)
   return core::workflow::StepResponse<Output>{{{"status","404"},
     {"body",support::not_found("Member not found.")}}};
  const auto email=support::scalar(&*member, "email");
  // The suppression-list removal is an outbound side effect: dispatching it
  // through the outbox is the C++ stand-in for Mailgun + Suppression.destroy.
  support::emit("suppression.remove", std::format(R"({{"email":"{}"}})", email), id);
  const bool removed=std::ranges::any_of(support::side_effects(), [&](const auto& event) {
   return event.type=="suppression.remove" && event.aggregate_id==id;
  });
  if (!removed)
   return core::workflow::StepResponse<Output>{{{"status","500"},
     {"body",support::bad_request("Failed to remove email suppression.")}}};
  const auto edited=ctx.store().edit("members", id, R"({"email_disabled":false})");
  if (edited.status()>=400)
   return core::workflow::StepResponse<Output>{{{"status",std::to_string(edited.status())},
     {"body",edited.body()}}};
  return core::workflow::StepResponse<Output>{{{"status","204"},{"body",""}}};
 }
 void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}
 static blog::BlogContext* s_ctx;
};
blog::BlogContext* ExecuteStep::s_ctx{nullptr};
}
