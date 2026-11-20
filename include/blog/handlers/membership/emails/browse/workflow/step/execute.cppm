export module blog_handlers:emails_browse_step;
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

export namespace blog::handlers::emails::browse::workflow {
class ExecuteStep {
public:
 using Input=interfaces::io::IRequest&; using Output=std::string;
 static constexpr std::string_view name{"emails_browse_execute"};
 [[nodiscard]] core::workflow::StepResponse<Output> invoke(const Input& in, core::workflow::WorkflowContext&) const {
  auto& ctx=*s_ctx;

  // Ghost options: limit, fields, filter, order, page (emails.js:24).
  // core::query::parse normalizes the aliases and valid_option allowlist;
  // parse_query additionally applies restrictAdminApiQueryOptions' restricted
  // field guard (password|token|gift_link_token|secret).
  static constexpr std::string_view kOptions[]{"limit","fields","filter","order","page"};
  std::string error;
  const auto q=blog::api::emails::parse_query(in.get_path(), kOptions, error);
  if(!q) return core::workflow::StepResponse<Output>{error};

  try {
   // mappers/emails.js: %%{}%% replacements, post removal, stripEmailAccounting.
   const auto rows=blog::api::emails::serialize_emails(ctx.store().json_for("emails", {}));
   const auto ran=core::query::run(rows, *q, 15);
   return core::workflow::StepResponse<Output>{blog::api::emails::collection("emails", ran)};
  } catch(const std::exception&) {
   return core::workflow::StepResponse<Output>{
    blog::api::emails::bad_request("Restricted fields cannot be used in order.")};
  }
 }
 void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}
 static blog::BlogContext* s_ctx;
};
blog::BlogContext* ExecuteStep::s_ctx{nullptr};
}
