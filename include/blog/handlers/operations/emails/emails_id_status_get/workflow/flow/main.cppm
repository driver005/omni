export module blog_operations:emails_emails_id_status_get_flow;

import std;
import core_workflow;
import :emails_emails_id_status_get_step;

export namespace blog::handlers::operations::emails::emails_id_status_get::workflow {

class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}.name("emails_emails_id_status_get").step<ExecuteStep>().build();
    }
};

} // namespace
