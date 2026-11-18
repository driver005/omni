export module blog_operations:tags_create_flow;

import std;
import core_workflow;
import :tags_create_step;

export namespace blog::handlers::operations::tags::create::workflow {

class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}.name("tags_create").step<ExecuteStep>().build();
    }
};

} // namespace
