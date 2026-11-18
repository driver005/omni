export module blog_operations:automated_emails_automated_emails_id_test_post_flow;

import std;
import core_workflow;
import :automated_emails_automated_emails_id_test_post_step;

export namespace blog::handlers::operations::automated_emails::automated_emails_id_test_post::workflow {

class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}.name("automated_emails_automated_emails_id_test_post").step<ExecuteStep>().build();
    }
};

} // namespace
