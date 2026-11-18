export module blog_operations:newsletters_read_by_id_flow;

import std;
import core_workflow;
import :newsletters_read_by_id_step;

export namespace blog::handlers::operations::newsletters::read_by_id::workflow {

class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}.name("newsletters_read_by_id").step<ExecuteStep>().build();
    }
};

} // namespace
