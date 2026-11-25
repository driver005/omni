export module blog_handlers:members_account_read_flow;
import :domain_all;
export import core_workflow;
import std;
import core_workflow;
import :members_account_read_step;

export namespace blog::handlers::members::members_account::read::workflow {
class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}
            .name("members_account_read")
            .step<ExecuteStep>()
            .build();
    }
};
} // namespace blog::handlers::members::members_account::read::workflow
