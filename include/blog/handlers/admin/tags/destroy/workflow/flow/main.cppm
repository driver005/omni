export module blog_handlers:tags_destroy_flow;
export import core_workflow;
import std;
import core_workflow;
import :tags_destroy_step;

export namespace blog::handlers::tags::destroy::workflow {
class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}.name("tags_destroy").step<ExecuteStep>().build();
    }
};
} // namespace blog::handlers::tags::destroy::workflow
