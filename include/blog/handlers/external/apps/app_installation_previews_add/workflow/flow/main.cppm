export module blog_handlers:apps_app_installation_previews_add_flow;
export import core_workflow;
import :domain_all;
import std;
import core_workflow;
import :apps_app_installation_previews_add_step;

export namespace blog::handlers::apps::app_installation_previews_add::workflow {
class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}
            .name("apps_app_installation_previews_add")
            .step<ExecuteStep>()
            .build();
    }
};
} // namespace blog::handlers::apps::app_installation_previews_add::workflow
