export module blog_handlers:comments_dislike_flow;
export import core_workflow;
import :domain_all;
import std;
import core_workflow;
import :comments_dislike_step;

export namespace blog::handlers::comments::dislike::workflow {
class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}
            .name("comments_dislike")
            .step<ExecuteStep>()
            .build();
    }
};
} // namespace blog::handlers::comments::dislike::workflow
