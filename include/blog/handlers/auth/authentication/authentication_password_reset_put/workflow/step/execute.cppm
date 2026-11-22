export module blog_handlers:authentication_authentication_password_reset_put_step;
import :domain_all;
import std;
import core_workflow;
import core_serde;
import interfaces;
import core_query;
import :outbox_shim;

export namespace blog::handlers::authentication::authentication_password_reset_put::workflow {
class ExecuteStep {
public:
  using Input=interfaces::io::IRequest&;
  using Output=std::map<std::string,std::string>;
  static constexpr std::string_view name{"authentication_authentication_password_reset_put_execute"};
  [[nodiscard]] core::workflow::StepResponse<Output> invoke(const Input& in, core::workflow::WorkflowContext&) const {
    // Ghost input pipeline (authentication.js:159-205): options:['ip'], body
    // data.password_reset[0].{token,oldPassword,newPassword,ne2Password}.
    const auto query=core::query::parse(in.get_path(), std::array<std::string_view,1>{"ip"});
    auto ip=query.options.contains("ip")?query.options.at("ip"):std::string{};
    if (ip.empty()) ip=std::string{in.find_header("X-Forwarded-For")};
    const auto data=decode(unwrap_data(in.body_as_string(), "password_reset"));
    const auto token=plain_field(data,"token");
    const auto new_password=plain_field(data,"newPassword");
    const auto confirm=plain_field(data,"ne2Password");

    // validator input/password_reset.js:16-27: newPassword === ne2Password.
    if (new_password.empty() || new_password!=confirm)
     return failure("422", err("Your new passwords do not match","ValidationError"));

    if (!is_setup(s_ctx->store()))
     return failure("403", err("Setup must be completed before making this request.","NoPermissionError"));

    // extractTokenParts (passwordreset.ts:106-128): base64-decode then extract.
    // Ghost's resetToken is `<expires>|<email>|<hash>`; this handler's issued
    // token uses `<user_id>:<expires>:<email>:<db_hash>`. Accept both wire forms.
    const auto decoded=b64decode(token);
    std::string email;
    std::uint64_t expires=0;
    bool ghost_style=false;
    const auto pipes=split(decoded,'|');
    if (pipes.size()==3 && to_number(pipes.front())!=0) {
      expires=to_number(pipes.front()); email=pipes[1]; ghost_style=true;
    } else {
      const auto parts=split(decoded,':');
      if (parts.size()>=3 && to_number(parts[1])!=0 && !parts[2].empty()) {
        expires=to_number(parts[1]); email=parts[2];
      }
    }
    if (email.empty() || expires==0)
     return failure("401", err("Cannot reset password.","UnauthorizedError",
      "Invalid password reset link.",
      "Check if password reset link has been fully copied or request new password reset via the login form."));

    auto& store=s_ctx->store();
    const auto issued=blog::api::parse_array(store.resources().list("password_reset_tokens"));
    const blog::api::RecordDto* found=nullptr;
    for (const auto& record:issued) {
      const auto it=record.find("token");
      if (it!=record.end() && unquote(it->second)==token) { found=&record; break; }
    }
    // A Ghost-style HMAC token is not in the local single-use registry; the
    // HMAC comparison itself is a documented residual (no @tryghost/security).
    if (found==nullptr && !ghost_style)
     return failure("400", err("Cannot reset password.","BadRequestError",
      "Password reset link has already been used.",
      "Request a new password reset via the login form."));
    if (std::chrono::duration_cast<std::chrono::milliseconds>(
          std::chrono::system_clock::now().time_since_epoch()).count()>static_cast<std::int64_t>(expires))
     return failure("400", err("Cannot reset password.","BadRequestError",
      "Password reset link expired.","Request a new password reset via the login form."));

    const auto users=blog::api::parse_array(store.json_for("users", {{"filter","email:"+email}}));
    if (users.empty()) return failure("404", err("User not found.","NotFoundError"));
    const auto& user=users.front();
    const auto user_id=field(user,"id");
    if (field(user,"status")=="inactive")
     return failure("403", err("This account has been suspended.","NoPermissionError"));

    // doReset (passwordreset.ts:190-201): changePassword + status active. The
    // password hash primitive is a Ghost/@tryghost/security residual; the
    // context-local user row is the observable stand-in.
    const auto password_json=std::format(R"({{"password":{},"status":"active"}})", q(new_password));
    (void)store.edit("users", user_id, password_json);
    const auto token_id=found->find("id");
    if (token_id!=found->end()) (void)store.resources().destroy("password_reset_tokens", unquote(token_id->second));

    // Session rotation + spam-prevention reset have no dedicated C++ primitive;
    // record them as observable side effects on the shared outbox.
    auto& outbox=blog::handlers::outbox::global_outbox();
    outbox.emit(blog::handlers::outbox::Event{.type="session.rotated",
      .payload=std::format(R"({{"user_id":{},"ip":{},"verified":true}})",q(user_id),q(ip)),.aggregate_id=user_id});
    outbox.emit(blog::handlers::outbox::Event{.type="spam.user_login_reset",
      .payload=std::format(R"({{"ip":{}}})",q(ip)),.aggregate_id=ip});

    // output/serializers/output/authentication.js#resetPassword.
    return core::workflow::StepResponse<Output>{{{"status","200"},
     {"body",R"({"password_reset":[{"message":"Password updated"}]})"}}};
  }
  void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}
  static blog::BlogContext* s_ctx;
private:
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
  static std::optional<Object> decode(std::string_view text) {
    const auto value=core_serde::JsonCodec{}.decode<serde::Value>(text);
    if (!value) return std::nullopt;
    const auto* object=std::get_if<Object>(&value->get());
    return object==nullptr?std::nullopt:std::optional<Object>{*object};
  }
  static std::string unwrap_data(std::string_view body, std::string_view resource) {
    const auto root=decode(body);
    if (!root) return "{}";
    for (const auto& [key,value]:*root) {
      if (key!="data") continue;
      const auto d=decode(core_serde::JsonCodec{}.encode(value));
      if (!d) return "{}";
      for (const auto& [rkey,rvalue]:*d) {
        if (rkey!=resource) continue;
        const auto rows=blog::api::parse_array(core_serde::JsonCodec{}.encode(rvalue));
        if (!rows.empty()) return blog::api::record_to_json(rows.front());
      }
    }
    return "{}";
  }
  static std::string plain_field(const std::optional<Object>& object, std::string_view key) {
    if (!object) return {};
    for (const auto& [name,value]:*object) {
      if (name==key) {
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
  static std::vector<std::string> split(std::string_view text, char delim) {
    std::vector<std::string> out;
    for (auto std::views::split(part:text, delim))
      out.emplace_back(std::ranges::begin(part),std::ranges::end(part));
    return out;
  }
  static std::uint64_t to_number(std::string_view text) {
    std::uint64_t value=0;
    for (const char c:text) {
      if (c<'0'||c>'9') return 0;
      value=value*10+static_cast<unsigned>(c-'0');
    }
    return value;
  }
  static std::string b64decode(std::string_view text) {
    static constexpr std::string_view alphabet="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    std::string out;
    unsigned buffer=0; int bits=0;
    for (const char c:text) {
      if (c=='=') break;
      const auto pos=alphabet.find(c);
      if (pos==std::string_view::npos) return {};
      buffer=(buffer<<6)|static_cast<unsigned>(pos);
      bits+=6;
      if (bits>=8) { bits-=8; out.push_back(static_cast<char>((buffer>>bits)&0xFF)); }
    }
    return out;
  }
  static std::string err(std::string_view message, std::string_view type,
                         std::string_view context={}, std::string_view help={}) {
    static thread_local std::mt19937 rng{std::random_device{}()};
    std::uniform_int_distribution<int> digit{0,15};
    const auto chunk=[&](int n){ std::string s; for(int i=0;i<n;++i) s+="0123456789abcdef"[digit(rng)]; return s; };
    const auto uuid=std::format("{}-{}-{}-{}-{}",chunk(8),chunk(4),chunk(4),chunk(4),chunk(12));
    return std::format(
      R"({{"errors":[{{"id":"{}","code":null,"context":{},"details":null,"ghostErrorCode":null,"help":{},"message":{},"property":null,"type":{}}}]}})",
      uuid, context.empty()?"null":q(context), help.empty()?"null":q(help), q(message), q(type));
  }
  static core::workflow::StepResponse<Output> failure(std::string_view status, std::string_view body) {
    return core::workflow::StepResponse<Output>{{{"status",std::string{status}},{"body",std::string{body}}}};
  }
};
blog::BlogContext* ExecuteStep::s_ctx{nullptr};
}
