export module blog_handlers:integrations_browse_step;
import :domain_all;
import :integrations_dto_more_resources;
import std;
import core_workflow;
import core_query;
import interfaces;

export namespace blog::handlers::integrations::browse::workflow {
class ExecuteStep {
public:
 using Input=interfaces::io::IRequest&; using Output=std::map<std::string,std::string>;
 static constexpr std::string_view name{"integrations_browse_execute"};
 [[nodiscard]] core::workflow::StepResponse<Output> invoke(const Input& in, core::workflow::WorkflowContext&) const {
  namespace support = blog::handlers::integrations::support;
  auto& ctx = *s_ctx;

  // Ghost options:['include','limit'] with include values {api_keys,webhooks}
  // (integrations.js:18-25). The input serializer always injects the default
  // type filter (input/integrations.js:4-10).
  static constexpr std::string_view kAllowed[]{
      "include", "limit", "page", "filter", "fields", "order"};
  auto q = core::query::parse(in.get_path(), kAllowed);

  const std::string include = join(q.include);
  if (!support::valid_include(include)) {
   return core::workflow::StepResponse<Output>{{{"status", "422"},
    {"body", blog::api::ApiError{}.validation("Validation error", "Invalid include value", "include")}}};
  }

  const std::string caller_filter = q.raw_filter;
  q.raw_filter = caller_filter.empty()
      ? std::string{"type:[custom,builtin,core]"}
      : std::format("({})+type:[custom,builtin,core]", caller_filter);

  // findPage loads the default api_keys relation for every integration row.
  std::vector<std::string> rows;
  for (const auto& record : support::array(ctx.store().resources().list("integrations"))) {
   rows.push_back(support::embed_relations(ctx.store(), record, include));
  }

  std::string data;
  std::string meta;
  try {
   const auto ran = support::object(core::query::run(support::make_array(rows), q, 15));
   const auto data_it = ran.find("data");
   const auto meta_it = ran.find("meta");
   data = support::prefix_admin_secrets(data_it == ran.end() ? std::string{"[]"} : data_it->second);
   meta = meta_it == ran.end() ? std::string{"{}"} : meta_it->second;
  } catch (const std::exception&) {
   // mongoTransformer / restricted order -> BadRequestError (api-filter-utils.ts).
   return core::workflow::StepResponse<Output>{{{"status", "400"},
    {"body", blog::api::ApiError{}.json("Restricted fields cannot be used in order.", "Bad Request")}}};
  }

  return core::workflow::StepResponse<Output>{{{"status", "200"},
   {"body", support::wrap("integrations", data, meta)}}};
 }
 void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}
 static blog::BlogContext* s_ctx;
private:
 template<class R>
 static std::string join(const R& values) {
  std::string out;
  for (const auto& value : values) out += (out.empty() ? "" : ",") + std::string{value};
  return out;
 }
};
blog::BlogContext* ExecuteStep::s_ctx{nullptr};
}
