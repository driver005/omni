export module blog_operations:users_browse_flow;

import std;
import core_workflow;
import :users_browse_step;

export namespace blog::handlers::operations::users::browse::workflow {

class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}.name("users_browse").step<ExecuteStep>().build();
    }
};

} // namespace
