export module blog_operations:posts_export_flow;

import std;
import core_workflow;
import :posts_export_step;

export namespace blog::handlers::operations::posts::export_op::workflow {

class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}.name("posts_export").step<ExecuteStep>().build();
    }
};

} // namespace
