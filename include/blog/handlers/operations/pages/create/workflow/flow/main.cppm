export module blog_operations:pages_create_flow;

import std;
import core_workflow;
import :pages_create_step;

export namespace blog::handlers::operations::pages::create::workflow {

class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}.name("pages_create").step<ExecuteStep>().build();
    }
};

} // namespace
