export module blog_handlers:labels_create_flow;
export import core_workflow;
import std;
import core_workflow;
import :labels_create_step;

export namespace blog::handlers::labels::create::workflow {
class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}.name("labels_create").step<ExecuteStep>().build();
    }
};
} // namespace blog::handlers::labels::create::workflow
