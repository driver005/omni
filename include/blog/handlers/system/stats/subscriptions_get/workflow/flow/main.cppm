export module blog_handlers:stats_subscriptions_get_flow;
export import core_workflow;
import std;
import core_workflow;
import :stats_subscriptions_get_step;

export namespace blog::handlers::stats::subscriptions_get::workflow {
class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}
            .name("subscriptions_get")
            .step<ExecuteStep>()
            .build();
    }
};
} // namespace blog::handlers::stats::subscriptions_get::workflow
