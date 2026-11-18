export module blog_operations:db_browse_flow;

import std;
import core_workflow;
import :db_browse_step;

export namespace blog::handlers::operations::db::browse::workflow {

class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}.name("db_browse").step<ExecuteStep>().build();
    }
};

} // namespace
