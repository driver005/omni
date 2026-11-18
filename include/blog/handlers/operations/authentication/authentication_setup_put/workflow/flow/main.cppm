export module blog_operations:authentication_authentication_setup_put_flow;

import std;
import core_workflow;
import :authentication_authentication_setup_put_step;

export namespace blog::handlers::operations::authentication::authentication_setup_put::workflow {

class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}.name("authentication_authentication_setup_put").step<ExecuteStep>().build();
    }
};

} // namespace
