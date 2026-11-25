export module blog_handlers:custom_theme_settings_bulk_edit_flow;
export import core_workflow;
import std;
import core_workflow;
import :custom_theme_settings_bulk_edit_step;

export namespace blog::handlers::custom_theme_settings::bulk_edit::workflow {
class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}
            .name("custom_theme_settings_bulk_edit")
            .step<ExecuteStep>()
            .build();
    }
};
} // namespace blog::handlers::custom_theme_settings::bulk_edit::workflow
