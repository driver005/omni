export module blog_handlers:authors_browse_flow;
export import core_workflow;
import std;
import core_workflow;
import :authors_browse_step;

export namespace blog::handlers::authors::browse::workflow {
class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}.name("authors_browse").step<ExecuteStep>().build();
    }
};
} // namespace blog::handlers::authors::browse::workflow
