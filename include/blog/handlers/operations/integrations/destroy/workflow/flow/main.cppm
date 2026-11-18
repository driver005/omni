export module blog_operations:integrations_destroy_flow;

import std;
import core_workflow;
import :integrations_destroy_step;

export namespace blog::handlers::operations::integrations::destroy::workflow {

class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}.name("integrations_destroy").step<ExecuteStep>().build();
    }
};

} // namespace
