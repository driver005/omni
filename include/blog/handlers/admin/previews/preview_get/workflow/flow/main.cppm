export module blog_handlers:previews_preview_get_flow;
export import core_workflow;
import std;
import core_workflow;
import :previews_preview_get_step;

export namespace blog::handlers::previews::preview_get::workflow {
class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}
            .name("previews_preview_get")
            .step<ExecuteStep>()
            .build();
    }
};
} // namespace blog::handlers::previews::preview_get::workflow
