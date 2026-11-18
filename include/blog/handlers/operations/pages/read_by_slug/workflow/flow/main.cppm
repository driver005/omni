export module blog_operations:pages_read_by_slug_flow;

import std;
import core_workflow;
import :pages_read_by_slug_step;

export namespace blog::handlers::operations::pages::read_by_slug::workflow {

class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}.name("pages_read_by_slug").step<ExecuteStep>().build();
    }
};

} // namespace
