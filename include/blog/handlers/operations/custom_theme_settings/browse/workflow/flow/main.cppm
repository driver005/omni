export module blog_operations:custom_theme_settings_browse_flow;

import std;
import core_workflow;
import :custom_theme_settings_browse_step;

export namespace blog::handlers::operations::custom_theme_settings::browse::workflow {

class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}.name("custom_theme_settings_browse").step<ExecuteStep>().build();
    }
};

} // namespace
