export module blog_operations:webhooks_create_flow;

import std;
import core_workflow;
import :webhooks_create_step;

export namespace blog::handlers::operations::webhooks::create::workflow {

class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}.name("webhooks_create").step<ExecuteStep>().build();
    }
};

} // namespace
