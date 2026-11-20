export module blog_handlers:automated_emails_create_step;
import :domain_all;
import std;
import core_workflow;
import core_serde;
import :outbox_shim;
import interfaces;

export namespace blog::handlers::automated_emails::create::workflow {

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
 static constexpr std::string_view name{"automated_emails_create_execute"};
 [[nodiscard]] core::workflow::StepResponse<Output> invoke(const Input& in, core::workflow::WorkflowContext&) const {
   auto& ctx=*s_ctx;
   std::string raw{in.body_as_string()};
   if (raw.empty()) raw="{}";
   Object automation;
   Object email;
   Object sender;
   std::string slug;
   const auto parsed=core_serde::JsonCodec{}.decode<serde::Value>(raw);
   if (parsed) {
     if (const auto* root=std::get_if<Object>(&parsed->get())) {
       const auto* data=value_for(root,"data");
       if (data!=nullptr) {
         if (const auto* dobj=std::get_if<Object>(&data->get())) {
           const auto* arr=value_for(dobj,"automated_emails");
           if (arr!=nullptr) {
             if (const auto* list=std::get_if<serde::Value::Array>(&arr->get())) {
               if (!list->empty()) {
                 if (const auto* item=std::get_if<Object>(&list->front().get())) {
                   automation=project(item,{"status","name","slug"});
                   email=project(item,{"subject","lexical","email_design_setting_id"});
                   sender=project(item,{"sender_name","sender_email","sender_reply_to"});
                   slug=field(item,"slug");
                 }
               }
             }
           }
         }
       }
     }
   }

   // validators/input/automated_emails.js#add (validateAutomatedEmail, required slug).
   if (!valid_name(field(&automation,"name")))
     return invalid("Name must be one of: Free member welcome flow, Paid member welcome flow","name");
   const std::string status=field(&automation,"status");
   if (!status.empty() && status!="inactive" && status!="active")
     return invalid("Status must be one of: inactive, active","status");
   if (!valid_slug(slug))
     return invalid("Slug must be one of: member-welcome-email-free, member-welcome-email-paid","slug");
   const std::string lexical=field(&email,"lexical");
   if (!lexical.empty() && !valid_json(lexical))
     return invalid("Lexical must be a valid JSON string","lexical");
   if (const auto err=validate_senders(&sender); !err.empty())
     return core::workflow::StepResponse<Output>{{{"status","422"},{"body",err}}};

   automation["description"]=description_for(slug);
   const std::string created_at=now_iso();

   // Ghost models.Base.transaction(): four ordered writes, staged as one outbox
   // transaction. ResourceStore has no SQL transaction primitive (accepted
   // data-layer residual); the outbox transaction makes the side effects atomic
   // and observable through drain().
   auto& outbox=blog::handlers::outbox::global_outbox();
   outbox.begin_transaction();
   const auto automationResult=ctx.store().create("automations",
     core_serde::JsonCodec{}.encode(serde::Value{automation}));
   const std::string automation_id=first_id(automationResult.body());
   const auto rule=rule_for(slug, automation_id, created_at);
   ctx.store().resources().create("automation_trigger_rules", rule);
   email["welcome_email_automation_id"]=serde::Value{automation_id};
   email["delay_days"]=serde::Value{0};
   const auto emailResult=ctx.store().create("welcome_email_automated_emails",
     core_serde::JsonCodec{}.encode(serde::Value{email}));
   static_cast<void>(emailResult);

   // updateEmailDesignSenderFields() targets the email's own design setting id;
   // fall back to the shared default when the email does not declare one.
   if (has_any(sender)) {
     std::string design_id=field(&email,"email_design_setting_id");
     if (design_id.empty()) design_id=default_design_id(ctx);
     if (!design_id.empty()) {
       (void)ctx.store().edit("email_design_settings", design_id,
         core_serde::JsonCodec{}.encode(serde::Value{sender}));
     }
   }
   outbox.emit(blog::handlers::outbox::Event{
     "automation.created",
     "{\"id\":\""+automation_id+"\",\"slug\":\""+slug+"\"}",
     automation_id});
   outbox.emit(blog::handlers::outbox::Event{
     "welcome_email.created",
     "{\"automation_id\":\""+automation_id+"\"}",
     automation_id});
   outbox.commit();
   [[maybe_unused]] const auto drained=outbox.drain();

   // Ghost flattenAutomation(): full 12-field shape in the default envelope, 201.
   const Object a=first_object(automationResult.body());
   const auto defaults=ctx.store().json_for("email_design_settings", {{"filter","slug:default-automated-email"}});
   const Object design=first_object(defaults);
   Object flat;
   if (!a.empty()) {
     flat["id"]=a.at("id");
     for (const auto& [key,val] : automation) flat[key]=val;
   }
   flat["subject"]=value_or_null(&email,"subject");
   flat["lexical"]=value_or_null(&email,"lexical");
   for (const auto key : {"sender_name","sender_email","sender_reply_to"}) {
     const auto* value=value_for(&sender,key);
     flat[key]= value!=nullptr ? *value : value_or_null(&design,key);
   }
   flat["email_design_setting_id"]=serde::Value{
     field(&email,"email_design_setting_id").empty()
       ? field(&design,"id") : field(&email,"email_design_setting_id")};
   flat["created_at"]=serde::Value{created_at};
   flat["updated_at"]=serde::Value{created_at};
   const auto body=local_mapping::wrap("automated_emails",
     core_serde::JsonCodec{}.encode(serde::Value{flat}));
   return core::workflow::StepResponse<Output>{{{"status","201"},{"body",body}}};
 }
 void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}
 static blog::BlogContext* s_ctx;
private:
 using Object=serde::Value::Object;
 using Array=serde::Value::Array;

 static core::workflow::StepResponse<Output> invalid(std::string_view message, std::string_view property) {
   return core::workflow::StepResponse<Output>{{{"status","422"},
     {"body",blog::api::ApiError{}.validation(message,"",property)}}};
 }
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
 static serde::Value value_or_null(const Object* object, std::string_view key) {
   const auto* value=value_for(object,key);
   return value!=nullptr ? *value : serde::Value{};
 }
 static Object project(const Object* object, std::initializer_list<std::string_view> keys) {
   Object out;
   for (const auto key : keys) {
     const auto* value=value_for(object,key);
     if (value!=nullptr) out[std::string{key}]=*value;
   }
   return out;
 }
 static bool has_any(const Object& object) { return !object.empty(); }
 static bool valid_name(std::string_view name) {
   return name=="Free member welcome flow" || name=="Paid member welcome flow";
 }
 static bool valid_slug(std::string_view slug) {
   return slug=="member-welcome-email-free" || slug=="member-welcome-email-paid";
 }
 static bool valid_json(std::string_view text) {
   return core_serde::JsonCodec{}.decode<serde::Value>(text).has_value();
 }
 static bool valid_email(std::string_view value) {
   const auto at=value.find('@');
   if (at==std::string_view::npos || at==0 || at+1>=value.size()) return false;
   if (value.find(' ')!=std::string_view::npos) return false;
   const auto domain=value.substr(at+1);
   return domain.find('.')!=std::string_view::npos && domain.front()!='.' && domain.back()!='.';
 }
 static std::string validate_senders(const Object* sender) {
   for (const auto key : {"sender_email","sender_reply_to"}) {
     const std::string value=field(sender,key);
     if (value.empty()) continue;
     if (!valid_email(value))
       return blog::api::ApiError{}.validation("You cannot set "+std::string{key}+" to "+value,"",key);
   }
   return {};
 }
 static std::string description_for(std::string_view slug) {
   if (slug=="member-welcome-email-free") return "Welcome new free members after they sign up.";
   if (slug=="member-welcome-email-paid") return "Welcome new paid members after they start their subscription.";
   return {};
 }
 static std::string rule_for(std::string_view slug, std::string_view automation_id, std::string_view created_at) {
   const std::string trigger=slug=="member-welcome-email-free" ? "free_signup" : "paid_subscription_started";
   const std::string scope=slug=="member-welcome-email-free" ? "null" : "\"any_paid_tier\"";
   const std::string id=[&]{
     std::string out;
     for (std::size_t i=0;i<24;++i)
       out.push_back("0123456789abcdef"[(static_cast<unsigned char>(automation_id.empty()?'0':automation_id[i%automation_id.size()])+i)%16]);
     return out;
   }();
   return "{\"id\":\""+id+"\",\"trigger_type\":\""+trigger+"\",\"scope\":"+scope+
     ",\"product_id\":null,\"automation_id\":\""+std::string{automation_id}+"\",\"created_at\":\""+std::string{created_at}+"\"}";
 }
 static std::string now_iso() { return "2026-01-01T00:00:00.000Z"; }
 static std::string first_id(std::string_view json) {
   const auto parsed=core_serde::JsonCodec{}.decode<serde::Value>(json);
   if (!parsed) return {};
   if (const auto* root=std::get_if<Object>(&parsed->get())) {
     for (const auto& entry : *root) {
       if (const auto* list=std::get_if<Array>(&entry.second.get())) {
         if (!list->empty()) {
           if (const auto* object=std::get_if<Object>(&list->front().get())) return field(object,"id");
         }
       }
     }
   }
   return {};
 }
 static Object first_object(std::string_view json) {
   const auto parsed=core_serde::JsonCodec{}.decode<serde::Value>(json);
   if (parsed) {
     if (const auto* root=std::get_if<Object>(&parsed->get())) {
       for (const auto& entry : *root) {
         if (const auto* list=std::get_if<Array>(&entry.second.get())) {
           if (!list->empty()) {
             if (const auto* object=std::get_if<Object>(&list->front().get())) return *object;
           }
         }
       }
     }
   }
   return {};
 }
 static std::string default_design_id(blog::BlogContext& ctx) {
   const auto rows=ctx.store().json_for("email_design_settings", {{"filter","slug:default-automated-email"}});
   const Object design=first_object(rows);
   return design.empty() ? std::string{} : field(&design,"id");
 }
};
blog::BlogContext* ExecuteStep::s_ctx{nullptr};
}
