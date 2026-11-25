export module blog_handlers:comments_undislike_flow;
export import core_workflow;
import :domain_all;
import std;
import core_workflow;
import :comments_undislike_step;

export namespace blog::handlers::comments::undislike::workflow {
class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}
            .name("comments_undislike")
            .step<ExecuteStep>()
            .build();
    }
};
} // namespace blog::handlers::comments::undislike::workflow
