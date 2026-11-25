export module blog_handlers:members_put_detail_flow;
export import core_workflow;
import std;
import core_workflow;
import :members_put_detail_step;

export namespace blog::handlers::members::put_detail::workflow {
class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}
            .name("members_put_detail")
            .step<ExecuteStep>()
            .build();
    }
};
} // namespace blog::handlers::members::put_detail::workflow
