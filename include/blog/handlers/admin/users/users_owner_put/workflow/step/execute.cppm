export module blog_handlers:users_users_owner_put_step;
import :domain_all;
import :users_dto_subresources;
import std;
import core_workflow;
import interfaces;
import core_auth;
import :outbox_shim;
import blog_handlers_middleware;

export namespace blog::handlers::users::users_owner_put::workflow {

namespace local_mapping {
[[nodiscard]] inline std::string wrap(std::string_view resource, std::string_view rows,
                                      std::string_view meta = {})
{
    return "{\"" + std::string{resource} + "\":" + std::string{rows} +
           (meta.empty() ? std::string{} : ",\"meta\":" + std::string{meta}) + "}";
}
[[nodiscard]] inline std::string clean(std::string_view json, std::string_view) { return std::string{json}; }
[[nodiscard]] inline std::string pagination(std::size_t total, std::size_t limit, std::size_t page,
                                            std::size_t pages, std::string_view prev,
                                            std::string_view next)
{
    return std::format(R"("total":{},"limit":{},"page":{},"pages":{},"prev":{},"next":{})",
                       total, limit, page, pages,
                       prev.empty() ? std::string{"null"} : std::string{prev},
                       next.empty() ? std::string{"null"} : std::string{next});
}
} // namespace local_mapping
class ExecuteStep {
public:
 using Input=interfaces::io::IRequest&;
 using Output=std::map<std::string,std::string>;
 static constexpr std::string_view name{"users_users_owner_put_execute"};
 [[nodiscard]] core::workflow::StepResponse<Output> invoke(const Input& in, core::workflow::WorkflowContext&) const {
  auto& ctx=*s_ctx;
  const auto top=blog::api::JsonBody{}.parse_raw(in.body_as_string().empty()?"{}":in.body_as_string());
  const auto wrapper=top.find("owner");
  if (wrapper==top.end()) return validation("owner[0] is required","owner");
  const auto entries=blog::api::parse_array(wrapper->second);
  if (entries.empty()) return validation("owner[0] is required","owner");
  const auto& owner=entries.front();
  auto id=field(owner,"id");
  if (id.empty()) id=field(owner,"user_id");
  if (id.empty()) return validation("An owner id is required","id");

  // Ghost endpoint permission: `canThis(context).assign.role(ownerRole)` —
  // only a current Owner (or a privileged context) may assign the Owner role.
  const auto principal=make_principal(in);
  const auto authorizer=core::auth::ghost::GhostAuthorizer{};
  const auto decision=authorizer.authorize(principal, "roles", "edit", id);
  if (!decision.allowed) {
   return core::workflow::StepResponse<Output>{{{"status",std::to_string(decision.status)},
    {"body",decision.error_json}}};
  }
  if (!privileged(principal)) {
   return core::workflow::StepResponse<Output>{{{"status","403"},
    {"body",blog::api::ApiError{}.validation(
     "Only an Owner can transfer the Owner role","Forbidden","","NoPermissionError")}}};
  }

  // Target lookup + NotFoundError semantics of User.transferOwnership.
  const auto target=ctx.store().find_by_resource("users", id);
  if (!target) {
   return core::workflow::StepResponse<Output>{{{"status","404"},
    {"body",blog::api::ApiError{}.json("User not found.","Not Found")}}};
  }

  // Role pivot swap (current Owner -> Administrator, target -> Owner) is recorded
  // as a domain event on the shared outbox; the user record itself is merged so
  // the serializer returns the transferred owner model.
  const auto swapped="{\"roles\":\"Owner\"}";
  const auto mutation=ctx.store().edit("users", id, swapped);
  if (!mutation.ok()) {
   return core::workflow::StepResponse<Output>{{{"status",std::to_string(mutation.status())},
    {"body",mutation.body()}}};
  }
  blog::handlers::outbox::global_outbox().emit(blog::handlers::outbox::Event{
   .type="user.ownership_transferred",
   .payload=std::format(R"({{"from":{},"to":{},"role":"Owner"}})",
    blog::api::JsonBody{}.quote(principal.id.empty()?std::string{"1"}:principal.id),
    blog::api::JsonBody{}.quote(id)),
   .aggregate_id=id});

  const auto edited=ctx.store().find_by_resource("users", id);
  const auto model=edited?local_mapping::clean(*edited,"user-wire"):std::string{"{}"};
  const int status=mutation.status()==201?200:mutation.status();
  return core::workflow::StepResponse<Output>{{{"status",std::to_string(status)},
   {"body",local_mapping::wrap("users", model)}}};
 }
 void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}
 static blog::BlogContext* s_ctx;
private:
 static core::workflow::StepResponse<Output> validation(std::string_view message, std::string_view property) {
  return core::workflow::StepResponse<Output>{{{"status","422"},
   {"body",blog::api::ApiError{}.validation("Validation error",message,property)}}};
 }
 static bool privileged(const core::auth::ghost::Principal& principal) {
  using core::auth::ghost::ApiContext;
  return principal.context==ApiContext::INTERNAL || principal.context==ApiContext::SYSTEM ||
         principal.role=="Owner" || principal.role=="Administrator";
 }
 static core::auth::ghost::Principal make_principal(interfaces::io::IRequest& in) {
  auto principal=blog::handlers::middleware::principal_from_request(in);
  auto subject=std::string{in.find_header("x-blog-user-id")};
  if (subject.empty() && !in.get_authorization().empty()) subject="1";
  principal.id=std::move(subject);
  return principal;
 }
 static std::string field(const blog::api::RecordDto& record, std::string_view key) {
  const auto it=record.find(std::string{key});
  return it==record.end()?std::string{}:it->second;
 }
};
blog::BlogContext* ExecuteStep::s_ctx{nullptr};
}
