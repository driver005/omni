export module blog_operations:newsletters_create_flow;

import std;
import core_workflow;
import :newsletters_create_step;

export namespace blog::handlers::operations::newsletters::create::workflow {

class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}.name("newsletters_create").step<ExecuteStep>().build();
    }
};

} // namespace
