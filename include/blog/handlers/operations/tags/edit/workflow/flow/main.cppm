export module blog_operations:tags_edit_flow;

import std;
import core_workflow;
import :tags_edit_step;

export namespace blog::handlers::operations::tags::edit::workflow {

class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}.name("tags_edit").step<ExecuteStep>().build();
    }
};

} // namespace
