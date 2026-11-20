export module blog_handlers:tiers_browse_step;
import :domain_all;
import std;
import core_workflow;
import core_query;
import core_serde;
import blog_handlers_middleware;
import interfaces;

export namespace blog::handlers::tiers::browse::workflow {

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
 static constexpr std::string_view name{"tiers_browse_execute"};
 [[nodiscard]] core::workflow::StepResponse<Output> invoke(const Input& in, core::workflow::WorkflowContext&) const {
  auto& ctx=*s_ctx;

  // Ghost `options` (tiers.js:15) + NQL input serializer (input/tiers.js:60-72).
  static constexpr std::string_view kOptions[]{
   "limit","fields","filter","order","debug","page","include"};
  const std::string_view path{in.get_path()};

  // core_query::parse drops syntactically invalid options; Ghost rejects them
  // (BadRequestError "The NQL filter you passed was invalid."). Validate first
  // with the shared validator so the error is observable.
  if (bad_filter(path))
   return failure("400","The NQL filter you passed was invalid.");
  if (bad_option(path,kOptions))
   return invalid("options","Invalid query option value");

  auto q=core::query::parse(in.get_path(),kOptions);

  // validation.options.include.values = ['requirements'] (tiers.js:5,16).
  for (const auto& inc : q.include)
   if (inc!="requirements") return invalid("include","Invalid include value");

  // Content API can only see active tiers; the input serializer ANDs
  // {active:true} into the NQL filter (input/tiers.js:12-27,74-77).
  const bool content=in.find_header(blog::handlers::middleware::kGhostContextHeader)=="CONTENT_API";
  if (content) q.filters.emplace_back("active","true");

  // TiersService.browse ignores limit/page/order/fields and fabricates a fixed
  // page over the whole filtered set (tiers-api.js:53-73, tier-repository.js:115-124).
  q.limit=std::numeric_limits<std::size_t>::max();
  q.page=1; q.offset=0; q.order.clear(); q.fields.clear();

  const auto rows=ctx.store().json_for("tiers",{});
  const auto cleaned=local_mapping::clean(rows,"tier");
  const auto serialized=serialize_tiers(cleaned);

  std::string ran;
  try {
   // run() applies the restricted-field guard, NQL filters, and Ghost-shaped
   // meta.pagination (page/pages 1, limit=total, prev/next null under limit=all).
   ran=core::query::run(serialized,q,15);
  } catch (const std::exception&) {
   return failure("400","The NQL filter you passed was invalid.");
  }

  const auto parts=blog::api::JsonBody{}.parse_raw(ran);
  const auto meta=blog::api::JsonBody{}.parse_raw(
   parts.contains("meta")?parts.at("meta"):std::string{"{}"});
  const auto page_meta=blog::api::JsonBody{}.parse_raw(
   meta.contains("pagination")?meta.at("pagination"):std::string{"{}"});
  const auto number=[&page_meta](const char* key,std::size_t fallback){
   const auto it=page_meta.find(key);
   if(it==page_meta.end()) return fallback;
   try{ return static_cast<std::size_t>(std::stoull(it->second)); }catch(...){ return fallback; }
  };
  const std::string prev=page_meta.contains("prev")?page_meta.at("prev"):std::string{"null"};
  const std::string next=page_meta.contains("next")?page_meta.at("next"):std::string{"null"};
  const auto pagination=local_mapping::pagination(
   number("total",0),number("limit",0),number("page",1),number("pages",1),prev,next);
  const auto data=parts.contains("data")?parts.at("data"):std::string{"[]"};
  const auto body=local_mapping::wrap("tiers",data,"{\"pagination\":"+pagination+"}");
  return core::workflow::StepResponse<Output>{{{"status","200"},{"body",body}}};
 }
 void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}
 static blog::BlogContext* s_ctx;
private:
 using Object=serde::Value::Object;

 static const serde::Value* find(const Object& object,std::string_view key){
  for(const auto& [k,v]: object) if(k==key) return &v;
  return nullptr;
 }

 /** @brief Ghost serializeTier (output/tiers.js:43-83): coerce non-array
  *  benefits to null and drop currency/prices on free tiers. */
 [[nodiscard]] static std::string serialize_tiers(std::string_view rows){
  auto value=core_serde::JsonCodec{}.decode<serde::Value>(rows);
  if(!value) return std::string{rows};
  auto* array=std::get_if<serde::Value::Array>(&value->get());
  if(array==nullptr) return std::string{rows};
  for(auto& item:*array){
   auto* object=std::get_if<Object>(&item.get());
   if(object==nullptr) continue;
   const bool is_free=[&]{
    const auto* type=find(*object,"type");
    const auto* text=type==nullptr?nullptr:std::get_if<std::string>(&type->get());
    return text!=nullptr&&*text=="free";
   }();
   const bool bad_benefits=[&]{
    const auto* benefits=find(*object,"benefits");
    return benefits!=nullptr&&std::get_if<serde::Value::Array>(&benefits->get())==nullptr;
   }();
   if(!is_free&&!bad_benefits) continue;
   Object rewritten;
   for(const auto& [key,val]: *object){
    if(is_free&&(key=="currency"||key=="monthly_price"||key=="yearly_price")) continue;
    if(key=="benefits"&&bad_benefits){ rewritten[key]=serde::Value{serde::Value::Null}; continue; }
    rewritten[key]=val;
   }
   item=serde::Value{std::move(rewritten)};
  }
  return core_serde::JsonCodec{}.encode(*value);
 }

 template<typename Range>
 static bool bad_option(std::string_view path,const Range& allowed){
  const auto qpos=path.find('?');
  if(qpos==std::string_view::npos) return false;
  auto query=path.substr(qpos+1);
  while(!query.empty()){
   const auto amp=query.find('&');
   const auto pair=query.substr(0,amp==std::string_view::npos?std::string_view::npos:amp);
   const auto eq=pair.find('=');
   const std::string key{ eq==std::string_view::npos?pair:pair.substr(0,eq) };
   const std::string value{ eq==std::string_view::npos?std::string_view{}:pair.substr(eq+1) };
   const bool bracket=key.starts_with("filter[")&&key.ends_with("]");
   const std::string base=bracket?std::string{"filter"}:key;
   if(std::ranges::find(allowed,std::string_view{base})!=std::ranges::end(allowed)
      &&!core::query::valid_option(key,value)) return true;
   if(amp==std::string_view::npos) break;
   query.remove_prefix(amp+1);
  }
  return false;
 }

 static bool bad_filter(std::string_view path){
  const auto qpos=path.find('?');
  if(qpos==std::string_view::npos) return false;
  auto query=path.substr(qpos+1);
  while(!query.empty()){
   const auto amp=query.find('&');
   const auto pair=query.substr(0,amp==std::string_view::npos?std::string_view::npos:amp);
   const auto eq=pair.find('=');
   const std::string key{ eq==std::string_view::npos?pair:pair.substr(0,eq) };
   const std::string value{ eq==std::string_view::npos?std::string_view{}:pair.substr(eq+1) };
   if((key=="filter"||(key.starts_with("filter[")&&key.ends_with("]")))
      &&!core::query::valid_option(key,value)) return true;
   if(amp==std::string_view::npos) break;
   query.remove_prefix(amp+1);
  }
  return false;
 }

 static core::workflow::StepResponse<Output> invalid(std::string_view property,std::string_view context){
  return core::workflow::StepResponse<Output>{{{"status","422"},
   {"body",blog::api::ApiError{}.validation("Validation error",context,property)}}};
 }
 static core::workflow::StepResponse<Output> failure(std::string_view code,std::string_view message){
  return core::workflow::StepResponse<Output>{{{"status",std::string{code}},
   {"body",blog::api::ApiError{}.json(message,"Bad Request")}}};
 }
};
blog::BlogContext* ExecuteStep::s_ctx{nullptr};
}
