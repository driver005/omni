export module blog_handlers:oembed_browse_flow;
export import core_workflow;
import std;
import core_workflow;
import :oembed_browse_step;

export namespace blog::handlers::oembed::browse::workflow {
class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}.name("oembed_browse").step<ExecuteStep>().build();
    }
};
} // namespace blog::handlers::oembed::browse::workflow
