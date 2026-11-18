export module blog_operations:emails_analytics_delete_flow;

import std;
import core_workflow;
import :emails_analytics_delete_step;

export namespace blog::handlers::operations::emails::analytics_delete::workflow {

class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}.name("emails_analytics_delete").step<ExecuteStep>().build();
    }
};

} // namespace
