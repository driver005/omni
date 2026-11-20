export module blog_handlers:authentication_authentication_invitation_get_step;
import :domain_all;
import std;
import core_workflow;
import core_serde;
import interfaces;
import core_query;

export namespace blog::handlers::authentication::authentication_invitation_get::workflow {
class ExecuteStep {
public:
  using Input=interfaces::io::IRequest&;
  using Output=std::map<std::string,std::string>;
  static constexpr std::string_view name{"authentication_authentication_invitation_get_execute"};
  [[nodiscard]] core::workflow::StepResponse<Output> invoke(const Input& in, core::workflow::WorkflowContext&) const {
    // Ghost input pipeline (authentication.js:221-235): data:['email'],
    // validation input/invitations.js:34-44 requires a valid email string.
    const auto query=core::query::parse(in.get_path(), std::array<std::string_view,1>{"email"});
    const auto it=query.options.find("email");
    const auto email=it==query.options.end()?std::string{}:it->second;
    if (!valid_email(email))
     return failure("400", err("The server did not receive a valid email","BadRequestError"));

    if (!is_setup(s_ctx->store()))
     return failure("403", err("Setup must be completed before making this request.","NoPermissionError"));

    // models.Invite.findOne({email,status:'sent'}) returns null (not a 404) on
    // a miss; output/authentication.js#isInvitation maps it to valid:false.
    const auto rows=s_ctx->store().json_for("invites", {{"filter","email:"+email}});
    bool valid=false;
    for (const auto& record:blog::api::parse_array(rows)) {
      const auto status=record.find("status");
      const auto mail=record.find("email");
      if (status==record.end() || unquote(status->second)!="sent") continue;
      if (mail==record.end() || unquote(mail->second)!=email) continue;
      valid=true; break;
    }
    const auto body=valid
      ? std::string{R"({"invitation":[{"valid":true}]})"}
      : std::string{R"({"invitation":[{"valid":false}]})"};
    return core::workflow::StepResponse<Output>{{{"status","200"},{"body",body}}};
  }
  void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}
  static blog::BlogContext* s_ctx;
private:
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
  static bool is_setup(const blog::api::ApiDataStore& store) {
    for (const auto& user : blog::api::parse_array(store.json_for("users", {}))) {
      if (field(user,"status")=="inactive") continue;
      return true;
    }
    return false;
  }
  static bool valid_email(std::string_view value) {
    const auto at=value.find('@');
    if (at==std::string_view::npos || at==0 || at+1==value.size()) return false;
    if (value.find(' ',0)!=std::string_view::npos) return false;
    return value.find('.',at+1)!=std::string_view::npos;
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
  static core::workflow::StepResponse<Output> failure(std::string_view status, std::string_view body) {
    return core::workflow::StepResponse<Output>{{{"status",std::string{status}},{"body",std::string{body}}}};
  }
};
blog::BlogContext* ExecuteStep::s_ctx{nullptr};
}
