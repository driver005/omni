export module blog_handlers:exports_create_flow;
export import core_workflow;
import std;
import core_workflow;
import :exports_create_step;

export namespace blog::handlers::exports::create::workflow {
class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}.name("exports_create").step<ExecuteStep>().build();
    }
};
} // namespace blog::handlers::exports::create::workflow
