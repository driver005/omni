export module blog_handlers:recommendations_create_flow;
export import core_workflow;
import std;
import core_workflow;
import :recommendations_create_step;

export namespace blog::handlers::recommendations::create::workflow {
class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}
            .name("recommendations_create")
            .step<ExecuteStep>()
            .build();
    }
};
} // namespace blog::handlers::recommendations::create::workflow
