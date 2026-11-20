export module blog_handlers:members_members_id_sessions_delete_step;
import :domain_all;
import std;
import core_workflow;
import interfaces;
import :members_operations;

// Strict-parity DELETE /members/:id/sessions (logout). Ghost 204 and
// memberBREADService.logout -> memberRepository.cycleTransientId rotates the
// transient id, invalidating sessions (M:181-185, BS:788-790, MR:556-...).
export namespace blog::handlers::members::members_id_sessions_delete::workflow {
namespace support=blog::api::members::members_support;
class ExecuteStep {
public:
 using Input=interfaces::io::IRequest&;
 using Output=std::map<std::string,std::string>;
 static constexpr std::string_view name{"members_members_id_sessions_delete_execute"};
 [[nodiscard]] core::workflow::StepResponse<Output> invoke(const Input& in, core::workflow::WorkflowContext&) const {
  auto& ctx=*s_ctx;
  const std::string id=support::segment_before(in.get_path(), "/sessions");
  if (id.empty())
   return core::workflow::StepResponse<Output>{{{"status","422"},
     {"body",support::invalid("Validation error, cannot logout member.", "", "id")}}};
  const auto member=ctx.store().find_by_resource("members", id);
  if (!member)
   return core::workflow::StepResponse<Output>{{{"status","404"},
     {"body",support::not_found("Member not found.")}}};
  // Session invalidation is the route's core side effect: cycle the transient
  // id as an observable outbox event (no member deletion, no external service).
  support::emit_event("MemberSessionInvalidated",
      std::format(R"({{"id":"{}","transient_id_cycled":true}})", id), id);
  return core::workflow::StepResponse<Output>{{{"status","204"},{"body",""}}};
 }
 void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}
 static blog::BlogContext* s_ctx;
};
blog::BlogContext* ExecuteStep::s_ctx{nullptr};
}
