export module blog_handlers:offers_create_flow;
export import core_workflow;
import std;
import core_workflow;
import :offers_create_step;

export namespace blog::handlers::offers::create::workflow {
class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}.name("offers_create").step<ExecuteStep>().build();
    }
};
} // namespace blog::handlers::offers::create::workflow
