export module blog_handlers:automations_automations_id_runs_get_step;
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
import core_query;
import interfaces;
import :automations_operations;

export namespace blog::handlers::automations::automations_id_runs_get::workflow {

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
  using Input=interfaces::io::IRequest&; using Output=std::map<std::string, std::string>;
  static constexpr std::string_view name{"automations_automations_id_runs_get_execute"};
  [[nodiscard]] core::workflow::StepResponse<Output> invoke(const Input& in, core::workflow::WorkflowContext&) const {
   const std::string id=blog::api::automations::automation_id(in.get_path());
   if (id.empty())
    return core::workflow::StepResponse<Output>{{{"status","400"},
      {"body",blog::api::automations::ghost_error("BadRequestError","id is required")}}};
   // Ghost options: status/date_from/date_to/timezone/order/cursor/search.
   // core::query::parse applies the Ghost allowlist + option regex validation.
   static constexpr std::string_view kOptions[]{
     "status","date_from","date_to","timezone","order","cursor","search"};
   const auto q=core::query::parse(in.get_path(), kOptions);
   if (const auto status=option(q,"status"); !status.empty() &&
       status!="in_progress"&&status!="completed"&&status!="exited_early")
    return core::workflow::StepResponse<Output>{{{"status","422"},
      {"body",blog::api::ApiError{}.validation("Validation error, cannot browse runs.","Invalid status","status")}}};
   if (!q.order.empty() && q.order!="created_at desc" && q.order!="created_at asc")
    return core::workflow::StepResponse<Output>{{{"status","422"},
      {"body",blog::api::ApiError{}.validation("Validation error, cannot browse runs.","Invalid order","order")}}};

   auto& store=s_ctx->store();
   if (!blog::api::automations::automation_exists(*s_ctx, id))
    return core::workflow::StepResponse<Output>{{{"status","404"},
      {"body",blog::api::automations::ghost_error("NotFoundError","Automation not found.")}}};

   std::map<std::string,std::string> params{{"filter[automation_id]",id},{"limit","15"}};
   if (const auto status=option(q,"status"); !status.empty()) params["filter[status]"]=status;
   const auto cursor=option(q,"cursor");
   std::size_t offset=0;
   try { offset=cursor.empty()?0:std::stoul(cursor); } catch (...) { offset=0; }
   params["offset"]=std::to_string(offset);
   const auto rows=store.json_for("automation_runs", params);

   std::string data="[";
   bool first=true;
   std::size_t count=0;
   for (const auto& run:as_array_or_empty(rows)) {
    ++count;
    auto json=core_serde::JsonCodec{}.encode(run);
    const auto member_id=field_string(as_object(&run),"member_id");
    if (!member_id.empty()) {
     const auto member=store.find_by_resource("members", member_id);
     if (member) if (const auto pos=json.rfind('}');pos!=std::string::npos)
      json.insert(pos, ",\"member\":"+*member);
    }
    data+=(first?"":",")+json;
    first=false;
   }
   data+="]";
   const std::string next=count>=15?core_serde::JsonCodec{}.encode(std::to_string(offset+15)):std::string{"null"};
   const std::string meta="{\"pagination\":{\"limit\":15,\"next_cursor\":"+next+"}}";
   return core::workflow::StepResponse<Output>{{{"status","200"},
     {"body",local_mapping::wrap("data",data,meta)}}};
  }
  void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}
  static blog::BlogContext* s_ctx;

private:
  using Object=serde::Value::Object;
  using Array=serde::Value::Array;
  static std::string option(const core::query::ParsedQuery& q, std::string_view key) {
   const auto it=q.options.find(std::string{key});
   return it==q.options.end()?std::string{}:it->second;
  }
  [[nodiscard]] static const Object* as_object(const serde::Value* v) {
   return v!=nullptr?std::get_if<Object>(&v->get()):nullptr;
  }
  [[nodiscard]] static std::vector<serde::Value> as_array_or_empty(std::string_view json) {
   const auto parsed=core_serde::JsonCodec{}.decode<serde::Value>(json);
   if (!parsed) return {};
   const auto* arr=std::get_if<Array>(&parsed->get());
   return arr==nullptr?std::vector<serde::Value>{}:*arr;
  }
  [[nodiscard]] static std::string field_string(const Object* object, std::string_view key) {
   if (object==nullptr) return {};
   for (const auto& [name,value]:*object)
    if (name==key) if (const auto* text=std::get_if<std::string>(&value.get())) return *text;
   return {};
  }
};
blog::BlogContext* ExecuteStep::s_ctx{nullptr};
}
