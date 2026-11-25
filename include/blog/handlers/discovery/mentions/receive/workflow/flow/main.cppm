export module blog_handlers:mentions_receive_flow;
export import core_workflow;
import std;
import core_workflow;
import :mentions_receive_step;

export namespace blog::handlers::mentions::receive::workflow {
class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}
            .name("mentions_receive")
            .step<ExecuteStep>()
            .build();
    }
};
} // namespace blog::handlers::mentions::receive::workflow
