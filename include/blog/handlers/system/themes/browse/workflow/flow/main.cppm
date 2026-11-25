export module blog_handlers:themes_browse_flow;
export import core_workflow;
import std;
import core_workflow;
import :themes_browse_step;

export namespace blog::handlers::themes::browse::workflow {
class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}.name("themes_browse").step<ExecuteStep>().build();
    }
};
} // namespace blog::handlers::themes::browse::workflow
