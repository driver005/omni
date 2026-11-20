export module blog_handlers:automations_automations_id_email_preview_post_step;
import :domain_all;
import :automations_dto_automation_email_preview;
import :automations_dto_automation_email_preview_input;
import :automations_dto_automation_email_preview_response;
import :automations_dto_automation_email_test_input;
import :automations_dto_links;
import :automations_dto_stats_empty_response;
import std;
import core_workflow;
import interfaces;
import :automations_operations;
export namespace blog::handlers::automations::automations_id_email_preview_post::workflow {
class ExecuteStep {
public:
  using Input=interfaces::io::IRequest&; using Output=std::map<std::string, std::string>;
  static constexpr std::string_view name{"automations_automations_id_email_preview_post_execute"};
  [[nodiscard]] core::workflow::StepResponse<Output> invoke(const Input& in, core::workflow::WorkflowContext&) const {
   const std::string id=blog::api::automations::automation_id(in.get_path());
   if (id.empty())
    return core::workflow::StepResponse<Output>{{{"status","400"},
      {"body",blog::api::automations::ghost_error("BadRequestError","id is required")}}};
   if (!blog::api::automations::automation_exists(*s_ctx, id))
    return core::workflow::StepResponse<Output>{{{"status","404"},
      {"body",blog::api::automations::ghost_error("NotFoundError","Automation not found.")}}};
   const auto r=s_ops->automation_email_preview(id, in.body_as_string());
   return core::workflow::StepResponse<Output>{{{"status",std::to_string(r.first)},{"body",r.second}}};
  }
  static blog::api::automations::AutomationsOperations* s_ops;
  void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}
  static blog::BlogContext* s_ctx;
};
blog::api::automations::AutomationsOperations* ExecuteStep::s_ops{nullptr};
blog::BlogContext* ExecuteStep::s_ctx{nullptr};
}
