export module blog_operations:automated_emails_automated_emails_verifications_put_flow;

import std;
import core_workflow;
import :automated_emails_automated_emails_verifications_put_step;

export namespace blog::handlers::operations::automated_emails::automated_emails_verifications_put::workflow {

class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}.name("automated_emails_automated_emails_verifications_put").step<ExecuteStep>().build();
    }
};

} // namespace
