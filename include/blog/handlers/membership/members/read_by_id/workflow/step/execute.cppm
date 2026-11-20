export module blog_handlers:members_read_by_id_step;
import :domain_all;
import std;
import core_workflow;
import core_query;
import interfaces;
import :members_operations;

// Strict-parity GET /members/:id (read). Ghost data id/email, options include
// allowlist (M:81-92); memberBREADService.read with metafieldsFor:ADMIN
// (M:94-99); NotFoundError 'Member not found.' 404 (M:100-104).
export namespace blog::handlers::members::read_by_id::workflow {
namespace support=blog::api::members::members_support;
class ExecuteStep {
public:
 using Input=interfaces::io::IRequest&;
 using Output=std::map<std::string,std::string>;
 static constexpr std::string_view name{"members_read_by_id_execute"};
 [[nodiscard]] core::workflow::StepResponse<Output> invoke(const Input& in, core::workflow::WorkflowContext&) const {
  auto& ctx=*s_ctx;
  static constexpr std::string_view kOptions[]{"include"};
  const auto q=core::query::parse(in.get_path(), kOptions);
  if (!support::valid_include(q))
   return core::workflow::StepResponse<Output>{{{"status","422"},
     {"body",support::invalid(support::kNoIncludeMessage,
       support::include_value(q), "include")}}};
  const std::string id=support::last_segment(in.get_path());
  if (id.empty())
   return core::workflow::StepResponse<Output>{{{"status","422"},
     {"body",support::invalid("Validation error, cannot read member.", "", "id")}}};

  std::string member;
  if (const auto found=ctx.store().find_by_resource("members", id)) member=*found;
  else {
   // Ghost data.email lookup path (M:81, IS:86-90): resolve by email in the
   // store stand-in when the path id does not match a member.
   for (const auto& row : support::rows_of(ctx.store().json_for("members", {}))) {
    if (const auto* object=std::get_if<support::Object>(&row.get())) {
     if (support::scalar(object, "email")==id || support::scalar(object, "uuid")==id) {
      member=support::encode(*object); break;
     }
    }
   }
  }
  if (member.empty())
   return core::workflow::StepResponse<Output>{{{"status","404"},
     {"body",support::not_found("Member not found.")}}};
  return core::workflow::StepResponse<Output>{{{"status","200"},
    {"body",support::wrap("members", "["+member+"]")}}};
 }
 void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}
 static blog::BlogContext* s_ctx;
};
blog::BlogContext* ExecuteStep::s_ctx{nullptr};
}
