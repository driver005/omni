export module blog_handlers:actions_browse_step;
import :domain_all;
import std;
import core_workflow;
import core_query;
import interfaces;

export namespace blog::handlers::actions::browse::workflow {

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
 static constexpr std::string_view name{"actions_browse_execute"};
 static constexpr std::string_view kDocName{"actions"};

 // Ghost actions.js:12 options: ['page','limit','fields','include','filter'].
 // No `order` and no `filter[x]` -> Frame.configure (frame.ts:66-76) would
 // ignore them; `allowlisted_query` enforces that before core::query::parse.
 static constexpr std::string_view kOptions[]{
  "page","limit","fields","include","filter"
 };

 [[nodiscard]] core::workflow::StepResponse<Output> invoke(const Input& in, core::workflow::WorkflowContext&) const {
  const std::string query = allowlisted_query(in.get_path());

  // Ghost global option validators (validators/input/all.ts:26-44):
  // page /^\d+$/, limit /^(?:\d+|all)$/. A present-but-invalid option is a
  // 422 ValidationError, not a silently dropped parameter.
  if (const auto context = invalid_option(query, "page"); !context.empty())
   return invalid("page", context);
  if (const auto context = invalid_option(query, "limit"); !context.empty())
   return invalid("limit", context);

  const auto q = core::query::parse("/actions?" + query, kOptions);
  auto& ctx = *s_ctx;

  // Action.findPage(restrictAdminApiQueryOptions(...)) over the actions
  // resource. Store returns raw rows; core::query owns NQL / paging and
  // blog::mapping owns the Ghost action wire shape (mappers/actions ->
  // clean.action, including embedded actor/resource field picking).
  const auto rows = ctx.store().json_for("actions", {});
  const auto cleaned = local_mapping::clean(rows, "action");

  std::string ran;
  try {
   // guard() rejects password|token|gift_link_token|secret in filter/order
   // before filtering (restrictAdminApiQueryOptions, api-filter-utils.ts:275).
   ran = core::query::run(cleaned, q, 15);
  } catch (const std::exception&) {
   return bad_request();
  }

  const auto parts = blog::api::JsonBody{}.parse_raw(ran);
  const auto meta = blog::api::JsonBody{}.parse_raw(
   parts.contains("meta") ? parts.at("meta") : std::string{"{}"});
  const auto page_meta = blog::api::JsonBody{}.parse_raw(
   meta.contains("pagination") ? meta.at("pagination") : std::string{"{}"});
  const auto number = [&page_meta](const char* key, std::size_t fallback) {
   const auto it = page_meta.find(key);
   if (it == page_meta.end()) return fallback;
   try { return static_cast<std::size_t>(std::stoull(it->second)); }
   catch (...) { return fallback; }
  };
  const auto total = number("total", 0);
  const auto limit = number("limit", 15);
  const auto page = number("page", 1);
  // Bookshelf pages == 0 for an empty result set; core::query::run reports 1.
  const auto pages = limit ? (total + limit - 1) / limit : std::size_t{0};
  const std::string prev = page > 1 ? std::to_string(page - 1) : "null";
  const std::string next = page < pages ? std::to_string(page + 1) : "null";

  const auto pagination = local_mapping::pagination(total, limit, page, pages, prev, next);
  const auto data = parts.contains("data") ? parts.at("data") : std::string{"[]"};
  const auto body = local_mapping::wrap(kDocName, data, "{\"pagination\":" + pagination + "}");
  return core::workflow::StepResponse<Output>{{{"status","200"},{"body",body}}};
 }
 void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}
 static blog::BlogContext* s_ctx;
private:
 /** @brief Keep only exact, allowlisted query keys (drops filter[x], offset, order, ...). */
 [[nodiscard]] static std::string allowlisted_query(std::string_view path)
 {
  const auto qpos = path.find('?');
  if (qpos == std::string_view::npos) return {};
  auto query = path.substr(qpos + 1);
  if (const auto hpos = query.find('#'); hpos != std::string_view::npos)
   query = query.substr(0, hpos);

  std::string out;
  std::size_t start = 0;
  while (start <= query.size()) {
   const auto amp = query.find('&', start);
   const auto pair = query.substr(start, amp == std::string_view::npos
                                          ? std::string_view::npos : amp - start);
   const auto eq = pair.find('=');
   const auto key = eq == std::string_view::npos ? pair : pair.substr(0, eq);
   if (std::ranges::any_of(kOptions, [key](std::string_view allowed) { return key == allowed; })) {
    if (!out.empty()) out += '&';
    out += pair;
   }
   if (amp == std::string_view::npos) break;
   start = amp + 1;
  }
  return out;
 }

 [[nodiscard]] static std::optional<std::string> raw_option(std::string_view query, std::string_view key)
 {
  std::size_t start = 0;
  while (start <= query.size()) {
   const auto amp = query.find('&', start);
   const auto pair = query.substr(start, amp == std::string_view::npos
                                          ? std::string_view::npos : amp - start);
   const auto eq = pair.find('=');
   if ((eq == std::string_view::npos ? pair : pair.substr(0, eq)) == key)
    return std::string{eq == std::string_view::npos ? std::string_view{} : pair.substr(eq + 1)};
   if (amp == std::string_view::npos) break;
   start = amp + 1;
  }
  return std::nullopt;
 }

 [[nodiscard]] static std::string invalid_option(std::string_view query, std::string_view key)
 {
  const auto value = raw_option(query, key);
  if (value && !core::query::valid_option(key, *value))
   return "Validation (matches) failed for " + std::string{key};
  return {};
 }

 [[nodiscard]] static core::workflow::StepResponse<Output> invalid(std::string_view property, std::string_view context)
 {
  return core::workflow::StepResponse<Output>{{{"status","422"},
   {"body",blog::api::ApiError{}.validation("Validation error",context,property)}}};
 }

 [[nodiscard]] static core::workflow::StepResponse<Output> bad_request()
 {
  return core::workflow::StepResponse<Output>{{{"status","400"},
   {"body",blog::api::ApiError{}.json("Restricted fields cannot be used in order.","Bad Request")}}};
 }
};
blog::BlogContext* ExecuteStep::s_ctx{nullptr};
}
