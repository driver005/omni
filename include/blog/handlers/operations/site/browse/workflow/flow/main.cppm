export module blog_operations:site_browse_flow;

import std;
import core_workflow;
import :site_browse_step;

export namespace blog::handlers::operations::site::browse::workflow {

class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}.name("site_browse").step<ExecuteStep>().build();
    }
};

} // namespace
