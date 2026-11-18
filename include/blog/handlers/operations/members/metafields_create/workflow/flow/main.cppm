export module blog_operations:members_metafields_create_flow;

import std;
import core_workflow;
import :members_metafields_create_step;

export namespace blog::handlers::operations::members::metafields_create::workflow {

class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}.name("members_metafields_create").step<ExecuteStep>().build();
    }
};

} // namespace
