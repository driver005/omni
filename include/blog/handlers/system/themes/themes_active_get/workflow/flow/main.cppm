export module blog_handlers:themes_themes_active_get_flow;
export import core_workflow;
import std;
import core_workflow;
import :themes_themes_active_get_step;

export namespace blog::handlers::themes::themes_active_get::workflow {
class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}
            .name("themes_themes_active_get")
            .step<ExecuteStep>()
            .build();
    }
};
} // namespace blog::handlers::themes::themes_active_get::workflow
