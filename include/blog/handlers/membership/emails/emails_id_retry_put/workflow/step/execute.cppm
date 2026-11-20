export module blog_handlers:emails_emails_id_retry_put_step;
import :domain_all;
import :emails_dto_analytics_job;
import :emails_dto_analytics_status;
import :emails_dto_analytics_status_input;
import :emails_dto_scheduled_analytics_job;
import :emails_model_member;
import :emails_model_wire_fixtures;
import :emails_operations;
import :outbox_shim;
import std;
import core_serde;
import core_workflow;
import interfaces;

export namespace blog::handlers::emails::emails_id_retry_put::workflow {
class ExecuteStep {
public:
 using Input=interfaces::io::IRequest&; using Output=std::map<std::string,std::string>;
 static constexpr std::string_view name{"emails_emails_id_retry_put_execute"};
 [[nodiscard]] core::workflow::StepResponse<Output> invoke(const Input& in, core::workflow::WorkflowContext&) const {
  auto& ctx=*s_ctx;
  const std::string id=blog::api::emails::id_from_path(in.get_path());

  // Ghost retryFailedEmail -> Email.findOne(require:false) -> NotFoundError.
  const auto found=blog::api::emails::find_email(ctx.store(), id);
  if(found.empty()) return core::workflow::StepResponse<Output>{{{"status","404"},
   {"body",blog::api::emails::not_found("Email not found.")}}};

  const auto objects=blog::api::emails::decode_objects("["+found+"]");
  if(objects.empty()) return core::workflow::StepResponse<Output>{{{"status","404"},
   {"body",blog::api::emails::not_found("Email not found.")}}};

  // checkCanRetryEmail + updateStatusLock(failed -> pending): only a failed
  // email may be claimed. Ghost reports IncorrectUsageError otherwise.
  auto claimed=objects.front();
  const std::string status=blog::api::emails::field_string(&claimed, "status");
  if(status!="failed") return core::workflow::StepResponse<Output>{{{"status","400"},
   {"body",blog::api::emails::incorrect_usage("Only failed emails can be retried")}}};

  claimed["status"]=serde::Value{std::string{"pending"}};
  const auto stored=blog::api::emails::encode_object(claimed);
  const auto result=ctx.store().edit("emails", id, stored);
  if(!result.ok()) return core::workflow::StepResponse<Output>{{{"status","400"},
   {"body",blog::api::emails::incorrect_usage("Only failed emails can be retried")}}};

  // Ghost records the email.edited domain event (and schedules the retry job).
  blog::handlers::outbox::global_outbox().emit(blog::handlers::outbox::Event{
   .type="email.edited",
   .payload=std::format(R"({{"id":{},"status":"pending"}})", blog::api::emails::json_quote(id)),
   .aggregate_id=id
  });

  // Ghost returns the claimed email under its resource key only.
  const std::string body=blog::api::emails::envelope("emails", "["+stored+"]");
  return core::workflow::StepResponse<Output>{{{"status",std::to_string(result.status())},{"body",body}}};
 }
 void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}
 static blog::BlogContext* s_ctx;
};
blog::BlogContext* ExecuteStep::s_ctx{nullptr};
}
