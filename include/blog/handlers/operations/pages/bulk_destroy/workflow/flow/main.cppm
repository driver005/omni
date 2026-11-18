export module blog_operations:pages_bulk_destroy_flow;

import std;
import core_workflow;
import :pages_bulk_destroy_step;

export namespace blog::handlers::operations::pages::bulk_destroy::workflow {

class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}.name("pages_bulk_destroy").step<ExecuteStep>().build();
    }
};

} // namespace
