export module blog_operations:webhooks_destroy_flow;

import std;
import core_workflow;
import :webhooks_destroy_step;

export namespace blog::handlers::operations::webhooks::destroy::workflow {

class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}.name("webhooks_destroy").step<ExecuteStep>().build();
    }
};

} // namespace
