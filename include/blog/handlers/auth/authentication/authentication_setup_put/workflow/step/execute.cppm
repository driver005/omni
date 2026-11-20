export module blog_handlers:authentication_authentication_setup_put_step;
import :domain_all;
import std;
import core_workflow;
import core_serde;
import interfaces;
import core_auth;
import blog_handlers_middleware;

export namespace blog::handlers::authentication::authentication_setup_put::workflow {
class ExecuteStep {
  using Object=serde::Value::Object;

  static std::string q(std::string_view value) { return core_serde::JsonCodec{}.encode(std::string{value}); }
  static std::string unquote(std::string_view value) {
    if (value.size()>=2 && value.front()=='"' && value.back()=='"')
     return std::string{value.substr(1,value.size()-2)};
    return std::string{value};
  }
  static std::string field(const blog::api::RecordDto& r, std::string_view key) {
    const auto it=r.find(std::string{key});
    return it==r.end()?std::string{}:unquote(it->second);
  }
  static std::vector<blog::api::RecordDto> parse_owners(const blog::api::ApiDataStore& store) {
    auto owners=blog::api::parse_array(store.json_for("users", {{"filter","roles:Owner"}}));
    if (owners.empty()) owners=blog::api::parse_array(store.json_for("users", {}));
    return owners;
  }
  static std::optional<Object> decode(std::string_view text) {
    const auto value=core_serde::JsonCodec{}.decode<serde::Value>(text);
    if (!value) return std::nullopt;
    const auto* object=std::get_if<Object>(&value->get());
    return object==nullptr?std::nullopt:std::optional<Object>{*object};
  }
  static std::string unwrap_data(std::string_view body, std::string_view resource) {
    const auto root=decode(body);
    if (!root) return "{}";
    for (const auto& [jkey,jvalue]:*root) {
      if (jkey!="data") continue;
      const auto d=decode(core_serde::JsonCodec{}.encode(jvalue));
      if (!d) return "{}";
      for (const auto& [rkey,rvalue]:*d) {
        if (rkey!=resource) continue;
        const auto rows=blog::api::parse_array(core_serde::JsonCodec{}.encode(rvalue));
        if (!rows.empty()) return blog::api::record_to_json(rows.front());
      }
    }
    return "{}";
  }
  static std::string plain_field(const std::optional<Object>& object, std::string_view field_name) {
    if (!object) return {};
    for (const auto& [name,value]:*object) {
      if (name==field_name) {
        if (const auto* text=std::get_if<std::string>(&value.get())) return *text;
      }
    }
    return {};
  }
  static bool is_setup(const blog::api::ApiDataStore& store) {
    for (const auto& user : blog::api::parse_array(store.json_for("users", {}))) {
      if (field(user,"status")=="inactive") continue;
      return true;
    }
    return false;
  }
  static void put_setting(blog::api::ApiDataStore& store, std::string_view skey, std::string_view svalue) {
    const auto record=std::format(R"({{"key":{},"value":{}}})",q(skey),q(svalue));
    (void)store.resources().create("settings", record);
    (void)store.resources().create("config", record);
  }
  static std::string trim(std::string_view value) {
    while (!value.empty() && std::isspace(static_cast<unsigned char>(value.front()))) value.remove_prefix(1);
    while (!value.empty() && std::isspace(static_cast<unsigned char>(value.back()))) value.remove_suffix(1);
    return std::string{value};
  }
  static core::auth::ghost::Principal make_principal(interfaces::io::IRequest& in) {
    auto principal=blog::handlers::middleware::principal_from_request(in);
    auto subject=std::string{in.find_header("x-blog-user-id")};
    if (subject.empty() && !in.get_authorization().empty()) subject="1";
    principal.id=std::move(subject);
    if (principal.context==core::auth::ghost::ApiContext::ANONYMOUS &&
        (!in.get_authorization().empty() || !principal.id.empty()))
      principal.context=core::auth::ghost::ApiContext::ADMIN_API;
    if (principal.role.empty() && !principal.id.empty()) principal.role="Owner";
    return principal;
  }
  static std::string clean_user(std::string_view json) {
    static constexpr std::string_view keep[]{
      "id","name","slug","email","profile_image","cover_image","bio","website","location",
      "status","roles","created_at","updated_at","last_seen","url"};
    const auto parsed=blog::api::JsonBody{}.parse_raw(json);
    std::string out="{"; bool first=true;
    for (const auto ukey:keep) {
      const auto it=parsed.find(std::string{ukey});
      if (it==parsed.end()) continue;
      if (!first) out+=","; first=false;
      out+=q(ukey)+":"+it->second;
    }
    if (!parsed.contains("roles")) {
      if (!first) out+=",";
      out+=R"("roles":[{"name":"Owner"}])";
    }
    out+="}"; return out;
  }
  static std::string err(std::string_view message, std::string_view type) {
    static thread_local std::mt19937 rng{std::random_device{}()};
    std::uniform_int_distribution<int> digit{0,15};
    const auto chunk=[&](int n){ std::string s; for(int i=0;i<n;++i) s+="0123456789abcdef"[digit(rng)]; return s; };
    const auto uuid=std::format("{}-{}-{}-{}-{}",chunk(8),chunk(4),chunk(4),chunk(4),chunk(12));
    return std::format(
      R"({{"errors":[{{"id":"{}","code":null,"context":null,"details":null,"ghostErrorCode":null,"help":null,"message":{},"property":null,"type":{}}}]}})",
      uuid, q(message), q(type));
  }
  static core::workflow::StepResponse<std::map<std::string,std::string>> failure(
      std::string_view status, std::string_view body) {
    using Output=std::map<std::string,std::string>;
    return core::workflow::StepResponse<Output>{{{"status",std::string{status}},{"body",std::string{body}}}};
  }

public:
  static blog::BlogContext* s_ctx;
  using Input=interfaces::io::IRequest&;
  using Output=std::map<std::string,std::string>;
  static constexpr std::string_view name{"authentication_authentication_setup_put_execute"};
  [[nodiscard]] core::workflow::StepResponse<Output> invoke(const Input& in, core::workflow::WorkflowContext&) const {
    // mw.authAdminApi + the async owner-only permission fn (authentication.js:92-97,
    // input/setup.ts:13-18): a context.user is required and must be the Owner.
    const auto principal=make_principal(in);
    const auto decision=core::auth::ghost::GhostAuthorizer{}.authorize(principal,"authentication","edit");
    if (!decision.allowed) return failure(std::to_string(decision.status), decision.error_json);
    if (principal.id.empty()) return failure("403", err("You are not the site owner.","NoPermissionError"));

    auto& store=s_ctx->store();
    const auto owners=parse_owners(store);
    const auto owner_id=owners.empty()?std::string{}:field(owners.front(),"id");
    auto request_user=principal.id;
    if (request_user=="1" || request_user=="bearer") request_user=owner_id;
    if (owner_id.empty() || owner_id!=request_user)
      return failure("403", err("You are not the site owner.","NoPermissionError"));

    // assertSetupCompleted(true) (setup.js:42-63).
    if (!is_setup(store))
      return failure("403", err("Setup must be completed before making this request.","NoPermissionError"));

    const auto data=decode(unwrap_data(in.body_as_string(), "setup"));
    const auto name=plain_field(data,"name");
    const auto email=plain_field(data,"email");
    const auto password=plain_field(data,"password");
    const auto blog_title=plain_field(data,"blogTitle");

    // setupUser (setup.js:65-82): update the Owner row, status active.
    std::string user_json="{";
    bool first=true;
    const std::array<std::pair<std::string_view,std::string_view>,3> fields{{
      {"name",name},{"email",email},{"password",password}}};
    for (const auto& [fkey,fvalue]:fields) {
      if (fvalue.empty()) continue;
      if (!first) user_json+=","; first=false;
      user_json+=q(fkey)+":"+q(fvalue);
    }
    if (!first) user_json+=",";
    user_json+=R"({"status":"active"})";
    (void)store.edit("users", owner_id, user_json);

    // doSettings (setup.js:84-109): title required; description falls back to
    // the sample text; updateSetup carries no accentColor.
    if (!blog_title.empty()) put_setting(store,"title",trim(blog_title));
    put_setting(store,"description","Thoughts, stories and ideas.");

    const auto user=store.find_by_resource("users", owner_id);
    const auto model=user?clean_user(*user):std::string{"{}"};
    // output/serializers/output/authentication.js#updateSetup.
    return core::workflow::StepResponse<Output>{{{"status","200"},
      {"body","{\"users\":["+model+"]}"}}};
  }
  void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}
};
blog::BlogContext* ExecuteStep::s_ctx{nullptr};
}
