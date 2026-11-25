export module blog_handlers:notifications_create_flow;
export import core_workflow;
import std;
import core_workflow;
import :notifications_create_step;

export namespace blog::handlers::notifications::create::workflow {
class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}
            .name("notifications_create")
            .step<ExecuteStep>()
            .build();
    }
};
} // namespace blog::handlers::notifications::create::workflow
