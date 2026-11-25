export module blog_handlers:integrations_read_by_id_flow;
export import core_workflow;
import std;
import core_workflow;
import :integrations_read_by_id_step;

export namespace blog::handlers::integrations::read_by_id::workflow {
class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}
            .name("integrations_read_by_id")
            .step<ExecuteStep>()
            .build();
    }
};
} // namespace blog::handlers::integrations::read_by_id::workflow
