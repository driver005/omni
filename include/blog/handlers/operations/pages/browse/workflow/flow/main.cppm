export module blog_operations:pages_browse_flow;

import std;
import core_workflow;
import :pages_browse_step;

export namespace blog::handlers::operations::pages::browse::workflow {

class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}.name("pages_browse").step<ExecuteStep>().build();
    }
};

} // namespace
