export module blog_operations:automated_emails_browse_flow;

import std;
import core_workflow;
import :automated_emails_browse_step;

export namespace blog::handlers::operations::automated_emails::browse::workflow {

class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}.name("automated_emails_browse").step<ExecuteStep>().build();
    }
};

} // namespace
