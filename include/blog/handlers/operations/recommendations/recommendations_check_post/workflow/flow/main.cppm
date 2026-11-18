export module blog_operations:recommendations_recommendations_check_post_flow;

import std;
import core_workflow;
import :recommendations_recommendations_check_post_step;

export namespace blog::handlers::operations::recommendations::recommendations_check_post::workflow {

class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}.name("recommendations_recommendations_check_post").step<ExecuteStep>().build();
    }
};

} // namespace
