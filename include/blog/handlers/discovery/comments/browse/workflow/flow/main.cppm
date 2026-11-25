export module blog_handlers:comments_browse_flow;
export import core_workflow;
import std;
import core_workflow;
import :comments_browse_step;

export namespace blog::handlers::comments::browse::workflow {
class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}
            .name("comments_browse")
            .step<ExecuteStep>()
            .build();
    }
};
} // namespace blog::handlers::comments::browse::workflow
