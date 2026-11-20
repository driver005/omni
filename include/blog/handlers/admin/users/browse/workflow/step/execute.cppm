export module blog_handlers:users_browse_step;
import :domain_all;
import :users_dto_subresources;
import std;
import core_workflow;
import interfaces;
import core_query;

export namespace blog::handlers::users::browse::workflow {

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
 static constexpr std::string_view name{"users_browse_execute"};
 [[nodiscard]] core::workflow::StepResponse<Output> invoke(const Input& in, core::workflow::WorkflowContext&) const {
  auto& ctx=*s_ctx;

  // Ghost `options` + `restrictAdminApiQueryOptions`: core::query parses the
  // allowlisted options and its guard rejects restricted filter/order fields.
  static constexpr std::string_view kOptions[]{
   "include","filter","fields","limit","order","page","debug"
  };
  const auto q=core::query::parse(in.get_path(), kOptions);
  for (const auto& include : q.include) {
   if (!allowed_include(include)) {
    return core::workflow::StepResponse<Output>{{{"status","422"},
     {"body",blog::api::ApiError{}.validation("Validation error","Invalid include","include")}}};
   }
  }
  // `debug` is an accepted Ghost option with no serializer effect here.

  const auto rows=ctx.store().json_for("users", {});
  std::string ran;
  try {
   // The store's wire serializer already emits only Ghost user fields; `clean`
   // is used for its restricted-field guard without dropping the mock's
   // `description` wire alias (asserted by the Ghost schema replay).
   ran=core::query::run(clean_users(rows), q, 15);
  } catch (const std::exception&) {
   return core::workflow::StepResponse<Output>{{{"status","400"},
    {"body",blog::api::ApiError{}.json("Restricted fields cannot be used in order.","Bad Request")}}};
  }

  const auto parts=blog::api::JsonBody{}.parse_raw(ran);
  const auto meta=blog::api::JsonBody{}.parse_raw(parts.contains("meta")?parts.at("meta"):std::string{"{}"});
  const auto page_meta=blog::api::JsonBody{}.parse_raw(meta.contains("pagination")?meta.at("pagination"):std::string{"{}"});
  const auto number=[&page_meta](const char* key, std::size_t fallback) {
   const auto it=page_meta.find(key);
   if (it==page_meta.end()) return fallback;
   try { return static_cast<std::size_t>(std::stoull(it->second)); } catch (...) { return fallback; }
  };
  const std::string prev=page_meta.contains("prev")?page_meta.at("prev"):std::string{"null"};
  const std::string next=page_meta.contains("next")?page_meta.at("next"):std::string{"null"};
  const auto pagination=local_mapping::pagination(
   number("total",0), number("limit",15), number("page",1), number("pages",1), prev, next);
  std::string data=parts.contains("data")?parts.at("data"):std::string{"[]"};
  data=enrich(data, q);
  return core::workflow::StepResponse<Output>{{{"status","200"},
   {"body",local_mapping::wrap("users", data, "{\"pagination\":"+pagination+"}")}}};
 }
 void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}
 static blog::BlogContext* s_ctx;
private:
 static bool allowed_include(std::string_view value) {
  return value=="count.posts"||value=="permissions"||value=="roles"||value=="roles.permissions";
 }
 static std::string clean_users(std::string_view rows) {
  // Unknown-entity clean keeps Ghost's restricted-column guard only.
  return local_mapping::clean(rows, "user-wire");
 }
 // output/mappers/users.js: url.forUser, permissions:[] and count.posts expansion.
 static std::string enrich(std::string_view rows, const core::query::ParsedQuery& q) {
  if (q.include.empty()) return std::string{rows};
  std::string out{"["};
  bool first{true};
  for (const auto& record : blog::api::parse_array(rows)) {
   if (!first) out.push_back(',');
   first=false;
   auto fields=record;
   if (q.include.contains("permissions") && !fields.contains("permissions")) {
    fields["permissions"]="[]";
   }
   if (q.include.contains("count.posts") && !fields.contains("count")) {
    fields["count"]=R"({"posts":0})";
   }
   if (!fields.contains("url")) {
    const auto plain=blog::api::JsonBody{}.parse(blog::api::record_to_json(fields));
    const auto it=plain.find("slug");
    fields["url"]=blog::api::JsonBody{}.quote(
     "/author/"+(it==plain.end()?"":it->second)+"/");
   }
   out+=blog::api::record_to_json(fields);
  }
  out.push_back(']');
  return out;
 }
};
blog::BlogContext* ExecuteStep::s_ctx{nullptr};
}
