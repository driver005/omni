export module blog_handlers:comments_destroy_flow;
export import core_workflow;
import :domain_all;
import std;
import core_workflow;
import :comments_destroy_step;

export namespace blog::handlers::comments::destroy::workflow {
class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}
            .name("comments_destroy")
            .step<ExecuteStep>()
            .build();
    }
};
} // namespace blog::handlers::comments::destroy::workflow
