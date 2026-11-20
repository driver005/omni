export module blog_handlers:authentication_authentication_reset_post_step;
import :domain_all;
import std;
import core_workflow;
import core_serde;
import interfaces;
import core_auth;
import blog_handlers_middleware;
import :outbox_shim;

export namespace blog::handlers::authentication::authentication_reset_post::workflow {
class ExecuteStep {
public:
  using Input=interfaces::io::IRequest&;
  using Output=std::map<std::string,std::string>;
  static constexpr std::string_view name{"authentication_authentication_reset_post_execute"};
  [[nodiscard]] core::workflow::StepResponse<Output> invoke(const Input& in, core::workflow::WorkflowContext&) const {
    // mw.authAdminApi + tokenPermissionCheck (middleware.js:34-43): staff tokens
    // are explicitly blocked on this path; integration tokens are not allowlisted.
    const auto principal=make_principal(in);
    if (principal.staff)
      return failure("403", err("Staff tokens are not allowed to access this endpoint","NoPermissionError"));
    const bool integration_token=!in.get_authorization().empty() && !principal.staff &&
      std::string{in.find_header("x-blog-role")}.empty() &&
      std::string{in.find_header("x-blog-user-id")}.empty();
    if (integration_token)
      return failure("403", err("API tokens do not have permission to access this endpoint","NoPermissionError"));
    // permissions:true (authentication.js:242).
    const auto decision=core::auth::ghost::GhostAuthorizer{}.authorize(principal,"authentication","edit");
    if (!decision.allowed) return failure(std::to_string(decision.status), decision.error_json);

    auto& store=s_ctx->store();
    // resetAuthentication (reset-authentication.ts:32-77).
    const auto previous_key=setting(store.resources().list("config"),"ghost-scheduler");

    // ApiKey.refreshAllSecrets inside the transaction.
    std::size_t api_keys_rotated=0;
    for (const auto& key:blog::api::parse_array(store.resources().list("api_keys"))) {
      const auto id=field(key,"id");
      if (id.empty()) continue;
      const auto secret=random_hex(32);
      (void)store.resources().edit("api_keys", id, std::format(R"({{"secret":{}}})",q(secret)));
      ++api_keys_rotated;
    }

    // userService.lockAll inside the transaction.
    std::size_t users_locked=0;
    for (const auto& user:blog::api::parse_array(store.json_for("users", {}))) {
      const auto id=field(user,"id");
      if (id.empty()) continue;
      (void)store.edit("users", id, R"({"status":"locked"})");
      ++users_locked;
    }

    // Action.add audit row when a user actor is present.
    const auto actor=principal.id.empty()?std::string{}:principal.id;
    if (!actor.empty()) {
      (void)store.resources().create("actions", std::format(
        R"({{"event":"edited","resource_type":"security_action","resource_id":null,"actor_type":"user","actor_id":{},"context":{{"action_name":"reset_authentication","api_keys_rotated":{},"users_locked":{}}}}})",
        q(actor),api_keys_rotated,users_locked));
    }

    // internalKeys.clear(); deleteAllSessions(); schedulerAdapter.rescheduleAll().
    (void)store.resources().destroy("config","ghost-scheduler");
    for (const auto& session:blog::api::parse_array(store.resources().list("sessions"))) {
      const auto id=field(session,"id");
      if (!id.empty()) (void)store.resources().destroy("sessions", id);
    }
    auto& outbox=blog::handlers::outbox::global_outbox();
    outbox.emit(blog::handlers::outbox::Event{.type="internal_keys.cleared",
      .payload="{}",.aggregate_id="ghost-scheduler"});
    outbox.emit(blog::handlers::outbox::Event{.type="scheduler.reschedule_all",
      .payload=std::format(R"({{"previous_key":{}}})",q(previous_key)),.aggregate_id="ghost-scheduler"});
    // destroyRequestSession (authentication.js:254): expire the client cookie.
    outbox.emit(blog::handlers::outbox::Event{.type="session.destroyed",
      .payload=std::format(R"({{"actor":{},"expire_cookie":true}})",q(actor)),.aggregate_id=actor});

    // output/serializers/output/authentication.js#reset.
    const auto body=std::format(
      R"({{"security_action":[{{"action":"reset_authentication","api_keys_rotated":{},"users_locked":{}}}]}})",
      api_keys_rotated,users_locked);
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
  static std::string setting(const std::string& json, std::string_view key) {
    for (const auto& record:blog::api::parse_array(json)) {
      if (field(record,"key")==key) return field(record,"value");
    }
    return {};
  }
  static core::auth::ghost::Principal make_principal(interfaces::io::IRequest& in) {
    auto principal=blog::handlers::middleware::principal_from_request(in);
    auto subject=std::string{in.find_header("x-blog-user-id")};
    if (subject.empty() && !in.get_authorization().empty()) subject="bearer";
    principal.id=std::move(subject);
    if (principal.context==core::auth::ghost::ApiContext::ANONYMOUS && !principal.id.empty())
      principal.context=core::auth::ghost::ApiContext::ADMIN_API;
    if (principal.role.empty() && !principal.staff && !principal.id.empty()) principal.role="Owner";
    return principal;
  }
  static std::string random_hex(std::size_t length) {
    static thread_local std::mt19937 rng{std::random_device{}()};
    std::uniform_int_distribution<int> digit{0,15};
    std::string out; out.reserve(length);
    for (std::size_t i=0;i<length;++i) out+="0123456789abcdef"[digit(rng)];
    return out;
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
