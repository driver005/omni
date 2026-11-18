export module blog_operations:invites_create_flow;

import std;
import core_workflow;
import :invites_create_step;

export namespace blog::handlers::operations::invites::create::workflow {

class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}.name("invites_create").step<ExecuteStep>().build();
    }
};

} // namespace
