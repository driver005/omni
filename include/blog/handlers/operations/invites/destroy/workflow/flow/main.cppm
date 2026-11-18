export module blog_operations:invites_destroy_flow;

import std;
import core_workflow;
import :invites_destroy_step;

export namespace blog::handlers::operations::invites::destroy::workflow {

class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}.name("invites_destroy").step<ExecuteStep>().build();
    }
};

} // namespace
