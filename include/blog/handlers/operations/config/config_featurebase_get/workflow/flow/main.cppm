export module blog_operations:config_config_featurebase_get_flow;

import std;
import core_workflow;
import :config_config_featurebase_get_step;

export namespace blog::handlers::operations::config::config_featurebase_get::workflow {

class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}.name("config_config_featurebase_get").step<ExecuteStep>().build();
    }
};

} // namespace
