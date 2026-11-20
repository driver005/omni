export module blog_handlers:members_members_events_get_step;
import :domain_all;
import std;
import core_workflow;
import core_query;
import interfaces;
import :members_operations;

// Strict-parity GET /members/events (activityFeed). Ghost options limit/filter
// with NQL (M:487), getEventTimeline default limit 10, meta page/next/prev:null
// (ER:77-188, OS:24,83-88). The `events` resource is the C++ stand-in for the
// ~20 event sources; events are sorted by created_at before slicing.
export namespace blog::handlers::members::members_events_get::workflow {
namespace support=blog::api::members::members_support;
class ExecuteStep {
public:
 using Input=interfaces::io::IRequest&;
 using Output=std::map<std::string,std::string>;
 static constexpr std::string_view name{"members_members_events_get_execute"};
 [[nodiscard]] core::workflow::StepResponse<Output> invoke(const Input& in, core::workflow::WorkflowContext&) const {
  auto& ctx=*s_ctx;
  static constexpr std::string_view kOptions[]{"limit","filter"};
  auto q=core::query::parse(in.get_path(), kOptions);
  if (core::query::has_restricted_field(q.raw_filter))
   return core::workflow::StepResponse<Output>{{{"status","400"},
     {"body",support::bad_request("Restricted fields cannot be used in a filter.")}}};
  if (!q.options.contains("limit")) q.limit=10; // Ghost getEventTimeline default
  q.order="created_at desc";
  std::string available=ctx.store().json_for("events", {});
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
  const auto meta=std::format(
    R"({{"pagination":{{"page":null,"limit":{},"pages":{},"total":{},"next":null,"prev":null}}}})",
    q.limit, total==0?1:(total+q.limit-1)/q.limit, total);
  const auto body=std::format(R"({{"events":{},"meta":{}}})", rows.empty()?"[]":rows, meta);
  return core::workflow::StepResponse<Output>{{{"status","200"},{"body",body}}};
 }
 void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}
 static blog::BlogContext* s_ctx;
};
blog::BlogContext* ExecuteStep::s_ctx{nullptr};
}
