export module blog_operations:oembed_browse_flow;

import std;
import core_workflow;
import :oembed_browse_step;

export namespace blog::handlers::operations::oembed::browse::workflow {

class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}.name("oembed_browse").step<ExecuteStep>().build();
    }
};

} // namespace
