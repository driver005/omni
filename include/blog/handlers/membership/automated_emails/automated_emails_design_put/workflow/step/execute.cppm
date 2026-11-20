export module blog_handlers:automated_emails_automated_emails_design_put_step;
import :domain_all;
import std;
import core_workflow;
import core_serde;
import interfaces;

export namespace blog::handlers::automated_emails::automated_emails_design_put::workflow {

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
 static constexpr std::string_view name{"automated_emails_automated_emails_design_put_execute"};
 [[nodiscard]] core::workflow::StepResponse<Output> invoke(const Input& in, core::workflow::WorkflowContext&) const {
   auto& ctx=*s_ctx;
   std::string raw{in.body_as_string()};
   if (raw.empty()) raw="{}";
   std::string inner="{}";
   bool has_slug=false;
   Object sender;
   const auto parsed=core_serde::JsonCodec{}.decode<serde::Value>(raw);
   if (parsed) {
     if (const auto* root=std::get_if<Object>(&parsed->get())) {
       const auto* data=value_for(root,"data");
       if (data!=nullptr) {
         if (const auto* dobj=std::get_if<Object>(&data->get())) {
           const auto* arr=value_for(dobj,"automated_email_design");
           if (arr!=nullptr) {
             if (const auto* list=std::get_if<serde::Value::Array>(&arr->get())) {
               if (!list->empty()) {
                 if (const auto* item=std::get_if<Object>(&list->front().get())) {
                   Object stripped;
                   for (const auto& [key,value] : *item) {
                     if (key=="id") continue;          // delete data.id
                     if (key=="slug") has_slug=true;   // immutable
                     if (key=="sender_email" || key=="sender_reply_to") sender[std::string{key}]=value;
                     stripped[std::string{key}]=value;
                   }
                   inner=core_serde::JsonCodec{}.encode(serde::Value{stripped});
                 }
               }
             }
           }
         }
       }
     }
   }
   // Ghost: reject any slug field with ValidationError "The slug field cannot be modified."
   if (has_slug) {
     return core::workflow::StepResponse<Output>{{{"status","422"},
       {"body",blog::api::ApiError{}.validation("The slug field cannot be modified.","","slug")}}};
   }
   // Ghost: emailAddressService.init() + validateEmailSenderFields(data) run BEFORE
   // resolveDefaultDesign. Self-hosted config (managedEmail disabled) allows any
   // structurally valid address; an invalid address yields a 422 ValidationError.
   // Managed-email sending-domain / verification policy needs the shared email
   // address service and is recorded as a new-partition need.
   if (const auto err=validate_senders(&sender); !err.empty()) {
     return core::workflow::StepResponse<Output>{{{"status","422"},{"body",err}}};
   }
   const auto defaults=ctx.store().json_for("email_design_settings", {{"filter","slug:default-automated-email"}});
   const std::string defaultId=first_id(defaults);
   if (defaultId.empty()) {
     return core::workflow::StepResponse<Output>{{{"status","500"},
       {"body",blog::api::ApiError{}.json("Default automated email design setting not found.","Internal Server Error")}}};
   }
   (void)ctx.store().edit("email_design_settings", defaultId, inner);
   // Ghost returns the full edited EmailDesignSetting model (output/default.js).
   const auto updated=ctx.store().json_for("email_design_settings", {{"filter","slug:default-automated-email"}});
   const auto reparsed=core_serde::JsonCodec{}.decode<serde::Value>(updated);
   std::string body="{}";
   if (reparsed) {
     if (const auto* arr=std::get_if<serde::Value::Array>(&reparsed->get()); arr!=nullptr && !arr->empty()) {
       body=local_mapping::clean(core_serde::JsonCodec{}.encode(arr->front()), "email_design_setting");
     }
   }
   return core::workflow::StepResponse<Output>{{{"status","200"},{"body",body}}};
 }
 void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}
 static blog::BlogContext* s_ctx;
private:
 using Object=serde::Value::Object;
 static const serde::Value* value_for(const Object* object, std::string_view key) {
   if (object==nullptr) return nullptr;
   for (const auto& [name,value] : *object) if (name==key) return &value;
   return nullptr;
 }
 static std::string field(const Object* object, std::string_view key) {
   const auto* value=value_for(object,key);
   if (value==nullptr) return {};
   if (const auto* text=std::get_if<std::string>(&value->get())) return *text;
   return {};
 }
 static bool valid_email(std::string_view value) {
   const auto at=value.find('@');
   if (at==std::string_view::npos || at==0 || at+1>=value.size()) return false;
   if (value.find(' ',0)!=std::string_view::npos) return false;
   const auto domain=value.substr(at+1);
   return domain.find('.')!=std::string_view::npos && domain.front()!='.' && domain.back()!='.';
 }
 static std::string validate_senders(const Object* sender) {
   for (const auto key : {"sender_email","sender_reply_to"}) {
     const std::string value=field(sender,key);
     if (value.empty()) continue;
     if (!valid_email(value)) {
       return blog::api::ApiError{}.validation(
         "You cannot set "+std::string{key}+" to "+value,"",key);
     }
   }
   return {};
 }
 static std::string first_id(std::string_view json) {
   const auto parsed=core_serde::JsonCodec{}.decode<serde::Value>(json);
   if (!parsed) return {};
   if (const auto* list=std::get_if<serde::Value::Array>(&parsed->get())) {
     if (!list->empty()) {
       if (const auto* object=std::get_if<Object>(&list->front().get())) return field(object,"id");
     }
   }
   return {};
 }
};
blog::BlogContext* ExecuteStep::s_ctx{nullptr};
}
