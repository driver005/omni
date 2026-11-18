export module blog_operations:pages_destroy_flow;

import std;
import core_workflow;
import :pages_destroy_step;

export namespace blog::handlers::operations::pages::destroy::workflow {

class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}.name("pages_destroy").step<ExecuteStep>().build();
    }
};

} // namespace
