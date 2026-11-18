export module blog_operations:settings_browse_flow;

import std;
import core_workflow;
import :settings_browse_step;

export namespace blog::handlers::operations::settings::browse::workflow {

class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}.name("settings_browse").step<ExecuteStep>().build();
    }
};

} // namespace
