export module blog_handlers:members_account_update_flow;
import :domain_all;
export import core_workflow;
import std;
import core_workflow;
import :members_account_update_step;

export namespace blog::handlers::members::members_account::update::workflow {
class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}
            .name("members_account_update")
            .step<ExecuteStep>()
            .build();
    }
};
} // namespace blog::handlers::members::members_account::update::workflow
