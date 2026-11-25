export module blog_handlers:themes_themes_install_post_flow;
export import core_workflow;
import std;
import core_workflow;
import :themes_themes_install_post_step;

export namespace blog::handlers::themes::themes_install_post::workflow {
class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}
            .name("themes_themes_install_post")
            .step<ExecuteStep>()
            .build();
    }
};
} // namespace blog::handlers::themes::themes_install_post::workflow
