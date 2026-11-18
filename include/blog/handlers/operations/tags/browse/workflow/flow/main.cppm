export module blog_operations:tags_browse_flow;

import std;
import core_workflow;
import :tags_browse_step;

export namespace blog::handlers::operations::tags::browse::workflow {

class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}.name("tags_browse").step<ExecuteStep>().build();
    }
};

} // namespace
