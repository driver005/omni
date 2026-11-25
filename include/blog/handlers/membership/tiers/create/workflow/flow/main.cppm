export module blog_handlers:tiers_create_flow;
export import core_workflow;
import std;
import core_workflow;
import :tiers_create_step;

export namespace blog::handlers::tiers::create::workflow {
class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}.name("tiers_create").step<ExecuteStep>().build();
    }
};
} // namespace blog::handlers::tiers::create::workflow
