export module blog_operations:exports_create_flow;

import std;
import core_workflow;
import :exports_create_step;

export namespace blog::handlers::operations::exports::create::workflow {

class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}.name("exports_create").step<ExecuteStep>().build();
    }
};

} // namespace
