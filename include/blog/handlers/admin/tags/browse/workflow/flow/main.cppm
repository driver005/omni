export module blog_handlers:tags_browse_flow;
export import core_workflow;
import std;
import core_workflow;
import :tags_browse_step;

export namespace blog::handlers::tags::browse::workflow {
class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}.name("tags_browse").step<ExecuteStep>().build();
    }
};
} // namespace blog::handlers::tags::browse::workflow
