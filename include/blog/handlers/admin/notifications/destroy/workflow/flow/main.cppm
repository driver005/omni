export module blog_handlers:notifications_destroy_flow;
export import core_workflow;
import std;
import core_workflow;
import :notifications_destroy_step;

export namespace blog::handlers::notifications::destroy::workflow {
class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}
            .name("notifications_destroy")
            .step<ExecuteStep>()
            .build();
    }
};
} // namespace blog::handlers::notifications::destroy::workflow
