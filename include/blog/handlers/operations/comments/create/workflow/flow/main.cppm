export module blog_operations:comments_create_flow;

import std;
import core_workflow;
import :comments_create_step;

export namespace blog::handlers::operations::comments::create::workflow {

class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}.name("comments_create").step<ExecuteStep>().build();
    }
};

} // namespace
