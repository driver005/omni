export module blog_handlers:members_browse_step;
import :domain_all;
import std;
import core_workflow;
import core_query;
import interfaces;
import :members_operations;

// Strict-parity GET /members (browse). Ghost: options limit,fields,filter,order,
// debug,page,search,include (M:62); include allowlist (M:49-52,64-70);
// restrictAdminApiQueryOptions rejects password/token/gift_link_token/secret
// (AF:218-220,247-267); limit>100/'all' capped to 100 (BS:792-826);
// autoOrder 'created_at DESC, id DESC' (IS:60-74); X-Cache-Invalidate:false
// (M:59-61) => api-framework emits no header.
export namespace blog::handlers::members::browse::workflow {
class ExecuteStep {
public:
 using Input=interfaces::io::IRequest&;
 using Output=std::map<std::string,std::string>;
 static constexpr std::string_view name{"members_browse_execute"};
 [[nodiscard]] core::workflow::StepResponse<Output> invoke(const Input& in, core::workflow::WorkflowContext&) const {
  namespace support=blog::api::members::members_support;
  auto& ctx=*s_ctx;
  static constexpr std::string_view kOptions[]{"limit","fields","filter","order","debug","page","search","include"};
  auto q=core::query::parse(in.get_path(), kOptions);

  if (!support::valid_include(q))
   return core::workflow::StepResponse<Output>{{{"status","422"},
     {"body",support::invalid(support::kNoIncludeMessage, support::include_value(q), "include")}}};
  if (core::query::has_restricted_field(q.raw_filter) ||
      core::query::has_restricted_field(q.order))
   return core::workflow::StepResponse<Output>{{{"status","400"},
     {"body",support::bad_request("Restricted fields cannot be used in a filter.")}}};
  if (q.order.empty()) q.order="created_at desc,id desc";
  if (q.limit>100) q.limit=100;

  const auto available=ctx.store().json_for("members", {});
  auto count_q=q;
  count_q.limit=std::numeric_limits<std::size_t>::max();
  count_q.page=1; count_q.offset=0; count_q.fields.clear();
  std::string rows, filtered;
  try {
   filtered=core::query::apply(available, count_q);
   rows=core::query::apply(available, q);
  } catch (...) {
   return core::workflow::StepResponse<Output>{{{"status","400"},
     {"body",support::bad_request("The NQL filter you passed was invalid.")}}};
  }
  const auto total=support::rows_of(filtered).size();
  const auto body=support::wrap("members", rows, support::pagination(total, q));
  return core::workflow::StepResponse<Output>{{{"status","200"},{"body",body}}};
 }
 void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}
 static blog::BlogContext* s_ctx;
};
blog::BlogContext* ExecuteStep::s_ctx{nullptr};
}
