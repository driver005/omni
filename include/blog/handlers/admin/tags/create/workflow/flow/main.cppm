export module blog_handlers:tags_create_flow;
export import core_workflow;
import std;
import core_workflow;
import :tags_create_step;

export namespace blog::handlers::tags::create::workflow {
class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}.name("tags_create").step<ExecuteStep>().build();
    }
};
} // namespace blog::handlers::tags::create::workflow
