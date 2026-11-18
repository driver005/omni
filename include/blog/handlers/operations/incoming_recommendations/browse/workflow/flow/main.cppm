export module blog_operations:incoming_recommendations_browse_flow;

import std;
import core_workflow;
import :incoming_recommendations_browse_step;

export namespace blog::handlers::operations::incoming_recommendations::browse::workflow {

class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}.name("incoming_recommendations_browse").step<ExecuteStep>().build();
    }
};

} // namespace
