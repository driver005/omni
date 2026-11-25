export module blog_handlers:members_members_metafields_values_flow;
import :domain_all;
export import core_workflow;
import std;
import core_workflow;
import :members_members_metafields_values_step;

export namespace blog::handlers::members::members_metafields_values::workflow {
class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}
            .name("members_members_metafields_values")
            .step<ExecuteStep>()
            .build();
    }
};
} // namespace blog::handlers::members::members_metafields_values::workflow
