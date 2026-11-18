export module blog_operations:users_destroy_flow;

import std;
import core_workflow;
import :users_destroy_step;

export namespace blog::handlers::operations::users::destroy::workflow {

class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}.name("users_destroy").step<ExecuteStep>().build();
    }
};

} // namespace
