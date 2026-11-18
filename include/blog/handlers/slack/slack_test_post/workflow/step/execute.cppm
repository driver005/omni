export module blog_handlers:slack_slack_test_post_step;
import std;
import core_workflow;
import blog_context;
import :mutation;
import :special;
import blog_dto;
import :slack_slack_test_post_model;
import :slack_slack_test_post_dto;
export namespace blog::handlers::slack::slack_test_post::workflow {
class ExecuteStep {
public:
 using Input=model::Request; using Output=std::string;
 static constexpr std::string_view name{"slack_slack_test_post_execute"};
 [[nodiscard]] core::workflow::StepResponse<Output> invoke(const Input& in, core::workflow::WorkflowContext&) const {
  auto& ctx=*s_ctx; blog::api::MutationStore mutations{ctx.store()};
  auto& special=*s_special;
  
  return core::workflow::StepResponse<Output>{mutations.create("slack", in.body.empty()?"{}":in.body).body()};
 }
 void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}
 static blog::BlogContext* s_ctx; static blog::api::SpecialEndpoints* s_special;
};
blog::BlogContext* ExecuteStep::s_ctx{nullptr};
blog::api::SpecialEndpoints* ExecuteStep::s_special{nullptr};
}