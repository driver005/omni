export module blog_handlers:members_bulk_destroy_step;
import :domain_all;
import std;
import core_workflow;
import core_query;
import interfaces;
import :members_operations;

// Strict-parity DELETE /members (bulkDestroy). Ghost guard requires
// all===true || filter || search (M:323, MR:1293-1323); restricted fields in a
// bulk filter are rejected by validateBulkFilter (IS:144-148, AF:291-301);
// output {meta:{stats:{successful,unsuccessful},unsuccessfulIds,errors}} (M:331-340).
export namespace blog::handlers::members::bulk_destroy::workflow {
namespace support=blog::api::members::members_support;
class ExecuteStep {
public:
 using Input=interfaces::io::IRequest&;
 using Output=std::map<std::string,std::string>;
 static constexpr std::string_view name{"members_bulk_destroy_execute"};
 [[nodiscard]] core::workflow::StepResponse<Output> invoke(const Input& in, core::workflow::WorkflowContext&) const {
  auto& ctx=*s_ctx;
  static constexpr std::string_view kOptions[]{"all","filter","search"};
  auto q=core::query::parse(in.get_path(), kOptions);
  const bool guarded=q.options.contains("all") || !q.raw_filter.empty() ||
                     q.options.contains("search");
  if (!guarded)
   return core::workflow::StepResponse<Output>{{{"status","400"},
     {"body",support::incorrect_usage("Bulk destroy requires all, filter or search.")}}};
  if (core::query::has_restricted_field(q.raw_filter))
   return core::workflow::StepResponse<Output>{{{"status","400"},
     {"body",support::bad_request("Restricted fields cannot be used in a filter.")}}};

  std::string available=ctx.store().json_for("members", {});
  try { available=core::query::apply(available, q); }
  catch (...) {
   return core::workflow::StepResponse<Output>{{{"status","400"},
     {"body",support::bad_request("The NQL filter you passed was invalid.")}}};
  }
  auto ids=extract_ids(available);
  std::string unsuccessful;
  for (const auto& id : ids) {
   if (!ctx.store().find_by_resource("members", id)) {
    if (!unsuccessful.empty()) unsuccessful += ',';
    unsuccessful += '"'+id+'"';
   }
  }
  std::string id_list;
  for (const auto& id : ids) { if (!id_list.empty()) id_list += ','; id_list += id; }
  const auto result=ctx.store().bulk_destroy("members", R"({"ids":")"+id_list+R"("})");
  const int successful=result.status()==200?static_cast<int>(ids.size())-count(unsuccessful):0;
  const auto body=std::format(
    R"({{"meta":{{"stats":{{"successful":{},"unsuccessful":{}}},"unsuccessfulIds":[{}],"errors":[]}}}}}})",
    successful, count(unsuccessful), unsuccessful);
  return core::workflow::StepResponse<Output>{{{"status",std::to_string(result.status())},{"body",body}}};
 }
 void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}
 static blog::BlogContext* s_ctx;
private:
 static int count(std::string_view list) {
  if (list.empty()) return 0;
  int n=1; for (char c:list) if (c==',') ++n; return n;
 }
 static std::vector<std::string> extract_ids(std::string_view rows) {
  std::vector<std::string> out;
  for (const auto& item : support::rows_of(rows)) {
   const auto* object=std::get_if<support::Object>(&item.get());
   if (object==nullptr) continue;
   const auto id=support::scalar(object, "id");
   if (!id.empty()) out.push_back(id);
  }
  return out;
 }
};
blog::BlogContext* ExecuteStep::s_ctx{nullptr};
}
