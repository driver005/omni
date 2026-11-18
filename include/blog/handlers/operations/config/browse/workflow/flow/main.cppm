export module blog_operations:config_browse_flow;

import std;
import core_workflow;
import :config_browse_step;

export namespace blog::handlers::operations::config::browse::workflow {

class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}.name("config_browse").step<ExecuteStep>().build();
    }
};

} // namespace
