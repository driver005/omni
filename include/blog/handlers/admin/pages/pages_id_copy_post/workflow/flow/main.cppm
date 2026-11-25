export module blog_handlers:pages_pages_id_copy_post_flow;
export import core_workflow;
import std;
import core_workflow;
import :pages_pages_id_copy_post_step;

export namespace blog::handlers::pages::pages_id_copy_post::workflow {
class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}
            .name("pages_pages_id_copy_post")
            .step<ExecuteStep>()
            .build();
    }
};
} // namespace blog::handlers::pages::pages_id_copy_post::workflow
