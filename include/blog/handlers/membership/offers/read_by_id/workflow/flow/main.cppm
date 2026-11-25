export module blog_handlers:offers_read_by_id_flow;
export import core_workflow;
import std;
import core_workflow;
import :offers_read_by_id_step;

export namespace blog::handlers::offers::read_by_id::workflow {
class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}
            .name("offers_read_by_id")
            .step<ExecuteStep>()
            .build();
    }
};
} // namespace blog::handlers::offers::read_by_id::workflow
