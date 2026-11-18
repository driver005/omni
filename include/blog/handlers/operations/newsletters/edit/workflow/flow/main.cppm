export module blog_operations:newsletters_edit_flow;

import std;
import core_workflow;
import :newsletters_edit_step;

export namespace blog::handlers::operations::newsletters::edit::workflow {

class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}.name("newsletters_edit").step<ExecuteStep>().build();
    }
};

} // namespace
