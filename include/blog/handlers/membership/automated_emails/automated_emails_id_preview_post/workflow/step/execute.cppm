export module blog_handlers:automated_emails_automated_emails_id_preview_post_step;
import :domain_all;
import std;
import core_workflow;
import core_serde;
import interfaces;

export namespace blog::handlers::automated_emails::automated_emails_id_preview_post::workflow {
class ExecuteStep {
public:
 using Input=interfaces::io::IRequest&;
 using Output=std::map<std::string,std::string>;
 static constexpr std::string_view name{"automated_emails_automated_emails_id_preview_post_execute"};
 [[nodiscard]] core::workflow::StepResponse<Output> invoke(const Input& in, core::workflow::WorkflowContext&) const {
   auto& ctx=*s_ctx;
   const std::string id=parent_segment(in.get_path());
   if (id.empty()) return invalid("id is required","id");

   std::string raw{in.body_as_string()};
   if (raw.empty()) raw="{}";
   Object data;
   const auto parsed=core_serde::JsonCodec{}.decode<serde::Value>(raw);
   if (parsed) {
     if (const auto* root=std::get_if<Object>(&parsed->get())) {
       const auto* node=value_for(root,"data");
       if (node!=nullptr) if (const auto* dobj=std::get_if<Object>(&node->get())) data=*dobj;
       if (data.empty()) data=*root;
     }
   }
   const std::string subject=field(&data,"subject");
   const std::string lexical=field(&data,"lexical");
   if (subject.empty()) return invalid("Subject is required","subject");
   if (lexical.empty()) return invalid("Email content is required","lexical");
   if (!core_serde::JsonCodec{}.decode<serde::Value>(lexical))
     return invalid("Lexical must be a valid JSON string","lexical");

   // #renderAutomationEmailPreview(requireWelcomeEmail:true): Automation.findOne
   // + welcomeEmailAutomatedEmail relation; missing automation/email -> NotFound.
   const auto rows=ctx.store().json_for("automated_emails", {{"filter","id:"+id}});
   const auto parsed_rows=core_serde::JsonCodec{}.decode<serde::Value>(rows);
   bool found=false;
   if (parsed_rows) if (const auto* list=std::get_if<serde::Value::Array>(&parsed_rows->get())) {
     for (const auto& item : *list) {
       if (const auto* object=std::get_if<Object>(&item.get())) {
         if (field(object,"id")==id) found=true;
       }
     }
   }
   if (!found) {
     return core::workflow::StepResponse<Output>{{{"status","404"},
       {"body",blog::api::ApiError{}.json("No member welcome email found","Not Found")}}};
   }

   // Ghost #renderer.render(): substitute test-member tokens in the subject and
   // turn Lexical text nodes into HTML/plaintext. Test member: Jamie Larson.
   const std::string rendered_subject=render_subject(subject, "jamie@example.com");
   const auto [html, plaintext]=render_lexical(lexical);
   const auto enc=[](std::string_view value){ return core_serde::JsonCodec{}.encode(serde::Value{std::string{value}}); };
   const std::string body="{\"html\":"+enc(html)+",\"plaintext\":"+enc(plaintext)+
     ",\"subject\":"+enc(rendered_subject)+"}";
   return core::workflow::StepResponse<Output>{{{"status","200"},{"body",body}}};
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
 static std::string parent_segment(std::string_view path) {
   const auto clean=path.substr(0, path.find('?'));
   const auto last=clean.rfind('/');
   if (last==std::string_view::npos || last==0) return {};
   const auto before=clean.substr(0,last);
   const auto prev=before.rfind('/');
   return std::string{before.substr(prev==std::string_view::npos?0:prev+1)};
 }
 static std::string replace_all(std::string text, std::string_view token, std::string_view value) {
   for (;;) {
     const auto pos=text.find(token);
     if (pos==std::string::npos) break;
     text.replace(pos, token.size(), value);
   }
   return text;
 }
 static std::string render_subject(std::string subject, std::string_view email) {
   subject=replace_all(subject,"%%{first_name}%%","Jamie");
   subject=replace_all(subject,"%%{name}%%","Jamie Larson");
   subject=replace_all(subject,"%%{email}%%",email);
   subject=replace_all(subject,"%%{uuid}%%","00000000-0000-4000-8000-000000000000");
   return subject;
 }
 static void collect_text(const serde::Value& node, std::vector<std::string>& out) {
   if (const auto* object=std::get_if<Object>(&node.get())) {
     if (field(object,"type")=="text") {
       const std::string text=field(object,"text");
       if (!text.empty()) out.push_back(text);
     }
     for (const auto& [key,value] : *object) {
       if (key=="children" || key=="root") collect_text(value,out);
     }
   } else if (const auto* array=std::get_if<Array>(&node.get())) {
     for (const auto& item : *array) collect_text(item,out);
   }
 }
 static std::pair<std::string,std::string> render_lexical(std::string_view lexical) {
   std::vector<std::string> parts;
   if (const auto doc=core_serde::JsonCodec{}.decode<serde::Value>(lexical)) collect_text(*doc,parts);
   std::string plaintext;
   for (std::size_t i=0;i<parts.size();++i) plaintext+=(i?"\n":"")+parts[i];
   std::string html;
   for (const auto& part : parts) html+="<p>"+part+"</p>";
   if (html.empty()) html="<p></p>";
   return {html, plaintext};
 }
};
blog::BlogContext* ExecuteStep::s_ctx{nullptr};
}
