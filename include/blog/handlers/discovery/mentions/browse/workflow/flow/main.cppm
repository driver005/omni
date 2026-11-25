export module blog_handlers:mentions_browse_flow;
export import core_workflow;
import std;
import core_workflow;
import :mentions_browse_step;

export namespace blog::handlers::mentions::browse::workflow {
class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}
            .name("mentions_browse")
            .step<ExecuteStep>()
            .build();
    }
};
} // namespace blog::handlers::mentions::browse::workflow
