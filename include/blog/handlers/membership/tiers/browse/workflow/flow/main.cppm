export module blog_handlers:tiers_browse_flow;
export import core_workflow;
import std;
import core_workflow;
import :tiers_browse_step;

export namespace blog::handlers::tiers::browse::workflow {
class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}.name("tiers_browse").step<ExecuteStep>().build();
    }
};
} // namespace blog::handlers::tiers::browse::workflow
