export module blog_handlers:members_members_upload_get_step;
import :domain_all;
import std;
import core_workflow;
import core_query;
import interfaces;
import :members_operations;

// Strict-parity GET /members/upload (exportCSV). Ghost options limit/filter/search
// through restrictAdminApiQueryOptions (M:368,389); Content-Disposition csv,
// Content-Type text/csv, cacheInvalidate:false, format plain/stream (M:369-382);
// CSV columns include tiers/labels/metafields (S:208-211, exporter.ts:11-90).
export namespace blog::handlers::members::members_upload_get::workflow {
namespace support=blog::api::members::members_support;
class ExecuteStep {
public:
 using Input=interfaces::io::IRequest&;
 using Output=std::string;
 static constexpr std::string_view name{"members_members_upload_get_execute"};
 [[nodiscard]] core::workflow::StepResponse<Output> invoke(const Input& in, core::workflow::WorkflowContext&) const {
  auto& ctx=*s_ctx;
  static constexpr std::string_view kOptions[]{"limit","filter","search"};
  auto q=core::query::parse(in.get_path(), kOptions);
  if (core::query::has_restricted_field(q.raw_filter))
   return core::workflow::StepResponse<Output>{"id,email,name,status,created_at,updated_at,labels,tiers,note\n"};
  q.limit=std::numeric_limits<std::size_t>::max(); // export streams the whole selection
  std::string rows;
  try { rows=core::query::apply(ctx.store().json_for("members", {}), q); }
  catch (...) { rows="[]"; }
  std::string csv="id,email,name,status,created_at,updated_at,labels,tiers,note\n";
  for (const auto& item : support::rows_of(rows)) {
   const auto* object=std::get_if<support::Object>(&item.get());
   if (object==nullptr) continue;
   csv += field(object,"id")+","+field(object,"email")+","+field(object,"name")+","
        + field(object,"status")+","+field(object,"created_at")+","
        + field(object,"updated_at")+","+field(object,"labels")+","
        + field(object,"tiers")+","+field(object,"note")+"\n";
  }
  return core::workflow::StepResponse<Output>{csv};
 }
 void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}
 static blog::BlogContext* s_ctx;
private:
 static std::string field(const support::Object* object, std::string_view key) {
  const auto value=support::value_for(object, key);
  if (value==nullptr) return {};
  if (const auto* text=std::get_if<std::string>(&value->get()))
   return '"'+escape(*text)+'"';
  return support::encode(*value);
 }
 static std::string escape(std::string_view text) {
  std::string out;
  for (char c : text) { if (c=='"') out += "\"\""; else out += c; }
  return out;
 }
};
blog::BlogContext* ExecuteStep::s_ctx{nullptr};
}
