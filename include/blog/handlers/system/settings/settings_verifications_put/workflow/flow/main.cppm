export module blog_handlers:settings_settings_verifications_put_flow;
export import core_workflow;
import std;
import core_workflow;
import :settings_settings_verifications_put_step;

export namespace blog::handlers::settings::settings_verifications_put::workflow {
class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}
            .name("settings_settings_verifications_put")
            .step<ExecuteStep>()
            .build();
    }
};
} // namespace blog::handlers::settings::settings_verifications_put::workflow
