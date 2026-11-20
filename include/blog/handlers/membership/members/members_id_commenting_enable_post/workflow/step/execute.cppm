export module blog_handlers:members_members_id_commenting_enable_post_step;
import :domain_all;
import std;
import core_workflow;
import interfaces;
import :members_operations;

// Strict-parity POST /members/:id/commenting/enable. Ghost: option id required,
// saveCommenting('commenting_enabled') audit action, re-read; output
// {members:[member]}; X-Cache-Invalidate: /members/.
export namespace blog::handlers::members::members_id_commenting_enable_post::workflow {
namespace support=blog::api::members::members_support;
class ExecuteStep {
public:
 using Input=interfaces::io::IRequest&;
 using Output=std::map<std::string,std::string>;
 static constexpr std::string_view name{"members_members_id_commenting_enable_post_execute"};
 [[nodiscard]] core::workflow::StepResponse<Output> invoke(const Input& in, core::workflow::WorkflowContext&) const {
  auto& ctx=*s_ctx;
  const std::string id=support::segment_before(in.get_path(), "/commenting");
  if (id.empty() || !ctx.store().find_by_resource("members", id))
   return core::workflow::StepResponse<Output>{{{"status","404"},
     {"body",support::not_found("Member not found.")}}};
  const auto r=s_ops->commenting_set(id, in.body_as_string(), false);
  support::emit_event("commenting_enabled",
      std::format(R"({{"id":"{}"}})", id), id);
  const auto member=support::member_json(ctx, id);
  const auto out=member.empty()?r.second
    :support::wrap("members", "["+s_ops->apply_commenting(id, member)+"]");
  return core::workflow::StepResponse<Output>{{{"status",std::to_string(r.first)},{"body",out}}};
 }
 static blog::BlogContext* s_ctx; static blog::api::members::MembersOperations* s_ops;
 void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}
};
blog::api::members::MembersOperations* ExecuteStep::s_ops{nullptr};
blog::BlogContext* ExecuteStep::s_ctx{nullptr};
}
