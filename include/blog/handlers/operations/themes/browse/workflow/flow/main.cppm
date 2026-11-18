export module blog_operations:themes_browse_flow;

import std;
import core_workflow;
import :themes_browse_step;

export namespace blog::handlers::operations::themes::browse::workflow {

class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}.name("themes_browse").step<ExecuteStep>().build();
    }
};

} // namespace
