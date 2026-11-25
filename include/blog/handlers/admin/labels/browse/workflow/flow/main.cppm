export module blog_handlers:labels_browse_flow;
export import core_workflow;
import std;
import core_workflow;
import :labels_browse_step;

export namespace blog::handlers::labels::browse::workflow {
class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}.name("labels_browse").step<ExecuteStep>().build();
    }
};
} // namespace blog::handlers::labels::browse::workflow
