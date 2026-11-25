export module blog_handlers:comments_counts_flow;
export import core_workflow;
import :domain_all;
import std;
import core_workflow;
import :comments_counts_step;

export namespace blog::handlers::comments::counts::workflow {
class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}
            .name("comments_counts")
            .step<ExecuteStep>()
            .build();
    }
};
} // namespace blog::handlers::comments::counts::workflow
