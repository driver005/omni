export module blog_handlers:users_users_password_put_step;
import :domain_all;
import :users_dto_subresources;
import std;
import core_workflow;
import interfaces;
import core_auth;
import :outbox_shim;
import blog_handlers_middleware;

export namespace blog::handlers::users::users_password_put::workflow {
class ExecuteStep {
public:
 using Input=interfaces::io::IRequest&;
 using Output=std::map<std::string,std::string>;
 static constexpr std::string_view name{"users_users_password_put_execute"};
 [[nodiscard]] core::workflow::StepResponse<Output> invoke(const Input& in, core::workflow::WorkflowContext&) const {
  auto& ctx=*s_ctx;
  const auto top=blog::api::JsonBody{}.parse_raw(in.body_as_string().empty()?"{}":in.body_as_string());
  const auto wrapper=top.find("password");
  if (wrapper==top.end()) {
   return validation("password[0] is required","password");
  }
  const auto entries=blog::api::parse_array(wrapper->second);
  if (entries.empty()) {
   return validation("password[0] is required","password");
  }
  const auto& entry=entries.front();
  const auto user_id=field(entry,"user_id");
  const auto new_password=field(entry,"newPassword");
  const auto repeat=field(entry,"ne2Password");
  if (user_id.empty()) return validation("user_id is required","user_id");
  if (new_password.empty()) return validation("newPassword is required","newPassword");
  if (repeat.empty()) return validation("ne2Password is required","ne2Password");
  if (new_password!=repeat) return validation("The passwords do not match","ne2Password");

  // Ghost permission `{docName:'user', method:'edit', identifier: user_id}`.
  const auto principal=make_principal(in);
  const auto authorizer=core::auth::ghost::GhostAuthorizer{};
  const auto decision=authorizer.authorize(principal, "user", "edit", user_id);
  if (!decision.allowed) {
   return core::workflow::StepResponse<Output>{{{"status",std::to_string(decision.status)},
    {"body",decision.error_json}}};
  }

  // Self password changes verify `oldPassword` before saving (user.js#changePassword).
  const auto old_password=field(entry,"oldPassword");
  if (!old_password.empty() && principal.id==user_id) {
   const auto stored=ctx.store().find_by_resource("users", user_id);
   if (stored) {
    const auto record=blog::api::JsonBody{}.parse_raw(*stored);
    const auto hash_field=record.find("password");
    if (hash_field!=record.end()) {
     const core::auth::PasswordHash stored_hash{decode_hash(hash_field->second)};
     if (stored_hash.valid() && !stored_hash.verify(old_password)) {
      return core::workflow::StepResponse<Output>{{{"status","403"},
       {"body",blog::api::ApiError{}.validation(
        "Your password is incorrect","Forbidden","oldPassword","ValidationError")}}};
     }
    }
   }
  }

  // `options:['ip']` is accepted for parity and recorded with the side effects.
  const auto ip=query_value(in.get_path(), "ip");

  core::auth::PasswordHasher hasher;
  const auto hashed=hasher.hash(new_password);
  if (!hashed) {
   return core::workflow::StepResponse<Output>{{{"status","500"},
    {"body",blog::api::ApiError{}.json("Could not hash the password.","Internal Server Error")}}};
  }
  const auto mapped="{\"password\":"+blog::api::JsonBody{}.quote(hashed->encoded())+"}";
  const auto mutation=ctx.store().edit("users", user_id, mapped);
  if (!mutation.ok()) {
   return core::workflow::StepResponse<Output>{{{"status",std::to_string(mutation.status())},
    {"body",mutation.body()}}};
  }

  // user.js#changePassword destroys every session; the endpoint then rotates the
  // current one. Both side effects are recorded on the shared transactional outbox.
  blog::handlers::outbox::global_outbox().emit(blog::handlers::outbox::Event{
   .type="user.password_changed",
   .payload=std::format(R"({{"user_id":{},"ip":{},"sessions_destroyed":true}})",
    blog::api::JsonBody{}.quote(user_id), blog::api::JsonBody{}.quote(ip)),
   .aggregate_id=user_id});

  return core::workflow::StepResponse<Output>{{{"status","200"},
   {"body",R"({"password":[{"message":"Password changed successfully."}]})"}}};
 }
 void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}
 static blog::BlogContext* s_ctx;
private:
 static core::workflow::StepResponse<Output> validation(std::string_view message, std::string_view property) {
  return core::workflow::StepResponse<Output>{{{"status","422"},
   {"body",blog::api::ApiError{}.validation("Validation error",message,property)}}};
 }
 static std::string decode_hash(std::string_view encoded) {
  if (encoded.size()>=2 && encoded.front()=='"' && encoded.back()=='"') {
   return std::string{encoded.substr(1,encoded.size()-2)};
  }
  return std::string{encoded};
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
 static std::string query_value(std::string_view path, std::string_view key) {
  const auto q=path.find('?');
  if (q==std::string_view::npos) return {};
  for (auto pair : std::views::split(path.substr(q+1), '&')) {
   const std::string text{std::ranges::begin(pair),std::ranges::end(pair)};
   const auto eq=text.find('=');
   if (eq!=std::string::npos && text.substr(0,eq)==key) return text.substr(eq+1);
  }
  return {};
 }
};
blog::BlogContext* ExecuteStep::s_ctx{nullptr};
}
