export module blog_handlers:settings_browse_flow;
export import core_workflow;
import std;
import core_workflow;
import :settings_browse_step;

export namespace blog::handlers::settings::browse::workflow {
class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}
            .name("settings_browse")
            .step<ExecuteStep>()
            .build();
    }
};
} // namespace blog::handlers::settings::browse::workflow
