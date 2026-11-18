export module blog_operations:themes_themes_active_get_flow;

import std;
import core_workflow;
import :themes_themes_active_get_step;

export namespace blog::handlers::operations::themes::themes_active_get::workflow {

class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}.name("themes_themes_active_get").step<ExecuteStep>().build();
    }
};

} // namespace
