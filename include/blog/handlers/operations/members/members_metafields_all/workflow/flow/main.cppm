export module blog_operations:members_members_metafields_all_flow;

import std;
import core_workflow;
import :members_members_metafields_all_step;

export namespace blog::handlers::operations::members::members_metafields_all::workflow {

class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}.name("members_members_metafields_all").step<ExecuteStep>().build();
    }
};

} // namespace
