export module blog_operations:tags_read_by_slug_flow;

import std;
import core_workflow;
import :tags_read_by_slug_step;

export namespace blog::handlers::operations::tags::read_by_slug::workflow {

class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}.name("tags_read_by_slug").step<ExecuteStep>().build();
    }
};

} // namespace
