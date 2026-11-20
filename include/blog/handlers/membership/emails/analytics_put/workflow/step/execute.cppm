export module blog_handlers:emails_analytics_put_step;
import :domain_all;
import :emails_dto_analytics_job;
import :emails_dto_analytics_status;
import :emails_dto_analytics_status_input;
import :emails_dto_scheduled_analytics_job;
import :emails_model_member;
import :emails_model_wire_fixtures;
import :emails_operations;
import std;
import core_query;
import core_serde;
import core_workflow;
import interfaces;
export namespace blog::handlers::emails::analytics_put::workflow {
class ExecuteStep {
public:
 using Input=interfaces::io::IRequest&; using Output=std::map<std::string,std::string>;
 static constexpr std::string_view name{"emails_analytics_put_execute"};
 [[nodiscard]] core::workflow::StepResponse<Output> invoke(const Input& in, core::workflow::WorkflowContext&) const {
  auto& ctx=*s_ctx;
  const std::string path{in.get_path()};
  const std::string id=blog::api::emails::id_from_path(path);

  // Ghost options:['begin','end'] — dates are parsed with new Date(...) and the
  // controller declares no explicit validation (no 422 begin>end branch).
  static constexpr std::string_view kOptions[]{"begin","end"};
  const auto q=core::query::parse(path, kOptions);
  std::string begin;
  std::string end;
  if(const auto it=q.options.find("begin"); it!=q.options.end()) begin=it->second;
  if(const auto it=q.options.find("end"); it!=q.options.end()) end=it->second;

  // Email.findOne(data, remainingOptions) supplies created_at when begin is absent.
  std::string created_at;
  const auto found=blog::api::emails::find_email(ctx.store(), id);
  if(!found.empty()) {
   const auto objects=blog::api::emails::decode_objects("["+found+"]");
   if(!objects.empty()) created_at=blog::api::emails::field_string(&objects.front(), "created_at");
  }

  // scheduleAnalytics persists job metadata (cache + outbox stand-in) and
  // returns void; EmailsOperations emits the Ghost-shaped error on a running job.
  const auto r=s_ops->emails_analytics_put(id, begin, end, created_at);
  return core::workflow::StepResponse<Output>{{{"status",std::to_string(r.first)},{"body",r.second}}};
 }
 void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}
 static blog::BlogContext* s_ctx; static blog::api::emails::EmailsOperations* s_ops;
};
blog::api::emails::EmailsOperations* ExecuteStep::s_ops{nullptr};
blog::BlogContext* ExecuteStep::s_ctx{nullptr};
}
