export module blog_operations:settings_settings_routes_yaml_get_flow;

import std;
import core_workflow;
import :settings_settings_routes_yaml_get_step;

export namespace blog::handlers::operations::settings::settings_routes_yaml_get::workflow {

class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}.name("settings_settings_routes_yaml_get").step<ExecuteStep>().build();
    }
};

} // namespace
