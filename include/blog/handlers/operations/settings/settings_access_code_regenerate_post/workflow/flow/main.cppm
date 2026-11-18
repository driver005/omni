export module blog_operations:settings_settings_access_code_regenerate_post_flow;

import std;
import core_workflow;
import :settings_settings_access_code_regenerate_post_step;

export namespace blog::handlers::operations::settings::settings_access_code_regenerate_post::workflow {

class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}.name("settings_settings_access_code_regenerate_post").step<ExecuteStep>().build();
    }
};

} // namespace
