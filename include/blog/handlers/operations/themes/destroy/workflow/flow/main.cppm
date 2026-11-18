export module blog_operations:themes_destroy_flow;

import std;
import core_workflow;
import :themes_destroy_step;

export namespace blog::handlers::operations::themes::destroy::workflow {

class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}.name("themes_destroy").step<ExecuteStep>().build();
    }
};

} // namespace
