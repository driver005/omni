export module blog_handlers:members_destroy_flow;
export import core_workflow;
import std;
import core_workflow;
import :members_destroy_step;

export namespace blog::handlers::members::destroy::workflow {
class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}
            .name("members_destroy")
            .step<ExecuteStep>()
            .build();
    }
};
} // namespace blog::handlers::members::destroy::workflow
