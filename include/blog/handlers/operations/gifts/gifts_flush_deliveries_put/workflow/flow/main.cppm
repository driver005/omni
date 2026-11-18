export module blog_operations:gifts_gifts_flush_deliveries_put_flow;

import std;
import core_workflow;
import :gifts_gifts_flush_deliveries_put_step;

export namespace blog::handlers::operations::gifts::gifts_flush_deliveries_put::workflow {

class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}.name("gifts_gifts_flush_deliveries_put").step<ExecuteStep>().build();
    }
};

} // namespace
