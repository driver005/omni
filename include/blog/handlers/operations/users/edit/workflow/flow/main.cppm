export module blog_operations:users_edit_flow;

import std;
import core_workflow;
import :users_edit_step;

export namespace blog::handlers::operations::users::edit::workflow {

class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}.name("users_edit").step<ExecuteStep>().build();
    }
};

} // namespace
