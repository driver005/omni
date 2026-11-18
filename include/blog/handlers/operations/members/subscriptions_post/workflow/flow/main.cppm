export module blog_operations:members_subscriptions_post_flow;

import std;
import core_workflow;
import :members_subscriptions_post_step;

export namespace blog::handlers::operations::members::subscriptions_post::workflow {

class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}.name("members_subscriptions_post").step<ExecuteStep>().build();
    }
};

} // namespace
