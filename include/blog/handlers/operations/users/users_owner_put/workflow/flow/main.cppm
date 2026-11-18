export module blog_operations:users_users_owner_put_flow;

import std;
import core_workflow;
import :users_users_owner_put_step;

export namespace blog::handlers::operations::users::users_owner_put::workflow {

class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}.name("users_users_owner_put").step<ExecuteStep>().build();
    }
};

} // namespace
