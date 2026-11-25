export module blog_handlers:members_subscriptions_post_flow;
export import core_workflow;
import std;
import core_workflow;
import :members_subscriptions_post_step;

export namespace blog::handlers::members::subscriptions_post::workflow {
class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}
            .name("members_subscriptions_post")
            .step<ExecuteStep>()
            .build();
    }
};
} // namespace blog::handlers::members::subscriptions_post::workflow
