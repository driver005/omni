export module blog_handlers:emails_analytics_get_flow;
export import core_workflow;
import std;
import core_workflow;
import :emails_analytics_get_step;

export namespace blog::handlers::emails::analytics_get::workflow {
class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}
            .name("emails_analytics_get")
            .step<ExecuteStep>()
            .build();
    }
};
} // namespace blog::handlers::emails::analytics_get::workflow
