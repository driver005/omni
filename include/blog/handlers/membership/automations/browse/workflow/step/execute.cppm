export module blog_handlers:automations_browse_step;
import :domain_all;
import :automations_dto_automation_email_preview;
import :automations_dto_automation_email_preview_input;
import :automations_dto_automation_email_preview_response;
import :automations_dto_automation_email_test_input;
import :automations_dto_links;
import :automations_dto_stats_empty_response;
import std;
import core_workflow;
import core_serde;
import interfaces;

export namespace blog::handlers::automations::browse::workflow {

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
 using Input=interfaces::io::IRequest&; using Output=std::string;
 static constexpr std::string_view name{"automations_browse_execute"};
 [[nodiscard]] core::workflow::StepResponse<Output> invoke(const Input&, core::workflow::WorkflowContext&) const {
  auto& ctx=*s_ctx;
  // Ghost browse has no controller options: repository.browse() -> buildPagination(total)
  // yields limit:'all', page/pages:1, prev/next:null. Stats are hydrated per row with
  // EMPTY_AUTOMATION_STATS fallback when Tinybird has no data.
  const auto raw=ctx.store().json_for("automations", {});
  const auto rows=with_stats(ctx, raw);
  const std::size_t total=std::max<std::size_t>(ctx.store().total("automations"), rows=="[]"?0U:1U);
  const std::string meta="{\"pagination\":{\"page\":1,\"pages\":1,\"limit\":\"all\",\"total\":"+
    std::to_string(total)+",\"prev\":null,\"next\":null}}";
  return core::workflow::StepResponse<Output>{local_mapping::wrap("automations", rows, meta)};
 }
 void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}
 static blog::BlogContext* s_ctx;
private:
 using Object=serde::Value::Object;
 using Array=serde::Value::Array;

 static std::string with_stats(blog::BlogContext& ctx, std::string_view json) {
  const auto parsed=core_serde::JsonCodec{}.decode<serde::Value>(json);
  if (!parsed) return "[]";
  const auto* list=std::get_if<Array>(&parsed->get());
  if (list==nullptr) return "[]";
  std::string out="[";
  bool first=true;
  for (const auto& item:*list) {
   auto object=std::get_if<Object>(&item.get())==nullptr?Object{}:std::get<Object>(item.get());
   std::string id;
   for (const auto& [name,value]:object) if (name=="id") if (const auto* text=std::get_if<std::string>(&value.get())) id=*text;
   object["stats"]=serde::Value{stats_for(ctx, id)};
   out+=(first?"":",")+core_serde::JsonCodec{}.encode(serde::Value{object});
   first=false;
  }
  return out+"]";
 }

 static Object stats_for(blog::BlogContext& ctx, std::string_view id) {
  Object stats;
  stats["last_run_created_at"]=serde::Value{};
  std::size_t total=0, running=0;
  std::string last;
  const auto rows=ctx.store().json_for("automation_runs", {{"filter[automation_id]",std::string{id}}});
  const auto parsed=core_serde::JsonCodec{}.decode<serde::Value>(rows);
  if (parsed) if (const auto* list=std::get_if<Array>(&parsed->get())) {
   for (const auto& item:*list) {
    const auto* object=std::get_if<Object>(&item.get());
    if (object==nullptr) continue;
    ++total;
    for (const auto& [name,value]:*object) {
     if (name=="status" && std::get_if<std::string>(&value.get()) && std::get<std::string>(value.get())=="in_progress") ++running;
     if (name=="created_at") if (const auto* text=std::get_if<std::string>(&value.get())) last=*text;
    }
   }
  }
  if (!last.empty()) stats["last_run_created_at"]=serde::Value{last};
  stats["total_run_count"]=serde::Value{static_cast<std::int64_t>(total)};
  stats["in_progress_run_count"]=serde::Value{static_cast<std::int64_t>(running)};
  return stats;
 }
};
blog::BlogContext* ExecuteStep::s_ctx{nullptr};
}
