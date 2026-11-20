export module blog_handlers:invites_create_step;
import :domain_all;
import :invites_dto_admin_resources;
import std;
import core_workflow;
import core_serde;
import core_query;
import core_auth;
import blog_handlers_middleware;
import interfaces;

export namespace blog::handlers::invites::create::workflow {
class ExecuteStep {
public:
 using Input=interfaces::io::IRequest&; using Output=std::map<std::string,std::string>;
 static constexpr std::string_view name{"invites_create_execute"};
 [[nodiscard]] core::workflow::StepResponse<Output> invoke(const Input& in, core::workflow::WorkflowContext&) const {
  auto& ctx=*s_ctx;

  // Ghost options:['include','email'], include∈[] (invites.js:80-84).
  static constexpr std::string_view kOptions[]{"include","email"};
  const auto q=core::query::parse(in.get_path(), kOptions);
  if (!q.include.empty()) return invalid("include","include is not supported");

  // Unwrap data.invites[0] (invites.js:97-108).
  const auto raw=std::string{in.body_as_string()};
  std::string inner_json=raw.empty()?std::string{"{}"}:raw;
  const std::optional<serde::Value> parsed=raw.empty()?std::nullopt:core_serde::JsonCodec{}.decode<serde::Value>(raw);
  if (parsed) {
   const auto* root=std::get_if<Object>(&parsed->get());
   if (root!=nullptr) {
    const auto* data=value_for(root,"data");
    if (data!=nullptr) {
     const auto* data_object=std::get_if<Object>(&data->get());
     if (data_object!=nullptr) {
      const auto* invites=value_for(data_object,"invites");
      if (invites!=nullptr) {
       const auto* array=std::get_if<serde::Value::Array>(&invites->get());
       if (array!=nullptr && !array->empty()) {
        const auto* invite=std::get_if<Object>(&array->front().get());
        if (invite!=nullptr) inner_json=core_serde::JsonCodec{}.encode(*invite);
       }
      }
     }
    }
   }
  }
  const auto fields=blog::api::JsonBody{}.parse_raw(inner_json);
  const std::string role_id=support::plain_field(fields,"role_id");
  const std::string email=support::plain_field(fields,"email");
  // validation.data: role_id + email required (invites.js:85-93).
  if (role_id.empty()) return invalid("role_id","role_id is required");
  if (email.empty()) return invalid("email","email is required");

  // permissions.unsafeAttrs = ['role_id'] (invites.js:94-96).
  const auto principal=blog::handlers::middleware::principal_from_request(in);
  const auto authorizer=core::auth::ghost::GhostAuthorizer{};
  const auto unsafe=authorizer.unsafe_attrs(principal,"role_id",role_id);
  if (!unsafe.allowed)
   return core::workflow::StepResponse<Output>{{{"status",std::to_string(unsafe.status)},{"body",unsafe.error_json}}};
  // model Invite.permissible: Owner can never be invited (model/invite.js:92-97).
  if (resolve_role_name(ctx, role_id)=="Owner") {
   return core::workflow::StepResponse<Output>{{{"status","403"},
    {"body",blog::api::ApiError{}.validation(
      "Not allowed to invite an owner user.","Forbidden","","NoPermissionError")}}};
  }

  // Duplicate replacement: InviteModel.findOne({email}) -> destroy (invites.ts:122-129).
  core::query::ParsedQuery by_email;
  by_email.filters.emplace_back("email", email);
  const auto existing=core::query::apply(support::clean(ctx.store().json_for("invites", {})), by_email, 15);
  for (const auto& item : blog::api::parse_array(existing)) {
   const auto id=support::plain_field(item,"id");
   if (!id.empty()) (void)ctx.store().destroy("invites", id);
  }

  // InviteModel.add: status pending, expires now+1 week, signed token (model/invite.js:38-54).
  const std::string token=support::new_token();
  const std::string expires=support::expires_millis();
  const std::string now=support::now_iso();
  const auto created=ctx.store().create("invites",
   support::full_invite("0", role_id, email, "pending", expires, now, now, token));
  const std::string id=created.ok()?extract_id(created.body()):std::string{};
  if (id.empty())
   return core::workflow::StepResponse<Output>{{{"status","500"},
    {"body",blog::api::ApiError{}.json("Could not create invite.","Internal Server Error")}}};

  // invite-created model action side effect (actionsResourceType:'invite').
  support::emit("invite.created",
   "{\"id\":"+support::quote(id)+",\"status\":\"pending\",\"role_id\":"+support::quote(role_id)+"}", id);

  // api.mail.send(payload,{context:{internal:true}}) (invites.ts:136-201).
  emit_invite_mail(in, email, token);

  // InviteModel.edit({status:'sent'}) (invites.ts:203-209).
  const auto sent=support::full_invite(id, role_id, email, "sent", expires, now, now, token);
  const auto body=support::wrap(support::clean(sent));
  return core::workflow::StepResponse<Output>{{{"status","201"},
   {"body",body},{"location",support::location_for(in.get_path(), id)}}};
 }
 void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}
 static blog::BlogContext* s_ctx;
private:
 using Object=serde::Value::Object;

 static const serde::Value* value_for(const Object* object, std::string_view key) {
  for (const auto& [name,value]:*object) if (name==key) return &value;
  return nullptr;
 }
 static std::string extract_id(std::string_view body) {
  const auto parts=blog::api::JsonBody{}.parse_raw(body);
  const auto it=parts.find("invites");
  if (it==parts.end()) return {};
  const auto rows=blog::api::parse_array(it->second);
  if (rows.empty()) return {};
  return support::plain_field(rows.front(),"id");
 }
 static std::string resolve_role_name(blog::BlogContext& ctx, std::string_view role_id) {
  const auto rows=blog::api::parse_array(ctx.store().json_for("roles", {}));
  for (const auto& row : rows) {
   if (support::plain_field(row,"id")==role_id) return support::plain_field(row,"name");
  }
  static constexpr std::string_view kNames[]{"Administrator","Owner","Editor","Author","Contributor","Super Editor"};
  for (auto name : kNames) if (name==role_id) return std::string{name};
  return {};
 }
 static std::string base64(std::string_view text) {
  static constexpr char kAlphabet[]="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
  std::string out;
  std::size_t i=0;
  while (i+2<text.size()) {
   const unsigned v=(static_cast<unsigned char>(text[i])<<16)|
                    (static_cast<unsigned char>(text[i+1])<<8)|
                     static_cast<unsigned char>(text[i+2]);
   out+=kAlphabet[(v>>18)&0x3F]; out+=kAlphabet[(v>>12)&0x3F];
   out+=kAlphabet[(v>>6)&0x3F];  out+=kAlphabet[v&0x3F];
   i+=3;
  }
  const auto left=text.size()-i;
  if (left==1) {
   const unsigned v=static_cast<unsigned char>(text[i])<<16;
   out+=kAlphabet[(v>>18)&0x3F]; out+=kAlphabet[(v>>12)&0x3F]; out+="==";
  } else if (left==2) {
   const unsigned v=(static_cast<unsigned char>(text[i])<<16)|
                    (static_cast<unsigned char>(text[i+1])<<8);
   out+=kAlphabet[(v>>18)&0x3F]; out+=kAlphabet[(v>>12)&0x3F];
   out+=kAlphabet[(v>>6)&0x3F]; out+='=';
  }
  return out;
 }
 static void emit_invite_mail(interfaces::io::IRequest& in, std::string_view email, std::string_view token) {
  const std::string name{std::string{in.find_header("x-blog-user-name")}};
  const std::string inviter{std::string{in.find_header("x-blog-user-email")}};
  const std::string blog_name="Ghost";
  const std::string reset_link=std::string{in.get_path()}.substr(0,in.get_path().find('?'))+
   "/signup/"+base64(token)+"/";
  const std::string reply_to=inviter.empty()?"ghost@ghost.local":inviter;
  const std::string subject=name.empty()
   ?"You have been invited to join "+blog_name
   :name+" has invited you to join "+blog_name;
  const std::string template_name=name.empty()?"invite-user-by-api-key":"invite-user";
  const std::string message=
   "{\"to\":"+support::quote(email)+",\"replyTo\":"+support::quote(reply_to)+
   ",\"subject\":"+support::quote(subject)+
   ",\"html\":"+support::quote("<p>"+subject+"</p>")+
   ",\"text\":"+support::quote(subject)+"}";
  std::string payload="{\"mail\":[{\"message\":"+message+",\"options\":{}}]";
  payload+=",\"context\":{\"internal\":true}";
  payload+=",\"template\":"+support::quote(template_name);
  payload+=",\"resetLink\":"+support::quote(reset_link)+"}";
  support::emit("mail.send", payload, std::string{email});
 }
 static core::workflow::StepResponse<Output> invalid(std::string_view property, std::string_view context) {
  return core::workflow::StepResponse<Output>{{{"status","422"},
   {"body",blog::api::ApiError{}.validation("Validation error",context,property)}}};
 }
};
blog::BlogContext* ExecuteStep::s_ctx{nullptr};
}
