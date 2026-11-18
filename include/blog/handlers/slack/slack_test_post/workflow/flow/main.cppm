export module blog_handlers:slack_slack_test_post_flow;
export import core_workflow;
import std;
import core_workflow;
import :slack_slack_test_post_step;
export namespace blog::handlers::slack::slack_test_post::workflow {
class MainFlow { public:
 [[nodiscard]] core::workflow::WorkflowDefinition build() const {
  return core::workflow::WorkflowBuilder{}.name("slack_slack_test_post").step<ExecuteStep>().build();
 }};
}
