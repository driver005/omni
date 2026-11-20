export module blog_handlers:members_create_step;
import :domain_all;
import std;
import core_workflow;
import core_serde;
import interfaces;
import :members_operations;

// Strict-parity POST /members (add). Ghost: data.members[0] with required email
// (M:116-120); options send_email/email_type, email_type in
// {signin,signup,subscribe} (M:115,120-124); verificationTrigger may force
// send_email=false (M:128-131); memberBREADService.add emits MemberCreatedEvent,
// `member.added` webhook, optional magic-link mail and comp subscription
// (BS:527-621, MR:585-840); statusCode 201 (M:111).
export namespace blog::handlers::members::create::workflow {
namespace support=blog::api::members::members_support;
class ExecuteStep {
public:
 using Input=interfaces::io::IRequest&;
 using Output=std::map<std::string,std::string>;
 static constexpr std::string_view name{"members_create_execute"};
 [[nodiscard]] core::workflow::StepResponse<Output> invoke(const Input& in, core::workflow::WorkflowContext&) const {
  namespace support=blog::api::members::members_support;
  auto& ctx=*s_ctx;
  const auto p=support::params(in.get_path());
  if (const auto it=p.find("email_type"); it!=p.end() && !it->second.empty()
      && !in_set({"signin","signup","subscribe"}, it->second))
   return core::workflow::StepResponse<Output>{{{"status","422"},
     {"body",support::invalid("Validation error, cannot save member.", it->second, "email_type")}}};
  auto inner=unwrap(in.body_as_string());
  const auto parsed=support::decode(inner);
  const auto* object=parsed ? std::get_if<support::Object>(&parsed->get()) : nullptr;
  const auto email=object ? support::scalar(object, "email") : std::string{};
  if (email.empty() || !email.contains('@'))
   return core::workflow::StepResponse<Output>{{{"status","422"},
     {"body",support::invalid("Validation error, cannot save member.", email, "email")}}};
  // Ghost memberAlreadyExists (BS:559-568): unique email -> ValidationError.
  for (const auto& row : support::rows_of(ctx.store().json_for("members", {}))) {
   if (const auto* existing=std::get_if<support::Object>(&row.get())) {
    if (support::scalar(existing, "email")==email)
     return core::workflow::StepResponse<Output>{{{"status","422"},
       {"body",support::invalid("A member with this email already exists.", email, "email")}}};
   }
  }
  const auto result=ctx.store().create("members", inner);
  if (result.status()>=400)
   return core::workflow::StepResponse<Output>{{{"status",std::to_string(result.status())},
     {"body",result.body()}}};
  // Observable side effects (CONTRACTS.md §4): domain event, webhook, optional mail.
  support::emit_event("MemberCreatedEvent", inner, email);
  support::emit_webhook("member.added", result.body(), email);
  if (const auto it=p.find("send_email"); it==p.end() || it->second!="false")
   support::emit_mail("magic_link", std::format(R"({{"email":"{}","type":"{}"}})",
       email, p.contains("email_type") ? p.at("email_type") : std::string{"signin"}), email);
  return core::workflow::StepResponse<Output>{{{"status",std::to_string(result.status())},
    {"body",result.body()}}};
 }
 void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}
 static blog::BlogContext* s_ctx;
private:
 static std::string unwrap(std::string_view raw) {
  if (raw.empty()) return "{}";
  const auto parsed=blog::api::members::members_support::decode(raw);
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
 static bool in_set(std::initializer_list<std::string_view> values, std::string_view value) {
  return std::ranges::find(values,value)!=values.end();
 }
};
blog::BlogContext* ExecuteStep::s_ctx{nullptr};
}
