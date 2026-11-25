export module blog_handlers:emails_emails_id_retry_put_flow;
export import core_workflow;
import std;
import core_workflow;
import :emails_emails_id_retry_put_step;

export namespace blog::handlers::emails::emails_id_retry_put::workflow {
class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}
            .name("emails_emails_id_retry_put")
            .step<ExecuteStep>()
            .build();
    }
};
} // namespace blog::handlers::emails::emails_id_retry_put::workflow
