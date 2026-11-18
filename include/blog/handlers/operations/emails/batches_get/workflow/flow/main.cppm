export module blog_operations:emails_batches_get_flow;

import std;
import core_workflow;
import :emails_batches_get_step;

export namespace blog::handlers::operations::emails::batches_get::workflow {

class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}.name("emails_batches_get").step<ExecuteStep>().build();
    }
};

} // namespace
