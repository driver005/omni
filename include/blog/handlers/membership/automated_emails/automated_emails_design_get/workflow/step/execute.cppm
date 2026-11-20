export module blog_handlers:automated_emails_automated_emails_design_get_step;
import :domain_all;
import std;
import core_workflow;
import core_serde;
import interfaces;

export namespace blog::handlers::automated_emails::automated_emails_design_get::workflow {

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
 using Input=interfaces::io::IRequest&;
 using Output=std::map<std::string,std::string>;
 static constexpr std::string_view name{"automated_emails_automated_emails_design_get_execute"};
 [[nodiscard]] core::workflow::StepResponse<Output> invoke(const Input&, core::workflow::WorkflowContext&) const {
   auto& ctx=*s_ctx;
   // Ghost resolveDefaultDesign(): EmailDesignSetting.findOne({slug:
   // DEFAULT_EMAIL_DESIGN_SETTING_SLUG}) -> InternalServerError (500) on a
   // missing row. No options/data/validation are declared by the controller.
   const auto rows=ctx.store().json_for("email_design_settings", {{"filter","slug:default-automated-email"}});
   const auto parsed=core_serde::JsonCodec{}.decode<serde::Value>(rows);
   if (parsed) {
     if (const auto* arr=std::get_if<serde::Value::Array>(&parsed->get()); arr!=nullptr && !arr->empty()) {
       // Ghost's output/default.js maps one model through toJSON(); the shared
       // mapping layer applies the restricted-field guard (token/secret/password)
       // before the row is emitted. No meta.pagination for a single resource.
       const std::string body=local_mapping::clean(
         core_serde::JsonCodec{}.encode(arr->front()), "email_design_setting");
       return core::workflow::StepResponse<Output>{{{"status","200"},{"body",body}}};
     }
   }
   return core::workflow::StepResponse<Output>{{{"status","500"},
     {"body",blog::api::ApiError{}.json("Default automated email design setting not found.","Internal Server Error")}}};
 }
 void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}
 static blog::BlogContext* s_ctx;
};
blog::BlogContext* ExecuteStep::s_ctx{nullptr};
}
