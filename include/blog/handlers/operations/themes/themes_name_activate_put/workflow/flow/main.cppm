export module blog_operations:themes_themes_name_activate_put_flow;

import std;
import core_workflow;
import :themes_themes_name_activate_put_step;

export namespace blog::handlers::operations::themes::themes_name_activate_put::workflow {

class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}.name("themes_themes_name_activate_put").step<ExecuteStep>().build();
    }
};

} // namespace
