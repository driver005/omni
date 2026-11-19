export module blog_handlers:automations_automations_automation_id_actions_action_id_links_get_step;
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
import blog_handlers_middleware;

export namespace blog::handlers::automations::automations_automation_id_actions_action_id_links_get::workflow {
class ExecuteStep {
public:
 using Input=interfaces::io::IRequest&; using Output=std::string;
 static constexpr std::string_view name{"automations_automations_automation_id_actions_action_id_links_get_execute"};
 [[nodiscard]] core::workflow::StepResponse<Output> invoke(const Input& in, core::workflow::WorkflowContext&) const {
  auto& ctx=*s_ctx; auto& mutations = ctx.store();
    const auto r=[&]()->std::pair<int,std::string>{
        blog::dto::automations::AutomationLinksResponse response;
        response.links.push_back(blog::dto::automations::LinkRecord{});
        return {200, blog::handlers::middleware::encode(response)};
    }();
  if(r.first!=0) return core::workflow::StepResponse<Output>{r.second};
  const auto one=ctx.store().find_by_resource("automations", std::string{}.empty()?"1":std::string{});
  if(one) return core::workflow::StepResponse<Output>{"{\"automations\":[" + *one + "]}"};
  return core::workflow::StepResponse<Output>{R"({"automations":[]})"};
 }
 void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}
 static blog::BlogContext* s_ctx;
};
blog::BlogContext* ExecuteStep::s_ctx{nullptr};
}
