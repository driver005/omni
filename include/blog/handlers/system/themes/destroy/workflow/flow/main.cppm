export module blog_handlers:themes_destroy_flow;
export import core_workflow;
import std;
import core_workflow;
import :themes_destroy_step;

export namespace blog::handlers::themes::destroy::workflow {
class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}.name("themes_destroy").step<ExecuteStep>().build();
    }
};
} // namespace blog::handlers::themes::destroy::workflow
