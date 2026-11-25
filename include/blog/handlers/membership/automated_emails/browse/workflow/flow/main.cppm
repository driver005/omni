export module blog_handlers:automated_emails_browse_flow;
export import core_workflow;
import std;
import core_workflow;
import :automated_emails_browse_step;

export namespace blog::handlers::automated_emails::browse::workflow {
class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}
            .name("automated_emails_browse")
            .step<ExecuteStep>()
            .build();
    }
};
} // namespace blog::handlers::automated_emails::browse::workflow
