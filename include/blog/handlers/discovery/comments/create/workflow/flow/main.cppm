export module blog_handlers:comments_create_flow;
export import core_workflow;
import std;
import core_workflow;
import :comments_create_step;

export namespace blog::handlers::comments::create::workflow {
class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}
            .name("comments_create")
            .step<ExecuteStep>()
            .build();
    }
};
} // namespace blog::handlers::comments::create::workflow
