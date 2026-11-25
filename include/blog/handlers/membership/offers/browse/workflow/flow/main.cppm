export module blog_handlers:offers_browse_flow;
export import core_workflow;
import std;
import core_workflow;
import :offers_browse_step;

export namespace blog::handlers::offers::browse::workflow {
class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}.name("offers_browse").step<ExecuteStep>().build();
    }
};
} // namespace blog::handlers::offers::browse::workflow
