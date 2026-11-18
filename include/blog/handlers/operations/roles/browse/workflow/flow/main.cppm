export module blog_operations:roles_browse_flow;

import std;
import core_workflow;
import :roles_browse_step;

export namespace blog::handlers::operations::roles::browse::workflow {

class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}.name("roles_browse").step<ExecuteStep>().build();
    }
};

} // namespace
