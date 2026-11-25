export module blog_handlers:site_browse_flow;
export import core_workflow;
import std;
import core_workflow;
import :site_browse_step;

export namespace blog::handlers::site::browse::workflow {
class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}.name("site_browse").step<ExecuteStep>().build();
    }
};
} // namespace blog::handlers::site::browse::workflow
