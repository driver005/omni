export module blog_handlers:session_bulk_destroy_flow;
export import core_workflow;
import std;
import core_workflow;
import :session_bulk_destroy_step;

export namespace blog::handlers::session::bulk_destroy::workflow {
class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}
            .name("session_bulk_destroy")
            .step<ExecuteStep>()
            .build();
    }
};
} // namespace blog::handlers::session::bulk_destroy::workflow
