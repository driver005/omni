export module blog_operations:emails_analytics_put_flow;

import std;
import core_workflow;
import :emails_analytics_put_step;

export namespace blog::handlers::operations::emails::analytics_put::workflow {

class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}.name("emails_analytics_put").step<ExecuteStep>().build();
    }
};

} // namespace
