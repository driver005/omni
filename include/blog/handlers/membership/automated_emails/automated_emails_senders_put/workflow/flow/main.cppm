export module blog_handlers:automated_emails_automated_emails_senders_put_flow;
export import core_workflow;
import std;
import core_workflow;
import :automated_emails_automated_emails_senders_put_step;

export namespace blog::handlers::automated_emails::automated_emails_senders_put::workflow {
class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}
            .name("automated_emails_automated_emails_senders_put")
            .step<ExecuteStep>()
            .build();
    }
};
} // namespace blog::handlers::automated_emails::automated_emails_senders_put::workflow
