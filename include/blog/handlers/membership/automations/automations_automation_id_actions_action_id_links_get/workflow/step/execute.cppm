export module blog_handlers:automations_automations_automation_id_actions_action_id_links_get_step;
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
import blog_handlers_middleware;
import :automations_operations;

export namespace blog::handlers::automations::automations_automation_id_actions_action_id_links_get::workflow {

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
  static constexpr std::string_view name{"automations_automations_automation_id_actions_action_id_links_get_execute"};
  [[nodiscard]] core::workflow::StepResponse<Output> invoke(const Input& in, core::workflow::WorkflowContext&) const {
   const std::string automationId=blog::api::automations::automation_id(in.get_path());
   const std::string actionId=blog::api::automations::suffix_id(in.get_path(), "/actions/");
   if (automationId.empty() || actionId.empty())
    return core::workflow::StepResponse<Output>{{{"status","422"},
      {"body",blog::api::ApiError{}.validation("automation_id and action_id are required","","automation_id")}}};

   auto& store=s_ctx->store();
   const auto actions=store.json_for("automation_actions", {
     {"filter[automation_id]",automationId},{"filter[action_id]",actionId}});
   if (actions=="[]")
    return core::workflow::StepResponse<Output>{{{"status","404"},
      {"body",blog::api::automations::ghost_error("NotFoundError","Action not found.")}}};

   const auto redirects=store.json_for("redirects", {{"filter[action_id]",actionId}});
   const auto revisions=store.json_for("automation_action_revisions", {{"filter[action_id]",actionId}});
   const auto clicks=store.json_for("members_click_events", {{"filter[action_id]",actionId}});
   std::string url=first_field(redirects,"url");
   if (url.empty()) url=first_field(revisions,"url");
   const auto record="{\"url\":"+core_serde::JsonCodec{}.encode(url)+
     ",\"clicked_count\":"+std::to_string(array_size(clicks))+"}";
   return core::workflow::StepResponse<Output>{{{"status","200"},
     {"body",local_mapping::wrap("data","["+record+"]")}}};
  }
  void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}
  static blog::BlogContext* s_ctx;
private:
  using Object=serde::Value::Object;
  using Array=serde::Value::Array;
  [[nodiscard]] static const Object* as_object(const serde::Value* v) {
   return v!=nullptr?std::get_if<Object>(&v->get()):nullptr;
  }
  [[nodiscard]] static const Array* as_array(const serde::Value* v) {
   return v!=nullptr?std::get_if<Array>(&v->get()):nullptr;
  }
  [[nodiscard]] static std::size_t array_size(std::string_view json) {
   const auto parsed=core_serde::JsonCodec{}.decode<serde::Value>(json);
   if (!parsed) return 0;
   const auto* arr=as_array(&*parsed);
   return arr==nullptr?0:arr->size();
  }
  [[nodiscard]] static std::string first_field(std::string_view json, std::string_view key) {
   const auto parsed=core_serde::JsonCodec{}.decode<serde::Value>(json);
   if (!parsed) return {};
   const auto* arr=as_array(&*parsed);
   if (arr==nullptr) return {};
   for (const auto& item:*arr) {
    const auto* object=as_object(&item);
    if (object==nullptr) continue;
    for (const auto& [name,value]:*object) {
     if (name==key) if (const auto* text=std::get_if<std::string>(&value.get())) return *text;
    }
   }
   return {};
  }
};
blog::BlogContext* ExecuteStep::s_ctx{nullptr};
}
