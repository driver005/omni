export module blog_operations:members_metafields_read_flow;

import std;
import core_workflow;
import :members_metafields_read_step;

export namespace blog::handlers::operations::members::metafields_read::workflow {

class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}.name("members_metafields_read").step<ExecuteStep>().build();
    }
};

} // namespace
