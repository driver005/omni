export module blog_operations:integrations_create_flow;

import std;
import core_workflow;
import :integrations_create_step;

export namespace blog::handlers::operations::integrations::create::workflow {

class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}.name("integrations_create").step<ExecuteStep>().build();
    }
};

} // namespace
