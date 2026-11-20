export module blog_handlers:members_edit_step;
import :domain_all;
import std;
import core_workflow;
import core_serde;
import interfaces;
import :members_operations;

// Strict-parity PUT /members/:id (edit). Ghost statusCode:200 (M:142);
// memberBREADService.edit -> memberRepository.update emits `member.edited`
// (BS:623-681, MR:850-...); the C++ store's wrap() returns 201 for a members
// edit (context.cppm:1423-1431), so the step remaps it to Ghost's 200.
export namespace blog::handlers::members::edit::workflow {
namespace support=blog::api::members::members_support;
class ExecuteStep {
public:
 using Input=interfaces::io::IRequest&;
 using Output=std::map<std::string,std::string>;
 static constexpr std::string_view name{"members_edit_execute"};
 [[nodiscard]] core::workflow::StepResponse<Output> invoke(const Input& in, core::workflow::WorkflowContext&) const {
  auto& ctx=*s_ctx;
  const std::string id=support::last_segment(in.get_path());
  if (id.empty())
   return core::workflow::StepResponse<Output>{{{"status","422"},
     {"body",support::invalid("Validation error, cannot save member.", "", "id")}}};
  const auto inner=unwrap(in.body_as_string());
  const auto result=ctx.store().edit("members", id, inner);
  const int status=result.status();
  if (status>=400)
   return core::workflow::StepResponse<Output>{{{"status",std::to_string(status)},
     {"body",result.body()}}};
  support::emit_event("member.edited", result.body(), id);
  support::emit_webhook("member.edited", result.body(), id);
  return core::workflow::StepResponse<Output>{{{"status","200"},{"body",result.body()}}};
 }
 void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}
 static blog::BlogContext* s_ctx;
private:
 static std::string unwrap(std::string_view raw) {
  if (raw.empty()) return "{}";
  const auto parsed=support::decode(raw);
  if (!parsed) return std::string{raw};
  const auto* root=std::get_if<support::Object>(&parsed->get());
  if (root==nullptr) return std::string{raw};
  const auto* data=support::value_for(root,"data");
  if (data==nullptr) return std::string{raw};
  const auto* data_object=std::get_if<support::Object>(&data->get());
  if (data_object==nullptr) return std::string{raw};
  const auto* members=support::value_for(data_object,"members");
  if (members==nullptr) return std::string{raw};
  const auto* array=std::get_if<support::Array>(&members->get());
  if (array==nullptr || array->empty()) return std::string{raw};
  const auto* member=std::get_if<support::Object>(&array->front().get());
  return member==nullptr?std::string{raw}:support::encode(*member);
 }
};
blog::BlogContext* ExecuteStep::s_ctx{nullptr};
}
