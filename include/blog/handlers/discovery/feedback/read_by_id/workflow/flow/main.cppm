export module blog_handlers:feedback_read_by_id_flow;
export import core_workflow;
import std;
import core_workflow;
import :feedback_read_by_id_step;

export namespace blog::handlers::feedback::read_by_id::workflow {
class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}
            .name("feedback_read_by_id")
            .step<ExecuteStep>()
            .build();
    }
};
} // namespace blog::handlers::feedback::read_by_id::workflow
