export module blog_operations:comments_edit_flow;

import std;
import core_workflow;
import :comments_edit_step;

export namespace blog::handlers::operations::comments::edit::workflow {

class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}.name("comments_edit").step<ExecuteStep>().build();
    }
};

} // namespace
