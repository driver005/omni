export module blog_handlers:comments_like_flow;
export import core_workflow;
import :domain_all;
import std;
import core_workflow;
import :comments_like_step;

export namespace blog::handlers::comments::like::workflow {
class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}.name("comments_like").step<ExecuteStep>().build();
    }
};
} // namespace blog::handlers::comments::like::workflow
