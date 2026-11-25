export module blog_handlers:newsletters_edit_flow;
export import core_workflow;
import std;
import core_workflow;
import :newsletters_edit_step;

export namespace blog::handlers::newsletters::edit::workflow {
class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}
            .name("newsletters_edit")
            .step<ExecuteStep>()
            .build();
    }
};
} // namespace blog::handlers::newsletters::edit::workflow
