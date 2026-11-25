export module blog_handlers:invites_create_flow;
export import core_workflow;
import std;
import core_workflow;
import :invites_create_step;

export namespace blog::handlers::invites::create::workflow {
class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}.name("invites_create").step<ExecuteStep>().build();
    }
};
} // namespace blog::handlers::invites::create::workflow
