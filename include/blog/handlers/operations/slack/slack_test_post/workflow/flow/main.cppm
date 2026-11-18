export module blog_operations:slack_slack_test_post_flow;

import std;
import core_workflow;
import :slack_slack_test_post_step;

export namespace blog::handlers::operations::slack::slack_test_post::workflow {

class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}.name("slack_slack_test_post").step<ExecuteStep>().build();
    }
};

} // namespace
