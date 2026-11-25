export module blog_handlers:recommendations_edit_flow;
export import core_workflow;
import std;
import core_workflow;
import :recommendations_edit_step;

export namespace blog::handlers::recommendations::edit::workflow {
class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}
            .name("recommendations_edit")
            .step<ExecuteStep>()
            .build();
    }
};
} // namespace blog::handlers::recommendations::edit::workflow
