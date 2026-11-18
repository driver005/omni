export module blog_operations:emails_recipient_failures_get_flow;

import std;
import core_workflow;
import :emails_recipient_failures_get_step;

export namespace blog::handlers::operations::emails::recipient_failures_get::workflow {

class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}.name("emails_recipient_failures_get").step<ExecuteStep>().build();
    }
};

} // namespace
