export module blog_handlers:invites_destroy_flow;
export import core_workflow;
import std;
import core_workflow;
import :invites_destroy_step;

export namespace blog::handlers::invites::destroy::workflow {
class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}
            .name("invites_destroy")
            .step<ExecuteStep>()
            .build();
    }
};
} // namespace blog::handlers::invites::destroy::workflow
