export module blog_handlers:upgrades_read_by_id_flow;
export import core_workflow;
import std;
import core_workflow;
import :upgrades_read_by_id_step;

export namespace blog::handlers::upgrades::read_by_id::workflow {
class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}
            .name("upgrades_read_by_id")
            .step<ExecuteStep>()
            .build();
    }
};
} // namespace blog::handlers::upgrades::read_by_id::workflow
