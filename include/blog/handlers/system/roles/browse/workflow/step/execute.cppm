export module blog_handlers:roles_browse_step;
import :domain_all;
import std;
import core_workflow;
import core_query;
import core_auth;
import core_serde;
import blog_handlers_middleware;
import interfaces;

export namespace blog::handlers::roles::browse::workflow {

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
/**
 * @brief Ghost `roles.browse` (roles.js:5-19, output/roles.js:5-31).
 *
 * Strict-parity implementation (group 54). Primary observable contract:
 * - options ['permissions'] with validation permissions∈{'assign'} -> 422 on
 *   any other value (Ghost validation.options.permissions, roles.js:11-15);
 * - data source is the roles table snapshot merged with rows persisted through
 *   the shared resource store (s_ctx). No Knex DB exists in C++; the canonical
 *   Ghost role set is the accepted fixture fallback (residual gap);
 * - `permissions=assign` applies Ghost's per-role canThis.assign.role filter:
 *     Owner            -> all assignable, then 'Owner' dropped by serializer
 *     Administrator    -> Administrator, Super Editor, Editor, Author, Contributor
 *     Editor/Super Ed. -> Author, Contributor
 *     Author/Contrib.  -> none (role:['browse'] only in permissions-data.ts)
 *   (role.js:76-107, permissions-data.ts:13-24,65-234, output/roles.js:15-27);
 * - envelope {roles:[...]}, no meta.pagination (output/roles.js:10-13,28-30).
 */
class ExecuteStep {
public:
 using Input=interfaces::io::IRequest&;
 using Output=std::map<std::string,std::string>;
 static constexpr std::string_view name{"roles_browse_execute"};

 [[nodiscard]] core::workflow::StepResponse<Output> invoke(const Input& in, core::workflow::WorkflowContext&) const {
  auto& ctx=*s_ctx;

  // Ghost options: ['permissions'] only (roles.js:10). core_query parses and
  // URL-decodes the option; the enum restriction is enforced below.
  static constexpr std::string_view kOptions[]{"permissions"};
  const auto q=core::query::parse(in.get_path(), kOptions);
  const auto perm=q.options.find("permissions");
  if (perm!=q.options.end() && perm->second!="assign")
   return invalid("permissions","Invalid value");

  const bool assign=perm!=q.options.end();

  std::vector<RoleRow> roles=canonical_roles();
  merge_stored(ctx.store().json_for("roles", {}), roles);

  if (assign) {
   const auto principal=blog::handlers::middleware::principal_from_request(in);
   std::erase_if(roles, [&](const RoleRow& r){
    return r.name=="Owner" || !can_assign(principal, r.name);
   });
  }

  std::string rows="[";
  for (std::size_t i=0; i<roles.size(); ++i) {
   if (i) rows+=",";
   rows+=to_json(roles[i]);
  }
  rows+="]";

  // local_mapping::wrap produces the Ghost collection envelope with no meta
  // when meta_json is empty (output/default.js + output/roles.js:10-13).
  return core::workflow::StepResponse<Output>{{{"status","200"},
   {"body", local_mapping::wrap("roles", rows)}}};
 }

 void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}
 static blog::BlogContext* s_ctx;

private:
 struct RoleRow {
  std::string id;
  std::string name;
  std::string description;
  std::string created_at{"2024-01-01T00:00:00.000Z"};
  std::string updated_at{"2024-01-01T00:00:00.000Z"};
 };

 /** @brief Ghost default roles table (fixtures.json:75-118, ids in insert order). */
 [[nodiscard]] static std::vector<RoleRow> canonical_roles() {
  return {
   {"1","Administrator","Administrators"},
   {"2","Editor","Editors"},
   {"3","Author","Authors"},
   {"4","Contributor","Contributors"},
   {"5","Owner","Blog Owner"},
   {"6","Admin Integration","External Apps"},
   {"7","Self-Serve Migration Integration","Internal Integration for the Self-Serve migration tool"},
   {"8","DB Backup Integration","Internal DB Backup Client"},
   {"9","Scheduler Integration","Internal Scheduler Client"},
   {"10","Super Editor","Super Editors"},
  };
 }

 /** @brief Decode a raw JSON fragment (RecordDto values are wire fragments). */
 [[nodiscard]] static std::string text_of(std::string_view raw) {
  const auto value=core_serde::JsonCodec{}.decode<serde::Value>(raw);
  if (value) if (const auto* s=std::get_if<std::string>(&value->get())) return *s;
  return std::string{raw};
 }

 /** @brief Observable store edits override the snapshot row with the same name. */
 static void merge_stored(std::string_view stored_json, std::vector<RoleRow>& roles) {
  for (const auto& record : blog::api::parse_array(stored_json)) {
   const auto name_it=record.find("name");
   if (name_it==record.end()) continue;
   const std::string name=text_of(name_it->second);
   for (auto& r : roles) {
    if (r.name!=name) continue;
    if (const auto v=record.find("id"); v!=record.end()) r.id=text_of(v->second);
    if (const auto v=record.find("description"); v!=record.end()) r.description=text_of(v->second);
    if (const auto v=record.find("created_at"); v!=record.end()) r.created_at=text_of(v->second);
    if (const auto v=record.find("updated_at"); v!=record.end()) r.updated_at=text_of(v->second);
   }
  }
 }

 /** @brief core_auth:ghost permission service + Role.permissible caller rules.
  *
  *  permissions-data.ts: role:assign held by Administrator/Owner/Super
  *  Editor/Editor/Admin Integration; Author + Contributor only hold role:browse.
  *  role.js:76-107 maps the caller role to the assignable role names. The
  *  coarse GhostAuthorizer maps `roles` -> ADMIN, so it is consulted for the
  *  service gate while the per-role staff rules below mirror role.js exactly.
  */
 [[nodiscard]] static bool can_assign(const core::auth::ghost::Principal& p, std::string_view name) {
  using core::auth::ghost::ApiContext;
  static const core::auth::ghost::GhostAuthorizer auth{};
  const std::string_view caller=p.role;

  // Service gate. Ghost grants role:assign to Administrator/Owner/Super Editor/
  // Editor/Admin Integration (permissions-data.ts:77,135,167,191) and to any
  // principal carrying an explicit roles:assign / wildcard scope. The coarse
  // GhostAuthorizer maps `roles` -> ADMIN, which does not cover Editor, so the
  // Ghost grant list is honored alongside the authorizer result.
  const bool privileged=p.context==ApiContext::SYSTEM || p.context==ApiContext::INTERNAL;
  const bool explicit_scope=p.permissions.contains("roles:assign")
   || p.permissions.contains("*:*") || p.permissions.contains("*");
  const bool service=privileged || explicit_scope || auth.authorize(p, "roles", "assign").allowed
   || caller=="Owner" || caller=="Administrator" || caller=="Admin Integration"
   || caller=="Super Editor" || caller=="Editor";
  if (!service) return false;

  if (caller=="Owner") return true;
  if (caller=="Administrator" || caller=="Admin Integration")
   return in_set(name, {"Administrator","Super Editor","Editor","Author","Contributor"});
  if (caller=="Super Editor" || caller=="Editor")
   return in_set(name, {"Author","Contributor"});
  // Author, Contributor and any non-staff role cannot assign roles.
  return false;
 }

 [[nodiscard]] static bool in_set(std::string_view v, std::initializer_list<std::string_view> set) {
  for (const auto entry : set) if (v==entry) return true;
  return false;
 }

 [[nodiscard]] static std::string quote(std::string_view s) {
  std::string out{"\""};
  for (const char c : s) {
   switch (c) {
    case '"': out+="\\\""; break;
    case '\\': out+="\\\\"; break;
    case '\n': out+="\\n"; break;
    case '\r': out+="\\r"; break;
    case '\t': out+="\\t"; break;
    default: out+=c; break;
   }
  }
  out+='"';
  return out;
 }

 [[nodiscard]] static std::string to_json(const RoleRow& r) {
  return "{\"id\":"+quote(r.id)
   +",\"name\":"+quote(r.name)
   +",\"description\":"+quote(r.description)
   +",\"created_at\":"+quote(r.created_at)
   +",\"updated_at\":"+quote(r.updated_at)+"}";
 }

 [[nodiscard]] static core::workflow::StepResponse<Output> invalid(std::string_view property, std::string_view context) {
  return core::workflow::StepResponse<Output>{{{"status","422"},
   {"body",blog::api::ApiError{}.validation("Validation error",context,property)}}};
 }
};
blog::BlogContext* ExecuteStep::s_ctx{nullptr};
}
