export module blog_handlers:upgrades_create_flow;
export import core_workflow;
import std;
import core_workflow;
import :upgrades_create_step;

export namespace blog::handlers::upgrades::create::workflow {
class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}
            .name("upgrades_create")
            .step<ExecuteStep>()
            .build();
    }
};
} // namespace blog::handlers::upgrades::create::workflow
