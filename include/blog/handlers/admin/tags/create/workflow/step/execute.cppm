export module blog_handlers:tags_create_step;
import :domain_all;
import std;
import core_workflow;
import core_serde;
import core_query;
import :outbox_shim;
import interfaces;

export namespace blog::handlers::tags::create::workflow {

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
 using Input=interfaces::io::IRequest&; using Output=std::map<std::string,std::string>;
 static constexpr std::string_view name{"tags_create_execute"};
 [[nodiscard]] core::workflow::StepResponse<Output> invoke(const Input& in, core::workflow::WorkflowContext&) const {
  auto& ctx=*s_ctx;

  constexpr std::array<std::string_view,1> kOptions{"include"};
  const auto q=core::query::parse(in.get_path(), kOptions);
  for(const auto& inc : q.include) {
   if(inc!="count.posts")
    return core::workflow::StepResponse<Output>{{{"status","422"},
      {"body",blog::api::ApiError{}.validation("Validation error","include invalid","include")}}};
  }

  const auto raw=in.body_as_string();
  const std::string inner=unwrap(raw);
  const auto parsed=core_serde::JsonCodec{}.decode<serde::Value>(inner);
  const auto* obj=parsed?std::get_if<serde::Value::Object>(&parsed->get()):nullptr;
  const std::string name=obj?scalar(*obj,"name"):std::string{};
  if(name.empty())
   return core::workflow::StepResponse<Output>{{{"status","422"},
     {"body",blog::api::ApiError{}.validation("Validation error","name is required","name")}}};

  const auto result=ctx.store().create("tags", inner);
  if(!result.ok())
   return core::workflow::StepResponse<Output>{{{"status",std::to_string(result.status())},
     {"body",result.body()}}};

  const std::string body=clean_wrap(result.body());
  const std::string id=extract_id(result.body());
  // tag.added == Ghost model onCreated emitChange('tag.added')
  blog::handlers::outbox::global_outbox().emit(blog::handlers::outbox::Event{"tag.added", body, id});
  return core::workflow::StepResponse<Output>{{{"status",std::to_string(result.status())},
    {"body",body},{"location",location(in,id)}}};
 }
 void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}
 static blog::BlogContext* s_ctx;
private:
 static const serde::Value* lookup(const serde::Value::Object& obj, std::string_view key){
  for(const auto& [k,v]: obj) if(k==key) return &v;
  return nullptr;
 }
 static std::string scalar(const serde::Value::Object& obj, std::string_view key){
  const auto* v=lookup(obj,key);
  if(v==nullptr) return {};
  if(const auto* text=std::get_if<std::string>(&v->get())) return *text;
  return {};
 }
 static std::string unwrap(std::string_view body){
  if(body.empty()) return "{}";
  const auto parsed=core_serde::JsonCodec{}.decode<serde::Value>(body);
  if(!parsed) return std::string{body};
  const auto* root=std::get_if<serde::Value::Object>(&parsed->get());
  if(root==nullptr) return std::string{body};
  const serde::Value* tags=lookup(*root,"tags");
  if(tags==nullptr){
   const auto* data=lookup(*root,"data");
   if(data!=nullptr){
    const auto* dobj=std::get_if<serde::Value::Object>(&data->get());
    if(dobj!=nullptr) tags=lookup(*dobj,"tags");
   }
  }
  if(tags==nullptr) return std::string{body};
  const auto* array=std::get_if<serde::Value::Array>(&tags->get());
  if(array==nullptr || array->empty()) return std::string{body};
  return core_serde::JsonCodec{}.encode((*array)[0]);
 }
 static std::string clean_wrap(std::string_view body){
  const auto parsed=core_serde::JsonCodec{}.decode<serde::Value>(body);
  if(!parsed) return std::string{body};
  const auto* root=std::get_if<serde::Value::Object>(&parsed->get());
  if(root==nullptr) return std::string{body};
  const auto* tags=lookup(*root,"tags");
  if(tags==nullptr) return std::string{body};
  return local_mapping::wrap("tags",
      local_mapping::clean(core_serde::JsonCodec{}.encode(*tags), "tag"));
 }
 static std::string extract_id(std::string_view body){
  const auto parsed=core_serde::JsonCodec{}.decode<serde::Value>(body);
  if(!parsed) return {};
  const auto* root=std::get_if<serde::Value::Object>(&parsed->get());
  if(root==nullptr) return {};
  const auto* tags=lookup(*root,"tags");
  if(tags==nullptr) return {};
  const auto* array=std::get_if<serde::Value::Array>(&tags->get());
  if(array==nullptr || array->empty()) return {};
  const auto* obj=std::get_if<serde::Value::Object>(&(*array)[0].get());
  if(obj==nullptr) return {};
  const auto* id=lookup(*obj,"id");
  if(id==nullptr) return {};
  const auto encoded=core_serde::JsonCodec{}.encode(*id);
  if(encoded.size()>=2 && encoded.front()=='"' && encoded.back()=='"')
   return encoded.substr(1,encoded.size()-2);
  return encoded;
 }
 static std::string location(const Input& in, std::string_view id){
  if(id.empty()) return {};
  const std::string host{in.find_header("host")};
  if(host.empty()) return {};
  return "https://"+host+"/ghost/api/admin/tags/"+std::string{id}+"/";
 }
};
blog::BlogContext* ExecuteStep::s_ctx{nullptr};
}
