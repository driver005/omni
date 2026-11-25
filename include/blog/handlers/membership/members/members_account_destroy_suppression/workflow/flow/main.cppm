export module blog_handlers:members_account_destroy_suppression_flow;
import :domain_all;
export import core_workflow;
import std;
import core_workflow;
import :members_account_destroy_suppression_step;

export namespace blog::handlers::members::members_account::destroy_suppression::workflow {
class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}
            .name("members_account_destroy_suppression")
            .step<ExecuteStep>()
            .build();
    }
};
} // namespace blog::handlers::members::members_account::destroy_suppression::workflow
