export module blog_operations:offers_create_flow;

import std;
import core_workflow;
import :offers_create_step;

export namespace blog::handlers::operations::offers::create::workflow {

class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}.name("offers_create").step<ExecuteStep>().build();
    }
};

} // namespace
