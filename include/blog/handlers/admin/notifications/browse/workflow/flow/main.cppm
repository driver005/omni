export module blog_handlers:notifications_browse_flow;
export import core_workflow;
import std;
import core_workflow;
import :notifications_browse_step;

export namespace blog::handlers::notifications::browse::workflow {
class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}
            .name("notifications_browse")
            .step<ExecuteStep>()
            .build();
    }
};
} // namespace blog::handlers::notifications::browse::workflow
